/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "QuestStrategies.h"

#include "CreateNextAction.h"
#include "AcceptQuestAction.h"
#include "TalkToQuestGiverAction.h"

QuestStrategy::QuestStrategy(PlayerbotAI* botAI) : PassTroughStrategy(botAI) { supported.push_back("accept quest"); }

void QuestStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    PassTroughStrategy::InitTriggers(triggers);

    triggers.push_back(
        new TriggerNode(
            "quest share",
            {
                CreateNextAction<AcceptQuestShareAction>(relevance)
            }
        )
    );
}

void DefaultQuestStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    QuestStrategy::InitTriggers(triggers);

    triggers.push_back(
        new TriggerNode(
            "use game object",
            {
                CreateNextAction<TalkToQuestGiverAction>(relevance)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "gossip hello",
            {
                CreateNextAction<TalkToQuestGiverAction>(relevance)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "complete quest",
            {
                CreateNextAction<TalkToQuestGiverAction>(relevance)
            }
        )
    );
}

DefaultQuestStrategy::DefaultQuestStrategy(PlayerbotAI* botAI) : QuestStrategy(botAI) {}

void AcceptAllQuestsStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    QuestStrategy::InitTriggers(triggers);

    triggers.push_back(
        new TriggerNode("use game object",
            {
                CreateNextAction<TalkToQuestGiverAction>(relevance),
                CreateNextAction<AcceptAllQuestsAction>(relevance)
            }
        )
    );
    triggers.push_back(
        new TriggerNode("gossip hello",
            {
                CreateNextAction<TalkToQuestGiverAction>(relevance),
                CreateNextAction<AcceptAllQuestsAction>(relevance)
            }
        )
    );
    triggers.push_back(
        new TriggerNode("complete quest",
            {
                CreateNextAction<TalkToQuestGiverAction>(relevance),
                CreateNextAction<AcceptAllQuestsAction>(relevance)
            }
        )
    );
}

AcceptAllQuestsStrategy::AcceptAllQuestsStrategy(PlayerbotAI* botAI) : QuestStrategy(botAI) {}
