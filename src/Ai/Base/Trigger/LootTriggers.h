/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_LOOTTRIGGERS_H
#define PLAYERBOTS_LOOTTRIGGERS_H

#include "Trigger.h"

class PlayerbotAI;

class LootAvailableTrigger : public Trigger
{
public:
    LootAvailableTrigger(PlayerbotAI* botAI) : Trigger(botAI, "loot available") {}

    bool IsActive() override;
};

class FarFromCurrentLootTrigger : public Trigger
{
public:
    FarFromCurrentLootTrigger(PlayerbotAI* botAI) : Trigger(botAI, "far from current loot") {}

    bool IsActive() override;
};

class CanLootTrigger : public Trigger
{
public:
    CanLootTrigger(PlayerbotAI* botAI) : Trigger(botAI, "can loot") {}

    bool IsActive() override;
};

#endif
