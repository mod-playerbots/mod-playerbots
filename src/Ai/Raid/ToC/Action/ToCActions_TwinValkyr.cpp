/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "ToCActions_TwinValkyr.h"
#include "ToCData.h"
#include "RaidDefensive.h"
#include "ToCHelpers_TwinValkyr.h"
#include "Playerbots.h"
#include "Creature.h"
#include "EncounterHelpers.h"
#include "LastMovementValue.h"
#include "MoveSpline.h"
#include "Spell.h"
#include "Timer.h"
#include "Unit.h"
#include "WorldSession.h"
#include "WorldPacket.h"

#include <cmath>

using namespace TrialOfTheCrusaderHelpers;
using namespace EncounterHelpers;

namespace
{
// MoveTo refuses the point it last issued for 5 s, moving or not, so a walk stopped short never
// restarts unless its booking goes once the bot has stood still this long.
constexpr uint32 STALLED_WALK_MS = 500;
constexpr float SAME_WALK_DESTINATION = 1.0f;
// a spot kept from an earlier wave is no answer to this one
constexpr uint32 DODGE_SPOT_LATCH_MS = 1000;
// inside the 30 yd taunt with room for her to drift
constexpr float TWIN_FETCH_STOP = 25.0f;
// the floor read is taken only this close to the stand's own height
constexpr float DOOR_FLOOR_TOLERANCE = 3.0f;
// a base walk lets a channel this close to its end finish
constexpr int32 BASE_WALK_CHANNEL_WAIT_MS = 1500;
// A twin following her tank in stops right over its spot, too close for any hunter stand. A step back
// from a lead this far out stays under the gap her chase ignores, so she parks there.
constexpr float TWIN_PARK_LEAD = 5.0f;
// longest wait at the lead for her to catch up
constexpr uint32 TWIN_PARK_SETTLE_MS = 3000;
// an orb this little past its trigger radius is about to go off on the bot
constexpr float ORB_CAST_CLIP_MARGIN = 0.5f;

// A warlock re-channels Drain Soul on low twins, so a base walk waiting on that never starts
bool LongChannel(Player* bot)
{
    Spell* channel = bot->GetCurrentSpell(CURRENT_CHANNELED_SPELL);
    return channel && channel->getState() == SPELL_STATE_CASTING &&
           channel->GetCastTimeRemaining() > BASE_WALK_CHANNEL_WAIT_MS;
}

Position DoorFloor(Player* bot, Position const& spot)
{
    float const ground = bot->GetMapWaterOrGroundLevel(spot.GetPositionX(), spot.GetPositionY(), spot.GetPositionZ());
    if (std::fabs(ground - spot.GetPositionZ()) > DOOR_FLOOR_TOLERANCE)
        return spot;

    return Position(spot.GetPositionX(), spot.GetPositionY(), ground);
}

// How far out of the spot toward the arena centre the point is
float ArenaOffset(Position const& point, Position const& spot)
{
    float const toCentreX = ARENA_CENTER.GetPositionX() - spot.GetPositionX();
    float const toCentreY = ARENA_CENTER.GetPositionY() - spot.GetPositionY();
    float const span = std::hypot(toCentreX, toCentreY);
    if (span <= 0.0f)
        return 0.0f;

    return ((point.GetPositionX() - spot.GetPositionX()) * toCentreX +
            (point.GetPositionY() - spot.GetPositionY()) * toCentreY) /
           span;
}

Position LeadPoint(Player* bot, Position const& spot)
{
    float const angle = spot.GetAngle(ARENA_CENTER.GetPositionX(), ARENA_CENTER.GetPositionY());
    return DoorFloor(bot, Position(spot.GetPositionX() + TWIN_PARK_LEAD * std::cos(angle),
                                   spot.GetPositionY() + TWIN_PARK_LEAD * std::sin(angle), spot.GetPositionZ()));
}

// Drops a stalled booking, and with releaseElsewhere one for any other destination, since forced doesn't
// outrank forced. A walk still headed for destination stays, so the duplicate check stops a re-issue.
void ReleaseWalk(PlayerbotAI* botAI, Player* bot, Position const& destination, bool releaseElsewhere)
{
    LastMovement& last = botAI->GetAiObjectContext()->GetValue<LastMovement&>("last movement")->Get();
    if (!last.msTime)
        return;

    bool const stalled = !bot->isMoving() && getMSTimeDiff(last.msTime, getMSTime()) > STALLED_WALK_MS;
    bool const elsewhere = last.lastMoveShort.GetExactDist2d(destination.GetPositionX(),
                                                             destination.GetPositionY()) > SAME_WALK_DESTINATION;
    if (stalled || (releaseElsewhere && elsewhere))
        last.clear();
}

// A healer, or a bot already inside the dodge's trigger, hands the tick on to its heal nodes or the
// dodge
bool StandForWait(PlayerbotAI* botAI, Player* bot, TwinWalkWait wait)
{
    if (bot->isMoving())
        bot->StopMoving();

    botAI->GetAiObjectContext()->GetValue<LastMovement&>("last movement")->Get().clear();
    return wait != TwinWalkWait::Heal && !PlayerbotAI::IsHeal(bot) &&
           !TwinOrbThreatens(botAI, TWIN_ORB_DODGE_CLEARANCE);
}
}  // namespace

