/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_SWPENCOUNTERBRUT_H
#define PLAYERBOTS_SWPENCOUNTERBRUT_H

#include "ObjectGuid.h"
#include "Position.h"
#include "SWPData.h"
#include <unordered_map>

class Player;
class PlayerbotAI;
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

struct BrutallusRangedSlotInfo
{
    bool isMainTankGroup = false;
    uint8 arcPositionIndex = 0;
};

inline constexpr float BRUTALLUS_ASSIST_TANK_ANGLE_OFFSET = -(2.0f * M_PI / 3.0f);
inline constexpr float BRUTALLUS_TANK_POSITION_RADIUS = 15.0f;

inline constexpr float BRUTALLUS_SHARED_SAFE_MELEE_ARC_WIDTH = M_PI / 3.0f;
inline constexpr float BRUTALLUS_INNERMOST_MELEE_RADIUS = 4.0f;
inline constexpr uint8 BRUTALLUS_INNERMOST_MELEE_POSITIONS = 2;
inline constexpr float BRUTALLUS_INNER_MELEE_RADIUS = 8.0f;
inline constexpr uint8 BRUTALLUS_INNER_MELEE_POSITIONS = 3;
inline constexpr float BRUTALLUS_OUTER_MELEE_RADIUS = 12.0f;
inline constexpr uint8 BRUTALLUS_OUTER_MELEE_POSITIONS = 4;
inline constexpr float BRUTALLUS_OUTERMOST_MELEE_RADIUS = 16.0f;
inline constexpr uint8 BRUTALLUS_OUTERMOST_MELEE_POSITIONS = 5;

inline constexpr float BRUTALLUS_RANGED_TANK_OFFSET = 10.0f;
inline constexpr float BRUTALLUS_RANGED_GROUP_ARC_WIDTH = M_PI_2;
inline constexpr uint8 BRUTALLUS_RANGED_POSITIONS_PER_GROUP = 10;
inline constexpr uint8 BRUTALLUS_BURN_PADS_PER_GROUP = 4;
inline constexpr float BRUTALLUS_LANE_OFFSET = 5.0f;

inline constexpr uint8 BRUTALLUS_TOTAL_RANGED_POSITIONS = BRUTALLUS_RANGED_POSITIONS_PER_GROUP * 2;
inline constexpr uint8 BRUTALLUS_TOTAL_BURN_PADS = BRUTALLUS_BURN_PADS_PER_GROUP * 2;
inline constexpr float BRUTALLUS_NORMAL_RANGED_RADIUS =
    BRUTALLUS_TANK_POSITION_RADIUS + BRUTALLUS_RANGED_TANK_OFFSET;
inline constexpr float BRUTALLUS_INNER_LANE_RADIUS =
    BRUTALLUS_NORMAL_RANGED_RADIUS - BRUTALLUS_LANE_OFFSET;
inline constexpr float BRUTALLUS_OUTER_LANE_RADIUS =
    BRUTALLUS_NORMAL_RANGED_RADIUS + BRUTALLUS_LANE_OFFSET;
inline constexpr float BRUTALLUS_BURN_PAD_RADIUS = BRUTALLUS_NORMAL_RANGED_RADIUS;

inline Position const BRUTALLUS_MAIN_TANK_POSITION = { 1483.528f, 595.346f, 23.552f };

extern std::unordered_map<uint32, std::unordered_map<ObjectGuid, uint8>>
    brutallusRangedAssignments;
extern std::unordered_map<uint32, std::unordered_map<ObjectGuid, uint8>>
    brutallusMeleeAssignments;
extern std::unordered_map<uint32, std::unordered_map<ObjectGuid, uint8>>
    brutallusRangedBurnPadAssignments;
extern std::unordered_map<ObjectGuid, BrutallusRangedBurnState> brutallusRangedBurnStates;

float GetBrutallusTankAngle(Unit* brutallus, Player* tank, float fallbackAngle);
float GetBrutallusMainTankAngle(Unit* brutallus);
Position GetBrutallusPositionAtAngle(Player* bot, Unit* brutallus, float angle, float radius);
float GetBrutallusCenteredArcSlotAngleOffset(uint8 slotIndex, uint8 slotCount, float arcWidth);
bool TryGetBrutallusAssignedPositionIndex(Player* bot, uint8& positionIndex);
bool TryGetBrutallusRangedPosition(
    Player* bot, Unit* brutallus, Player* mainTank, Player* assistTank,
    uint8 rangedIndex, float radius, Position& position);
bool TryGetBrutallusBurnPadPosition(
    Player* bot, Unit* brutallus, Player* mainTank,
    uint8 rangedIndex, float radius, Position& position);
bool TryGetBrutallusLaneTraversalPosition(
    Player* bot, Unit* brutallus, float targetX, float targetY, float radius,
    float currentX, float currentY, Position& position);
bool ReleaseBrutallusBurnPad(Player* bot);

}

#endif
