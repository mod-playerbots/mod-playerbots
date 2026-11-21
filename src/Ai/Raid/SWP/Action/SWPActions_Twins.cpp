/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "SWPActions.h"
#include "EncounterHelpers.h"
#include "Pet.h"
#include "Playerbots.h"
#include "SWPEncounter_Twins.h"
#include "SWPShared.h"
#include "ThreatHelpers.h"
#include <vector>

using namespace SwpHelpers;
using namespace EncounterHelpers;

// While Sacrolash lives, everyone lands on her side, a ranged bot she is chasing included so she
// comes back to her tanks; the Alythess tank lands on Alythess's side, as does everyone in phase 2.
// Each then moves on to its position.
bool EredarTwinsJumpFromBalconyAction::Execute(Event /*event*/)
{
    Position const& jumpPosition = EREDAR_TWINS_P1_RANGED_POSITION;
    Position const& landingPosition =
        FindSacrolash(bot) && !IsAlythessTank(bot) ?
        EREDAR_TWINS_SACROLASH_LANDING_POSITION : EREDAR_TWINS_ALYTHESS_LANDING_POSITION;

    constexpr float arrivalDistance = 2.0f;
    if (bot->GetExactDist2d(jumpPosition) > arrivalDistance)
    {
        float moveX;
        float moveY;
        bool backwards;
        if (!GetStepToPosition(
                bot, jumpPosition, arrivalDistance, nullptr, moveX, moveY, backwards))
        {
            return false;
        }

        return MoveTo(
            SWP_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
            MovementPriority::MOVEMENT_FORCED, true, false);
    }

    return JumpTo(
        SWP_MAP_ID, landingPosition.GetPositionX(), landingPosition.GetPositionY(),
        landingPosition.GetPositionZ(), MovementPriority::MOVEMENT_FORCED);
}

bool EredarTwinsAnnounceClassRolesAction::Execute(Event /*event*/)
{
    return IsMechanicTrackerBot(bot, SWP_MAP_ID) && UpdateEredarTwinsClassRoleAnnouncement(botAI);
}

// At the pull the first bot hunter gives Alythess to her tank and the rest give Sacrolash to her
// first tank. Then every hunter feeds Sacrolash's second tank, whom Confounding Blow sends her to
// and whose threat sets the DPS cap.
bool EredarTwinsMisdirectToTanksAction::Execute(Event /*event*/)
{
    Unit* sacrolash = FindSacrolash(bot);
    if (!sacrolash)
        return false;

    Unit* boss = sacrolash;
    Player* tank = nullptr;
    if (sacrolash->GetHealthPct() <= BOSS_ENGAGED_HEALTH_PCT)
    {
        tank = GetSacrolashSecondTank(bot, sacrolash);
    }
    else if (IsAlythessPullMisdirectHunter(bot))
    {
        boss = FindAlythess(bot);
        tank = GetAlythessTank(bot);
    }
    else
    {
        tank = GetSacrolashTank(bot, 0);
    }

    if (!boss || !tank || !tank->IsAlive())
        return false;

    return MisdirectTargetToTank(botAI, boss, tank);
}

bool EredarTwinsPositionSacrolashTanksAction::Execute(Event /*event*/)
{
    Unit* sacrolash = FindSacrolash(bot);
    if (!sacrolash)
        return false;

    if (AI_VALUE(Unit*, "current target") != sacrolash)
        return Attack(sacrolash);

    // At the pull the tanks wait on the drag line until one of them holds her in its melee range.
    // Then the tank she is hitting draws her along it, and the other tank simply joins it.
    Unit* victim = sacrolash->GetVictim();
    Player* victimPlayer = victim ? victim->ToPlayer() : nullptr;
    bool const isHeldByTank = victimPlayer && IsAnySacrolashTank(victimPlayer) &&
        victimPlayer->IsWithinMeleeRange(sacrolash);
    if (!isHeldByTank && !IsEredarTwinsStartHoldActive(botAI))
        return false;

    Position const& position =
        isHeldByTank ? SACROLASH_TANK_POSITION : SACROLASH_TANK_PULL_POSITION;

    // Backwards only for the tank she is hitting, when the step leads away from her.
    float moveX;
    float moveY;
    bool backwards;
    if (!GetStepToPosition(
            bot, position, SACROLASH_TANK_ARRIVAL_DISTANCE, sacrolash, moveX, moveY, backwards))
    {
        return false;
    }

    return MoveTo(
        SWP_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_COMBAT, true, backwards);
}

