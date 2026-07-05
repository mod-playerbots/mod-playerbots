/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef PLAYERBOTS_QUESTSTRATEGIES_H
#define PLAYERBOTS_QUESTSTRATEGIES_H

#include "PassThroughStrategy.h"

class PlayerbotAI;

class QuestStrategy : public PassThroughStrategy
{
public:
    QuestStrategy(PlayerbotAI* botAI);

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

class DefaultQuestStrategy : public QuestStrategy
{
public:
    DefaultQuestStrategy(PlayerbotAI* botAI);

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "quest"; }
};

class AcceptAllQuestsStrategy : public QuestStrategy
{
public:
    AcceptAllQuestsStrategy(PlayerbotAI* botAI);

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "accept all quests"; }
};

#endif
