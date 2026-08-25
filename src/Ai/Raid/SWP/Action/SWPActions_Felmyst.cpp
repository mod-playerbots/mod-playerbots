/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "SWPActions.h"
#include "EncounterHelpers.h"
#include "Playerbots.h"
#include "PlayerbotTextMgr.h"
#include "SWPEncounter_Felmyst.h"
#include "SWPSharedConstants.h"
#include "Timer.h"
#include <cmath>
#include <string>

using namespace SwpHelpers;
using namespace EncounterHelpers;

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
    float const distToPosition = bot->GetExactDist2d(position);
    if (distToPosition <= 2.0f)
        return false;

    float const posX = position.GetPositionX();
    float const posY = position.GetPositionY();
    float const botX = bot->GetPositionX();
    float const botY = bot->GetPositionY();

    float const toPosX = posX - botX;
    float const toPosY = posY - botY;
    float const toBossX = felmyst->GetPositionX() - botX;
    float const toBossY = felmyst->GetPositionY() - botY;
    bool const backwards = (toPosX * toBossX + toPosY * toBossY) < 0.0f;

    float const maxMoveDist = backwards ? 2.25f : 3.5f;
    float const moveDist = std::min(maxMoveDist, distToPosition);
    float const moveX = botX + (toPosX / distToPosition) * moveDist;
    float const moveY = botY + (toPosY / distToPosition) * moveDist;

    return MoveTo(
        SWP_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false,
        false, false, MovementPriority::MOVEMENT_COMBAT, true, true);
}

bool FelmystRangedStackInThreeGroupsAction::Execute(Event /*event*/)
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

