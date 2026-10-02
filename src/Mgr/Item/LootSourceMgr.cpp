/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "LootSourceMgr.h"
#include "DatabaseEnv.h"
#include "Log.h"
#include "QueryResult.h"
#include <functional>
#include <unordered_set>

LootSourceMgr& LootSourceMgr::instance()
{
    static LootSourceMgr instance;
    return instance;
}

void LootSourceMgr::Init()
{
    if (initialized_.exchange(true))
        return;

    BuildReferenceIndex();
    BuildSourceMaps();

    IndexLootTable("creature_loot_template", creatureLootIds_, false);
    IndexLootTable("skinning_loot_template", skinLootIds_, false);
    IndexLootTable("pickpocketing_loot_template", pickLootIds_, false);
    IndexLootTable("gameobject_loot_template", goLootIds_, true);

    LOG_INFO("playerbots", "LootSourceMgr: indexed {} creature and {} gameobject item sources",
             itemToCreatures_.size(), itemToGameObjects_.size());
}

void LootSourceMgr::BuildReferenceIndex()
{
    refToItems_.clear();

    // raw reference rows: refId -> (item, nestedReference)
    std::unordered_map<uint32, std::vector<std::pair<uint32, uint32>>> raw;
    if (QueryResult result = WorldDatabase.Query("SELECT Entry, Item, Reference FROM reference_loot_template"))
    {
        do
        {
            Field* fields = result->Fetch();
            uint32 entry = fields[0].Get<uint32>();
            uint32 item = fields[1].Get<uint32>();
            uint32 reference = fields[2].Get<uint32>();
            raw[entry].push_back({item, reference});
        } while (result->NextRow());
    }

    // Expand every reference id into its full item list, resolving nested
    // references (a reference_loot_template row may itself reference another)
    // with a visited set to guard against cycles.
    for (auto const& [refId, unused] : raw)
    {
        std::unordered_set<uint32> visited;
        std::vector<uint32>& items = refToItems_[refId];
        std::function<void(uint32)> collect = [&](uint32 rid)
        {
            if (!visited.insert(rid).second)
                return;
            auto it = raw.find(rid);
            if (it == raw.end())
                return;
            for (auto const& [item, reference] : it->second)
            {
                if (reference)
                    collect(reference);
                else if (item)
                    items.push_back(item);
            }
        };
        collect(refId);
    }
}

void LootSourceMgr::BuildSourceMaps()
{
    creatureLootIds_.clear();
    skinLootIds_.clear();
    pickLootIds_.clear();
    goLootIds_.clear();

    // Creature loot ids: corpse lootid defaults to the creature entry when 0;
    // skinning and pickpocketing use their own columns.
    if (QueryResult result = WorldDatabase.Query("SELECT entry, lootid, skinloot, pickpocketloot FROM creature_template"))
    {
        do
        {
            Field* fields = result->Fetch();
            uint32 entry = fields[0].Get<uint32>();
            uint32 lootid = fields[1].Get<uint32>();
            uint32 skinloot = fields[2].Get<uint32>();
            uint32 pickpocketloot = fields[3].Get<uint32>();

            creatureLootIds_.emplace(lootid ? lootid : entry, entry);
            if (skinloot)
                skinLootIds_.emplace(skinloot, entry);
            if (pickpocketloot)
                pickLootIds_.emplace(pickpocketloot, entry);
        } while (result->NextRow());
    }

    // Gameobject loot ids: chest (3) and fishing hole (25) store the loot id in Data1.
    if (QueryResult result = WorldDatabase.Query(
            "SELECT entry, Data1 FROM gameobject_template WHERE type IN (3, 25) AND Data1 <> 0"))
    {
        do
        {
            Field* fields = result->Fetch();
            uint32 entry = fields[0].Get<uint32>();
            uint32 lootId = fields[1].Get<uint32>();
            goLootIds_.emplace(lootId, entry);
        } while (result->NextRow());
    }
}

void LootSourceMgr::IndexLootTable(char const* table,
                                   std::unordered_multimap<uint32, uint32> const& lootIdToSources,
                                   bool gameObject)
{
    QueryResult result = WorldDatabase.Query("SELECT Entry, Item, Reference FROM {}", table);
    if (!result)
        return;

    auto& target = gameObject ? itemToGameObjects_ : itemToCreatures_;

    do
    {
        Field* fields = result->Fetch();
        uint32 entry = fields[0].Get<uint32>();       // loot id
        uint32 item = fields[1].Get<uint32>();
        uint32 reference = fields[2].Get<uint32>();

        std::vector<uint32> items;
        if (reference)
        {
            // The Item value on a reference row is an ignored placeholder; the
            // real items live in the shared reference template.
            auto it = refToItems_.find(reference);
            if (it == refToItems_.end())
                continue;
            items = it->second;
        }
        else if (item)
        {
            items.push_back(item);
        }
        else
        {
            continue;
        }

        auto range = lootIdToSources.equal_range(entry);
        for (auto sit = range.first; sit != range.second; ++sit)
            for (uint32 itemId : items)
                target.emplace(itemId, sit->second);
    } while (result->NextRow());
}

std::vector<uint32> LootSourceMgr::GetCreatureSources(uint32 itemId) const
{
    std::vector<uint32> out;
    auto range = itemToCreatures_.equal_range(itemId);
    for (auto it = range.first; it != range.second; ++it)
        out.push_back(it->second);
    return out;
}

std::vector<uint32> LootSourceMgr::GetGameObjectSources(uint32 itemId) const
{
    std::vector<uint32> out;
    auto range = itemToGameObjects_.equal_range(itemId);
    for (auto it = range.first; it != range.second; ++it)
        out.push_back(it->second);
    return out;
}
