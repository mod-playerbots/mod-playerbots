/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "ToCActions_Anubarak.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <vector>

#include "Creature.h"
#include "EncounterHelpers.h"
#include "LastMovementValue.h"
#include "Player.h"
#include "Playerbots.h"
#include "RaidCombatHelpers.h"
#include "RaidTankDefensive.h"
#include "Spell.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include "Timer.h"
#include "ToCData.h"
#include "ToCHelpers_Anubarak.h"

using namespace TrialOfTheCrusaderHelpers;
using namespace EncounterHelpers;

namespace
{

constexpr uint32 ANUBARAK_STALL_MS = 500;
constexpr uint32 SPELL_CAT_FORM = 768;
constexpr float ANUBARAK_RANGED_REACH_MARGIN = 2.0f;
constexpr float ANUBARAK_BOOKED_TOLERANCE = 0.5f;
constexpr float ANUBARAK_ESCORT_REAIM = 3.0f;
constexpr float ANUBARAK_SIDE_SHOUT_REACH = 10.0f;
// A bubble older than this since the cold save cast it isn't the save's to cancel
constexpr uint32 ANUBARAK_COLD_BUBBLE_CANCEL_MS = 3000;

constexpr uint32 SPELL_DEMONIC_CIRCLE_SUMMON = 48018;

char const* const PENETRATING_COLD_HEALS[] = {
    "greater heal", "flash heal", "penance",
    "healing touch", "nourish", "regrowth",
    "holy light", "flash of light", "holy shock",
    "lesser healing wave", "healing wave", "riptide",
};

char const* const CLASS_TAUNTS[] = {"taunt", "hand of reckoning", "dark command", "growl"};
char const* const BURROWER_SHOUTS[] = {"challenging roar", "challenging shout"};
char const* const BURROWER_CLOSERS[] = {"feral charge - bear", "intercept", "avenger's shield", "death grip"};

// MoveTo refuses a repeat of its last point for up to 5 s even once the bot has stopped (a potion's
// StopMoving, a knockback), so drop the booking when the bot stands still past the issue.
void ReleaseStalledWalk(PlayerbotAI* botAI, Player* bot)
{
    LastMovement& last = botAI->GetAiObjectContext()->GetValue<LastMovement&>("last movement")->Get();
    if (!bot->isMoving() && last.msTime && getMSTimeDiff(last.msTime, getMSTime()) > ANUBARAK_STALL_MS)
        last.clear();
}

bool BookedOn(LastMovement const& last, Position const& spot)
{
    return last.msTime && std::fabs(last.lastMoveToX - spot.GetPositionX()) <= ANUBARAK_BOOKED_TOLERANCE &&
           std::fabs(last.lastMoveToY - spot.GetPositionY()) <= ANUBARAK_BOOKED_TOLERANCE;
}

// A walk waits behind any booking of equal or higher priority until it runs out
void DropForeignBooking(LastMovement& last, Position const& own, MovementPriority upTo)
{
    if (last.msTime && last.priority <= upTo && !BookedOn(last, own))
        last.clear();
}

// A point move issued mid channel never starts its spline, though MoveTo still reports it issued
void BreakChannelPinningTheFeet(Player* bot)
{
    if (bot->IsMovementPreventedByCasting())
        bot->InterruptNonMeleeSpells(true);
}

// True when the bot can cast spell on target from where it stands. Otherwise stop is how close to
// walk first, edge to edge as MoveTo(target, distance) measures it.
bool InSpellReach(PlayerbotAI* botAI, Player* bot, Unit* target, char const* spell, float& stop)
{
    uint32 const spellId = botAI->GetAiObjectContext()->GetValue<uint32>("spell id", spell)->Get();
    SpellInfo const* info = sSpellMgr->GetSpellInfo(spellId);

    // Self range covers War Stomp and Shockwave, which only reach what stands next to the bot
    if (!info || !info->RangeEntry || info->RangeEntry->ID == 1 || (info->RangeEntry->Flags & SPELL_RANGE_MELEE))
    {
        stop = CONTACT_DISTANCE;
        return bot->IsWithinMeleeRange(target);
    }

    float const range = bot->GetSpellMaxRangeForTarget(target, info);
    stop = std::max(range - ANUBARAK_RANGED_REACH_MARGIN, CONTACT_DISTANCE);
    return bot->IsWithinCombatRange(target, range) && bot->IsWithinLOSInMap(target);
}

// CastSpell selects the target, and faces it for a spell that needs facing, before the cast can fail.
// Gated so a taunt on cooldown doesn't do that every tick to a tank holding something else.
bool TryClassTaunt(PlayerbotAI* botAI, Player* bot, Unit* target)
{
    for (char const* taunt : CLASS_TAUNTS)
    {
        float stop = 0.0f;
        if (botAI->CanCastSpell(taunt, target) && InSpellReach(botAI, bot, target, taunt, stop))
            return CastClassTaunt(botAI, target);
    }

    return false;
}

// The swarm keeps everyone under the rescue pct, so in phase 3 that would let every taunt go from any range
bool TauntLands(Unit* victim, float dist, bool swarm)
{
    return dist <= ANUBARAK_TAUNT_ARRIVE ||
           (!swarm && victim && victim->GetHealthPct() < ANUBARAK_TAUNT_RESCUE_PCT);
}

// Loose burrowers are mostly on healers or ranged, so first Righteous Defense off whoever most of them hit, then
// a shout, the class taunt, a charge or pull
bool TryBurrowerPickup(PlayerbotAI* botAI, Player* bot, AnubarakBurrowerPick const& pick)
{
    float stop = 0.0f;
    bool const swarm = GetAnubarakPhase(botAI) == AnubarakPhase::Swarm;
    // Righteous Defense taunts everything hitting the member, him included
    Creature* boss = GetAnubarak(bot);
    bool const bossOnRescue = boss && pick.rescue && boss->GetVictim() == pick.rescue;
    if (pick.rescue && pick.rescue != bot && !bossOnRescue && botAI->CanCastSpell("righteous defense", pick.rescue) &&
        InSpellReach(botAI, bot, pick.rescue, "righteous defense", stop) &&
        TauntLands(pick.rescue, pick.rescueFar, swarm))
        return botAI->CastSpell("righteous defense", pick.rescue);

    // A shout takes in whatever stands within 10 yd plus its reach, his 10 among them
    bool const bossClear =
        !boss || bot->GetExactDist2d(boss) > ANUBARAK_SIDE_SHOUT_REACH + boss->GetCombatReach();
    if (pick.looseNear >= 2 && !pick.otherNear && bossClear)
        for (char const* shout : BURROWER_SHOUTS)
            if (botAI->CanCastSpell(shout, bot))
                return botAI->CastSpell(shout, bot);

    bool const lands = TauntLands(pick.loose->GetVictim(), bot->GetExactDist2d(pick.loose), swarm);
    if (lands && TryClassTaunt(botAI, bot, pick.loose))
        return true;

    // A DK owed a grip keeps Death Grip for the side's tank patch
    AnubarakGripDuty grip;
    bool const gripOwed = GetAnubarakGripDuty(bot, grip);
    for (char const* closer : BURROWER_CLOSERS)
    {
        if (gripOwed && std::strcmp(closer, "death grip") == 0)
            continue;

        if (botAI->CanCastSpell(closer, pick.loose) && InSpellReach(botAI, bot, pick.loose, closer, stop))
            return botAI->CastSpell(closer, pick.loose);
    }

    return false;
}

// Already walking there: every MoveTo clears the spline, so a walk in flight is left alone
bool WalkInFlight(PlayerbotAI* botAI, Player* bot, Position const& spot)
{
    ReleaseStalledWalk(botAI, bot);
    LastMovement& last = botAI->GetAiObjectContext()->GetValue<LastMovement&>("last movement")->Get();
    return bot->isMoving() && BookedOn(last, spot);
}

}

