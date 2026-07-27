/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "SWPActions.h"
#include "SWPEncounter_Felmyst.h"
#include "Playerbots.h"
#include "RaidBossHelpers.h"
#include "Timer.h"
#include <array>
#include <cmath>

using namespace SwpHelpers;

bool FelmystMisdirectBossToMainTankAction::Execute(Event /*event*/)
{
    Unit* felmyst = AI_VALUE2(Unit*, "find target", "felmyst");
    if (!felmyst)
        return false;

    Player* mainTank = GetGroupMainTank(botAI, bot);
    if (!mainTank)
        return false;

    if (botAI->CanCastSpell("misdirection", mainTank))
        return botAI->CastSpell("misdirection", mainTank);

    if (bot->HasAura(static_cast<uint32>(SwpSpells::SPELL_MISDIRECTION)) &&
        botAI->CanCastSpell("steady shot", felmyst))
    {
        return botAI->CastSpell("steady shot", felmyst);
    }

    return false;
}

bool FelmystMainTankPositionBossOnGroundAction::Execute(Event /*event*/)
{
    ClearFelmystDemonicVaporKiteState(bot);

    Unit* felmyst = AI_VALUE2(Unit*, "find target", "felmyst");
    if (!felmyst)
        return false;

    if (AI_VALUE(Unit*, "current target") != felmyst)
        return Attack(felmyst);

    if (felmyst->GetVictim() != bot || bot->GetHealthPct() < 50.0f ||
        !bot->IsWithinMeleeRange(felmyst))
    {
        return false;
    }

    Position const position = GetFelmystMainTankGroundPosition(bot);
    float const distToPosition = bot->GetExactDist2d(
        position.GetPositionX(), position.GetPositionY());

    if (distToPosition < 2.0f)
        return false;

    float const dX = position.GetPositionX() - bot->GetPositionX();
    float const dY = position.GetPositionY() - bot->GetPositionY();
    float const moveDist = std::min(2.25f, distToPosition);
    float const moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
    float const moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

    return MoveTo(
        SWP_MAP_ID, moveX, moveY, position.GetPositionZ(), false, false,
        false, false, MovementPriority::MOVEMENT_COMBAT, true, true);
}

bool FelmystPositionRangedOnGroundAction::Execute(Event /*event*/)
{
    ClearFelmystDemonicVaporKiteState(bot);

    Unit* felmyst = AI_VALUE2(Unit*, "find target", "felmyst");
    if (!felmyst)
        return false;

    Position position;
    if (!TryGetFelmystRangedPosition(bot, felmyst, position))
        return false;

    return MoveInside(
        SWP_MAP_ID, position.GetPositionX(), position.GetPositionY(), position.GetPositionZ(),
        FELMYST_RANGED_GROUP_RADIUS, MovementPriority::MOVEMENT_COMBAT);
}

bool FelmystPositionMeleeOnGroundAction::Execute(Event /*event*/)
{
    ClearFelmystDemonicVaporKiteState(bot);

    Unit* felmyst = AI_VALUE2(Unit*, "find target", "felmyst");
    if (!felmyst)
        return false;

    Position position;
    if (!TryGetFelmystGroundStackPosition(
            bot, felmyst, FelmystGroundStack::Melee, position))
    {
        return false;
    }

    if (bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY()) < 0.25f)
        return false;

    return MoveTo(
        SWP_MAP_ID, position.GetPositionX(), position.GetPositionY(), position.GetPositionZ(),
        false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
}

bool FelmystRemoveEncapsulateAction::Execute(Event /*event*/)
{
    if (bot->getClass() == CLASS_MAGE)
        return botAI->CanCastSpell("ice block", bot) && botAI->CastSpell("ice block", bot);
    else
        return botAI->CanCastSpell("divine shield", bot) && botAI->CastSpell("divine shield", bot);
}

