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
// tables, covering direct corpse drops, grouped drops, shared
// reference_loot_template indirection, skinning, and gameobject loot, so quest
// objectives can resolve an item to its actual spawns. Pickpocketing is
// intentionally not indexed: the quest engine does not pickpocket.
class LootSourceMgr
{
public:
    static LootSourceMgr& instance();

    void Init();

    // Creature entries whose corpse loot can contain `itemId`.
    [[nodiscard]] std::vector<uint32> GetCreatureSources(uint32 itemId) const;
    // Creature entries whose skinning loot can contain `itemId`.
    [[nodiscard]] std::vector<uint32> GetSkinningSources(uint32 itemId) const;
    // Gameobject entries whose loot can contain `itemId`.
    [[nodiscard]] std::vector<uint32> GetGameObjectSources(uint32 itemId) const;

private:
    enum class LootTableKind : uint8
    {
        Creature,
        Skinning,
        GameObject
    };

    void BuildReferenceIndex();
    void BuildSourceMaps();
    void IndexLootTable(char const* table, std::unordered_multimap<uint32, uint32> const& lootIdToSources,
                        LootTableKind kind);

    std::atomic<bool> _initialized{false};

    // reference_loot_template: refId -> expanded item ids (nested refs resolved)
    std::unordered_map<uint32, std::vector<uint32>> _refToItems;

    // loot id -> source template entry
    std::unordered_multimap<uint32, uint32> _creatureLootIds;   // creature_loot_template.Entry -> creature entry
    std::unordered_multimap<uint32, uint32> _skinLootIds;       // skinning_loot_template.Entry -> creature entry
    std::unordered_multimap<uint32, uint32> _goLootIds;         // gameobject_loot_template.Entry -> GO entry

    // reverse: item id -> source template entry
    std::unordered_multimap<uint32, uint32> _itemToCreatures;
    std::unordered_multimap<uint32, uint32> _itemToSkinning;
    std::unordered_multimap<uint32, uint32> _itemToGameObjects;
};

#define sLootSourceMgr LootSourceMgr::instance()

#endif
