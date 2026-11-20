/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "KaraActions.h"
#include "EncounterHelpers.h"
#include "KaraHelpers.h"
#include "ObjectAccessor.h"
#include "Playerbots.h"
#include "RtiTargetValue.h"
#include "Spell.h"
#include "ThreatHelpers.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include <list>

using namespace KaraHelpers;
using namespace EncounterHelpers;

// Shared

bool KarazhanResetEncounterStatesAction::Execute(Event /*event*/)
{
    uint32 const instanceId = bot->GetInstanceId();
    bool reset = false;

    Action* redAction = context->GetAction("netherspite block red beam");
    if (redAction &&
        static_cast<NetherspiteBlockRedBeamAction*>(redAction)->ResetRedBeamState())
    {
        reset = true;
    }

    Action* areaAction = context->GetAction("netherspite move between blue and green beams");
    if (areaAction &&
        static_cast<NetherspiteMoveBetweenBlueAndGreenBeamsAction*>(areaAction)->ResetDestination())
    {
        reset = true;
    }

    Action* wolfAction = context->GetAction("big bad wolf red hood run away");
    if (wolfAction &&
        static_cast<BigBadWolfRedHoodRunAwayAction*>(wolfAction)->ResetRun())
    {
        reset = true;
    }

    Action* enfeebleAction = context->GetAction("prince malchezaar enfeebled bot run to safety");
    if (enfeebleAction &&
        static_cast<PrinceMalchezaarEnfeebledBotRunToSafetyAction*>(enfeebleAction)
            ->ResetDestination())
    {
        reset = true;
    }

    Action* hazardAction = context->GetAction("prince malchezaar move clear of hazards");
    if (hazardAction &&
        static_cast<PrinceMalchezaarMoveClearOfHazardsAction*>(hazardAction)->ResetDestination())
    {
        reset = true;
    }

    Action* tankAction = context->GetAction("prince malchezaar tank position boss");
    if (tankAction &&
        static_cast<PrinceMalchezaarTankPositionBossAction*>(tankAction)->ResetWalk())
    {
        reset = true;
    }

    Action* netherspitePetAction = context->GetAction("netherspite set pet stance");
    if (netherspitePetAction &&
        static_cast<NetherspiteSetPetStanceAction*>(netherspitePetAction)->ResetPetStance())
    {
        reset = true;
    }

    Action* petAction = context->GetAction("nightbane set pet stance");
    if (petAction &&
        static_cast<NightbaneSetPetStanceAction*>(petAction)->ResetPetStance())
    {
        reset = true;
    }

    if (!IsMechanicTrackerBot(bot, KARA_MAP_ID))
        return reset;

    reset |= ClearStrategyTargetIcon(bot, RtiTargetValue::skullIndex, {
        Id(KaraNpcs::NPC_BARONESS_DOROTHEA_MILLSTIPE),
        Id(KaraNpcs::NPC_LADY_CATRIONA_VON_INDI),
        Id(KaraNpcs::NPC_LADY_KEIRA_BERRYBUCK),
        Id(KaraNpcs::NPC_BARON_RAFE_DREUGER),
        Id(KaraNpcs::NPC_LORD_ROBIN_DARIS),
        Id(KaraNpcs::NPC_LORD_CRISPIN_FERENCE),
        Id(KaraNpcs::NPC_ROMULO),
        Id(KaraNpcs::NPC_JULIANNE),
        Id(KaraNpcs::NPC_DOROTHEE),
        Id(KaraNpcs::NPC_TITO),
        Id(KaraNpcs::NPC_ROAR),
        Id(KaraNpcs::NPC_STRAWMAN),
        Id(KaraNpcs::NPC_TINHEAD),
        Id(KaraNpcs::NPC_KILREK),
        Id(KaraNpcs::NPC_TERESTIAN_ILLHOOF),
    });

    reset |= KaraResetInstance(instanceId);

    return reset;
}

bool KarazhanSetTremorTotemAction::Execute(Event /*event*/)
{
    return botAI->CanCastSpell(Id(KaraSpells::SPELL_TREMOR_TOTEM), bot) &&
        botAI->CastSpell(Id(KaraSpells::SPELL_TREMOR_TOTEM), bot);
}

// Trash

bool ManaWarpCastStunSpellAction::Execute(Event /*event*/)
{
    Unit* warp = GetManaWarpToStun(botAI);
    if (!warp)
        return false;

    auto const castStun = [&](char const* spell)
    {
        return botAI->CanCastSpell(spell, warp) && botAI->CastSpell(spell, warp);
    };

    // The warp is within the bot's stun reach, but a Tauren's reach is War Stomp's, so melee stuns
    // still test melee range: CanCastSpell passes targets out of range, and a failed cast still
    // turns the bot.
    bool const inMeleeRange = bot->IsWithinMeleeRange(warp);
    bool stunned = false;
    switch (bot->getClass())
    {
        case CLASS_DRUID:
        {
            ShapeshiftForm const form = bot->GetShapeshiftForm();
            if (form == FORM_BEAR || form == FORM_DIREBEAR)
                stunned = inMeleeRange && castStun("bash");
            else if (form == FORM_CAT)
                stunned = inMeleeRange && castStun("maim");
            break;
        }
        case CLASS_PALADIN:
            stunned = castStun("hammer of justice");
            break;
        case CLASS_ROGUE:
            stunned = inMeleeRange && castStun("kidney shot");
            break;
        case CLASS_WARLOCK:
            stunned = castStun("shadowfury");
            break;
        case CLASS_WARRIOR:
            stunned = inMeleeRange && (castStun("concussion blow") || castStun("shockwave"));
            break;
        default:
            break;
    }

    if (stunned)
        return true;

    if (bot->getRace() != RACE_TAUREN || bot->GetExactDist(warp) > WAR_STOMP_RADIUS)
        return false;

    // Too late if Warp Breach would finish before War Stomp lands.
    Spell* breach = warp->FindCurrentSpellBySpellId(Id(KaraSpells::SPELL_WARP_BREACH));
    if (breach && breach->GetCastTimeRemaining() <= static_cast<int32>(WAR_STOMP_CAST_MS))
        return false;

    if (!botAI->CanCastSpell("war stomp", bot) || !botAI->CastSpell("war stomp", bot))
        return false;

    ClaimManaWarpForWarStomp(warp);
    return true;
}

// Standing by casts nothing, so the stun is off the global cooldown when the warp is due.
bool ManaWarpStandByToStunAction::Execute(Event /*event*/)
{
    Unit* warp = GetManaWarpToStunNext(botAI);
    if (!warp)
        return false;

    float const reach = GetManaWarpStunReach(bot);
    if (bot->GetExactDist(warp) <= reach)
        return true;

    // MoveTo measures edge to edge.
    float const distance = std::max(
        0.0f, reach - MANA_WARP_STUN_APPROACH_MARGIN - bot->GetCombatReach() -
            warp->GetCombatReach());
    return MoveTo(warp, distance, MovementPriority::MOVEMENT_COMBAT);
}

// Attumen the Huntsman

// Steps the tank away from every player in reach of Shadow Cleave (from the tank) or Intangible
// Presence (from Attumen), until each is clear.
bool AttumenTheHuntsmanAssistTankPositionUnmountedAction::Execute(Event /*event*/)
{
    Unit* midnight = FindThreatTarget(bot, "midnight");
    Unit* attumen = GetKaraBoss(bot, KaraBosses::DATA_ATTUMEN);
    if (!midnight || !attumen)
        return false;

    if (AI_VALUE(Unit*, "current target") != attumen)
        return Attack(attumen);

    if (attumen->GetVictim() != bot || midnight->GetVictim() == bot)
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    float shortfall = 0.0f;
    float sumX = 0.0f;
    float sumY = 0.0f;
    uint32 count = 0;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member == bot || !member->IsAlive() || member->GetMapId() != KARA_MAP_ID)
            continue;

        // A cleave's jump adds the member's reach; Intangible Presence doesn't.
        float const cleaveJump = SHADOW_CLEAVE_JUMP_RADIUS + member->GetCombatReach();
        float const memberShortfall = ATTUMEN_HAZARD_CLEARANCE + std::max(
            cleaveJump - bot->GetExactDist2d(member),
            INTANGIBLE_PRESENCE_RADIUS - attumen->GetExactDist2d(member));
        if (memberShortfall <= 0.0f)
            continue;

        shortfall = std::max(shortfall, memberShortfall);
        sumX += member->GetPositionX();
        sumY += member->GetPositionY();
        ++count;
    }

    if (!count)
        return false;

    float const angle = bot->GetAngle(sumX / count, sumY / count) + M_PI;
    Position const destination(
        bot->GetPositionX() + std::cos(angle) * shortfall,
        bot->GetPositionY() + std::sin(angle) * shortfall, bot->GetPositionZ());

    float moveX;
    float moveY;
    bool backwards;
    if (!GetStepToPosition(bot, destination, 0.0f, attumen, moveX, moveY, backwards))
        return false;

    float moveZ = bot->GetPositionZ();
    if (!bot->GetMap()->CheckCollisionAndGetValidCoords(
            bot, bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(), moveX, moveY, moveZ,
            false))
    {
        return false;
    }

    return MoveTo(
        KARA_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_COMBAT, true, backwards);
}

bool AttumenTheHuntsmanAttackMidnightAction::Execute(Event /*event*/)
{
    Unit* midnight = FindThreatTarget(bot, "midnight");
    return midnight && Attack(midnight);
}

