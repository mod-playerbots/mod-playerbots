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
    do
    {
        Field* fields = result->Fetch();

        QuestPoiZCandidate candidate;
        candidate.pointIdx = fields[2].Get<uint16>();
        candidate.rank = fields[3].Get<uint8>();
        candidate.z = fields[4].Get<float>();
        candidate.confidence = fields[5].Get<uint8>();
        candidate.navmeshOk = fields[6].Get<uint8>() != 0;
        candidate.mapId = fields[7].Get<uint32>();

        _candidates[Key(fields[0].Get<uint32>(), fields[1].Get<uint32>())].push_back(candidate);
        ++rows;
    } while (result->NextRow());

    for (auto& [key, list] : _candidates)
        std::sort(list.begin(), list.end(),
                  [](QuestPoiZCandidate const& a, QuestPoiZCandidate const& b) { return a.rank < b.rank; });

    LOG_INFO("playerbots", "Quest POI Z store: loaded {} candidate rows for {} quest POIs.", rows, _candidates.size());
}

QuestPoiZCandidate const* QuestPoiZStore::Lookup(uint32 questId, uint32 poiId, uint32 mapId) const
{
    auto itr = _candidates.find(Key(questId, poiId));
    if (itr == _candidates.end() || itr->second.empty())
        return nullptr;

    // Rows are rank-ordered; return the best one the baker marked navmesh-reachable on this map.
    // If every candidate is non-navmesh or belongs to another map the caller keeps the fallback.
    for (QuestPoiZCandidate const& candidate : itr->second)
        if (candidate.navmeshOk && candidate.mapId == mapId)
            return &candidate;

    return nullptr;
}
