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
        new TriggerNode("inner fire",{ NextAction("inner fire", 10.0f) }));
    triggers.push_back(new TriggerNode(
        "party member dead",{ NextAction("remove shadowform", ACTION_CRITICAL_HEAL + 11),
                                               NextAction("resurrection", ACTION_CRITICAL_HEAL + 10) }));
    triggers.push_back(new TriggerNode("often",{ NextAction("apply oil", 1.0f) }));
    triggers.push_back(
        new TriggerNode("party member critical health",
                       { NextAction("renew on party", ACTION_CRITICAL_HEAL + 3),
                                          NextAction("penance on party", ACTION_CRITICAL_HEAL + 2),
                                          NextAction("greater heal on party", ACTION_CRITICAL_HEAL + 1) }));

    triggers.push_back(
        new TriggerNode("party member low health",
                       { NextAction("renew on party", ACTION_MEDIUM_HEAL + 3),
                                          NextAction("penance on party", ACTION_MEDIUM_HEAL + 2),
                                          NextAction("greater heal on party", ACTION_MEDIUM_HEAL + 1) }));

    triggers.push_back(
        new TriggerNode("party member medium health",
                       { NextAction("renew on party", ACTION_LIGHT_HEAL + 9),
                                          NextAction("penance on party", ACTION_LIGHT_HEAL + 8) }));

    triggers.push_back(
        new TriggerNode("party member almost full health",
                       { NextAction("renew on party", ACTION_LIGHT_HEAL + 3) }));

    triggers.push_back(
        new TriggerNode("group heal setting",{ NextAction("circle of healing on party", 27.0f) }));
    triggers.push_back(new TriggerNode("new pet",
                                      { NextAction("set pet stance", 10.0f) }));
}

void PriestBuffStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    NonCombatStrategy::InitTriggers(triggers);

    triggers.push_back(
        new TriggerNode("prayer of fortitude on party",
                       { NextAction("prayer of fortitude on party", 12.0f) }));
    triggers.push_back(
        new TriggerNode("prayer of spirit on party",
                       { NextAction("prayer of spirit on party", 14.0f) }));
    triggers.push_back(
        new TriggerNode("power word: fortitude on party",
                       { NextAction("power word: fortitude on party", 11.0f) }));
    triggers.push_back(new TriggerNode("divine spirit on party",
                                      { NextAction("divine spirit on party", 13.0f) }));
}

void PriestShadowResistanceStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    NonCombatStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode("shadow protection",
                                      { NextAction("shadow protection", 12.0f) }));
    triggers.push_back(
        new TriggerNode("shadow protection on party",
                       { NextAction("shadow protection on party", 11.0f) }));
}