bool TwinValkyrInterruptPactAction::Execute(Event /*event*/)
{
    Unit* twin = GetTwinCastingPact(botAI);
    if (!twin)
        return false;

    char const* interrupt = TwinReadyInterrupt(bot, twin);
    return interrupt && botAI->CastSpell(interrupt, twin);
}

bool TwinValkyrEssenceActionBase::Execute(Event /*event*/)
{
    TwinEssenceWant const want = GetWantedEssence(botAI);
    if (want.reason != _reason || want.colour == TwinColour::None)
        return false;

    return AcquireEssence(want.colour);
}

bool TwinValkyrEssenceActionBase::AcquireEssence(TwinColour colour)
{
    Creature* portal = GetEssencePortal(bot, colour);
    if (!portal)
        return false;

    // 3D on purpose: GetNPCIfCanInteractWith gates on IsWithinDistInMap, so a 2D check can call an
    // elevated portal in range and the gossip silently does nothing
    if (bot->GetDistance(portal) > INTERACTION_DISTANCE)
    {
        // ahead of the cast clip, so a healer waiting out an orb keeps its heal
        TwinWalkWait const wait = TwinValkyrWalkWait(botAI, portal->GetPosition(), _reason, TwinColour::None, _orbWait);
        if (wait != TwinWalkWait::None)
            return StandForWait(botAI, bot, wait);

        // a cast pins the feet; the base colour can wait for it to end, though not for a long channel,
        // and the others can't wait at all
        if (bot->IsMovementPreventedByCasting() && (_reason != TwinEssenceReason::Base || LongChannel(bot)))
            bot->InterruptNonMeleeSpells(true);

        TwinValkyrBoostEssenceWalk(botAI, _reason, portal);

        bool const urgent = _reason == TwinEssenceReason::Touch || _reason == TwinEssenceReason::Vortex;
        ReleaseWalk(botAI, bot, portal->GetPosition(), urgent);

        if (MoveTo(TRIAL_OF_THE_CRUSADER_MAP_ID, portal->GetPositionX(), portal->GetPositionY(),
                   portal->GetPositionZ(), false, false, false, false, MovementPriority::MOVEMENT_FORCED, true, false))
            return true;

        // Hold the tick while the walk is refused as a duplicate: lower nodes like a feral charge
        // would leap the bot back onto the twin, and the forced lock doesn't stop spell movers
        return bot->isMoving();
    }

    TwinWalkWait const wait = TwinValkyrWalkWait(botAI, portal->GetPosition(), _reason, colour, _orbWait);
    if (wait != TwinWalkWait::None)
        return StandForWait(botAI, bot, wait);

    // The essence comes from the portal's C++ OnGossipHello, so the hello opcode fires it with or
    // without DB gossip items.
    bot->CastStop();
    bot->SetFacingToObject(portal);

    WorldPacket packet;
    packet << portal->GetGUID();
    bot->GetSession()->HandleGossipHelloOpcode(packet);

    if (EssenceOf(bot) != colour)
        return false;

    // Gossip reach ends well short of the portal the walk is booked to. Stop here so the bot heads
    // straight back instead of finishing the walk under a forced lock.
    if (bot->isMoving())
    {
        bot->StopMoving();
        AI_VALUE(LastMovement&, "last movement").clear();
    }

    return true;
}

