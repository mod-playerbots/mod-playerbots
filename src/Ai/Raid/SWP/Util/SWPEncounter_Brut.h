/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_SWPENCOUNTERBRUT_H
#define PLAYERBOTS_SWPENCOUNTERBRUT_H

#include "ObjectGuid.h"
#include "Position.h"
#include "SWPShared.h"
#include <array>
#include <unordered_map>

class Player;
class Unit;

namespace SwpHelpers
{

enum class BrutallusRangedBurnState : uint8
{
    None,
    MovingToInnerLane,
    TraversingInnerLane,
    MovingToBurnPosition,
    AtBurnPosition,
    MovingToOuterLane,
    TraversingOuterLane,
    ReturningToNormalPosition
};

struct BrutallusMeleeRingLayout
{
    float radius;
    uint8 slotCount;
};

inline constexpr uint32 BRUTALLUS_ASSIGNMENT_REBUILD_INTERVAL_MS = 1000;

inline constexpr float BRUTALLUS_ASSIST_TANK_ANGLE_OFFSET = -(2.0f * M_PI / 3.0f);
inline constexpr float BRUTALLUS_TANK_POSITION_RADIUS = 15.0f;
inline constexpr float BRUTALLUS_TANK_ARRIVAL_DISTANCE = 1.0f;
inline constexpr uint8 METEOR_SLASH_SWAP_STACKS = 3;

// Main tank placement. Whatever the pull was, the main tank draws Brutallus onto the anchor
// position, and then stands in at BRUTALLUS_TANK_POSITION_RADIUS on the bearing below.
inline constexpr float BRUTALLUS_ANCHOR_TOLERANCE = 4.0f;
// World angle from Brutallus to the main tank (-52°). Within the tolerance of it, the whole layout
// is on floor and no position is at the crate, wherever on the anchor he stands.
inline constexpr float BRUTALLUS_MAIN_TANK_BEARING = -52.0f * M_PI / 180.0f;
inline constexpr float BRUTALLUS_MAIN_TANK_BEARING_TOLERANCE = 10.0f * M_PI / 180.0f;
// Brutallus stops 20 yd from the center of his victim, so a tank this far past the anchor draws
// him onto it.
inline constexpr float BRUTALLUS_DRAG_DISTANCE = 20.5f;
// Before drawing Brutallus, the main tank lines up on the far side of him from the anchor,
// close enough that he does not follow.
inline constexpr float BRUTALLUS_TANK_LINE_UP_RADIUS = 17.0f;
inline constexpr float BRUTALLUS_TANK_LINE_UP_ANGLE = 12.0f * M_PI / 180.0f;
// Brutallus backs away from a victim within 9.75 yd of his center.
inline constexpr float BRUTALLUS_TANK_MIN_RADIUS = 12.0f;
// The main tank steps forwards only within this angle of Brutallus, and backwards only within
// this angle of straight away from him, so he is never behind the tank (more than 90°).
inline constexpr float BRUTALLUS_TANK_STEER_ANGLE = M_PI / 3.0f;
// The navmesh treats most of the crate as floor, so the main tank's steps keep this far from
// its center.
inline constexpr float BRUTALLUS_CRATE_CLEARANCE = 3.0f;
// Placement ends where the main tank stands if it has not finished by then.
inline constexpr uint32 BRUTALLUS_PLACEMENT_TIMEOUT_MS = 25000;

inline constexpr float BRUTALLUS_SHARED_SAFE_MELEE_ARC_WIDTH = M_PI / 3.0f;
// Concentric arcs behind the boss, innermost first. The spacing is what keeps Burn from
// spreading between neighbors, so melee hold these positions rather than moving when burning.
inline constexpr std::array BRUTALLUS_MELEE_RING_LAYOUTS = {
    BrutallusMeleeRingLayout{ 4.0f, 2 },
    BrutallusMeleeRingLayout{ 8.0f, 3 },
    BrutallusMeleeRingLayout{ 12.0f, 4 },
    BrutallusMeleeRingLayout{ 16.0f, 5 },
};
constexpr uint8 GetBrutallusTotalMeleePositions()
{
    uint8 total = 0;
    for (auto const& ring : BRUTALLUS_MELEE_RING_LAYOUTS)
        total += ring.slotCount;

    return total;
}
// Melee double up if every slot is taken (unlikely since there are 14 and TBC hates melee).
inline constexpr uint8 BRUTALLUS_TOTAL_MELEE_POSITIONS = GetBrutallusTotalMeleePositions();

inline constexpr float BRUTALLUS_RANGED_TANK_OFFSET = 10.0f;
inline constexpr float BRUTALLUS_RANGED_GROUP_ARC_WIDTH = M_PI_2;
inline constexpr uint8 BRUTALLUS_RANGED_POSITIONS_PER_GROUP = 10;
inline constexpr uint8 BRUTALLUS_BURN_PADS_PER_GROUP = 4;
inline constexpr float BRUTALLUS_LANE_OFFSET = 5.0f;
// Ranged double up if every slot is taken (unlikely even though TBC hates melee, as there are 20).
inline constexpr uint8 BRUTALLUS_TOTAL_RANGED_POSITIONS = BRUTALLUS_RANGED_POSITIONS_PER_GROUP * 2;
inline constexpr uint8 BRUTALLUS_TOTAL_BURN_PADS = BRUTALLUS_BURN_PADS_PER_GROUP * 2;
inline constexpr float BRUTALLUS_NORMAL_RANGED_RADIUS =
    BRUTALLUS_TANK_POSITION_RADIUS + BRUTALLUS_RANGED_TANK_OFFSET;
inline constexpr float BRUTALLUS_INNER_LANE_RADIUS =
    BRUTALLUS_NORMAL_RANGED_RADIUS - BRUTALLUS_LANE_OFFSET;
inline constexpr float BRUTALLUS_OUTER_LANE_RADIUS =
    BRUTALLUS_NORMAL_RANGED_RADIUS + BRUTALLUS_LANE_OFFSET;
inline constexpr float BRUTALLUS_BURN_PAD_RADIUS = BRUTALLUS_NORMAL_RANGED_RADIUS;
// Burn moves count a position as reached within 1 yd but step to within this distance of it. A step
// that stopped at 1 yd would end right on the line on level ground and could leave the bot stuck.
inline constexpr float BRUTALLUS_BURN_STEP_STOP_DISTANCE = 0.5f;

// Where Brutallus is held: open floor 19 yd from his spawn, with room for the layout on all sides.
inline Position const BRUTALLUS_ANCHOR_POSITION = { 1472.0f, 594.0f, 22.4f };
inline Position const BRUTALLUS_CRATE_POSITION =  { 1482.8f, 591.4f, 23.4f };

float GetBrutallusMainTankAngle(Unit* brutallus, Player* mainTank);
float GetBrutallusAssistTankAngle(Unit* brutallus, Player* assistTank, float mainTankAngle);
Position GetBrutallusPositionAtAngle(Player* bot, Unit& brutallus, float angle, float radius);
Position GetBrutallusMainTankDestination(Player* bot, Unit& brutallus);
bool GetBrutallusMainTankStep(
    Player* bot, Unit* brutallus, Position const& destination, float& stepX, float& stepY,
    float& stepZ, bool& backwards);
bool IsBrutallusMainTankInPlace(Player* bot, Unit* brutallus);
bool TryGetBrutallusAssignedPositionIndex(Player* bot, uint8& positionIndex);
bool TryGetBrutallusRangedPosition(
    Player* bot, Unit* brutallus, Player* mainTank, Player* assistTank, uint8 rangedIndex,
    float radius, Position& position);
bool TryGetBrutallusBurnPadPosition(
    Player* bot, Unit* brutallus, Player* mainTank, uint8 rangedIndex, float radius,
    Position& position);
bool TryGetBrutallusLaneTraversalPosition(
    Player* bot, Unit* brutallus, float targetX, float targetY, float radius, float currentX,
    float currentY, Position& position);
bool ReleaseBrutallusBurnPad(Player* bot);
bool HasBrutallusBurn(Player* bot);

// Shared encounter state

struct BrutallusEncounterState
{
    std::unordered_map<ObjectGuid, uint8> rangedAssignments;
    std::unordered_map<ObjectGuid, uint8> meleeAssignments;
    std::unordered_map<ObjectGuid, uint8> rangedBurnPadAssignments;
    std::unordered_map<ObjectGuid, BrutallusRangedBurnState> rangedBurnStates;
    uint32 rangedAssignmentRebuildMs = 0;
    uint32 meleeAssignmentRebuildMs = 0;
};

BrutallusEncounterState& BrutallusState(uint32 instanceId);
BrutallusEncounterState* FindBrutallusState(uint32 instanceId);

}

#endif
