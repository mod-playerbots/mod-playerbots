/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "MagHelpers.h"
#include "Creature.h"
#include "GameObject.h"
#include "Map.h"
#include "ObjectGuid.h"
#include "Playerbots.h"

namespace MagtheridonHelpers
{

const Position WAITING_FOR_MAGTHERIDON_POSITION = { -31.962f,  -8.514f, -0.304f, 0.657f };
const Position MAGTHERIDON_TANK_POSITION =        {  -6.147f, -37.812f, -0.411f,   0.0f };
const Position NW_CHANNELER_TANK_POSITION =       { -11.764f,  30.818f, -0.411f,   0.0f };
const Position NE_CHANNELER_TANK_POSITION =       { -12.490f, -26.211f, -0.411f,   0.0f };
const Position RANGED_SPREAD_POSITION =           { -14.890f,   1.995f, -0.406f,   0.0f };
const Position HEALER_SPREAD_POSITION =           {  -2.265f,   1.874f, -0.404f,   0.0f };

std::unordered_map<uint32, time_t> blastNovaTimer;
std::unordered_map<uint32, time_t> dpsWaitTimer;
std::unordered_map<uint32, bool> ceilingCollapseApplied;
std::unordered_map<uint32, bool> lastBlastNovaState;
std::unordered_map<uint32, std::unordered_map<ObjectGuid, CubeInfo>>
    botToCubeAssignments;
std::unordered_map<uint32, std::vector<DebrisData>> activeDebrisPositions;

// Identify channelers by their database GUIDs
Creature* GetChanneler(Player* bot, uint32 dbGuid)
{
    Map* map = bot->GetMap();
    if (!map)
        return nullptr;

    auto it = map->GetCreatureBySpawnIdStore().find(dbGuid);
    if (it == map->GetCreatureBySpawnIdStore().end())
        return nullptr;

    Creature* channeler = it->second;
    if (!channeler->IsAlive())
        return nullptr;

    return channeler;
}

bool IsMagtheridonActive(Unit* magtheridon)
{
    return magtheridon && !magtheridon->HasAura(
        static_cast<uint32>(MagtheridonSpells::SPELL_SHADOW_CAGE));
}

const std::vector<uint32> MANTICRON_CUBE_DB_GUIDS = { 43157, 43158, 43159, 43160, 43161 };

// Get the positions of all Manticron Cubes by their database GUIDs
std::vector<CubeInfo> GetAllCubeInfosByDbGuids(
    Map* map, const std::vector<uint32>& cubeDbGuids)
{
    std::vector<CubeInfo> cubes;
    if (!map)
        return cubes;

    for (uint32 dbGuid : cubeDbGuids)
    {
        auto bounds = map->GetGameObjectBySpawnIdStore().equal_range(dbGuid);
        if (bounds.first == bounds.second)
            continue;

        GameObject* go = bounds.first->second;
        if (!go)
            continue;

        CubeInfo info;
        info.guid = go->GetGUID();
        info.x = go->GetPositionX();
        info.y = go->GetPositionY();
        info.z = go->GetPositionZ();
        cubes.push_back(info);
    }

    return cubes;
}

bool IsCubeClicker(Player* bot)
{
    auto mapIt = botToCubeAssignments.find(bot->GetMap()->GetInstanceId());
    return mapIt != botToCubeAssignments.end() &&
           mapIt->second.find(bot->GetGUID()) != mapIt->second.end();
}

bool IsPositionInActiveDebris(uint32 instanceId, float x, float y, float radius)
{
    constexpr uint32 maxAgeMs = 8000;

    auto it = activeDebrisPositions.find(instanceId);
    if (it == activeDebrisPositions.end())
        return false;

    const uint32 now = getMSTime();
    for (DebrisData const& debris : it->second)
    {
        if (getMSTimeDiff(debris.spawnTime, now) > maxAgeMs)
            continue;

        float dx = x - debris.position.GetPositionX();
        float dy = y - debris.position.GetPositionY();
        if ((dx * dx + dy * dy) < (radius * radius))
            return true;
    }

    return false;
}

bool IsPositionInActiveConflagration(PlayerbotAI* botAI, Player* bot, float x, float y)
{
    constexpr float conflagrationHazardRadius = 5.0f;
    GuidVector const& gameObjects =
        botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest game objects")->Get();
    for (auto const& goGuid : gameObjects)
    {
        GameObject* go = botAI->GetGameObject(goGuid);
        if (!go || !go->isSpawned() ||
            go->GetEntry() !=
                static_cast<uint32>(MagtheridonHelpers::MagtheridonObjects::GO_BLAZE))
        {
            continue;
        }

        if (go->GetDistance2d(x, y) < conflagrationHazardRadius)
            return true;
    }

    return false;
}

}
