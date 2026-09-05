/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_FIREMAGESTRATEGY_H
#define PLAYERBOTS_FIREMAGESTRATEGY_H

#include "GenericMageStrategy.h"

class PlayerbotAI;

class FireMageStrategy : public GenericMageStrategy
{
public:
    FireMageStrategy(PlayerbotAI* botAI);

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "fire"; }
    std::vector<NextAction> getDefaultActions() override;
};

class FirestarterStrategy : public CombatStrategy
{
public:
    FirestarterStrategy(PlayerbotAI* botAI);

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "firestarter"; }
};

#endif
