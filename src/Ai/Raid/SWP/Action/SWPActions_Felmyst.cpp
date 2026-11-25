/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "SWPActions.h"
#include "EncounterHelpers.h"
#include "Playerbots.h"
#include "SWPEncounter_Felmyst.h"
#include "SWPShared.h"

using namespace SwpHelpers;
using namespace EncounterHelpers;

bool FelmystMainTankPositionBossOnGroundAction::Execute(Event /*event*/)
{
    Unit* felmyst = GetSwpBoss(bot, SwpBosses::DATA_FELMYST);
    if (!felmyst)
        return false;

    if (AI_VALUE(Unit*, "current target") != felmyst)
        return Attack(felmyst);

    if (felmyst->GetVictim() != bot || bot->GetHealthPct() < 50.0f ||
        !bot->IsWithinMeleeRange(felmyst))
    {
        return false;
    }

    constexpr float arrivalDist = 3.0f;
    constexpr MovementPriority priority = MovementPriority::MOVEMENT_COMBAT;
    if (IsWaitingForLastMove(priority))
        return false;

    float moveX;
    float moveY;
    bool backwards;
    if (!GetTankPathStep(
            bot, GetFelmystMainTankGroundPosition(bot), arrivalDist, felmyst, moveX, moveY,
            backwards))
    {
        return false;
    }

    return MoveTo(
        SWP_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false, priority,
        true, backwards);
}

bool FelmystRangedStackInThreeGroupsAction::Execute(Event /*event*/)
{
    Unit* felmyst = GetSwpBoss(bot, SwpBosses::DATA_FELMYST);
    if (!felmyst)
        return false;

    Position position;
    if (!TryGetFelmystRangedPosition(bot, felmyst, position))
        return false;

    constexpr MovementPriority priority = MovementPriority::MOVEMENT_COMBAT;
    if (IsWaitingForLastMove(priority))
        return false;

    float moveX;
    float moveY;
    if (!GetPathStepTowardPoint(
            bot, position, FELMYST_RANGED_GROUP_RADIUS, PATH_STEP_DISTANCE, moveX, moveY))
    {
        return false;
    }

    return MoveTo(
        SWP_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false, priority,
        true, false);
}

bool FelmystMeleeStackBehindBossAction::Execute(Event /*event*/)
{
    Unit* felmyst = GetSwpBoss(bot, SwpBosses::DATA_FELMYST);
    if (!felmyst)
        return false;

    Position position;
    if (!TryGetFelmystGroundStackPosition(bot, felmyst, FelmystGroundStack::Melee, position))
        return false;

    constexpr float arrivalDist = 0.25f;
    constexpr MovementPriority priority = MovementPriority::MOVEMENT_COMBAT;
    if (IsWaitingForLastMove(priority))
        return false;

    float moveX;
    float moveY;
    if (!GetPathStepTowardPoint(bot, position, arrivalDist, PATH_STEP_DISTANCE, moveX, moveY))
        return false;

    return MoveTo(
        SWP_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false, priority,
        true, false);
}

bool FelmystRemoveEncapsulateAction::Execute(Event /*event*/)
{
    uint32 const spellId = GetSelfImmunitySpell(bot);
    return spellId && botAI->CanCastSpell(spellId, bot) && botAI->CastSpell(spellId, bot);
}