bool EredarTwinsAlythessTankMoveOutOfBlazeAction::Execute(Event /*event*/)
{
    Unit* alythess = FindAlythess(bot);
    if (!alythess)
        return false;

    if (AI_VALUE(Unit*, "current target") != alythess)
        return Attack(alythess);

    auto const findSafeAlythessTankIndex =
        [&](uint8 startIndex, bool includeStart, uint8& safeIndex)
    {
        size_t const offsetStart = includeStart ? 0 : 1;
        for (size_t offset = offsetStart; offset < ALYTHESS_TANK_POSITIONS.size(); ++offset)
        {
            uint8 const candidateIndex =
                static_cast<uint8>((startIndex + offset) % ALYTHESS_TANK_POSITIONS.size());

            if (IsAlythessTankPositionSafe(
                    botAI, GetAlythessTankPosition(alythess, candidateIndex)))
            {
                safeIndex = candidateIndex;
                return true;
            }
        }

        return false;
    };

    uint8 index = _alythessTankStep.value_or(ALYTHESS_TANK_FIRST_INDEX);

    if (!IsAlythessTankPositionSafe(botAI, GetAlythessTankPosition(alythess, index)))
    {
        uint8 safeIndex = index;
        if (!findSafeAlythessTankIndex(index, false, safeIndex))
            return false;

        index = safeIndex;
        _alythessTankStep = index;
    }

    // From its landing it goes to its first position whether or not it holds her yet, to melee
    // her for threat; after that it moves only while she is hitting it.
    if (alythess->GetVictim() != bot && _alythessTankStep)
        return false;

    Position const position = GetAlythessTankPosition(alythess, index);
    constexpr float arrivalDist = ALYTHESS_TANK_ARRIVAL_DISTANCE;
    if (bot->GetExactDist2d(position) <= arrivalDist)
        return false;

    return StepToPosition(alythess, position, arrivalDist);
}

bool EredarTwinsAlythessTankMoveOutOfBlazeAction::StepToPosition(
    Unit* alythess, Position const& position, float arrivalDist)
{
    float moveX;
    float moveY;
    bool backwards;
    if (!GetStepToPosition(bot, position, arrivalDist, alythess, moveX, moveY, backwards))
        return false;

    return MoveTo(
        SWP_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_COMBAT, true, backwards);
}

bool EredarTwinsRangedStackAtBalconyEdgeAction::Execute(Event /*event*/)
{
    Position const& position = EREDAR_TWINS_P1_RANGED_POSITION;

    if (bot->GetExactDist2d(position) <= EREDAR_TWINS_BALCONY_ARRIVAL_DISTANCE)
        return false;

    float moveX;
    float moveY;
    bool backwards;
    if (!GetStepToPosition(
            bot, position, EREDAR_TWINS_BALCONY_ARRIVAL_DISTANCE, nullptr, moveX, moveY,
            backwards))
    {
        return false;
    }

    return MoveTo(
        SWP_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_FORCED, true, false);
}

bool EredarTwinsStackBehindAlythessAction::Execute(Event /*event*/)
{
    Unit* alythess = FindAlythess(bot);
    if (!alythess)
        return false;

    // The route moves on to the next waypoint before the bot is this close to the last one.
    Position const position = GetEredarTwinsPhase2RouteTarget(botAI, alythess);
    constexpr float arrivalDist = EREDAR_TWINS_P2_STACK_ARRIVAL_DISTANCE;
    if (bot->GetExactDist2d(position) <= arrivalDist)
        return false;

    float moveX;
    float moveY;
    bool backwards;
    if (!GetStepToPosition(bot, position, arrivalDist, nullptr, moveX, moveY, backwards))
        return false;

    return MoveTo(
        SWP_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_FORCED, true, false);
}

