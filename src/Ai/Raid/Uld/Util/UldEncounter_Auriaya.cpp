/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "UldEncounter_Auriaya.h"

#include "AttackersValue.h"
#include "CellImpl.h"
#include "Creature.h"
#include "EncounterHelpers.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "Group.h"
#include "Map.h"
#include "ObjectGuid.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"
#include "Timer.h"
#include "UldScripts.h"
#include "UldTargets.h"
#include "Unit.h"

#include <algorithm>
#include <cmath>
#include <iterator>
#include <limits>
#include <list>
#include <string>
#include <vector>

using namespace EncounterHelpers;

namespace
{
// The check behind "possible targets no los", with the entry tested first.
struct UnfriendlyOfEntryInRangeCheck
{
    Acore::AnyUnfriendlyUnitInObjectRangeCheck inRange;
    uint32 entry;

    bool operator()(Unit* unit) { return unit->GetEntry() == entry && inRange(unit); }
};

// What "possible targets no los" holds for one entry, in the same order. Reading the value itself
// rebuilds and copies the whole list on every call, with IsPossibleTarget on every hostile in sight.
std::vector<Unit*> CollectPossibleTargetsByEntry(Player* bot, uint32 entry)
{
    float const range = sPlayerbotAIConfig.SightDistance;

    std::vector<Unit*> matches;
    UnfriendlyOfEntryInRangeCheck check{Acore::AnyUnfriendlyUnitInObjectRangeCheck(bot, bot, range), entry};
    Acore::UnitListSearcher<UnfriendlyOfEntryInRangeCheck> searcher(bot, matches, check);
    Cell::VisitObjects(bot, searcher, range);

    std::vector<Unit*> targets;
    for (Unit* unit : matches)
        if (AttackersValue::IsPossibleTarget(unit, bot, range))
            targets.push_back(unit);

    return targets;
}

// Boss and pools once per bot per tick: a bot never ticks twice in one ms and runs a whole tick on
// one thread. Guids, resolved on read, so a mid-tick death or despawn reads null.
struct AuriayaRoomScan
{
    PlayerbotAI* botAI = nullptr;
    uint32 atMs = 0;
    ObjectGuid boss;
    GuidVector pools;

