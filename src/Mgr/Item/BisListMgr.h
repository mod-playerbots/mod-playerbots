/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_BISLISTMGR_H
#define _PLAYERBOT_BISLISTMGR_H

#include "Define.h"

#include <map>

class BisListMgr
{
public:
    static BisListMgr* instance()
    {
        static BisListMgr inst;
        return &inst;
    }

    void LoadAll();

    // faction: 1=Alliance, 2=Horde. Faction-specific rows override faction=0 (Both).
    // Returns slot -> itemId for the matching auto_gear_score_limit tier. Empty map = no data.
    std::map<uint8, uint32> GetBisFor(uint16 autoGearScoreLimit, uint8 cls, uint8 tab, uint8 faction) const;

    // Closest-lower fallback: scan ilvls down from requested to (requested - maxDrop), return first non-empty set.
    // outResolved receives the matched ilvl (0 if nothing matched within the window).
    std::map<uint8, uint32> GetBisForNearest(uint16 requestedIlvl, uint16 maxDrop, uint8 cls, uint8 tab,
                                             uint8 faction, uint16* outResolved = nullptr) const;

private:
    BisListMgr() = default;

    static uint16 MakeKey(uint8 cls, uint8 tab) { return (uint16(cls) << 8) | tab; }

    // autoGearScoreLimit -> (cls<<8|tab) -> faction (0/1/2) -> slot -> itemId
    std::map<uint16, std::map<uint16, std::map<uint8, std::map<uint8, uint32>>>> _bis;
};

#define sBisListMgr BisListMgr::instance()

#endif