bool EredarTwinsStepAsideFromStackAction::Execute(Event /*event*/)
{
    Unit* alythess = FindAlythess(bot);
    if (!alythess)
        return false;

    Position const position = GetEredarTwinsStepAsidePosition(
        alythess, bot->GetPositionZ() > EREDAR_TWINS_BALCONY_Z);
    constexpr float arrivalDist = EREDAR_TWINS_STEP_ASIDE_ARRIVAL_DISTANCE;
    if (bot->GetExactDist2d(position) <= arrivalDist)
        return false;

    float moveX;
    float moveY;
    bool backwards;
    if (!GetStepToPosition(bot, position, arrivalDist, nullptr, moveX, moveY, backwards))
        return false;

    return MoveTo(
        SWP_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_FORCED, true, false);
}

bool EredarTwinsDpsPrioritizeSacrolashAction::Execute(Event /*event*/)
{
    // Start the 8s clock for tanks to get aggro first.
    EmplaceIfUnset(EredarTwinsState(bot->GetInstanceId()).dpsHoldStartMs, getMSTime());
    // Select a target without attacking while the DPS hold is active.
    bool const isDpsHeld = IsEredarTwinsStartHoldActive(botAI);

    Unit* sacrolash = FindSacrolash(bot);
    Unit* twinTarget = sacrolash ? sacrolash : FindAlythess(bot);

    if (!twinTarget)
        return false;

    EredarTwinsThreatHolds const holds =
        AI_VALUE(EredarTwinsThreatHolds, "eredar twins threat holds");
    bool const shouldHoldThreat =
        (sacrolash ? holds.sacrolash : holds.alythess) == twinTarget->GetGUID();

    if (shouldHoldThreat)
    {
        return IsAttackingTarget(bot, twinTarget) ? StopAttackingTarget(bot, twinTarget) :
            SelectTarget(botAI, twinTarget);
    }

    if (AI_VALUE(Unit*, "current target") == twinTarget)
        return false;

    return isDpsHeld ? SelectTarget(botAI, twinTarget) : Attack(twinTarget);
}

bool EredarTwinsSetPetStanceAction::Execute(Event /*event*/)
{
    return SetMinionStancesForHold(bot, IsEredarTwinsStartHoldActive(botAI), _previousReactStates);
}

bool EredarTwinsSetPetStanceAction::ResetPetStance()
{
    bool const restored = SetMinionStancesForHold(bot, false, _previousReactStates);
    _previousReactStates.clear();
    return restored;
}

bool EredarTwinsConflagrationTargetMoveFromGroupAction::Execute(Event /*event*/)
{
    if (bot->getClass() == CLASS_ROGUE && botAI->CanCastSpell("vanish", bot) &&
        botAI->CastSpell("vanish", bot))
    {
        return true;
    }

    // By level, not role: a ranged bot on the floor can't reach the balcony in time.
    Position const& position = bot->GetPositionZ() > EREDAR_TWINS_BALCONY_Z ?
        EREDAR_TWINS_BALCONY_CONFLAG_POSITION : EREDAR_TWINS_FLOOR_CONFLAG_POSITION;

    constexpr float arrivalDist = 1.0f;
    if (bot->GetExactDist2d(position) <= arrivalDist)
        return false;

    float moveX;
    float moveY;
    bool backwards;
    if (!GetStepToPosition(bot, position, arrivalDist, nullptr, moveX, moveY, backwards))
        return false;

    return MoveTo(
        SWP_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_FORCED, true, false);
}

bool EredarTwinsMoveAwayFromSacrolashVictimAction::Execute(Event /*event*/)
{
    Unit* sacrolash = FindSacrolash(bot);
    if (!sacrolash)
        return false;

    Unit* victim = sacrolash->GetVictim();
    if (!victim)
        return false;

    if (bot->GetExactDist2d(victim) >= CONFLAGRATION_CLEARANCE)
        return false;

    bot->CastStop();
    return MoveFromGroup(CONFLAGRATION_CLEARANCE);
}
