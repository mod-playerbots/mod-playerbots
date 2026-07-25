/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_HOSACTIONS_H
#define PLAYERBOTS_HOSACTIONS_H

#include "Action.h"
#include "AttackAction.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "HoSTriggers.h"

class ShatterSpreadAction : public MovementAction
{
public:
    ShatterSpreadAction(PlayerbotAI* ai) : MovementAction(ai, "shatter spread") {}
    bool Execute(Event event) override;
};

class AvoidLightningRingAction : public MovementAction
{
public:
    AvoidLightningRingAction(PlayerbotAI* ai) : MovementAction(ai, "avoid lightning ring") {}
    bool Execute(Event event) override;
};

#endif
