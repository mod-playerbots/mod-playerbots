/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "DpsPaladinStrategy.h"

#include "Playerbots.h"
#include "Strategy.h"

class DpsPaladinStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    DpsPaladinStrategyActionNodeFactory()
    {
        creators["sanctity aura"] = &sanctity_aura;
        creators["retribution aura"] = &retribution_aura;
        creators["seal of corruption"] = &seal_of_corruption;
        creators["seal of vengeance"] = &seal_of_vengeance;
        creators["seal of command"] = &seal_of_command;
        creators["blessing of might"] = &blessing_of_might;
        creators["crusader strike"] = &crusader_strike;
        creators["repentance"] = &repentance;
        creators["repentance on enemy healer"] = &repentance_on_enemy_healer;
        creators["repentance on snare target"] = &repentance_on_snare_target;
        creators["repentance of shield"] = &repentance_or_shield;
    }

private:
    static ActionNode* seal_of_corruption([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("seal of corruption",
                              /*P*/ {},
                              /*A*/ { new NextAction("seal of vengeance") },
                              /*C*/ {});
    }

    static ActionNode* seal_of_vengeance([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("seal of vengeance",
                              /*P*/ {},
                              /*A*/ { new NextAction("seal of command") },
                              /*C*/ {});
    }

    static ActionNode* seal_of_command([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("seal of command",
                              /*P*/ {},
                              /*A*/ { new NextAction("seal of righteousness") },
                              /*C*/ {});
    }

    static ActionNode* blessing_of_might([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("blessing of might",
                              /*P*/ {},
                              /*A*/ { new NextAction("blessing of kings") },
                              /*C*/ {});
    }

    static ActionNode* crusader_strike([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("crusader strike",
                              /*P*/ {},
                              /*A*/ {},
                              /*C*/ {});
    }

    ACTION_NODE_A(repentance, "repentance", "hammer of justice");
    ACTION_NODE_A(repentance_on_enemy_healer, "repentance on enemy healer", "hammer of justice on enemy healer");
    ACTION_NODE_A(repentance_on_snare_target, "repentance on snare target", "hammer of justice on snare target");
    ACTION_NODE_A(sanctity_aura, "sanctity aura", "retribution aura");
    ACTION_NODE_A(retribution_aura, "retribution aura", "devotion aura");
    ACTION_NODE_A(repentance_or_shield, "repentance", "divine shield");
};

DpsPaladinStrategy::DpsPaladinStrategy(PlayerbotAI* botAI) : GenericPaladinStrategy(botAI)
{
    actionNodeFactories.Add(new DpsPaladinStrategyActionNodeFactory());
}

std::vector<NextAction*> DpsPaladinStrategy::getDefaultActions()
{
    return {
        new NextAction("hammer of wrath", ACTION_DEFAULT + 0.6f),
        new NextAction("judgement of wisdom", ACTION_DEFAULT + 0.5f),
        new NextAction("crusader strike", ACTION_DEFAULT + 0.4f),
        new NextAction("divine storm", ACTION_DEFAULT + 0.3f),
        new NextAction("consecration", ACTION_DEFAULT + 0.1f),
        new NextAction("melee", ACTION_DEFAULT)
    };
}

void DpsPaladinStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericPaladinStrategy::InitTriggers(triggers);

    triggers.push_back(
        new TriggerNode("art of war", { new NextAction("exorcism", ACTION_DEFAULT + 0.2f) }));
    triggers.push_back(
        new TriggerNode("seal", { new NextAction("seal of corruption", ACTION_HIGH) }));
    triggers.push_back(
        new TriggerNode("low mana", { new NextAction("seal of wisdom", ACTION_HIGH + 5) }));

    triggers.push_back(new TriggerNode(
        "avenging wrath", { new NextAction("avenging wrath", ACTION_HIGH + 2) }));
    triggers.push_back(new TriggerNode(
        "medium aoe", {
        new NextAction("divine storm", ACTION_HIGH + 4),
        new NextAction("consecration", ACTION_HIGH + 3)
    }));
    triggers.push_back(new TriggerNode("enemy out of melee",
                                       { new NextAction("reach melee", ACTION_HIGH + 1) }));
}