bool AttumenTheHuntsmanTankPositionMountedAction::Execute(Event /*event*/)
{
    Unit* attumen = GetMountedAttumen(bot);
    if (!attumen)
        return false;

    if (AI_VALUE(Unit*, "current target") != attumen)
        return Attack(attumen);

    if (attumen->GetVictim() != bot || !bot->IsWithinMeleeRange(attumen))
        return false;

    // Do an LoS check because the party may pull Midnight to a place where the tank position (the
    // middle of the stable) is out of LoS and not reachable in a straight move.
    Position const& position = ATTUMEN_TANK_POSITION;
    if (!bot->IsWithinLOS(
            position.GetPositionX(), position.GetPositionY(), position.GetPositionZ()))
    {
        return false;
    }

    constexpr float arrivalDist = 2.0f;
    float moveX;
    float moveY;
    bool backwards;
    if (!GetStepToPosition(bot, position, arrivalDist, attumen, moveX, moveY, backwards))
        return false;

    return MoveTo(
        KARA_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_COMBAT, true, backwards);
}

bool AttumenTheHuntsmanStackBehindAction::Execute(Event /*event*/)
{
    Unit* attumen = GetMountedAttumen(bot);
    if (!attumen)
        return false;

    std::optional<Position> const position = GetAttumenStackPosition(bot, attumen);
    if (!position)
        return false;

    return MoveTo(
        KARA_MAP_ID, position->GetPositionX(), position->GetPositionY(), position->GetPositionZ(),
        false, false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
}

bool AttumenTheHuntsmanStopAttackingAction::Execute(Event /*event*/)
{
    return StopAttackingAndChanneling(bot);
}

bool AttumenTheHuntsmanSelectTargetForHoldAction::isUseful()
{
    return botAI->GetState() == BOT_STATE_NON_COMBAT;
}

// This keeps bots in combat during the brief period when Midnight turns invisible and becomes
// invalid but mounted Attumen has not yet engaged.
bool AttumenTheHuntsmanSelectTargetForHoldAction::Execute(Event /*event*/)
{
    return SelectTarget(
        botAI, AI_VALUE(Unit*, PlayerbotAI::IsTank(bot) ? "tank target" : "dps target"));
}

bool AttumenTheHuntsmanManageDpsTimerAction::Execute(Event /*event*/)
{
    uint32 const now = getMSTime();
    return EmplaceIfUnset(KaraState(bot->GetInstanceId()).attumenDpsHoldTimer, now);
}

// Moroes

bool MoroesMarkTargetAction::Execute(Event /*event*/)
{
    // A guest zones in only on its own aggro, so it can be late onto threat lists. In kill order.
    static std::vector<uint32> const guestEntries = {
        Id(KaraNpcs::NPC_BARONESS_DOROTHEA_MILLSTIPE),
        Id(KaraNpcs::NPC_LADY_CATRIONA_VON_INDI),
        Id(KaraNpcs::NPC_LADY_KEIRA_BERRYBUCK),
        Id(KaraNpcs::NPC_BARON_RAFE_DREUGER),
        Id(KaraNpcs::NPC_LORD_ROBIN_DARIS),
        Id(KaraNpcs::NPC_LORD_CRISPIN_FERENCE),
    };
    std::list<Creature*> guests;
    bot->GetCreatureListWithEntryInGrid(guests, guestEntries, MOROES_GUEST_SEARCH_RADIUS);

    for (uint32 entry : guestEntries)
    {
        for (Creature* guest : guests)
        {
            if (guest->GetEntry() == entry && guest->IsAlive())
                return MarkTargetWithSkull(bot, guest);
        }
    }

    return false;
}

// Maiden of Virtue

bool MaidenOfVirtueTankPositionBossAction::Execute(Event /*event*/)
{
    Unit* maiden = GetKaraBoss(bot, KaraBosses::DATA_MAIDEN);
    if (!maiden)
        return false;

    if (AI_VALUE(Unit*, "current target") != maiden)
        return Attack(maiden);

    if (maiden->GetVictim() != bot || !bot->IsWithinMeleeRange(maiden))
        return false;

    if (GetNearestRepentantHealer(bot, maiden))
        return false;

    Position const& position = MAIDEN_OF_VIRTUE_POSITION;
    float const distToPosition = maiden->GetExactDist2d(position);
    if (distToPosition <= MAIDEN_OF_VIRTUE_POSITION_TOLERANCE)
        return false;

    float const ratio = MAIDEN_OF_VIRTUE_TANK_LEAD_DISTANCE / distToPosition;
    Position const destination(
        position.GetPositionX() + (position.GetPositionX() - maiden->GetPositionX()) * ratio,
        position.GetPositionY() + (position.GetPositionY() - maiden->GetPositionY()) * ratio,
        bot->GetPositionZ());

    float moveX;
    float moveY;
    bool backwards;
    if (!GetStepToPosition(
            bot, destination, MAIDEN_OF_VIRTUE_POSITION_TOLERANCE, maiden, moveX, moveY, backwards))
    {
        return false;
    }

    return MoveTo(
        KARA_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_COMBAT, true, backwards);
}

bool MaidenOfVirtueMoveBossToStunnedHealerAction::Execute(Event /*event*/)
{
    Unit* maiden = GetKaraBoss(bot, KaraBosses::DATA_MAIDEN);
    if (!maiden || maiden->GetVictim() != bot)
        return false;

    Player* healer = GetNearestRepentantHealer(bot, maiden);
    if (!healer)
        return false;

    if (maiden->GetExactDist2d(healer) <= MAIDEN_REPENTANCE_BREAK_DISTANCE)
        return false;

    constexpr float arrivalDist = 2.0f;
    float moveX;
    float moveY;
    bool backwards;
    if (!GetStepToPosition(bot, *healer, arrivalDist, maiden, moveX, moveY, backwards))
        return false;

    return MoveTo(
        KARA_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_COMBAT, true, backwards);
}

bool MaidenOfVirtuePositionRangedBetweenPillarsAction::Execute(Event /*event*/)
{
    constexpr float arrivalDist = 2.0f;
    for (Position const& position : MAIDEN_OF_VIRTUE_RANGED_POSITIONS)
    {
        if (bot->GetExactDist2d(position) <= arrivalDist)
            return false;
    }

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    constexpr uint8 maxIndex = 7;
    uint8 index = 0;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !PlayerbotAI::IsRanged(member))
            continue;

        if (member == bot)
            break;

        if (index >= maxIndex)
        {
            index = 0;
            continue;
        }
        index++;
    }

    Position const& position = MAIDEN_OF_VIRTUE_RANGED_POSITIONS[index];
    return MoveTo(
        KARA_MAP_ID, position.GetPositionX(), position.GetPositionY(), position.GetPositionZ(),
        false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
}

bool MaidenOfVirtueSetGroundingTotemAction::Execute(Event /*event*/)
{
    return botAI->CanCastSpell(Id(KaraSpells::SPELL_GROUNDING_TOTEM), bot) &&
        botAI->CastSpell(Id(KaraSpells::SPELL_GROUNDING_TOTEM), bot);
}

// The Big Bad Wolf

bool BigBadWolfTankPositionBossAction::Execute(Event /*event*/)
{
    Unit* wolf = GetOperaActor(bot, "the big bad wolf");
    if (!wolf)
        return false;

    if (AI_VALUE(Unit*, "current target") != wolf)
        return Attack(wolf);

    if (wolf->GetVictim() != bot || !bot->IsWithinMeleeRange(wolf))
        return false;

    Position const& position = BIG_BAD_WOLF_TANK_POSITION;
    constexpr float arrivalDist = 2.0f;
    float moveX;
    float moveY;
    bool backwards;
    if (!GetStepToPosition(bot, position, arrivalDist, wolf, moveX, moveY, backwards))
        return false;

    return MoveTo(
        KARA_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false,
        false, false, MovementPriority::MOVEMENT_COMBAT, true, backwards);
}

bool BigBadWolfRedHoodUseEscapeAbilityAction::isUseful()
{
    return IsLittleRedRidingHood(bot) &&
        !bot->HasAura(Id(KaraSpells::SPELL_GOBLIN_ROCKET_BOOTS)) &&
        !bot->HasAura(Id(KaraSpells::SPELL_FEIGN_DEATH));
}

bool BigBadWolfRedHoodUseEscapeAbilityAction::Execute(Event /*event*/)
{
    if (bot->getClass() == CLASS_HUNTER &&
        !bot->HasSpellCooldown(Id(KaraSpells::SPELL_FEIGN_DEATH)))
    {
        return botAI->CanCastSpell("feign death", bot) && botAI->CastSpell("feign death", bot);
    }

    // I'm not sure what the exact issue is, but with BBW's reach (4.5 yd) and speed (8.5 yd/s),
    // which are from sniffs, it is geometrically impossible to outrun him with Hood's +50% speed.
    // Perhaps the chase mechanics in retail were different? In any case, something is clearly
    // wrong, so to get around it, a cheat is implemented to give +70% speed instead, which is in
    // most cases enough to avoid getting hit. The aura lasts 20s, same as Hood, so there's no need
    // to separately remove it, conveniently enough.
    return bot->AddAura(Id(KaraSpells::SPELL_GOBLIN_ROCKET_BOOTS), bot);
}

