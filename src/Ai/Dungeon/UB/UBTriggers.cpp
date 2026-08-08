/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "UBTriggers.h"
#include "Playerbots.h"
#include "UBShared.h"

using namespace UnderbogHungarfen;

bool UBFoulSporesTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "hungarfen");
    return boss && boss->HasAura(SPELL_FOUL_SPORES);
}

bool UBSporeCloudDangerTrigger::IsActive()
{
    if (!bot->IsAlive())
        return false;

    GuidVector const& mushrooms = AI_VALUE_REF(GuidVector, "ub mushrooms");
    return GetNearestDangerousMushroom(bot, mushrooms, MushroomDangerRange(bot)) != nullptr;
}

bool UBUnderbatLashTrigger::IsActive()
{
    if (!bot->IsAlive() || botAI->IsTank(bot))
        return false;

    GuidVector const& attackers = AI_VALUE_REF(GuidVector, "attackers");
    return GetNearestUnderbatInLashRange(bot, attackers) != nullptr;
}