// Hand of Protection keeps the spike on the paladin; the rest hand it on at its current speed
bool AnubarakKiterImmunityAction::Execute(Event /*event*/)
{
    std::vector<char const*> immunities;
    switch (bot->getClass())
    {
        case CLASS_PALADIN:
            if (AnubarakKiterKeepsMark(bot))
                immunities = {"hand of protection", "divine shield"};
            else
                immunities = {"divine shield", "hand of protection"};
            break;
        case CLASS_MAGE:
            immunities = {"ice block"};
            break;
        case CLASS_ROGUE:
            immunities = {"vanish"};
            break;
        case CLASS_HUNTER:
            immunities = {"feign death"};
            break;
        default:
            break;
    }

    for (char const* immunity : immunities)
    {
        if (!botAI->CanCastSpell(immunity, bot))
            continue;

        // Feign Death drops the moment the hunter moves, the same ms it lands when cast mid run
        if (bot->getClass() == CLASS_HUNTER && bot->isMoving())
            bot->StopMoving();

        return botAI->CastSpell(immunity, bot);
    }

    return false;
}

bool AnubarakProtectKiterAction::Execute(Event /*event*/)
{
    Player* kiter = GetAnubarakKiterToProtect(bot);
    return kiter && botAI->CastSpell("hand of protection", kiter);
}

