/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_UBSHARED_H
#define PLAYERBOTS_UBSHARED_H

#include "ObjectGuid.h"

class Creature;
class Player;

namespace UnderbogHungarfen
{
    constexpr uint32 NPC_UNDERBOG_MUSHROOM = 17990;

    constexpr uint32 SPELL_FOUL_SPORES = 31673;
    constexpr uint32 SPELL_GROW        = 31698;
    constexpr uint32 SPELL_SPORE_CLOUD = 34168;

    constexpr float SPORE_CLOUD_RADIUS_FALLBACK = 8.0f;
    constexpr float FOUL_SPORES_RADIUS_FALLBACK = 20.0f;

    constexpr float SPORE_CLOUD_MARGIN = 1.5f;
    constexpr float FOUL_SPORES_MARGIN = 2.0f;

    constexpr uint32 GROW_STACKS_DANGER = 8;

    constexpr float MUSHROOM_SCAN_RANGE = 40.0f;

    float MaxEffectRadius(uint32 spellId, float fallback);

    float MushroomDangerRange(Player* bot);

    bool IsMushroomDangerous(Creature* mushroom);

    GuidVector FindMushroomGuids(Player* bot);

    Creature* GetNearestDangerousMushroom(Player* bot, GuidVector const& mushrooms, float range);

    bool RetreatPathUnsafe(Player* bot, GuidVector const& mushrooms, float destX, float destY);
}

#endif
