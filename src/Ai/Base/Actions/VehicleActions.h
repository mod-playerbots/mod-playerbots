/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_VEHICLEACTIONS_H
#define PLAYERBOTS_VEHICLEACTIONS_H

#include "Event.h"
#include "MovementActions.h"

class PlayerbotAI;

class EnterVehicleAction : public MovementAction
{
public:
    EnterVehicleAction(PlayerbotAI* botAI, std::string const& name = "enter vehicle") : MovementAction(botAI, name) {}

    bool Execute(Event event) override;
protected:
    bool EnterVehicle(Unit* vehicleBase, bool moveIfFar);
};

class LeaveVehicleAction : public MovementAction
{
public:
    LeaveVehicleAction(PlayerbotAI* botAI, std::string const& name = "leave vehicle") : MovementAction(botAI, name) {}

    bool Execute(Event event) override;
};

#endif
