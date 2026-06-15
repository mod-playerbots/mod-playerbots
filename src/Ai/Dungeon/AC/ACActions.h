/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_ACACTIONS_H
#define _PLAYERBOT_ACACTIONS_H

#include "AttackAction.h"
#include "MovementActions.h"
#include "ACTriggers.h"

// Shirrak the Dead Watcher

class ShirrakTankPositionBossAction : public AttackAction
{
public:
    ShirrakTankPositionBossAction(PlayerbotAI* botAI) : AttackAction(botAI, "shirrak tank position boss") {}
    bool Execute(Event event) override;
};

class ShirrakFleeFocusFireAction : public MovementAction
{
public:
    ShirrakFleeFocusFireAction(PlayerbotAI* botAI) : MovementAction(botAI, "shirrak flee focus fire") {}
    bool Execute(Event event) override;
};

class ShirrakRangedKeepDistanceAction : public MovementAction
{
public:
    ShirrakRangedKeepDistanceAction(PlayerbotAI* botAI) : MovementAction(botAI, "shirrak ranged keep distance") {}
    bool Execute(Event event) override;
};

#endif
