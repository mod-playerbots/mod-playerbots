/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "ShadowPriestStrategy.h"

#include "Playerbots.h"
#include "ShadowPriestStrategyActionNodeFactory.h"

ShadowPriestStrategy::ShadowPriestStrategy(PlayerbotAI* botAI) : GenericPriestStrategy(botAI)
{
    actionNodeFactories.Add(new ShadowPriestStrategyActionNodeFactory());
}

std::vector<NextAction> ShadowPriestStrategy::getDefaultActions()
{
    return {
        NextAction("mind blast", ACTION_DEFAULT + 0.3f),
        NextAction("mind flay", ACTION_DEFAULT + 0.2f),
        NextAction("shadow word: death", ACTION_DEFAULT + 0.1f), // cast during movement
        NextAction("shoot", ACTION_DEFAULT)
    };
}

void ShadowPriestStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericPriestStrategy::InitTriggers(triggers);

    triggers.push_back(
        new TriggerNode(
            "shadowform",
            {
                NextAction("shadowform", ACTION_HIGH)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "low mana",
            {
                NextAction("dispersion", ACTION_HIGH + 5)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "critical health",
            {
                NextAction("dispersion", ACTION_HIGH + 5)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "vampiric embrace",
            {
                NextAction("vampiric embrace", 16.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "silence",
            {
                NextAction("silence", ACTION_INTERRUPT + 1)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "silence on enemy healer",
            {
                NextAction("silence on enemy healer", ACTION_INTERRUPT)
            }
        )
    );
}

void ShadowPriestAoeStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "shadow word: pain on attacker",
            {
                NextAction("shadow word: pain on attacker", ACTION_NORMAL + 5)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "vampiric touch on attacker",
            {
                NextAction("vampiric touch on attacker", ACTION_NORMAL + 4)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "mind sear channel check",
            {
                NextAction("cancel channel", ACTION_HIGH + 5)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "medium aoe",
            {
                NextAction("mind sear", ACTION_HIGH + 4)
            }
        )
    );
}

void ShadowPriestDebuffStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "vampiric touch",
            {
                NextAction("vampiric touch", ACTION_HIGH + 3)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "devouring plague",
            {
                NextAction("devouring plague", ACTION_HIGH + 2)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "shadow word: pain",
            {
                NextAction("shadow word: pain", ACTION_HIGH + 1)
            }
        )
    );
}
