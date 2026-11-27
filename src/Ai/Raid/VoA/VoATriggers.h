/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_VOATRIGGERS_H
#define PLAYERBOTS_VOATRIGGERS_H

#include "GenericTriggers.h"
#include "Trigger.h"

enum VoAIDs
{
    // Emalon the Storm Watcher
    AURA_OVERCHARGE = 64217,
    BOSS_EMALON = 33993,
    NPC_TEMPEST_MINION = 33998,
    SPELL_LIGHTNING_NOVA_10_MAN = 64216,
    SPELL_LIGHTNING_NOVA_25_MAN = 65279,

    // Toravon the Ice Watcher
    BOSS_TORAVON_10 = 38433,
    BOSS_TORAVON_25 = 38462,
    NPC_FROZEN_ORB_10 = 38456,
    NPC_FROZEN_ORB_25 = 38698,
};

//
// Toravon the Ice Watcher
//

// An ice orb is up during the Toravon fight. The orbs must be killed by the
// DPS; the tank has to keep the boss still (see RaidVoAStrategy::
// AppendTargetExclusions, which removes orbs from the tank target value).
class VoAToravonFrozenOrbTrigger : public Trigger
{
public:
    VoAToravonFrozenOrbTrigger(PlayerbotAI* botAI) : Trigger(botAI, "voa toravon frozen orb", 1) {}
    bool IsActive() override;
};

// Frozen Mallet (71993) stacks on the current tank; swap at 3 stacks.
class VoAToravonTankSwapTrigger : public TankSwapTriggerBase
{
public:
    VoAToravonTankSwapTrigger(PlayerbotAI* botAI)
        : TankSwapTriggerBase(botAI, "voa toravon tank swap", 71993, 3, "toravon the ice watcher")
    {
    }
};

//
// Emalon the Storm Watcher
//
class EmalonMarkBossTrigger : public Trigger
{
public:
    EmalonMarkBossTrigger(PlayerbotAI* botAI) : Trigger(botAI, "emalon mark boss trigger") {}
    bool IsActive() override;
};

class EmalonLightingNovaTrigger : public Trigger
{
public:
    EmalonLightingNovaTrigger(PlayerbotAI* botAI) : Trigger(botAI, "emalon lighting nova trigger") {}
    bool IsActive() override;
};

class EmalonOverchargeTrigger : public Trigger
{
public:
    EmalonOverchargeTrigger(PlayerbotAI* botAI) : Trigger(botAI, "emalon overcharge trigger") {}
    bool IsActive() override;
};

class EmalonFallFromFloorTrigger : public Trigger
{
public:
    EmalonFallFromFloorTrigger(PlayerbotAI* botAI) : Trigger(botAI, "emalon fall from floor trigger") {}
    bool IsActive() override;
};

#endif
