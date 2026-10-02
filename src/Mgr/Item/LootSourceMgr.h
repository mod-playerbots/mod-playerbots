/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_LOOTSOURCEMGR_H
#define PLAYERBOTS_LOOTSOURCEMGR_H

#include "Define.h"
#include <atomic>
#include <unordered_map>
#include <vector>

// Reverse loot index: given an item id, which creature/gameobject entries can
// drop (or otherwise source) it. Built once at module load from the loot
// tables, covering direct drops, grouped drops, and shared reference_loot_template
// indirection, so quest objectives can resolve an item to its actual spawns.
class LootSourceMgr
{
public:
    static LootSourceMgr& instance();

    void Init();

    [[nodiscard]] bool IsInitialized() const { return initialized_.load(); }

    // Creature entries that can drop/loot `itemId`.
    [[nodiscard]] std::vector<uint32> GetCreatureSources(uint32 itemId) const;
    // Gameobject entries whose loot can contain `itemId`.
    [[nodiscard]] std::vector<uint32> GetGameObjectSources(uint32 itemId) const;

private:
    void BuildReferenceIndex();
    void BuildSourceMaps();
    void IndexLootTable(char const* table, std::unordered_multimap<uint32, uint32> const& lootIdToSources, bool gameObject);

    std::atomic<bool> initialized_{false};

    // reference_loot_template: refId -> expanded item ids (nested refs resolved)
    std::unordered_map<uint32, std::vector<uint32>> refToItems_;

    // loot id -> source template entry
    std::unordered_multimap<uint32, uint32> creatureLootIds_;   // creature_loot_template.Entry -> creature entry
    std::unordered_multimap<uint32, uint32> skinLootIds_;       // skinning_loot_template.Entry -> creature entry
    std::unordered_multimap<uint32, uint32> pickLootIds_;       // pickpocketing_loot_template.Entry -> creature entry
    std::unordered_multimap<uint32, uint32> goLootIds_;         // gameobject_loot_template.Entry -> GO entry

    // reverse: item id -> source template entry
    std::unordered_multimap<uint32, uint32> itemToCreatures_;
    std::unordered_multimap<uint32, uint32> itemToGameObjects_;
};

#define sLootSourceMgr LootSourceMgr::instance()

#endif
