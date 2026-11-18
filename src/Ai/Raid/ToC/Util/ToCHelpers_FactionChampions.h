/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RAID_TOCHELPERS_FACTIONCHAMPIONS_H
#define PLAYERBOTS_RAID_TOCHELPERS_FACTIONCHAMPIONS_H

#include <functional>
#include <vector>

#include "PlayerbotAI.h"
#include "ToCData.h"

namespace TrialOfTheCrusaderHelpers
{

// Both rosters, since the raid fights the other faction's. The same entries on every difficulty.
enum class ToCFactionChampions : uint32
{
    // Healers (priority kill / crowd-control / interrupt targets)
    NPC_ALLIANCE_DRUID_RESTORATION   = 34469,
    NPC_ALLIANCE_SHAMAN_RESTORATION  = 34470,
    NPC_ALLIANCE_PALADIN_HOLY        = 34465,
    NPC_ALLIANCE_PRIEST_DISCIPLINE   = 34466,
    NPC_HORDE_DRUID_RESTORATION      = 34459,
    NPC_HORDE_SHAMAN_RESTORATION     = 34444,
    NPC_HORDE_PALADIN_HOLY           = 34445,
    NPC_HORDE_PRIEST_DISCIPLINE      = 34447,

    // Damage dealers
    NPC_ALLIANCE_DEATH_KNIGHT        = 34461,
    NPC_ALLIANCE_DRUID_BALANCE       = 34460,
    NPC_ALLIANCE_HUNTER              = 34467,
    NPC_ALLIANCE_MAGE                = 34468,
    NPC_ALLIANCE_PALADIN_RETRIBUTION = 34471,
    NPC_ALLIANCE_PRIEST_SHADOW       = 34473,
    NPC_ALLIANCE_ROGUE               = 34472,
    NPC_ALLIANCE_SHAMAN_ENHANCEMENT  = 34463,
    NPC_ALLIANCE_WARLOCK             = 34474,
    NPC_ALLIANCE_WARRIOR             = 34475,
    NPC_HORDE_DEATH_KNIGHT           = 34458,
    NPC_HORDE_DRUID_BALANCE          = 34451,
    NPC_HORDE_HUNTER                 = 34448,
    NPC_HORDE_MAGE                   = 34449,
    NPC_HORDE_PALADIN_RETRIBUTION    = 34456,
    NPC_HORDE_PRIEST_SHADOW          = 34441,
    NPC_HORDE_ROGUE                  = 34454,
    NPC_HORDE_SHAMAN_ENHANCEMENT     = 34455,
    NPC_HORDE_WARLOCK                = 34450,
    NPC_HORDE_WARRIOR                = 34453,
};

// The mage's only interrupt, one rank
inline constexpr uint32 SPELL_COUNTERSPELL = 2139;
// Cast-time spells get kicked this close to their end, so the champion wastes the cast. A few 100 ms bot
// ticks of slack. Channels go at once.
inline constexpr int32 LATE_INTERRUPT_MS = 600;
// One rank, and damage doesn't break it
inline constexpr uint32 SPELL_CYCLONE = 33786;
// Heroic champions cast it on their first fear, stun, confuse or freeze, then 2 min cooldown
// (boss_faction_champions.cpp IsCCed). It also clears every snare and root.
inline constexpr uint32 SPELL_PVP_TRINKET = 65547;

// True if the entry is any Faction Champion (either faction roster). Pets are not champions.
bool IsFactionChampion(uint32 entry);

// True if the entry is a Faction Champion healer spec (Resto Druid/Shaman, Holy Paladin, Disc Priest)
bool IsFactionChampionHealer(uint32 entry);

// DK, Warrior, Rogue, Enhancement Shaman or Retribution Paladin
bool IsFactionChampionMelee(uint32 entry);

// The raid's one kill target, latched per instance. Moves on its death, combat drop or full immunity, to a
// player's skull, or to a healer gripped out of the pack. Null unless the encounter is live.
Unit* FactionChampionsKillTarget(PlayerbotAI* botAI);

// The champion the kill latch left over immunity and goes back to once it can be hurt again. Null
// unless the encounter is live.
Unit* FactionChampionsSuspendedTarget(PlayerbotAI* botAI);

// First alive, in-combat champion in kill order that accept takes, skipping the kill target, the
// suspended one and any FactionChampionsCcProtected. Null unless the encounter is live.
Unit* FactionChampionsNextTarget(PlayerbotAI* botAI, std::function<bool(Unit*)> const& accept);

// Every alive, in-combat champion in kill order, pets left out. Empty unless the encounter is live.
std::vector<Unit*> FactionChampionsInCombat(PlayerbotAI* botAI);

// Every non-healer bot with no ice duty burns the kill target
bool FactionChampionsFocusBot(PlayerbotAI* botAI);

// Bot tanks. Their Consecration and Death and Decay break CC and roots round them
bool FactionChampionsIsPackTank(Player* player);

// The melee champion this bot DK keeps in Chains of Ice (its ice duty): dealt Ret, Enhancement, Rogue, DK,
// Warrior, the kill target last, each to the nearest free DK. Null unless live.
Unit* FactionChampionsIceTarget(PlayerbotAI* botAI);

bool FactionChampionsHasIceDuty(PlayerbotAI* botAI);

// The member's ice duty champion as of the last refresh, no refresh. Empty for none
ObjectGuid FactionChampionsIceDutyOf(Player* member);

// The champion has an ice duty and that DK's Chains of Ice on it: the snares, peels, CC and danger go
// elsewhere. Once the Chains lapse it is open to them again.
bool FactionChampionsIced(PlayerbotAI* botAI, Unit* champion);

// Heroic, and the champion's PvP trinket is off cooldown
bool FactionChampionsTrinketReady(Unit* champion);

// While live: where the champions landed, and the middle of the pack, which follows it wherever it walks:
// those within 20 yd of the last centre, the kill target left out
bool FactionChampionsPack(PlayerbotAI* botAI, Position& anchor, Position& centre);

// A healer champion gripped off the pack (10+ yd inside 1 s), still 20+ yd off its centre with no champion
// within 12 yd
bool FactionChampionsIsPulled(PlayerbotAI* botAI, Unit* champion);

// The kill target has been pulled out at least once, even if the pack has walked onto it since
bool FactionChampionsKillTargetOut(PlayerbotAI* botAI);

// A champion 20+ yd off the pack centre, not iced, whose victim isn't a tank
bool FactionChampionsIsEscapee(PlayerbotAI* botAI, Unit* champion);

// Live, and a champion that fears is still up: warlock, disc or shadow priest, warrior.
bool FactionChampionsFearWindowActive(PlayerbotAI* botAI);

// This bot's CC duty, the champion and spell id: one caster per champion, never the kill target or one the
// grip guard holds. A cast at one that stops qualifying is interrupted. Null unless the encounter is live.
Unit* FactionChampionsCcTarget(PlayerbotAI* botAI, uint32& spellId);

// The member holds a CC duty
bool FactionChampionsHasCcDuty(PlayerbotAI* botAI, ObjectGuid member);

// A player's CC on it, or a CC or peel duty's spell at it, breaks on damage
bool FactionChampionsCcProtected(PlayerbotAI* botAI, Unit* unit);

// A player's fear, stun or confuse that breaks on damage is on it now, duties and peels aside
bool FactionChampionsHeldByBreakableCc(Unit* unit);

// The spell's own aura ends on a hit, or past its damage cap
bool FactionChampionsBreaksOnDamage(uint32 spellId);

// A player's, pet's, totem's or guardian's periodic damage is on it
bool FactionChampionsCarriesGroupDot(Unit* unit);

// This bot is the mage on duty and the kill target is casting a heal Counterspell would stop: a channel
// at once, a cast in its last LATE_INTERRUPT_MS
bool FactionChampionsCounterspellDuty(PlayerbotAI* botAI);

// This mage gets the Counterspell once the kill target's heal reaches its last LATE_INTERRUPT_MS, so it
// starts no cast meanwhile
bool FactionChampionsCounterspellHold(PlayerbotAI* botAI);

// A ranged dps druid bot that knows Cyclone
bool FactionChampionsCycloneBot(Player* player);

// The champion healer in the pack nearest the kill target: not the kill target, pulled or suspended. Null
// unless live
Unit* FactionChampionsBlobHealer(PlayerbotAI* botAI);

// The melee champion kept stunned, CC'd or disarmed: first alive of rogue, DK, Ret, warrior that isn't
// iced, the kill target, pulled or suspended, one not already held first. Null unless live
Unit* FactionChampionsDangerTarget(PlayerbotAI* botAI);

}

#endif
