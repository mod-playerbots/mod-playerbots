/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "QuestStrategies.h"

#include "Playerbots.h"

QuestStrategy::QuestStrategy(PlayerbotAI* botAI) : PassTroughStrategy(botAI) { supported.push_back("accept quest"); }

void QuestStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    PassTroughStrategy::InitTriggers(triggers);

    triggers.push_back(
        new TriggerNode("quest share", {  NextAction("accept quest share", relevance) }));
}

void DefaultQuestStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    QuestStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "use game object", {  NextAction("talk to quest giver", relevance) }));
    triggers.push_back(new TriggerNode(
        "gossip hello", {  NextAction("talk to quest giver", relevance) }));
    triggers.push_back(new TriggerNode(
        "complete quest", {  NextAction("talk to quest giver", relevance) }));
}

DefaultQuestStrategy::DefaultQuestStrategy(PlayerbotAI* botAI) : QuestStrategy(botAI) {}

void AcceptAllQuestsStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    QuestStrategy::InitTriggers(triggers);

    triggers.push_back(
        new TriggerNode("use game object", {  NextAction("talk to quest giver", relevance),
                                                             NextAction("accept all quests", relevance) }));
    triggers.push_back(
        new TriggerNode("gossip hello", {  NextAction("talk to quest giver", relevance),
                                                          NextAction("accept all quests", relevance) }));
    triggers.push_back(
        new TriggerNode("complete quest", {  NextAction("talk to quest giver", relevance),
                                                            NextAction("accept all quests", relevance) }));
}

AcceptAllQuestsStrategy::AcceptAllQuestsStrategy(PlayerbotAI* botAI) : QuestStrategy(botAI) {}
