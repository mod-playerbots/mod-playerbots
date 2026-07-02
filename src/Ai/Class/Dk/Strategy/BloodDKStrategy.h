/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_BLOODDKSTRATEGY_H
#define PLAYERBOTS_BLOODDKSTRATEGY_H

#include "GenericDKStrategy.h"

class PlayerbotAI;

class BloodDKStrategy : public GenericDKStrategy
{
public:
    BloodDKStrategy(PlayerbotAI* botAI);

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "blood"; }
    std::vector<NextAction> getDefaultActions() override;
    uint32 GetType() const override { return STRATEGY_TYPE_TANK | STRATEGY_TYPE_MELEE; }
};

#endif
