/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_AQ20ACTIONS_H
#define _PLAYERBOT_AQ20ACTIONS_H

#include "MovementActions.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"

class Aq20UseCrystalAction : public MovementAction
{
public:
    Aq20UseCrystalAction(PlayerbotAI* botAI, std::string const name = "aq20 use crystal") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};
#endif
