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

class Creature;
class Player;
class PlayerbotAI;
class Unit;

namespace SwpHelpers
{

enum class FelmystFogLane : uint8
{
    None = std::numeric_limits<uint8>::max(),
    Top = 0,
    Middle = 1,
    Bottom = 2,
};

enum class FelmystFogPhase : uint8
{
    None,
    Windup,
    Sweep,
    Recovery,
};

enum class FelmystFogLocation : uint8
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

struct FelmystFogOfCorruptionState
{
    FelmystFogLane lane = FelmystFogLane::None;
    FelmystFogPhase phase = FelmystFogPhase::None;
    uint32 expireMs = 0;
};

struct FelmystFogPassState
{
    FelmystFogLocation lastDestinationLocation = FelmystFogLocation::None;
    FelmystFogLane lastCompletedLane = FelmystFogLane::None;
    FelmystFogLane armedSweepLane = FelmystFogLane::None;
    uint8 completedPassCount = 0;
    uint32 thirdPassWindowExpireMs = 0;
};

struct FelmystIncomingEncapsulateState
{
    ObjectGuid targetGuid = ObjectGuid::Empty;
    uint32 delayMs = 0;
    uint32 expireMs = 0;
    bool auraObserved = false;
};

struct FelmystFogCrateStuckState
{
    Position destination;
    float nearestDestinationDistance = std::numeric_limits<float>::max();
    uint32 sampleMs = 0;
};

struct FelmystEncounterState
{
    std::unordered_map<ObjectGuid, uint8> rangedAssignments;
    FelmystIncomingEncapsulateState incomingEncapsulate;
    bool encapsulateOccurredThisGroundPhase = false;
    std::unordered_map<ObjectGuid, uint8> demonicVaporRegionIndices;
    uint8 demonicVaporUsedRegionMask = 0;
    uint8 demonicVaporFirstRegionIndex = 0;
    FelmystFogOfCorruptionState fogOfCorruption;
    FelmystFogPassState fogPass;
    time_t landingDpsWaitTimer = 0;
    time_t landingTouchdownTimer = 0;
};

constexpr float FELMYST_ENCAPSULATE_SAFE_DISTANCE = 20.0f;
constexpr float FELMYST_FOG_SAFE_SPOT_ARRIVAL_DISTANCE = 8.0f;
constexpr float FELMYST_FOG_CURRENT_POINT_MATCH_DISTANCE = 3.0f;
constexpr float FELMYST_FOG_DESTINATION_MATCH_DISTANCE = 1.0f;
constexpr float FELMYST_MELEE_DISTANCE = 12.5f;
constexpr float FELMYST_RANGED_GROUP_RADIUS = 0.5f;
constexpr float FELMYST_RANGED_SIDE_DISTANCE = 24.0f;
constexpr uint32 FELMYST_INCOMING_ENCAPSULATE_DELAY_MS = 500;

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
void ClearFelmystDemonicVaporKiteState(Player* bot);
bool TryGetFelmystDemonicVaporKiteDestination(Player* bot, Position& destination);
bool TryGetFelmystFogSafeDestinations(
    Player* bot, FelmystFogLane dangerLane, std::array<Position, 3>& destinations,
    uint8& destinationCount);
bool TryGetFelmystLandingDestination(Unit* felmyst, Position& destination);
bool IsFelmystAirPhaseTargetSuppressed(Unit* felmyst);
bool TryGetFelmystPostThirdPassWindow(Unit* felmyst, FelmystFogLane& lane);
bool TryGetFelmystFogOfCorruptionStageState(Unit* felmyst, FelmystFogOfCorruptionState& state);
bool TryGetActiveFelmystFogOfCorruptionState(
    Player* bot, Unit* felmyst, FelmystFogOfCorruptionState& state);
void RecordFelmystIncomingEncapsulateTarget(Player* target, uint32 durationMs = 3000);
Player* GetFelmystEncapsulateTarget(Player* bot);
bool DidFelmystEncapsulateOccurThisGroundPhase(Player* bot);
Player* GetFelmystGasNovaDispelTarget(Player* bot);
Player* GetFelmystCharmedTarget(Player* bot, Unit* felmyst);

}

#endif
