/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_SWPENCOUNTERFELMYST_H
#define PLAYERBOTS_SWPENCOUNTERFELMYST_H

#include "ObjectGuid.h"
#include "Position.h"
#include "SWPData.h"
#include <array>
#include <ctime>
#include <limits>
#include <unordered_map>
#include <vector>

class Creature;
class Player;
class PlayerbotAI;
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
    std::unordered_map<ObjectGuid, uint8> rangedAssignments;
    IncomingEncapsulateState incomingEncapsulate;
    bool encapsulateOccurredThisGroundPhase = false;
    std::unordered_map<ObjectGuid, uint8> demonicVaporRegionIndices;
    uint8 demonicVaporUsedRegionMask = 0;
    uint8 demonicVaporFirstRegionIndex = 0;
    FogOfCorruptionState fogOfCorruption;
    FogPassState fogPass;
    time_t landingDpsWaitTimer = 0;
    time_t landingTouchdownTimer = 0;
    ObjectGuid flightLeaderGuid = ObjectGuid::Empty;
};

struct FogSafeThreshold
{
    Position a, b;
    bool safeSideIsNorth;  // true = safe side has higher X (north), false = lower X (south)
};

constexpr float FELMYST_RANGED_GROUP_RADIUS = 0.5f;
constexpr float FELMYST_LOCATION_MATCH_DISTANCE = 2.0f;

extern std::unordered_map<uint32, FelmystEncounterState> felmystEncounterStates;

Position const& GetFelmystMainTankGroundPosition(Player* bot);
bool TryGetFelmystGroundStackPosition(
    Player* bot, Unit* felmyst, FelmystGroundStack stack, Position& position);
FelmystGroundStack GetClosestFelmystGroundStack(Player* bot, Unit* felmyst, Unit* unit);
float GetFelmystFrontAngle(Player* bot, Unit* felmyst);
void EnsureFelmystRangedAssignments(Player* bot);
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
