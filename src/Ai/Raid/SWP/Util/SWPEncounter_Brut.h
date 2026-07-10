/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef PLAYERBOTS_SWPENCOUNTERBRUT_H
#define PLAYERBOTS_SWPENCOUNTERBRUT_H

#include <unordered_map>

#include "ObjectGuid.h"
#include "Position.h"
#include "SWPData.h"

class Player;
class PlayerbotAI;
class Unit;

namespace SunwellHelpers
{

struct BrutallusRangedSlotInfo
{
    bool isMainTankGroup = false;
    uint8 arcPositionIndex = 0;
};

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

extern Position const BRUTALLUS_MAIN_TANK_POSITION;
constexpr float BRUTALLUS_ASSIST_TANK_ANGLE_OFFSET = -(2.0f * M_PI / 3.0f);
constexpr float BRUTALLUS_TANK_POSITION_RADIUS = 15.0f;
constexpr float BRUTALLUS_RANGED_TANK_OFFSET = 10.0f;
constexpr float BRUTALLUS_LANE_OFFSET = 5.0f;
constexpr float BRUTALLUS_NORMAL_RANGED_RADIUS =
    BRUTALLUS_TANK_POSITION_RADIUS + BRUTALLUS_RANGED_TANK_OFFSET;
constexpr float BRUTALLUS_INNER_LANE_RADIUS =
    BRUTALLUS_NORMAL_RANGED_RADIUS - BRUTALLUS_LANE_OFFSET;
constexpr float BRUTALLUS_OUTER_LANE_RADIUS =
    BRUTALLUS_NORMAL_RANGED_RADIUS + BRUTALLUS_LANE_OFFSET;
constexpr float BRUTALLUS_BURN_PAD_RADIUS = BRUTALLUS_NORMAL_RANGED_RADIUS;
constexpr float BRUTALLUS_SHARED_SAFE_MELEE_ARC_WIDTH = M_PI / 3.0f;
constexpr float BRUTALLUS_INNERMOST_MELEE_RADIUS = 4.0f;
constexpr uint8 BRUTALLUS_INNERMOST_MELEE_POSITIONS = 2;
constexpr float BRUTALLUS_INNER_MELEE_RADIUS = 8.0f;
constexpr uint8 BRUTALLUS_INNER_MELEE_POSITIONS = 3;
constexpr float BRUTALLUS_OUTER_MELEE_RADIUS = 12.0f;
constexpr uint8 BRUTALLUS_OUTER_MELEE_POSITIONS = 4;
constexpr float BRUTALLUS_OUTERMOST_MELEE_RADIUS = 16.0f;
constexpr uint8 BRUTALLUS_OUTERMOST_MELEE_POSITIONS = 5;
constexpr float BRUTALLUS_RANGED_GROUP_ARC_WIDTH = M_PI_2;
constexpr uint8 BRUTALLUS_RANGED_POSITIONS_PER_GROUP = 10;
constexpr uint8 BRUTALLUS_TOTAL_RANGED_POSITIONS = BRUTALLUS_RANGED_POSITIONS_PER_GROUP * 2;
constexpr uint8 BRUTALLUS_BURN_PADS_PER_GROUP = 4;
constexpr uint8 BRUTALLUS_TOTAL_BURN_PADS = BRUTALLUS_BURN_PADS_PER_GROUP * 2;

extern std::unordered_map<uint32, std::unordered_map<ObjectGuid, uint8>>
    brutallusRangedAssignments;
extern std::unordered_map<uint32, std::unordered_map<ObjectGuid, uint8>>
    brutallusRangedBurnPadAssignments;
extern std::unordered_map<ObjectGuid, BrutallusRangedBurnState> brutallusRangedBurnStates;

float GetBrutallusMainTankAngle(Unit* brutallus);
Position GetBrutallusPositionAtAngle(Player* bot, Unit* brutallus, float angle, float radius);
float GetCenteredArcSlotAngleOffset(uint8 slotIndex, uint8 slotCount, float arcWidth);
bool TryGetBrutallusAssignedPositionIndex(
    PlayerbotAI* botAI, Player* bot, bool wantRanged, uint8& positionIndex);
void EnsureBrutallusRangedAssignments(PlayerbotAI* botAI, Player* bot);
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
