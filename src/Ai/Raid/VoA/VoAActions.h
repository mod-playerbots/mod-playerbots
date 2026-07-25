/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_VOAACTIONS_H
#define PLAYERBOTS_VOAACTIONS_H

#include "Action.h"
#include "MovementActions.h"
#include "PlayerbotAI.h"
#include "Event.h"

//
//  Emalon the Storm Watcher
//

class EmalonMarkBossAction : public MovementAction
{
public:
    EmalonMarkBossAction(PlayerbotAI* botAI) : MovementAction(botAI, "emalon mark boss action") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class EmalonLightingNovaAction : public MovementAction
{
public:
    EmalonLightingNovaAction(PlayerbotAI* botAI) : MovementAction(botAI, "emalon lighting nova action") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class EmalonOverchargeAction : public Action
{
public:
    EmalonOverchargeAction(PlayerbotAI* botAI) : Action(botAI, "emalon overcharge action") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class EmalonFallFromFloorAction : public Action
{
public:
    EmalonFallFromFloorAction(PlayerbotAI* botAI) : Action(botAI, "emalon fall from floor action") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

#endif
