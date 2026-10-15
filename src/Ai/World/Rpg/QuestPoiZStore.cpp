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
    candidates.clear();

    // Raw query on purpose: installations without the baked data would fail a prepared-statement
    // prepare at startup; a missing table here just logs and leaves the fallback path active.
    QueryResult result = PlayerbotsDatabase.Query(
        "SELECT `QuestId`, `PoiIdx`, `Rank`, `Z`, `Confidence`, `Evidence`, `NavmeshOk` "
        "FROM `playerbots_quest_poi_z` ORDER BY `QuestId`, `PoiIdx`, `Rank`");

    if (!result)
    {
        LOG_INFO("playerbots", "Quest POI Z store: no baked data (playerbots_quest_poi_z absent or empty); "
                               "quest destinations use the terrain-height fallback.");
        return;
    }

    uint32 rows = 0;
    do
    {
        Field* fields = result->Fetch();

        QuestPoiZCandidate candidate;
        candidate.rank = fields[2].Get<uint8>();
        candidate.z = fields[3].Get<float>();
        candidate.confidence = fields[4].Get<uint8>();
        candidate.evidence = fields[5].Get<std::string>();
        candidate.navmeshOk = fields[6].Get<uint8>() != 0;

        candidates[Key(fields[0].Get<uint32>(), fields[1].Get<uint32>())].push_back(candidate);
        ++rows;
    } while (result->NextRow());

    for (auto& [key, list] : candidates)
        std::sort(list.begin(), list.end(),
                  [](QuestPoiZCandidate const& a, QuestPoiZCandidate const& b) { return a.rank < b.rank; });

    LOG_INFO("playerbots", "Quest POI Z store: loaded {} candidate rows for {} quest POIs.",
             rows, candidates.size());
}

QuestPoiZCandidate const* QuestPoiZStore::Lookup(uint32 questId, uint32 poiId) const
{
    auto itr = candidates.find(Key(questId, poiId));
    if (itr == candidates.end() || itr->second.empty())
        return nullptr;

    return &itr->second.front();
}

std::vector<QuestPoiZCandidate> const* QuestPoiZStore::LookupAll(uint32 questId, uint32 poiId) const
{
    auto itr = candidates.find(Key(questId, poiId));
    if (itr == candidates.end() || itr->second.empty())
        return nullptr;

    return &itr->second;
}
