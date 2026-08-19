/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_GATHERINGLEVELINGSTRATEGY_H
#define PLAYERBOTS_GATHERINGLEVELINGSTRATEGY_H

#include "Strategy.h"

class PlayerbotAI;

// "gather leveling" (non-combat): levels the bot's collecting profession
// (mining / herbalism / skinning). Reuses the existing 'gather' strategy to
// harvest nodes and adds the management layer (trainer visits, zone roaming,
// 30-minute / max-level time box). Active whenever the bot has a collecting
// profession below max level.
class GatheringLevelingStrategy : public Strategy
{
public:
    GatheringLevelingStrategy(PlayerbotAI* botAI);

    std::string const getName() override { return "gather leveling"; }
    std::vector<NextAction> getDefaultActions() override;
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

#endif
