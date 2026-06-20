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
    NPC_FIENDISH_HOUND                    = 17540,
    //Vazruden (Skull the two guards and dps them down. Tank Vazruden in the middle (X:-1405 Y: 1745 Z: 81) of the platform to prevent the tank from attempting to grab the dragon flying around the platform. Will see if bots need to avoid the flames, easiest fight in the dungeon.)

};

// Watchkeeper Gargolmar

class GargolmarTankPositionBossTrigger : public Trigger
{
public:
    GargolmarTankPositionBossTrigger(PlayerbotAI* botAI) : Trigger(botAI, "gargolmar tank position boss") {}

    bool IsActive() override;
};

class GargolmarHellfireWatchersAreActiveTrigger : public Trigger
{
public:
    GargolmarHellfireWatchersAreActiveTrigger(PlayerbotAI* botAI) : Trigger(botAI, "gargolmar hellfire watchers are active") {}

    bool IsActive() override;
};

// Omor the Unscarred

class OmorTreacherousAuraTrigger : public Trigger
{
public:
    OmorTreacherousAuraTrigger(PlayerbotAI* botAI) : Trigger(botAI, "omor treacherous aura") {}

    bool IsActive() override;
};

class OmorBaneOfTreacheryAuraTrigger : public Trigger
{
public:
    OmorBaneOfTreacheryAuraTrigger(PlayerbotAI* botAI) : Trigger(botAI, "omor bane of treachery aura") {}

    bool IsActive() override;
};
