/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_WORLDPACKETHANDLERSTRATEGY_H
#define _PLAYERBOT_WORLDPACKETHANDLERSTRATEGY_H

#include "AcceptQuestAction.h"
#include "BattleGroundJoinAction.h"
#include "CheckMountStateAction.h"
#include "CreateNextAction.h"
#include "GuildAcceptAction.h"
#include "InventoryChangeFailureAction.h"
#include "LeaveGroupAction.h"
#include "LfgActions.h"
#include "LootRollAction.h"
#include "PassTroughStrategy.h"
#include "QuestAction.h"
#include "RandomBotUpdateAction.h"
#include "ReadyCheckAction.h"
#include "SecurityCheckAction.h"

class PlayerbotAI;

class WorldPacketHandlerStrategy : public PassTroughStrategy
{
public:
    WorldPacketHandlerStrategy(PlayerbotAI* botAI) : PassTroughStrategy(botAI)
    {
        this->supported.push_back({ "loot roll", CreateNextAction<LootRollAction>(relevance).factory });
        this->supported.push_back({ "check mount state", CreateNextAction<CheckMountStateAction>(relevance).factory });
        this->supported.push_back({ "party command", CreateNextAction<PartyCommandAction>(relevance).factory });
        this->supported.push_back({ "ready check", CreateNextAction<ReadyCheckAction>(relevance).factory });
        this->supported.push_back({ "uninvite", CreateNextAction<UninviteAction>(relevance).factory });
        this->supported.push_back({ "lfg role check", CreateNextAction<LfgRoleCheckAction>(relevance).factory });
        this->supported.push_back({ "lfg teleport", CreateNextAction<LfgTeleportAction>(relevance).factory });
        this->supported.push_back({ "random bot update", CreateNextAction<RandomBotUpdateAction>(relevance).factory });
        this->supported.push_back({ "inventory change failure", CreateNextAction<InventoryChangeFailureAction>(relevance).factory });
        this->supported.push_back({ "guild accept", CreateNextAction<GuildAcceptAction>(relevance).factory });
        this->supported.push_back({ "security check", CreateNextAction<SecurityCheckAction>(relevance).factory });
        this->supported.push_back({ "bg status", CreateNextAction<BGStatusAction>(relevance).factory});

        // quests
        this->supported.push_back({ "quest update add kill", CreateNextAction<QuestUpdateAddKillAction>(relevance).factory});
        this->supported.push_back({ "quest update add item", CreateNextAction<QuestUpdateAddItemAction>(relevance).factory});
        this->supported.push_back({ "quest update failed", CreateNextAction<QuestUpdateFailedAction>(relevance).factory});
        this->supported.push_back({ "quest update failed timer", CreateNextAction<QuestUpdateFailedTimerAction>(relevance).factory});
        this->supported.push_back({ "quest update complete", CreateNextAction<QuestUpdateCompleteAction>(relevance).factory});
        this->supported.push_back({ "confirm quest", CreateNextAction<ConfirmQuestAction>(relevance).factory});
    }

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "default"; }
};

class ReadyCheckStrategy : public PassTroughStrategy
{
public:
    ReadyCheckStrategy(PlayerbotAI* botAI) : PassTroughStrategy(botAI) { }

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "ready check"; }
};

#endif