bool BigBadWolfRedHoodRunAwayAction::Execute(Event /*event*/)
{
    Aura const* hood = bot->GetAura(Id(KaraSpells::SPELL_LITTLE_RED_RIDING_HOOD));
    Unit* wolf = GetOperaActor(bot, "the big bad wolf");
    if (!hood || !wolf)
        return false;

    bot->CastStop();
    // Feared or stunned: the first step after it replans from where both are then.
    if (!botAI->CanMove())
        return false;

    if (GetMSTimeDiffToNow(_lastRunStepTime) > BIG_BAD_WOLF_RUN_RESTART_MS)
    {
        _run = GetBigBadWolfRunPlan(
            bot, *wolf, hood->GetDuration() / static_cast<float>(IN_MILLISECONDS));
    }

    if (bot->GetExactDist2d(BIG_BAD_WOLF_RUN_POSITIONS[_run.index]) <
        BIG_BAD_WOLF_RUN_TURN_DISTANCE)
    {
        _run.index = GetNextBigBadWolfRunIndex(_run.index, _run.direction);
    }

    float moveX;
    float moveY;
    bool backwards;
    if (!GetStepToPosition(
            bot, BIG_BAD_WOLF_RUN_POSITIONS[_run.index], BIG_BAD_WOLF_RUN_TURN_DISTANCE, nullptr,
            moveX, moveY, backwards))
    {
        return false;
    }

    if (!MoveTo(
            KARA_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
            MovementPriority::MOVEMENT_FORCED, true, backwards))
    {
        return false;
    }

    _lastRunStepTime = getMSTime();
    return true;
}

bool BigBadWolfProtectRedHoodAction::Execute(Event /*event*/)
{
    Unit* wolf = GetOperaActor(bot, "the big bad wolf");
    if (!wolf)
        return false;

    switch (bot->getClass())
    {
        // GetBigBadWolfTargetToProtect returns only humans.
        case CLASS_PALADIN:
        {
            return CastOnTarget(
                "hand of protection", GetBigBadWolfTargetToProtect(wolf), HAND_OF_PROTECTION_RANGE);
        }
        // Weakened Soul fails CanCastSpell, so only one priest shields at a time.
        case CLASS_PRIEST:
        {
            return CastOnTarget(
                "power word: shield", GetBigBadWolfRedHood(wolf), POWER_WORD_SHIELD_RANGE);
        }
        // The next swing at the target hits the warrior instead. Defensive Stance only, which
        // CanCastSpell tests.
        case CLASS_WARRIOR:
        {
            Player* target = GetBigBadWolfRedHood(wolf);
            return target && !target->HasAura(Id(KaraSpells::SPELL_INTERVENE)) &&
                !bot->IsWithinCombatRange(target, INTERVENE_MIN_RANGE) &&
                CastOnTarget("intervene", target, INTERVENE_MAX_RANGE);
        }
        default:
            return false;
    }
}

// Range is edge to edge. As I've noted ad nauseaum across raid strategy comments, CanCastSpell
// passes targets out of range.
bool BigBadWolfProtectRedHoodAction::CastOnTarget(char const* spell, Player* target, float range)
{
    return target && target != bot && bot->IsWithinCombatRange(target, range) &&
        botAI->CanCastSpell(spell, target) && botAI->CastSpell(spell, target);
}

// Romulo and Julianne

// Since Julianne is present (but faking death) in phase 2, the skull gets put on Romulo in phase 2,
// but he is the only valid target. This is useless but not adverse so I've let it be.
bool RomuloAndJulianneSkullHigherHealthAction::Execute(Event /*event*/)
{
    Unit* romulo = GetOperaActor(bot, "romulo");
    if (!romulo)
        return false;

    Unit* julianne = GetOperaActor(bot, "julianne");
    if (!julianne)
        return false;

    bool const romuloDown = romulo->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE);
    bool const julianneDown = julianne->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE);
    if (romuloDown && julianneDown)
        return false;

    if (romuloDown)
        return MarkTargetWithSkull(bot, julianne);

    if (julianneDown)
        return MarkTargetWithSkull(bot, romulo);

    Unit* higher = romulo->GetHealth() >= julianne->GetHealth() ? romulo : julianne;
    Unit* lower = higher == romulo ? julianne : romulo;
    if (higher->GetHealth() - lower->GetHealth() <= ROMULO_AND_JULIANNE_HEALTH_GAP)
        return false;

    return MarkTargetWithSkull(bot, higher);
}

// The Wizard of Oz

bool WizardOfOzMarkTargetAction::Execute(Event /*event*/)
{
    for (char const* actor : OZ_TARGETS)
    {
        if (Unit* target = GetOperaActor(bot, actor))
            return MarkTargetWithSkull(bot, target);
    }

    return false;
}

bool WizardOfOzCastScorchOnStrawmanAction::Execute(Event /*event*/)
{
    Unit* strawman = GetOperaActor(bot, "strawman");
    if (!strawman)
        return false;

    // Both measure edge to edge.
    if (!bot->IsWithinCombatRange(strawman, SCORCH_RANGE))
    {
        return MoveTo(
            strawman, SCORCH_RANGE - SCORCH_APPROACH_MARGIN, MovementPriority::MOVEMENT_COMBAT);
    }

    return botAI->CanCastSpell("scorch", strawman) && botAI->CastSpell("scorch", strawman);
}

// The Curator

bool TheCuratorMarkAstralFlareAction::Execute(Event /*event*/)
{
    Unit* flare = FindThreatTarget(bot, "astral flare");
    return flare && MarkTargetWithSkull(bot, flare);
}

bool TheCuratorTankPositionBossAction::Execute(Event /*event*/)
{
    Unit* curator = GetKaraBoss(bot, KaraBosses::DATA_CURATOR);
    if (!curator)
        return false;

    if (AI_VALUE(Unit*, "current target") != curator)
        return Attack(curator);

    if (curator->GetVictim() != bot || !bot->IsWithinMeleeRange(curator))
        return false;

    Position const& position = THE_CURATOR_TANK_POSITION;
    constexpr float arrivalDist = 2.0f;
    float moveX;
    float moveY;
    bool backwards;
    if (!GetStepToPosition(bot, position, arrivalDist, curator, moveX, moveY, backwards))
        return false;

    return MoveTo(
        KARA_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_COMBAT, true, backwards);
}

bool TheCuratorSpreadRangedAction::Execute(Event /*event*/)
{
    Player* nearestPlayer = GetNearestPlayerInRadius(bot, THE_CURATOR_RANGED_SPREAD_DISTANCE);
    return nearestPlayer &&
        FleePosition(nearestPlayer->GetPosition(), THE_CURATOR_RANGED_SPREAD_DISTANCE);
}

// Terestian Illhoof

bool TerestianIllhoofMarkTargetAction::Execute(Event /*event*/)
{
    // Chains do not attack and are not zoned in, and neither is Kil'rek after being resummoned.
    // Thus, we need to use grid searches, but they can be limited: chains only during a Sacrifice,
    // and Kil'rek only when Illhoof does not have Broken Pact.
    Unit* terestian = GetKaraBoss(bot, KaraBosses::DATA_TERESTIAN);
    std::vector<uint32> entries;
    if (IsTerestianSacrificeActive(bot))
        entries.push_back(Id(KaraNpcs::NPC_DEMON_CHAINS));

    if (!terestian || !terestian->HasAura(Id(KaraSpells::SPELL_BROKEN_PACT)))
        entries.push_back(Id(KaraNpcs::NPC_KILREK));

    if (!entries.empty())
    {
        constexpr float searchRadius = 100.0f;
        std::list<Creature*> adds;
        bot->GetCreatureListWithEntryInGrid(adds, entries, searchRadius);

        Creature* kilrek = nullptr;
        for (Creature* add : adds)
        {
            if (!add->IsAlive())
                continue;

            if (add->GetEntry() == Id(KaraNpcs::NPC_DEMON_CHAINS))
                return MarkTargetWithSkull(bot, add);

            kilrek = add;
        }

        if (kilrek)
            return MarkTargetWithSkull(bot, kilrek);
    }

    return terestian && MarkTargetWithSkull(bot, terestian);
}

// Shade of Aran

bool ShadeOfAranRunAwayFromArcaneExplosionAction::Execute(Event /*event*/)
{
    Unit* aran = GetKaraBoss(bot, KaraBosses::DATA_ARAN);
    if (!aran)
        return false;

    float const distance = bot->GetExactDist(aran);
    if (distance >= ARAN_ARCANE_EXPLOSION_CLEARANCE)
        return false;

    bot->CastStop();
    if (bot->GetExactDist2d(aran) > ARAN_PULLED_ONTO_BOSS_DISTANCE)
        return MoveAway(aran, ARAN_ARCANE_EXPLOSION_CLEARANCE - distance);

    uint32 const direction = bot->GetGUID().GetCounter() % ARAN_ARCANE_EXPLOSION_EXIT_DIRECTIONS;
    float const angle =
        2.0f * static_cast<float>(M_PI) * direction / ARAN_ARCANE_EXPLOSION_EXIT_DIRECTIONS;
    float x = aran->GetPositionX() + ARAN_ARCANE_EXPLOSION_CLEARANCE * std::cos(angle);
    float y = aran->GetPositionY() + ARAN_ARCANE_EXPLOSION_CLEARANCE * std::sin(angle);
    float z = bot->GetPositionZ();
    bool const exact = bot->GetMap()->CheckCollisionAndGetValidCoords(
        bot, bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(), x, y, z);

    return MoveTo(
        KARA_MAP_ID, x, y, z, false, false, true, exact, MovementPriority::MOVEMENT_COMBAT, false);
}

bool ShadeOfAranMoveBehindBlizzardAction::Execute(Event /*event*/)
{
    constexpr MovementPriority priority = MovementPriority::MOVEMENT_FORCED;
    if (IsWaitingForLastMove(priority))
        return false;

    std::optional<Position> const position = GetBlizzardEscapePosition(bot);
    if (!position)
        return false;

    bot->CastStop();
    return MoveTo(
        KARA_MAP_ID, position->GetPositionX(), position->GetPositionY(), position->GetPositionZ(),
        false, false, false, false, priority, true, false);
}

