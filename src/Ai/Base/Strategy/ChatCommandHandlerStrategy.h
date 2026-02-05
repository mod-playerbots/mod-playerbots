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
#include "CreateNextAction.h"
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
#include "NewRpgAction.h"
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

protected:
    std::vector<PassthroughStrategySupportedActionsStruct> supported = {
        { "tell pvp stats", CreateNextAction<TellPvpStatsAction>(relevance).factory },
        { "quests", CreateNextAction<ListQuestsAction>(relevance).factory },
        { "stats", CreateNextAction<StatsAction>(relevance).factory },
        { "leave", CreateNextAction<LeaveGroupAction>(relevance).factory },
        { "reputation", CreateNextAction<TellReputationAction>(relevance).factory },
        { "log", CreateNextAction<LogLevelAction>(relevance).factory },
        { "los", CreateNextAction<TellLosAction>(relevance).factory },
        { "rpg status", CreateNextAction<TellRpgStatusAction>(relevance).factory },
        { "rpg do quest", CreateNextAction<StartRpgDoQuestAction>(relevance).factory },
        { "aura", CreateNextAction<TellAuraAction>(relevance).factory },
        { "drop", CreateNextAction<DropQuestAction>(relevance).factory },
        { "share", CreateNextAction<ShareQuestAction>(relevance).factory },
        { "ll", CreateNextAction<LootStrategyAction>(relevance).factory },
        { "ss", CreateNextAction<SkipSpellsListAction>(relevance).factory },
        { "release", CreateNextAction<ReleaseSpiritAction>(relevance).factory },
        { "teleport", CreateNextAction<TeleportAction>(relevance).factory },
        { "taxi", CreateNextAction<TaxiAction>(relevance).factory },
        { "repair", CreateNextAction<RepairAllAction>(relevance).factory },
        { "talents", CreateNextAction<ChangeTalentsAction>(relevance).factory },
        { "spells", CreateNextAction<ListSpellsAction>(relevance).factory },
        { "co", CreateNextAction<ChangeCombatStrategyAction>(relevance).factory },
        { "nc", CreateNextAction<ChangeNonCombatStrategyAction>(relevance).factory },
        { "de", CreateNextAction<ChangeDeadStrategyAction>(relevance).factory },
        { "trainer", CreateNextAction<TrainerAction>(relevance).factory },
        { "maintenance", CreateNextAction<MaintenanceAction>(relevance).factory },
        { "remove glyph", CreateNextAction<RemoveGlyphAction>(relevance).factory },
        { "autogear", CreateNextAction<AutoGearAction>(relevance).factory },
        { "equip upgrade", CreateNextAction<EquipUpgradeAction>(relevance).factory },
        { "chat", CreateNextAction<ChangeChatAction>(relevance).factory },
        { "home", CreateNextAction<SetHomeAction>(relevance).factory },
        { "destroy", CreateNextAction<DestroyItemAction>(relevance).factory },
        { "reset botAI", CreateNextAction<ResetAiAction>(relevance).factory },
        { "emote", CreateNextAction<EmoteAction>(relevance).factory },
        { "buff", CreateNextAction<BuffAction>(relevance).factory },
        { "help", CreateNextAction<HelpAction>(relevance).factory },
        { "gb", CreateNextAction<GuildBankAction>(relevance).factory },
        { "bank", CreateNextAction<BankAction>(relevance).factory },
        { "invite", CreateNextAction<InviteToGroupAction>(relevance).factory },
        { "lfg", CreateNextAction<LfgAction>(relevance).factory },
        { "spell", CreateNextAction<TellSpellAction>(relevance).factory },
        { "rti", CreateNextAction<RtiAction>(relevance).factory },
        { "position", CreateNextAction<PositionAction>(relevance).factory },
        { "summon", CreateNextAction<SummonAction>(relevance).factory },
        { "who", CreateNextAction<WhoAction>(relevance).factory },
        { "save mana", CreateNextAction<SaveManaAction>(relevance).factory },
        { "formation", CreateNextAction<SetFormationAction>(relevance).factory },
        { "stance", CreateNextAction<SetStanceAction>(relevance).factory },
        { "sendmail", CreateNextAction<SendMailAction>(relevance).factory },
        { "mail", CreateNextAction<MailAction>(relevance).factory },
        // It seems "outfit" command is not implemented
        // { "outfit" },
        { "go", CreateNextAction<GoAction>(relevance).factory },
        { "debug", CreateNextAction<DebugAction>(relevance).factory },
        { "cdebug", CreateNextAction<DebugAction>(relevance).factory },
        { "cs", CreateNextAction<CustomStrategyEditAction>(relevance).factory },
        { "wts", CreateNextAction<WtsAction>(relevance).factory },
        { "hire", CreateNextAction<HireAction>(relevance).factory },
        { "craft", CreateNextAction<SetCraftAction>(relevance).factory },
        { "flag", CreateNextAction<FlagAction>(relevance).factory },
        { "range", CreateNextAction<RangeAction>(relevance).factory },
        // It seems "ra" command is not implemented
        // { "ra" },
        { "give leader", CreateNextAction<GiveLeaderAction>(relevance).factory },
        { "cheat", CreateNextAction<CheatAction>(relevance).factory },
        { "ginvite", CreateNextAction<GuildInviteAction>(relevance).factory },
        { "guild promote", CreateNextAction<GuildPromoteAction>(relevance).factory },
        { "guild demote", CreateNextAction<GuildDemoteAction>(relevance).factory },
        { "guild remove", CreateNextAction<GuildRemoveAction>(relevance).factory },
        { "guild leave", CreateNextAction<GuildLeaveAction>(relevance).factory },
        { "rtsc", CreateNextAction<RTSCAction>(relevance).factory },
        { "drink", CreateNextAction<DrinkAction>(relevance).factory },
        { "calc", CreateNextAction<TellCalculateItemAction>(relevance).factory },
        { "open items", CreateNextAction<OpenItemAction>(relevance).factory },
        // It seems "qi" command is not implemented
        // { "qi" },
        { "unlock items", CreateNextAction<UnlockItemAction>(relevance).factory },
        { "unlock traded item", CreateNextAction<UnlockTradedItemAction>(relevance).factory },
        { "tame", CreateNextAction<TameAction>(relevance).factory },
        { "glyphs", CreateNextAction<TellGlyphsAction>(relevance).factory }, // Added for custom Glyphs
        { "glyph equip", CreateNextAction<EquipGlyphsAction>(relevance).factory }, // Added for custom Glyphs
        { "pet", CreateNextAction<PetsAction>(relevance).factory },
        { "pet attack", CreateNextAction<ChatPetAttackAction>(relevance).factory },
    };
};

#endif
