/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "DeadStrategy.h"

#include "CreateNextAction.h"
#include "ReleaseSpiritAction.h"
#include "ReviveFromCorpseAction.h"
#include "AcceptResurrectAction.h"

void DeadStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    PassTroughStrategy::InitTriggers(triggers);

    triggers.push_back(
        new TriggerNode(
            "often",
            {
                CreateNextAction<AutoReleaseSpiritAction>(relevance)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "bg active",
            {
                CreateNextAction<AutoReleaseSpiritAction>(relevance)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "dead",
            {
                CreateNextAction<FindCorpseAction>(relevance)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "corpse near",
            {
                CreateNextAction<ReviveFromCorpseAction>(relevance - 1.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "resurrect request",
            {
                CreateNextAction<AcceptResurrectAction>(relevance)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "falling far",
            {
                CreateNextAction<RepopAction>(relevance + 1.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "location stuck",
            {
                CreateNextAction<RepopAction>(relevance + 1.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "can self resurrect",
            {
                CreateNextAction<SelfResurrectAction>(relevance + 2.0f)
            }
        )
    );
}

DeadStrategy::DeadStrategy(PlayerbotAI* botAI) : PassTroughStrategy(botAI) {}
