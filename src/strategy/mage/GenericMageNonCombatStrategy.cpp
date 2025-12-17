/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "GenericMageNonCombatStrategy.h"
#include "AiFactory.h"
#include "Playerbots.h"

class GenericMageNonCombatStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    GenericMageNonCombatStrategyActionNodeFactory()
    {
        creators["molten armor"] = &molten_armor;
        creators["mage armor"] = &mage_armor;
        creators["ice armor"] = &ice_armor;
    }

private:
    static ActionNode* molten_armor([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("molten armor",
                              /*P*/ {},
                              /*A*/ { new NextAction("mage armor") },
                              /*C*/ {});
    }

    static ActionNode* mage_armor([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("mage armor",
                              /*P*/ {},
                              /*A*/ { new NextAction("ice armor") },
                              /*C*/ {});
    }

    static ActionNode* ice_armor([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("ice armor",
                              /*P*/ {},
                              /*A*/ { new NextAction("frost armor") },
                              /*C*/ {});
    }
};

GenericMageNonCombatStrategy::GenericMageNonCombatStrategy(PlayerbotAI* botAI) : NonCombatStrategy(botAI)
{
    actionNodeFactories.Add(new GenericMageNonCombatStrategyActionNodeFactory());
}

void GenericMageNonCombatStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    NonCombatStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode("arcane intellect", { new NextAction("arcane intellect", 21.0f) }));
    triggers.push_back(new TriggerNode("no focus magic", { new NextAction("focus magic on party", 19.0f) }));
    triggers.push_back(new TriggerNode("often", { new NextAction("apply oil", 1.0f) }));
    triggers.push_back(new TriggerNode("no mana gem", { new NextAction("conjure mana gem", 20.0f) }));
}

void MageBuffManaStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("mage armor", { new NextAction("mage armor", 19.0f) }));
}

void MageBuffDpsStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("mage armor", { new NextAction("molten armor", 19.0f) }));
}

void MageBuffStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("arcane intellect on party", { new NextAction("arcane intellect on party", 20.0f) }));
}
