/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "NearestUnitsValue.h"

#include "Playerbots.h"

GuidVector NearestUnitsValue::Calculate()
{
    std::list<Unit*> targets;
    FindUnits(targets);

    GuidVector results;
    for (Unit* unit : targets)
    {
        if (AcceptUnit(unit) && (ignoreLos || bot->IsWithinLOSInMap(unit)))
            results.push_back(unit->GetGUID());
    }

    return results;
}
