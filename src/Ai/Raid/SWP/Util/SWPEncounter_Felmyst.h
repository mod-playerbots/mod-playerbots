/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_SWPENCOUNTERFELMYST_H
#define PLAYERBOTS_SWPENCOUNTERFELMYST_H

#include "ObjectGuid.h"
#include "Position.h"
#include "SWPShared.h"
#include <array>
#include <limits>
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

enum class FelmystFlightState : uint8
{
    Unknown,
    Flying,
    Grounded,
};

// The end of the courtyard whose air-phase plan is used.
enum class FelmystAirEnd : uint8
{
    None,
    West,
    East,
};

enum class FelmystAirStep : uint8
{
    Step,
    PatchesOnly,
    Blocked,
};

struct IncomingEncapsulateState
{
    ObjectGuid targetGuid = ObjectGuid::Empty;
    uint32 delayMs = 0;
    uint32 expireMs = 0;
    bool auraObserved = false;
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

struct FogSafeThreshold
{
    Position a, b;
    bool safeSideIsNorth;  // true = safe side has higher X (north), false = lower X (south)
};

struct FelmystDemonicVaporGuids
{
    GuidVector heads;
    GuidVector trails;
};

inline constexpr uint32 DEMONIC_VAPOR_CACHE_INTERVAL_MS = 200;
inline constexpr float DEMONIC_VAPOR_SEARCH_RADIUS = 100.0f;
// A trail patch's reach, center to center.
inline constexpr float DEMONIC_VAPOR_HAZARD_RADIUS = 3.0f;
// How far bots keep from a patch or another player's head, except in the fog move.
inline constexpr float DEMONIC_VAPOR_CLEARANCE = DEMONIC_VAPOR_HAZARD_RADIUS + 2.0f;
inline constexpr float FOG_MOVE_VAPOR_CLEARANCE = DEMONIC_VAPOR_HAZARD_RADIUS + 1.0f;
inline constexpr float FELMYST_SIDESTEP_SEARCH_RADIUS = 15.0f;
// The group stands within this distance of its position in the air.
inline constexpr float FELMYST_GROUP_ARRIVAL_DISTANCE = 5.0f;
// A vapor target turns to the next point of its route within this distance of the current one.
inline constexpr float FELMYST_ROUTE_POINT_ARRIVAL_DISTANCE = 2.0f;
// Before takeoff, a bot farther than this from G0 leaves early by what the excess needs at
// FELMYST_EARLY_MOVE_SPEED.
inline constexpr float FELMYST_EARLY_MOVE_FREE_DISTANCE = 35.0f;
inline constexpr float FELMYST_EARLY_MOVE_SPEED = 7.0f;
inline constexpr uint32 FELMYST_TAKEOFF_AFTER_TOUCHDOWN_MS = 60 * IN_MILLISECONDS;
// The lane is read once Felmyst is this far from the side point she is leaving.
inline constexpr float FELMYST_LANE_READ_DISTANCE = 5.0f;
// The fog move stops this far past a lane's threshold line; a bot is clear of the lane from
// FOG_CLEAR_PAST_THRESHOLD past it.
inline constexpr float FOG_STOP_PAST_THRESHOLD = 3.0f;
inline constexpr float FOG_CLEAR_PAST_THRESHOLD = 2.5f;

// How close to the center of a ground stack a bot stops, center to center.
inline constexpr float FELMYST_RANGED_GROUP_RADIUS = 2.0f;
// Bots wait this long to react to Encapsulate (to make the action look less artificial).
inline constexpr uint32 ENCAPSULATE_DELAY_MS = 300;
// Encapsulate (45662) hits friendly units within this distance of its target, center to center.
inline constexpr float ENCAPSULATE_RADIUS = 20.0f;
inline constexpr float ENCAPSULATE_CLEARANCE = ENCAPSULATE_RADIUS + 2.0f;
// How close ranged have to be to a player charmed by Fog to attack.
inline constexpr float FELMYST_CHARMED_TARGET_RANGE = 30.0f;
// How long after landing following a flight phase does the raid keeps holding DPS.
inline constexpr uint32 FELMYST_GROUNDED_DPS_WAIT_MS = 3 * IN_MILLISECONDS;
// How close Felmyst must be to a specified position to be considered there.
inline constexpr float FELMYST_LOCATION_MATCH_DISTANCE = 2.0f;
// How far a fog destination must be from any vapor. Vapor has a 3 yd radius and no real buffer is
// needed since this distance is for a non-moving bot, but the visual looks much wider than 3 yd
// so a bit of leeway allows for some more realism as players would not stand so close.
inline constexpr float FOG_DESTINATION_VAPOR_CLEARANCE = 8.0f;
inline constexpr uint8 TOTAL_FOG_LANES = 3;
// Felmyst lands after this many passes.
inline constexpr uint8 FOG_PASSES_PER_AIR_PHASE = 3;

inline Position const FOG_LEFT_SIDE =  { 1469.064f, 729.585f, 59.824f, 4.677f };
inline Position const FOG_RIGHT_SIDE = { 1458.556f, 502.200f, 59.900f, 1.606f };

inline Position const FELMYST_LEFT_LANDING_POSITION =  { 1476.770f, 665.094f, 20.642f };
inline Position const FELMYST_RIGHT_LANDING_POSITION = { 1469.930f, 557.009f, 22.632f };

// The navmesh treats most of the crate as floor, so stepped moves keep this far from its center.
inline Position const FELMYST_CRATE_POSITION = { 1483.0f, 591.0f, 23.4f };
inline constexpr float FELMYST_CRATE_CLEARANCE = 4.0f;

inline std::array const FELMYST_TANK_POSITIONS = {
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

// Note that WoW coordinates are rotated 90° from real-life coordinates.
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

// Air phase: G0 at takeoff, G1 after the first vapor, G2 after the second.
inline std::array const FELMYST_WEST_GROUP_POSITIONS = {
    Position{ 1480.0f, 649.0f, 21.4f },
    Position{ 1480.0f, 634.0f, 22.3f },
    Position{ 1480.0f, 619.0f, 23.5f },
};
inline std::array const FELMYST_EAST_GROUP_POSITIONS = {
    Position{ 1473.0f, 582.0f, 22.6f },
    Position{ 1473.0f, 597.0f, 23.1f },
    Position{ 1473.0f, 612.0f, 23.3f },
};

// The routes vapor targets follow: the first vapor's loops north, the second's south, both toward
// the near end. The last point is far enough away that the beam always ends first.
inline std::vector<Position> const FELMYST_WEST_FIRST_VAPOR_ROUTE = {
    Position{ 1484.9f, 653.9f, 21.6f },
    Position{ 1488.5f, 657.5f, 21.7f },
    Position{ 1494.5f, 660.8f, 22.5f },
    Position{ 1501.4f, 660.2f, 24.9f },
    Position{ 1506.8f, 655.9f, 25.9f },
    Position{ 1509.0f, 649.3f, 26.3f },
    Position{ 1507.2f, 642.7f, 26.2f },
    Position{ 1502.0f, 638.1f, 25.3f },
    Position{ 1483.4f, 621.5f, 23.8f },
};
inline std::vector<Position> const FELMYST_WEST_SECOND_VAPOR_ROUTE = {
    Position{ 1475.1f, 638.9f, 21.7f },
    Position{ 1471.5f, 642.5f, 21.4f },
    Position{ 1465.7f, 645.8f, 20.4f },
    Position{ 1459.0f, 645.8f, 19.8f },
    Position{ 1453.1f, 642.5f, 18.9f },
    Position{ 1449.8f, 636.7f, 18.4f },
    Position{ 1449.8f, 629.9f, 18.9f },
    Position{ 1453.1f, 624.1f, 19.7f },
    Position{ 1465.6f, 602.4f, 23.0f },
};
inline std::vector<Position> const FELMYST_EAST_FIRST_VAPOR_ROUTE = {
    Position{ 1477.9f, 577.1f, 23.3f },
    Position{ 1481.5f, 573.5f, 23.6f },
    Position{ 1487.5f, 570.2f, 23.7f },
    Position{ 1494.4f, 570.8f, 24.1f },
    Position{ 1499.8f, 575.1f, 25.1f },
    Position{ 1502.0f, 581.7f, 25.7f },
    Position{ 1500.2f, 588.3f, 25.6f },
    Position{ 1495.0f, 592.9f, 24.9f },
    Position{ 1476.4f, 609.5f, 23.9f },
};
inline std::vector<Position> const FELMYST_EAST_SECOND_VAPOR_ROUTE = {
    Position{ 1468.1f, 592.1f, 22.4f },
    Position{ 1464.5f, 588.5f, 21.8f },
    Position{ 1458.5f, 585.2f, 21.5f },
    Position{ 1451.6f, 585.8f, 21.1f },
    Position{ 1446.2f, 590.1f, 20.5f },
    Position{ 1444.0f, 596.7f, 19.8f },
    Position{ 1445.8f, 603.3f, 19.9f },
    Position{ 1451.0f, 607.9f, 20.3f },
    Position{ 1469.6f, 624.5f, 22.0f },
};

Position const& GetFelmystMainTankGroundPosition(Player* bot);
bool TryGetFelmystGroundStackPosition(
    Player* bot, Unit* felmyst, FelmystGroundStack stack, Position& position);
FelmystGroundStack GetClosestFelmystGroundStack(Player* bot, Unit* felmyst, Player* reference);
float GetFelmystFrontAngle(Player* bot, Unit* felmyst);
bool TryGetFelmystRangedPosition(Player* bot, Unit* felmyst, Position& position);
// The ranged stack rule, apart from the early move, which the stack trigger checks itself.
bool IsFelmystRangedStackActive(Player* bot, Unit* felmyst);
FelmystDemonicVaporGuids FindFelmystDemonicVaporGuids(Player* bot);
Creature* GetFelmystOwnDemonicVapor(Player* bot);
std::vector<Creature*> GetDemonicVaporHazards(Player* bot);
bool IsNearFelmystDemonicVaporHazard(Player* bot);
void RecordFelmystDemonicVaporSummon(Player* target);
void AdvanceFelmystEncounterState(Unit* felmyst);
FelmystAirEnd GetNearestFelmystAirEnd(Unit* felmyst);
Position const& GetFelmystGroupPosition(FelmystAirEnd end, uint8 demonicVaporCount);
std::vector<Position> const& GetFelmystDemonicVaporRoute(
    FelmystAirEnd end, uint8 demonicVaporCount);
bool TryGetFelmystAirGroupPosition(Unit* felmyst, Position& position);
bool IsFelmystAirGroupMoveActive(Player* bot, Unit* felmyst);
bool IsFelmystEarlyMoveDue(Player* bot, Unit* felmyst);
FelmystAirStep TryGetFelmystAirStep(
    Player* bot, Position const& destination, std::vector<Creature*> const& hazards,
    float hazardClearance, FogLane refusedLane, float& stepX, float& stepY);
bool TryGetFelmystSidestepPosition(
    Player* bot, std::vector<Creature*> const& hazards, Position& destination);
bool IsFelmystClearOfFogLane(float positionX, float positionY, FogLane lane);
Position GetFelmystFogStopPosition(Player* bot, FogLane lane);
bool TryGetFelmystLandingApproachDestination(
    Player* bot, FogLane lastCompletedLane, Unit* felmyst, Position& destination);
bool IsFelmystLanding(Unit* felmyst);
bool IsFelmystAirPhaseTargetSuppressed(Unit* felmyst);
// From the pull or the landing approach to FELMYST_GROUNDED_DPS_WAIT_MS after touchdown, for
// everyone but the main tank.
bool IsFelmystLandingDpsHoldActive(PlayerbotAI* botAI);
bool TryGetFelmystPostThirdPassWindow(Unit* felmyst, FogLane& lane);
bool IsFelmystFogMovementSuppressed(Unit* felmyst);
bool IsFelmystFogActiveForBot(Player* bot, Unit* felmyst);
bool TryGetActiveFogOfCorruptionState(Player* bot, Unit* felmyst, FogOfCorruptionState& state);
void RecordFelmystIncomingEncapsulateTarget(
    Player* target, uint32 durationMs = 3 * IN_MILLISECONDS);
Player* GetFelmystEncapsulateTarget(Player* bot);
bool ShouldMoveAwayFromFelmystEncapsulateTarget(
    Player* bot, Unit* felmyst, Player* encapsulateTarget);
bool DidEncapsulateOccurThisGroundPhase(Player* bot);
Player* GetFelmystGasNovaDispelTarget(Player* bot);
bool IsFelmystMassDispelReady(Player* bot);
Player* GetFelmystCharmedTarget(Player* bot, Unit* felmyst);

// Shared encounter state

struct FelmystEncounterState
{
    IncomingEncapsulateState incomingEncapsulate;
    bool encapsulateOccurredThisGroundPhase = false;
    FelmystFlightState lastSeenFlightState = FelmystFlightState::Unknown;
    uint32 touchdownMs = 0;
    FelmystAirEnd airPhaseEnd = FelmystAirEnd::None;
    uint8 demonicVaporCount = 0;
    ObjectGuid firstDemonicVaporTargetGuid = ObjectGuid::Empty;
    bool laneReadThisPhase = false;
    FogOfCorruptionState fogOfCorruption;
    FogPassState fogPass;
    uint32 landingDpsWaitStartMs = 0;
    uint32 landingTouchdownMs = 0;
};

FelmystEncounterState& FelmystState(uint32 instanceId);
FelmystEncounterState* FindFelmystState(uint32 instanceId);

}

#endif