// I will not move when Flame Wreath is cast or the raid blows up.
bool ShadeOfAranStopMovingAction::Execute(Event /*event*/)
{
    AI_VALUE(LastMovement&, "last movement").Set(nullptr);
    if (!bot->isMoving())
        return false;

    bot->GetMotionMaster()->Clear();
    bot->StopMoving();
    return true;
}

bool ShadeOfAranMarkElementalAction::Execute(Event /*event*/)
{
    GuidVector const elementals =
        context->GetValue<GuidVector>("shade of aran unbanished elementals")->Get();
    for (ObjectGuid const& guid : elementals)
    {
        Unit* elemental = botAI->GetUnit(guid);
        if (elemental && elemental->IsAlive())
            return MarkTargetWithSkull(bot, elemental);
    }

    return false;
}

bool ShadeOfAranRangedMaintainDistanceAction::Execute(Event /*event*/)
{
    constexpr MovementPriority priority = MovementPriority::MOVEMENT_COMBAT;
    if (IsWaitingForLastMove(priority))
        return false;

    Unit* aran = GetKaraBoss(bot, KaraBosses::DATA_ARAN);
    if (!aran)
        return false;

    float const distance = bot->GetExactDist2d(aran);
    float goal;
    if (distance > ARAN_RANGED_MAX_DISTANCE)
        goal = ARAN_RANGED_MAX_DISTANCE;
    else if (distance < ARAN_RANGED_MIN_DISTANCE)
        goal = ARAN_RANGED_MIN_DISTANCE;
    else
        return false;

    float const angle = aran->GetAngle(bot);
    Position const position(
        aran->GetPositionX() + goal * std::cos(angle),
        aran->GetPositionY() + goal * std::sin(angle), bot->GetPositionZ());

    float moveX;
    float moveY;
    bool backwards;
    if (!GetStepToPosition(
            bot, position, ARAN_RANGED_BAND_TOLERANCE, nullptr, moveX, moveY, backwards))
    {
        return false;
    }

    // Wait out a Blizzard rather than walk back into it.
    for (Position const& patch : GetDynamicObjectPositions(
             bot, ARAN_ROOM_SEARCH_RADIUS, Id(KaraSpells::SPELL_BLIZZARD)))
    {
        if (patch.GetExactDist2d(position) <= ARAN_BLIZZARD_CLEARANCE)
            return false;
    }

    return MoveTo(
        KARA_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false, priority, true,
        backwards);
}

// Netherspite

// The red beam dance (5 seconds in, 5 seconds out). This allows a single tank to block the beam for
// an entire phase by reducing the number of stacks accumulated.
bool NetherspiteBlockRedBeamAction::Execute(Event /*event*/)
{
    Unit* netherspite = GetKaraBoss(bot, KaraBosses::DATA_NETHERSPITE);
    if (!netherspite)
        return false;

    KaraInstanceState const* state = FindKaraState(netherspite->GetInstanceId());
    if (!state)
        return false;

    NetherspiteBeamState const& beam = state->netherspiteBeams[Id(NetherspiteBeam::Red)];
    if (beam.blocker != bot->GetGUID() || !beam.portal)
        return false;

    if (bot->HasAura(Id(KaraSpells::SPELL_RED_BEAM_DEBUFF)))
    {
        if (!_redBeamTimerWasSet)
        {
            _redBeamMoveTimer = getMSTime();
            _redBeamTimerWasSet = true;
        }
    }
    else
    {
        _redBeamTimerWasSet = false;
        _lastBeamMoveSideways = false;
    }

    constexpr uint32 intervalMs = 5 * IN_MILLISECONDS;
    if (_redBeamTimerWasSet && GetMSTimeDiffToNow(_redBeamMoveTimer) >= intervalMs)
    {
        _lastBeamMoveSideways = !_lastBeamMoveSideways;
        _redBeamMoveTimer = getMSTime();
    }

    float x = beam.blockerPosition.GetPositionX();
    float y = beam.blockerPosition.GetPositionY();
    if (_lastBeamMoveSideways)
    {
        float const length = netherspite->GetExactDist2d(*beam.portal);
        if (length == 0.0f)
            return false;

        // A unit vector to the left of the line, as Netherspite faces the portal.
        float const leftX = (netherspite->GetPositionY() - beam.portal->GetPositionY()) / length;
        float const leftY = (beam.portal->GetPositionX() - netherspite->GetPositionX()) / length;
        x += leftX * NETHERSPITE_RED_DANCE_STEP;
        y += leftY * NETHERSPITE_RED_DANCE_STEP;
    }

    // The position is 18 yd from Netherspite toward the portal, so it shifts as he moves. Thus, the
    // tank needs to wait until Netherspite stops moving to establish a stable position.
    if (netherspite->isMoving() ||
        bot->GetExactDist2d(x, y) <= NETHERSPITE_BEAM_POSITION_TOLERANCE)
    {
        return false;
    }

    return MoveTo(
        KARA_MAP_ID, x, y, bot->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_FORCED, true, false);
}

bool NetherspiteBlockBlueOrGreenBeamAction::Execute(Event /*event*/)
{
    constexpr MovementPriority priority = MovementPriority::MOVEMENT_FORCED;
    if (IsWaitingForLastMove(priority))
        return false;

    Unit* netherspite = GetKaraBoss(bot, KaraBosses::DATA_NETHERSPITE);
    if (!netherspite)
        return false;

    KaraInstanceState const* state = FindKaraState(bot->GetInstanceId());
    Position const* position =
        state ? FindNetherspiteBlockerPosition(*state, bot->GetGUID()) : nullptr;
    if (!position)
        return false;

    std::optional<Position> const step =
        GetStepAroundVoidZones(bot, netherspite, *state, *position);
    if (!step)
        return false;

    bot->CastStop();
    return MoveTo(
        KARA_MAP_ID, step->GetPositionX(), step->GetPositionY(), bot->GetPositionZ(), false, false,
        false, false, priority, true, false);
}

bool NetherspiteWaitToBlockBeamAction::Execute(Event /*event*/)
{
    constexpr MovementPriority priority = MovementPriority::MOVEMENT_FORCED;
    if (IsWaitingForLastMove(priority))
        return false;

    Unit* netherspite = GetKaraBoss(bot, KaraBosses::DATA_NETHERSPITE);
    if (!netherspite)
        return false;

    KaraInstanceState const* state = FindKaraState(bot->GetInstanceId());
    Position const* position =
        state ? FindNetherspiteNextPosition(*state, bot->GetGUID()) : nullptr;
    if (!position)
        return false;

    std::optional<Position> const step =
        GetStepAroundVoidZones(bot, netherspite, *state, *position);
    if (!step)
        return false;

    bot->CastStop();
    return MoveTo(
        KARA_MAP_ID, step->GetPositionX(), step->GetPositionY(), bot->GetPositionZ(), false, false,
        false, false, priority, true, false);
}

bool NetherspiteMoveBetweenBlueAndGreenBeamsAction::Execute(Event /*event*/)
{
    constexpr MovementPriority priority = MovementPriority::MOVEMENT_COMBAT;
    if (IsWaitingForLastMove(priority))
        return false;

    Unit* netherspite = GetKaraBoss(bot, KaraBosses::DATA_NETHERSPITE);
    if (!netherspite)
        return false;

    KaraInstanceState const* state = FindKaraState(netherspite->GetInstanceId());
    if (!state || !AreNetherspitePortalsKnown(*state))
        return false;

    if (!_destination || !CanMoveBetweenNetherspiteBeams(
            bot, netherspite, *state, _destination->GetPositionX(), _destination->GetPositionY()))
    {
        _destination = FindPositionBetweenNetherspiteBeams(bot, netherspite, *state);
        if (!_destination)
            return false;
    }

    bot->CastStop();
    return MoveTo(
        KARA_MAP_ID, _destination->GetPositionX(), _destination->GetPositionY(),
        bot->GetPositionZ(), false, false, false, false, priority, true, false);
}

bool NetherspiteMoveToBanishSlotAction::Execute(Event /*event*/)
{
    constexpr MovementPriority priority = MovementPriority::MOVEMENT_FORCED;
    if (IsWaitingForLastMove(priority))
        return false;

    Unit* netherspite = GetKaraBoss(bot, KaraBosses::DATA_NETHERSPITE);
    if (!netherspite)
        return false;

    KaraInstanceState const* state = FindKaraState(bot->GetInstanceId());
    NetherspiteBanishSlot const* slot =
        state ? FindNetherspiteBanishSlot(*state, bot->GetGUID()) : nullptr;
    if (!slot)
        return false;

    std::optional<Position> const step =
        GetStepAroundVoidZones(bot, netherspite, *state, slot->position);
    if (!step)
        return false;

    bot->CastStop();
    return MoveTo(
        KARA_MAP_ID, step->GetPositionX(), step->GetPositionY(), bot->GetPositionZ(), false, false,
        false, false, priority, true, false);
}

bool NetherspiteStopAttackingAction::Execute(Event /*event*/)
{
    return StopAttackingAndChanneling(bot);
}

bool NetherspiteSelectTargetForHoldAction::isUseful()
{
    return botAI->GetState() == BOT_STATE_NON_COMBAT;
}

bool NetherspiteSelectTargetForHoldAction::Execute(Event /*event*/)
{
    return SelectTarget(
        botAI, AI_VALUE(Unit*, PlayerbotAI::IsTank(bot) ? "tank target" : "dps target"));
}

bool NetherspiteSetPetStanceAction::Execute(Event /*event*/)
{
    Unit* netherspite = GetKaraBoss(bot, KaraBosses::DATA_NETHERSPITE);
    return netherspite && SetMinionStancesForHold(
        bot, IsNetherspiteDpsHeld(bot, netherspite), _previousReactStates);
}

