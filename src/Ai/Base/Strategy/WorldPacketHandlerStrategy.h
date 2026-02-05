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
    WorldPacketHandlerStrategy(PlayerbotAI* botAI) : PassTroughStrategy(botAI) { }

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "default"; }

protected:
    std::vector<PassthroughStrategySupportedActionsStruct> supported = {
        { "loot roll", CreateNextAction<LootRollAction>(relevance).factory },
        { "check mount state", CreateNextAction<CheckMountStateAction>(relevance).factory },
        { "party command", CreateNextAction<PartyCommandAction>(relevance).factory },
        { "ready check", CreateNextAction<ReadyCheckAction>(relevance).factory },
        { "uninvite", CreateNextAction<UninviteAction>(relevance).factory },
        { "lfg role check", CreateNextAction<LfgRoleCheckAction>(relevance).factory },
        { "lfg teleport", CreateNextAction<LfgTeleportAction>(relevance).factory },
        { "random bot update", CreateNextAction<RandomBotUpdateAction>(relevance).factory },
        { "inventory change failure", CreateNextAction<InventoryChangeFailureAction>(relevance).factory },
        { "guild accept", CreateNextAction<GuildAcceptAction>(relevance).factory },
        { "security check", CreateNextAction<SecurityCheckAction>(relevance).factory },
        { "bg status", CreateNextAction<BGStatusAction>(relevance).factory},

        // quests
        { "quest update add kill", CreateNextAction<QuestUpdateAddKillAction>(relevance).factory},
        { "quest update add item", CreateNextAction<QuestUpdateAddItemAction>(relevance).factory},
        { "quest update failed", CreateNextAction<QuestUpdateFailedAction>(relevance).factory},
        { "quest update failed timer", CreateNextAction<QuestUpdateFailedTimerAction>(relevance).factory},
        { "quest update complete", CreateNextAction<QuestUpdateCompleteAction>(relevance).factory},
        { "confirm quest", CreateNextAction<ConfirmQuestAction>(relevance).factory},
    };
};

class ReadyCheckStrategy : public PassTroughStrategy
{
public:
    ReadyCheckStrategy(PlayerbotAI* botAI) : PassTroughStrategy(botAI) { }

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "ready check"; }
};

#endif
