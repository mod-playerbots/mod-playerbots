/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "LfgStrategy.h"

#include "CreateNextAction.h"
#include "LfgActions.h"
#include "PassLeadershipToMasterAction.h"

void LfgStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "random",
            {
                CreateNextAction<LfgJoinAction>(relevance)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "seldom",
            {
                CreateNextAction<LfgLeaveAction>(relevance)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "unknown dungeon",
            {
                CreateNextAction<GiveLeaderInDungeonAction>(relevance)
            }
        )
    );
}

LfgStrategy::LfgStrategy(PlayerbotAI* botAI) : PassTroughStrategy(botAI) {}
