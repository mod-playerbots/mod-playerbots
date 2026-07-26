/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "UBShared.h"
#include "Creature.h"
#include "ObjectAccessor.h"
#include "Playerbots.h"
#include "SpellAuras.h"
#include "SpellInfo.h"
#include "SpellMgr.h"

#include <algorithm>
#include <cmath>
#include <list>

namespace UnderbogHungarfen
{
    float MaxEffectRadius(uint32 spellId, float fallback)
    {
        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
        if (!spellInfo)
            return fallback;

        float radius = 0.0f;
        for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
            radius = std::max(radius, spellInfo->Effects[i].CalcRadius());

        return radius > 0.0f ? radius : fallback;
    }

    float MushroomDangerRange(Player* bot)
    {
        return MaxEffectRadius(SPELL_SPORE_CLOUD, SPORE_CLOUD_RADIUS_FALLBACK) + bot->GetCombatReach() +
               SPORE_CLOUD_MARGIN;
    }

    bool IsMushroomDangerous(Creature* mushroom)
    {
        if (!mushroom || !mushroom->IsAlive())
            return false;

        if (mushroom->HasAura(SPELL_SPORE_CLOUD))
            return true;

        Aura* grow = mushroom->GetAura(SPELL_GROW);
        return grow && grow->GetStackAmount() >= GROW_STACKS_DANGER;
    }

    GuidVector FindMushroomGuids(Player* bot)
    {
        std::list<Creature*> mushrooms;
        bot->GetCreatureListWithEntryInGrid(mushrooms, NPC_UNDERBOG_MUSHROOM, MUSHROOM_SCAN_RANGE);

        GuidVector guids;
        for (Creature* mushroom : mushrooms)
        {
            if (mushroom->IsAlive())
                guids.push_back(mushroom->GetGUID());
        }

        return guids;
    }

    Creature* GetNearestDangerousMushroom(Player* bot, GuidVector const& mushrooms, float range)
    {
        Creature* best = nullptr;
        float bestDist = range;
        for (ObjectGuid guid : mushrooms)
        {
            Creature* mushroom = ObjectAccessor::GetCreature(*bot, guid);
            if (!IsMushroomDangerous(mushroom))
                continue;

            float const dist = bot->GetDistance2d(mushroom);
            if (dist <= bestDist)
            {
                bestDist = dist;
                best = mushroom;
            }
        }

        return best;
    }

    namespace
    {
        float PointSegmentDist2d(float px, float py, float ax, float ay, float bx, float by)
        {
            float const dx = bx - ax;
            float const dy = by - ay;
            float const len2 = dx * dx + dy * dy;
            float t = (len2 < 1e-6f) ? 0.0f : ((px - ax) * dx + (py - ay) * dy) / len2;
            t = std::max(0.0f, std::min(1.0f, t));
            return std::hypot(px - (ax + t * dx), py - (ay + t * dy));
        }
    }

    bool RetreatPathUnsafe(Player* bot, GuidVector const& mushrooms, float destX, float destY)
    {
        float const dangerRange = MushroomDangerRange(bot);
        float const startX = bot->GetPositionX();
        float const startY = bot->GetPositionY();

        for (ObjectGuid guid : mushrooms)
        {
            Creature* mushroom = ObjectAccessor::GetCreature(*bot, guid);
            if (!mushroom || !mushroom->IsAlive())
                continue;

            if (mushroom->GetExactDist2d(destX, destY) < dangerRange)
                return true;

            if (IsMushroomDangerous(mushroom) &&
                PointSegmentDist2d(mushroom->GetPositionX(), mushroom->GetPositionY(), startX, startY, destX, destY) <
                    dangerRange)
                return true;
        }

        return false;
    }
}
