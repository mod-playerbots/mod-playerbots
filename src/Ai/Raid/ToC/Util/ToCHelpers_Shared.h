/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RAID_TOCHELPERS_SHARED_H
#define PLAYERBOTS_RAID_TOCHELPERS_SHARED_H

#include <vector>

#include "ObjectGuid.h"
#include "PlayerbotAI.h"
#include "RaidCombatHelpers.h"
#include "RaidDirectTargeting.h"
#include "Timer.h"
#include "ToCData.h"

class Creature;
class Player;
class WorldObject;

namespace TrialOfTheCrusaderHelpers
{

// Fixed entry list with an O(1) membership test
class ToCEntrySet
{
public:
    explicit ToCEntrySet(std::vector<uint32> const& entries);

    bool Contains(uint32 entry) const
    {
        uint32 const offset = entry - _base;  // wraps past _bits.size() below _base
        return offset < _bits.size() && _bits[offset];
    }

private:
    uint32 _base = 0;
    std::vector<bool> _bits;
};

// GetCreatureListWithEntryInGrid over an entry list, into a vector: same check, cells, phase test and
// order, without copying the list per call or testing every entry per creature
void GetCreatureListWithEntriesInGrid(WorldObject const* origin, std::vector<Creature*>& creatures,
                                      ToCEntrySet const& entries, float radius);

// The ToC spellings the Anub'arak helpers and triggers call
inline constexpr auto& IsToCDpsBot = IsRaidDpsBot;
inline constexpr auto& GetToCHumanSkull = GetRaidHumanSkull;
inline constexpr auto& ToCPetOffPick = RaidPetOffPick;

// Empty when the bot has no group
ObjectGuid ToCGroupGuid(Player* bot);

// One group walk per group per ms, shared by every bot ticking in that ms. A groupless bot's answer
// is its own and serves nobody else.
template <typename T>
class ToCGroupMemo
{
public:
    // fill(T&) rebuilds the answer in place: copy it before calling anything that reads this memo again
    template <typename Fill>
    T const& Get(Player* bot, Fill&& fill, ObjectGuid const& key = ObjectGuid::Empty) const
    {
        ObjectGuid const group = ToCGroupGuid(bot);
        uint32 const now = getMSTime();
        if (group.IsEmpty() || !_valid || _ms != now || _group != group || _key != key)
        {
            _value = T();
            fill(_value);
            _valid = !group.IsEmpty();
            _ms = now;
            _group = group;
            _key = key;
        }

        return _value;
    }

private:
    mutable bool _valid = false;  // 0 is a real getMSTime value
    mutable uint32 _ms = 0;
    mutable ObjectGuid _group;
    mutable ObjectGuid _key;
    mutable T _value{};
};

}

#endif