bool FelmystRunAwayFromEncapsulatedPlayerAction::Execute(Event /*event*/)
{
    Player* encapsulateTarget = GetFelmystEncapsulateTarget(bot);
    if (!encapsulateTarget || encapsulateTarget == bot)
        return false;

    Unit* felmyst = GetSwpBoss(bot, SwpBosses::DATA_FELMYST);
    if (!felmyst)
        return false;

    FelmystGroundStack const targetStack = GetClosestFelmystGroundStack(
        bot, felmyst, encapsulateTarget);

    if (targetStack == FelmystGroundStack::None ||
        bot->GetExactDist2d(encapsulateTarget) >= ENCAPSULATE_CLEARANCE)
    {
        return false;
    }

    auto const tryMoveToStack = [&](FelmystGroundStack stack)
    {
        Position position;
        if (!TryGetFelmystGroundStackPosition(bot, felmyst, stack, position) ||
            encapsulateTarget->GetExactDist2d(position) < ENCAPSULATE_CLEARANCE)
        {
            return false;
        }

        constexpr MovementPriority priority = MovementPriority::MOVEMENT_FORCED;
        if (IsWaitingForLastMove(priority))
            return false;

        float moveX;
        float moveY;
        if (!GetPathStepTowardPoint(
                bot, position, FELMYST_RANGED_GROUP_RADIUS, PATH_STEP_DISTANCE, moveX, moveY))
        {
            return false;
        }

        return MoveTo(
            SWP_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false, priority,
            true, false);
    };

    if (targetStack == FelmystGroundStack::Left || targetStack == FelmystGroundStack::Right)
    {
        if (tryMoveToStack(FelmystGroundStack::Melee))
            return true;

        return tryMoveToStack(targetStack == FelmystGroundStack::Left ?
            FelmystGroundStack::Right : FelmystGroundStack::Left);
    }

    Position leftPosition;
    Position rightPosition;
    if (!TryGetFelmystGroundStackPosition(
            bot, felmyst, FelmystGroundStack::Left, leftPosition) ||
        !TryGetFelmystGroundStackPosition(
            bot, felmyst, FelmystGroundStack::Right, rightPosition))
    {
        return false;
    }

    if (bot->GetExactDist2d(leftPosition) <= bot->GetExactDist2d(rightPosition))
    {
        if (tryMoveToStack(FelmystGroundStack::Left))
            return true;

        return tryMoveToStack(FelmystGroundStack::Right);
    }

    if (tryMoveToStack(FelmystGroundStack::Right))
        return true;

    return tryMoveToStack(FelmystGroundStack::Left);
}

bool FelmystMassDispelGasNovaAction::Execute(Event /*event*/)
{
    Player* gasNovaTarget = GetFelmystGasNovaDispelTarget(bot);
    if (!gasNovaTarget || bot->GetExactDist(gasNovaTarget) > MASS_DISPEL_REACH)
        return false;

    return botAI->CanCastSpell(Id(SwpSpells::SPELL_MASS_DISPEL), gasNovaTarget) &&
        botAI->CastSpell(Id(SwpSpells::SPELL_MASS_DISPEL), gasNovaTarget);
}

bool FelmystMoveIntoMassDispelReachAction::Execute(Event /*event*/)
{
    Player* gasNovaTarget = GetFelmystGasNovaDispelTarget(bot);
    if (!gasNovaTarget || bot->GetExactDist(gasNovaTarget) <= MASS_DISPEL_REACH ||
        !IsFelmystMassDispelReady(bot))
    {
        return false;
    }

    constexpr float arrivalDist = MASS_DISPEL_REACH - 2.0f;
    constexpr MovementPriority priority = MovementPriority::MOVEMENT_COMBAT;
    if (IsWaitingForLastMove(priority))
        return false;

    float moveX;
    float moveY;
    if (!GetPathStepTowardUnit(bot, gasNovaTarget, arrivalDist, PATH_STEP_DISTANCE, moveX, moveY))
        return false;

    return MoveTo(
        SWP_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false, priority,
        true, false);
}

bool FelmystMoveToAirGroupPositionAction::Execute(Event /*event*/)
{
    Unit* felmyst = GetSwpBoss(bot, SwpBosses::DATA_FELMYST);
    Position position;
    if (!TryGetFelmystAirGroupPosition(felmyst, position) ||
        bot->GetExactDist2d(position) <= FELMYST_GROUP_ARRIVAL_DISTANCE)
    {
        return false;
    }

    constexpr MovementPriority priority = MovementPriority::MOVEMENT_COMBAT;
    if (IsWaitingForLastMove(priority))
        return false;

    // Hold rather than cross a patch: it is gone within 20 s, and the sidestep keeps the bot safe.
    float stepX;
    float stepY;
    if (TryGetFelmystAirStep(
            bot, position, GetDemonicVaporHazards(bot), DEMONIC_VAPOR_CLEARANCE, FogLane::None,
            stepX, stepY) != FelmystAirStep::Step)
    {
        return false;
    }

    bot->CastStop();
    return MoveTo(
        SWP_MAP_ID, stepX, stepY, bot->GetPositionZ(), false, false, false, false, priority,
        true, false);
}

bool FelmystAvoidDemonicVaporAction::Execute(Event /*event*/)
{
    if (!IsNearFelmystDemonicVaporHazard(bot))
        return false;

    constexpr MovementPriority priority = MovementPriority::MOVEMENT_FORCED;
    if (IsWaitingForLastMove(priority))
        return false;

    Position destination;
    if (!TryGetFelmystSidestepPosition(bot, GetDemonicVaporHazards(bot), destination))
        return false;

    float stepX;
    float stepY;
    bool backwards;
    if (!GetStepToPosition(bot, destination, 0.0f, nullptr, stepX, stepY, backwards))
        return false;

    bot->CastStop();
    return MoveTo(
        SWP_MAP_ID, stepX, stepY, bot->GetPositionZ(), false, false, false, false, priority,
        true, false);
}

