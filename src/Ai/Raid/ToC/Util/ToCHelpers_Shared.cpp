/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "ToCHelpers_Shared.h"

#include <algorithm>

#include "CellImpl.h"
#include "Playerbots.h"
#include "Creature.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "Group.h"
#include "Player.h"
#include "Unit.h"

namespace TrialOfTheCrusaderHelpers
{
namespace
{
class CreaturesOfEntrySetInRange
{
public:
    CreaturesOfEntrySetInRange(WorldObject const* object, ToCEntrySet const& entries, float maxRange)
        : _object(object), _entries(entries), _range(maxRange)
    {
    }

    bool operator()(Unit* unit) const
    {
        return _entries.Contains(unit->GetEntry()) && _object->IsWithinDist(unit, _range, false);
    }

private:
    WorldObject const* _object;
    ToCEntrySet const& _entries;
    float _range;
};
}  // namespace

ToCEntrySet::ToCEntrySet(std::vector<uint32> const& entries)
{
    if (entries.empty())
        return;

    auto const [low, high] = std::minmax_element(entries.begin(), entries.end());
    _base = *low;
    _bits.assign(static_cast<size_t>(*high - *low) + 1, false);
    for (uint32 entry : entries)
        _bits[entry - _base] = true;
}

void GetCreatureListWithEntriesInGrid(WorldObject const* origin, std::vector<Creature*>& creatures,
                                      ToCEntrySet const& entries, float radius)
{
    CreaturesOfEntrySetInRange check(origin, entries, radius);
    Acore::CreatureListSearcher<CreaturesOfEntrySetInRange> searcher(origin, creatures, check);
    Cell::VisitObjects(origin, searcher, radius);
}

ObjectGuid ToCGroupGuid(Player* bot)
{
    Group* group = bot ? bot->GetGroup() : nullptr;
    return group ? group->GetGUID() : ObjectGuid::Empty;
}

}
