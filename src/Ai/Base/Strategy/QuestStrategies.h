/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_QUESTSTRATEGIES_H
#define _PLAYERBOT_QUESTSTRATEGIES_H

#include "AcceptQuestAction.h"
#include "CreateAction.h"
#include "PassTroughStrategy.h"

class PlayerbotAI;

class QuestStrategy : public PassTroughStrategy
{
public:
    QuestStrategy(PlayerbotAI* botAI) : PassTroughStrategy(botAI) {};

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;

protected:
    std::vector<PassthroughStrategySupportedActionsStruct> supported = {
        {
            .name = "accept quest",
            .factory = &CreateAction<AcceptQuestAction>
        }
    };
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
