/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_COMBATDELAYSTRATEGY_H
#define _PLAYERBOT_COMBATDELAYSTRATEGY_H

#include "Strategy.h"

class PlayerbotAI;

class CombatDelayStrategy : public Strategy
{
public:
    CombatDelayStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    std::string const getName() override { return "combat delay"; }
};

#endif
