/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RITUALOFSOULSACTIONS_H
#define PLAYERBOTS_RITUALOFSOULSACTIONS_H

#include "MovementActions.h"

class PlayerbotAI;
class GameObject;

class UseRitualOfSoulsObjectAction : public MovementAction
{
public:
    UseRitualOfSoulsObjectAction(PlayerbotAI* botAI, std::string const name) : MovementAction(botAI, name) {}

protected:
    bool MoveToAndUse(GameObject* go);
};

class JoinRitualOfSoulsAction : public UseRitualOfSoulsObjectAction
{
public:
    JoinRitualOfSoulsAction(PlayerbotAI* botAI) : UseRitualOfSoulsObjectAction(botAI, "join ritual of souls") {}
    bool Execute(Event event) override;
};

class UseSoulwellAction : public UseRitualOfSoulsObjectAction
{
public:
    UseSoulwellAction(PlayerbotAI* botAI) : UseRitualOfSoulsObjectAction(botAI, "use soulwell") {}
    bool isUseful() override;
    bool Execute(Event event) override;
};

#endif
