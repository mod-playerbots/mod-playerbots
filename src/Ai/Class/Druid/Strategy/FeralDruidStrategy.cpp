/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "FeralDruidStrategy.h"

#include "Playerbots.h"

class FeralDruidStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    FeralDruidStrategyActionNodeFactory()
    {
        creators["survival instincts"] = &survival_instincts;
        creators["thorns"] = &thorns;
        creators["cure poison"] = &cure_poison;
        creators["cure poison on party"] = &cure_poison_on_party;
        creators["abolish poison"] = &abolish_poison;
        creators["abolish poison on party"] = &abolish_poison_on_party;
    }

private:
    static ActionNode* survival_instincts([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("survival instincts",
                              /*P*/ {},
                              /*A*/ { NextAction("barkskin") },
                              /*C*/ {});
    }

    static ActionNode* thorns([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("thorns",
                              /*P*/ { NextAction("caster form") },
                              /*A*/ {},
                              /*C*/ {});
    }

    static ActionNode* cure_poison([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("cure poison",
                              /*P*/ { NextAction("caster form") },
                              /*A*/ {},
                              /*C*/ {});
    }

    static ActionNode* cure_poison_on_party([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("cure poison on party",
                              /*P*/ { NextAction("caster form") },
                              /*A*/ {},
                              /*C*/ {});
    }

    static ActionNode* abolish_poison([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("abolish poison",
                              /*P*/ { NextAction("caster form") },
                              /*A*/ {},
                              /*C*/ {});
    }

    static ActionNode* abolish_poison_on_party([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("abolish poison on party",
                              /*P*/ { NextAction("caster form") },
                              /*A*/ {},
                              /*C*/ {});
    }

};

FeralDruidStrategy::FeralDruidStrategy(PlayerbotAI* botAI) : GenericDruidStrategy(botAI)
{
    actionNodeFactories.Add(new FeralDruidStrategyActionNodeFactory());
    actionNodeFactories.Add(new ShapeshiftDruidStrategyActionNodeFactory());
}

void FeralDruidStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericDruidStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "enemy out of melee", { NextAction("reach melee", 21.0f) }));
    triggers.push_back(new TriggerNode(
        "low health", { NextAction("survival instincts", 91.0f) }));
    triggers.push_back(new TriggerNode("player has flag",
                                       { NextAction("dash", 92.0f) }));
    triggers.push_back(new TriggerNode("enemy flagcarrier near",
                                       { NextAction("dash", 92.0f) }));
}

void FeralChargeDruidStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    Player* bot = botAI->GetBot();

    if (bot->HasSpell(SPELL_CAT_FORM) && !bot->HasAura(AURA_THICK_HIDE))
        triggers.push_back(new TriggerNode(
            "enemy out of melee", { NextAction("feral charge - cat", 29.0f) }));
    else
        triggers.push_back(new TriggerNode(
            "enemy out of melee", { NextAction("feral charge - bear", 18.0f) }));
}