bool NetherspiteSetPetStanceAction::ResetPetStance()
{
    bool const restored = SetMinionStancesForHold(bot, false, _previousReactStates);
    _previousReactStates.clear();
    return restored;
}

bool NetherspiteManageTimersAndTrackersAction::Execute(Event /*event*/)
{
    Unit* netherspite = GetKaraBoss(bot, KaraBosses::DATA_NETHERSPITE);
    if (!netherspite)
        return false;

    KaraInstanceState& state = KaraState(netherspite->GetInstanceId());
    UpdateVoidZones(state);

    if (IsBanishPhase(netherspite))
    {
        ResetIfSet(state.netherspitePortalPhaseTimer);
        state.netherspiteBeams = {};
        UpdateBanishSlots(netherspite, state);
        return false;
    }

    EmplaceIfUnset(state.netherspitePortalPhaseTimer, getMSTime());
    state.netherspiteBanishSlots.clear();

    constexpr float searchRadius = 150.0f;
    for (NetherspiteBeamData const& data : NETHERSPITE_BEAMS)
    {
        NetherspiteBeamState& beamState = state.netherspiteBeams[Id(data.beam)];
        if (beamState.portal)
            continue;

        if (Unit* portal = bot->FindNearestCreature(Id(data.portal), searchRadius))
            beamState.portal = portal->GetPosition();
    }

    if (!AreNetherspitePortalsKnown(state))
        return false;

    // An otherwise-eligible bot blocking or waiting for one beam can't be picked for another.
    std::vector<ObjectGuid> taken;
    for (NetherspiteBeamState const& beamState : state.netherspiteBeams)
    {
        taken.push_back(beamState.blocker);
        taken.push_back(beamState.next);
    }

    for (NetherspiteBeamData const& data : NETHERSPITE_BEAMS)
        UpdateBeamBlockers(netherspite, state, data, taken);

    return false;
}

// Beam blocker candidates are bots only, in order of preference:
// Red: tanks only. No stack limit.
// Blue: mana DPS, then non-mana DPS.
// Green: non-mana DPS (no stack limit), then healers, then mana DPS.
// The current blocker stays a candidate until it surpasses the stack limit. Any other bot cannot be
// a candidate unless it is under the stack limit.
std::vector<Player*> NetherspiteManageTimersAndTrackersAction::GetBeamCandidates(
    NetherspiteBeamData const& data, NetherspiteBeamState const& beamState,
    std::vector<ObjectGuid> const& taken)
{
    std::vector<Player*> tiers[3];
    Group* group = bot->GetGroup();
    if (!group)
        return {};

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || member->GetMapId() != KARA_MAP_ID ||
            !GET_PLAYERBOT_AI(member) || member->HasAura(Id(data.exhaustion)))
        {
            continue;
        }

        ObjectGuid const guid = member->GetGUID();
        if (guid != beamState.blocker && guid != beamState.next &&
            std::find(taken.begin(), taken.end(), guid) != taken.end())
        {
            continue;
        }

        uint8 const stackLimit = guid == beamState.blocker ?
            NETHERSPITE_BEAM_STACK_LIMIT : NETHERSPITE_BEAM_STACK_LIMIT - 1;
        if (HasNetherspiteBeamStackLimit(member, data.beam) &&
            GetNetherspiteBeamStacks(member, data) >= stackLimit)
        {
            continue;
        }

        bool const isMana = IsManaUser(member);
        bool const isDps = PlayerbotAI::IsDps(member);
        bool const isHealer = PlayerbotAI::IsHeal(member);
        switch (data.beam)
        {
            case NetherspiteBeam::Red:
            {
                if (PlayerbotAI::IsTank(member))
                    tiers[0].push_back(member);
                break;
            }
            case NetherspiteBeam::Green:
            {
                if (isDps && !isMana)
                    tiers[0].push_back(member);
                else if (isHealer)
                    tiers[1].push_back(member);
                else if (isDps)
                    tiers[2].push_back(member);
                break;
            }
            case NetherspiteBeam::Blue:
            {
                if (isDps && isMana)
                    tiers[0].push_back(member);
                else if (isDps)
                    tiers[1].push_back(member);
                break;
            }
            default:
                break;
        }
    }

    std::vector<Player*> candidates;
    for (std::vector<Player*> const& tier : tiers)
        candidates.insert(candidates.end(), tier.begin(), tier.end());

    return candidates;
}

// Whether the waiting position is still usable: (1) in the band FindNextPosition picks from, within
// 0.5 yd, or moved up to 4 yd along the line by ShiftNextPositionAlongLine (hunters only away from
// Netherspite because 4 yd toward him would take them to the edge of his melee range, 20.8 yd or
// 21.3 under Bloodlust, where Auto Shot fails), and (2) clear of void zones and beams.
bool NetherspiteManageTimersAndTrackersAction::IsNextPositionGood(
    Unit* netherspite, Position const& portal, KaraInstanceState const& state, float side,
    Position const& position, float low, bool isHunter)
{
    NetherspiteBeamOffset const offset = GetNetherspiteBeamOffset(
        *netherspite, portal, position.GetPositionX(), position.GetPositionY());
    float const off = offset.off * side;

    if (off < NETHERSPITE_NEXT_MIN_OFF_LINE - NETHERSPITE_BEAM_POSITION_TOLERANCE ||
        off > NETHERSPITE_NEXT_MAX_OFF_LINE + NETHERSPITE_BEAM_POSITION_TOLERANCE)
    {
        return false;
    }

    float const lowShift = isHunter ? 0.0f : NETHERSPITE_NEXT_SHIFT_RANGE;
    if (offset.along < low - lowShift - NETHERSPITE_BEAM_POSITION_TOLERANCE ||
        offset.along > low + NETHERSPITE_NEXT_ALONG_RANGE + NETHERSPITE_NEXT_SHIFT_RANGE +
            NETHERSPITE_BEAM_POSITION_TOLERANCE)
    {
        return false;
    }

    return IsClearOfVoidZones(
            state, position.GetPositionX(), position.GetPositionY(), NETHERSPITE_VZ_CLEARANCE) &&
        IsClearOfNetherspiteBeams(
            *netherspite, state, position.GetPositionX(), position.GetPositionY());
}

// Where the next blocker waits: a random point 3.5-6 yd off the line on its inner side (toward
// the other beam), 17-19.5 yd from Netherspite along it (hunters 25-27.5), and clear of void zones
// and beams.
std::optional<Position> NetherspiteManageTimersAndTrackersAction::FindNextPosition(
    Unit* netherspite, Position const& portal, KaraInstanceState const& state, float side,
    float low)
{
    float const length = netherspite->GetExactDist2d(portal);
    float const dirX = (portal.GetPositionX() - netherspite->GetPositionX()) / length;
    float const dirY = (portal.GetPositionY() - netherspite->GetPositionY()) / length;
    for (uint8 i = 0; i < NETHERSPITE_POSITION_TRIES; ++i)
    {
        float const along = low + frand(0.0f, NETHERSPITE_NEXT_ALONG_RANGE);
        float const off =
            side * frand(NETHERSPITE_NEXT_MIN_OFF_LINE, NETHERSPITE_NEXT_MAX_OFF_LINE);
        float const x = netherspite->GetPositionX() + dirX * along - dirY * off;
        float const y = netherspite->GetPositionY() + dirY * along + dirX * off;
        if (!IsClearOfVoidZones(state, x, y, NETHERSPITE_VZ_CLEARANCE) ||
            !IsClearOfNetherspiteBeams(*netherspite, state, x, y))
        {
            continue;
        }

        if (std::optional<Position> const position = GetFloorPosition(netherspite, x, y))
            return position;
    }

    return std::nullopt;
}

// The bot waiting to block has a narrow band off the line, so a void zone dropped on it moves the
// position along the line instead, by the smallest shift that clears it. At each distance, toward
// Netherspite is tried first, so melee stay in range.
std::optional<Position> NetherspiteManageTimersAndTrackersAction::ShiftNextPositionAlongLine(
    Unit* netherspite, Position const& portal, KaraInstanceState const& state, float side,
    Position const& position, float low, bool isHunter)
{
    float const length = netherspite->GetExactDist2d(portal);
    float const dirX = (portal.GetPositionX() - netherspite->GetPositionX()) / length;
    float const dirY = (portal.GetPositionY() - netherspite->GetPositionY()) / length;
    constexpr float shiftStep = 0.5f;
    for (float shift = shiftStep; shift <= NETHERSPITE_NEXT_SHIFT_RANGE; shift += shiftStep)
    {
        for (float const direction : { -1.0f, 1.0f })
        {
            float const x = position.GetPositionX() + dirX * shift * direction;
            float const y = position.GetPositionY() + dirY * shift * direction;
            Position const shifted(x, y, position.GetPositionZ());
            if (!IsNextPositionGood(netherspite, portal, state, side, shifted, low, isHunter))
                continue;

            if (std::optional<Position> const floor = GetFloorPosition(netherspite, x, y))
                return floor;
        }
    }

    return std::nullopt;
}

std::optional<Position> NetherspiteManageTimersAndTrackersAction::GetFloorPosition(
    Unit* netherspite, float x, float y)
{
    float clippedX = x;
    float clippedY = y;
    float z = netherspite->GetPositionZ();
    if (!bot->GetMap()->CheckCollisionAndGetValidCoords(
            bot, netherspite->GetPositionX(), netherspite->GetPositionY(),
            netherspite->GetPositionZ(), clippedX, clippedY, z, false) ||
        std::abs(clippedX - x) >= 1.0f || std::abs(clippedY - y) >= 1.0f)
    {
        return std::nullopt;
    }

    return Position(clippedX, clippedY, z);
}

