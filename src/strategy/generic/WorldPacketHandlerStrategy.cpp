/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "WorldPacketHandlerStrategy.h"

void WorldPacketHandlerStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    PassTroughStrategy::InitTriggers(triggers);

    triggers.push_back(
        new TriggerNode("group invite", { new NextAction("accept invitation", relevance) }));
    triggers.push_back(
        new TriggerNode("uninvite", { new NextAction("uninvite", relevance) }));
    triggers.push_back(
        new TriggerNode("uninvite guid", { new NextAction("uninvite", relevance) }));
    triggers.push_back(
        new TriggerNode("group set leader", { /*new NextAction("leader", relevance),*/ }));
    triggers.push_back(new TriggerNode(
        "not enough money", { new NextAction("tell not enough money", relevance) }));
    triggers.push_back(
        new TriggerNode("not enough reputation",
                        { new NextAction("tell not enough reputation", relevance) }));
    triggers.push_back(
        new TriggerNode("cannot equip", { new NextAction("tell cannot equip", relevance) }));
    triggers.push_back(
        new TriggerNode("use game object", { new NextAction("add loot", relevance),
                                                             new NextAction("use meeting stone", relevance) }));
    triggers.push_back(
        new TriggerNode("gossip hello", { new NextAction("trainer", relevance) }));
    triggers.push_back(new TriggerNode("activate taxi", { new NextAction("remember taxi", relevance),
                                                                          new NextAction("taxi", relevance) }));
    triggers.push_back(new TriggerNode("taxi done", { new NextAction("taxi", relevance) }));
    triggers.push_back(new TriggerNode("trade status", { new NextAction("accept trade", relevance), new NextAction("equip upgrades", relevance) }));
    triggers.push_back(new TriggerNode("trade status extended", { new NextAction("trade status extended", relevance) }));
    triggers.push_back(new TriggerNode("area trigger", { new NextAction("reach area trigger", relevance) }));
    triggers.push_back(new TriggerNode("within area trigger", { new NextAction("area trigger", relevance) }));
    triggers.push_back(new TriggerNode("loot response", { new NextAction("store loot", relevance) }));
    triggers.push_back(new TriggerNode("item push result", { new NextAction("unlock items", relevance),
                                                                                new NextAction("open items", relevance),
                                                                                new NextAction("query item usage", relevance),
                                                                                new NextAction("equip upgrades", relevance) }));
    triggers.push_back(new TriggerNode("item push result", { new NextAction("quest item push result", relevance) }));
    triggers.push_back(new TriggerNode("ready check finished", { new NextAction("finish ready check", relevance) }));
    // triggers.push_back(new TriggerNode("often", { new NextAction("security check", relevance), new NextAction("check mail", relevance) }));
    triggers.push_back(new TriggerNode("guild invite", { new NextAction("guild accept", relevance) }));
    triggers.push_back(new TriggerNode("petition offer", { new NextAction("petition sign", relevance) }));
    triggers.push_back(new TriggerNode("lfg proposal", { new NextAction("lfg accept", relevance) }));
    triggers.push_back(new TriggerNode("lfg proposal active", { new NextAction("lfg accept", relevance) }));
    triggers.push_back(new TriggerNode("arena team invite", { new NextAction("arena team accept", relevance) }));
    //triggers.push_back(new TriggerNode("no non bot players around", { new NextAction("delay", relevance) }));
    triggers.push_back(new TriggerNode("bg status", { new NextAction("bg status", relevance) }));
    triggers.push_back(new TriggerNode("xpgain", { new NextAction("xp gain", relevance) }));
    triggers.push_back(
        new TriggerNode("levelup", { new NextAction("auto maintenance on levelup", relevance + 3) }));
    // triggers.push_back(new TriggerNode("group destroyed", { new NextAction("reset botAI",
    // relevance) }));
    triggers.push_back(new TriggerNode("group list", { new NextAction("reset botAI", relevance) }));
    triggers.push_back(new TriggerNode("see spell", { new NextAction("see spell", relevance) }));
    triggers.push_back(new TriggerNode("release spirit", { new NextAction("release", relevance) }));
    triggers.push_back(new TriggerNode("revive from corpse", { new NextAction("revive from corpse", relevance) }));
    triggers.push_back(new TriggerNode("master loot roll", { new NextAction("master loot roll", relevance) }));

    // quest ?
    //triggers.push_back(new TriggerNode("quest confirm", { new NextAction("quest confirm", relevance) }));
    triggers.push_back(new TriggerNode("questgiver quest details", { new NextAction("turn in query quest", relevance) }));

    // loot roll
    triggers.push_back(new TriggerNode("very often", { new NextAction("loot roll", relevance) }));
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
        new TriggerNode("timer", { new NextAction("ready check", relevance) }));
}
