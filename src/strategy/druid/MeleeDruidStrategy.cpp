/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "MeleeDruidStrategy.h"

#include "Playerbots.h"

MeleeDruidStrategy::MeleeDruidStrategy(PlayerbotAI* botAI) : CombatStrategy(botAI) {}

std::vector<NextAction> MeleeDruidStrategy::getDefaultActions()
{
    return {
        NextAction("faerie fire", ACTION_DEFAULT + 0.1f),
        NextAction("melee", ACTION_DEFAULT)
    };
}

void MeleeDruidStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "omen of clarity",
            {
                NextAction("omen of clarity", ACTION_HIGH + 9)
            }
        )
    );

    CombatStrategy::InitTriggers(triggers);
}
