/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RAIDDEFENSIVE_H
#define PLAYERBOTS_RAIDDEFENSIVE_H

#include "Define.h"
#include "SharedDefines.h"

class PlayerbotAI;

// Personal buttons spent ahead of a hit the encounter sees coming. The encounter says when the hit lands
// and what it does, this says what the bot answers it with. Nothing resolves off the scope's map.

struct RaidDefensiveScope
{
    uint32 mapId;
};

struct RaidPredictedHit
{
    // getMSTime() of its first damage, already past once it's running
    uint32 startMs = 0;
    // first damage to last
    uint32 durationMs = 0;
    // per player, what is still to come of it, before resistance and damage taken modifiers
    uint32 damage = 0;
    // damage read off the member's own health loss, so resistance and running cuts are already in it
    bool measured = false;
    // four or more enemies on the bot, now or at their next pick
    bool focus = false;
    // the bot holds an enemy that picks by victim: an immunity drops it from that pick, so only a lethal
    // hit gets one
    bool keepVictim = false;
    SpellSchoolMask school = SPELL_SCHOOL_MASK_NONE;
    uint8 casterLevel = 83;
};

// What a member should still have left after the hit, of its max health
inline constexpr float RAID_DEFENSIVE_FLOOR_PCT = 25.0f;
// A personal button goes up from this long before the first damage
inline constexpr uint32 RAID_PERSONAL_LEAD_MS = 1000;

struct RaidPersonalPick
{
    uint32 spellId = 0;
    char const* consumable = nullptr;

    explicit operator bool() const { return spellId || consumable; }
};

// The bot's own answer to a hit that would leave it under the floor, from RAID_PERSONAL_LEAD_MS before
// the first damage: NextMagicDefensive's row for what is left of the hit (NextPhysicalDefensive's, then
// Evasion, Deterrence or Pain Suppression, for a physical one), and on a lethal one with no row
// Dispersion, the class immunity, then a healthstone at 70% or under. A non-tank paladin on a measured
// hit takes Divine Shield first, a mage on a focus hit Ice Block, the GCD or a cast in progress
// notwithstanding. Nothing while one runs or while it channels Divine Hymn or Tranquility.
RaidPersonalPick GetRaidPersonalDefensive(PlayerbotAI* botAI, RaidDefensiveScope const& scope,
                                          RaidPredictedHit const* hit);

#endif
