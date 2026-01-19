/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_PLAYERBOTSPELLREPOSITORY_H
#define _PLAYERBOT_PLAYERBOTSPELLREPOSITORY_H

#include "Playerbots.h"

class PlayerbotSpellRepository
{
public:
    static PlayerbotSpellRepository* Instance()
    {
        static PlayerbotSpellRepository instance;
        return &instance;
    }

    void Initialize(); // call once on startup

    SkillLineAbilityEntry const* GetSkillLine(uint32 spellId) const;
    bool IsItemBuyable(uint32 itemId) const;

private:
    PlayerbotSpellRepository() = default;

    std::map<uint32, SkillLineAbilityEntry const*> skillSpells;
    std::set<uint32> vendorItems;
};

#define sPlayerbotSpellRepository PlayerbotSpellRepository::Instance()

#endif
