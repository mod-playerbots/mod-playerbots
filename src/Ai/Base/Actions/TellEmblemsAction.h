/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_TELLEMBLEMSACTION_H
#define _PLAYERBOT_TELLEMBLEMSACTION_H

#include "InventoryAction.h"

class PlayerbotAI;

class TellEmblemsAction : public InventoryAction
{
public:
    TellEmblemsAction(PlayerbotAI* botAI) : InventoryAction(botAI, "emblems") {}

    bool Execute(Event event) override;
};

#endif
