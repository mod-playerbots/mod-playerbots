/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_SETMASTERACTION_H
#define _PLAYERBOT_SETMASTERACTION_H

#include "Action.h"

class PlayerbotAI;

class SetMasterAction : public Action
{
public:
    SetMasterAction(PlayerbotAI* botAI, std::string const name = "set master") : Action(botAI, name) {}

    bool Execute(Event event) override;
    bool isUseful() override;

protected:
 
};

#endif
