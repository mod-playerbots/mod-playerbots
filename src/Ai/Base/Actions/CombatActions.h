/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_COMBATACTIONS_H
#define PLAYERBOTS_COMBATACTIONS_H

#include "ChangeStrategyAction.h"

class PlayerbotAI;

class SwitchToMeleeAction : public ChangeCombatStrategyAction
{
public:
    SwitchToMeleeAction(PlayerbotAI* botAI) : ChangeCombatStrategyAction(botAI, "-ranged,+close") {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

class SwitchToRangedAction : public ChangeCombatStrategyAction
{
public:
    SwitchToRangedAction(PlayerbotAI* botAI) : ChangeCombatStrategyAction(botAI, "-close,+ranged") {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

#endif
