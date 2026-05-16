/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_PALADINHELPER_H
#define _PLAYERBOT_PALADINHELPER_H

#include <initializer_list>

#include "Unit.h"

class Player;

namespace ai::paladin
{
static constexpr uint32 SPELL_HAND_OF_PROTECTION = 1022;
static constexpr uint32 SPELL_HAND_OF_SALVATION = 1038;
static constexpr uint32 SPELL_HAND_OF_FREEDOM = 1044;
static constexpr uint32 SPELL_HAND_OF_SACRIFICE = 6940;

inline bool HasHandFromCaster(Unit* target, Player* caster, std::initializer_list<uint32> spellIds)
{
    if (!target || !caster)
        return false;

    for (uint32 spellId : spellIds)
    {
        if (target->HasAura(spellId, caster->GetGUID()))
            return true;
    }

    return false;
}

inline bool HasAnyPaladinHandFromCaster(Unit* target, Player* caster)
{
    return HasHandFromCaster(target, caster,
        { SPELL_HAND_OF_PROTECTION, SPELL_HAND_OF_SALVATION, SPELL_HAND_OF_FREEDOM, SPELL_HAND_OF_SACRIFICE });
}
}

#endif