bool FelmystKiteDemonicVaporAction::Execute(Event /*event*/)
{
    Creature* demonicVapor = GetFelmystOwnDemonicVapor(bot);
    if (!demonicVapor)
        return false;

    Unit* felmyst = GetSwpBoss(bot, SwpBosses::DATA_FELMYST);
    if (!felmyst)
        return false;

    if (demonicVapor->GetGUID() != _demonicVaporGuid)
        StartRoute(felmyst, demonicVapor->GetGUID());

    constexpr MovementPriority priority = MovementPriority::MOVEMENT_FORCED;
    if (IsWaitingForLastMove(priority))
        return false;

    if (_throughPosition &&
        bot->GetExactDist2d(*_throughPosition) <= FELMYST_GROUP_ARRIVAL_DISTANCE)
    {
        _throughPosition = nullptr;
    }

    while (!_throughPosition && _routePointIndex + 1 < _route->size() &&
        bot->GetExactDist2d((*_route)[_routePointIndex]) <= FELMYST_ROUTE_POINT_ARRIVAL_DISTANCE)
    {
        ++_routePointIndex;
    }

    Position const& target = _throughPosition ? *_throughPosition : (*_route)[_routePointIndex];

    // The route is not turned aside for patches, heads, or the crate; it passes clear of all three.
    float stepX;
    float stepY;
    bool backwards;
    if (!GetStepToPosition(
            bot, target, FELMYST_ROUTE_POINT_ARRIVAL_DISTANCE, nullptr, stepX, stepY, backwards))
    {
        return false;
    }

    bot->CastStop();
    return MoveTo(
        SWP_MAP_ID, stepX, stepY, bot->GetPositionZ(), false, false, false, false, priority,
        true, false);
}

// The route is chosen by the vapor count when the head appears. The first vapor's target, if the
// second vapor picks it again, goes back through G1 first.
void FelmystKiteDemonicVaporAction::StartRoute(Unit* felmyst, ObjectGuid const& demonicVaporGuid)
{
    FelmystEncounterState const* state = FindFelmystState(bot->GetInstanceId());
    FelmystAirEnd end = state ? state->airPhaseEnd : FelmystAirEnd::None;
    if (end == FelmystAirEnd::None)
        end = GetNearestFelmystAirEnd(felmyst);

    uint8 const demonicVaporCount = state ? state->demonicVaporCount : 0;

    _demonicVaporGuid = demonicVaporGuid;
    _route = &GetFelmystDemonicVaporRoute(end, demonicVaporCount);
    _routePointIndex = 0;
    _throughPosition = demonicVaporCount >= 2 &&
        state->firstDemonicVaporTargetGuid == bot->GetGUID() ?
        &GetFelmystGroupPosition(end, 1) : nullptr;
}

bool FelmystKiteDemonicVaporAction::ResetRouteProgress()
{
    if (_demonicVaporGuid.IsEmpty() && !_route && !_routePointIndex && !_throughPosition)
        return false;

    _demonicVaporGuid = ObjectGuid::Empty;
    _route = nullptr;
    _routePointIndex = 0;
    _throughPosition = nullptr;
    return true;
}