bool TwinValkyrWalkToNextPortalAction::Execute(Event /*event*/)
{
    Position spot;
    Creature* portal = nullptr;
    TwinEssenceReason reason = TwinEssenceReason::None;
    if (!TwinValkyrNextPortalDue(botAI, spot, portal, reason))
        return false;

    TwinWalkWait const wait = TwinValkyrWalkWait(botAI, spot, reason, TwinColour::None, _orbWait);
    if (wait != TwinWalkWait::None)
        return StandForWait(botAI, bot, wait);

    // A Touch ticks from the channel's end, so that walk has to be there first. The base walk has the
    // whole channel: a cast finishes, though not a long channel.
    bool const touch = reason == TwinEssenceReason::Touch;
    if (bot->IsMovementPreventedByCasting() && (touch || LongChannel(bot)))
        bot->InterruptNonMeleeSpells(true);

    if (touch)
        TwinValkyrBoostEssenceWalk(botAI, reason, portal);

    ReleaseWalk(botAI, bot, spot, touch);

    if (MoveTo(TRIAL_OF_THE_CRUSADER_MAP_ID, spot.GetPositionX(), spot.GetPositionY(), spot.GetPositionZ(), false,
               false, false, false, MovementPriority::MOVEMENT_FORCED, true, false))
        return true;

    // the walk in flight is refused as a duplicate; hold the tick until it lands
    return bot->isMoving();
}

bool TwinValkyrDodgeOrbAction::Execute(Event /*event*/)
{
    // only clip a cast when the orb is about to go off on this bot, a far one isn't worth it
    if (bot->IsMovementPreventedByCasting() && TwinOrbThreatens(botAI, TWIN_ORB_TRIGGER_RADIUS + ORB_CAST_CLIP_MARGIN))
        bot->InterruptNonMeleeSpells(true);

    uint32 const now = getMSTime();
    bool const kept = _dodgeSpotMs && getMSTimeDiff(_dodgeSpotMs, now) < DODGE_SPOT_LATCH_MS &&
                      TwinOrbSpotClear(botAI, _dodgeSpot, TWIN_ORB_DODGE_CLEARANCE);
    if (!kept && !FindTwinOrbDodgeSpot(botAI, _dodgeSpot))
    {
        _dodgeSpotMs = 0;
        return false;
    }

    _dodgeSpotMs = now;

    if (bot->GetExactDist2d(_dodgeSpot.GetPositionX(), _dodgeSpot.GetPositionY()) <= CONTACT_DISTANCE)
        return false;

    ReleaseWalk(botAI, bot, _dodgeSpot, true);

    if (MoveTo(bot->GetMapId(), _dodgeSpot.GetPositionX(), _dodgeSpot.GetPositionY(), _dodgeSpot.GetPositionZ(),
               false, false, false, true, MovementPriority::MOVEMENT_FORCED, true, false))
        return true;

    // the walk in flight is refused as a duplicate; hold the tick until it lands
    return bot->isMoving();
}

bool TwinValkyrCastVortexRaidCoverAction::Execute(Event /*event*/)
{
    RaidPredictedHit hit;
    RaidCover const cover = GetRaidCover(botAI, TOC_DEFENSIVE_SCOPE, GetTwinVortexRaidHit(botAI, hit) ? &hit : nullptr);
    uint32 const spellId = RaidCoverSpellId(botAI, cover);
    if (!spellId)
        return false;

    // a channel fails any other cast's check, and a moving bot breaks its own channel
    if (bot->IsNonMeleeSpellCast(false))
        bot->InterruptNonMeleeSpells(false);

    if (IsRaidCoverChannel(cover) && bot->isMoving())
        bot->StopMoving();

    if (!botAI->CastSpell(spellId, bot))
        return false;

    FinishRaidCover(botAI, cover);
    return true;
}

