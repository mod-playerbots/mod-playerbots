/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_PLAYERBOTSPELLREPOSITORY_H
#define PLAYERBOTS_PLAYERBOTSPELLREPOSITORY_H

#include <cstdint>

#include "DBCStructure.h"

class PlayerbotSpellRepository
{
public:
    static PlayerbotSpellRepository& Instance()
    {
        static PlayerbotSpellRepository instance;

        return instance;
    }

    void Initialize();

    SkillLineAbilityEntry const* GetSkillLine(uint32_t spellId) const;
    bool IsItemBuyable(uint32_t itemId) const;

private:
    PlayerbotSpellRepository() = default;
    ~PlayerbotSpellRepository() = default;

    PlayerbotSpellRepository(const PlayerbotSpellRepository&) = delete;
    PlayerbotSpellRepository& operator=(const PlayerbotSpellRepository&) = delete;

    PlayerbotSpellRepository(PlayerbotSpellRepository&&) = delete;
    PlayerbotSpellRepository& operator=(PlayerbotSpellRepository&&) = delete;

    std::map<uint32_t, SkillLineAbilityEntry const*> skillSpells;
    std::set<uint32_t> vendorItems;
};

#endif
