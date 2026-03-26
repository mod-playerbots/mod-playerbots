/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "ChatCommandHandlerStrategy.h"

#include "ActionNode.h"
#include "CreateNextAction.h"
#include "CastCustomSpellAction.h"
#include "AttackAction.h"
#include "TellReputationAction.h"
#include "QueryQuestAction.h"
#include "QueryItemUsageAction.h"
#include "AddLootAction.h"
#include "LootAction.h"
#include "UseItemAction.h"
#include "TellItemCountAction.h"
#include "EquipAction.h"
#include "UnequipAction.h"
#include "TradeAction.h"
#include "SellAction.h"
#include "BuyAction.h"
#include "RewardAction.h"
#include "AcceptQuestAction.h"
#include "ChatShortcutActions.h"
#include "GossipHelloAction.h"
#include "TalkToQuestGiverAction.h"
#include "VehicleActions.h"
#include "ReviveFromCorpseAction.h"
#include "TellTargetAction.h"
#include "ReadyCheckAction.h"
#include "MovementActions.h"
#include "TellLosAction.h"
#include "OpenItemAction.h"
#include "QueryItemUsageAction.h"
#include "UnlockItemAction.h"
#include "UnlockTradedItemAction.h"
#include "WipeAction.h"
#include "TameAction.h"
#include "TellGlyphsAction.h"
#include "EquipGlyphsAction.h"
#include "PetsAction.h"
#include "PetAttackAction.h"
#include "LootRollAction.h"

class ChatCommandActionNodeFactoryInternal : public NamedObjectFactory<ActionNode>
{
public:
    ChatCommandActionNodeFactoryInternal() { creators["tank attack chat shortcut"] = &tank_attack_chat_shortcut; }

private:
    static ActionNode* tank_attack_chat_shortcut(PlayerbotAI*)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ {},
            /*C*/ { CreateNextAction<AttackMyTargetAction>(100.0f) }
        );
    }
};

void ChatCommandHandlerStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    PassTroughStrategy::InitTriggers(triggers);

    triggers.push_back(
        new TriggerNode(
            "pvp stats",
            {
                CreateNextAction<TellPvpStatsAction>(relevance)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "rep",
            {
                CreateNextAction<TellReputationAction>(relevance)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "q",
            {
                CreateNextAction<QueryQuestAction>(relevance),
                CreateNextAction<QueryItemUsageAction>(relevance)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "add all loot",
            {
                CreateNextAction<AddAllLootAction>(relevance),
                CreateNextAction<LootAction>(relevance)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "u",
            {
                CreateNextAction<UseItemAction>(relevance)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "c",
            {
                CreateNextAction<TellItemCountAction>(relevance)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "items",
            {
                CreateNextAction<TellItemCountAction>(relevance)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "inv",
            {
                CreateNextAction<TellItemCountAction>(relevance)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "e",
            {
                CreateNextAction<EquipAction>(relevance)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "ue",
            {
                CreateNextAction<UnequipAction>(relevance)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "t",
            {
                CreateNextAction<TradeAction>(relevance)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "nt",
            {
                CreateNextAction<TradeAction>(relevance)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "s",
            {
                CreateNextAction<SellAction>(relevance)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "b",
            {
                CreateNextAction<BuyAction>(relevance)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "r",
            {
                CreateNextAction<RewardAction>(relevance)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "attack",
            {
                CreateNextAction<AttackMyTargetAction>(relevance)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "accept",
            {
                CreateNextAction<AcceptQuestAction>(relevance)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "follow",
            {
                CreateNextAction<FollowChatShortcutAction>(relevance)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "stay",
            {
                CreateNextAction<StayChatShortcutAction>(relevance)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "move from group",
            {
                CreateNextAction<MoveFromGroupChatShortcutAction>(relevance)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "flee",
            {
                CreateNextAction<FleeChatShortcutAction>(relevance)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "naxx",
            {
                CreateNextAction<NaxxChatShortcutAction>(relevance)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
        "tank attack",
        {
            CreateNextAction<TankAttackChatShortcutAction>(relevance)
        }
    )
);
    triggers.push_back(
        new TriggerNode(
            "grind",
            {
                CreateNextAction<GrindChatShortcutAction>(relevance)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "talk",
            {
                CreateNextAction<GossipHelloAction>(relevance),
                CreateNextAction<TalkToQuestGiverAction>(relevance)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "enter vehicle",
            {
                CreateNextAction<EnterVehicleAction>(relevance)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "leave vehicle",
            {
                CreateNextAction<LeaveVehicleAction>(relevance)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "cast",
            {
                CreateNextAction<CastCustomSpellAction>(relevance)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "castnc",
            {
                CreateNextAction<CastCustomNcSpellAction>(relevance)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "revive",
            {
                CreateNextAction<SpiritHealerAction>(relevance)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "runaway",
            {
                CreateNextAction<GoawayChatShortcutAction>(relevance)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "warning",
            {
                CreateNextAction<GoawayChatShortcutAction>(relevance)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "max dps",
            {
                CreateNextAction<MaxDpsChatShortcutAction>(relevance)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "attackers",
            {
                CreateNextAction<TellAttackersAction>(relevance)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "target",
            {
                CreateNextAction<TellTargetAction>(relevance)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "ready",
            {
                CreateNextAction<ReadyCheckAction>(relevance)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "bwl",
            {
                CreateNextAction<BwlChatShortcutAction>(relevance)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "dps",
            {
                CreateNextAction<TellEstimatedDpsAction>(relevance)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "disperse",
            {
                CreateNextAction<DisperseSetAction>(relevance)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "open items",
            {
                CreateNextAction<OpenItemAction>(relevance)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "qi",
            {
                CreateNextAction<QueryItemUsageAction>(relevance)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "unlock items",
            {
                CreateNextAction<UnlockItemAction>(relevance)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "unlock traded item",
            {
                CreateNextAction<UnlockTradedItemAction>(relevance)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "wipe",
            {
                CreateNextAction<WipeAction>(relevance)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "tame",
            {
                CreateNextAction<TameAction>(relevance)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "glyphs",
            {
                CreateNextAction<TellGlyphsAction>(relevance)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "glyph equip",
            {
                CreateNextAction<EquipGlyphsAction>(relevance)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "pet",
            {
                CreateNextAction<PetsAction>(relevance)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "pet attack",
            {
                CreateNextAction<ChatPetAttackAction>(relevance)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "roll",
            {
                CreateNextAction<RollAction>(relevance)
            }
        )
    );
}

ChatCommandHandlerStrategy::ChatCommandHandlerStrategy(PlayerbotAI* botAI) : PassTroughStrategy(botAI)
{
    actionNodeFactories.Add(new ChatCommandActionNodeFactoryInternal());

    this->supported.push_back({ "tell pvp stats", CreateNextAction<TellPvpStatsAction>(relevance).factory });
    this->supported.push_back({ "quests", CreateNextAction<ListQuestsAction>(relevance).factory });
    this->supported.push_back({ "stats", CreateNextAction<StatsAction>(relevance).factory });
    this->supported.push_back({ "leave", CreateNextAction<LeaveGroupAction>(relevance).factory });
    this->supported.push_back({ "reputation", CreateNextAction<TellReputationAction>(relevance).factory });
    this->supported.push_back({ "log", CreateNextAction<LogLevelAction>(relevance).factory });
    this->supported.push_back({ "los", CreateNextAction<TellLosAction>(relevance).factory });
    this->supported.push_back({ "rpg status", CreateNextAction<TellRpgStatusAction>(relevance).factory });
    this->supported.push_back({ "rpg do quest", CreateNextAction<StartRpgDoQuestAction>(relevance).factory });
    this->supported.push_back({ "aura", CreateNextAction<TellAuraAction>(relevance).factory });
    this->supported.push_back({ "drop", CreateNextAction<DropQuestAction>(relevance).factory });
    this->supported.push_back({ "share", CreateNextAction<ShareQuestAction>(relevance).factory });
    this->supported.push_back({ "ll", CreateNextAction<LootStrategyAction>(relevance).factory });
    this->supported.push_back({ "ss", CreateNextAction<SkipSpellsListAction>(relevance).factory });
    this->supported.push_back({ "release", CreateNextAction<ReleaseSpiritAction>(relevance).factory });
    this->supported.push_back({ "teleport", CreateNextAction<TeleportAction>(relevance).factory });
    this->supported.push_back({ "taxi", CreateNextAction<TaxiAction>(relevance).factory });
    this->supported.push_back({ "repair", CreateNextAction<RepairAllAction>(relevance).factory });
    this->supported.push_back({ "talents", CreateNextAction<ChangeTalentsAction>(relevance).factory });
    this->supported.push_back({ "spells", CreateNextAction<ListSpellsAction>(relevance).factory });
    this->supported.push_back({ "co", CreateNextAction<ChangeCombatStrategyAction>(relevance).factory });
    this->supported.push_back({ "nc", CreateNextAction<ChangeNonCombatStrategyAction>(relevance).factory });
    this->supported.push_back({ "de", CreateNextAction<ChangeDeadStrategyAction>(relevance).factory });
    this->supported.push_back({ "trainer", CreateNextAction<TrainerAction>(relevance).factory });
    this->supported.push_back({ "maintenance", CreateNextAction<MaintenanceAction>(relevance).factory });
    this->supported.push_back({ "remove glyph", CreateNextAction<RemoveGlyphAction>(relevance).factory });
    this->supported.push_back({ "autogear", CreateNextAction<AutoGearAction>(relevance).factory });
    this->supported.push_back({ "equip upgrade", CreateNextAction<EquipUpgradeAction>(relevance).factory });
    this->supported.push_back({ "chat", CreateNextAction<ChangeChatAction>(relevance).factory });
    this->supported.push_back({ "home", CreateNextAction<SetHomeAction>(relevance).factory });
    this->supported.push_back({ "destroy", CreateNextAction<DestroyItemAction>(relevance).factory });
    this->supported.push_back({ "reset botAI", CreateNextAction<ResetAiAction>(relevance).factory });
    this->supported.push_back({ "emote", CreateNextAction<EmoteAction>(relevance).factory });
    this->supported.push_back({ "buff", CreateNextAction<BuffAction>(relevance).factory });
    this->supported.push_back({ "help", CreateNextAction<HelpAction>(relevance).factory });
    this->supported.push_back({ "gb", CreateNextAction<GuildBankAction>(relevance).factory });
    this->supported.push_back({ "bank", CreateNextAction<BankAction>(relevance).factory });
    this->supported.push_back({ "invite", CreateNextAction<InviteToGroupAction>(relevance).factory });
    this->supported.push_back({ "lfg", CreateNextAction<LfgAction>(relevance).factory });
    this->supported.push_back({ "spell", CreateNextAction<TellSpellAction>(relevance).factory });
    this->supported.push_back({ "rti", CreateNextAction<RtiAction>(relevance).factory });
    this->supported.push_back({ "position", CreateNextAction<PositionAction>(relevance).factory });
    this->supported.push_back({ "summon", CreateNextAction<SummonAction>(relevance).factory });
    this->supported.push_back({ "who", CreateNextAction<WhoAction>(relevance).factory });
    this->supported.push_back({ "save mana", CreateNextAction<SaveManaAction>(relevance).factory });
    this->supported.push_back({ "formation", CreateNextAction<SetFormationAction>(relevance).factory });
    this->supported.push_back({ "stance", CreateNextAction<SetStanceAction>(relevance).factory });
    this->supported.push_back({ "sendmail", CreateNextAction<SendMailAction>(relevance).factory });
    this->supported.push_back({ "mail", CreateNextAction<MailAction>(relevance).factory });
    // It seems "outfit" command is not implemente);
    // this->supported.push_back({ "outfit" });
    this->supported.push_back({ "go", CreateNextAction<GoAction>(relevance).factory });
    this->supported.push_back({ "debug", CreateNextAction<DebugAction>(relevance).factory });
    this->supported.push_back({ "cdebug", CreateNextAction<DebugAction>(relevance).factory });
    this->supported.push_back({ "cs", CreateNextAction<CustomStrategyEditAction>(relevance).factory });
    this->supported.push_back({ "wts", CreateNextAction<WtsAction>(relevance).factory });
    this->supported.push_back({ "hire", CreateNextAction<HireAction>(relevance).factory });
    this->supported.push_back({ "craft", CreateNextAction<SetCraftAction>(relevance).factory });
    this->supported.push_back({ "flag", CreateNextAction<FlagAction>(relevance).factory });
    this->supported.push_back({ "range", CreateNextAction<RangeAction>(relevance).factory });
    // It seems "ra" command is not implemente);
    // this->supported.push_back({ "ra" });
    this->supported.push_back({ "give leader", CreateNextAction<GiveLeaderAction>(relevance).factory });
    this->supported.push_back({ "cheat", CreateNextAction<CheatAction>(relevance).factory });
    this->supported.push_back({ "ginvite", CreateNextAction<GuildInviteAction>(relevance).factory });
    this->supported.push_back({ "guild promote", CreateNextAction<GuildPromoteAction>(relevance).factory });
    this->supported.push_back({ "guild demote", CreateNextAction<GuildDemoteAction>(relevance).factory });
    this->supported.push_back({ "guild remove", CreateNextAction<GuildRemoveAction>(relevance).factory });
    this->supported.push_back({ "guild leave", CreateNextAction<GuildLeaveAction>(relevance).factory });
    this->supported.push_back({ "rtsc", CreateNextAction<RTSCAction>(relevance).factory });
    this->supported.push_back({ "drink", CreateNextAction<DrinkAction>(relevance).factory });
    this->supported.push_back({ "calc", CreateNextAction<TellCalculateItemAction>(relevance).factory });
    this->supported.push_back({ "open items", CreateNextAction<OpenItemAction>(relevance).factory });
    // It seems "qi" command is not implemente);
    // this->supported.push_back({ "qi" });
    this->supported.push_back({ "unlock items", CreateNextAction<UnlockItemAction>(relevance).factory });
    this->supported.push_back({ "unlock traded item", CreateNextAction<UnlockTradedItemAction>(relevance).factory });
    this->supported.push_back({ "tame", CreateNextAction<TameAction>(relevance).factory });
    this->supported.push_back({ "glyphs", CreateNextAction<TellGlyphsAction>(relevance).factory }); // Added for custom Glyph
    this->supported.push_back({ "glyph equip", CreateNextAction<EquipGlyphsAction>(relevance).factory }); // Added for custom Glyph
    this->supported.push_back({ "pet", CreateNextAction<PetsAction>(relevance).factory });
    this->supported.push_back({ "pet attack", CreateNextAction<ChatPetAttackAction>(relevance).factory });
}
