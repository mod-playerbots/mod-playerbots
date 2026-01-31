/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "RpgStrategy.h"

#include "ChooseRpgTargetAction.h"
#include "CreateNextAction.h"
#include "MoveToRpgTargetAction.h"
#include "RpgSubActions.h"
#include "RpgAction.h"

// float RpgActionMultiplier::GetValue(Action& action)
// {
//     std::string const nextAction = AI_VALUE(std::string, "next rpg action");
//     std::string const name = action.getName();

//     if (!nextAction.empty() && dynamic_cast<RpgEnabled*>(action) && name != nextAction)
//         return 0.0f;

//     return 1.0f;
// }

RpgStrategy::RpgStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

std::vector<NextAction> RpgStrategy::getDefaultActions()
{
    return {
        CreateNextAction<RpgAction>(1.0f)
    };
}

void RpgStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "no rpg target",
            {
                CreateNextAction<ChooseRpgTargetAction>(5.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "often",
            {
                CreateNextAction<MoveRandomAction>(1.10f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "far from rpg target",
            {
                CreateNextAction<MoveToRpgTargetAction>(5.0f)
            }
        )
    );

    // Sub actions
    triggers.push_back(
        new TriggerNode(
            "rpg",
            {
                CreateNextAction<RpgStayAction>(1.101f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "rpg",
            {
                CreateNextAction<RpgWorkAction>(1.101f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "rpg",
            {
                CreateNextAction<RpgEmoteAction>(1.101f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "has rpg target",
            {
                CreateNextAction<RpgCancelAction>(1.101f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "rpg discover",
            {
                CreateNextAction<RpgDiscoverAction>(1.210f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "rpg start quest",
            {
                CreateNextAction<RpgStartQuestAction>(1.180f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "rpg end quest",
            {
                CreateNextAction<RpgEndQuestAction>(1.190f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "rpg buy",
            {
                CreateNextAction<RpgBuyAction>(1.130f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "rpg repair",
            {
                CreateNextAction<RpgRepairAction>(1.195f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "rpg heal",
            {
                CreateNextAction<RpgHealAction>(1.125f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "rpg home bind",
            {
                CreateNextAction<RpgHomeBindAction>(1.160f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "rpg buy petition",
            {
                CreateNextAction<RpgBuyPetitionAction>(1.140f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "rpg use",
            {
                CreateNextAction<RpgUseAction>(1.102f)
            }
        )
    );
}

void RpgStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new RpgActionMultiplier(botAI));
}
