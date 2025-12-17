/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "SurvivalHunterStrategy.h"
#include "Playerbots.h"

// ===== Action Node Factory =====
class SurvivalHunterStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    SurvivalHunterStrategyActionNodeFactory()
    {
        creators["auto shot"] = &auto_shot;
        creators["kill command"] = &kill_command;
        creators["kill shot"] = &kill_shot;
        creators["explosive shot"] = &explosive_shot;
        creators["black arrow"] = &black_arrow;
        creators["viper sting"] = &viper_sting;
        creators["serpent sting"] = serpent_sting;
        creators["aimed shot"] = &aimed_shot;
        creators["arcane shot"] = &arcane_shot;
        creators["steady shot"] = &steady_shot;
        creators["multi-shot"] = &multi_shot;
        creators["volley"] = &volley;
    }

private:
    static ActionNode* auto_shot(PlayerbotAI*) { return new ActionNode("auto shot", {}, {}, {}); }
    static ActionNode* kill_command(PlayerbotAI*) { return new ActionNode("kill command", {}, {}, {}); }
    static ActionNode* kill_shot(PlayerbotAI*) { return new ActionNode("kill shot", {}, {}, {}); }
    static ActionNode* explosive_shot(PlayerbotAI*) { return new ActionNode("explosive shot", {}, {}, {}); }
    static ActionNode* black_arrow(PlayerbotAI*) { return new ActionNode("black arrow", {}, {}, {}); }
    static ActionNode* viper_sting(PlayerbotAI*) { return new ActionNode("viper sting", {}, {}, {}); }
    static ActionNode* serpent_sting(PlayerbotAI*) { return new ActionNode("serpent sting", {}, {}, {}); }
    static ActionNode* aimed_shot(PlayerbotAI*) { return new ActionNode("aimed shot", {}, {}, {}); }
    static ActionNode* arcane_shot(PlayerbotAI*) { return new ActionNode("arcane shot", {}, {}, {}); }
    static ActionNode* steady_shot(PlayerbotAI*) { return new ActionNode("steady shot", {}, {}, {}); }
    static ActionNode* multi_shot(PlayerbotAI*) { return new ActionNode("multi shot", {}, {}, {}); }
    static ActionNode* volley(PlayerbotAI*) { return new ActionNode("volley", {}, {}, {}); }
};

// ===== Single Target Strategy =====
SurvivalHunterStrategy::SurvivalHunterStrategy(PlayerbotAI* botAI) : GenericHunterStrategy(botAI)
{
    actionNodeFactories.Add(new SurvivalHunterStrategyActionNodeFactory());
}

// ===== Default Actions =====
std::vector<NextAction*> SurvivalHunterStrategy::getDefaultActions()
{
    return {
        new NextAction("kill command", 5.9f),
        new NextAction("kill shot", 5.8f),
        new NextAction("explosive shot", 5.7f),
        new NextAction("black arrow", 5.6f),
        new NextAction("serpent sting", 5.5f),
        new NextAction("aimed shot", 5.4f),
        new NextAction("arcane shot", 5.3f),
        new NextAction("steady shot", 5.2f),
        new NextAction("auto shot", 5.1f)
    };
}

// ===== Trigger Initialization ===
void SurvivalHunterStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericHunterStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode("lock and load", { new NextAction("explosive shot rank 4", 28.0f) }));
    triggers.push_back(new TriggerNode("lock and load", { new NextAction("explosive shot rank 3", 27.5f) }));
    triggers.push_back(new TriggerNode("lock and load", { new NextAction("explosive shot rank 2", 27.0f) }));
    triggers.push_back(new TriggerNode("lock and load", { new NextAction("explosive shot rank 1", 26.5f) }));

    triggers.push_back(new TriggerNode("kill command", { new NextAction("kill command", 18.5f) }));
    triggers.push_back(new TriggerNode("target critical health", { new NextAction("kill shot", 18.0f) }));
    triggers.push_back(new TriggerNode("explosive shot", { new NextAction("explosive shot", 17.5f) }));
    triggers.push_back(new TriggerNode("black arrow", { new NextAction("black arrow", 16.5f) }));
    triggers.push_back(new TriggerNode("low mana", { new NextAction("viper sting", 16.0f) }));
    triggers.push_back(new TriggerNode("no stings", { new NextAction("serpent sting", 15.5f) }));
    triggers.push_back(new TriggerNode("serpent sting on attacker", { new NextAction("serpent sting on attacker", 15.0f) }));
}
