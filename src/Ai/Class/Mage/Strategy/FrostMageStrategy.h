/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_FROSTMAGESTRATEGY_H
#define PLAYERBOTS_FROSTMAGESTRATEGY_H

#include "GenericMageStrategy.h"

class PlayerbotAI;

class FrostMageStrategy : public GenericMageStrategy
{
public:
    FrostMageStrategy(PlayerbotAI* botAI);

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "frost"; }
    std::vector<NextAction> getDefaultActions() override;
};

#endif
