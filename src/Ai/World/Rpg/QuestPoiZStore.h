/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_QUESTPOIZSTORE_H
#define PLAYERBOTS_QUESTPOIZSTORE_H

#include "Define.h"
#include <unordered_map>
#include <vector>

// One baked walkable Z candidate for a quest POI (see the destination-Z plan). The store collapses
// the baker's per-point rows to the single best-ranked navmesh-reachable candidate per POI.
struct QuestPoiZCandidate
{
    float z{0.0f};
    uint8 rank{1};
    uint8 confidence{0};
    bool navmeshOk{true};
};

// Baked walkable Z candidates for quest POIs (`playerbots_quest_poi_z`), generated offline by
// acore-data/scripts/quest_poi_z_baker.py. Loaded once at startup on the world thread; read-only
// afterwards. Absent table/rows = destinations keep using the terrain-height fallback.
class QuestPoiZStore
{
public:
    static QuestPoiZStore& Instance();

    void Load();

    // Best (lowest rank, navmesh-reachable) baked candidate for a quest_poi row, or nullptr when
    // unmapped or every candidate is flagged non-navmesh.
    QuestPoiZCandidate const* Lookup(uint32 questId, uint32 poiId) const;

private:
    static uint64_t Key(uint32 questId, uint32 poiId) { return (uint64_t(questId) << 32) | uint64_t(poiId); }

    std::unordered_map<uint64_t, std::vector<QuestPoiZCandidate>> _candidates;
};

#endif
