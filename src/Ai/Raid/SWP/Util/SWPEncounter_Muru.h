/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_SWPENCOUNTERMURU_H
#define PLAYERBOTS_SWPENCOUNTERMURU_H

#include "ObjectGuid.h"
#include "Position.h"
#include "SWPData.h"
#include <unordered_map>
#include <vector>

class Creature;
class Player;
class PlayerbotAI;
class Unit;

namespace SwpHelpers
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

extern Position const MURU_STACK_POSITION;
extern Position const MURU_VOID_SENTINEL_N_TANK_POSITION;
extern Position const MURU_VOID_SENTINEL_E_TANK_POSITION;
extern Position const MURU_CENTER_POSITION;
extern Position const MURU_ENTRANCE_POSITION;

extern std::unordered_map<uint32, MuruDarknessState> muruDarknessStates;
extern std::unordered_map<uint32, std::unordered_map<ObjectGuid, uint8>>
    muruVoidSentinelTankAssignments;

bool TryGetMuruDarknessActiveState(Player* bot, Unit* muru);
bool TryGetMuruDarknessEarlyState(Player* bot, Unit* muru, uint32 earlyWindowMs = 10000);
void GatherMuruEncounterTargets(PlayerbotAI* botAI, MuruEncounterTargets& targets);
Creature* FindAvailableVoidSpawnForEnslave(Player* bot);

}

#endif