void NetherspiteManageTimersAndTrackersAction::UpdateBeamBlockers(
    Unit* netherspite, KaraInstanceState& state, NetherspiteBeamData const& data,
    std::vector<ObjectGuid>& taken)
{
    NetherspiteBeamState& beamState = state.netherspiteBeams[Id(data.beam)];
    if (!beamState.portal || netherspite->GetExactDist2d(*beamState.portal) == 0.0f)
        return;

    std::vector<Player*> const candidates = GetBeamCandidates(data, beamState, taken);
    Player* blocker = ObjectAccessor::GetPlayer(*bot, beamState.blocker);
    Player* next = nullptr;
    bool const isBlockerEligible = blocker &&
        std::find(candidates.begin(), candidates.end(), blocker) != candidates.end();
    bool const isBlockerAtLimit = blocker && !isBlockerEligible && blocker->IsAlive() &&
        !blocker->HasAura(Id(data.exhaustion)) && HasNetherspiteBeamStackLimit(blocker, data.beam);

    if (data.beam != NetherspiteBeam::Red)
    {
        Player* wanted = ObjectAccessor::GetPlayer(*bot, beamState.next);
        for (Player* candidate : candidates)
        {
            if (candidate == blocker)
                continue;
            if (!next || candidate == wanted)
                next = candidate;
            if (candidate == wanted)
                break;
        }
    }

    // Near the end of the Portal Phase, the blocker does not swap, even at threshold stacks.
    bool const isPhaseEnding = state.netherspitePortalPhaseTimer &&
        getMSTimeDiff(*state.netherspitePortalPhaseTimer, getMSTime()) >= NETHERSPITE_LAST_SWAP_MS;
    Player* newBlocker = blocker;
    bool const isDueToStepOn = !isPhaseEnding && blocker && next &&
        HasNetherspiteBeamStackLimit(blocker, data.beam) &&
        GetNetherspiteBeamStacks(blocker, data) >= NETHERSPITE_NEXT_STEPS_ON_AT_STACKS;

    if (!isBlockerEligible && !isBlockerAtLimit)
        newBlocker = next ? next : (candidates.empty() ? nullptr : candidates.front());
    else if (isDueToStepOn && IsOnNetherspiteBeam(netherspite, *beamState.portal, next))
        newBlocker = next;
    else if (isBlockerAtLimit && !next && !isPhaseEnding)
        newBlocker = nullptr;

    if (newBlocker != blocker)
    {
        YellNetherspiteBeamHandover(blocker, newBlocker, data.beam);
        blocker = newBlocker;
        next = nullptr;
        if (data.beam != NetherspiteBeam::Red)
        {
            for (Player* candidate : candidates)
            {
                if (candidate != blocker)
                {
                    next = candidate;
                    break;
                }
            }
        }
    }

    ObjectGuid const blockerGuid = blocker ? blocker->GetGUID() : ObjectGuid::Empty;
    ObjectGuid const nextGuid = next ? next->GetGUID() : ObjectGuid::Empty;
    bool const isNextNew = nextGuid != beamState.next;
    beamState.blocker = blockerGuid;
    beamState.next = nextGuid;
    if (blocker)
    {
        taken.push_back(blockerGuid);
        // The red tank ignores void zones: moving out of one could move Netherspite and disrupt
        // the other beam blockers.
        float const distance = data.beam == NetherspiteBeam::Red ? NETHERSPITE_BLOCKER_DISTANCE :
            FindNetherspiteBeamPositionDistance(*netherspite, *beamState.portal, state, blocker);
        beamState.blockerPosition =
            GetPositionOnNetherspiteBeam(*netherspite, *beamState.portal, distance);
    }

    if (!next)
        return;

    taken.push_back(nextGuid);
    if (!isPhaseEnding && blocker && HasNetherspiteBeamStackLimit(blocker, data.beam) &&
        GetNetherspiteBeamStacks(blocker, data) >= NETHERSPITE_NEXT_STEPS_ON_AT_STACKS)
    {
        beamState.nextPosition = GetPositionOnNetherspiteBeam(
            *netherspite, *beamState.portal,
            FindNetherspiteBeamPositionDistance(*netherspite, *beamState.portal, state, next));
        return;
    }

    float const side = GetNetherspiteInnerSide(*netherspite, state, data.beam);
    bool const isHunter = next->getClass() == CLASS_HUNTER;
    float const low =
        isHunter ? NETHERSPITE_HUNTER_BLOCKER_DISTANCE : NETHERSPITE_BLOCKER_MIN_DISTANCE;
    if (isNextNew || !IsNextPositionGood(
            netherspite, *beamState.portal, state, side, beamState.nextPosition, low, isHunter))
    {
        std::optional<Position> position;
        if (!isNextNew)
        {
            position = ShiftNextPositionAlongLine(
                netherspite, *beamState.portal, state, side, beamState.nextPosition, low,
                isHunter);
        }
        if (!position)
            position = FindNextPosition(netherspite, *beamState.portal, state, side, low);
        if (position)
            beamState.nextPosition = *position;
        else if (isNextNew)
            beamState.nextPosition = next->GetPosition();
    }
}

// A banish slot's position on its bearing, moved in toward Netherspite 1 yd at a time until it's
// short of any wall and 3 yd clear of void zones. The bearing never changes, so the necessary
// spacing to prevent Netherbreath from hitting more than one bot at a time is maintained.
Position NetherspiteManageTimersAndTrackersAction::GetBanishPosition(
    Unit& netherspite, KaraInstanceState const& state, float angle, float startRadius)
{
    constexpr float minRadius = 6.0f;
    constexpr float radiusStep = 1.0f;
    float const bossX = netherspite.GetPositionX();
    float const bossY = netherspite.GetPositionY();
    float const bossZ = netherspite.GetPositionZ();

    float radius = startRadius;
    float x = bossX;
    float y = bossY;
    float z = bossZ;
    while (radius >= minRadius)
    {
        x = bossX + std::cos(angle) * radius;
        y = bossY + std::sin(angle) * radius;
        z = bossZ;
        if (bot->GetMap()->CheckCollisionAndGetValidCoords(
                bot, bossX, bossY, bossZ, x, y, z, false) &&
            IsClearOfVoidZones(state, x, y, NETHERSPITE_VZ_CLEARANCE))
        {
            break;
        }

        radius = std::min(radius, netherspite.GetExactDist2d(x, y)) - radiusStep;
    }

    return Position(x, y, z);
}

// Gives every group member a slot on its own bearing, spread evenly around Netherspite.
void NetherspiteManageTimersAndTrackersAction::UpdateBanishSlots(
    Unit* netherspite, KaraInstanceState& state)
{
    std::vector<NetherspiteBanishSlot>& slots = state.netherspiteBanishSlots;
    if (slots.empty())
    {
        Group* group = bot->GetGroup();
        if (!group)
            return;

        // Hunter bots choose their slot first and pick the bearing with the most room before a wall
        // since they are the only class with a minimum distance to attack. Then, in group order,
        // everyone else takes the free bearing closest to them.
        std::vector<Player*> tiers[3];
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member)
                continue;

            if (member->GetMapId() != KARA_MAP_ID)
                tiers[2].push_back(member);
            else if (member->getClass() == CLASS_HUNTER && GET_PLAYERBOT_AI(member))
                tiers[0].push_back(member);
            else
                tiers[1].push_back(member);
        }

        std::vector<Player*> members;
        for (std::vector<Player*> const& tier : tiers)
            members.insert(members.end(), tier.begin(), tier.end());

        if (members.empty())
            return;

        std::vector<float> angles;
        for (size_t i = 0; i < members.size(); ++i)
            angles.push_back(i * 2.0f * M_PI / members.size());

        for (Player* member : members)
        {
            bool const isHunter = member->getClass() == CLASS_HUNTER;
            float const radius =
                isHunter ? NETHERSPITE_HUNTER_BANISH_SLOT_RADIUS : NETHERSPITE_BANISH_SLOT_RADIUS;
            bool const isInKara = member->GetMapId() == KARA_MAP_ID;
            bool const isHunterBot = isHunter && GET_PLAYERBOT_AI(member) && isInKara;

            auto best = angles.begin();
            float bestScore = std::numeric_limits<float>::lowest();
            for (auto angle = angles.begin(); angle != angles.end() && isInKara; ++angle)
            {
                float score;
                if (isHunterBot)
                {
                    Position const position =
                        GetBanishPosition(*netherspite, state, *angle, radius);
                    score = netherspite->GetExactDist2d(position);
                }
                else
                {
                    float const gap = Position::NormalizeOrientation(
                        netherspite->GetAngle(member->GetPositionX(), member->GetPositionY()) -
                        *angle);
                    score = -std::min(gap, 2.0f * static_cast<float>(M_PI) - gap);
                }

                if (score > bestScore)
                {
                    bestScore = score;
                    best = angle;
                }
            }

            slots.push_back({
                member->GetGUID(), GetBanishPosition(*netherspite, state, *best, radius),
            });
            angles.erase(best);
        }

        return;
    }

    for (NetherspiteBanishSlot& slot : slots)
    {
        if (IsClearOfVoidZones(
                state, slot.position.GetPositionX(), slot.position.GetPositionY(),
                NETHERSPITE_VZ_CLEARANCE))
        {
            continue;
        }

        slot.position = GetBanishPosition(
            *netherspite, state,
            netherspite->GetAngle(slot.position.GetPositionX(), slot.position.GetPositionY()),
            netherspite->GetExactDist2d(slot.position));
    }
}

