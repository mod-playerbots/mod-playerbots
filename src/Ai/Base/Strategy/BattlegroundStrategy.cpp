/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "BattlegroundStrategy.h"

#include "BattleGroundJoinAction.h"
#include "CreateNextAction.h"
#include "BattlegroundTacticsMoveToStart.h"
#include "BattlegroundTacticsMoveToObjective.h"
#include "BattlegroundTacticsCheckObjective.h"
#include "BattlegroundTacticsResetObjectiveForce.h"
#include "BattlegroundTacticsCheckFlag.h"
#include "ChooseTargetActions.h"
#include "BattlegroundTacticsProtectFC.h"
#include "BattlegroundTacticsUseBuff.h"
#include "VehicleActions.h"
#include "GenericSpellActions.h"

void BGStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "often",
            {
                CreateNextAction<BGJoinAction>(relevance)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "bg invite active",
            {
                CreateNextAction<BGStatusCheckAction>(relevance)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "timer",
            {
                CreateNextAction<BGStrategyCheckAction>(relevance)
            }
        )
    );
}

BGStrategy::BGStrategy(PlayerbotAI* botAI) : PassTroughStrategy(botAI) {}

void BattlegroundStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "bg waiting",
            {
                CreateNextAction<BattlegroundTacticsMoveToStart>(ACTION_BG)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "bg active",
            {
                CreateNextAction<BattlegroundTacticsMoveToObjective>(ACTION_BG)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "often",
            {
                CreateNextAction<BattlegroundTacticsCheckObjective>(ACTION_BG + 1.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "dead",
            {
                CreateNextAction<BattlegroundTacticsResetObjectiveForce>(ACTION_EMERGENCY)
            }
        )
    );
}

void WarsongStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "bg active",
            {
                CreateNextAction<BattlegroundTacticsCheckFlag>(ACTION_EMERGENCY )
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "enemy flagcarrier near",
            {
                CreateNextAction<AttackEnemyFlagCarrierAction>(ACTION_RAID + 1.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "team flagcarrier near",
            {
                CreateNextAction<BattlegroundTacticsProtectFC>(ACTION_RAID)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "often",
            {
                CreateNextAction<BattlegroundTacticsUseBuff>(ACTION_BG)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "low health",
            {
                CreateNextAction<BattlegroundTacticsUseBuff>(ACTION_MOVE)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "low mana",
            {
                CreateNextAction<BattlegroundTacticsUseBuff>(ACTION_MOVE)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "player has flag",
            {
                CreateNextAction<BattlegroundTacticsMoveToObjective>(ACTION_EMERGENCY)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "timer bg",
            {
                CreateNextAction<BattlegroundTacticsResetObjectiveForce>(ACTION_EMERGENCY)
            }
        )
    );
}

void AlteracStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "alliance no snowfall gy",
            {
                CreateNextAction<BattlegroundTacticsMoveToObjective>(ACTION_EMERGENCY)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "timer bg",
            {
                CreateNextAction<BattlegroundTacticsResetObjectiveForce>(ACTION_EMERGENCY)
            }
        )
    );
}

void ArathiStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "bg active",
            {
                CreateNextAction<BattlegroundTacticsCheckFlag>(ACTION_EMERGENCY)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "often",
            {
                CreateNextAction<BattlegroundTacticsUseBuff>(ACTION_BG)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "low health",
            {
                CreateNextAction<BattlegroundTacticsUseBuff>(ACTION_MOVE)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "low mana",
            {
                CreateNextAction<BattlegroundTacticsUseBuff>(ACTION_MOVE)
            }
        )
    );
}

void EyeStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "bg active",
            {
                CreateNextAction<BattlegroundTacticsCheckFlag>(ACTION_EMERGENCY)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "often",
            {
                CreateNextAction<BattlegroundTacticsUseBuff>(ACTION_BG)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "low health",
            {
                CreateNextAction<BattlegroundTacticsUseBuff>(ACTION_MOVE)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "low mana",
            {
                CreateNextAction<BattlegroundTacticsUseBuff>(ACTION_MOVE)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "enemy flagcarrier near",
            {
                CreateNextAction<AttackEnemyFlagCarrierAction>(ACTION_RAID)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "player has flag",
            {
                CreateNextAction<BattlegroundTacticsMoveToObjective>(ACTION_EMERGENCY)
            }
        )
    );
}

//TODO: Do Priorities
void IsleStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "bg active",
            {
                CreateNextAction<BattlegroundTacticsCheckFlag>(ACTION_MOVE)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "timer",
            {
                CreateNextAction<EnterVehicleAction>(ACTION_MOVE + 8.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "random",
            {
                CreateNextAction<LeaveVehicleAction>(ACTION_MOVE + 7.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "in vehicle",
            {
                CreateNextAction<CastHurlBoulderAction>(ACTION_MOVE + 9.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "in vehicle",
            {
                CreateNextAction<CastFireCannonAction>(ACTION_MOVE + 9.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "in vehicle",
            {
                CreateNextAction<CastNapalmAction>(ACTION_MOVE + 9.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "enemy is close",
            {
                CreateNextAction<CastSteamBlastAction>(ACTION_MOVE + 9.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "in vehicle",
            {
                CreateNextAction<CastRamAction>(ACTION_MOVE + 9.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "enemy is close",
            {
                CreateNextAction<CastRamAction>(ACTION_MOVE + 9.1f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "enemy out of melee",
            {
                CreateNextAction<CastSteamRushAction>(ACTION_MOVE + 9.2f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "in vehicle",
            {
                CreateNextAction<CastIncendiaryRocketAction>(ACTION_MOVE + 9.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "in vehicle",
            {
                CreateNextAction<CastRocketBlastAction>(ACTION_MOVE + 9.0f)
            }
        )
    );
    // this is bugged: it doesn't work, and stops glaive throw working (which is needed to take down gate)
    // triggers.push_back(
    //     new TriggerNode(
    //     "in vehicle",
    //         {
    //             CreateNextAction("blade salvo", ACTION_MOVE + 9.0f)
    //         }
    //     )
    // );
    triggers.push_back(
        new TriggerNode(
            "in vehicle",
            {
                CreateNextAction<CastGlaiveThrowAction>(ACTION_MOVE + 9.0f)
            }
        )
    );
}

void ArenaStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("no possible targets",
            {
                CreateNextAction<ArenaTactics>(ACTION_BG)
            }
        )
    );
}