bool AnubarakFreeKiterAction::Execute(Event /*event*/)
{
    Player* kiter = GetAnubarakKiterToFree(bot);
    return kiter && botAI->CastSpell("hand of freedom", kiter);
}

bool AnubarakHopEscortAction::Execute(Event /*event*/)
{
    Position spot;
    if (!GetAnubarakEscortSpot(bot, spot))
        return false;

    // A straight walk to the spot can cross a patch, and the 80% slow leaves the kiter out of Hand reach
    Position const walk = GetAnubarakPatchSafeLeg(bot, spot);
    // The spot moves with the kiter, so a walk already headed close enough is left running
    ReleaseStalledWalk(botAI, bot);
    LastMovement& last = AI_VALUE(LastMovement&, "last movement");
    if (bot->isMoving() && last.msTime &&
        walk.GetExactDist2d(last.lastMoveToX, last.lastMoveToY) <= ANUBARAK_ESCORT_REAIM)
        return true;

    BreakChannelPinningTheFeet(bot);
    return MoveTo(bot->GetMapId(), walk.GetPositionX(), walk.GetPositionY(), walk.GetPositionZ(), false, false, false,
                  false, MovementPriority::MOVEMENT_COMBAT, true);
}

Player* AnubarakRedirectThreatAction::GetRedirectTank()
{
    AnubarakRedirect redirect;
    return GetAnubarakRedirect(bot, redirect) ? redirect.tank : nullptr;
}

Unit* AnubarakRedirectThreatAction::GetThreatDumpTarget()
{
    AnubarakRedirect redirect;
    return GetAnubarakRedirect(bot, redirect) ? redirect.dump : nullptr;
}

bool AnubarakKiteSpikeToPermafrostAction::Execute(Event /*event*/)
{
    // Heroic chains its speed boosts once the spike gains on it, the walk to a patch included; each is
    // instant and leaves the run going
    AnubarakKiteBoost boost;
    GetAnubarakKiteStage(bot, &boost);
    if (boost.nitro)
    {
        if (UseNitroBoosts(bot))
            return true;
    }
    else if (boost.spellId)
    {
        if (boost.face)
            bot->SetOrientation(boost.facing);

        // Cat Form won't cast from another form. RemoveShapeshift leaves Tree of Life on, so drop the aura.
        bool shifted = false;
        if (boost.catForm && botAI->CanCastSpell(SPELL_CAT_FORM, bot))
        {
            bot->RemoveAurasByType(SPELL_AURA_MOD_SHAPESHIFT);
            shifted = botAI->CastSpell(SPELL_CAT_FORM, bot);
        }

        Unit* target = boost.target.IsEmpty() ? bot : botAI->GetUnit(boost.target);
        if ((!boost.catForm || shifted) && target && botAI->CastSpell(boost.spellId, target))
            return true;

        // Dash goes next tick from Cat Form
        if (shifted)
            return true;
    }

    AnubarakKiteStand next;
    if (!GetAnubarakKiteStand(bot, next))
        return false;

    // Yields the tick on the stand: generic movers are vetoed for the marked bot, so it can cast here
    if (next.branch == AnubarakKiteBranch::Hold)
        return false;

    BreakChannelPinningTheFeet(bot);
    ReleaseStalledWalk(botAI, bot);

    // MoveTo refuses a point no higher than a FORCED leg still in flight, so a re-aimed stand, or the
    // first leg after this bot's own dodge, would wait out the stale one. Nothing outranks the kite.
    LastMovement& last = AI_VALUE(LastMovement&, "last movement");
    MovementPriority const held = last.priority;
    bool const lowered = held == MovementPriority::MOVEMENT_FORCED &&
                         last.lastMoveShort.GetExactDist(next.stand.GetPositionX(), next.stand.GetPositionY(),
                                                         next.stand.GetPositionZ()) > 0.01f;
    if (lowered)
        last.priority = MovementPriority::MOVEMENT_COMBAT;

    if (MoveTo(bot->GetMapId(), next.stand.GetPositionX(), next.stand.GetPositionY(), next.stand.GetPositionZ(),
               false, false, false, false, MovementPriority::MOVEMENT_FORCED, true))
        return true;

    if (lowered)
        last.priority = held;

    return false;
}

