/*
* This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
* information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
* or (at your option) any later version.
*/

#ifndef _PLAYERBOT_TBCDUNGEONHELLFIRERAMPARTSTRIGGERS_H
#define _PLAYERBOT_TBCDUNGEONHELLFIRERAMPARTSTRIGGERS_H

#include "Trigger.h"
#include "GenericTriggers.h"
#include "DungeonStrategyUtils.h"

enum class HellfireRampartsIDs : uint32
{
    // Omor the Unscarred
    SPELL_TREACHEROUS_AURA                = 30695,
    SPELL_BANE_OF_TREACHERY               = 37566,

};

// Watchkeeper Gargolmar

class GargolmarHellfireWatchersAreActiveTrigger : public Trigger
{
public:
    GargolmarHellfireWatchersAreActiveTrigger(PlayerbotAI* botAI) : Trigger(botAI, "gargolmar hellfire watchers are active") {}

    bool IsActive() override;
};

// Omor the Unscarred

class OmorTreacheryAuraTrigger : public Trigger
{
public:
    OmorTreacheryAuraTrigger(PlayerbotAI* botAI) : Trigger(botAI, "omor treachery aura") {}

    bool IsActive() override;
};

class OmorRangedSpreadTrigger : public Trigger
{
public:    OmorRangedSpreadTrigger(PlayerbotAI* botAI) : Trigger(botAI, "omor ranged spread") {}

    bool IsActive() override;
};

class OmorFiendishHoundIsActiveTrigger : public Trigger
{
public:    OmorFiendishHoundIsActiveTrigger(PlayerbotAI* botAI) : Trigger(botAI, "omor fiendish hound is active") {}

    bool IsActive() override;
};

class OmorTankHasTreacheryAuraTrigger : public Trigger
{
public:    OmorTankHasTreacheryAuraTrigger(PlayerbotAI* botAI) : Trigger(botAI, "omor tank has treachery aura") {}
    bool IsActive() override;
};

// Vazruden

class VazrudenTankPositionBossTrigger : public Trigger
{
public:    VazrudenTankPositionBossTrigger(PlayerbotAI* botAI) : Trigger(botAI, "vazruden tank position boss") {}

    bool IsActive() override;
};

#endif
