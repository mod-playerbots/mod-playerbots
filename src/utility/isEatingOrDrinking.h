#ifndef IS_DRINKING_H
#define IS_DRINKING_H

#include "SpellAuraDefines.h"
#include "Player.h"

inline bool isEatingOrDrinking(Player* player)
{
	if (!player)
		return false;

    if (!player->IsSitState())
        return false;

    // Drinking = mana regen aura
    bool drinking =
        player->HasAuraType(SPELL_AURA_MOD_POWER_REGEN) ||
        player->HasAuraType(SPELL_AURA_MOD_POWER_REGEN_PERCENT);

    // Eating = health regen aura
    bool eating =
        player->HasAuraType(SPELL_AURA_MOD_REGEN) ||
        player->HasAuraType(SPELL_AURA_MOD_HEALTH_REGEN_PERCENT);

    return drinking || eating;
}

#endif