bool FelmystMoveToSafeFogLaneAction::Execute(Event /*event*/)
{
    Unit* felmyst = GetSwpBoss(bot, SwpBosses::DATA_FELMYST);
    if (!felmyst)
        return false;

    FogOfCorruptionState fogState;
    bool const hasActiveFog = TryGetActiveFogOfCorruptionState(bot, felmyst, fogState);

    FogLane thirdPassLane = FogLane::None;
    bool const shouldRepositionAfterThirdPass = !hasActiveFog &&
        TryGetFelmystPostThirdPassWindow(felmyst, thirdPassLane);

    if (!hasActiveFog && !shouldRepositionAfterThirdPass)
        return false;

    // After the third pass, steps are kept out of that lane's fog, which is still down
    Position destination;
    if (shouldRepositionAfterThirdPass)
    {
        if (!TryGetFelmystLandingApproachDestination(bot, thirdPassLane, felmyst, destination))
            return false;
    }
    else
    {
        destination = GetFelmystFogStopPosition(bot, fogState.lane);
    }

    // Try ccing skeletons in place before first pass move
    if (bot->getClass() == CLASS_MAGE &&
        botAI->CanCastSpell("frost nova", bot) && botAI->CastSpell("frost nova", bot))
    {
        return true;
    }

    if (bot->getClass() == CLASS_HUNTER &&
        botAI->CanCastSpell("frost trap", bot) && botAI->CastSpell("frost trap", bot))
    {
        return true;
    }

    constexpr MovementPriority priority = MovementPriority::MOVEMENT_FORCED;
    if (IsWaitingForLastMove(priority))
        return false;

    // Cross a patch when nothing else is allowed: a charmed bot costs the raid more.
    float stepX;
    float stepY;
    if (TryGetFelmystAirStep(
            bot, destination, GetDemonicVaporHazards(bot), FOG_MOVE_VAPOR_CLEARANCE,
            shouldRepositionAfterThirdPass ? thirdPassLane : FogLane::None, stepX,
            stepY) == FelmystAirStep::Blocked)
    {
        return false;
    }

    bot->CastStop();
    return MoveTo(
        SWP_MAP_ID, stepX, stepY, bot->GetPositionZ(), false, false, false, false, priority,
        true, false);
}

bool FelmystMeleeClearTargetAction::Execute(Event /*event*/)
{
    StandDown(botAI);
    CallBackPets(bot);
    return true;
}

bool FelmystKillCharmedPlayerAction::Execute(Event /*event*/)
{
    Unit* felmyst = GetSwpBoss(bot, SwpBosses::DATA_FELMYST);
    if (!felmyst)
        return false;

    Player* charmedPlayer = GetFelmystCharmedTarget(bot, felmyst);
    if (!charmedPlayer)
        return false;

    return AI_VALUE(Unit*, "current target") != charmedPlayer && Attack(charmedPlayer);
}

bool FelmystManageLandingDpsTimerAction::Execute(Event /*event*/)
{
    Unit* felmyst = GetSwpBoss(bot, SwpBosses::DATA_FELMYST);
    if (!felmyst)
        return false;

    uint32 const instanceId = felmyst->GetInstanceId();
    auto& state = FelmystState(instanceId);

    auto const clearTimers = [&state]
    {
        if (!state.landingDpsWaitStartMs && !state.landingTouchdownMs)
            return false;

        state.landingDpsWaitStartMs = 0;
        state.landingTouchdownMs = 0;
        return true;
    };

    if (felmyst->IsFlying())
    {
        // Felmyst is coming down for the pull on the engaging unit's position (95% health proxy).
        if (!IsFelmystLanding(felmyst) && felmyst->GetHealthPct() <= BOSS_ENGAGED_HEALTH_PCT)
            return clearTimers();

        if (state.landingDpsWaitStartMs)
            return false;

        state.landingDpsWaitStartMs = getMSTime();
        state.landingTouchdownMs = 0;
        return true;
    }

    // Grounded
    if (!state.landingDpsWaitStartMs)
        return false;

    if (!state.landingTouchdownMs)
    {
        state.landingTouchdownMs = getMSTime();
        return true;
    }

    if (GetMSTimeDiffToNow(state.landingTouchdownMs) < FELMYST_GROUNDED_DPS_WAIT_MS)
        return false;

    return clearTimers();
}

bool FelmystStopAttackingForLandingAction::Execute(Event /*event*/)
{
    return StopAttackingAndChanneling(bot);
}

bool FelmystSelectTargetForHoldAction::isUseful()
{
    return botAI->GetState() == BOT_STATE_NON_COMBAT;
}

bool FelmystSelectTargetForHoldAction::Execute(Event /*event*/)
{
    if (!PlayerbotAI::IsTank(bot))
        return SelectTarget(botAI, AI_VALUE(Unit*, "dps target"));

    return SelectTarget(botAI, AI_VALUE(Unit*, "tank target"));
}

// Owners stop attacking for the landing hold, but an imp or water elemental still casting at
// Felmyst from the air phase keeps her as its target, so every pet is held.
bool FelmystSetPetStanceAction::Execute(Event /*event*/)
{
    return SetMinionStancesForHold(bot, IsFelmystPetHoldActive(bot), _previousReactStates);
}

bool FelmystSetPetStanceAction::ResetPetStance()
{
    bool const restored = SetMinionStancesForHold(bot, false, _previousReactStates);
    _previousReactStates.clear();
    return restored;
}
