/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_AKACTIONS_H
#define _PLAYERBOT_AKACTIONS_H

#include "Action.h"
#include "AttackAction.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "AKTriggers.h"

class AttackNadoxGuardianAction : public AttackAction
{
public:
    AttackNadoxGuardianAction(PlayerbotAI* ai) : AttackAction(ai, "attack nadox guardian") {}
    bool Execute(Event event) override;
};

class AttackJedogaVolunteerAction : public AttackAction
{
public:
    AttackJedogaVolunteerAction(PlayerbotAI* ai) : AttackAction(ai, "attack jedoga volunteer") {}
    bool Execute(Event event) override;
};

class AvoidShadowCrashAction : public MovementAction
{
public:
    AvoidShadowCrashAction(PlayerbotAI* ai) : MovementAction(ai, "avoid shadow crash") {}
    bool Execute(Event event) override;
};

#endif