bool AnubarakAvoidSpikeAction::Execute(Event /*event*/)
{
    Position spot;
    if (GetAnubarakDodgeHold(bot, spot))
    {
        if (bot->GetExactDist2d(spot.GetPositionX(), spot.GetPositionY()) <= ANUBARAK_DODGE_ARRIVE)
            return false;

        // Every MoveTo clears the spline, so a walk still carrying the bot there is left alone
        if (bot->isMoving() && BookedOn(AI_VALUE(LastMovement&, "last movement"), spot))
            return true;

        return WalkToDodge(spot);
    }

    if (!GetAnubarakSpikeDodgeSpot(bot, spot))
        return false;

    SetAnubarakDodgeHold(bot, spot);
    return WalkToDodge(spot);
}

bool AnubarakAvoidSpikeAction::WalkToDodge(Position const& spot)
{
    BreakChannelPinningTheFeet(bot);
    ReleaseStalledWalk(botAI, bot);

    // A FORCED leg still in flight to some other spot refuses this one at the same priority
    LastMovement& last = AI_VALUE(LastMovement&, "last movement");
    MovementPriority const held = last.priority;
    bool const lowered = held == MovementPriority::MOVEMENT_FORCED && !BookedOn(last, spot);
    if (lowered)
        last.priority = MovementPriority::MOVEMENT_COMBAT;

    bool const issued = MoveTo(bot->GetMapId(), spot.GetPositionX(), spot.GetPositionY(), spot.GetPositionZ(), false,
                               false, false, false, MovementPriority::MOVEMENT_FORCED, true);
    if (!issued && lowered)
        last.priority = held;

    return issued || bot->isMoving();
}

bool AnubarakInterruptShadowStrikeAction::Execute(Event /*event*/)
{
    Creature* caster = GetAnubarakShadowStrikeDuty(bot);
    if (!caster)
        return false;

    char const* interrupt = AnubarakReadyInterrupt(bot, caster);
    if (!interrupt)
        return false;

    float stop = 0.0f;
    if (!InSpellReach(botAI, bot, caster, interrupt, stop))
        return MoveTo(caster, stop, MovementPriority::MOVEMENT_COMBAT);

    return botAI->CastSpell(interrupt, caster);
}

bool AnubarakTankDefensiveAction::Execute(Event /*event*/)
{
    bool const physical = _physicalOnly && GetAnubarakPhase(botAI) != AnubarakPhase::Swarm;
    uint32 const defensive = NextTankDefensive(botAI, bot, physical);
    return defensive && botAI->CastSpell(defensive, bot);
}

bool AnubarakColdSelfSaveAction::Execute(Event /*event*/)
{
    // The purge is done once it's up. Kept up, the mage can't cast and the paladin hits for half.
    if (_bubbleSpell && bot->HasAura(_bubbleSpell) &&
        getMSTimeDiff(_bubbleMs, getMSTime()) < ANUBARAK_COLD_BUBBLE_CANCEL_MS)
    {
        bot->RemoveAurasDueToSpell(_bubbleSpell);
        _bubbleSpell = 0;
        return true;
    }

    uint32 const save = GetAnubarakColdSelfSave(botAI);
    if (!save || !botAI->CastSpell(save, bot))
        return false;

    _bubbleSpell = IsAnubarakColdBubble(save) ? save : 0;
    _bubbleMs = getMSTime();
    return true;
}

bool AnubarakDestroyFrostSphereAction::Execute(Event /*event*/)
{
    Creature* sphere = GetAnubarakSphereToShoot(bot);
    if (!sphere)
        return false;

    if (Attack(sphere))
        return true;

    // On it already, and the rotation shoots it from here
    if (AnubarakSphereInReach(bot, sphere))
        return false;

    // Also covers Attack() refusing a sphere out of sight. A walk issued under a cast never starts
    // but still books its travel time. A bot parked on its dodge spot shoots only from there.
    Position stand;
    if (bot->IsMovementPreventedByCasting() || IsAnubarakDodgeHeld(botAI) ||
        !GetAnubarakSphereStand(bot, sphere, stand))
        return false;

    ReleaseStalledWalk(botAI, bot);
    return MoveTo(bot->GetMapId(), stand.GetPositionX(), stand.GetPositionY(), stand.GetPositionZ(), false, false,
                  false, false, MovementPriority::MOVEMENT_COMBAT, true);
}

bool AnubarakDpsTargetAction::Execute(Event /*event*/) { return AttackDirect(GetAnubarakDpsTarget(botAI)); }