void NetherspiteManageTimersAndTrackersAction::UpdateVoidZones(KaraInstanceState& state)
{
    constexpr float searchRadius = 150.0f;
    std::list<Creature*> creatures;
    bot->GetCreatureListWithEntryInGrid(creatures, Id(KaraNpcs::NPC_VOID_ZONE), searchRadius);

    state.netherspiteVoidZones.clear();
    for (Creature* creature : creatures)
    {
        if (creature && creature->IsAlive())
            state.netherspiteVoidZones.push_back(creature->GetPosition());
    }
}

// Prince Malchezaar

// Move out of Shadow Nova range in a straight line that does not pass through Hellfire.
bool PrinceMalchezaarEnfeebledBotRunToSafetyAction::Execute(Event /*event*/)
{
    constexpr MovementPriority priority = MovementPriority::MOVEMENT_FORCED;
    if (IsWaitingForLastMove(priority))
        return false;

    Unit* malchezaar = GetKaraBoss(bot, KaraBosses::DATA_MALCHEZAAR);
    if (!malchezaar)
        return false;

    Aura const* enfeeble = bot->GetAura(Id(KaraSpells::SPELL_ENFEEBLE));
    if (!enfeeble)
        return false;

    // Once Shadow Nova's cast finishes, there is no longer any need to keep distance for it. The
    // enfeebled bot needs only to stay out of Hellfire.
    bool const canNovaHit = CanShadowNovaHitEnfeebled(malchezaar, *enfeeble);
    std::vector<Position> const infernals = GetLandedInfernals(botAI);
    if (_destination &&
        (!HasDestinationFor(enfeeble->GetApplyTime(), canNovaHit) ||
         !IsClearOfInfernals(*_destination, infernals, MALC_HELLFIRE_CLEARANCE) ||
         (canNovaHit && malchezaar->GetExactDist2d(*_destination) < MALC_SHADOW_NOVA_CLEARANCE) ||
         !IsLineClearOfInfernals(bot->GetPosition(), *_destination, infernals)))
    {
        _destination.reset();
    }

    if (!_destination)
    {
        // How far a bot can run (7 yd/s) in the 7.5s from Enfeeble to Shadow Nova landing.
        constexpr float searchDistance = 52.0f;
        _destination = FindNearestMalchezaarSafePosition(
            bot, bot->GetPosition(), infernals, searchDistance, false, MALC_HELLFIRE_CLEARANCE,
            canNovaHit ? malchezaar : nullptr);

        if (!_destination)
            return false;

        _enfeebleApplyTime = enfeeble->GetApplyTime();
        _forShadowNova = canNovaHit;

        // An in-progress move cut short by Enfeeble would otherwise resume after Enfeeble, even
        // though it would no longer make sense to execute.
        Action* hazardAction = context->GetAction("prince malchezaar move clear of hazards");
        if (hazardAction)
        {
            static_cast<PrinceMalchezaarMoveClearOfHazardsAction*>(hazardAction)
                ->ResetDestination();
        }
    }

    float moveX;
    float moveY;
    bool backwards;
    if (!GetStepToPosition(
            bot, *_destination, MALCHEZAAR_ARRIVAL_DISTANCE, nullptr, moveX, moveY, backwards))
    {
        _destination.reset();
        return false;
    }

    bot->CastStop();
    return MoveTo(
        KARA_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false, priority, true,
        backwards);
}

// Moves a bot out of Hellfire, and a ranged bot out of Shadow Nova, to a position from which it can
// reach its target. This prevents the bot from running back into the hazard with standard reach.
bool PrinceMalchezaarMoveClearOfHazardsAction::Execute(Event /*event*/)
{
    constexpr MovementPriority priority = MovementPriority::MOVEMENT_FORCED;
    if (IsWaitingForLastMove(priority))
        return false;

    Unit* malchezaar = GetKaraBoss(bot, KaraBosses::DATA_MALCHEZAAR);
    std::vector<Position> const infernals = GetLandedInfernals(botAI);
    if (_destination && !IsClearOfMalchezaarHazards(bot, malchezaar, infernals, *_destination))
        _destination.reset();

    if (!_destination)
    {
        _destination = FindMalchezaarClearPosition(botAI, malchezaar, infernals);
        if (!_destination)
            return false;
    }

    float moveX;
    float moveY;
    bool backwards;
    if (!GetStepToPosition(
            bot, *_destination, MALCHEZAAR_ARRIVAL_DISTANCE, nullptr, moveX, moveY, backwards))
    {
        _destination.reset();
        return false;
    }

    bot->CastStop();
    return MoveTo(
        KARA_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false, priority, true,
        backwards);
}

bool PrinceMalchezaarTankPositionBossAction::Execute(Event /*event*/)
{
    Unit* malchezaar = GetKaraBoss(bot, KaraBosses::DATA_MALCHEZAAR);
    if (!malchezaar)
        return false;

    if (AI_VALUE(Unit*, "current target") != malchezaar)
        return Attack(malchezaar);

    if (malchezaar->GetVictim() != bot || !bot->IsWithinMeleeRange(malchezaar))
        return false;

    std::vector<Position> const infernals = GetLandedInfernals(botAI);
    if (IsClearOfInfernals(*malchezaar, infernals, MALCHEZAAR_MELEE_CLEARANCE))
    {
        ResetWalk();
        return false;
    }

    constexpr MovementPriority priority = MovementPriority::MOVEMENT_FORCED;
    if (IsWaitingForLastMove(priority))
        return false;

    if (_destination && !IsClearOfInfernals(*_destination, infernals, _destinationClearance))
        _destination.reset();

    if (!_destination)
    {
        if (_searchedInfernalCount == infernals.size())
            return false;

        _searchedInfernalCount = infernals.size();
        _destination = FindMalchezaarTankPosition(bot, infernals, _destinationClearance);
        if (!_destination)
            return false;
    }

    float moveX;
    float moveY;
    bool backwards;
    if (!GetStepToPosition(
            bot, *_destination, MALCHEZAAR_ARRIVAL_DISTANCE, malchezaar, moveX, moveY, backwards))
    {
        _destination.reset();
        return false;
    }

    return MoveTo(
        KARA_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false, priority, true,
        backwards);
}

// Nightbane

// The tank backs up to the parapet until it stands between Nightbane and the parapet. This mirrors
// the retail strategy of keeping Nightbane perpendicular to the walkway, which largely keeps his
// breach, cleave, and tail sweep off the playing field and allows melee to attack his flanks
// safely. The tank does not move out of Charred Earth so this positioning can be maintained.
bool NightbaneTankPositionBossAction::Execute(Event /*event*/)
{
    Unit* nightbane = GetKaraBoss(bot, KaraBosses::DATA_NIGHTBANE);
    if (!nightbane)
        return false;

    if (AI_VALUE(Unit*, "current target") != nightbane)
        return Attack(nightbane);

    if (nightbane->GetVictim() != bot || !bot->IsWithinMeleeRange(nightbane))
        return false;

    constexpr MovementPriority priority = MovementPriority::MOVEMENT_FORCED;
    if (IsWaitingForLastMove(priority))
        return false;

    float const angle =
        std::clamp(GetTerraceAngle(*nightbane), TERRACE_ARC_START, TERRACE_ARC_END);
    Position const destination(
        TERRACE_DOME_CENTER.GetPositionX() + TERRACE_ARC_RADIUS * std::cos(angle),
        TERRACE_DOME_CENTER.GetPositionY() + TERRACE_ARC_RADIUS * std::sin(angle),
        bot->GetPositionZ());

    constexpr float arrivalDist = 0.5f;
    float moveX;
    float moveY;
    bool backwards;
    if (!GetStepToPosition(bot, destination, arrivalDist, nightbane, moveX, moveY, backwards))
        return false;

    return MoveTo(
        KARA_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false, priority, true,
        backwards);
}

bool NightbaneRangedMoveFromCharredEarthAction::Execute(Event /*event*/)
{
    Unit* nightbane = GetKaraBoss(bot, KaraBosses::DATA_NIGHTBANE);
    if (!nightbane)
        return false;

    KaraInstanceState const* state = FindKaraState(nightbane->GetInstanceId());
    if (!state || !state->nightbaneRangedPosition)
        return false;

    constexpr float arrivalDist = 0.5f;
    float moveX;
    float moveY;
    bool backwards;
    if (!GetStepToPosition(
            bot, GetTerraceWalkPosition(*bot, *state->nightbaneRangedPosition), arrivalDist,
            nullptr, moveX, moveY, backwards))
    {
        return false;
    }

    bot->CastStop();
    return MoveTo(
        KARA_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_FORCED, true, backwards);
}

bool NightbaneMeleeMoveFromCharredEarthAction::Execute(Event /*event*/)
{
    Unit* nightbane = GetKaraBoss(bot, KaraBosses::DATA_NIGHTBANE);
    if (!nightbane)
        return false;

    KaraInstanceState const* state = FindKaraState(nightbane->GetInstanceId());
    if (!state || !state->nightbaneMeleePosition)
        return false;

    Position const position = GetNightbaneMeleePosition(*nightbane, *state->nightbaneMeleePosition);
    constexpr float arrivalDist = 0.5f;
    float moveX;
    float moveY;
    bool backwards;
    if (!GetStepToPosition(
            bot, GetTerraceWalkPosition(*bot, position), arrivalDist, nightbane, moveX, moveY,
            backwards))
    {
        return false;
    }

    bot->CastStop();
    return MoveTo(
        KARA_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_FORCED, true, backwards);
}

