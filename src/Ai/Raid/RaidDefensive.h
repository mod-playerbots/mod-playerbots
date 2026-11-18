/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RAIDDEFENSIVE_H
#define PLAYERBOTS_RAIDDEFENSIVE_H

#include <string>
#include <vector>

#include "Define.h"
#include "SharedDefines.h"

class Player;
class PlayerbotAI;

// Raid cooldowns and personal buttons for a hit the encounter sees coming: it says when and how hard, this
// says who answers with what. Nothing resolves off the scope's map.

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
    // a raid boss's level, for its resistance roll
    uint8 casterLevel = 83;
    // members who mustn't channel, null for nobody
    bool (*noChannel)(Player*) = nullptr;
};

enum class RaidCover : uint8
{
    None,
    AuraMastery,
    DivineSacrifice,
    AntiMagicZone,
    DivineHymn,
    Tranquility,
};

// A cover goes up from this long before the first damage until this long after it: Aura Mastery and
// Divine Guardian last 6 s, so either one cast in the window still runs past a 5 s hit's last tick.
inline constexpr uint32 RAID_COVER_WINDOW_LEAD_MS = 1500;
inline constexpr uint32 RAID_COVER_WINDOW_LATE_MS = 1000;
// Covers are dealt once the hit would leave this many members under the floor
inline constexpr uint32 RAID_COVER_MIN_UNDER = 3;
// What a member should still have left after the hit, of its max health
inline constexpr float RAID_DEFENSIVE_FLOOR_PCT = 25.0f;
// A personal button goes up from this long before the first damage
inline constexpr uint32 RAID_PERSONAL_LEAD_MS = 1000;

// The raid cooldown this bot casts now, None otherwise, from a deal latched for the hit and cast inside its
// window. Null with no hit coming drops the deal.
RaidCover GetRaidCover(PlayerbotAI* botAI, RaidDefensiveScope const& scope, RaidPredictedHit const* hit);
// The bot's own rank of it, 0 when it doesn't know it
uint32 RaidCoverSpellId(PlayerbotAI* botAI, RaidCover cover);
bool IsRaidCoverChannel(RaidCover cover);
// After a cast went out: Divine Sacrifice comes off again once Divine Guardian is up, since under a hit
// on everyone its 30% redirect drains the paladin.
void FinishRaidCover(PlayerbotAI* botAI, RaidCover cover);

// The paladin is talented into Divine Guardian, so its Divine Sacrifice cuts damage to the whole raid
bool RaidHasDivineGuardian(Player* paladin);

// Before the first damage: the class nodes for the raid cooldowns are held over this, so none goes out
// early or twice.
bool RaidHitLeadIn(RaidPredictedHit const& hit);
std::vector<std::string> RaidCoverClassNodes();

struct RaidPersonalPick
{
    uint32 spellId = 0;
    char const* consumable = nullptr;

    explicit operator bool() const { return spellId || consumable; }
};

// The bot's own button for a hit that would leave it under the floor, from RAID_PERSONAL_LEAD_MS out: its
// class defensive row, else on a lethal hit an immunity or a healthstone.
RaidPersonalPick GetRaidPersonalDefensive(PlayerbotAI* botAI, RaidDefensiveScope const& scope,
                                          RaidPredictedHit const* hit);

#endif
