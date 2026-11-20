/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_SWPENCOUNTERTWINS_H
#define PLAYERBOTS_SWPENCOUNTERTWINS_H

#include "EncounterHelpers.h"
#include "ObjectGuid.h"
#include "PlayerbotAI.h"
#include "Position.h"
#include "SharedDefines.h"
#include "SWPShared.h"
#include <array>
#include <optional>
#include <vector>

class Player;
class PlayerbotAI;
class Unit;

namespace SwpHelpers
{

// The twins whose threat cap the bot has reached; empty for a twin it may still attack.
// A completed Conflagration cast whose missile hasn't landed yet.
struct EredarTwinsConflagrationImpact
{
    ObjectGuid targetGuid;
    uint32 expireMs = 0;
};

struct EredarTwinsThreatHolds
{
    ObjectGuid sacrolash;
    ObjectGuid alythess;
};

// Alythess's tank keeps moving out of her Blazes, so a paladin, who holds threat on the move.
inline constexpr EncounterHelpers::ClassRole ALYTHESS_PALADIN_TANK_ROLE = {
    .playerClass = CLASS_PALADIN,
    .requiredSpec = PALADIN_TAB_PROTECTION,
    .role = { "class_role_alythess_paladin_tank", "Alythess paladin tank" },
    .candidate = { "class_role_class_protection_paladin", "Protection Paladin" },
};
// 60 yd covers the twins' room and the balcony above it.
inline constexpr float EREDAR_TWINS_CLASS_ROLE_ANNOUNCE_DISTANCE = 60.0f;
// 150 yd is short of the gauntlet, whose center is 210 yd from Alythess.
inline constexpr float EREDAR_TWINS_CLASS_ROLE_REARM_DISTANCE = 150.0f;

// Used to measure if a bot is on the balcony; deliberately a little below the actual balcony Z.
inline constexpr float EREDAR_TWINS_BALCONY_Z = 50.0f;
// Ranged count as on their balcony position within this distance.
inline constexpr float EREDAR_TWINS_BALCONY_ARRIVAL_DISTANCE = 1.0f;

inline constexpr uint32 EREDAR_TWINS_BLAZE_CACHE_INTERVAL_MS = 200;
// The Blaze trap GO casts 45246, dealing damage within 3 yd of its center.
inline constexpr float BLAZE_HAZARD_RADIUS = 3.0f;
inline constexpr float BLAZE_CLEARANCE = BLAZE_HAZARD_RADIUS + 1.5f;
inline constexpr float BLAZE_SEARCH_RADIUS = 30.0f;
// Moves to the phase 2 stack keep at least this far from every Alythess tank position.
inline constexpr float BLAZE_ROUTE_CLEARANCE = BLAZE_HAZARD_RADIUS + 1.4f;

// Grace period for the tanks to build threat before the rest of the raid opens fire.
inline constexpr uint32 EREDAR_TWINS_DPS_HOLD_MS = 8 * IN_MILLISECONDS;
// DPS cooldowns are held until Sacrolash is at this percent. Eredar Twins is a threat-sensitive
// fight due to Sacrolash dropping threat on tanks and Alythess targeting Conflagration based on
// Sacrolash's threat table.
inline constexpr float EREDAR_TWINS_MAX_DPS_HP_PERCENT = 80.0f;
// Don't exceed this percentage of the tank's threat. For Sacrolash, the limit is based on the
// second highest tank (unless there is only one Sacrolash tank).
inline constexpr float SACROLASH_THREAT_HOLD_RATIO = 0.8f;
inline constexpr float ALYTHESS_THREAT_HOLD_RATIO = 0.9f;
inline constexpr uint32 EREDAR_TWINS_THREAT_HOLDS_CACHE_INTERVAL_MS = 100;

// Conflagration hits players within 8 yd of its target, who then burn players within 8 yd of them.
inline constexpr float CONFLAGRATION_HAZARD_RADIUS = 8.0f;
inline constexpr float CONFLAGRATION_CLEARANCE = CONFLAGRATION_HAZARD_RADIUS + 2.0f;
// Bots wait 300ms to react to Conflagration (to make the action look less artificial).
inline constexpr int32 CONFLAGRATION_DELAY_MS = 300;
// The missile (speed 30) lands up to about 1.5s after the cast ends and hits its target wherever it
// has gone, so the target holds its position this long after the cast.
inline constexpr uint32 CONFLAGRATION_IMPACT_WINDOW_MS = 2 * IN_MILLISECONDS;

inline constexpr uint8 FLAME_TOUCHED_PROTECT_STACKS = 5;
inline constexpr int32 FLAME_SEAR_PROTECT_WINDOW_MS = 2 * IN_MILLISECONDS;

inline Position const ALYTHESS_START_POSITION = { 1819.180f, 625.539f, 33.4038f };
// All tank positions are in Alythess's melee range (4.7-5.15 yd from her) and within Sacrolash's
// Shadow Blades (20 yd), which clears the tank's Flame Touched. Visited in order; the move from the
// last to the first passes 2.56 yd from her center, outside the 2.375 yd radius within which she
// backs off her victim once Empower has grown her from Sacrolash dying (2 yd before).
inline std::array const ALYTHESS_TANK_POSITIONS = {
    Position{ 1820.310f, 630.150f, 33.404f },
    Position{ 1815.800f, 628.800f, 33.404f },
    Position{ 1814.700f, 624.200f, 33.404f },
    Position{ 1818.100f, 621.000f, 33.404f },
    Position{ 1823.040f, 622.130f, 33.404f },
};
// The tank's Blaze patch lands where it stops, and the positions are 4.67 yd apart against a 4.5 yd
// clearance, so it must stop on the position, not short of it on the way in.
inline constexpr float ALYTHESS_TANK_ARRIVAL_DISTANCE = 0.2f;
// The Alythess tank's first position, the one nearest the balcony.
inline constexpr uint8 ALYTHESS_TANK_FIRST_INDEX = 3;

// Jumps from the balcony land within 13 yd of the ranged position, as far as a player's jump down
// the 18 yd drop carries.
// The melee, the Sacrolash tanks, and a ranged bot Sacrolash chases land here while she lives.
inline Position const EREDAR_TWINS_SACROLASH_LANDING_POSITION = { 1808.100f, 616.500f, 33.404f };
// The Alythess tank lands here, and everyone in phase 2.
inline Position const EREDAR_TWINS_ALYTHESS_LANDING_POSITION =  { 1818.600f, 611.000f, 33.404f };

// Phase 1 positions (ranged up top, melee on Sacrolash).
// Both Sacrolash tanks stand here. Her tank backs along the line from her spawn, so she ends
// 4.5 yd short of it, 16.7 yd from Alythess, facing away: melee behind her (up to 4.75 yd) stay
// 12 yd from Alythess, out of reach of their area attacks (8 yd plus her 2.5 yd reach).
inline constexpr float SACROLASH_TANK_ARRIVAL_DISTANCE = 1.0f;
inline Position const SACROLASH_TANK_POSITION =               { 1799.000f, 618.900f, 33.404f };
// The Sacrolash tanks wait here at the pull, on that line, until one of them holds her.
inline Position const SACROLASH_TANK_PULL_POSITION =          { 1807.400f, 622.800f, 33.404f };
inline Position const EREDAR_TWINS_P1_RANGED_POSITION =       { 1808.076f, 603.460f, 51.684f };
// A ranged bot that pulls aggro on Alythess waits here, out of the stack, so Blaze doesn't hit the
// ranged stack. The twins are in LoS from only ~10 yd of the balcony edge; this is its east end.
inline Position const EREDAR_TWINS_P1_RANGED_ASIDE_POSITION = { 1813.000f, 602.000f, 51.610f };
inline Position const EREDAR_TWINS_FLOOR_CONFLAG_POSITION =   { 1807.400f, 635.800f, 33.404f };
inline Position const EREDAR_TWINS_BALCONY_CONFLAG_POSITION = { 1801.133f, 584.456f, 50.696f };

// A bot (not the Alythess tank) holding Alythess waits here on the floor, where her Blazes land
// away from everyone.
inline Position const EREDAR_TWINS_FLOOR_ASIDE_POSITION = { 1817.200f, 616.000f, 33.404f };
inline constexpr float EREDAR_TWINS_STEP_ASIDE_ARRIVAL_DISTANCE = 1.0f;

// Phase 2: everybody but the Alythess tank in one stack, in her melee range on her far side from
// Sacrolash, 4.8 yd from the nearest Alythess tank position, so a Shadow Nova reaches the tank.
inline constexpr float EREDAR_TWINS_P2_STACK_ARRIVAL_DISTANCE = 0.3f;
inline Position const EREDAR_TWINS_P2_STACK_POSITION =      { 1823.900f, 626.900f, 33.404f };
// Hunters' shots fail within their melee range of the target, 6.08 yd from Empowered Alythess's
// center, so they stand just outside it, 1.8 yd from the stack and inside the Nova on it.
inline Position const EREDAR_TWINS_P2_HUNTER_POSITION =     { 1825.430f, 627.790f, 33.404f };
// Phase 2 moves to the stack go round the Alythess tank positions, clear of the patches.
inline Position const EREDAR_TWINS_P2_POSITION_0_WAYPOINT = { 1816.300f, 639.800f, 33.404f };
inline Position const EREDAR_TWINS_P2_STACK_WAYPOINT =      { 1828.800f, 628.400f, 33.404f };
inline Position const EREDAR_TWINS_P2_POSITION_4_WAYPOINT = { 1827.000f, 618.500f, 33.404f };
inline constexpr float EREDAR_TWINS_WAYPOINT_ARRIVAL_DISTANCE = 1.0f;
inline constexpr float EREDAR_TWINS_ROUTE_LEG_TOLERANCE = 0.5f;
// A stray Blaze patch (from a human holding Alythess, or a victim that can't step aside) on a bot's
// phase 2 position sends it to the first of these clear of every patch: 9 yd from Alythess, a
// quarter turn apart, starting on the stack's side. Bots go straight there.
inline std::array const EREDAR_TWINS_P2_FALLBACK_POSITIONS = {
    Position{ 1827.800f, 628.000f, 33.404f },
    Position{ 1816.700f, 634.200f, 33.404f },
    Position{ 1821.700f, 616.900f, 33.404f },
    Position{ 1810.500f, 623.000f, 33.404f },
};
// Burn's 3 yd radius plus the 0.3 yd arrival and a margin.
inline constexpr float EREDAR_TWINS_P2_BLAZE_CLEARANCE = BLAZE_HAZARD_RADIUS + 0.8f;

// Each twin, if it is on the bot's threat list. Alythess also stands for the encounter boss, so
// she is found only while the encounter is in progress.
Unit* FindAlythess(Player* bot);
Unit* FindSacrolash(Player* bot);
Position GetAlythessTankPosition(Unit* alythess, uint8 index);
// The stack, or the hunters' position for a hunter.
Position GetEredarTwinsPhase2Position(Player* bot, Unit* alythess);
// The first clear fallback while a Blaze patch is on the bot's phase 2 position; empty otherwise,
// and while every fallback has a patch too.
std::optional<Position> GetEredarTwinsPhase2Fallback(PlayerbotAI* botAI, Unit* alythess);
// Where a bot holding Alythess waits: on the balcony, or on the floor.
Position GetEredarTwinsStepAsidePosition(Unit* alythess, bool onBalcony);
// A fallback, else the bot's phase 2 position, or the next waypoint while the straight line to it
// crosses the Alythess tank positions.
Position GetEredarTwinsPhase2RouteTarget(PlayerbotAI* botAI, Unit* alythess);
bool UpdateEredarTwinsClassRoleAnnouncement(PlayerbotAI* botAI);
Player* GetAlythessTank(Player* bot);
bool IsAlythessTank(Player* player);
// Alythess is attacking the player, who isn't her tank.
bool IsHoldingAlythessAggro(Player* player, Unit* alythess);
Player* GetSacrolashTank(Player* bot, uint8 index);
bool IsAnySacrolashTank(Player* bot);
bool ShouldHoldAlythessThreat(Player* bot, Unit* alythess);
bool ShouldHoldSacrolashThreat(Player* bot, Unit* sacrolash);
Player* GetSacrolashSecondTank(Player* bot, Unit* sacrolash);
// The first living bot hunter in group order, who misdirects Alythess at the pull.
bool IsAlythessPullMisdirectHunter(Player* bot);
// The first EREDAR_TWINS_DPS_HOLD_MS of Sacrolash's fight, which the tanks spend taking aggro.
bool IsEredarTwinsStartHoldActive(PlayerbotAI* botAI);
EredarTwinsThreatHolds FindEredarTwinsThreatHolds(Player* bot);
std::vector<Position> FindEredarTwinsBlazePositions(Player* bot);
bool IsAlythessTankPositionSafe(PlayerbotAI* botAI, Position const& position);
// The target of a twin's Conflagration cast, once the reaction delay has passed; empty otherwise.
// A Vanish that stops the cast ends it at once.
ObjectGuid GetEredarTwinsConflagrationTargetGuid(Unit* alythess, Unit* sacrolash);
bool IsEredarTwinsConflagrationTarget(Player* bot, Unit* alythess, Unit* sacrolash);
// For the spell listener, when a Conflagration cast completes (a Vanish that stops it never does).
void RecordEredarTwinsConflagrationImpact(Player* target);

// Shared encounter state

struct EredarTwinsEncounterState
{
    std::optional<EredarTwinsConflagrationImpact> conflagrationImpact;
    std::optional<uint32> dpsHoldStartMs;
};

EredarTwinsEncounterState& EredarTwinsState(uint32 instanceId);
EredarTwinsEncounterState* FindEredarTwinsState(uint32 instanceId);

}

#endif