bool AnubarakAssistTankHoldBurrowerAction::Execute(Event /*event*/)
{
    AnubarakBurrowerPick const pick = GetAnubarakBurrowerPick(botAI);
    Creature* target = pick.held ? pick.held : pick.loose;
    if (!target)
        return false;

    // Picks up even with one already held: a wave brings more burrowers than there are side tanks
    if (pick.loose && TryBurrowerPickup(botAI, bot, pick))
        return true;

    if (Attack(target))
        return true;

    // Walking off with one that isn't on this bot yet only leaves it on whoever it's hitting
    if (target != pick.held)
        return false;

    // A taunt holds it only 3 s, and walking off to the hold keeps it out of reach that long. Stands so it
    // comes in and the rotation lands threat first.
    if (pick.heldTaunted && !bot->IsWithinMeleeRange(pick.held))
        return false;

    // The spike dodge would walk the tank straight back out
    Position const& hold = pick.hold;
    if (bot->GetExactDist2d(hold.GetPositionX(), hold.GetPositionY()) <= ANUBARAK_TANK_PATCH_ARRIVE ||
        AnubarakSpikeReachesFirst(bot, hold))
        return false;

    return MoveTo(bot->GetMapId(), hold.GetPositionX(), hold.GetPositionY(), hold.GetPositionZ(), false, false,
                  false, false, MovementPriority::MOVEMENT_COMBAT, true);
}

bool AnubarakSideTankAreaThreatAction::Execute(Event /*event*/)
{
    AnubarakSideTankAoe aoe;
    if (!GetAnubarakSideTankAoe(botAI, aoe))
        return false;

    if (!aoe.point)
        return botAI->CastSpell(aoe.spell, aoe.target);

    uint32 const spellId = AI_VALUE2(uint32, "spell id", aoe.spell);
    return spellId && botAI->CastSpell(spellId, aoe.pos.GetPositionX(), aoe.pos.GetPositionY(), aoe.pos.GetPositionZ());
}

bool AnubarakDropCleaveBuffAction::Execute(Event /*event*/)
{
    char const* buff = GetAnubarakCleaveBuffToDrop(botAI);
    if (!buff)
        return false;

    botAI->RemoveAura(buff);
    return true;
}

bool AnubarakSideTankReadyAction::Execute(Event /*event*/)
{
    int8 const side = GetAnubarakBurrowerTankSide(bot);
    if (side < 0)
        return false;

    // Scarabs that come to it get Permafrost's slow on the patch
    for (Unit* attacker : bot->getAttackers())
        if (attacker->GetEntry() == static_cast<uint32>(ToCNpcs::NPC_SWARM_SCARAB) && Attack(attacker))
            return true;

    Position const hold = GetAnubarakSideTankHold(bot, static_cast<uint8>(side));
    if (bot->GetExactDist2d(hold.GetPositionX(), hold.GetPositionY()) <= ANUBARAK_TANK_PATCH_ARRIVE ||
        AnubarakSpikeReachesFirst(bot, hold) || WalkInFlight(botAI, bot, hold))
        return false;

    return MoveTo(bot->GetMapId(), hold.GetPositionX(), hold.GetPositionY(), hold.GetPositionZ(), false, false,
                  false, false, MovementPriority::MOVEMENT_COMBAT, true);
}

bool AnubarakGripFrostSphereAction::Execute(Event /*event*/)
{
    AnubarakGripDuty duty;
    if (!GetAnubarakGripDuty(bot, duty))
        return false;

    switch (duty.step)
    {
        case AnubarakGripStep::Kill:
            return duty.sphere && Attack(duty.sphere);
        case AnubarakGripStep::Walk:
        {
            Position const& spot = duty.spot;
            if (WalkInFlight(botAI, bot, spot))
                return true;

            return MoveTo(bot->GetMapId(), spot.GetPositionX(), spot.GetPositionY(), spot.GetPositionZ(), false,
                          false, false, false, MovementPriority::MOVEMENT_COMBAT, true);
        }
        case AnubarakGripStep::Cast:
        {
            Creature* sphere = duty.sphere;
            if (bot->GetExactDist(sphere) > ANUBARAK_GRIP_RANGE || !bot->IsWithinLOSInMap(sphere) ||
                !botAI->CanCastSpell("death grip", sphere))
                return false;

            // The sphere comes down where the DK stands as the grip goes out, so a walk still running
            // would leave the patch behind it
            if (bot->isMoving())
                bot->StopMoving();

            if (!botAI->CastSpell("death grip", sphere))
                return false;

            TrackAnubarakGripCast(bot, sphere, duty.side);
            return true;
        }
        default:
            return false;
    }
}

