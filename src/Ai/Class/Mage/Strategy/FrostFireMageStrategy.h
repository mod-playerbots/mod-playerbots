/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_FROSTFIREMAGESTRATEGY_H
#define PLAYERBOTS_FROSTFIREMAGESTRATEGY_H

#include "GenericMageStrategy.h"

class PlayerbotAI;

class FrostFireMageStrategy : public GenericMageStrategy
{
public:
    FrostFireMageStrategy(PlayerbotAI* botAI);

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "frostfire"; }
    std::vector<NextAction> getDefaultActions() override;
};

#endif
