/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef PLAYERBOTS_BEASTMASTERYHUNTERSTRATEGY_H
#define PLAYERBOTS_BEASTMASTERYHUNTERSTRATEGY_H

#include "GenericHunterStrategy.h"
#include "CombatStrategy.h"

class PlayerbotAI;

class BeastMasteryHunterStrategy : public GenericHunterStrategy
{
public:
    BeastMasteryHunterStrategy(PlayerbotAI* botAI);

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "bm"; }
    std::vector<NextAction> getDefaultActions() override;
};

#endif
