/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_REWARDACTION_H
#define PLAYERBOTS_REWARDACTION_H

#include "InventoryAction.h"

class PlayerbotAI;
class Object;

class RewardAction : public InventoryAction
{
public:
    RewardAction(PlayerbotAI* botAI) : InventoryAction(botAI, "reward") {}

    bool Execute(Event event) override;

private:
    bool Reward(uint32 itemId, Object* pNpc);
};

#endif
