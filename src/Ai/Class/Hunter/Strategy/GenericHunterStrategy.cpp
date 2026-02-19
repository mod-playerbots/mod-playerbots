/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "GenericHunterStrategy.h"

class GenericHunterStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    GenericHunterStrategyActionNodeFactory()
    {
        creators["rapid fire"] = &rapid_fire;
        creators["boost"] = &rapid_fire;
        creators["aspect of the pack"] = &aspect_of_the_pack;
        creators["aspect of the dragonhawk"] = &aspect_of_the_dragonhawk;
        creators["feign death"] = &feign_death;
        creators["wing clip"] = &wing_clip;
        creators["mongoose bite"] = &mongoose_bite;
        creators["raptor strike"] = &raptor_strike;
        creators["explosive trap"] = &explosive_trap;
    }

private:
    static ActionNode* rapid_fire([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("rapid fire",
                              /*P*/ {},
                              /*A*/ { NextAction("readiness") },
                              /*C*/ {});
    }

    static ActionNode* aspect_of_the_pack([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("aspect of the pack",
                              /*P*/ {},
                              /*A*/ { NextAction("aspect of the cheetah") },
                              /*C*/ {});
    }

    static ActionNode* aspect_of_the_dragonhawk([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("aspect of the dragonhawk",
                              /*P*/ {},
                              /*A*/ { NextAction("aspect of the hawk") },
                              /*C*/ {});
    }

    static ActionNode* feign_death([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("feign death",
                              /*P*/ {},
                              /*A*/ {},
                              /*C*/ {});
    }

    static ActionNode* wing_clip([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("wing clip",
                              /*P*/ {},
                              // /*A*/ { NextAction("mongoose bite") },
                              {},
                              /*C*/ {});
    }

    static ActionNode* mongoose_bite([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("mongoose bite",
                              /*P*/ {},
                              /*A*/ { NextAction("raptor strike") },
                              /*C*/ {});
    }

    static ActionNode* raptor_strike([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("raptor strike",
                              /*P*/ { NextAction("melee") },
                              /*A*/ {},
                              /*C*/ {});
    }

    static ActionNode* explosive_trap([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("explosive trap",
                              /*P*/ {},
                              /*A*/ { NextAction("immolation trap") },
                              /*C*/ {});
    }
};

GenericHunterStrategy::GenericHunterStrategy(PlayerbotAI* botAI) : CombatStrategy(botAI)
{
    actionNodeFactories.Add(new GenericHunterStrategyActionNodeFactory());
}

void GenericHunterStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    CombatStrategy::InitTriggers(triggers);

    // Mark/Ammo/Mana Triggers
    triggers.push_back(new TriggerNode("no ammo", { NextAction("equip upgrades packet action", 30.0f) }));
    triggers.push_back(new TriggerNode("hunter's mark", { NextAction("hunter's mark", 29.5f) }));
    triggers.push_back(new TriggerNode("rapid fire", { NextAction("rapid fire", 29.0f) }));
    triggers.push_back(new TriggerNode("aspect of the viper", { NextAction("aspect of the viper", 28.0f) }));
    triggers.push_back(new TriggerNode("aspect of the hawk", { NextAction("aspect of the dragonhawk", 27.5f) }));

    // Aggro/Threat/Defensive Triggers
    triggers.push_back(new TriggerNode("has aggro", { NextAction("concussive shot", 20.0f) }));
    triggers.push_back(new TriggerNode("low tank threat", { NextAction("misdirection on main tank", 27.0f) }));
    triggers.push_back(new TriggerNode("low health", { NextAction("deterrence", 35.0f) }));
    triggers.push_back(new TriggerNode("concussive shot on snare target", { NextAction("concussive shot", 20.0f) }));
    triggers.push_back(new TriggerNode("medium threat", { NextAction("feign death", 35.0f) }));
    triggers.push_back(new TriggerNode("hunters pet medium health", { NextAction("mend pet", 22.0f) }));
    triggers.push_back(new TriggerNode("hunters pet low health", { NextAction("mend pet", 21.0f) }));

    // Dispel Triggers
    triggers.push_back(new TriggerNode("tranquilizing shot enrage", { NextAction("tranquilizing shot", 61.0f) }));
    triggers.push_back(new TriggerNode("tranquilizing shot magic", { NextAction("tranquilizing shot", 61.0f) }));

    // Ranged-based Triggers
    triggers.push_back(new TriggerNode("enemy within melee", {
                                                         NextAction("explosive trap", 37.0f),
                                                         NextAction("mongoose bite", 22.0f),
                                                         NextAction("wing clip", 21.0f) }));

    triggers.push_back(new TriggerNode("enemy too close for auto shot", {
                                                                    NextAction("disengage", 35.0f),
                                                                    NextAction("flee", 34.0f) }));
}

// ===== AoE Strategy, 2/3+ enemies =====
AoEHunterStrategy::AoEHunterStrategy(PlayerbotAI* botAI) : CombatStrategy(botAI) {}

void AoEHunterStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("volley channel check", { NextAction("cancel channel", 23.0f) }));
    triggers.push_back(new TriggerNode("medium aoe", { NextAction("volley", 22.0f) }));
    triggers.push_back(new TriggerNode("light aoe", { NextAction("multi-shot", 21.0f) }));
}

void HunterBoostStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
}

void HunterCcStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("scare beast", { NextAction("scare beast on cc", 23.0f) }));
    triggers.push_back(new TriggerNode("freezing trap", { NextAction("freezing trap on cc", 23.0f) }));
}

void HunterTrapWeaveStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("immolation trap no cd", { NextAction("reach melee", 23.0f) }));
}
