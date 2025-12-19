/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "GenericPaladinNonCombatStrategy.h"

#include "GenericPaladinStrategyActionNodeFactory.h"
#include "Playerbots.h"
#include "AiFactory.h"

GenericPaladinNonCombatStrategy::GenericPaladinNonCombatStrategy(PlayerbotAI* botAI) : NonCombatStrategy(botAI)
{
    actionNodeFactories.Add(new GenericPaladinStrategyActionNodeFactory());
}

void GenericPaladinNonCombatStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    NonCombatStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode("party member dead", { NextAction("redemption", ACTION_CRITICAL_HEAL + 10) }));
    triggers.push_back(new TriggerNode("party member almost full health", { NextAction("flash of light on party", 25.0f) }));
    triggers.push_back(new TriggerNode("party member medium health", { NextAction("flash of light on party", 26.0f) }));
    triggers.push_back(new TriggerNode("party member low health", { NextAction("holy light on party", 27.0f) }));
    triggers.push_back(new TriggerNode("party member critical health", { NextAction("holy light on party", 28.0f) }));

    int specTab = AiFactory::GetPlayerSpecTab(botAI->GetBot());
    if (specTab == 0 || specTab == 1) // Holy or Protection
        triggers.push_back(new TriggerNode("often", { NextAction("apply oil", 1.0f) }));
    if (specTab == 2) // Retribution
        triggers.push_back(new TriggerNode("often", { NextAction("apply stone", 1.0f) }));
}
