/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "PriestNonCombatStrategy.h"

#include "Playerbots.h"
#include "PriestNonCombatStrategyActionNodeFactory.h"

PriestNonCombatStrategy::PriestNonCombatStrategy(PlayerbotAI* botAI) : NonCombatStrategy(botAI)
{
    actionNodeFactories.Add(new PriestNonCombatStrategyActionNodeFactory());
}

void PriestNonCombatStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    NonCombatStrategy::InitTriggers(triggers);

    triggers.push_back(
        new TriggerNode("inner fire",{ new NextAction("inner fire", 10.0f) }));
    triggers.push_back(new TriggerNode(
        "party member dead",{ new NextAction("remove shadowform", ACTION_CRITICAL_HEAL + 11),
                                               new NextAction("resurrection", ACTION_CRITICAL_HEAL + 10) }));
    triggers.push_back(new TriggerNode("often",{ new NextAction("apply oil", 1.0f) }));
    triggers.push_back(
        new TriggerNode("party member critical health",
                       { new NextAction("renew on party", ACTION_CRITICAL_HEAL + 3),
                                          new NextAction("penance on party", ACTION_CRITICAL_HEAL + 2),
                                          new NextAction("greater heal on party", ACTION_CRITICAL_HEAL + 1) }));

    triggers.push_back(
        new TriggerNode("party member low health",
                       { new NextAction("renew on party", ACTION_MEDIUM_HEAL + 3),
                                          new NextAction("penance on party", ACTION_MEDIUM_HEAL + 2),
                                          new NextAction("greater heal on party", ACTION_MEDIUM_HEAL + 1) }));

    triggers.push_back(
        new TriggerNode("party member medium health",
                       { new NextAction("renew on party", ACTION_LIGHT_HEAL + 9),
                                          new NextAction("penance on party", ACTION_LIGHT_HEAL + 8) }));

    triggers.push_back(
        new TriggerNode("party member almost full health",
                       { new NextAction("renew on party", ACTION_LIGHT_HEAL + 3) }));

    triggers.push_back(
        new TriggerNode("group heal setting",{ new NextAction("circle of healing on party", 27.0f) }));
    triggers.push_back(new TriggerNode("new pet",
                                      { new NextAction("set pet stance", 10.0f) }));
}

void PriestBuffStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    NonCombatStrategy::InitTriggers(triggers);

    triggers.push_back(
        new TriggerNode("prayer of fortitude on party",
                       { new NextAction("prayer of fortitude on party", 12.0f) }));
    triggers.push_back(
        new TriggerNode("prayer of spirit on party",
                       { new NextAction("prayer of spirit on party", 14.0f) }));
    triggers.push_back(
        new TriggerNode("power word: fortitude on party",
                       { new NextAction("power word: fortitude on party", 11.0f) }));
    triggers.push_back(new TriggerNode("divine spirit on party",
                                      { new NextAction("divine spirit on party", 13.0f) }));
}

void PriestShadowResistanceStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    NonCombatStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode("shadow protection",
                                      { new NextAction("shadow protection", 12.0f) }));
    triggers.push_back(
        new TriggerNode("shadow protection on party",
                       { new NextAction("shadow protection on party", 11.0f) }));
}