bool TwinValkyrCastVortexDefensiveAction::Execute(Event /*event*/)
{
    RaidPredictedHit hit;
    RaidPersonalPick const pick =
        GetRaidPersonalDefensive(botAI, TOC_DEFENSIVE_SCOPE, GetTwinVortexHit(botAI, hit) ? &hit : nullptr);
    if (!pick)
        return false;

    if (bot->IsNonMeleeSpellCast(false))
        bot->InterruptNonMeleeSpells(false);

    return pick.consumable ? botAI->DoSpecificAction(pick.consumable, Event(), true)
                           : botAI->CastSpell(pick.spellId, bot);
}

bool TwinValkyrMoveToDoorStandAction::Execute(Event /*event*/)
{
    TwinDoorStand const stand = GetTwinDoorStand(bot);
    if (stand.role == TwinDoorRole::None)
        return false;

    Position const spot = DoorFloor(bot, stand.spot);
    if (bot->GetExactDist2d(spot) <= TWIN_DOOR_PARK)
        return false;

    TwinWalkWait const wait = TwinValkyrWalkWait(botAI, spot, TwinEssenceReason::None, TwinColour::None, _orbWait);
    if (wait != TwinWalkWait::None)
        return StandForWait(botAI, bot, wait);

    ReleaseWalk(botAI, bot, spot, false);

    // the tunnel is straight and has no navmesh
    if (MoveTo(TRIAL_OF_THE_CRUSADER_MAP_ID, spot.GetPositionX(), spot.GetPositionY(), spot.GetPositionZ(), false,
               false, false, true, MovementPriority::MOVEMENT_COMBAT, true, false))
        return true;

    return bot->isMoving();
}

bool TwinValkyrTankHoldAction::HoldTwin(Unit* twin)
{
    // Before the pull she waits 85 yd out, and attacking her already would make the attack command that
    // calls the pull a duplicate the bot refuses
    bool const fetch = TwinValkyrFetchDue(botAI, twin);
    if (!twin->IsInCombat() && !fetch)
        return WalkToDoor(twin);

    // the target can be out of sight round the tunnel mouth, and the walk below still has to run
    if (AI_VALUE(Unit*, "current target") != twin && Attack(twin))
        return true;

    // Taunt back an assigned twin that's on someone else. Taunts diminish on the twins, so the 5th
    // without a 15 s gap is immune.
    for (Unit* assigned : {GetFjola(botAI), GetEydis(botAI)})
        if (assigned && assigned->GetVictim() != bot && IsTwinTank(bot, assigned) &&
            CastClassTaunt(botAI, assigned))
            return true;

    return fetch ? FetchTwin(twin) : WalkToDoor(twin);
}

// Walks to 25 yd of her for the taunt. If it can't land, the door hold is off while fetching, so
// reach melee walks the rest.
bool TwinValkyrTankHoldAction::FetchTwin(Unit* twin)
{
    _doorWalk = false;
    if (bot->GetExactDist2d(twin) <= TWIN_FETCH_STOP)
        return false;

    Position const door = GetTwinDoorSpot(bot, twin);
    float const span = twin->GetExactDist2d(door);
    if (span <= 0.0f)
        return false;

    float const x = twin->GetPositionX() + (door.GetPositionX() - twin->GetPositionX()) * TWIN_FETCH_STOP / span;
    float const y = twin->GetPositionY() + (door.GetPositionY() - twin->GetPositionY()) * TWIN_FETCH_STOP / span;
    float const ground = bot->GetMapWaterOrGroundLevel(x, y, bot->GetPositionZ());
    float const z = ground > INVALID_HEIGHT ? ground : bot->GetPositionZ();

    ReleaseWalk(botAI, bot, Position(x, y, z), false);
    if (MoveTo(TRIAL_OF_THE_CRUSADER_MAP_ID, x, y, z, false, false, false, false, MovementPriority::MOVEMENT_COMBAT,
               true, false))
        return true;

    return bot->isMoving();
}

