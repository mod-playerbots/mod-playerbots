/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RAID_TOCHELPERS_JARAXXUS_H
#define PLAYERBOTS_RAID_TOCHELPERS_JARAXXUS_H

#include <string>
#include <vector>

#include "PlayerbotAI.h"
#include "Position.h"
#include "RaidDefensive.h"
#include "ToCData.h"

namespace TrialOfTheCrusaderHelpers
{

inline constexpr uint32 SPELL_LEGION_FLAME_TRAIL = 66199;   // row 66199/68126/68127/68128
inline constexpr uint32 SPELL_MISTRESS_KISS = 66334;        // row 66334/67905/67906/67907
inline constexpr uint32 SPELL_BURNING_INFERNO = 66242;      // row 66242/67059/67060/67061
// The portal's own aura, whose periodic tick summons a Mistress: one at 8 s on normal, every 6 s on heroic
inline constexpr uint32 SPELL_NETHER_PORTAL_SPAWN = 66263;  // row 66263/67103/67104/67105

// Display 29615, scale 1
inline constexpr float JARAXXUS_COMBAT_REACH = 5.0f;

// Incinerate Flesh's last tick casts Burning Inferno, whose own ticks start a second later
inline constexpr uint32 JARAXXUS_INFERNO_FIRST_TICK_MS = 1000;
inline constexpr uint32 JARAXXUS_INFERNO_TICKS = 5;
// An Incinerate Flesh this close to running out with absorb left is a Burning Inferno coming
inline constexpr uint32 JARAXXUS_INFERNO_LEAD_MS = 2000;
// A carrier under this share of its health counts the rest of its trail as a hit
inline constexpr float JARAXXUS_CARRIER_DEFENSIVE_PCT = 50.0f;
// A carrier's next spot stays this close to a living healer
inline constexpr float JARAXXUS_CARRIER_HEALER_REACH = 30.0f;

// A Mistress' arrival hits everyone within 10 yd of her portal (66264 effect 1, a creature's area, centre
// to centre). Melee step out from this, to this, from this long before it.
inline constexpr float JARAXXUS_PORTAL_TRIGGER = 12.0f;
inline constexpr float JARAXXUS_PORTAL_CLEAR = 14.0f;
inline constexpr uint32 JARAXXUS_PORTAL_SPAWN_LEAD_MS = 1000;

// DPS bots stop on him at this share of his tank victim's threat, resume under the release share
inline constexpr float JARAXXUS_THREAT_CAP = 0.9f;
inline constexpr float JARAXXUS_THREAT_RELEASE = 0.8f;

inline constexpr float JARAXXUS_LEGION_FLAME_RADIUS = 3.0f;       // 66877 row, centre to centre
inline constexpr float JARAXXUS_LEGION_FLAME_TRIGGER = 4.0f;      // dodge starts inside this
inline constexpr float JARAXXUS_LEGION_FLAME_CLEAR = 6.0f;        // a spot counts clear of a flame past this
inline constexpr float JARAXXUS_LEGION_FLAME_SEARCH = 15.0f;      // sweep maxRadius
inline constexpr float JARAXXUS_FLAME_CARRIER_MIN_LEG = 6.0f;     // carrier travel floor per leg
inline constexpr float JARAXXUS_FLAME_CARRIER_RAID_CLEAR = 8.0f;  // carrier spot from other members
inline constexpr float JARAXXUS_FLAME_CARRIER_BOSS_CLEAR = 15.0f; // carrier spot from the boss
inline constexpr float JARAXXUS_FLAME_ARRIVE = 1.5f;
// Behind, flank and formation moves pick spots blind to the flames, so they stay off this close to one
inline constexpr float JARAXXUS_FLAME_STAND_REACH = 10.0f;
// A dodge spot stays this far inside the bot's reach of its target, or the generic reach walks it back in
inline constexpr float JARAXXUS_DODGE_REACH_MARGIN = 0.5f;
inline constexpr float JARAXXUS_SCAN_RADIUS = 200.0f;
inline constexpr uint32 JARAXXUS_SCAN_MS = 200;

enum class JaraxxusFlameRole : uint8
{
    None,
    Carrier,
    Dodge
};

// Every reader below gives its empty answer off map 649.

// Alive and attackable, so null through the intro and while a retry waits chained for the gossip
Unit* GetJaraxxus(PlayerbotAI* botAI);

bool JaraxxusHasNetherPower(Unit* jaraxxus);
// Stack count of whichever Nether Power id he carries, 0 without him
uint32 JaraxxusNetherPowerStacks(PlayerbotAI* botAI);
// Mage always, shaman or priest when not healing. A healing one only when no other remover is alive
// and nobody carries Incinerate Flesh.
bool JaraxxusIsNetherPowerRemover(Player* bot);

// Incinerate Flesh heal absorb on the unit, under the map difficulty's id
bool HasIncinerateFlesh(Unit* unit);
// Lowest guid alive group member carrying it
Unit* GetIncinerateFleshTarget(PlayerbotAI* botAI);

// The raid's Burning Inferno, running on a member or coming from an Incinerate Flesh within
// JARAXXUS_INFERNO_LEAD_MS of running out with absorb left. Nether Power counts in the damage.
bool GetJaraxxusBurningInferno(PlayerbotAI* botAI, RaidPredictedHit& hit);
// The same hit as this bot takes it once its own Burning Inferno runs, else the raid's; and the rest of
// the trail on a Legion Flame carrier under JARAXXUS_CARRIER_DEFENSIVE_PCT.
bool GetJaraxxusPersonalHit(PlayerbotAI* botAI, RaidPredictedHit& hit);
// Before a predicted Burning Inferno lands: when the class nodes for the raid cooldowns are held
bool JaraxxusRaidCoverHeld(PlayerbotAI* botAI);
// A Burning Inferno coming or running, or the bot carrying Legion Flame: what Evasion and Deterrence
// would be spent on
bool JaraxxusMagicHitOnBot(PlayerbotAI* botAI);
// A group bot holding a raid cover inside its window, mid cast: it runs no triggers until the cast ends
bool JaraxxusRaidCoverPinned(Player* member);

// Legion Flame's 2 s warning or its 6 s trail, either one means the flames follow this unit
bool IsLegionFlameCarrier(Unit* unit);
bool HasMistressKiss(Unit* unit);

// Nether Portal, Infernal Volcano, Mistress of Pain or Felflame Infernal
bool IsJaraxxusAdd(Unit* unit);
// Kill order, lowest guid first within a kind: selectable portal, selectable volcano, Mistress,
// Infernal.
Unit* GetJaraxxusFocusAdd(PlayerbotAI* botAI);
// Index 0 takes Mistresses, index 1 Infernals, and each falls back to the other's kind (index 0 only
// with no living index 1 tank).
Unit* GetJaraxxusAssistTankAdd(PlayerbotAI* botAI, uint8 index);

// The heroic portal a melee DPS bot on or near it keeps JARAXXUS_PORTAL_CLEAR from, from just before its
// next Mistress until each of its Mistresses has a victim.
Unit* GetJaraxxusPortalToClear(PlayerbotAI* botAI);
// A spot JARAXXUS_PORTAL_CLEAR from the portal and clear of flames, nearest him
bool DeriveJaraxxusPortalExit(Player* bot, Unit* portal, Position& spot);

// Latched per DPS bot: capped at JARAXXUS_THREAT_CAP of his tank victim's threat, freed under the
// release share. Nothing while his victim isn't a tank.
bool JaraxxusThreatCapped(Player* bot);

// The tank holds' trigger tests. The tank targeting rule reads the same calls, so it never zeroes
// tank assist for a tank no hold covers.
bool JaraxxusMainTankHolds(PlayerbotAI* botAI);
bool JaraxxusAssistTankHolds(PlayerbotAI* botAI, uint8 index);

// DPS bots only, while the gate is open (his adds outlive the kill): a player's skull on him or an
// add, else the kill-order add, else him once in combat.
Unit* GetJaraxxusDpsTarget(PlayerbotAI* botAI);

bool JaraxxusIsFelFireballCasting(Unit* boss);
// First interrupt the bot can cast on him right now, can pay for, and reaches from where it stands.
// Null for a bot mid-cast or without a bot AI.
char const* JaraxxusReadyInterrupt(Player* bot, Unit* boss);
// Fel Fireball being cast, the bot ready, and no living group member with a lower guid ready
bool JaraxxusIsFelFireballInterrupter(Player* bot, Unit* boss);

// Leaves the bot in UNIT_STATE_CASTING: a cast time, or a channel unless withChannels is false.
// Unknown names answer false.
bool JaraxxusSpellHasCastTime(PlayerbotAI* botAI, std::string const& spell, bool withChannels = true);

// Group bots mid-cast while kissed, carrying Legion Flame or due a raid cover, read once per group per ms.
// A casting bot runs no triggers, so another bot's tick has to break the cast for it.
std::vector<Player*> GetJaraxxusPinnedCasters(Player* bot);

JaraxxusFlameRole GetJaraxxusFlameRole(Player* bot);
// Bearing from the boss (ARENA_CENTER without him) to the bot, so the trail leads away from him
float GetLegionFlameCarrierHeading(Player* bot);
// One carrier leg, near a healer and off heroic portals where it can, or one dodge, in reach of its target
// and off its held portal where it can.
bool DeriveLegionFlameSpot(Player* bot, JaraxxusFlameRole role, float heading, Position& spot);
bool IsLegionFlameSpotClear(PlayerbotAI* botAI, Position const& spot);
// The straight walk from the bot to `to` comes nearer a flame than JARAXXUS_LEGION_FLAME_CLEAR, or
// nearer than the bot already stands to one inside that
bool LegionFlameCrossesPath(Player* bot, Position const& to);
// A spot at most maxStep away, clear of every flame and walked to without crossing one, nearest anchor
// on the first ring that has one
bool DeriveLegionFlameDetour(Player* bot, Position const& anchor, float maxStep, Position& spot);
// Any Legion Flame on the floor
bool JaraxxusAnyLegionFlame(PlayerbotAI* botAI);
// A Legion Flame within JARAXXUS_FLAME_STAND_REACH of the bot
bool JaraxxusFlameNear(PlayerbotAI* botAI);

}

#endif
