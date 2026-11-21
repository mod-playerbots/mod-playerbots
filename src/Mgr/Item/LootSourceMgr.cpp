/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "LootSourceMgr.h"
#include "DatabaseEnv.h"
#include "Log.h"
#include "QueryResult.h"
#include "SharedDefines.h"
#include <functional>
#include <unordered_set>

LootSourceMgr& LootSourceMgr::instance()
{
    static LootSourceMgr instance;
    return instance;
}

void LootSourceMgr::Init()
{
    if (_initialized.exchange(true))
        return;

    BuildReferenceIndex();
    BuildSourceMaps();

    IndexLootTable("creature_loot_template", _creatureLootIds, LootTableKind::Creature);
    IndexLootTable("skinning_loot_template", _skinLootIds, LootTableKind::Skinning);
    IndexLootTable("gameobject_loot_template", _goLootIds, LootTableKind::GameObject);

    LOG_INFO("playerbots", "LootSourceMgr: indexed {} creature, {} skinning and {} gameobject item sources",
             _itemToCreatures.size(), _itemToSkinning.size(), _itemToGameObjects.size());
}

void LootSourceMgr::BuildReferenceIndex()
{
    _refToItems.clear();

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
        std::vector<uint32>& items = _refToItems[refId];
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
    _creatureLootIds.clear();
    _skinLootIds.clear();
    _goLootIds.clear();

    // Creature loot ids: a creature with lootid 0 has no corpse loot (the core
    // only fills loot when lootid is non-zero); skinning uses its own column.
    if (QueryResult result = WorldDatabase.Query("SELECT entry, lootid, skinloot FROM creature_template"))
    {
        do
        {
            Field* fields = result->Fetch();
            uint32 entry = fields[0].Get<uint32>();
            uint32 lootid = fields[1].Get<uint32>();
            uint32 skinloot = fields[2].Get<uint32>();

            if (lootid)
                _creatureLootIds.emplace(lootid, entry);
            if (skinloot)
                _skinLootIds.emplace(skinloot, entry);
        } while (result->NextRow());
    }

    // Gameobject loot ids: chests and fishing holes store the loot id in Data1.
    if (QueryResult result = WorldDatabase.Query(
            "SELECT entry, Data1 FROM gameobject_template WHERE type IN ({}, {}) AND Data1 <> 0",
            static_cast<uint32>(GAMEOBJECT_TYPE_CHEST), static_cast<uint32>(GAMEOBJECT_TYPE_FISHINGHOLE)))
    {
        do
        {
            Field* fields = result->Fetch();
            uint32 entry = fields[0].Get<uint32>();
            uint32 lootId = fields[1].Get<uint32>();
            _goLootIds.emplace(lootId, entry);
        } while (result->NextRow());
    }
}

void LootSourceMgr::IndexLootTable(char const* table,
                                   std::unordered_multimap<uint32, uint32> const& lootIdToSources,
                                   LootTableKind kind)
{
    QueryResult result = WorldDatabase.Query("SELECT Entry, Item, Reference FROM {}", table);
    if (!result)
    {
        LOG_INFO("playerbots", "LootSourceMgr: no rows returned from {}; no item sources indexed from it", table);
        return;
    }

    auto& target = kind == LootTableKind::GameObject ? _itemToGameObjects
                   : kind == LootTableKind::Skinning ? _itemToSkinning
                                                     : _itemToCreatures;

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
            auto it = _refToItems.find(reference);
            if (it == _refToItems.end())
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
    auto range = _itemToCreatures.equal_range(itemId);
    for (auto it = range.first; it != range.second; ++it)
        out.push_back(it->second);
    return out;
}

std::vector<uint32> LootSourceMgr::GetSkinningSources(uint32 itemId) const
{
    std::vector<uint32> out;
    auto range = _itemToSkinning.equal_range(itemId);
    for (auto it = range.first; it != range.second; ++it)
        out.push_back(it->second);
    return out;
}

std::vector<uint32> LootSourceMgr::GetGameObjectSources(uint32 itemId) const
{
    std::vector<uint32> out;
    auto range = _itemToGameObjects.equal_range(itemId);
    for (auto it = range.first; it != range.second; ++it)
        out.push_back(it->second);
    return out;
}
