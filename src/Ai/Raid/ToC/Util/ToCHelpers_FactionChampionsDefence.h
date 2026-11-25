/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RAID_TOCHELPERS_FACTIONCHAMPIONSDEFENCE_H
#define PLAYERBOTS_RAID_TOCHELPERS_FACTIONCHAMPIONSDEFENCE_H

#include <array>
#include <vector>

#include "EncounterHelpers.h"
#include "PlayerbotAI.h"
#include "Position.h"
#include "ToCData.h"

struct RaidPredictedHit;

namespace TrialOfTheCrusaderHelpers
{

// Faction Champion spells (boss_faction_champions.cpp). These sit in a spell difficulty row, so read
// them through sSpellMgr->GetSpellIdForDifficulty.
inline constexpr uint32 SPELL_UNSTABLE_AFFLICTION = 65812;  // dispelling it silences and hits the dispeller
inline constexpr uint32 SPELL_HELLFIRE = 65816;             // channel, the aura sits on the warlock
inline constexpr uint32 SPELL_HAND_OF_FREEDOM = 68757;      // the script casts the 10H id of row 66115
inline constexpr uint32 SPELL_POWER_WORD_SHIELD = 66099;
inline constexpr uint32 SPELL_RENEW = 66177;
inline constexpr uint32 SPELL_RIPTIDE = 66053;
inline constexpr uint32 SPELL_REJUVENATION = 66065;
inline constexpr uint32 SPELL_LIFEBLOOM = 66093;
inline constexpr uint32 SPELL_REGROWTH = 66067;

// Warlock Death Coil, highest rank first: the peel and the escape both cast it
inline constexpr std::array<uint32, 6> DEATH_COIL_RANKS = {47860, 47859, 27223, 17926, 17925, 6789};

// One id on every difficulty
inline constexpr uint32 SPELL_POLYMORPH = 65801;
inline constexpr uint32 SPELL_FEAR = 65809;
inline constexpr uint32 SPELL_PSYCHIC_SCREAM = 65543;
inline constexpr uint32 SPELL_REPENTANCE = 66008;
inline constexpr uint32 SPELL_HAMMER_OF_JUSTICE_HOLY = 66613;
inline constexpr uint32 SPELL_HAMMER_OF_JUSTICE_RET = 66007;
inline constexpr uint32 SPELL_SILENCE = 65542;
inline constexpr uint32 SPELL_STRANGULATE = 66018;
inline constexpr uint32 SPELL_HEX = 66054;
inline constexpr uint32 SPELL_WYVERN_STING = 65877;
inline constexpr uint32 SPELL_BLADESTORM = 65947;
inline constexpr uint32 SPELL_HAND_OF_PROTECTION = 66009;
inline constexpr uint32 SPELL_DIVINE_SHIELD = 66010;
inline constexpr uint32 SPELL_EARTH_SHIELD = 66063;
inline constexpr uint32 SPELL_HEROISM = 65983;
inline constexpr uint32 SPELL_BLOODLUST = 65980;
inline constexpr uint32 SPELL_AVENGING_WRATH = 66011;
inline constexpr uint32 SPELL_BARKSKIN = 65860;

// The player spell
inline constexpr uint32 SPELL_MASS_DISPEL = 32375;

// The Enhancement Shaman's totems. The raid's own shamans summon the same entries.
inline constexpr uint32 NPC_GROUNDING_TOTEM = 5925;
inline constexpr uint32 NPC_WINDFURY_TOTEM = 6112;
inline constexpr uint32 NPC_TREMOR_TOTEM = 5913;

// Every reader below answers null or false unless Faction Champions is live, and for any bot its
// rule doesn't name. All of them share one refresh per instance per millisecond.

// The group member this bot dispels a counted CC from, and the spell to use. Healers are freed
// first and non-healer dispellers used first; each dispeller takes one member.
Unit* FactionChampionsDispelCcTarget(PlayerbotAI* botAI, char const*& spell);

// Where this bot, the priest on duty, aims Mass Dispel: Hand of Protection or Divine Shield on the kill
// target, else Divine Shield on the suspended one. None near an Unstable Affliction carrier.
Unit* FactionChampionsMassDispelTarget(PlayerbotAI* botAI);

// The champion and spell to purge it with: the kill target's buff for the raid's purger, a pack champion's
// lust, Avenging Wrath or Hand of Freedom for the next one, ice duty champions first.
Unit* FactionChampionsPurgeTarget(PlayerbotAI* botAI, char const*& spell);

// Where a physical attacker goes while the kill target is immune to physical damage only (Hand of
// Protection). Latched raid-wide.
Unit* FactionChampionsPhysicalSwitchTarget(PlayerbotAI* botAI);

// The champion totem this bot kills. Each goes to the nearest non-healer bot within 30 yd holding none, and
// stays with it until the totem dies or the bot dies, loses control or is past 30 yd.
Unit* FactionChampionsTotemTarget(PlayerbotAI* botAI);

// The warlock whose Hellfire this bot interrupts at once, and the spell: one bot per channel, the nearest
// with an interrupt ready that reaches him, ranged before a melee kick.
Unit* FactionChampionsChannelInterruptTarget(PlayerbotAI* botAI, char const*& spell);

// The three below skip a Hellfire for its first 2 s for its interrupter when that's a melee kick, so it
// stays in place to land it.

// Inside an active Bladestorm or Hellfire, plus a yard. Not a Bladestorm whose warrior the bot kites from
// while it can move: the kite runs from him instead.
bool FactionChampionsInAoe(PlayerbotAI* botAI);
// The active Bladestorms and Hellfires at the distance a dodge keeps from them
std::vector<EncounterHelpers::HazardCircle> FactionChampionsAoeClearances(PlayerbotAI* botAI);
// A melee bot whose current target is an AoE source or stands in one, so reaching it walks into it
bool FactionChampionsReachIntoAoe(PlayerbotAI* botAI);

// The target carries Unstable Affliction, so a dispel that removes Magic may hit the dispeller
bool FactionChampionsDispelBackfires(PlayerbotAI* botAI, Unit* target);

// The champion and stand to Death Grip it from, for this bot DK on grip duty: with no pull open, a pulled
// kill target still near the pack or another champion healer. Latched raid-wide.
Unit* FactionChampionsGripTarget(PlayerbotAI* botAI, Position& stand);

// The player targets a champion and types pull: a tank bot's generic pull at it opens the grip chain here
// instead. Reads the Faction Champions stage, not the fight, so it works before the pull.
void FactionChampionsTakePullRequest(PlayerbotAI* botAI);

// For a bot DK when the player types pull: opens the chain on the player's target, a champion. False when
// it's open on that one already.
bool FactionChampionsRequestChain(PlayerbotAI* botAI, Player* requester);

// This bot is a link of the open chain still to grip
bool FactionChampionsChainLink(PlayerbotAI* botAI);

// This link's turn, and the last grip has landed
bool FactionChampionsChainGripDue(PlayerbotAI* botAI);

// The champion this link grips and its stand: its post before its turn (walkOnly, none while staging holds
// it), then its own spot if a grip from there lands it 12+ yd farther from the pack, else 27 yd off it
Unit* FactionChampionsChainGripTarget(PlayerbotAI* botAI, Position& stand, bool& walkOnly);

// A Death Grip went out on target: the turn passes on when this was the turn's link
void FactionChampionsTrackChainGrip(PlayerbotAI* botAI, Unit* target);

// The open request's champion, empty without one
ObjectGuid FactionChampionsChainTarget(Player* bot);

// A DK is on its way to grip the kill target out of the pack, which it hasn't left yet
bool FactionChampionsGripPending(PlayerbotAI* botAI);

// Roots, stuns, fears, confuses, charges or knocks back, itself or one trigger or linked spell down.
// A rooted champion can't take Death Grip's pull, and a root or stun landing mid-flight ends the jump.
bool FactionChampionsSpellHoldsTarget(uint32 spellId);

// Nothing may hold it: the player's pull until 6 s after the last bot DK's grip, a grip duty's target, and
// any champion in the air
bool FactionChampionsGripGuarded(PlayerbotAI* botAI, Unit* champion);

// Some champion is guarded
bool FactionChampionsGripGuardActive(PlayerbotAI* botAI);

// The guard's chain half, no fight refresh: the player's pull, its 6 s tail, or in the air. Safe inside
// the champion refresh.
bool FactionChampionsChainGuarded(Player* bot, Unit* champion);

// A pet autocast to flip, and which way: taunts off on heroic for the fight, holds off while a guard is
// active and back on after it. 0 when nothing needs flipping.
uint32 FactionChampionsPetAutocastToggle(PlayerbotAI* botAI, bool& enable);

// The melee champion this bot snares and the spell id, when its victim kites from it or it is loose off the
// tank and nothing slows it yet. One snarer per champion, never the grip, wall grip or ice duty DK.
Unit* FactionChampionsSnareTarget(PlayerbotAI* botAI, uint32& spellId);

// The melee champion this bot peels and the spell id: the danger champion, the Warrior near 2+ of ours, or
// one close to a victim that's no tank. The nearest free bot with a ready peel takes it, one per champion.
Unit* FactionChampionsPeelTarget(PlayerbotAI* botAI, uint32& spellId);

// A peel duty's spell that a hit breaks is aimed at the champion
bool FactionChampionsBreakablePeelAt(PlayerbotAI* botAI, Unit* champion);

// The group member this bot covers with an external defensive and the spell id: one caster per member with
// enough champions on it, those about to pick it counted. Divine Sacrifice and Lay on Hands go this way too.
Unit* FactionChampionsProtectTarget(PlayerbotAI* botAI, uint32& spellId);

// The corpse this druid bot battle-resurrects and the Rebirth rank: healers then tanks, one druid per corpse,
// none from a druid with 2 or more champions within 8 yd, since the res lands where the druid stands. Never a dps.
Unit* FactionChampionsBattleResTarget(PlayerbotAI* botAI, uint32& spellId);

// 2+ champions on the bot and its last 2 s of loss, run on 4 s, ends under RAID_DEFENSIVE_FLOOR_PCT, or 4+
// on a non-tank, those about to pick it counted: fills the hit for GetRaidPersonalDefensive.
bool FactionChampionsPersonalHit(PlayerbotAI* botAI, RaidPredictedHit& hit);

// Where a bot dodging a Bladestorm or Hellfire goes: the nearest clear ring, the spot in it nearest the
// healers' centre, within 30 yd of another living healer and 8+ yd off every melee champion when one is.
Position FactionChampionsAoeDodgeSpot(PlayerbotAI* botAI);

// The member 4+ champions are about to pick at their next re-pick. Empty for none
ObjectGuid FactionChampionsNextFocus(PlayerbotAI* botAI);

// The ice duty champion this DK casts Chains of Ice on now: its own Chains off it or under 4 s, the champion
// in reach, not CC'd, snare immune or in the air. Chains never pops the trinket.
Unit* FactionChampionsIceChains(PlayerbotAI* botAI, uint32& spellId);

// Where an ice duty DK walks when its Chains of Ice is due but the champion is out of 20 yd or sight: 15 yd
// off it on the DK's side, turned up to 90° for the floor and the AoE.
bool FactionChampionsIceReach(PlayerbotAI* botAI, Position& stand);

// The melee champion this bot DK grips to the wall off a raider, its stand, and the spell to cast now (0 while
// it walks or waits): Death Grip at the stand, then Chains of Ice once it lands. gripped: the grip is out.
Unit* FactionChampionsWallGrip(PlayerbotAI* botAI, Position& stand, uint32& spellId, bool* gripped = nullptr);

// This DK's wall grip went out on champion
void FactionChampionsTrackWallGrip(PlayerbotAI* botAI, Unit* champion);

// The group member this paladin bot puts Hand of Freedom on and the spell id: rooted or snared with a melee
// champion it kites from within 15 yd, healers first. One paladin per member.
Unit* FactionChampionsFreedomTarget(PlayerbotAI* botAI, uint32& spellId);

}

#endif
