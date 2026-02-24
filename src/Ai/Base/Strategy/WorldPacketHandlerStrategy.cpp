/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "WorldPacketHandlerStrategy.h"
#include "AddLootAction.h"
#include "AreaTriggerAction.h"
#include "ArenaTeamActions.h"
#include "AutoMaintenanceOnLevelupAction.h"
#include "BattleGroundJoinAction.h"
#include "CreateNextAction.h"
#include "AcceptInvitationAction.h"
#include "EquipAction.h"
#include "GuildAcceptAction.h"
#include "InventoryChangeFailureAction.h"
#include "LeaveGroupAction.h"
#include "LfgActions.h"
#include "LootAction.h"
#include "LootRollAction.h"
#include "OpenItemAction.h"
#include "PassLeadershipToMasterAction.h"
#include "PetitionSignAction.h"
#include "QuestAction.h"
#include "ReadyCheckAction.h"
#include "ReleaseSpiritAction.h"
#include "RememberTaxiAction.h"
#include "ResetAiAction.h"
#include "SeeSpellAction.h"
#include "TalkToQuestGiverAction.h"
#include "TaxiAction.h"
#include "TellMasterAction.h"
#include "TradeStatusAction.h"
#include "TradeStatusExtendedAction.h"
#include "TrainerAction.h"
#include "UnlockItemAction.h"
#include "XpGainAction.h"

void WorldPacketHandlerStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    PassTroughStrategy::InitTriggers(triggers);

    triggers.push_back(
        new TriggerNode(
            "group invite",
            {
                CreateNextAction<AcceptInvitationAction>(relevance)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "uninvite",
            {
                CreateNextAction<UninviteAction>(relevance)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "uninvite guid",
            {
                CreateNextAction<UninviteAction>(relevance)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "group set leader",
            {
                CreateNextAction<PassLeadershipToMasterAction>(relevance),
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "not enough money",
            {
                CreateNextAction<TellMasterNotEnoughMoneyAction>(relevance)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "not enough reputation",
            {
                CreateNextAction<TellMasterNotEnoughReputationAction>(relevance)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "cannot equip",
            {
                CreateNextAction<InventoryChangeFailureAction>(relevance)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "use game object",
            {
                CreateNextAction<AddLootAction>(relevance),
                CreateNextAction<UseMeetingStoneAction>(relevance)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "gossip hello",
            {
                CreateNextAction<TrainerAction>(relevance)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "activate taxi",
            {
                CreateNextAction<RememberTaxiAction>(relevance),
                CreateNextAction<TaxiAction>(relevance)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "taxi done",
            {
                CreateNextAction<TaxiAction>(relevance)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "trade status",
            {
                CreateNextAction<TradeStatusAction>(relevance),
                CreateNextAction<EquipUpgradesPacketAction>(relevance)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "trade status extended",
            {
                CreateNextAction<TradeStatusExtendedAction>(relevance)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "area trigger",
            {
                CreateNextAction<ReachAreaTriggerAction>(relevance)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "within area trigger",
            {
                CreateNextAction<AreaTriggerAction>(relevance)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "loot response",
            {
                CreateNextAction<StoreLootAction>(relevance)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "item push result",
            {
                CreateNextAction<UnlockItemAction>(relevance),
                CreateNextAction<OpenItemAction>(relevance),
                CreateNextAction<QueryItemUsageAction>(relevance),
                CreateNextAction<EquipUpgradesPacketAction>(relevance)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "loot roll won",
            {
                CreateNextAction<EquipUpgradesPacketAction>(relevance)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "item push result",
            {
                CreateNextAction<QuestItemPushResultAction>(relevance)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "ready check finished",
            {
                CreateNextAction<FinishReadyCheckAction>(relevance)
            }
        )
    );
    // triggers.push_back(new TriggerNode("often", { NextAction("security check", relevance), NextAction("check mail", relevance) }));
    triggers.push_back(
        new TriggerNode(
            "guild invite",
            {
                CreateNextAction<GuildAcceptAction>(relevance)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "petition offer",
            {
                CreateNextAction<PetitionSignAction>(relevance)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "lfg proposal",
            {
                CreateNextAction<LfgAcceptAction>(relevance)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "lfg proposal active",
            {
                CreateNextAction<LfgAcceptAction>(relevance)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "arena team invite",
            {
                CreateNextAction<ArenaTeamAcceptAction>(relevance)
            }
        )
    );
    //triggers.push_back(new TriggerNode("no non bot players around", { NextAction("delay", relevance) }));
    triggers.push_back(
        new TriggerNode(
            "bg status",
            {
                CreateNextAction<BGStatusAction>(relevance)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "xpgain",
            {
                CreateNextAction<XpGainAction>(relevance)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "levelup",
            {
                CreateNextAction<AutoMaintenanceOnLevelupAction>(relevance + 3.0f)
            }
        )
    );
    // triggers.push_back(new TriggerNode("group destroyed", { NextAction("reset botAI",
    // relevance) }));
    triggers.push_back(
        new TriggerNode(
            "group list",
            {
                CreateNextAction<ResetAiAction>(relevance)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "see spell",
            {
                CreateNextAction<SeeSpellAction>(relevance)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "release spirit",
            {
                CreateNextAction<ReleaseSpiritAction>(relevance)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "revive from corpse",
            {
                CreateNextAction<ReviveFromCorpseAction>(relevance)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "master loot roll",
            {
                CreateNextAction<MasterLootRollAction>(relevance)
            }
        )
    );

    // quest ?
    //triggers.push_back(new TriggerNode("quest confirm", { NextAction("quest confirm", relevance) }));
    triggers.push_back(
        new TriggerNode(
            "questgiver quest details",
            {
                CreateNextAction<TurnInQueryQuestAction>(relevance)
            }
        )
    );

    // loot roll
    triggers.push_back(
        new TriggerNode(
            "very often",
            {
                CreateNextAction<LootRollAction>(relevance)
            }
        )
    );
}

void ReadyCheckStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "timer",
            {
                CreateNextAction<ReadyCheckAction>(relevance)
            }
        )
    );
}
