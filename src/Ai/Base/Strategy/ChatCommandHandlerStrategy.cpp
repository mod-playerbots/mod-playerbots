/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "ChatCommandHandlerStrategy.h"

class ChatCommandActionNodeFactoryInternal : public NamedObjectFactory<ActionNode>
{
public:
    ChatCommandActionNodeFactoryInternal() { creators["tank attack chat shortcut"] = &tank_attack_chat_shortcut; }

private:
    static ActionNode* tank_attack_chat_shortcut(PlayerbotAI* botAI)
    {
        return new ActionNode("tank attack chat shortcut",
                              /*P*/ {},
                              /*A*/ {},
                              /*C*/ { NextAction("attack my target", 100.0f) });
    }
};

void ChatCommandHandlerStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    PassTroughStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode("rep", { NextAction("reputation", relevance) }));
    triggers.push_back(new TriggerNode("pvp stats", { NextAction("tell pvp stats", relevance) }));
    triggers.push_back(new TriggerNode("q", { NextAction("query quest", relevance),
                                                              NextAction("query item usage", relevance) }));
    triggers.push_back(new TriggerNode("add all loot", { NextAction("add all loot", relevance),
                                                                         NextAction("loot", relevance) }));
    triggers.push_back(new TriggerNode("u", { NextAction("use", relevance) }));
    triggers.push_back(new TriggerNode("c", { NextAction("item count", relevance) }));
    triggers.push_back(
        new TriggerNode("items", { NextAction("item count", relevance) }));
    triggers.push_back(new TriggerNode("inv", { NextAction("item count", relevance) }));
    triggers.push_back(new TriggerNode("e", { NextAction("equip", relevance) }));
    triggers.push_back(new TriggerNode("ue", { NextAction("unequip", relevance) }));
    triggers.push_back(new TriggerNode("t", { NextAction("trade", relevance) }));
    triggers.push_back(new TriggerNode("nt", { NextAction("trade", relevance) }));
    triggers.push_back(new TriggerNode("s", { NextAction("sell", relevance) }));
    triggers.push_back(new TriggerNode("b", { NextAction("buy", relevance) }));
    triggers.push_back(new TriggerNode("r", { NextAction("reward", relevance) }));
    triggers.push_back(
        new TriggerNode("attack", { NextAction("attack my target", relevance) }));
    triggers.push_back(
        new TriggerNode("accept", { NextAction("accept quest", relevance) }));
    triggers.push_back(
        new TriggerNode("follow", { NextAction("follow chat shortcut", relevance) }));
    triggers.push_back(
        new TriggerNode("stay", { NextAction("stay chat shortcut", relevance) }));
    triggers.push_back(
        new TriggerNode("move from group", { NextAction("move from group chat shortcut", relevance) }));
    triggers.push_back(
        new TriggerNode("flee", { NextAction("flee chat shortcut", relevance) }));
    triggers.push_back(new TriggerNode(
        "tank attack", { NextAction("tank attack chat shortcut", relevance) }));
    triggers.push_back(
        new TriggerNode("grind", { NextAction("grind chat shortcut", relevance) }));
    triggers.push_back(
        new TriggerNode("talk", { NextAction("gossip hello", relevance),
                                                  NextAction("talk to quest giver", relevance) }));
    triggers.push_back(
        new TriggerNode("enter vehicle", { NextAction("enter vehicle", relevance) }));
    triggers.push_back(
        new TriggerNode("leave vehicle", { NextAction("leave vehicle", relevance) }));
    triggers.push_back(
        new TriggerNode("cast", { NextAction("cast custom spell", relevance) }));
    triggers.push_back(
        new TriggerNode("castnc", { NextAction("cast custom nc spell", relevance) }));
    triggers.push_back(
        new TriggerNode("revive", { NextAction("spirit healer", relevance) }));
    triggers.push_back(
        new TriggerNode("runaway", { NextAction("runaway chat shortcut", relevance) }));
    triggers.push_back(
        new TriggerNode("warning", { NextAction("runaway chat shortcut", relevance) }));
    triggers.push_back(
        new TriggerNode("max dps", { NextAction("max dps chat shortcut", relevance) }));
    triggers.push_back(
        new TriggerNode("attackers", { NextAction("tell attackers", relevance) }));
    triggers.push_back(
        new TriggerNode("target", { NextAction("tell target", relevance) }));
    triggers.push_back(
        new TriggerNode("ready", { NextAction("ready check", relevance) }));
    triggers.push_back(
        new TriggerNode("bwl", { NextAction("bwl chat shortcut", relevance) }));
    triggers.push_back(
        new TriggerNode("dps", { NextAction("tell estimated dps", relevance) }));
    triggers.push_back(
        new TriggerNode("disperse", { NextAction("disperse set", relevance) }));
    triggers.push_back(
        new TriggerNode("open items", { NextAction("open items", relevance) }));
    triggers.push_back(
        new TriggerNode("qi", { NextAction("query item usage", relevance) }));
    triggers.push_back(
        new TriggerNode("unlock items", { NextAction("unlock items", relevance) }));
    triggers.push_back(
        new TriggerNode("unlock traded item", { NextAction("unlock traded item", relevance) }));
    triggers.push_back(
        new TriggerNode("wipe", { NextAction("wipe", relevance) }));
    triggers.push_back(new TriggerNode("tame", { NextAction("tame", relevance) }));
    triggers.push_back(new TriggerNode("glyphs", { NextAction("glyphs", relevance) })); // Added for custom Glyphs
    triggers.push_back(new TriggerNode("glyph equip", { NextAction("glyph equip", relevance) })); // Added for custom Glyphs
    triggers.push_back(new TriggerNode("pet", { NextAction("pet", relevance) }));
    triggers.push_back(new TriggerNode("pet attack", { NextAction("pet attack", relevance) }));
    triggers.push_back(new TriggerNode("roll", { NextAction("roll", relevance) }));
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
