/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_ACTRIGGERS_H
#define _PLAYERBOT_ACTRIGGERS_H

#include "Trigger.h"
#include "GenericTriggers.h"
#include "DungeonStrategyUtils.h"

enum class AuchenaiCryptsIDs : uint32
{
    // Shirrak The Dead Watcher
    NPC_FOCUS_FIRE                  = 18374,
};

class ShirrakTankPositionBossTrigger : public Trigger
{
public:
    ShirrakTankPositionBossTrigger(PlayerbotAI* botAI) : Trigger(botAI, "shirrak tank position boss") {}

    bool IsActive() override;
};

class ShirrakFleeFocusFireTrigger : public Trigger
{
public:
    ShirrakFleeFocusFireTrigger(PlayerbotAI* botAI) : Trigger(botAI, "shirrak flee focus fire") {}

    bool IsActive() override;
};

class ShirrakRangedKeepDistanceTrigger : public Trigger
{
public:
    ShirrakRangedKeepDistanceTrigger(PlayerbotAI* botAI) : Trigger(botAI, "shirrak ranged keep distance") {}

    bool IsActive() override;
};

#endif
