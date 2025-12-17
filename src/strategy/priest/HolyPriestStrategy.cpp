/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "HolyPriestStrategy.h"

#include "Playerbots.h"

class HolyPriestStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    HolyPriestStrategyActionNodeFactory() { creators["smite"] = &smite; }

private:
    static ActionNode* smite([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("smite",
                              /*P*/ {},
                              /*A*/ { new NextAction("shoot") },
                              /*C*/ {});
    }
};

HolyPriestStrategy::HolyPriestStrategy(PlayerbotAI* botAI) : HealPriestStrategy(botAI)
{
    actionNodeFactories.Add(new HolyPriestStrategyActionNodeFactory());
}

std::vector<NextAction*> HolyPriestStrategy::getDefaultActions()
{
    return { new NextAction("smite", ACTION_DEFAULT + 0.2f),
                             new NextAction("mana burn", ACTION_DEFAULT + 0.1f),
                             new NextAction("starshards", ACTION_DEFAULT) };
}

void HolyPriestStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    HealPriestStrategy::InitTriggers(triggers);

    triggers.push_back(
        new TriggerNode("holy fire", { new NextAction("holy fire", ACTION_NORMAL + 9) }));
    triggers.push_back(
        new TriggerNode("shadowfiend", { new NextAction("shadowfiend", ACTION_HIGH) }));
    triggers.push_back(
        new TriggerNode("medium mana", { new NextAction("shadowfiend", ACTION_HIGH) }));
    triggers.push_back(
        new TriggerNode("low mana", { new NextAction("mana burn", ACTION_HIGH) }));
}

HolyHealPriestStrategy::HolyHealPriestStrategy(PlayerbotAI* botAI) : GenericPriestStrategy(botAI)
{
    actionNodeFactories.Add(new GenericPriestStrategyActionNodeFactory());
}

std::vector<NextAction*> HolyHealPriestStrategy::getDefaultActions()
{
    return { new NextAction("shoot", ACTION_DEFAULT) };
}

void HolyHealPriestStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericPriestStrategy::InitTriggers(triggers);

    triggers.push_back(
        new TriggerNode("group heal setting",
                        {
                            new NextAction("prayer of mending on party", ACTION_MEDIUM_HEAL + 9),
                            new NextAction("circle of healing on party", ACTION_MEDIUM_HEAL + 8) }));

    triggers.push_back(new TriggerNode(
        "medium group heal setting",
        { new NextAction("divine hymn", ACTION_CRITICAL_HEAL + 7),
                          new NextAction("prayer of mending on party", ACTION_CRITICAL_HEAL + 6),
                          new NextAction("circle of healing on party", ACTION_CRITICAL_HEAL + 5),
                          new NextAction("prayer of healing on party", ACTION_CRITICAL_HEAL + 4) }));

    triggers.push_back(new TriggerNode(
        "party member critical health",
        {
                          new NextAction("guardian spirit on party", ACTION_CRITICAL_HEAL + 6),
                          new NextAction("power word: shield on party", ACTION_CRITICAL_HEAL + 5),
                          new NextAction("prayer of mending on party", ACTION_CRITICAL_HEAL + 3),
                          new NextAction("greater heal on party", ACTION_MEDIUM_HEAL + 2),
                          new NextAction("flash heal on party", ACTION_CRITICAL_HEAL + 1),
}));

    triggers.push_back(
        new TriggerNode("party member low health",
                        { new NextAction("circle of healing on party", ACTION_MEDIUM_HEAL + 4),
                                          new NextAction("prayer of mending on party", ACTION_MEDIUM_HEAL + 3),
                                          new NextAction("greater heal on party", ACTION_MEDIUM_HEAL + 2),
                                          new NextAction("flash heal on party", ACTION_MEDIUM_HEAL + 1) }));

    triggers.push_back(
        new TriggerNode("party member medium health",
                        { new NextAction("circle of healing on party", ACTION_LIGHT_HEAL + 7),
                                          new NextAction("prayer of mending on party", ACTION_LIGHT_HEAL + 6),
                                          new NextAction("greater heal on party", ACTION_MEDIUM_HEAL + 5),
                                          new NextAction("flash heal on party", ACTION_LIGHT_HEAL + 4),
                                          // new NextAction("renew on party", ACTION_LIGHT_HEAL + 8),
}));

    triggers.push_back(
        new TriggerNode("party member almost full health",
                        {
                                          new NextAction("renew on party", ACTION_LIGHT_HEAL + 2),
                                          new NextAction("prayer of mending on party", ACTION_LIGHT_HEAL + 1),
}));

    triggers.push_back(new TriggerNode(
        "party member to heal out of spell range",
        { new NextAction("reach party member to heal", ACTION_CRITICAL_HEAL + 10) }));
}
