/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "HealPriestStrategy.h"

#include "GenericPriestStrategyActionNodeFactory.h"
#include "Playerbots.h"

HealPriestStrategy::HealPriestStrategy(PlayerbotAI* botAI) : GenericPriestStrategy(botAI)
{
    actionNodeFactories.Add(new GenericPriestStrategyActionNodeFactory());
}

std::vector<NextAction*> HealPriestStrategy::getDefaultActions()
{
    return { new NextAction("shoot", ACTION_DEFAULT) };
}

void HealPriestStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericPriestStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "group heal setting",
        {
                          new NextAction("prayer of mending on party", ACTION_MEDIUM_HEAL + 8),
                          new NextAction("power word: shield on not full", ACTION_MEDIUM_HEAL + 7) }));

    triggers.push_back(new TriggerNode(
        "medium group heal setting",
        { new NextAction("divine hymn", ACTION_CRITICAL_HEAL + 7),
                           new NextAction("prayer of mending on party", ACTION_CRITICAL_HEAL + 6),
                           new NextAction("power word: shield on not full", ACTION_CRITICAL_HEAL + 5),
                           new NextAction("prayer of healing on party", ACTION_CRITICAL_HEAL + 4) }));

    triggers.push_back(new TriggerNode(
        "party member critical health",
        { new NextAction("power word: shield on party", ACTION_CRITICAL_HEAL + 5),
                          new NextAction("penance on party", ACTION_CRITICAL_HEAL + 4),
                          new NextAction("prayer of mending on party", ACTION_CRITICAL_HEAL + 3),
                          new NextAction("flash heal on party", ACTION_CRITICAL_HEAL + 2) }));

    triggers.push_back(
        new TriggerNode("party member low health",
                        { new NextAction("power word: shield on party", ACTION_MEDIUM_HEAL + 4),
                                          new NextAction("prayer of mending on party", ACTION_MEDIUM_HEAL + 3),
                                          new NextAction("penance on party", ACTION_MEDIUM_HEAL + 2),
                                          new NextAction("flash heal on party", ACTION_MEDIUM_HEAL + 0) }));

    triggers.push_back(
        new TriggerNode("party member medium health",
                        { new NextAction("power word: shield on party", ACTION_LIGHT_HEAL + 9),
                                          new NextAction("prayer of mending on party", ACTION_LIGHT_HEAL + 7),
                                          new NextAction("penance on party", ACTION_LIGHT_HEAL + 6),
                                          new NextAction("flash heal on party", ACTION_LIGHT_HEAL + 5) }));

    triggers.push_back(
        new TriggerNode("party member almost full health",
                        {
                                          // new NextAction("penance on party", ACTION_LIGHT_HEAL + 3),
                                          new NextAction("prayer of mending on party", ACTION_LIGHT_HEAL + 2),
                                          new NextAction("renew on party", ACTION_LIGHT_HEAL + 1) }));

    triggers.push_back(new TriggerNode(
        "party member to heal out of spell range",
        { new NextAction("reach party member to heal", ACTION_CRITICAL_HEAL + 10) }));

    triggers.push_back(new TriggerNode(
        "critical health", { new NextAction("pain suppression", ACTION_EMERGENCY + 1) }));
    triggers.push_back(
        new TriggerNode("protect party member",
                        { new NextAction("pain suppression on party", ACTION_EMERGENCY) }));
}