    // Trigger and action both ask for the soak spot in one tick, and picking one costs raycasts.
    bool soakPicked = false;
    bool soakFound = false;
    Position soak;
};

thread_local AuriayaRoomScan auriayaRoomScan;

AuriayaRoomScan& RefreshedAuriayaRoomScan(PlayerbotAI* botAI)
{
    uint32 const now = getMSTime();
    if (auriayaRoomScan.botAI == botAI && auriayaRoomScan.atMs == now)
        return auriayaRoomScan;

    auriayaRoomScan = AuriayaRoomScan();
    auriayaRoomScan.botAI = botAI;
    auriayaRoomScan.atMs = now;

    std::vector<Unit*> const found = CollectPossibleTargetsByEntry(botAI->GetBot(), NPC_AURIAYA);
    if (found.empty())
        return auriayaRoomScan;

    auriayaRoomScan.boss = found.front()->GetGUID();
    for (Unit* pool : CollectAuriayaEssencePools(found.front(), ULDUAR_AURIAYA_ROOM_SEARCH_RADIUS))
        auriayaRoomScan.pools.push_back(pool->GetGUID());

    return auriayaRoomScan;
}

// Her cone faces whoever she's hitting, a human tank or a taunting player included.
float GetSoakAxis(Unit* boss)
{
    Unit* victim = boss->GetVictim();
    return victim && victim != boss ? boss->GetAngle(victim) : boss->GetOrientation();
}

bool IsInSoakBand(Unit* boss, float axis, float x, float y, float halfAngle, float minDistance, float maxDistance)
{
    float const distance = boss->GetExactDist2d(x, y);
    if (distance < minDistance || distance > maxDistance)
        return false;

    float const offset = Position::NormalizeOrientation(boss->GetAngle(x, y) - axis);
    return std::min(offset, 2.0f * static_cast<float>(M_PI) - offset) <= halfAngle;
}

// Clear spots first, then fewest pools, then shortest walk. Every candidate tried costs a navmesh
// raycast, so they go in that order and the first one on the floor wins.
bool PickSoakSpot(WorldObject* floorFrom, Unit* boss, std::vector<Unit*> const& pools, Position& out)
{
    struct Candidate
    {
        float x;
        float y;
        bool clear;
        int fouling;
        float walk;
    };

    constexpr int stepsPerSide = 4;
    constexpr float distances[] = {12.0f, 16.0f, 20.0f, 24.0f};
    static_assert(distances[0] >= ULDUAR_AURIAYA_SOAK_TARGET_MIN &&
                  distances[3] <= ULDUAR_AURIAYA_SOAK_TARGET_MAX);

    float const axis = GetSoakAxis(boss);
    float const angleStep = ULDUAR_AURIAYA_SOAK_TARGET_HALF_ANGLE / stepsPerSide;

    std::vector<Candidate> candidates;
    candidates.reserve((2 * stepsPerSide + 1) * std::size(distances));
    for (int step = -stepsPerSide; step <= stepsPerSide; ++step)
    {
        float const angle = axis + step * angleStep;
        for (float distance : distances)
        {
            float const x = boss->GetPositionX() + distance * std::cos(angle);
            float const y = boss->GetPositionY() + distance * std::sin(angle);

            int fouling = 0;
            for (Unit* pool : pools)
                if (pool->GetExactDist2d(x, y) < ULDUAR_AURIAYA_SEEPING_ESSENCE_RADIUS)
                    ++fouling;

            candidates.push_back({x, y, GetAuriayaPoolClearance(pools, x, y) >= ULDUAR_AURIAYA_ESSENCE_CLEARANCE,
                                  fouling, floorFrom->GetExactDist2d(x, y)});
        }
    }

    std::sort(candidates.begin(), candidates.end(),
              [](Candidate const& a, Candidate const& b)
              {
                  if (a.clear != b.clear)
                      return a.clear;

                  if (a.fouling != b.fouling)
                      return a.fouling < b.fouling;

                  return a.walk < b.walk;
              });

    for (Candidate const& candidate : candidates)
    {
        float x = candidate.x;
        float y = candidate.y;
        float z = boss->GetPositionZ();
        if (!floorFrom->GetMap()->CheckCollisionAndGetValidCoords(floorFrom, floorFrom->GetPositionX(),
                                                                  floorFrom->GetPositionY(),
                                                                  floorFrom->GetPositionZ(), x, y, z))
            continue;

        // The raycast stops at the floor's edge, so a spot past it comes back clipped. The room is
        // round with a drop past it, and her far stations sit near that edge.
        if (std::hypot(x - candidate.x, y - candidate.y) > 1.0f)
            continue;

        out = Position(x, y, z);
        return true;
    }

    return false;
}
}  // namespace

// Off her spawn (1956.2, 49.32) along her home facing (-0.955, 0.296), away from the corridor, at
// 5/15/25 yd. All on the floor. The soak band, not a fixed point, fits the raid in past each one.
const Position ULDUAR_AURIAYA_MAINTANK_SPOTS[ULDUAR_AURIAYA_STATION_COUNT] = {
    Position(1951.42f, 50.79f, 411.36f), Position(1941.87f, 53.75f, 411.36f),
    Position(1932.32f, 56.71f, 411.36f)};

bool AuriayaFearWindowActive(PlayerbotAI* botAI) { return AuriayaEncounterActive(botAI); }

Unit* GetAuriaya(PlayerbotAI* botAI)
{
    ObjectGuid const& guid = RefreshedAuriayaRoomScan(botAI).boss;
    if (!guid)
        return nullptr;

    // Alive check too: she can die after the scan in the same tick, and callers never expect a corpse.
    Unit* boss = botAI->GetUnit(guid);
    return boss && boss->IsAlive() ? boss : nullptr;
}

std::vector<Unit*> GetAuriayaRoomPools(PlayerbotAI* botAI)
{
    std::vector<Unit*> pools;
    for (ObjectGuid const& guid : RefreshedAuriayaRoomScan(botAI).pools)
        if (Unit* pool = botAI->GetUnit(guid))
            pools.push_back(pool);

    return pools;
}

bool AuriayaEncounterActive(PlayerbotAI* botAI) { return GetAuriaya(botAI) != nullptr; }

bool IsAuriayaEngaged(PlayerbotAI* botAI)
{
    Unit* boss = GetAuriaya(botAI);
    return boss && boss->IsInCombat();
}

Unit* GetAuriayaFocusTarget(PlayerbotAI* botAI)
{
    if (Unit* sentry = GetUldUnitByEntry(botAI, NPC_AURIAYA_SANCTUM_SENTRY))
        return sentry;

    // Feigned at 1 HP and unselectable between lives, so IsAlive() alone isn't enough.
    Unit* defender = GetUldUnitByEntry(botAI, NPC_AURIAYA_FERAL_DEFENDER);

    return IsDownOrFeigning(defender) ? nullptr : defender;
}

