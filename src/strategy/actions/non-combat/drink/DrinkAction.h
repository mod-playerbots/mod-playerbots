/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_DRINK_ACTION_H
#define _PLAYERBOT_DRINK_ACTION_H

#include "UseItemAction.h"

class PlayerbotAI;

class DrinkAction : public UseItemAction
{
public:
    DrinkAction(PlayerbotAI* botAI) : UseItemAction(botAI, "drink") {}

    bool Execute(Event event) override;
    bool isUseful() override;
    bool isPossible() override;
};

#endif
