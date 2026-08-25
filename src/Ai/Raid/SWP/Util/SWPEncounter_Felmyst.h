/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_SWPENCOUNTERFELMYST_H
#define PLAYERBOTS_SWPENCOUNTERFELMYST_H

#include "ObjectGuid.h"
#include "Position.h"
#include "SWPSharedConstants.h"
#include <array>
#include <limits>
#include <unordered_map>
#include <vector>

class Creature;
class Player;
class Unit;

namespace SwpHelpers
{

enum class FogLane : uint8
{
    None = std::numeric_limits<uint8>::max(),
    Top = 0,
    Middle = 1,
    Bottom = 2,
};

enum class FogPhase : uint8
{
    None,
    Windup,
    Sweep,
    Recovery,
};

enum class FogLocation : uint8
{
    None,
    LeftSide,
    RightSide,
    LeftTop,
    LeftMiddle,
    LeftBottom,
    RightTop,
    RightMiddle,
    RightBottom,
};

enum class FelmystGroundStack : uint8
{
    None = std::numeric_limits<uint8>::max(),
    Melee = 0,
    Left = 1,
    Right = 2,
};

struct FogOfCorruptionState
{
    FogLane lane = FogLane::None;
    FogPhase phase = FogPhase::None;
    uint32 expireMs = 0;
};

struct FogPassState
{
    FogLocation lastDestinationLocation = FogLocation::None;
    FogLane lastCompletedLane = FogLane::None;
    FogLane armedSweepLane = FogLane::None;
    uint8 completedPassCount = 0;
    uint32 thirdPassWindowExpireMs = 0;
};

struct IncomingEncapsulateState
{
    ObjectGuid targetGuid = ObjectGuid::Empty;
    uint32 delayMs = 0;
    uint32 expireMs = 0;
    bool auraObserved = false;
};

struct FelmystEncounterState
{
    IncomingEncapsulateState incomingEncapsulate;
    bool encapsulateOccurredThisGroundPhase = false;
    std::unordered_map<ObjectGuid, uint8> demonicVaporRegionIndices;
    uint8 demonicVaporUsedRegionMask = 0;
    uint8 demonicVaporFirstRegionIndex = 0;
    FogOfCorruptionState fogOfCorruption;
    FogPassState fogPass;
    uint32 landingDpsWaitStartMs = 0;
    uint32 landingTouchdownMs = 0;
    ObjectGuid flightLeaderGuid = ObjectGuid::Empty;
};

extern std::unordered_map<uint32, FelmystEncounterState> felmystEncounterStates;

struct FogSafeThreshold
{
    Position a, b;
    bool safeSideIsNorth;  // true = safe side has higher X (north), false = lower X (south)
};

inline constexpr float FELMYST_RANGED_GROUP_RADIUS = 0.5f;
inline constexpr float FELMYST_LOCATION_MATCH_DISTANCE = 2.0f;

// How close ranged have to be before a charmed player is worth committing to
inline constexpr float FELMYST_CHARMED_TARGET_RANGE = 30.0f;

struct DemonicVaporAnchor
{
    Position position;
    FogLane lane;
    uint8 sideMask;
};

inline constexpr uint8 DEMONIC_VAPOR_LEFT_SIDE = 0x1;
inline constexpr uint8 DEMONIC_VAPOR_RIGHT_SIDE = 0x2;

inline Position const FOG_LEFT_SIDE =  { 1469.064f, 729.585f, 59.824f, 4.677f };
inline Position const FOG_RIGHT_SIDE = { 1458.556f, 502.200f, 59.900f, 1.606f };

inline Position const LEFT_LANDING_POSITION =   { 1476.770f, 665.094f, 20.642f };
inline Position const RIGHT_LANDING_POSITION =  { 1469.930f, 557.009f, 22.632f };
inline Position const CENTER_GROUND_REFERENCE = { 1473.350f, 611.052f, 21.637f };

inline Position const FOG_CRATE_STUCK_POSITION =    { 1484.443f, 591.337f, 23.391f };
inline Position const FOG_CRATE_TELEPORT_POSITION = { 1482.181f, 591.253f, 24.545f };

inline std::array const TANK_POSITIONS = {
    Position{ 1460.145f, 598.290f, 21.869f },
    Position{ 1480.587f, 636.805f, 21.713f },
    Position{ 1479.524f, 584.069f, 23.231f },
};

inline std::array const FOG_LEFT_LANES = {
    Position{ 1494.745f, 704.000f, 50.085f, 4.747f },
    Position{ 1469.923f, 703.239f, 50.086f, 4.747f },
    Position{ 1446.515f, 701.518f, 50.085f, 4.747f },
};

inline std::array const FOG_RIGHT_LANES = {
    Position{ 1492.820f, 515.668f, 50.083f, 1.449f },
    Position{ 1466.732f, 515.595f, 50.572f, 1.449f },
    Position{ 1441.640f, 520.520f, 50.083f, 1.449f },
};

// Note that WoW coordinates are rotated 90° from real-life coordinates
inline std::array const FOG_SAFE_THRESHOLDS = {
    FogSafeThreshold{ // Top lane safe threshold (west→east: safe = south)
        Position{ 1470.122f, 660.345f, 20.462f },
        Position{ 1470.358f, 560.042f, 22.635f },
        false,
    },
    FogSafeThreshold{ // Middle lane safe threshold (west→east: safe = north)
        Position{ 1498.880f, 675.159f, 22.511f },
        Position{ 1497.864f, 546.197f, 26.351f },
        true,
    },
    FogSafeThreshold{ // Bottom lane safe threshold (west→east: safe = north)
        Position{ 1477.381f, 659.824f, 21.051f },
        Position{ 1477.397f, 555.516f, 23.968f },
        true,
    }
};

inline float const LEFT_LANDING_Y = LEFT_LANDING_POSITION.GetPositionY();
inline float const LEFT_LANDING_Z = LEFT_LANDING_POSITION.GetPositionZ();
inline float const RIGHT_LANDING_Y = RIGHT_LANDING_POSITION.GetPositionY();
inline float const RIGHT_LANDING_Z = RIGHT_LANDING_POSITION.GetPositionZ();

inline std::array const DEMONIC_VAPOR_KITE_ANCHORS = {
    DemonicVaporAnchor{
        Position{ 1492.820f, RIGHT_LANDING_Y, RIGHT_LANDING_Z },
        FogLane::Top, DEMONIC_VAPOR_RIGHT_SIDE,
    },
    DemonicVaporAnchor{
        Position{ 1494.745f, LEFT_LANDING_Y, LEFT_LANDING_Z },
        FogLane::Top, DEMONIC_VAPOR_LEFT_SIDE,
    },
    DemonicVaporAnchor{
        Position{ 1466.732f, RIGHT_LANDING_Y, RIGHT_LANDING_Z },
        FogLane::Middle, DEMONIC_VAPOR_RIGHT_SIDE,
    },
    DemonicVaporAnchor{
        Position{ 1469.923f, LEFT_LANDING_Y, LEFT_LANDING_Z },
        FogLane::Middle, DEMONIC_VAPOR_LEFT_SIDE,
    },
    DemonicVaporAnchor{
        Position{ 1441.640f, RIGHT_LANDING_Y, RIGHT_LANDING_Z },
        FogLane::Bottom, DEMONIC_VAPOR_RIGHT_SIDE,
    },
    DemonicVaporAnchor{
        Position{ 1446.515f, LEFT_LANDING_Y, LEFT_LANDING_Z },
        FogLane::Bottom, DEMONIC_VAPOR_LEFT_SIDE,
    }
};

inline std::array const DEMONIC_VAPOR_LANE_REFERENCES = {
    Position{ 1493.783f, 609.834f, 50.084f },
    Position{ 1468.328f, 609.417f, 50.329f },
    Position{ 1444.078f, 611.019f, 50.084f },
};

Position const& GetFelmystMainTankGroundPosition(Player* bot);
bool TryGetFelmystGroundStackPosition(
    Player* bot, Unit* felmyst, FelmystGroundStack stack, Position& position);
FelmystGroundStack GetClosestFelmystGroundStack(Player* bot, Unit* felmyst, Unit* unit);
float GetFelmystFrontAngle(Player* bot, Unit* felmyst);
bool TryGetFelmystRangedPosition(Player* bot, Unit* felmyst, Position& position);
Creature* GetFelmystDemonicVaporSummonedByBot(Player* bot);
bool IsFelmystDemonicVaporHeadNearBot(Player* bot);
std::vector<Creature*> GetDemonicVaporHazards(Player* bot);
void ClearFelmystDemonicVaporKiteState(Player* bot);
bool TryGetFelmystDemonicVaporKiteDestination(Player* bot, Position& destination);
bool TryGetFelmystFogSafeDestination(
    Player* bot, FogLane dangerLane, Position& destination,
    Position const* referencePoint = nullptr);
bool IsFelmystLanding(Unit* felmyst);
bool IsFelmystAirPhaseTargetSuppressed(Unit* felmyst);
bool TryGetFelmystPostThirdPassWindow(Unit* felmyst, FogLane& lane);
bool IsFelmystFogMovementSuppressed(Unit* felmyst);
bool IsFelmystFogActiveForBot(Player* bot, Unit* felmyst);
bool TryGetFelmystFogOfCorruptionStageState(Unit* felmyst, FogOfCorruptionState& state);
bool TryGetActiveFogOfCorruptionState(Player* bot, Unit* felmyst, FogOfCorruptionState& state);
void RecordFelmystIncomingEncapsulateTarget(Player* target, uint32 durationMs = 3000);
Player* GetFelmystEncapsulateTarget(Player* bot);
bool DidEncapsulateOccurThisGroundPhase(Player* bot);
Player* GetFelmystGasNovaDispelTarget(Player* bot);
Player* GetFelmystCharmedTarget(Player* bot, Unit* felmyst);
Player* GetFelmystFlightLeader(Player* player);

}

#endif
