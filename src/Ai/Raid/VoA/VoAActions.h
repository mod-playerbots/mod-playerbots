/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_VOAACTIONS_H
#define PLAYERBOTS_VOAACTIONS_H

#include "Action.h"
#include "AttackAction.h"
#include "Event.h"
#include "MovementActions.h"
#include "PlayerbotAI.h"

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

//
// Toravon the Ice Watcher
//

// DPS/healer only: burn the ice orb and mark it with the skull so the whole
// group focuses it. Tanks are handled by the tank target exclusion - they must
// not move, or the boss turns and its breath sweeps the raid.
class VoAToravonFrozenOrbAction : public AttackAction
{
public:
    VoAToravonFrozenOrbAction(PlayerbotAI* botAI) : AttackAction(botAI, "voa toravon frozen orb action") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

#endif