bool FelmystRunAwayFromEncapsulatedPlayerAction::Execute(Event /*event*/)
{
    Player* encapsulateTarget = GetFelmystEncapsulateTarget(bot);
    if (!encapsulateTarget || encapsulateTarget == bot)
        return false;

    Unit* felmyst = AI_VALUE2(Unit*, "find target", "felmyst");
    if (!felmyst)
        return false;

    FelmystGroundStack const botStack = GetClosestFelmystGroundStack(bot, felmyst, bot);
    FelmystGroundStack const targetStack = GetClosestFelmystGroundStack(
        bot, felmyst, encapsulateTarget);

    if (botStack == FelmystGroundStack::None || targetStack == FelmystGroundStack::None ||
        botStack != targetStack)
    {
        return false;
    }

    auto const tryMoveToStack = [&](FelmystGroundStack stack)
    {
        Position position;
        if (!TryGetFelmystGroundStackPosition(bot, felmyst, stack, position))
            return false;

        return MoveInside(
            SWP_MAP_ID, position.GetPositionX(), position.GetPositionY(), position.GetPositionZ(),
            FELMYST_RANGED_GROUP_RADIUS, MovementPriority::MOVEMENT_FORCED);
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

    if (bot->GetExactDist2d(leftPosition.GetPositionX(), leftPosition.GetPositionY()) <=
        bot->GetExactDist2d(rightPosition.GetPositionX(), rightPosition.GetPositionY()))
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
    return gasNovaTarget &&
        botAI->CanCastSpell("mass dispel", gasNovaTarget) &&
        botAI->CastSpell("mass dispel", gasNovaTarget);
}

bool FelmystAvoidDemonicVaporAction::Execute(Event /*event*/)
{
    constexpr float searchRadius = 20.0f;
    Unit* nearestTrail = bot->FindNearestCreature(
        static_cast<uint32>(SwpNpcs::NPC_DEMONIC_VAPOR_TRAIL), searchRadius, true);
    Unit* nearestVapor = bot->FindNearestCreature(
        static_cast<uint32>(SwpNpcs::NPC_DEMONIC_VAPOR), searchRadius, true);

    Unit* hazard = nearestTrail ? nearestTrail : nearestVapor;
    if (!hazard)
        return false;

    constexpr float safeDistFromVapor = 15.0f;
    float const currentDistance = bot->GetDistance2d(hazard);
    if (currentDistance > safeDistFromVapor)
        return false;

    botAI->InterruptSpell();
    return MoveAway(hazard, safeDistFromVapor - currentDistance);
}

bool FelmystKiteDemonicVaporAction::Execute(Event /*event*/)
{
    Position destination;
    if (!TryGetFelmystDemonicVaporKiteDestination(bot, destination))
        return false;

    float const distToDestination = bot->GetExactDist2d(
        destination.GetPositionX(), destination.GetPositionY());
    if (distToDestination < 0.5f)
        return false;

    float const dX = destination.GetPositionX() - bot->GetPositionX();
    float const dY = destination.GetPositionY() - bot->GetPositionY();
    float const moveDist = std::min(3.5f, distToDestination);
    float const moveX = bot->GetPositionX() + (dX / distToDestination) * moveDist;
    float const moveY = bot->GetPositionY() + (dY / distToDestination) * moveDist;

    return MoveTo(
        SWP_MAP_ID, moveX, moveY, destination.GetPositionZ(), false, false,
        false, false, MovementPriority::MOVEMENT_FORCED, true, false);
}

bool FelmystMoveToSafeFogLaneAction::Execute(Event /*event*/)
{
    Unit* felmyst = AI_VALUE2(Unit*, "find target", "felmyst");
    if (!felmyst)
    {
        _fogCrateStuckSampleMs = 0;
        return false;
    }

    FelmystFogOfCorruptionState fogState;
    bool const hasActiveFog =
        TryGetActiveFelmystFogOfCorruptionState(bot, felmyst, fogState);
    FelmystFogLane thirdPassLane = FelmystFogLane::None;
    bool const shouldRepositionAfterThirdPass = !hasActiveFog &&
        TryGetFelmystPostThirdPassWindow(felmyst, thirdPassLane);

    if (!hasActiveFog && !shouldRepositionAfterThirdPass)
    {
        _fogCrateStuckSampleMs = 0;
        return false;
    }

    std::array<Position, 3> destinations;
    uint8 destinationCount = 0;
    if (!TryGetFelmystFogSafeDestinations(
            bot, shouldRepositionAfterThirdPass ? thirdPassLane : fogState.lane,
            destinations, destinationCount))
    {
        _fogCrateStuckSampleMs = 0;
        return false;
    }

    LastMovement const& lastMove = AI_VALUE(LastMovement&, "last movement");
    bool trackedDestinationFound = false;
    for (uint8 index = 0; index < destinationCount; ++index)
    {
        Position const& destination = destinations[index];
        if (Position(
                lastMove.lastMoveToX, lastMove.lastMoveToY,
                lastMove.lastMoveToZ).GetExactDist(destination) >
            FELMYST_FOG_DESTINATION_MATCH_DISTANCE)
        {
            continue;
        }

        trackedDestinationFound = true;
        if (TryTeleportStuckBotOntoCrate(destination))
            return true;

        break;
    }

    if (!trackedDestinationFound)
        _fogCrateStuckSampleMs = 0;

    if (shouldRepositionAfterThirdPass)
    {
        uint8 bestIndex = 0;
        float bestDistance = std::numeric_limits<float>::max();
        for (uint8 index = 0; index < destinationCount; ++index)
        {
            Position const& destination = destinations[index];
            float const distanceToFelmyst = felmyst->GetExactDist2d(
                destination.GetPositionX(), destination.GetPositionY());

            if (distanceToFelmyst < bestDistance)
            {
                bestDistance = distanceToFelmyst;
                bestIndex = index;
            }
        }

        Position const& destination = destinations[bestIndex];
        return MoveTo(
            SWP_MAP_ID, destination.GetPositionX(), destination.GetPositionY(),
            destination.GetPositionZ(), false, false, false, false,
            MovementPriority::MOVEMENT_FORCED, true, false);
    }

    for (uint8 index = 0; index < destinationCount; ++index)
    {
        Position const& destination = destinations[index];
        if (MoveTo(
                SWP_MAP_ID, destination.GetPositionX(), destination.GetPositionY(),
                destination.GetPositionZ(), false, false, false, false,
                MovementPriority::MOVEMENT_FORCED, true, false))
        {
            return true;
        }
    }

    return false;
}

bool FelmystMoveToSafeFogLaneAction::TryTeleportStuckBotOntoCrate(
    Position const& destination)
{
    constexpr float crateCollisionCheckDistance = 2.0f;
    Position const stuckCratePosition = { 1484.443f, 591.337f, 23.391f };

    if (bot->GetExactDist2d(
            stuckCratePosition.GetPositionX(),
            stuckCratePosition.GetPositionY()) > crateCollisionCheckDistance)
    {
        _fogCrateStuckSampleMs = 0;
        return false;
    }

    uint32 const now = getMSTime();
    float const distanceToDestination = bot->GetExactDist(
        destination.GetPositionX(), destination.GetPositionY(), destination.GetPositionZ());

    if (!_fogCrateStuckSampleMs || _fogCrateStuckDestination.GetExactDist(destination) >
        FELMYST_FOG_DESTINATION_MATCH_DISTANCE)
    {
        _fogCrateStuckDestination = destination;
        _fogCrateStuckNearestDistance = distanceToDestination;
        _fogCrateStuckSampleMs = now;
        return false;
    }

    constexpr float progressResetDistance = 1.0f;

    if (distanceToDestination + progressResetDistance < _fogCrateStuckNearestDistance)
    {
        _fogCrateStuckNearestDistance = distanceToDestination;
        _fogCrateStuckSampleMs = now;
        return false;
    }

    constexpr uint32 stuckTimeoutMs = 1500;

    if (getMSTimeDiff(_fogCrateStuckSampleMs, now) < stuckTimeoutMs)
        return false;

    Position const onCratePosition = { 1482.181f, 591.253f, 24.545f };

    _fogCrateStuckSampleMs = 0;
    botAI->InterruptSpell();
    return bot->TeleportTo(
        SWP_MAP_ID, onCratePosition.GetPositionX(),onCratePosition.GetPositionY(),
        onCratePosition.GetPositionZ(), bot->GetOrientation());
}

bool FelmystMeleeClearTargetAction::Execute(Event /*event*/)
{
    botAI->InterruptSpell();
    bot->AttackStop();
    context->GetValue<Unit*>("current target")->Set(nullptr);
    bot->SetSelection(ObjectGuid());
    return true;
}

bool FelmystKillCharmedPlayerAction::Execute(Event /*event*/)
{
    Unit* felmyst = AI_VALUE2(Unit*, "find target", "felmyst");
    if (!felmyst)
        return false;

    Player* charmedPlayer = GetFelmystCharmedTarget(bot, felmyst);
    if (!charmedPlayer || AI_VALUE(Unit*, "current target") == charmedPlayer)
        return false;

    return Attack(charmedPlayer);
}

bool FelmystManageLandingDpsTimerAction::Execute(Event /*event*/)
{
    Unit* felmyst = AI_VALUE2(Unit*, "find target", "felmyst");
    if (!felmyst)
        return false;

    uint32 const instanceId = felmyst->GetMap()->GetInstanceId();
    auto& state = felmystEncounterStates[instanceId];

    Position landingDestination;
    if (felmyst->IsFlying() && TryGetFelmystLandingDestination(felmyst, landingDestination))
    {
        if (state.landingDpsWaitTimer)
            return false;

        state.landingDpsWaitTimer = std::time(nullptr);
        state.landingTouchdownTimer = 0;
        return true;
    }

    if (felmyst->IsFlying())
    {
        state.landingDpsWaitTimer = 0;
        state.landingTouchdownTimer = 0;
        return true;
    }

    // Grounded
    if (!state.landingDpsWaitTimer)
        return false;

    if (!state.landingTouchdownTimer)
    {
        state.landingTouchdownTimer = std::time(nullptr);
        return true;
    }

    time_t const now = std::time(nullptr);
    constexpr uint8 groundedDpsWaitSeconds = 3;
    if ((now - state.landingTouchdownTimer) < groundedDpsWaitSeconds)
        return false;

    state.landingDpsWaitTimer = 0;
    state.landingTouchdownTimer = 0;
    return true;
}
