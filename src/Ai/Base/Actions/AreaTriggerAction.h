/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_AREATRIGGERACTION_H
#define PLAYERBOTS_AREATRIGGERACTION_H

#include "MovementActions.h"

class PlayerbotAI;

class ReachAreaTriggerAction : public MovementAction
{
public:
    ReachAreaTriggerAction(PlayerbotAI* botAI) : MovementAction(botAI, "reach area trigger") {}

    bool Execute(Event event) override;
};

class AreaTriggerAction : public MovementAction
{
public:
    AreaTriggerAction(PlayerbotAI* botAI) : MovementAction(botAI, "area trigger") {}

    bool Execute(Event event) override;
};

#endif
