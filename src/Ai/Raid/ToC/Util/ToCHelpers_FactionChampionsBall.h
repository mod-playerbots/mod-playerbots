/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RAID_TOCHELPERS_FACTIONCHAMPIONSBALL_H
#define PLAYERBOTS_RAID_TOCHELPERS_FACTIONCHAMPIONSBALL_H

#include "Define.h"
#include "Position.h"

class Player;
class PlayerbotAI;
class Unit;

// Faction Champions positioning: stands, step-offs, the trinket opener, runs, kites and escapes. Every reader
// answers null or false unless the encounter is live, but the pre-pull Demonic Circle and the kite test.
namespace TrialOfTheCrusaderHelpers
{

// Once the kill target has been out, a Cyclone druid stands 20-30 yd from it where it still reaches the pack
// healer nearest it. None while it holds a CC duty.
bool FactionChampionsCycloneStand(PlayerbotAI* botAI, Position& spot);

// Healers and ranged, tanks aside: they move only to dodge, run, kite, step off, CC or get their target in
// range
bool FactionChampionsStaysPut(PlayerbotAI* botAI);

// A melee champion with the member as victim that the member runs from: the member alive and not a tank,
// or a DK whose ice duty it is, a tank DK too. Reads no refresh, so the champion refresh may ask it.
bool FactionChampionsKitesFrom(Player* member, Unit* champion);

// A healer not casting, rooted or stunned, with a melee champion within 8 yd: 10 yd away from them, on the
// floor and clear of Bladestorm and Hellfire. Latched to arrival or 2.5 s, then none for 3 s.
bool FactionChampionsStepOff(PlayerbotAI* botAI, Position& spot);

// Heroic: the bot whose ready instant AoE fear or Frost Nova reaches the most champions with their trinket
// up, 3 or more, casts it where it stands so they burn it. The spell, and the unit to cast it at.
Unit* FactionChampionsOpener(PlayerbotAI* botAI, uint32& spellId);

// Where a bot runs on Nitro Boosts when champions focus it, are about to, or a melee one is in contact: as
// far from the melee on it as the boost goes, still in heal reach. Latched to arrival or the boost's end.
bool FactionChampionsChased(PlayerbotAI* botAI, Position& spot);

// Where a bot hops 15 yd away from a melee champion it kites from, in heal reach. A hard cast blocks the
// kite, so any bot's refresh breaks it once a chaser would reach melee before it lands.
bool FactionChampionsKite(PlayerbotAI* botAI, Position& spot);

// A melee champion the bot kites from is within 45 yd: reaching its target or a heal target would walk it
// back in
bool FactionChampionsKiteHold(PlayerbotAI* botAI);

// The Sprint or Dash rank to cast now, else 0: a rogue or Cat Form druid with an unslowed melee champion it
// kites from within 8 yd and no speed buff as fast on it yet, Nitro Boosts' included
uint32 FactionChampionsSpeedBoost(PlayerbotAI* botAI);

inline constexpr uint32 SPELL_DEMONIC_CIRCLE_SUMMON = 48018;

struct FactionChampionsEscapeCast
{
    uint32 spellId = 0;
    Unit* target = nullptr;
    bool face = false;  // turn to facing first: Blink leaps forward, Disengage back, Dragon's Breath is a cone
    float facing = 0.0f;
};

// A healer or ranged bot with a melee champion on it within 8 yd: its first ready instant escape, CC before
// movement. An AoE never catches a champion our CC or the grip guard holds, and a fear never the kill target.
bool FactionChampionsEscape(PlayerbotAI* botAI, FactionChampionsEscapeCast& cast);

// A warlock parked on its Champions staging stand without its Demonic Circle, or with under a minute left
bool FactionChampionsCircleDue(PlayerbotAI* botAI);

}

#endif
