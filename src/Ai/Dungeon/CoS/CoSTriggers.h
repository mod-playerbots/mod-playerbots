/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_COSTRIGGERS_H
#define _PLAYERBOT_COSTRIGGERS_H

#include "Trigger.h"
#include "PlayerbotAIConfig.h"
#include "GenericTriggers.h"
#include "DungeonStrategyUtils.h"

enum CullingOfStratholmeIDs
{
    // Salramm the Fleshcrafter
    NPC_GHOUL_MINION                   = 27733,
};

class ExplodeGhoulTrigger : public Trigger
{
public:
    ExplodeGhoulTrigger(PlayerbotAI* ai) : Trigger(ai, "explode ghoul") {}
    bool IsActive() override;
};

class EpochRangedTrigger : public Trigger
{
public:
    EpochRangedTrigger(PlayerbotAI* ai) : Trigger(ai, "chrono-lord epoch ranged") {}
    bool IsActive() override;
};

#endif
