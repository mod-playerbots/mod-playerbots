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
}
