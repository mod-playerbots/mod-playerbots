/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_PETSACTION_H
#define _PLAYERBOT_PETSACTION_H

#include <string>

#include "Action.h"
#include "PlayerbotFactory.h"
#include "Unit.h"

class PlayerbotAI;

class PetsAction : public Action
{
public:
    PetsAction(PlayerbotAI* botAI, const std::string& defaultCmd = "") : Action(botAI, "pet"), defaultCmd(defaultCmd) {}

    bool Execute(Event event) override;

private:
    bool warningEnabled = true;
    std::string defaultCmd;
};

#endif
