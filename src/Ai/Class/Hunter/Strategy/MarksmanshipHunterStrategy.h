/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_MARKSMANSHIPHUNTERSTRATEGY_H
#define PLAYERBOTS_MARKSMANSHIPHUNTERSTRATEGY_H

#include "GenericHunterStrategy.h"
#include "CombatStrategy.h"

class PlayerbotAI;

class MarksmanshipHunterStrategy : public GenericHunterStrategy
{
public:
    MarksmanshipHunterStrategy(PlayerbotAI* botAI);

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "mm"; }
    std::vector<NextAction> getDefaultActions() override;
};

#endif
