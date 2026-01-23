/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#pragma once

#include <string>

#include "Action.h"
#include "PlayerbotFactory.h"
#include "Unit.h"
#include "PetsAction.h"

class PlayerbotAI;

class PetAttackAction : public PetsAction
{
public:
    PetAttackAction(PlayerbotAI* botAI, const std::string& defaultCmd = "") : PetsAction(botAI, "attack") {}

    bool Execute(Event event) override;
};
