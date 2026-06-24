/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef PLAYERBOTS_RESETINSTANCESACTION_H
#define PLAYERBOTS_RESETINSTANCESACTION_H

#include "Action.h"

class PlayerbotAI;

class ResetInstancesAction : public Action
{
public:
    ResetInstancesAction(PlayerbotAI* botAI) : Action(botAI, "reset instances") {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

#endif