bool AnubarakBattleResSideTankAction::Execute(Event /*event*/)
{
    Player* dead = GetAnubarakBattleResTarget(botAI);
    if (!dead)
        return false;

    if (bot->GetExactDist(dead) > ANUBARAK_BATTLE_RES_REACH || !bot->IsWithinLOSInMap(dead))
        return !IsAnubarakDodgeHeld(botAI) && MoveTo(dead, ANUBARAK_BATTLE_RES_REACH - ANUBARAK_RANGED_REACH_MARGIN,
                                                     MovementPriority::MOVEMENT_COMBAT);

    return botAI->CastSpell("rebirth", dead);
}

bool AnubarakGatherAction::Execute(Event /*event*/)
{
    Position spot;
    if (!GetAnubarakGatherSpot(bot, spot))
        return false;

    if (bot->GetExactDist2d(spot.GetPositionX(), spot.GetPositionY()) <= ANUBARAK_GATHER_ARRIVE)
        return false;

    Position const walk = GetAnubarakPatchSafeLeg(bot, ValidateFloorPoint(bot, spot));
    // The spike's path still crosses the way back: wait where it stands, out of danger or the dodge would
    // have moved it
    if (!AnubarakWalkClearOfSpike(bot, spot))
    {
        if (WalkInFlight(botAI, bot, walk) || WalkInFlight(botAI, bot, spot))
        {
            bot->StopMoving();
            AI_VALUE(LastMovement&, "last movement").clear();
        }

        return false;
    }

    BreakChannelPinningTheFeet(bot);
    if (WalkInFlight(botAI, bot, walk))
        return true;

    return MoveTo(bot->GetMapId(), walk.GetPositionX(), walk.GetPositionY(), walk.GetPositionZ(), false, false, false,
                  false, MovementPriority::MOVEMENT_COMBAT, true);
}

bool AnubarakHealerStandAction::Execute(Event /*event*/)
{
    Position slot;
    if (!GetAnubarakHealerStand(bot, slot))
        return false;

    Position const walk = GetAnubarakPatchSafeLeg(bot, ValidateFloorPoint(bot, slot));
    // heals don't cast on the move: stop and let the ones below run. Booking cleared or the walk back
    // is refused as a repeat
    if (AnubarakHealerStandWaits(bot))
    {
        if (WalkInFlight(botAI, bot, walk) || WalkInFlight(botAI, bot, slot))
        {
            bot->StopMoving();
            AI_VALUE(LastMovement&, "last movement").clear();
        }

        return false;
    }

    if (WalkInFlight(botAI, bot, walk))
        return true;

    return MoveTo(bot->GetMapId(), walk.GetPositionX(), walk.GetPositionY(), walk.GetPositionZ(), false, false, false,
                  false, MovementPriority::MOVEMENT_COMBAT, true);
}

bool AnubarakRangedStandAction::Execute(Event /*event*/)
{
    Position stand;
    if (!GetAnubarakRangedStand(bot, stand))
        return false;

    Position const walk = GetAnubarakPatchSafeLeg(bot, stand);
    if (WalkInFlight(botAI, bot, walk))
        return true;

    return MoveTo(bot->GetMapId(), walk.GetPositionX(), walk.GetPositionY(), walk.GetPositionZ(), false, false, false,
                  false, MovementPriority::MOVEMENT_COMBAT, true);
}

bool AnubarakLeavePermafrostAction::Execute(Event /*event*/)
{
    Position spot;
    if (!GetAnubarakPermafrostStepOut(bot, spot))
        return false;

    BreakChannelPinningTheFeet(bot);
    if (WalkInFlight(botAI, bot, spot))
        return true;

    return MoveTo(bot->GetMapId(), spot.GetPositionX(), spot.GetPositionY(), spot.GetPositionZ(), false, false, false,
                  false, MovementPriority::MOVEMENT_COMBAT, true);
}

bool AnubarakMeleeBurrowerStandAction::Execute(Event /*event*/)
{
    Unit* target = AI_VALUE(Unit*, "current target");
    Creature* burrower = target ? target->ToCreature() : nullptr;
    Position stand;
    if (!burrower || !GetAnubarakMeleeBurrowerStand(bot, burrower, stand))
        return false;

    Position const walk = GetAnubarakPatchSafeLeg(bot, stand);
    if (WalkInFlight(botAI, bot, walk))
        return true;

    return MoveTo(bot->GetMapId(), walk.GetPositionX(), walk.GetPositionY(), walk.GetPositionZ(), false, false, false,
                  false, MovementPriority::MOVEMENT_COMBAT, true);
}

