/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "NearestCorpsesValue.h"

#include "CellImpl.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"

class AnyDeadUnitInObjectRangeCheck
{
public:
    AnyDeadUnitInObjectRangeCheck(WorldObject const* obj, float /*range*/) : i_obj(obj) {}
    WorldObject const& GetFocusObject() const { return *i_obj; }
    bool operator()(Unit* u) { return !u->IsAlive(); }

private:
    WorldObject const* i_obj;
};

void NearestCorpsesValue::FindUnits(std::list<Unit*>& targets)
{
    AnyDeadUnitInObjectRangeCheck u_check(bot, range);
    Acore::UnitListSearcher<AnyDeadUnitInObjectRangeCheck> searcher(bot, targets, u_check);
    Cell::VisitObjects(bot, searcher, range);
}

bool NearestCorpsesValue::AcceptUnit(Unit* /*unit*/) { return true; }
