/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RITUALOFSOULSACTIONS_H
#define PLAYERBOTS_RITUALOFSOULSACTIONS_H

#include "MovementActions.h"

class PlayerbotAI;

class JoinRitualOfSoulsAction : public MovementAction
{
public:
    JoinRitualOfSoulsAction(PlayerbotAI* botAI) : MovementAction(botAI, "join ritual of souls") {}
    bool Execute(Event event) override;
};

class UseSoulwellAction : public MovementAction
{
public:
    UseSoulwellAction(PlayerbotAI* botAI) : MovementAction(botAI, "use soulwell") {}
    bool isUseful() override;
    bool Execute(Event event) override;
};

#endif
