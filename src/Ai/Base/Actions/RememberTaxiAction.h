/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef PLAYERBOTS_REMEMBERTAXIACTION_H
#define PLAYERBOTS_REMEMBERTAXIACTION_H

#include "Action.h"

class PlayerbotAI;

class RememberTaxiAction : public Action
{
public:
    RememberTaxiAction(PlayerbotAI* botAI) : Action(botAI, "remember taxi") {}

    bool Execute(Event event) override;
};

#endif
