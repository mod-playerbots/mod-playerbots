/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_TELLPVPSTATSACTION_H
#define _PLAYERBOT_TELLPVPSTATSACTION_H

#include "Action.h"

class PlayerbotAI;

class TellPvpStatsAction : public Action
{
public:
    TellPvpStatsAction(PlayerbotAI* botAI) : Action(botAI, "tell pvp stats") {}

    bool Execute(Event event) override;
};
#endif