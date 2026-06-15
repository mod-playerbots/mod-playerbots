/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_COSACTIONS_H
#define _PLAYERBOT_COSACTIONS_H

#include "Action.h"
#include "AttackAction.h"
#include "GenericSpellActions.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "CoSTriggers.h"

class ExplodeGhoulSpreadAction : public MovementAction
{
public:
    ExplodeGhoulSpreadAction(PlayerbotAI* ai) : MovementAction(ai, "explode ghoul spread") {}
    bool Execute(Event event) override;
};

class EpochStackAction : public MovementAction
{
public:
    EpochStackAction(PlayerbotAI* ai) : MovementAction(ai, "epoch stack") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

#endif