// During any DPS hold period, pets are set passive. This includes the entire flight phase because
// pets tend to go out-of-bounds off the terrace to chase Nightbane or skeletons.
bool NightbaneSetPetStanceAction::Execute(Event /*event*/)
{
    Unit* nightbane = GetKaraBoss(bot, KaraBosses::DATA_NIGHTBANE);
    return nightbane && SetMinionStancesForHold(
        bot, IsNightbaneDpsHeld(bot, nightbane), _previousReactStates);
}

bool NightbaneSetPetStanceAction::ResetPetStance()
{
    bool const restored = SetMinionStancesForHold(bot, false, _previousReactStates);
    _previousReactStates.clear();
    return restored;
}

bool NightbaneStopAttackingAction::Execute(Event /*event*/)
{
    Unit* nightbane = GetKaraBoss(bot, KaraBosses::DATA_NIGHTBANE);
    if (!nightbane || IsNightbaneFlying(nightbane))
        return false;

    return StopAttackingTarget(bot, nightbane);
}

bool NightbaneSelectTargetForHoldAction::isUseful()
{
    return botAI->GetState() == BOT_STATE_NON_COMBAT;
}

bool NightbaneSelectTargetForHoldAction::Execute(Event /*event*/)
{
    Unit* nightbane = GetKaraBoss(bot, KaraBosses::DATA_NIGHTBANE);
    if (!nightbane || IsNightbaneFlying(nightbane))
        return false;

    return SelectTarget(
        botAI, AI_VALUE(Unit*, PlayerbotAI::IsTank(bot) ? "tank target" : "dps target"));
}

// Everybody stacks on one position, clear of Charred Earth. The stack moves away when Rain of Bones
// lands on it. All reach target actions are held so skeletons come to the stack.
bool NightbaneStackAwayFromRainOfBonesAction::Execute(Event /*event*/)
{
    Unit* nightbane = GetKaraBoss(bot, KaraBosses::DATA_NIGHTBANE);
    if (!nightbane)
        return false;

    KaraInstanceState const* state = FindKaraState(nightbane->GetInstanceId());
    if (!state || !state->nightbaneFlightStackPosition)
        return false;

    Position const walkPosition =
        GetTerraceWalkPosition(*bot, *state->nightbaneFlightStackPosition);
    constexpr float arrivalDist = 0.5f;
    float moveX;
    float moveY;
    bool backwards;
    if (!GetStepToPosition(bot, walkPosition, arrivalDist, nullptr, moveX, moveY, backwards))
        return false;

    bot->CastStop();
    return MoveTo(
        KARA_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false,
        false, false, MovementPriority::MOVEMENT_FORCED, true, backwards);
}

// Nightbane does not attack or cast as he goes to land. Allow bots to drop into their non-combat
// engines so the player has flexibility to reposition the raid before he lands.
bool NightbaneLeaveCombatForLandingAction::Execute(Event /*event*/)
{
    StandDown(botAI);
    return true;
}

bool NightbaneManageTimersAndTrackersAction::Execute(Event /*event*/)
{
    Unit* nightbane = GetKaraBoss(bot, KaraBosses::DATA_NIGHTBANE);
    if (!nightbane)
        return false;

    KaraInstanceState& state = KaraState(nightbane->GetInstanceId());

    if (!IsNightbaneFlying(nightbane))
    {
        bool updated = ResetIfSet(state.nightbaneFlightStage);
        updated |= ResetIfSet(state.nightbaneFlightStackPosition);
        updated |= ResetIfSet(state.nightbaneRainOfBonesCenter);
        updated |= EmplaceIfUnset(state.nightbaneDpsHoldTimer, getMSTime());
        std::vector<Position> const charredEarths = GetCharredEarths(botAI);
        updated |= UpdateMeleePosition(state, *nightbane, charredEarths);
        updated |= UpdateRangedPosition(state, *nightbane, charredEarths);
        return updated;
    }

    bool updated = ResetIfSet(state.nightbaneDpsHoldTimer);
    updated |= ResetIfSet(state.nightbaneMeleePosition);
    updated |= ResetIfSet(state.nightbaneRangedPosition);

    constexpr float atFlightPositionDist = 1.0f;
    NightbaneFlightStage stage = NightbaneFlightStage::TakeOff;
    if (nightbane->GetExactDist(NIGHTBANE_FLIGHT_POSITION) < atFlightPositionDist)
    {
        stage = NightbaneFlightStage::Hover;
    }
    else if (state.nightbaneFlightStage &&
        *state.nightbaneFlightStage != NightbaneFlightStage::TakeOff)
    {
        stage = NightbaneFlightStage::Landing;
    }

    if (state.nightbaneFlightStage != stage)
    {
        state.nightbaneFlightStage = stage;
        updated = true;
    }

    if (stage == NightbaneFlightStage::Landing)
    {
        updated |= ResetIfSet(state.nightbaneFlightStackPosition);
        updated |= ResetIfSet(state.nightbaneRainOfBonesCenter);
        return updated;
    }

    Position const* rainCenter = GetRainOfBonesCenter(nightbane);
    bool const isNewRain = rainCenter && (!state.nightbaneRainOfBonesCenter ||
        state.nightbaneRainOfBonesCenter->GetExactDist2d(*rainCenter) > 0.1f);
    if (state.nightbaneFlightStackPosition && !isNewRain)
        return updated;

    if (isNewRain)
        state.nightbaneRainOfBonesCenter = *rainCenter;

    if (state.nightbaneFlightStackPosition && state.nightbaneFlightStackPosition->GetExactDist2d(
            *rainCenter) >= RAIN_OF_BONES_CLEARANCE)
    {
        return true;
    }

    std::vector<Position> const charredEarths = GetCharredEarths(botAI);

    float const startAngle = state.nightbaneFlightStackPosition ?
        GetTerraceAngle(*state.nightbaneFlightStackPosition) : NIGHTBANE_FLIGHT_STACK_HOME_ANGLE;
    std::optional<Position> const position =
        FindNightbaneStackPosition(startAngle, charredEarths, rainCenter);

    if (position)
    {
        state.nightbaneFlightStackPosition = position;
    }
    else if (!state.nightbaneFlightStackPosition)
    {
        state.nightbaneFlightStackPosition = Position(
            TERRACE_DOME_CENTER.GetPositionX() +
                TERRACE_INNER_LANE_DIST * std::cos(NIGHTBANE_FLIGHT_STACK_HOME_ANGLE),
            TERRACE_DOME_CENTER.GetPositionY() +
                TERRACE_INNER_LANE_DIST * std::sin(NIGHTBANE_FLIGHT_STACK_HOME_ANGLE),
            NIGHTBANE_GROUND_Z);
    }

    return true;
}

// Melee start each ground phase on Nightbane's right flank. If Charred Earth appears under melee,
// they move to the other flank. If both flanks are covered, melee steps out of range and waits
// until it is safe to attack again.
bool NightbaneManageTimersAndTrackersAction::UpdateMeleePosition(
    KaraInstanceState& state, Unit& nightbane, std::vector<Position> const& charredEarths)
{
    NightbaneMeleePosition const current =
        state.nightbaneMeleePosition.value_or(NightbaneMeleePosition::ClockwiseFlank);
    Position const currentPosition = GetNightbaneMeleePosition(nightbane, current);
    if (state.nightbaneMeleePosition && current != NightbaneMeleePosition::Hold &&
        IsClearOfCharredEarth(currentPosition, charredEarths))
    {
        return false;
    }

    NightbaneMeleePosition next = NightbaneMeleePosition::Hold;
    float nextDist = std::numeric_limits<float>::max();
    for (NightbaneMeleePosition const flank :
         { NightbaneMeleePosition::ClockwiseFlank, NightbaneMeleePosition::CounterclockwiseFlank })
    {
        Position const position = GetNightbaneMeleePosition(nightbane, flank);
        float const dist = position.GetExactDist2d(currentPosition);
        if (dist < nextDist && IsClearOfCharredEarth(position, charredEarths))
        {
            next = flank;
            nextDist = dist;
        }
    }

    if (state.nightbaneMeleePosition == next)
        return false;

    state.nightbaneMeleePosition = next;
    return true;
}

// Ranged start each ground phase clockwise from Nightbane (his right side if he is properly
// positioned by the tank), up against the parapet. This generally leaves enough room for ranged to
// move laterally toward the dome to escape the first Charred Earth while keeping LoS. If there are
// no safe positions at range that are within LoS, ranged will step forward.
bool NightbaneManageTimersAndTrackersAction::UpdateRangedPosition(
    KaraInstanceState& state, Unit& nightbane, std::vector<Position> const& charredEarths)
{
    std::optional<Position>& position = state.nightbaneRangedPosition;
    bool const isClear = position && IsClearOfCharredEarth(*position, charredEarths);
    if (isClear && IsOnNightbaneRangedSide(*position, nightbane))
    {
        constexpr float tolerance = 0.5f;
        float const bossDist = nightbane.GetExactDist2d(*position);
        if (bossDist >= NIGHTBANE_RANGED_MIN_DIST - tolerance &&
            bossDist <= NIGHTBANE_RANGED_MAX_DIST + tolerance)
        {
            return false;
        }
    }

    Position const from = position ? *position : nightbane.GetPosition();
    std::optional<Position> next;
    if (!position)
    {
        next = FindNightbaneRangedPosition(
            *bot, nightbane, from, charredEarths, NightbaneRangedTier::Parapet);
    }

    if (!next)
    {
        next = FindNightbaneRangedPosition(
            *bot, nightbane, from, charredEarths, NightbaneRangedTier::Band);
    }

    if (!next && position && !isClear)
    {
        next = FindNightbaneRangedPosition(
            *bot, nightbane, from, charredEarths, NightbaneRangedTier::Forward);
    }

    if (!next)
        return false;

    position = next;
    return true;
}
