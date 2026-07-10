/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ARCANEMAGESTRATEGY_H
#define PLAYERBOTS_ARCANEMAGESTRATEGY_H

#include "GenericMageStrategy.h"

class PlayerbotAI;

class ArcaneMageStrategy : public GenericMageStrategy
{
public:
    ArcaneMageStrategy(PlayerbotAI* botAI);

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "arcane"; }
    std::vector<NextAction> getDefaultActions() override;
};

#endif
