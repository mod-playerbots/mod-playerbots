/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_HEALPRIESTSTRATEGY_H
#define PLAYERBOTS_HEALPRIESTSTRATEGY_H

#include "GenericPriestStrategy.h"

class PlayerbotAI;

class HealPriestStrategy : public GenericPriestStrategy
{
public:
    HealPriestStrategy(PlayerbotAI* botAI);

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::vector<NextAction> getDefaultActions() override;
    std::string const getName() override { return "heal"; }
    uint32 GetType() const override { return STRATEGY_TYPE_HEAL | STRATEGY_TYPE_RANGED; }
};

#endif