bool FelmystMeleeStackBehindBossAction::Execute(Event /*event*/)
{
    ClearFelmystDemonicVaporKiteState(bot);

    Unit* felmyst = AI_VALUE2(Unit*, "find target", "felmyst");
    if (!felmyst)
        return false;

    Position position;
    if (!TryGetFelmystGroundStackPosition(bot, felmyst, FelmystGroundStack::Melee, position))
        return false;

    if (bot->GetExactDist2d(position) <= 0.25f)
        return false;

    return MoveTo(
        SWP_MAP_ID, position.GetPositionX(), position.GetPositionY(), position.GetPositionZ(),
        false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
}

bool FelmystRemoveEncapsulateAction::Execute(Event /*event*/)
{
    if (bot->getClass() == CLASS_MAGE)
    {
        return botAI->CanCastSpell(Id(SwpSpells::SPELL_ICE_BLOCK), bot) &&
            botAI->CastSpell(Id(SwpSpells::SPELL_ICE_BLOCK), bot);
    }

    return botAI->CanCastSpell(Id(SwpSpells::SPELL_DIVINE_SHIELD), bot) &&
        botAI->CastSpell(Id(SwpSpells::SPELL_DIVINE_SHIELD), bot);
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
    return gasNovaTarget &&
        botAI->CanCastSpell(Id(SwpSpells::SPELL_MASS_DISPEL), gasNovaTarget) &&
        botAI->CastSpell(Id(SwpSpells::SPELL_MASS_DISPEL), gasNovaTarget);
}

bool FelmystAvoidDemonicVaporAction::Execute(Event /*event*/)
{
    Player* leader = GetFelmystFlightLeader(bot);

    if (leader == bot && MarkTargetWithDiamond(bot, leader))
        return true;

    if (leader && leader->GetGUID() != _announcedFlightLeaderGuid)
    {
        _announcedFlightLeaderGuid = leader->GetGUID();
        AnnounceFlightLeader(leader);
    }

    if (!leader || leader == bot)
        return MoveAwayFromVapor();

    constexpr float farDistance = 20.0f;
    if (bot->GetDistance2d(leader) > farDistance && MoveAwayFromVapor(true))
        return true;

    return MoveToFlightLeader(leader);
}

bool FelmystAvoidDemonicVaporAction::MoveAwayFromVapor(bool unrestricted)
{
    std::vector<Creature*> const hazards = GetDemonicVaporHazards(bot);

    constexpr float hazardRadius = 13.5f;
    bool inDanger = false;
    for (Creature* hazard : hazards)
    {
        if (hazard && bot->GetDistance2d(hazard) < hazardRadius)
        {
            inDanger = true;
            break;
        }
    }

    if (!inDanger)
        return false;

    constexpr float maxSearchRadius = 40.0f;
    constexpr float distanceStep = 1.0f;

    std::vector<float> angles;
    if (unrestricted)
    {
        for (int i = 0; i < 8; ++i)
            angles.push_back(static_cast<float>(i) * (2.0f * static_cast<float>(M_PI) / 8.0f));
    }
    else
    {
        angles.push_back(0.0f);
        angles.push_back(static_cast<float>(M_PI));
    }

    Position bestPos;
    float minMoveDistance = std::numeric_limits<float>::max();
    bool foundSafe = false;

    float const botX = bot->GetPositionX();
    float const botY = bot->GetPositionY();
    float const botZ = bot->GetPositionZ();

    uint32 const stepCount = static_cast<uint32>(maxSearchRadius / distanceStep);
    for (uint32 step = 0; step <= stepCount; ++step)
    {
        float const distance = static_cast<float>(step) * distanceStep;
        for (float angle : angles)
        {
            float candidateX = botX + distance * std::cos(angle);
            float candidateY = botY + distance * std::sin(angle);

            bool isSafe = true;
            for (Creature* hazard : hazards)
            {
                if (hazard && hazard->GetDistance2d(candidateX, candidateY) < hazardRadius)
                {
                    isSafe = false;
                    break;
                }
            }

            if (!isSafe)
                continue;

            float candidateZ = botZ;
            bot->GetMap()->CheckCollisionAndGetValidCoords(
                bot, botX, botY, botZ, candidateX, candidateY, candidateZ, false);

            float const moveDistance = bot->GetExactDist2d(candidateX, candidateY);
            if (!foundSafe || moveDistance < minMoveDistance)
            {
                bestPos = Position(candidateX, candidateY, candidateZ);
                minMoveDistance = moveDistance;
                foundSafe = true;
            }
        }

        if (foundSafe)
            break;
    }

    if (!foundSafe)
        return false;

    bot->CastStop();
    return MoveTo(
        SWP_MAP_ID, bestPos.GetPositionX(), bestPos.GetPositionY(), bestPos.GetPositionZ(),
        false, false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
}

bool FelmystAvoidDemonicVaporAction::MoveToFlightLeader(Player* leader)
{
    constexpr float followDist = 2.0f;
    float const currentDistance = bot->GetDistance2d(leader);
    if (currentDistance <= followDist)
        return false;

    float const botX = bot->GetPositionX();
    float const botY = bot->GetPositionY();
    float const botZ = bot->GetPositionZ();

    float const leaderX = leader->GetPositionX();
    float const leaderY = leader->GetPositionY();
    float const leaderZ = leader->GetPositionZ();

    float const toPosX = leaderX - botX;
    float const toPosY = leaderY - botY;
    float const toPosZ = leaderZ - botZ;

    bot->CastStop();

    // 1) Try exact leader position
    if (MoveTo(
            SWP_MAP_ID, leaderX, leaderY, leaderZ, false, false, false, false,
            MovementPriority::MOVEMENT_COMBAT, true, false))
    {
        return true;
    }

    // 2) Try leader XY with bot's own Z
    if (MoveTo(
            SWP_MAP_ID, leaderX, leaderY, botZ, false, false, false, false,
            MovementPriority::MOVEMENT_COMBAT, true, false))
    {
        return true;
    }

    // 3) Try an incremental step toward the leader with linearly interpolated Z.
    float const moveDist = std::min(3.5f, currentDistance);
    float const moveX = botX + (toPosX / currentDistance) * moveDist;
    float const moveY = botY + (toPosY / currentDistance) * moveDist;
    float const moveZ = botZ + (toPosZ / currentDistance) * moveDist;

    return MoveTo(
        SWP_MAP_ID, moveX, moveY, moveZ, false, false, false, false,
        MovementPriority::MOVEMENT_COMBAT, true, false);
}

void FelmystAvoidDemonicVaporAction::AnnounceFlightLeader(Player* leader)
{
    std::string const text = PlayerbotTextMgr::instance().GetBotTextOrDefault(
        "felmyst_flight_leader",
        "[NAME] is now the flight phase leader. Everybody needs to stack on [NAME] "
        "during the flight phase.",
        std::map<std::string, std::string>{{"[NAME]", leader->GetName()}});

    botAI->SayToRaid(text);
}

bool FelmystKiteDemonicVaporAction::Execute(Event /*event*/)
{
    Position destination;
    if (!TryGetFelmystDemonicVaporKiteDestination(bot, destination))
        return false;

    float const distToDestination = bot->GetExactDist2d(destination);
    if (distToDestination <= 0.5f)
        return false;

    float const botX = bot->GetPositionX();
    float const botY = bot->GetPositionY();
    float const toPosX = destination.GetPositionX() - botX;
    float const toPosY = destination.GetPositionY() - botY;

    float const moveDist = std::min(3.5f, distToDestination);
    float const moveX = botX + (toPosX / distToDestination) * moveDist;
    float const moveY = botY + (toPosY / distToDestination) * moveDist;

    return MoveTo(
        SWP_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false,
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

    FogOfCorruptionState fogState;
    bool const hasActiveFog = TryGetActiveFogOfCorruptionState(bot, felmyst, fogState);

    FogLane thirdPassLane = FogLane::None;
    bool const shouldRepositionAfterThirdPass = !hasActiveFog &&
        TryGetFelmystPostThirdPassWindow(felmyst, thirdPassLane);

    if (!hasActiveFog && !shouldRepositionAfterThirdPass)
    {
        _fogCrateStuckSampleMs = 0;
        return false;
    }

    Position destination;
    Position const referencePoint(
        felmyst->GetPositionX(), felmyst->GetPositionY(), felmyst->GetPositionZ());
    if (!TryGetFelmystFogSafeDestination(
            bot, shouldRepositionAfterThirdPass ? thirdPassLane : fogState.lane,
            destination, shouldRepositionAfterThirdPass ? &referencePoint : nullptr))
    {
        _fogCrateStuckSampleMs = 0;
        return false;
    }

    LastMovement const& lastMove = AI_VALUE(LastMovement&, "last movement");
    if (Position(
            lastMove.lastMoveToX, lastMove.lastMoveToY,
            lastMove.lastMoveToZ).GetExactDist(destination) > FELMYST_LOCATION_MATCH_DISTANCE)
    {
        _fogCrateStuckSampleMs = 0;
    }
    else if (TryTeleportStuckBotOntoCrate(destination))
    {
        return true;
    }

    // Try CCing skeletons in place before first pass move
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

    bot->CastStop();
    return MoveTo(
        SWP_MAP_ID, destination.GetPositionX(), destination.GetPositionY(),
        destination.GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_FORCED, true, false);
}

bool FelmystMoveToSafeFogLaneAction::TryTeleportStuckBotOntoCrate(
    Position const& destination)
{
    constexpr float collisionCheckDist = 2.0f;
    if (bot->GetExactDist2d(FOG_CRATE_STUCK_POSITION) > collisionCheckDist)
    {
        _fogCrateStuckSampleMs = 0;
        return false;
    }

    uint32 const now = getMSTime();
    float const distToDestination = bot->GetExactDist(destination);

    if (!_fogCrateStuckSampleMs || _fogCrateStuckDestination.GetExactDist(destination) >
        FELMYST_LOCATION_MATCH_DISTANCE)
    {
        _fogCrateStuckDestination = destination;
        _fogCrateStuckNearestDist = distToDestination;
        _fogCrateStuckSampleMs = now;
        return false;
    }

    constexpr float progressResetDist = 1.0f;
    if (distToDestination + progressResetDist < _fogCrateStuckNearestDist)
    {
        _fogCrateStuckNearestDist = distToDestination;
        _fogCrateStuckSampleMs = now;
        return false;
    }

    constexpr uint32 stuckTimeoutMs = 1000;
    if (getMSTimeDiff(_fogCrateStuckSampleMs, now) < stuckTimeoutMs)
        return false;

    Position const& onCratePosition = FOG_CRATE_TELEPORT_POSITION;

    _fogCrateStuckSampleMs = 0;
    bot->CastStop();
    bot->NearTeleportTo(
        onCratePosition.GetPositionX(), onCratePosition.GetPositionY(),
        onCratePosition.GetPositionZ(), bot->GetOrientation());
    return true;
}

bool FelmystMeleeClearTargetAction::Execute(Event /*event*/)
{
    bot->AttackStop();
    bot->InterruptSpell(CURRENT_MELEE_SPELL);
    bot->CastStop();
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
    if (!charmedPlayer)
        return false;

    return AI_VALUE(Unit*, "current target") != charmedPlayer && Attack(charmedPlayer);
}

bool FelmystManageLandingDpsTimerAction::Execute(Event /*event*/)
{
    Unit* felmyst = AI_VALUE2(Unit*, "find target", "felmyst");
    if (!felmyst)
        return false;

    uint32 const instanceId = felmyst->GetInstanceId();
    auto& state = felmystEncounterStates[instanceId];

    if (felmyst->IsFlying() && IsFelmystLanding(felmyst))
    {
        if (state.landingDpsWaitStartMs)
            return false;

        state.landingDpsWaitStartMs = getMSTime();
        state.landingTouchdownMs = 0;
        return true;
    }

    if (felmyst->IsFlying())
    {
        state.landingDpsWaitStartMs = 0;
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

    constexpr uint32 groundedDpsWaitMs = 3000;
    if (GetMSTimeDiffToNow(state.landingTouchdownMs) < groundedDpsWaitMs)
        return false;

    state.landingDpsWaitStartMs = 0;
    state.landingTouchdownMs = 0;
    return true;
}
