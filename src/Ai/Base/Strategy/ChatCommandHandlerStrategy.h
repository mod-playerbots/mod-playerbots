/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_CHATCOMMANDHANDLERSTRATEGY_H
#define _PLAYERBOT_CHATCOMMANDHANDLERSTRATEGY_H

#include "BuffAction.h"
#include "ChangeChatAction.h"
#include "ChangeStrategyAction.h"
#include "ChangeTalentsAction.h"
#include "ChatActionContext.h"
#include "DebugAction.h"
#include "DestroyItemAction.h"
#include "DrinkAction.h"
#include "EmoteAction.h"
#include "EquipAction.h"
#include "FlagAction.h"
#include "Formations.h"
#include "GoAction.h"
#include "HelpAction.h"
#include "HireAction.h"
#include "InviteToGroupAction.h"
#include "LeaveGroupAction.h"
#include "ListQuestsActions.h"
#include "ListSpellsAction.h"
#include "LogLevelAction.h"
#include "MailAction.h"
#include "PassLeadershipToMasterAction.h"
#include "PassTroughStrategy.h"
#include "PetAttackAction.h"
#include "PositionAction.h"
#include "RangeAction.h"
#include "ReleaseSpiritAction.h"
#include "RepairAllAction.h"
#include "ResetAiAction.h"
#include "RtiAction.h"
#include "SaveManaAction.h"
#include "SendMailAction.h"
#include "SetCraftAction.h"
#include "SetHomeAction.h"
#include "SkipSpellsListAction.h"
#include "Stances.h"
#include "StatsAction.h"
#include "TaxiAction.h"
#include "TeleportAction.h"
#include "TellCastFailedAction.h"
#include "TellLosAction.h"
#include "TellReputationAction.h"
#include "TrainerAction.h"
#include "UseMeetingStoneAction.h"
#include "WhoAction.h"
#include "WtsAction.h"

class PlayerbotAI;

class ChatCommandHandlerStrategy : public PassTroughStrategy
{
public:
    ChatCommandHandlerStrategy(PlayerbotAI* botAI);

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "chat"; }
};

#endif
