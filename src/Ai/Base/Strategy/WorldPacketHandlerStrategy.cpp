/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "WorldPacketHandlerStrategy.h"
#include "CreateNextAction.h"
#include "AcceptInvitationAction.h"
#include "LeaveGroupAction.h"

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
                /*NextAction("leader", relevance),*/
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "not enough money",
            {
                CreateNextAction("tell not enough money", relevance)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "not enough reputation",
            {
                CreateNextAction("tell not enough reputation", relevance)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "cannot equip",
            {
                CreateNextAction("tell cannot equip", relevance)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "use game object",
            {
                CreateNextAction("add loot", relevance),
                CreateNextAction("use meeting stone", relevance)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "gossip hello",
            {
                CreateNextAction("trainer", relevance)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "activate taxi",
            {
                CreateNextAction("remember taxi", relevance),
                CreateNextAction("taxi", relevance)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "taxi done",
            {
                CreateNextAction("taxi", relevance)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "trade status",
            {
                CreateNextAction("accept trade", relevance),
                CreateNextAction("equip upgrades", relevance)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "trade status extended",
            {
                CreateNextAction("trade status extended", relevance)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "area trigger",
            {
                CreateNextAction("reach area trigger", relevance)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "within area trigger",
            {
                CreateNextAction("area trigger", relevance)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "loot response",
            {
                CreateNextAction("store loot", relevance)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "item push result",
            {
                CreateNextAction("unlock items", relevance),
                CreateNextAction("open items", relevance),
                CreateNextAction("query item usage", relevance),
                CreateNextAction("equip upgrades", relevance)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "item push result",
            {
                CreateNextAction("quest item push result", relevance)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "ready check finished",
            {
                CreateNextAction("finish ready check", relevance)
            }
        )
    );
    // triggers.push_back(new TriggerNode("often", { NextAction("security check", relevance), NextAction("check mail", relevance) }));
    triggers.push_back(
        new TriggerNode(
            "guild invite",
            {
                CreateNextAction("guild accept", relevance)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "petition offer",
            {
                CreateNextAction("petition sign", relevance)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "lfg proposal",
            {
                CreateNextAction("lfg accept", relevance)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "lfg proposal active",
            {
                CreateNextAction("lfg accept", relevance)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "arena team invite",
            {
                CreateNextAction("arena team accept", relevance)
            }
        )
    );
    //triggers.push_back(new TriggerNode("no non bot players around", { NextAction("delay", relevance) }));
    triggers.push_back(
        new TriggerNode(
            "bg status",
            {
                CreateNextAction("bg status", relevance)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "xpgain",
            {
                CreateNextAction("xp gain", relevance)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "levelup",
            {
                CreateNextAction("auto maintenance on levelup", relevance + 3)
            }
        )
    );
    // triggers.push_back(new TriggerNode("group destroyed", { NextAction("reset botAI",
    // relevance) }));
    triggers.push_back(
        new TriggerNode(
            "group list",
            {
                CreateNextAction("reset botAI", relevance)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "see spell",
            {
                CreateNextAction("see spell", relevance)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "release spirit",
            {
                CreateNextAction("release", relevance)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "revive from corpse",
            {
                CreateNextAction("revive from corpse", relevance)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "master loot roll",
            {
                CreateNextAction("master loot roll", relevance)
            }
        )
    );

    // quest ?
    //triggers.push_back(new TriggerNode("quest confirm", { NextAction("quest confirm", relevance) }));
    triggers.push_back(
        new TriggerNode(
            "questgiver quest details",
            {
                CreateNextAction("turn in query quest", relevance)
            }
        )
    );

    // loot roll
    triggers.push_back(
        new TriggerNode(
            "very often",
            {
                CreateNextAction("loot roll", relevance)
            }
        )
    );
}

WorldPacketHandlerStrategy::WorldPacketHandlerStrategy(PlayerbotAI* botAI) : PassTroughStrategy(botAI)
{
    supported.push_back("loot roll");
    supported.push_back("check mount state");
    supported.push_back("party command");
    supported.push_back("ready check");
    supported.push_back("uninvite");
    supported.push_back("lfg role check");
    supported.push_back("lfg teleport");
    supported.push_back("random bot update");
    supported.push_back("inventory change failure");
    supported.push_back("bg status");

    // quests
    supported.push_back("quest update add kill");
    // supported.push_back("quest update add item");
    supported.push_back("quest update failed");
    supported.push_back("quest update failed timer");
    supported.push_back("quest update complete");
    supported.push_back("confirm quest");
}

void ReadyCheckStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "timer",
            {
                CreateNextAction<ReadyCheckStrategy>(relevance)
            }
        )
    );
}
