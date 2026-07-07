/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include <unordered_map>
#include <ctime>

#include "MagMultipliers.h"
#include "MagActions.h"
#include "MagHelpers.h"
#include "DruidBearActions.h"
#include "DKActions.h"
#include "ChooseTargetActions.h"
#include "FollowActions.h"
#include "GenericSpellActions.h"
#include "HunterActions.h"
#include "MageActions.h"
#include "MovementActions.h"
#include "PaladinActions.h"
#include "Playerbots.h"
#include "ReachTargetActions.h"
#include "WarriorActions.h"
#include "WipeAction.h"

using namespace MagtheridonHelpers;

// When a cube clicker is in the handling phase (waiting near cube or moving
// to use), suppress movement actions that would pull them away from the cube
float MagtheridonUseManticronCubeMultiplier::GetValue(Action* action)
{
    if (!AI_VALUE2(Unit*, "find target", "magtheridon"))
        return 1.0f;

    if (!IsCubeClicker(bot))
        return 1.0f;

    auto timerIt = blastNovaTimer.find(bot->GetMap()->GetInstanceId());
    if (timerIt == blastNovaTimer.end() ||
        time(nullptr) - timerIt->second < BLAST_NOVA_INTERIM_SECONDS)
    {
        return 1.0f;
    }

    if (dynamic_cast<FleeAction*>(action) ||
        dynamic_cast<FollowAction*>(action) ||
        dynamic_cast<ReachTargetAction*>(action) ||
        dynamic_cast<CastBlinkBackAction*>(action) ||
        dynamic_cast<CastReachTargetSpellAction*>(action) ||
        dynamic_cast<CastDisengageAction*>(action))
    {
        return 0.0f;
    }

    return 1.0f;
}

// Wait for 6 seconds after Magtheridon becomes attackable before engaging
float MagtheridonWaitToAttackMultiplier::GetValue(Action* action)
{
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    if (!magtheridon || !IsMagtheridonActive(magtheridon))
        return 1.0f;

    if (botAI->IsHeal(bot) || botAI->IsMainTank(bot))
        return 1.0f;

    constexpr uint8 dpsWaitSeconds = 6;
    auto it = dpsWaitTimer.find(magtheridon->GetMap()->GetInstanceId());
    if (it != dpsWaitTimer.end() && time(nullptr) - it->second > dpsWaitSeconds)
        return 1.0f;

    if (dynamic_cast<AttackAction*>(action) ||
        dynamic_cast<CastSpellAction*>(action))
    {
        return 0.0f;
    }

    return 1.0f;
}

float MagtheridonControlTankActionsMultiplier::GetValue(Action* action)
{
    if (!botAI->IsTank(bot) || bot->GetVictim() == nullptr)
        return 1.0f;

    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    if (!magtheridon)
        return 1.0f;

    if (dynamic_cast<CombatFormationMoveAction*>(action) ||
        dynamic_cast<TankAssistAction*>(action))
    {
        return 0.0f;
    }

    if (!botAI->IsMainTank(bot) && magtheridon->GetVictim() != bot)
        return 1.0f;

    if (dynamic_cast<AvoidAoeAction*>(action))
        return 0.0f;

    if (IsMagtheridonActive(magtheridon) || GetChanneler(bot, SOUTH_CHANNELER) ||
        GetChanneler(bot, WEST_CHANNELER) || GetChanneler(bot, EAST_CHANNELER))
    {
        return 1.0f;
    }

    if (dynamic_cast<CastReachTargetSpellAction*>(action) ||
        dynamic_cast<CastTauntAction*>(action) ||
        dynamic_cast<CastGrowlAction*>(action) ||
        dynamic_cast<CastHandOfReckoningAction*>(action) ||
        dynamic_cast<CastDarkCommandAction*>(action))
    {
        return 0.0f;
    }

    return 1.0f;
}

float MagtheridonDebrisDangerMultiplier::GetValue(Action* action)
{
    if (!AI_VALUE2(Unit*, "find target", "magtheridon") ||
        dynamic_cast<WipeAction*>(action) ||
        dynamic_cast<MagtheridonMoveOutOfDebrisAction*>(action))
    {
        return 1.0f;
    }

    // 15y is wider than the default hazard zone but is needed to
    // keep the multiplier in effect
    if (IsPositionInActiveDebris(
            bot->GetMap()->GetInstanceId(), bot->GetPositionX(),
            bot->GetPositionY(), 15.0f))
    {
        return 0.0f;
    }

    return 1.0f;
}