Unit* GetAuriayaLooseSentry(PlayerbotAI* botAI, Player* tank)
{
    for (Unit* sentry : CollectPossibleTargetsByEntry(botAI->GetBot(), NPC_AURIAYA_SANCTUM_SENTRY))
        if (sentry->GetVictim() != tank)
            return sentry;

    return nullptr;
}

std::vector<Unit*> CollectAuriayaEssencePools(WorldObject* from, float radius)
{
    std::vector<Unit*> pools;
    if (!from)
        return pools;

    std::list<Creature*> found;
    from->GetCreatureListWithEntryInGrid(found, NPC_AURIAYA_SEEPING_FERAL_ESSENCE, radius);

    for (Creature* creature : found)
        if (creature && creature->IsAlive())
            pools.push_back(creature);

    return pools;
}

float GetAuriayaPoolClearance(std::vector<Unit*> const& pools, float x, float y)
{
    float clearance = std::numeric_limits<float>::max();
    for (Unit* pool : pools)
        clearance = std::min(clearance, pool->GetExactDist2d(x, y));

    return clearance;
}

bool IsInAuriayaPool(Player* bot)
{
    return GetAuriayaPoolClearance(CollectAuriayaEssencePools(bot, ULDUAR_AURIAYA_SEEPING_ESSENCE_RADIUS),
                                   bot->GetPositionX(), bot->GetPositionY()) < ULDUAR_AURIAYA_SEEPING_ESSENCE_RADIUS;
}

int GetAuriayaStationIndex(std::vector<Unit*> const& roomPools)
{
    int best = 0;
    float bestClearance = -1.0f;

    for (int i = 0; i < ULDUAR_AURIAYA_STATION_COUNT; ++i)
    {
        Position const& spot = ULDUAR_AURIAYA_MAINTANK_SPOTS[i];
        float const clearance = GetAuriayaPoolClearance(roomPools, spot.GetPositionX(), spot.GetPositionY());
        if (clearance >= ULDUAR_AURIAYA_STATION_FOUL_RADIUS)
            return i;

        if (clearance > bestClearance)
        {
            bestClearance = clearance;
            best = i;
        }
    }

    return best;
}

bool GetAuriayaSoakSpot(PlayerbotAI* botAI, Player* bot, Unit* boss, std::vector<Unit*> const& pools,
                        Position& out)
{
    AuriayaRoomScan& scan = RefreshedAuriayaRoomScan(botAI);
    if (!scan.soakPicked)
    {
        scan.soakPicked = true;

        bool const clear = GetAuriayaPoolClearance(pools, bot->GetPositionX(), bot->GetPositionY()) >=
                           ULDUAR_AURIAYA_SEEPING_ESSENCE_RADIUS;

        // Judge drift only once she stops. The band rides on her, so a tank walking to its next
        // station would otherwise re-aim every ranged bot at once.
        if (clear && (boss->isMoving() ||
                      IsInSoakBand(boss, GetSoakAxis(boss), bot->GetPositionX(), bot->GetPositionY(),
                                   ULDUAR_AURIAYA_SOAK_STAY_HALF_ANGLE, ULDUAR_AURIAYA_SOAK_STAY_MIN,
                                   ULDUAR_AURIAYA_SOAK_STAY_MAX)))
        {
            scan.soakFound = true;
            scan.soak = bot->GetPosition();
        }
        else
        {
            scan.soakFound = PickSoakSpot(bot, boss, pools, scan.soak);
        }
    }

    out = scan.soak;
    return scan.soakFound;
}

bool PickAuriayaSoakSpotFromBoss(Unit* boss, std::vector<Unit*> const& pools, Position& out)
{
    return boss && PickSoakSpot(boss, boss, pools, out);
}

bool GetAuriayaAnchor(PlayerbotAI* botAI, Player* bot, Unit* boss, std::vector<Unit*> const& roomPools,
                      Position& out, float& tolerance)
{
    if (!boss)
        return false;

    if (PlayerbotAI::IsMainTank(bot))
    {
        int const station = GetAuriayaStationIndex(roomPools);
        out = ULDUAR_AURIAYA_MAINTANK_SPOTS[station];
        tolerance = ULDUAR_AURIAYA_MAINTANK_SPOT_TOLERANCE;
        return true;
    }

    if (!PlayerbotAI::IsRanged(bot))
        return false;

    tolerance = ULDUAR_AURIAYA_SOAK_SPOT_TOLERANCE;
    return GetAuriayaSoakSpot(botAI, bot, boss, roomPools, out);
}
