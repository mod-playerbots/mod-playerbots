/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ZGHELPERS_H
#define PLAYERBOTS_ZGHELPERS_H

#include "Define.h"

class Player;

namespace ZulGurubHelpers
{
constexpr uint32 ZG_MAP_ID = 309;

enum class ZulGurubSpells : uint32
{
    // Hakkar
    SPELL_POISONOUS_BLOOD = 24321,

    // Bloodlord Mandokir
    SPELL_THREATENING_GAZE = 24314,
};

enum class ZulGurubNpcs : uint32
{
    // High Priest Thekal
    NPC_HIGH_PRIEST_THEKAL = 14509,
    NPC_ZEALOT_LORKHAN     = 11347,
    NPC_ZEALOT_ZATH        = 11348,

    // Bloodlord Mandokir
    NPC_BLOODLORD_MANDOKIR = 11382,
};

// A member that goes down is restored to full health 10 s later (a zealot's death also restores Thekal) unless
// all three are down by then.
// At 15% a member can be finished inside that window.
constexpr float THEKAL_TRIO_SYNC_HEALTH_PCT = 15.0f;

// Mandokir charges the gazed player if they cause him any threat from the start of his Threatening Gaze cast until
// the aura ends.
bool IsGazedByMandokir(Player* bot);
}  // namespace ZulGurubHelpers

#endif
