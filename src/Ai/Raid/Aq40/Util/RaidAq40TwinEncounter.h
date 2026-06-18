#ifndef _PLAYERBOT_RAIDAQ40TWINENCOUNTER_H_
#define _PLAYERBOT_RAIDAQ40TWINENCOUNTER_H_

#include <array>
#include <string>
#include <vector>

#include "ObjectGuid.h"
#include "Player.h"
#include "Position.h"

class PlayerbotAI;

namespace Aq40TwinEncounter
{
enum class TwinBoss : uint8
{
    Veklor = 0,
    Veknilash = 1,
};

enum class TwinSide : uint8
{
    Side0 = 0,
    Side1 = 1,
    Unknown = 255,
};

enum class TwinRoleCohort : uint8
{
    None = 0,
    WarlockTank,
    MeleeTank,
    SideHealer,
    RaidHealer,
    RangedDps,
    Hunter,
    MeleeDps,
};

enum class TwinStrategyMode : uint8
{
    Inactive = 0,
    CenterCommitted,
    StandardCompReady,
    Combat,
    Degraded,
};

enum class TwinEncounterPhase : uint8
{
    PrePull = 0,
    DualPullWindow,
    Stable,
    TeleportWindow,
    PickupRecovery,
    TerminalFailure,
    Degraded,
};

enum class TwinSplitBand : uint8
{
    Stable = 0,
    Warning,
    Urgent,
    Terminal,
};

enum class TwinScriptedEvent : uint8
{
    Teleport = 0,
    Blizzard,
    ArcaneBurst,
    HealBrother,
    ExplodeBug,
    MutateBug,
    Uppercut,
    UnbalancingStrike,
};

struct TwinAnchor
{
    Position position;
    float preferredRange = 0.0f;
    float facing = 0.0f;
};

struct TwinCenterSpreadSlot
{
    uint8 slotIndex = 0;
    TwinAnchor anchor;
};

struct TwinEncounterGeometry
{
    TwinAnchor roomCenter;
    std::array<TwinAnchor, 2> bossPark;
    std::array<TwinAnchor, 2> sidePrep;
    std::array<TwinAnchor, 2> stableVeklorWarlock;
    std::array<TwinAnchor, 2> reserveMeleeProxy;
    std::array<TwinAnchor, 2> reserveWarlockPrep;
    std::array<TwinAnchor, 2> sideHealer;
    std::array<TwinCenterSpreadSlot, 6> centerSpread;
};

struct TwinRoleAssignment
{
    ObjectGuid memberGuid = ObjectGuid::Empty;
    TwinRoleCohort cohort = TwinRoleCohort::None;
    TwinSide stableSide = TwinSide::Unknown;
    uint8 slotIndex = 0;
};

struct TwinStableOwnership
{
    ObjectGuid expectedOwner = ObjectGuid::Empty;
    ObjectGuid reserveOwner = ObjectGuid::Empty;
    ObjectGuid candidateOwner = ObjectGuid::Empty;
    ObjectGuid stableOwner = ObjectGuid::Empty;
    uint32 stableSinceMs = 0;
    bool reservePromotionUsed = false;
    uint32 lastValidConfirmationMs = 0;
};

struct TwinScriptedHazardWindows
{
    uint32 teleportAtMs = 0;
    uint32 blizzardAtMs = 0;
    uint32 arcaneBurstAtMs = 0;
    uint32 healBrotherAtMs = 0;
    uint32 explodeBugAtMs = 0;
    ObjectGuid explodeBugSourceGuid = ObjectGuid::Empty;
    Position explodeBugSourcePosition;
    uint32 mutateBugAtMs = 0;
    uint32 uppercutAtMs = 0;
    uint32 unbalancingStrikeAtMs = 0;
};

struct TwinBossRecoveryState
{
    uint32 threatHoldUntilMs = 0;
    bool pickupEstablished = false;
    ObjectGuid pickupOwner = ObjectGuid::Empty;
    uint32 pickupEstablishedAtMs = 0;
    uint32 pickupLostAtMs = 0;
};

struct TwinRecoveryState
{
    std::array<TwinBossRecoveryState, 2> boss;
    TwinSplitBand splitBand = TwinSplitBand::Stable;
    uint32 splitBandEnteredAtMs = 0;
};

struct TwinLockedPickupAnchor
{
    uint32 instanceId = 0;
    TwinBoss boss = TwinBoss::Veklor;
    TwinSide side = TwinSide::Unknown;
    uint32 lockedAtMs = 0;
    uint32 expiresAtMs = 0;
    TwinAnchor anchor;
};

struct TwinPetControlState
{
    uint32 instanceId = 0;
    ObjectGuid petGuid = ObjectGuid::Empty;
    bool forcedPassive = false;
    bool previousReactStateCaptured = false;
    uint8 previousReactState = 0;
    std::vector<uint32> disabledAutocastSpellIds;
};

struct TwinEncounterState
{
    uint32 instanceId = 0;
    TwinStrategyMode mode = TwinStrategyMode::Inactive;
    uint32 modeEnteredAtMs = 0;
    TwinEncounterPhase phase = TwinEncounterPhase::PrePull;
    uint32 phaseEnteredAtMs = 0;
    uint16 approachMemberCount = 0;
    uint16 stagedMemberCount = 0;
    uint16 centerCommittedMemberCount = 0;
    uint16 strictReadyMemberCount = 0;
    std::array<TwinStableOwnership, 2> ownership;
    TwinRecoveryState recovery;
    TwinScriptedHazardWindows scriptedHazards;
    uint32 lastTeleportAtMs = 0;
    uint32 nextTeleportEarliestAtMs = 0;
    uint32 nextTeleportLatestAtMs = 0;
    uint32 swapPrepStartAtMs = 0;
    uint32 closestTargetGrantDelayMs = 1000;
    uint32 swapPrepArmedAtMs = 0;
    std::vector<TwinRoleAssignment> assignments;
    std::string unsupportedReason;
};

TwinBoss GetOtherBoss(TwinBoss boss);
TwinSide GetInitialSideForBoss(TwinBoss boss);
TwinSide GetOppositeSide(TwinSide side);
bool IsKnownSide(TwinSide side);
bool IsTwinEncounterParticipant(Player const* bot, bool allowExtendedRoom = true);

TwinEncounterGeometry const& GetGeometry();
TwinRoleAssignment const* GetAssignmentForMember(TwinEncounterState const& state, ObjectGuid memberGuid);
TwinRoleAssignment const* GetAssignmentForMember(Player const* bot);
bool HasTwinAssignmentForMember(TwinEncounterState const& state, Player const* bot);
bool IsTwinAssignedParticipant(TwinEncounterState const& state, Player const* bot, bool allowExtendedRoom = true);
bool IsAssignedToCohort(TwinEncounterState const& state, ObjectGuid memberGuid, TwinRoleCohort cohort);
bool HasDeterministicAssignments(TwinEncounterState const& state);
std::string const& GetUnsupportedReason(TwinEncounterState const& state);
bool IsTwinApproachWindow(TwinEncounterState const& state, Player const* bot);
bool IsTwinApproachWindow(Player const* bot);
bool IsTwinCenterCommitted(TwinEncounterState const& state);
bool IsTwinCenterCommitted(Player const* bot);
bool IsTwinPrePullStageWindow(TwinEncounterState const& state, Player const* bot);
bool IsTwinPrePullStageWindow(Player const* bot);
bool IsTwinPrePullReady(Player const* bot);
bool IsTwinDesignatedWarlockTank(Player const* bot);
bool MarkTwinLocalCleanupState(Player* bot);
bool HasTwinLocalCleanupState(Player const* bot);
bool ClearTwinLocalCombatState(Player* bot, PlayerbotAI* botAI, bool clearCleanupState = true);
bool ShouldUseTwinWarlockTankStrategy(Player const* bot);
bool SyncTwinWarlockTankStrategy(Player* bot);
bool ClearTwinWarlockTankStrategy(Player* bot);

TwinStableOwnership& GetOwnership(TwinEncounterState& state, TwinBoss boss);
TwinStableOwnership const& GetOwnership(TwinEncounterState const& state, TwinBoss boss);
TwinBossRecoveryState& GetRecoveryState(TwinEncounterState& state, TwinBoss boss);
TwinBossRecoveryState const& GetRecoveryState(TwinEncounterState const& state, TwinBoss boss);

bool SetMode(TwinEncounterState& state, TwinStrategyMode mode, uint32 nowMs = 0);
bool CanTransitionPhase(TwinEncounterPhase from, TwinEncounterPhase to);
bool SetPhase(TwinEncounterState& state, TwinEncounterPhase phase, uint32 nowMs = 0);
bool IsActivePhase(TwinEncounterPhase phase);
bool IsRecoveryPhase(TwinEncounterPhase phase);
bool IsTerminalPhase(TwinEncounterPhase phase);
uint32 GetPhaseElapsedMs(TwinEncounterState const& state, uint32 nowMs = 0);
bool HasTeleportCadence(TwinEncounterState const& state);
void ArmTeleportCadence(TwinEncounterState& state, uint32 nowMs = 0);
bool IsSwapPrepActive(TwinEncounterState const& state, uint32 nowMs = 0);
bool IsSwapPrepActive(Player const* bot, uint32 nowMs = 0);
bool ArmSwapPrep(TwinEncounterState& state, uint32 nowMs = 0);
uint32 GetScriptedEventAtMs(TwinEncounterState const& state, TwinScriptedEvent event);
bool IsScriptedEventActive(TwinEncounterState const& state, TwinScriptedEvent event, uint32 windowMs,
                           uint32 nowMs = 0, uint32* outElapsedMs = nullptr);
bool IsScriptedEventActive(Player const* bot, TwinScriptedEvent event, uint32 windowMs, uint32 nowMs = 0,
                           uint32* outElapsedMs = nullptr);
ObjectGuid GetExplodeBugSourceGuid(TwinEncounterState const& state);
Position const& GetExplodeBugSourcePosition(TwinEncounterState const& state);
bool SetExplodeBugSource(TwinEncounterState& state, ObjectGuid sourceGuid, Position const& sourcePosition);
void ClearExplodeBugSource(TwinEncounterState& state);

bool SetExpectedOwner(TwinEncounterState& state, TwinBoss boss, ObjectGuid ownerGuid);
bool SetReserveOwner(TwinEncounterState& state, TwinBoss boss, ObjectGuid ownerGuid);
bool SetCandidateOwner(TwinEncounterState& state, TwinBoss boss, ObjectGuid ownerGuid);
void ClearCandidateOwner(TwinEncounterState& state, TwinBoss boss);
bool ConfirmOwner(TwinEncounterState& state, TwinBoss boss, ObjectGuid ownerGuid, uint32 nowMs = 0);
bool SetStableOwner(TwinEncounterState& state, TwinBoss boss, ObjectGuid ownerGuid, uint32 nowMs = 0);
void ClearStableOwner(TwinEncounterState& state, TwinBoss boss);
void ResetStableOwnership(TwinEncounterState& state, TwinBoss boss, bool keepAssignments = true);
void ResetAllStableOwnership(TwinEncounterState& state, bool keepAssignments = true);
bool HasStableOwner(TwinEncounterState const& state, TwinBoss boss);
bool HasCandidateOwner(TwinEncounterState const& state, TwinBoss boss);
bool IsStableOwner(TwinEncounterState const& state, TwinBoss boss, ObjectGuid ownerGuid);
bool IsPrimaryController(TwinEncounterState const& state, TwinBoss boss, ObjectGuid ownerGuid);
bool CanPromoteReserveOwner(TwinEncounterState const& state, TwinBoss boss);
bool PromoteReserveOwner(TwinEncounterState& state, TwinBoss boss, uint32 nowMs = 0);
uint32 GetStableOwnershipAgeMs(TwinEncounterState const& state, TwinBoss boss, uint32 nowMs = 0);
uint32 GetTimeSinceOwnershipConfirmationMs(TwinEncounterState const& state, TwinBoss boss, uint32 nowMs = 0);

void ArmThreatHoldWindow(TwinEncounterState& state, TwinBoss boss, uint32 durationMs, uint32 nowMs = 0);
void ClearThreatHoldWindow(TwinEncounterState& state, TwinBoss boss);
bool IsThreatHoldWindowActive(TwinEncounterState const& state, TwinBoss boss, uint32 nowMs = 0);
uint32 GetThreatHoldRemainingMs(TwinEncounterState const& state, TwinBoss boss, uint32 nowMs = 0);
bool IsAnyThreatHoldWindowActive(TwinEncounterState const& state, uint32 nowMs = 0);
uint32 GetMaxThreatHoldRemainingMs(TwinEncounterState const& state, uint32 nowMs = 0);

bool MarkPickupEstablished(TwinEncounterState& state, TwinBoss boss, ObjectGuid ownerGuid, uint32 nowMs = 0);
void ClearPickupEstablished(TwinEncounterState& state, TwinBoss boss, uint32 nowMs = 0);
bool IsPickupEstablished(TwinEncounterState const& state, TwinBoss boss);
ObjectGuid GetPickupOwner(TwinEncounterState const& state, TwinBoss boss);
uint32 GetPickupEstablishedAgeMs(TwinEncounterState const& state, TwinBoss boss, uint32 nowMs = 0);

bool SetSplitBand(TwinEncounterState& state, TwinSplitBand band, uint32 nowMs = 0);
uint32 GetSplitBandAgeMs(TwinEncounterState const& state, uint32 nowMs = 0);

void EnterDualPullWindow(TwinEncounterState& state, uint32 nowMs = 0);
void EnterStablePhase(TwinEncounterState& state, uint32 nowMs = 0);
void EnterTeleportWindow(TwinEncounterState& state, uint32 threatHoldDurationMs, uint32 nowMs = 0);
void EnterPickupRecovery(TwinEncounterState& state, uint32 nowMs = 0);
void EnterTerminalFailure(TwinEncounterState& state, uint32 nowMs = 0);
void EnterDegradedPhase(TwinEncounterState& state, uint32 nowMs = 0);

TwinEncounterState* GetEncounterState(Player* bot);
TwinEncounterState const* GetEncounterState(Player const* bot);
TwinEncounterState& EnsureEncounterState(Player* bot);

TwinLockedPickupAnchor* GetLockedPickupAnchor(Player* bot);
TwinLockedPickupAnchor const* GetLockedPickupAnchor(Player const* bot);
TwinLockedPickupAnchor& EnsureLockedPickupAnchor(Player* bot);
TwinPetControlState* GetPetControlState(Player* bot);
TwinPetControlState const* GetPetControlState(Player const* bot);
TwinPetControlState& EnsurePetControlState(Player* bot);
bool IsLockedPickupAnchorExpired(TwinLockedPickupAnchor const& state, uint32 nowMs = 0);
bool HasLockedPickupAnchor(Player const* bot, TwinBoss boss, uint32 nowMs = 0);
bool SetLockedPickupAnchor(Player* bot, TwinBoss boss, TwinSide side, TwinAnchor const& anchor, uint32 durationMs,
                           uint32 nowMs = 0);
bool PruneExpiredLockedPickupAnchor(Player* bot, uint32 nowMs = 0);
void ClearLockedPickupAnchor(Player* bot);
bool HasActiveLockedPickupAnchor(Player const* bot, uint32 nowMs = 0);
bool IsImmediateRepositionWindow(TwinEncounterState const& state, uint32 nowMs = 0);
bool IsImmediateRepositionWindow(Player const* bot, uint32 nowMs = 0);
bool RequestImmediateMovementInterrupt(Player* bot);

void ResetEncounterState(TwinEncounterState& state, uint32 instanceId);
void ResetPickupAnchorState(TwinLockedPickupAnchor& state);
void ResetPetControlState(TwinPetControlState& state);
bool RestorePetControlState(Player* bot);
bool ResetState(Player* bot);
bool HasPersistentState(Player* bot);
uint32 GetInstanceId(Player const* bot);

char const* ToString(TwinBoss boss);
char const* ToString(TwinSide side);
char const* ToString(TwinRoleCohort cohort);
char const* ToString(TwinStrategyMode mode);
char const* ToString(TwinEncounterPhase phase);
char const* ToString(TwinScriptedEvent event);
char const* ToString(TwinSplitBand band);
}    // namespace Aq40TwinEncounter

#endif
