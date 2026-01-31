/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_PLAYERBOTSPELLREPOSITORY_H
#define _PLAYERBOT_PLAYERBOTSPELLREPOSITORY_H

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