bool AnubarakBurrowerLeadAction::Execute(Event /*event*/)
{
    AnubarakBurrowerLead lead;
    if (!GetAnubarakBurrowerLead(botAI, lead) || !lead.walk)
        return false;

    Position const to = ValidateFloorPoint(bot, lead.to);
    if (!AnubarakWalkClearOfSpike(bot, to))
        return false;

    // The point moves with the side tank, so a walk headed close enough is left running
    Position const walk = GetAnubarakPatchSafeLeg(bot, to);
    ReleaseStalledWalk(botAI, bot);
    LastMovement& last = AI_VALUE(LastMovement&, "last movement");
    if (bot->isMoving() && last.msTime &&
        walk.GetExactDist2d(last.lastMoveToX, last.lastMoveToY) <= ANUBARAK_LEAD_REAIM)
        return true;

    BreakChannelPinningTheFeet(bot);
    return MoveTo(bot->GetMapId(), walk.GetPositionX(), walk.GetPositionY(), walk.GetPositionZ(), false, false, false,
                  false, MovementPriority::MOVEMENT_COMBAT, true);
}

bool AnubarakKiteScarabAction::Execute(Event /*event*/)
{
    Position step;
    if (!GetAnubarakScarabKite(botAI, step))
        return false;

    // Re-picked every tick 10 yd on from where the bot is, so a walk headed close enough is left running
    ReleaseStalledWalk(botAI, bot);
    LastMovement& last = AI_VALUE(LastMovement&, "last movement");
    if (bot->isMoving() && last.msTime &&
        step.GetExactDist2d(last.lastMoveToX, last.lastMoveToY) <= ANUBARAK_LEAD_REAIM)
        return true;

    BreakChannelPinningTheFeet(bot);
    return MoveTo(bot->GetMapId(), step.GetPositionX(), step.GetPositionY(), step.GetPositionZ(), false, false, false,
                  false, MovementPriority::MOVEMENT_COMBAT, true);
}

bool AnubarakTopUpManaAction::Execute(Event /*event*/)
{
    char const* spell = GetAnubarakManaTopUp(botAI);
    return spell && botAI->CastSpell(spell, bot);
}

bool AnubarakJudgeLightAction::Execute(Event /*event*/)
{
    Creature* boss = AnubarakLightJudgeDue(botAI) ? GetAnubarak(bot) : nullptr;
    return boss && botAI->CastSpell("judgement of light", boss);
}

// What caster form does, past its mana gate
bool AnubarakKiteCatDropAction::Execute(Event /*event*/)
{
    if (!AnubarakKiteCatDropDue(botAI))
        return false;

    botAI->RemoveShapeshift();
    return true;
}

bool AnubarakTankRescueAction::Execute(Event /*event*/)
{
    AnubarakTankRescue rescue;
    return GetAnubarakTankRescue(botAI, rescue) && botAI->CastSpell(rescue.spell, rescue.tank);
}

bool AnubarakLustAction::Execute(Event /*event*/)
{
    char const* lust = GetAnubarakLust(botAI);
    return lust && botAI->CastSpell(lust, bot);
}

bool AnubarakDemonicCircleAction::Execute(Event /*event*/)
{
    if (!AnubarakDemonicCircleDue(botAI))
        return false;

    // the circle lands at the caster's feet
    Position spot;
    if (GetAnubarakCircleSpot(botAI, spot) &&
        bot->GetExactDist2d(spot.GetPositionX(), spot.GetPositionY()) > ANUBARAK_CIRCLE_CENTRE_SLACK / 2.0f)
        return MoveTo(bot->GetMapId(), spot.GetPositionX(), spot.GetPositionY(), spot.GetPositionZ(), false, false,
                      false, true, MovementPriority::MOVEMENT_NORMAL, true);

    if (bot->isMoving())
        bot->StopMoving();

    return botAI->CastSpell(SPELL_DEMONIC_CIRCLE_SUMMON, bot);
}

bool AnubarakTankPickUpScarabAction::Execute(Event /*event*/)
{
    Creature* scarab = GetAnubarakScarabToPickUp(bot);
    if (!scarab)
        return false;

    bool const taunted = scarab->GetVictim() != bot && TryClassTaunt(botAI, bot, scarab);
    return Attack(scarab) || taunted;
}

