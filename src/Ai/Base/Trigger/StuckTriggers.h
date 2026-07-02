/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_STUCKTRIGGERS_H
#define PLAYERBOTS_STUCKTRIGGERS_H

#include "Trigger.h"

class MoveStuckTrigger : public Trigger
{
public:
    MoveStuckTrigger(PlayerbotAI* botAI) : Trigger(botAI, "move stuck", 5) {}

    bool IsActive() override;
};

class MoveLongStuckTrigger : public Trigger
{
public:
    MoveLongStuckTrigger(PlayerbotAI* botAI) : Trigger(botAI, "move long stuck", 5) {}

    bool IsActive() override;
};

class CombatStuckTrigger : public Trigger
{
public:
    CombatStuckTrigger(PlayerbotAI* botAI) : Trigger(botAI, "combat stuck", 5) {}

    bool IsActive() override;
};

class CombatLongStuckTrigger : public Trigger
{
public:
    CombatLongStuckTrigger(PlayerbotAI* botAI) : Trigger(botAI, "combat long stuck", 5) {}

    bool IsActive() override;
};

#endif
