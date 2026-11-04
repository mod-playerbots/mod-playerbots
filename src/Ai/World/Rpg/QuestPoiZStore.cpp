/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "QuestPoiZStore.h"

#include "DatabaseEnv.h"
#include "PlayerbotsDatabase.h"
#include "QueryResult.h"
#include "Log.h"
#include <algorithm>
#include <cfloat>
#include <cstdio>

QuestPoiZStore& QuestPoiZStore::Instance()
{
    static QuestPoiZStore instance;
    return instance;
}

void QuestPoiZStore::Load()
{
    _candidates.clear();

    // The table is created by the module's SQL update; an empty table keeps the fallback path.
    PreparedQueryResult result = PlayerbotsDatabase.Query(PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_SEL_QUEST_POI_Z));

    if (!result)
    {
        LOG_INFO("playerbots",
                 "Quest POI Z store: no baked data (playerbots_quest_poi_z empty); "
                 "quest destinations use the terrain-height fallback.");
        return;
    }

    uint32 rows = 0;
    uint32 triggerRows = 0;
    uint32 walkableRows = 0;
    do
    {
        Field* fields = result->Fetch();

        QuestPoiZCandidate candidate;
        candidate.pointIdx = fields[2].Get<uint16>();
        candidate.rank = fields[3].Get<uint8>();
        candidate.z = fields[4].Get<float>();
        candidate.confidence = fields[5].Get<uint8>();
        candidate.navmeshOk = fields[6].Get<uint8>() != 0;
        // PoiIdx and MapId are SMALLINT in the schema; reading them as uint32 would consume the
        // neighbouring field in the prepared-statement row.
        candidate.mapId = fields[7].Get<uint16>();
        candidate.evidence = fields[8].Get<std::string>();

        if (!fields[9].IsNull())
        {
            std::string const& walkable = fields[9].Get<std::string>();
            size_t start = 0;
            while (start < walkable.size())
            {
                size_t const end = walkable.find(';', start);
                std::string const token =
                    walkable.substr(start, end == std::string::npos ? std::string::npos : end - start);

                float x = 0.0f, y = 0.0f, z = 0.0f;
                if (sscanf(token.c_str(), "%f,%f,%f", &x, &y, &z) == 3)
                    candidate.walkablePoints.push_back({x, y, z});

                if (end == std::string::npos)
                    break;
                start = end + 1;
            }
        }

        if (candidate.evidence.find("trigger") != std::string::npos)
            ++triggerRows;
        if (!candidate.walkablePoints.empty())
            ++walkableRows;

        _candidates[Key(fields[0].Get<uint32>(), fields[1].Get<uint16>())].push_back(candidate);
        ++rows;
    } while (result->NextRow());

    // Rank first; for equal rank prefer the baker's confidence, then trigger evidence. This keeps a
    // surface candidate from beating the underground/trigger floor for multi-level POIs.
    for (auto& [key, list] : _candidates)
        std::stable_sort(list.begin(), list.end(), [](QuestPoiZCandidate const& a, QuestPoiZCandidate const& b)
        {
            if (a.rank != b.rank)
                return a.rank < b.rank;
            if (a.confidence != b.confidence)
                return a.confidence > b.confidence;

            bool const aTrigger = a.evidence.find("trigger") != std::string::npos;
            bool const bTrigger = b.evidence.find("trigger") != std::string::npos;
            return aTrigger && !bTrigger;
        });

    LOG_INFO("playerbots",
             "Quest POI Z store: loaded {} candidate rows for {} quest POIs ({} trigger evidence, "
             "{} with walkable points).",
             rows, _candidates.size(), triggerRows, walkableRows);
}

QuestPoiZCandidate const* QuestPoiZStore::Lookup(uint32 questId, uint32 poiId, uint32 mapId) const
{
    auto itr = _candidates.find(Key(questId, poiId));
    if (itr == _candidates.end() || itr->second.empty())
        return nullptr;

    // Rows are rank/confidence ordered; return the best one the baker marked navmesh-reachable on
    // this map. If every candidate is non-navmesh or belongs to another map the caller keeps the
    // fallback.
    for (QuestPoiZCandidate const& candidate : itr->second)
        if (candidate.navmeshOk && candidate.mapId == mapId)
            return &candidate;

    return nullptr;
}

bool QuestPoiZStore::FindNearestWalkablePoint(uint32 questId, uint32 poiId, uint32 mapId, float x, float y,
                                              float& outX, float& outY, float& outZ) const
{
    QuestPoiZCandidate const* best = Lookup(questId, poiId, mapId);
    if (!best)
        return false;

    auto itr = _candidates.find(Key(questId, poiId));
    if (itr == _candidates.end())
        return false;

    // Only consider candidates as trustworthy as the selected floor: same-or-higher confidence,
    // and trigger-backed candidates when the best one is trigger-backed. This keeps a surface
    // sample from winning the 2D nearest-point test for an underground trigger.
    uint8 const bestConfidence = best->confidence;
    bool const preferTrigger = best->evidence.find("trigger") != std::string::npos;

    bool found = false;
    float bestDistance = FLT_MAX;
    for (QuestPoiZCandidate const& candidate : itr->second)
    {
        if (candidate.mapId != mapId || !candidate.navmeshOk)
            continue;
        if (candidate.confidence < bestConfidence)
            continue;
        if (preferTrigger && candidate.evidence.find("trigger") == std::string::npos)
            continue;

        for (std::array<float, 3> const& point : candidate.walkablePoints)
        {
            float const dx = point[0] - x;
            float const dy = point[1] - y;
            float const dist = dx * dx + dy * dy;
            if (dist < bestDistance)
            {
                bestDistance = dist;
                outX = point[0];
                outY = point[1];
                outZ = point[2];
                found = true;
            }
        }
    }

    return found;
}