bool AnubarakMainTankHoldBossAction::Execute(Event /*event*/)
{
    Creature* boss = GetAnubarak(bot);
    if (!boss || boss->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE))
        return false;

    // He never drops threat, so a taunt is only owed when a non-tank ended up with him
    Unit* victim = boss->GetVictim();
    Player* victimPlayer = victim ? victim->ToPlayer() : nullptr;
    bool const onNonTank = victim && victim != bot && !IsAnubarakTankPlayer(victimPlayer);
    if (onNonTank && TryClassTaunt(botAI, bot, boss))
        return true;

    if (Attack(boss))
        return true;

    // Slowed 80% in a patch the tank would crawl through every leg
    Position stepOut;
    if (GetAnubarakTankStepOut(bot, boss, stepOut))
    {
        ClearAnubarakDrag(bot);
        if (WalkInFlight(botAI, bot, stepOut))
            return false;

        DropForeignBooking(AI_VALUE(LastMovement&, "last movement"), stepOut, MovementPriority::MOVEMENT_COMBAT);
        return MoveTo(bot->GetMapId(), stepOut.GetPositionX(), stepOut.GetPositionY(), stepOut.GetPositionZ(), false,
                      false, false, false, MovementPriority::MOVEMENT_COMBAT, true);
    }

    Position point;
    bool const submerge = GetAnubarakSubmergeSpot(bot, point);
    std::vector<Position> const points =
        submerge ? GetAnubarakSubmergeDragPoints(bot, boss, point) : GetAnubarakDragPoints(bot, boss);
    if (points.empty())
    {
        ClearAnubarakDrag(bot);
        return false;
    }

    // A latched leg comes back whatever the point; otherwise the first point with a clean leg wins
    AnubarakDragLeg leg;
    for (Position const& candidate : points)
    {
        leg = GetAnubarakDragLeg(bot, boss, candidate, submerge);
        if (leg.step != AnubarakDragStep::None)
            break;
    }

    if (leg.step != AnubarakDragStep::Walk)
        return false;

    // True only on the issuing tick, so the tank's threat abilities get the ticks while he follows
    ReleaseStalledWalk(botAI, bot);
    LastMovement& last = AI_VALUE(LastMovement&, "last movement");
    Position const& spot = leg.tankSpot;
    if (BookedOn(last, spot))
        return false;

    // Backwards keeps the tank's front to him, so his first swings on arrival can be parried and blocked
    DropForeignBooking(last, spot, MovementPriority::MOVEMENT_COMBAT);
    return MoveTo(bot->GetMapId(), spot.GetPositionX(), spot.GetPositionY(), spot.GetPositionZ(), false, false, false,
                  false, MovementPriority::MOVEMENT_COMBAT, true, true);
}

bool AnubarakHealPenetratingColdAction::Execute(Event /*event*/)
{
    Player* target = GetAnubarakPenetratingColdHealTarget(botAI);
    if (!target)
        return false;

    if (CastAnubarakColdCarrierCover(botAI, target))
        return true;

    if (target->GetHealthPct() >= ANUBARAK_PENETRATING_COLD_HEAL_PCT)
        return false;

    for (char const* heal : PENETRATING_COLD_HEALS)
    {
        if (botAI->CanCastSpell(heal, target))
            return botAI->CastSpell(heal, target);
    }

    return false;
}

bool AnubarakSwarmHealAction::Execute(Event /*event*/)
{
    AnubarakSwarmHealPick const pick = GetAnubarakSwarmHealPick(botAI);
    if (!pick.target)
        return false;

    if (pick.floor)
        return CastAnubarakSwarmFloor(botAI);

    if (pick.rescue && CastAnubarakColdRescue(botAI, pick.target))
        return true;

    // the HoT or nothing: a bigger heal on a non-carrier goes straight back to him
    if (pick.hot)
        return CastAnubarakSwarmHot(botAI, pick.target);

    if (AnubarakSwarmShieldsTank(bot, pick.target) && botAI->CanCastSpell("power word: shield", pick.target))
        return botAI->CastSpell("power word: shield", pick.target);

    if (CastAnubarakColdCarrierCover(botAI, pick.target))
        return true;

    // Wild Growth heals five around the target, and the swarm hands the four others back to him
    return CastRaidThroughputHeal(botAI, pick.target,
                                  [](std::string const& heal) { return heal == "wild growth"; });
}

bool AnubarakAuraMasteryAction::Execute(Event /*event*/)
{
    return botAI->CastSpell("aura mastery", bot);
}

bool AnubarakAuraMasteryAction::isUseful() { return AnubarakAuraMasteryDue(botAI); }

bool AnubarakInnervateHealerAction::Execute(Event /*event*/)
{
    Player* healer = GetAnubarakInnervateTarget(botAI);
    return healer && botAI->CastSpell("innervate", healer);
}

bool AnubarakInnervateHealerAction::isUseful() { return AnubarakInnervateHealerDue(botAI); }

std::vector<NextAction> AnubarakInnervateHealerAction::getPrerequisites() { return {NextAction("caster form")}; }
