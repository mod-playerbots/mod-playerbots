/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "UBActions.h"
#include "Playerbots.h"
#include "UBShared.h"

#include <cmath>

using namespace UnderbogHungarfen;

bool UBRetreatFromFoulSporesAction::Execute(Event /*event*/)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "hungarfen");
    if (!boss)
        return false;

    float const safeDistance = MaxEffectRadius(SPELL_FOUL_SPORES, FOUL_SPORES_RADIUS_FALLBACK) + FOUL_SPORES_MARGIN;
    float const currentDistance = bot->GetDistance2d(boss);
    if (currentDistance >= safeDistance)
        return false;

    float const moveDist = safeDistance - currentDistance + 1.0f;
    float const awayAngle = boss->GetAngle(bot);

    GuidVector const& mushrooms = AI_VALUE_REF(GuidVector, "ub mushrooms");

    for (float delta : { 0.0f, float(M_PI / 8), float(-M_PI / 8), float(M_PI / 4), float(-M_PI / 4),
                         float(3 * M_PI / 8), float(-3 * M_PI / 8), float(M_PI / 2), float(-M_PI / 2) })
    {
        float const angle = awayAngle + delta;
        float dx = bot->GetPositionX() + cos(angle) * moveDist;
        float dy = bot->GetPositionY() + sin(angle) * moveDist;
        float dz = bot->GetPositionZ();
        if (!bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bot->GetPositionX(), bot->GetPositionY(),
                                                            bot->GetPositionZ(), dx, dy, dz))
            continue;

        if (RetreatPathUnsafe(bot, mushrooms, dx, dy))
            continue;

        if (MoveTo(bot->GetMapId(), dx, dy, dz, false, false, true, true,
                   MovementPriority::MOVEMENT_COMBAT))
            return true;
    }

    return MoveAway(boss, moveDist);
}

bool UBVacateSporeCloudAction::Execute(Event /*event*/)
{
    float const dangerRange = MushroomDangerRange(bot);
    GuidVector const& mushrooms = AI_VALUE_REF(GuidVector, "ub mushrooms");
    Creature* mushroom = GetNearestDangerousMushroom(bot, mushrooms, dangerRange);
    if (!mushroom)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "hungarfen");
    if (botAI->IsTank(bot) && boss && boss->GetVictim() == bot)
    {
        float const currentDistance = bot->GetDistance2d(mushroom);
        return MoveAway(mushroom, dangerRange - currentDistance + 2.0f);
    }

    return FleePosition(mushroom->GetPosition(), dangerRange);
}
