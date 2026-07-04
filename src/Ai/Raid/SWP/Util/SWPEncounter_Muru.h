/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef PLAYERBOTS_SWPENCOUNTERMURU_H
#define PLAYERBOTS_SWPENCOUNTERMURU_H

#include <unordered_map>
#include <vector>

#include "ObjectGuid.h"
#include "Position.h"
#include "SWPData.h"

class Creature;
class Player;
class PlayerbotAI;
class Unit;

namespace SunwellHelpers
{

struct MuruEncounterTargets
{
    Unit* muru = nullptr;
    Unit* entropius = nullptr;
    std::vector<Unit*> voidSentinels;
    std::vector<Unit*> voidSpawns;
    std::vector<Unit*> furyMages;
    std::vector<Unit*> berserkers;
};

struct MuruDarknessState
{
    uint32 startMs = 0;
    uint32 expireMs = 0;
};

extern const Position MURU_STACK_POSITION;
extern const Position MURU_VOID_SENTINEL_N_TANK_POSITION;
extern const Position MURU_VOID_SENTINEL_E_TANK_POSITION;
extern const Position MURU_CENTER_POSITION;
extern const Position MURU_ENTRANCE_POSITION;

extern std::unordered_map<uint32, MuruDarknessState> muruDarknessStates;
extern std::unordered_map<uint32, std::unordered_map<ObjectGuid, uint8>>
    muruVoidSentinelTankAssignments;

bool TryGetMuruDarknessActiveState(Player* bot, Unit* muru);
bool TryGetMuruDarknessEarlyState(Player* bot, Unit* muru, uint32 earlyWindowMs = 10000);
void GatherMuruEncounterTargets(PlayerbotAI* botAI, MuruEncounterTargets& targets);
Creature* FindAvailableVoidSpawnForEnslave(
    PlayerbotAI* botAI, Player* bot, Unit* muru, Unit* entropius);
Creature* GetNearestMuruSingularity(Player* bot, float searchRadius = 30.0f);

}

#endif
