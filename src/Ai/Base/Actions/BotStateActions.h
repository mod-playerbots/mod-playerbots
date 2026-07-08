/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_BOTSTATEACTIONS_H
#define _PLAYERBOT_BOTSTATEACTIONS_H

#include "Action.h"

class PlayerbotAI;

class SetCombatStateAction : public Action
{
public:
    SetCombatStateAction(PlayerbotAI* botAI) : Action(botAI, "set combat state") {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

#endif