bool TwinValkyrTankHoldAction::WalkToDoor(Unit* twin)
{
    Position const spot = DoorFloor(bot, GetTwinDoorStand(bot).spot);

    // Only a walk in from the arena leads; one that never got out that far, or is left over from an
    // earlier pull, doesn't
    float const out = ArenaOffset(bot->GetPosition(), spot);
    if (twin->GetVictim() != bot || (_leading && !_leadReachedMs && out < TWIN_PARK_LEAD - TWIN_DOOR_UNPARK))
        _leading = false;
    else if (!_leading && out > TWIN_PARK_LEAD + TWIN_DOOR_UNPARK)
    {
        _leading = true;
        _leadReachedMs = 0;
    }

    if (_leading)
    {
        Position const lead = LeadPoint(bot, spot);
        if (!_leadReachedMs && bot->GetExactDist2d(lead) > TWIN_DOOR_PARK)
            return WalkFacingTwin(lead, twin);

        if (!_leadReachedMs)
            _leadReachedMs = getMSTime();

        // false so the rotation keeps her threat while she catches up
        if (!twin->movespline->Finalized() && getMSTimeDiff(_leadReachedMs, getMSTime()) < TWIN_PARK_SETTLE_MS)
            return false;

        _leading = false;
    }

    float const off = bot->GetExactDist2d(spot);
    if (off <= TWIN_DOOR_PARK || (!_doorWalk && off <= TWIN_DOOR_UNPARK))
    {
        _doorWalk = false;
        return false;
    }

    _doorWalk = true;
    return WalkFacingTwin(spot, twin);
}

bool TwinValkyrTankHoldAction::WalkFacingTwin(Position const& to, Unit* twin)
{
    ReleaseWalk(botAI, bot, to, false);

    // Backing up keeps her in front, and a tank takes no parry or block from behind. Only when she's on
    // the far side of the walk, or it would turn its back on her.
    float const toSpotX = to.GetPositionX() - bot->GetPositionX();
    float const toSpotY = to.GetPositionY() - bot->GetPositionY();
    float const toTwinX = twin->GetPositionX() - bot->GetPositionX();
    float const toTwinY = twin->GetPositionY() - bot->GetPositionY();
    bool const backwards = twin->GetVictim() == bot && toSpotX * toTwinX + toSpotY * toTwinY < 0.0f;

    if (MoveTo(TRIAL_OF_THE_CRUSADER_MAP_ID, to.GetPositionX(), to.GetPositionY(), to.GetPositionZ(), false, false,
               false, true, MovementPriority::MOVEMENT_COMBAT, true, backwards))
        return true;

    return bot->isMoving();
}

bool TwinValkyrMainTankHoldLightTwinAction::Execute(Event /*event*/)
{
    Unit* fjola = GetFjola(botAI);
    if (!fjola)
        return false;

    return HoldTwin(fjola);
}

bool TwinValkyrAssistTankHoldDarkTwinAction::Execute(Event /*event*/)
{
    Unit* eydis = GetEydis(botAI);
    if (!eydis)
        return false;

    return HoldTwin(eydis);
}

Player* TwinValkyrRedirectThreatAction::GetRedirectTank()
{
    Unit* twin = GetTwinDpsTarget(botAI);
    if (!twin)
        return nullptr;

    Player* tank = GetTwinTank(bot, twin);
    return tank && tank->IsAlive() ? tank : nullptr;
}

Unit* TwinValkyrRedirectThreatAction::GetThreatDumpTarget() { return GetTwinDpsTarget(botAI); }

bool TwinValkyrFocusTwinAction::Execute(Event /*event*/) { return AttackDirect(GetTwinDpsTarget(botAI)); }
