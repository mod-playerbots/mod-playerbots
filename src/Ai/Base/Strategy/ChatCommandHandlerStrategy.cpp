/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "ChatCommandHandlerStrategy.h"

#include "Playerbots.h"
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
                CreateNextAction<PetAttackAction>(relevance)
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

    supported.push_back("quests");
    supported.push_back("stats");
    supported.push_back("leave");
    supported.push_back("reputation");
    supported.push_back("tell pvp stats");
    supported.push_back("log");
    supported.push_back("los");
    supported.push_back("rpg status");
    supported.push_back("rpg do quest");
    supported.push_back("aura");
    supported.push_back("drop");
    supported.push_back("share");
    supported.push_back("ll");
    supported.push_back("ss");
    supported.push_back("release");
    supported.push_back("teleport");
    supported.push_back("taxi");
    supported.push_back("repair");
    supported.push_back("talents");
    supported.push_back("spells");
    supported.push_back("co");
    supported.push_back("nc");
    supported.push_back("de");
    supported.push_back("trainer");
    supported.push_back("maintenance");
    supported.push_back("remove glyph");
    supported.push_back("autogear");
    supported.push_back("equip upgrade");
    supported.push_back("chat");
    supported.push_back("home");
    supported.push_back("destroy");
    supported.push_back("reset botAI");
    supported.push_back("emote");
    supported.push_back("buff");
    supported.push_back("help");
    supported.push_back("gb");
    supported.push_back("bank");
    supported.push_back("invite");
    supported.push_back("lfg");
    supported.push_back("spell");
    supported.push_back("rti");
    supported.push_back("position");
    supported.push_back("summon");
    supported.push_back("who");
    supported.push_back("save mana");
    supported.push_back("formation");
    supported.push_back("stance");
    supported.push_back("sendmail");
    supported.push_back("mail");
    supported.push_back("outfit");
    supported.push_back("go");
    supported.push_back("debug");
    supported.push_back("cdebug");
    supported.push_back("cs");
    supported.push_back("wts");
    supported.push_back("hire");
    supported.push_back("craft");
    supported.push_back("flag");
    supported.push_back("range");
    supported.push_back("ra");
    supported.push_back("give leader");
    supported.push_back("cheat");
    supported.push_back("ginvite");
    supported.push_back("guild promote");
    supported.push_back("guild demote");
    supported.push_back("guild remove");
    supported.push_back("guild leave");
    supported.push_back("rtsc");
    supported.push_back("drink");
    supported.push_back("calc");
    supported.push_back("open items");
    supported.push_back("qi");
    supported.push_back("unlock items");
    supported.push_back("unlock traded item");
    supported.push_back("tame");
    supported.push_back("glyphs"); // Added for custom Glyphs
    supported.push_back("glyph equip"); // Added for custom Glyphs
    supported.push_back("pet");
    supported.push_back("pet attack");
}
