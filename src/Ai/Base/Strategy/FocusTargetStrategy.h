/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_FOCUSTARGETSTRATEGY_H
#define _PLAYERBOT_FOCUSTARGETSTRATEGY_H

#include "Strategy.h"

class PlayerbotAI;

class FocusHealTargetsStrategy : public Strategy
{
public:
    FocusHealTargetsStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}
    std::string const getName() override { return "focus heal targets"; }
};

#endif
