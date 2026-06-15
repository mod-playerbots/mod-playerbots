/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_HOSTRIGGERS_H
#define _PLAYERBOT_HOSTRIGGERS_H

#include "Trigger.h"
#include "PlayerbotAIConfig.h"
#include "GenericTriggers.h"
#include "DungeonStrategyUtils.h"

enum HallsOfStoneIDs
{
    // Krystallus
    SPELL_GROUND_SLAM               = 50827,
    DEBUFF_GROUND_SLAM              = 50833,

    // Sjonnir The Ironshaper
    SPELL_LIGHTNING_RING_N          = 50840,
    SPELL_LIGHTNING_RING_H          = 59848,
};

#define SPELL_LIGHTNING_RING        DUNGEON_MODE(bot, SPELL_LIGHTNING_RING_N, SPELL_LIGHTNING_RING_H)

class KrystallusGroundSlamTrigger : public Trigger
{
public:
    KrystallusGroundSlamTrigger(PlayerbotAI* ai) : Trigger(ai, "krystallus ground slam") {}
    bool IsActive() override;
};

class SjonnirLightningRingTrigger : public Trigger
{
public:
    SjonnirLightningRingTrigger(PlayerbotAI* ai) : Trigger(ai, "sjonnir lightning ring") {}
    bool IsActive() override;
};

#endif
