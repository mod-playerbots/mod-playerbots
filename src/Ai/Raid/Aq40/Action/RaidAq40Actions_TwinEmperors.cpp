#include "RaidAq40Actions.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <list>
#include <sstream>
#include <string>

#include "CharmInfo.h"
#include "Map.h"
#include "Pet.h"
#include "SpellInfo.h"
#include "SpellMgr.h"

#include "../RaidAq40SpellIds.h"
#include "../Util/RaidAq40Helpers_Shared.h"
#include "../Util/RaidAq40TwinEncounter.h"

namespace
{
float constexpr kTwinExplodeBugDangerRadius = 17.0f;
float constexpr kTwinArcaneBurstDangerRadius = 18.0f;
float constexpr kTwinArcaneBurstLooseRadius = 24.0f;
float constexpr kTwinWarlockMinRange = 19.0f;
float constexpr kTwinWarlockMaxRange = 30.0f;
float constexpr kTwinWarlockPreferredRange = 24.0f;
float constexpr kTwinMeleeContactRange = 5.0f;
float constexpr kTwinAnchorTolerance = 4.0f;
float constexpr kTwinStrictReadyAnchorTolerance = 6.0f;
float constexpr kTwinFacingTolerance = 0.15f;
float constexpr kPi = 3.14159265358979323846f;
uint32 constexpr kTwinWarlockThreatLeadMs = 4500;
uint32 constexpr kTwinStableControllerConfirmationWindowMs = 6000;
uint32 constexpr kTwinBlizzardWindowMs = 5000;
uint32 constexpr kTwinExplodeBugWindowMs = 2500;
uint32 constexpr kTwinMutateBugWindowMs = 2500;
uint32 constexpr kTwinArcaneBurstWindowMs = 2500;
float constexpr kTwinSplitWarningDistance = 75.0f;
float constexpr kTwinSplitUrgentDistance = 65.0f;
float constexpr kTwinBossParkWarningError = 8.0f;
float constexpr kTwinBossParkUrgentError = 16.0f;
float constexpr kTwinHealerAnchorNearDistance = 10.0f;
float constexpr kTwinHealerStepRecoveryDistance = 18.0f;
float constexpr kTwinHealerCenterReentryDistance = 12.0f;
float constexpr kTwinRangedBugServiceRange = 32.0f;
float constexpr kTwinRangedGenericBugServiceRange = 22.0f;
float constexpr kTwinHunterMarkedBugServiceRange = 26.0f;
float constexpr kTwinRangedBugArcaneSafeRadius = kTwinArcaneBurstDangerRadius + 2.0f;
float constexpr kTwinHunterBugArcaneSafeRadius = kTwinArcaneBurstLooseRadius + 4.0f;
float constexpr kTwinMeleeBugArcaneSafeRadius = kTwinArcaneBurstLooseRadius;
float constexpr kTwinExplodeBugServiceSafeRadius = kTwinExplodeBugDangerRadius + 2.0f;

struct TwinAnchorOffsetPattern
{
    float forward = 0.0f;
    float lateral = 0.0f;
};

struct TwinPrePullAnchorChoice
{
    Aq40TwinEncounter::TwinAnchor anchor;
    char const* label = "room_center";
};

TwinPrePullAnchorChoice GetTwinPrePullAnchorChoice(Aq40TwinEncounter::TwinEncounterState const& state,
                                                   Aq40TwinEncounter::TwinRoleAssignment const& assignment);
TwinPrePullAnchorChoice GetTwinStableAnchorChoice(Aq40TwinEncounter::TwinEncounterState const& state,
                                                  Aq40TwinEncounter::TwinRoleAssignment const& assignment);
Aq40TwinEncounter::TwinSide GetTwinExpectedOwnerSide(Aq40TwinEncounter::TwinEncounterState const& state,
                                                     Aq40TwinEncounter::TwinBoss boss);
bool HasTwinCredibleStableController(Aq40TwinEncounter::TwinEncounterState const& state,
                                     Aq40TwinEncounter::TwinBoss boss);
bool DoesTwinAssignmentAllowBossTarget(Aq40TwinEncounter::TwinEncounterState const& state,
                                       Aq40TwinEncounter::TwinRoleAssignment const* assignment,
                                       Unit const* target);

struct Direction2d
{
    float x = 0.0f;
    float y = 0.0f;
    float length = 0.0f;
};

enum class TwinTargetIntent : uint8
{
    None = 0,
    Veklor,
    Veknilash,
    HoldReserve,
    HoldVeknilash,
};

enum class TwinBugPriority : uint8
{
    Explode = 0,
    Mutate = 1,
    Hostile = 2,
    None = 255,
};

struct TwinBugSelection
{
    Unit* target = nullptr;
    TwinBugPriority priority = TwinBugPriority::None;
    float serviceDistance = std::numeric_limits<float>::max();
};

class TwinMovementActionShim : public MovementAction
{
public:
    TwinMovementActionShim(PlayerbotAI* botAI) : MovementAction(botAI, "aq40 twin movement shim") {}

    using MovementAction::MoveInside;
};

class TwinAttackActionShim : public AttackAction
{
public:
    TwinAttackActionShim(PlayerbotAI* botAI) : AttackAction(botAI, "aq40 twin attack shim") {}

    using AttackAction::Attack;
};

float ComputeFacing(Position const& from, Position const& to)
{
    return std::atan2(to.GetPositionY() - from.GetPositionY(), to.GetPositionX() - from.GetPositionX());
}

float GetFacingDelta(float targetFacing, float currentFacing)
{
    float const delta = std::fabs(Position::NormalizeOrientation(targetFacing - currentFacing));
    return delta > kPi ? (2.0f * kPi - delta) : delta;
}

Direction2d GetDirection2d(Position const& from, Position const& to)
{
    Direction2d direction;
    float const dx = to.GetPositionX() - from.GetPositionX();
    float const dy = to.GetPositionY() - from.GetPositionY();
    direction.length = std::sqrt(dx * dx + dy * dy);

    if (direction.length >= 0.01f)
    {
        direction.x = dx / direction.length;
        direction.y = dy / direction.length;
    }

    return direction;
}

Aq40TwinEncounter::TwinAnchor BuildOffsetAnchor(Aq40TwinEncounter::TwinAnchor const& origin, Position const& toward,
                                                float forwardDistance, float lateralDistance,
                                                Position const& facingTarget, float preferredRange = 0.0f)
{
    Direction2d const direction = GetDirection2d(origin.position, toward);
    Position position;

    if (direction.length < 0.01f)
    {
        position.Relocate(origin.position.GetPositionX(), origin.position.GetPositionY(), origin.position.GetPositionZ());
    }
    else
    {
        float const rightX = direction.y;
        float const rightY = -direction.x;
        float const zRatio = std::min(std::fabs(forwardDistance) / direction.length, 1.0f);
        position.Relocate(origin.position.GetPositionX() + direction.x * forwardDistance + rightX * lateralDistance,
            origin.position.GetPositionY() + direction.y * forwardDistance + rightY * lateralDistance,
            origin.position.GetPositionZ() +
                (toward.GetPositionZ() - origin.position.GetPositionZ()) * zRatio);
    }

    Aq40TwinEncounter::TwinAnchor anchor;
    anchor.position = position;
    anchor.preferredRange = preferredRange > 0.0f ? preferredRange : origin.preferredRange;
    anchor.facing = ComputeFacing(position, facingTarget);
    return anchor;
}

GuidVector GetTwinEncounterUnits(PlayerbotAI* botAI)
{
    if (!botAI || !botAI->GetAiObjectContext())
        return GuidVector();

    return Aq40BossHelper::GetEncounterUnits(
        botAI, botAI->GetAiObjectContext()->GetValue<GuidVector>("attackers")->Get());
}

Unit* FindTwinUnitByEntry(PlayerbotAI* botAI, GuidVector const& units, uint32 entry)
{
    if (!botAI)
        return nullptr;

    Player* bot = botAI->GetBot();
    if (!bot)
        return nullptr;

    for (ObjectGuid const guid : units)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive() || !unit->IsInWorld() || unit->IsFriendlyTo(bot) ||
            unit->GetMapId() != bot->GetMapId())
        {
            continue;
        }

        if (unit->GetEntry() == entry)
            return unit;
    }

    return nullptr;
}

Unit* FindTwinBoss(PlayerbotAI* botAI, GuidVector const& units, Aq40TwinEncounter::TwinBoss boss)
{
    return FindTwinUnitByEntry(botAI, units,
        boss == Aq40TwinEncounter::TwinBoss::Veklor ? Aq40SpellIds::TwinVeklorNpcEntry
                                                    : Aq40SpellIds::TwinVeknilashNpcEntry);
}

Unit* FindNearestTwinBug(Player* bot, PlayerbotAI* botAI, GuidVector const& units, float maxDistance)
{
    if (!bot || !botAI)
        return nullptr;

    Unit* nearestBug = nullptr;
    float nearestDistance = std::numeric_limits<float>::max();
    for (ObjectGuid const guid : units)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive() || !Aq40SpellIds::IsTwinBugEntry(unit->GetEntry()))
            continue;

        float const distance = bot->GetDistance2d(unit);
        if (distance > maxDistance || distance >= nearestDistance)
            continue;

        nearestBug = unit;
        nearestDistance = distance;
    }

    return nearestBug;
}

bool IsTwinExplodeBugCast(Spell const* spell)
{
    return spell && Aq40SpellIds::MatchesAnySpellId(spell->GetSpellInfo(), { Aq40SpellIds::TwinExplodeBug });
}

bool TryGetTwinExplodeBugHazard(Player* bot, PlayerbotAI* botAI,
                               Aq40TwinEncounter::TwinEncounterState const& state,
                               GuidVector const& units, float maxDistance, bool allowTrackedSource,
                               Unit*& outBug, Position& outPosition)
{
    outBug = nullptr;
    if (!bot || !botAI)
        return false;

    if (allowTrackedSource)
    {
        ObjectGuid const trackedSourceGuid = Aq40TwinEncounter::GetExplodeBugSourceGuid(state);
        if (!trackedSourceGuid.IsEmpty())
        {
            outPosition = Aq40TwinEncounter::GetExplodeBugSourcePosition(state);
            if (Unit* trackedBug = botAI->GetUnit(trackedSourceGuid);
                trackedBug && trackedBug->IsAlive() && trackedBug->IsInWorld() &&
                Aq40SpellIds::IsTwinBugEntry(trackedBug->GetEntry()))
            {
                outBug = trackedBug;
                outPosition = trackedBug->GetPosition();
                if (bot->GetDistance2d(trackedBug) <= maxDistance)
                    return true;
            }
            else if (bot->GetExactDist2d(outPosition.GetPositionX(), outPosition.GetPositionY()) <= maxDistance)
            {
                return true;
            }
        }
    }

    for (ObjectGuid const guid : units)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive() || !Aq40SpellIds::IsTwinBugEntry(unit->GetEntry()))
            continue;

        if (!IsTwinExplodeBugCast(unit->GetCurrentSpell(CURRENT_GENERIC_SPELL)))
            continue;

        float const distance = bot->GetDistance2d(unit);
        if (distance > maxDistance)
            continue;

        outBug = unit;
        outPosition = unit->GetPosition();
        return true;
    }

    return false;
}

bool IsTwinEncounterActive(Aq40TwinEncounter::TwinEncounterState const* state)
{
    return state && Aq40TwinEncounter::IsActivePhase(state->phase) &&
           !Aq40TwinEncounter::IsTerminalPhase(state->phase);
}

bool IsTwinWarlockProfile(Player* bot)
{
    return bot && bot->getClass() == CLASS_WARLOCK;
}

bool IsTwinHealerProfile(Player* bot, PlayerbotAI* botAI)
{
    return bot && botAI && botAI->IsHeal(bot);
}

bool IsTwinMeleeProfile(Player* bot, PlayerbotAI* botAI)
{
    if (!bot || !botAI)
        return false;

    if (Aq40BossHelper::IsEncounterTank(bot, bot))
        return true;

    if (bot->getClass() == CLASS_HUNTER)
        return true;

    return !PlayerbotAI::IsRanged(bot) && !botAI->IsHeal(bot);
}

bool IsTwinVeklorTarget(Unit const* unit)
{
    return unit && unit->GetEntry() == Aq40SpellIds::TwinVeklorNpcEntry;
}

bool IsTwinVeknilashTarget(Unit const* unit)
{
    return unit && unit->GetEntry() == Aq40SpellIds::TwinVeknilashNpcEntry;
}

bool IsTwinEmperorTarget(Unit const* unit)
{
    return IsTwinVeklorTarget(unit) || IsTwinVeknilashTarget(unit);
}

size_t ToSideIndex(Aq40TwinEncounter::TwinSide side)
{
    return side == Aq40TwinEncounter::TwinSide::Side1 ? 1u : 0u;
}

Aq40TwinEncounter::TwinSide GetTwinSideForPosition(float x, float y)
{
    Aq40TwinEncounter::TwinEncounterGeometry const& geometry = Aq40TwinEncounter::GetGeometry();
    float const side0Distance = geometry.bossPark[0].position.GetExactDist2d(x, y);
    float const side1Distance = geometry.bossPark[1].position.GetExactDist2d(x, y);
    return side0Distance <= side1Distance ? Aq40TwinEncounter::TwinSide::Side0
                                          : Aq40TwinEncounter::TwinSide::Side1;
}

GuidVector GetTwinPrePullUnits(PlayerbotAI* botAI)
{
    GuidVector units;
    if (!botAI || !botAI->GetAiObjectContext())
        return units;

    auto* context = botAI->GetAiObjectContext();
    GuidVector const attackers = context->GetValue<GuidVector>("attackers")->Get();
    units.insert(units.end(), attackers.begin(), attackers.end());

    GuidVector const& possibleTargetsNoLos = context->GetValue<GuidVector>("possible targets no los")->Get();
    for (ObjectGuid const guid : possibleTargetsNoLos)
    {
        if (std::find(units.begin(), units.end(), guid) == units.end())
            units.push_back(guid);
    }

    return units;
}

Unit* FindTwinBossForPrePull(PlayerbotAI* botAI, Aq40TwinEncounter::TwinBoss boss)
{
    if (!botAI)
        return nullptr;

    if (auto* aiContext = botAI->GetAiObjectContext())
    {
        Unit* directTarget = aiContext->GetValue<Unit*>(
            "find target",
            boss == Aq40TwinEncounter::TwinBoss::Veklor ? "emperor vek'lor" : "emperor vek'nilash")->Get();
        if (directTarget)
            return directTarget;
    }

    return FindTwinBoss(botAI, GetTwinPrePullUnits(botAI), boss);
}

Unit* GetTwinCurrentTarget(PlayerbotAI* botAI)
{
    if (!botAI || !botAI->GetAiObjectContext())
        return nullptr;

    return botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Get();
}

bool MoveTwinInside(PlayerbotAI* botAI, uint32 mapId, float x, float y, float z, float distance,
                    MovementPriority priority)
{
    if (!botAI)
        return false;

    TwinMovementActionShim action(botAI);
    return action.MoveInside(mapId, x, y, z, distance, priority);
}

bool AttackTwinTarget(PlayerbotAI* botAI, Unit* target)
{
    if (!botAI)
        return false;

    TwinAttackActionShim action(botAI);
    return action.Attack(target);
}

bool IsTwinOpeningWarlockTankAssignment(Aq40TwinEncounter::TwinRoleAssignment const& assignment)
{
    return assignment.cohort == Aq40TwinEncounter::TwinRoleCohort::WarlockTank &&
           assignment.stableSide == Aq40TwinEncounter::GetInitialSideForBoss(Aq40TwinEncounter::TwinBoss::Veklor);
}

bool IsTwinOpeningMeleeTankAssignment(Aq40TwinEncounter::TwinRoleAssignment const& assignment)
{
    return assignment.cohort == Aq40TwinEncounter::TwinRoleCohort::MeleeTank &&
           assignment.stableSide == Aq40TwinEncounter::GetInitialSideForBoss(Aq40TwinEncounter::TwinBoss::Veknilash);
}

bool IsTwinReserveTankHoldAssignment(Aq40TwinEncounter::TwinRoleAssignment const& assignment)
{
    if (assignment.cohort == Aq40TwinEncounter::TwinRoleCohort::WarlockTank)
        return !IsTwinOpeningWarlockTankAssignment(assignment);

    if (assignment.cohort == Aq40TwinEncounter::TwinRoleCohort::MeleeTank)
        return !IsTwinOpeningMeleeTankAssignment(assignment);

    return false;
}

bool ShouldStartTwinOpeningPullFromPrePull(Aq40TwinEncounter::TwinRoleAssignment const& assignment)
{
    return IsTwinOpeningWarlockTankAssignment(assignment) || IsTwinOpeningMeleeTankAssignment(assignment);
}

bool IsTwinTankAssignment(Aq40TwinEncounter::TwinRoleAssignment const& assignment)
{
    return assignment.cohort == Aq40TwinEncounter::TwinRoleCohort::WarlockTank ||
           assignment.cohort == Aq40TwinEncounter::TwinRoleCohort::MeleeTank;
}

struct TwinOpeningTargets
{
    Unit* veklor = nullptr;
    Unit* veknilash = nullptr;
};

TwinOpeningTargets FindTwinOpeningTargetsForPrePull(PlayerbotAI* botAI)
{
    TwinOpeningTargets targets;
    targets.veklor = FindTwinBossForPrePull(botAI, Aq40TwinEncounter::TwinBoss::Veklor);
    targets.veknilash = FindTwinBossForPrePull(botAI, Aq40TwinEncounter::TwinBoss::Veknilash);
    return targets;
}

bool AreTwinOpeningTargetsDiscoverable(TwinOpeningTargets const& targets)
{
    return targets.veklor && targets.veknilash;
}

Unit* GetTwinOpeningTargetForAssignment(Aq40TwinEncounter::TwinRoleAssignment const& assignment,
                                        TwinOpeningTargets const& targets)
{
    if (IsTwinOpeningWarlockTankAssignment(assignment))
        return targets.veklor;

    if (IsTwinOpeningMeleeTankAssignment(assignment))
        return targets.veknilash;

    return nullptr;
}

bool HasTwinSeededDualPullOwnership(Aq40TwinEncounter::TwinEncounterState const& state)
{
    return state.phase != Aq40TwinEncounter::TwinEncounterPhase::PrePull;
}

Unit* GetTwinPendingAttackIntentTarget(Player* bot, PlayerbotAI* botAI)
{
    if (!bot || !botAI || !botAI->GetAiObjectContext())
        return nullptr;

    auto* context = botAI->GetAiObjectContext();
    if (Unit* currentTarget = context->GetValue<Unit*>("current target")->Get())
        return currentTarget;

    ObjectGuid const pullTargetGuid = context->GetValue<ObjectGuid>("pull target")->Get();
    if (!pullTargetGuid.IsEmpty())
    {
        if (Unit* pullTarget = botAI->GetUnit(pullTargetGuid))
            return pullTarget;
    }

    ObjectGuid const selectionGuid = bot->GetTarget();
    if (!selectionGuid.IsEmpty())
        return botAI->GetUnit(selectionGuid);

    return nullptr;
}

Player* FindTwinInstanceMember(Player* bot, ObjectGuid guid)
{
    if (!bot || guid.IsEmpty() || !bot->GetMap())
        return nullptr;

    Map::PlayerList const& players = bot->GetMap()->GetPlayers();
    for (Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
    {
        Player* member = itr->GetSource();
        if (member && member->GetGUID() == guid)
            return member;
    }

    return nullptr;
}

uint32 GetTwinModeElapsedMs(Aq40TwinEncounter::TwinEncounterState const& state, uint32 nowMs = 0)
{
    if (!state.modeEnteredAtMs)
        return 0;

    uint32 const now = nowMs ? nowMs : getMSTime();
    return getMSTimeDiff(state.modeEnteredAtMs, now);
}

uint32 GetTwinTeleportElapsedMs(Aq40TwinEncounter::TwinEncounterState const& state, uint32 nowMs = 0)
{
    if (!state.lastTeleportAtMs)
        return 0;

    uint32 const now = nowMs ? nowMs : getMSTime();
    return getMSTimeDiff(state.lastTeleportAtMs, now);
}

ObjectGuid GetTwinTelemetryControllerGuid(Aq40TwinEncounter::TwinEncounterState const& state,
                                          Aq40TwinEncounter::TwinBoss boss)
{
    Aq40TwinEncounter::TwinStableOwnership const& ownership = Aq40TwinEncounter::GetOwnership(state, boss);
    if (!ownership.stableOwner.IsEmpty())
        return ownership.stableOwner;
    if (!ownership.candidateOwner.IsEmpty())
        return ownership.candidateOwner;

    return ownership.expectedOwner;
}

void AppendTwinBossOwnershipTelemetryFields(std::ostringstream& fields, Player* bot,
                                            Aq40TwinEncounter::TwinEncounterState const& state,
                                            Aq40TwinEncounter::TwinBoss boss, char const* prefix,
                                            uint32 nowMs = 0)
{
    Aq40TwinEncounter::TwinStableOwnership const& ownership = Aq40TwinEncounter::GetOwnership(state, boss);
    uint32 const now = nowMs ? nowMs : getMSTime();

    fields << " " << prefix << "_controller=" << Aq40Helpers::GetAq40LogUnit(
                  FindTwinInstanceMember(bot, GetTwinTelemetryControllerGuid(state, boss)))
           << " " << prefix << "_candidate_owner=" << Aq40Helpers::GetAq40LogUnit(
                  FindTwinInstanceMember(bot, ownership.candidateOwner))
           << " " << prefix << "_stable_owner=" << Aq40Helpers::GetAq40LogUnit(
                  FindTwinInstanceMember(bot, ownership.stableOwner))
           << " " << prefix << "_pickup_owner=" << Aq40Helpers::GetAq40LogUnit(
                  FindTwinInstanceMember(bot, Aq40TwinEncounter::GetPickupOwner(state, boss)))
           << " " << prefix << "_pickup=" << (Aq40TwinEncounter::IsPickupEstablished(state, boss) ? 1 : 0)
           << " " << prefix << "_confirm_age_ms="
           << Aq40TwinEncounter::GetTimeSinceOwnershipConfirmationMs(state, boss, now)
           << " " << prefix << "_pickup_age_ms="
           << Aq40TwinEncounter::GetPickupEstablishedAgeMs(state, boss, now)
           << " " << prefix << "_threat_hold_remaining_ms="
           << Aq40TwinEncounter::GetThreatHoldRemainingMs(state, boss, now);
}

void AppendTwinOpeningOwnershipFields(std::ostringstream& fields, Player* bot,
                                      Aq40TwinEncounter::TwinEncounterState const& state,
                                      uint32 nowMs = 0)
{
    auto appendBossFields = [&](Aq40TwinEncounter::TwinBoss boss, char const* prefix)
    {
        Aq40TwinEncounter::TwinStableOwnership const& ownership = Aq40TwinEncounter::GetOwnership(state, boss);
        fields << " " << prefix << "_opener=" << Aq40Helpers::GetAq40LogUnit(
                      FindTwinInstanceMember(bot, ownership.expectedOwner))
               << " " << prefix << "_reserve=" << Aq40Helpers::GetAq40LogUnit(
                      FindTwinInstanceMember(bot, ownership.reserveOwner));
        AppendTwinBossOwnershipTelemetryFields(fields, bot, state, boss, prefix, nowMs);
    };

    appendBossFields(Aq40TwinEncounter::TwinBoss::Veklor, "veklor");
    appendBossFields(Aq40TwinEncounter::TwinBoss::Veknilash, "veknilash");
}

void AppendTwinAnchorLogFields(std::ostringstream& fields, Player const* bot,
                               Aq40TwinEncounter::TwinAnchor const& anchor, char const* anchorLabel)
{
    fields << " anchor=" << anchorLabel
           << " anchor_x=" << anchor.position.GetPositionX()
           << " anchor_y=" << anchor.position.GetPositionY()
           << " anchor_z=" << anchor.position.GetPositionZ();

    if (bot)
    {
        fields << " anchor_error="
               << bot->GetExactDist2d(anchor.position.GetPositionX(), anchor.position.GetPositionY());
    }
}

void LogTwinTankAttackIntentDecision(Player* bot, Aq40TwinEncounter::TwinEncounterState const& state,
                                     Aq40TwinEncounter::TwinRoleAssignment const& assignment,
                                     TwinPrePullAnchorChoice const& anchorChoice, Unit* requestedTarget,
                                     Unit* openerTarget, char const* stage, char const* decision)
{
    if (!bot || !requestedTarget)
        return;

    std::ostringstream fields;
    fields << "boss=twin phase=" << Aq40TwinEncounter::ToString(state.phase)
           << " mode=" << Aq40TwinEncounter::ToString(state.mode)
            << " mode_elapsed_ms=" << GetTwinModeElapsedMs(state)
           << " stage=" << stage
           << " decision=" << decision
           << " cohort=" << Aq40TwinEncounter::ToString(assignment.cohort)
           << " side=" << Aq40TwinEncounter::ToString(assignment.stableSide)
           << " slot=" << static_cast<uint32>(assignment.slotIndex);
    AppendTwinAnchorLogFields(fields, bot, anchorChoice.anchor, anchorChoice.label);
    fields
           << " center_committed=" << state.centerCommittedMemberCount
           << " strict_ready=" << state.strictReadyMemberCount
           << " requested_target=" << Aq40Helpers::GetAq40LogUnit(requestedTarget)
           << " opener_target=" << Aq40Helpers::GetAq40LogUnit(openerTarget);
    AppendTwinOpeningOwnershipFields(fields, bot, state);
    Aq40Helpers::LogAq40Info(bot, "twin_prepull",
        "twin:" + std::string(stage) + ":tank_order:" + decision, fields.str(), 1000);
}

void LogTwinCenterCommitHandoff(Player* bot, Aq40TwinEncounter::TwinEncounterState const& state,
                                Aq40TwinEncounter::TwinRoleAssignment const& assignment,
                                TwinPrePullAnchorChoice const& anchorChoice)
{
    if (!bot || state.mode != Aq40TwinEncounter::TwinStrategyMode::CenterCommitted)
        return;

    std::ostringstream fields;
    fields << "boss=twin phase=" << Aq40TwinEncounter::ToString(state.phase)
           << " mode=" << Aq40TwinEncounter::ToString(state.mode)
           << " mode_elapsed_ms=" << GetTwinModeElapsedMs(state)
           << " cohort=" << Aq40TwinEncounter::ToString(assignment.cohort)
           << " side=" << Aq40TwinEncounter::ToString(assignment.stableSide)
           << " slot=" << static_cast<uint32>(assignment.slotIndex);
    AppendTwinAnchorLogFields(fields, bot, anchorChoice.anchor, anchorChoice.label);
    fields << " approach=" << state.approachMemberCount
           << " staged=" << state.stagedMemberCount
           << " center_committed=" << state.centerCommittedMemberCount
           << " strict_ready=" << state.strictReadyMemberCount
           << " assigned=" << state.assignments.size()
           << " unsupported_reason=" << (state.unsupportedReason.empty() ? "none" : state.unsupportedReason);
    AppendTwinOpeningOwnershipFields(fields, bot, state);
    Aq40Helpers::LogAq40Info(bot, "twin_prepull", "twin:center_commit:handoff", fields.str(), 1000);
}

void LogTwinInitialEngagementArm(Player* bot, Aq40TwinEncounter::TwinEncounterState const& state,
                                 TwinOpeningTargets const& openingTargets)
{
    if (!bot)
        return;

    std::ostringstream fields;
    fields << "boss=twin phase=" << Aq40TwinEncounter::ToString(state.phase)
           << " mode=" << Aq40TwinEncounter::ToString(state.mode)
           << " phase_elapsed_ms=" << Aq40TwinEncounter::GetPhaseElapsedMs(state)
            << " mode_elapsed_ms=" << GetTwinModeElapsedMs(state)
            << " teleport_elapsed_ms=" << GetTwinTeleportElapsedMs(state)
           << " approach=" << state.approachMemberCount
           << " staged=" << state.stagedMemberCount
           << " center_committed=" << state.centerCommittedMemberCount
           << " strict_ready=" << state.strictReadyMemberCount
           << " assigned=" << state.assignments.size();
    AppendTwinOpeningOwnershipFields(fields, bot, state);
    fields << " veklor_target=" << Aq40Helpers::GetAq40LogUnit(openingTargets.veklor)
           << " veknilash_target=" << Aq40Helpers::GetAq40LogUnit(openingTargets.veknilash);
    Aq40Helpers::LogAq40Info(bot, "twin_validation", "twin:initial_engagement:arm_dual_pull",
        fields.str(), 1000);
}

void LogTwinPostTeleportPickupPending(Player* bot, Aq40TwinEncounter::TwinEncounterState const& state,
                                      Aq40TwinEncounter::TwinRoleAssignment const& assignment,
                                      char const* reason)
{
    if (!bot)
        return;

    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
    Unit* const currentTarget = botAI ? GetTwinCurrentTarget(botAI) : nullptr;
    Unit* const selectionTarget = (botAI && !bot->GetTarget().IsEmpty()) ? botAI->GetUnit(bot->GetTarget()) : nullptr;

    std::ostringstream fields;
    fields << "boss=twin phase=" << Aq40TwinEncounter::ToString(state.phase)
           << " mode=" << Aq40TwinEncounter::ToString(state.mode)
           << " reason=" << reason
           << " cohort=" << Aq40TwinEncounter::ToString(assignment.cohort)
           << " side=" << Aq40TwinEncounter::ToString(assignment.stableSide)
           << " slot=" << static_cast<uint32>(assignment.slotIndex)
           << " phase_elapsed_ms=" << Aq40TwinEncounter::GetPhaseElapsedMs(state)
           << " mode_elapsed_ms=" << GetTwinModeElapsedMs(state)
           << " teleport_elapsed_ms=" << GetTwinTeleportElapsedMs(state)
           << " current_target=" << Aq40Helpers::GetAq40LogUnit(currentTarget)
           << " selection_target=" << Aq40Helpers::GetAq40LogUnit(selectionTarget)
           << " victim=" << Aq40Helpers::GetAq40LogUnit(bot->GetVictim());
    AppendTwinOpeningOwnershipFields(fields, bot, state);
    Aq40Helpers::LogAq40Warn(bot, "twin_validation", "twin:post_swap:veklor_pickup_pending_overrun",
        fields.str(), 1000);
}

bool ShouldHoldTwinReserveTankAssignment(Aq40TwinEncounter::TwinRoleAssignment const& assignment)
{
    return IsTwinReserveTankHoldAssignment(assignment);
}

bool IsTwinWarlockTankController(Aq40TwinEncounter::TwinEncounterState const& state,
                                 Aq40TwinEncounter::TwinRoleAssignment const& assignment)
{
    return assignment.cohort == Aq40TwinEncounter::TwinRoleCohort::WarlockTank &&
           Aq40TwinEncounter::IsPrimaryController(
               state, Aq40TwinEncounter::TwinBoss::Veklor, assignment.memberGuid);
}

bool IsTwinMeleeTankController(Aq40TwinEncounter::TwinEncounterState const& state,
                               Aq40TwinEncounter::TwinRoleAssignment const& assignment)
{
    return assignment.cohort == Aq40TwinEncounter::TwinRoleCohort::MeleeTank &&
           Aq40TwinEncounter::IsPrimaryController(
               state, Aq40TwinEncounter::TwinBoss::Veknilash, assignment.memberGuid);
}

bool ShouldHoldTwinReserveTankAssignmentNow(Aq40TwinEncounter::TwinEncounterState const& state,
                                            Aq40TwinEncounter::TwinRoleAssignment const& assignment)
{
    if (!ShouldHoldTwinReserveTankAssignment(assignment))
        return false;

    if (assignment.cohort == Aq40TwinEncounter::TwinRoleCohort::WarlockTank)
        return !IsTwinWarlockTankController(state, assignment);

    if (assignment.cohort == Aq40TwinEncounter::TwinRoleCohort::MeleeTank)
        return !IsTwinMeleeTankController(state, assignment);

    return true;
}

bool TryArmTwinOpeningDualPullFromPrePull(Player* bot, Aq40TwinEncounter::TwinEncounterState& state,
                                          Aq40TwinEncounter::TwinRoleAssignment const& assignment,
                                          TwinOpeningTargets const& openingTargets)
{
    if (!bot || state.phase != Aq40TwinEncounter::TwinEncounterPhase::PrePull ||
        state.mode != Aq40TwinEncounter::TwinStrategyMode::StandardCompReady ||
        !Aq40TwinEncounter::HasDeterministicAssignments(state) ||
        !ShouldStartTwinOpeningPullFromPrePull(assignment) ||
        !AreTwinOpeningTargetsDiscoverable(openingTargets) ||
        !GetTwinOpeningTargetForAssignment(assignment, openingTargets))
    {
        return false;
    }

    uint32 const nowMs = getMSTime();
    Aq40TwinEncounter::SetMode(state, Aq40TwinEncounter::TwinStrategyMode::Combat, nowMs);
    Aq40TwinEncounter::EnterDualPullWindow(state, nowMs);

    std::ostringstream fields;
    fields << "boss=twin phase=" << Aq40TwinEncounter::ToString(state.phase)
           << " mode=" << Aq40TwinEncounter::ToString(state.mode)
            << " mode_elapsed_ms=" << GetTwinModeElapsedMs(state, nowMs)
           << " cohort=" << Aq40TwinEncounter::ToString(assignment.cohort)
           << " side=" << Aq40TwinEncounter::ToString(assignment.stableSide)
           << " slot=" << static_cast<uint32>(assignment.slotIndex)
           << " approach=" << state.approachMemberCount
           << " staged=" << state.stagedMemberCount
           << " center_committed=" << state.centerCommittedMemberCount
           << " strict_ready=" << state.strictReadyMemberCount
           << " assigned=" << state.assignments.size()
           << " opener_target=" << Aq40Helpers::GetAq40LogUnit(GetTwinOpeningTargetForAssignment(assignment, openingTargets))
           << " veklor_target=" << Aq40Helpers::GetAq40LogUnit(openingTargets.veklor)
           << " veknilash_target=" << Aq40Helpers::GetAq40LogUnit(openingTargets.veknilash);
    AppendTwinOpeningOwnershipFields(fields, bot, state, nowMs);
    Aq40Helpers::LogAq40Info(bot, "twin_dual_pull", "twin:prepull:arm_dual_pull", fields.str(), 1000);
    LogTwinInitialEngagementArm(bot, state, openingTargets);
    return true;
}

bool IsTwinPrimaryTankController(Aq40TwinEncounter::TwinEncounterState const& state,
                                 Aq40TwinEncounter::TwinRoleAssignment const* assignment)
{
    if (!assignment)
        return false;

    if (assignment->cohort == Aq40TwinEncounter::TwinRoleCohort::WarlockTank)
        return IsTwinWarlockTankController(state, *assignment);

    if (assignment->cohort == Aq40TwinEncounter::TwinRoleCohort::MeleeTank)
        return IsTwinMeleeTankController(state, *assignment);

    return false;
}

bool ShouldSuppressTwinWarlockVeklorThreat(Player* bot,
                                           Aq40TwinEncounter::TwinEncounterState const& state,
                                           Aq40TwinEncounter::TwinRoleAssignment const* assignment)
{
    if (!IsTwinWarlockProfile(bot))
        return false;

    if (assignment && assignment->cohort == Aq40TwinEncounter::TwinRoleCohort::WarlockTank &&
        IsTwinWarlockTankController(state, *assignment))
    {
        return false;
    }

    if (state.phase == Aq40TwinEncounter::TwinEncounterPhase::Stable &&
        !Aq40TwinEncounter::IsSwapPrepActive(state) &&
        !HasTwinCredibleStableController(state, Aq40TwinEncounter::TwinBoss::Veklor))
    {
        return true;
    }

    if (Aq40TwinEncounter::IsSwapPrepActive(state))
        return true;

    return !Aq40TwinEncounter::IsPickupEstablished(state, Aq40TwinEncounter::TwinBoss::Veklor) ||
           Aq40TwinEncounter::IsThreatHoldWindowActive(state, Aq40TwinEncounter::TwinBoss::Veklor);
}

Unit* FindTwinDualPullOpeningBossTarget(PlayerbotAI* botAI, Aq40TwinEncounter::TwinEncounterState const& state,
                                        Aq40TwinEncounter::TwinRoleAssignment const* assignment,
                                        TwinTargetIntent intent,
                                        GuidVector const& units)
{
    if (state.phase != Aq40TwinEncounter::TwinEncounterPhase::DualPullWindow)
        return nullptr;

    switch (intent)
    {
        case TwinTargetIntent::Veklor:
            if (!assignment || !IsTwinOpeningWarlockTankAssignment(*assignment) ||
                !IsTwinWarlockTankController(state, *assignment))
            {
                return nullptr;
            }

            if (Unit* veklor = FindTwinBoss(botAI, units, Aq40TwinEncounter::TwinBoss::Veklor))
                return veklor;

            return FindTwinBossForPrePull(botAI, Aq40TwinEncounter::TwinBoss::Veklor);

        case TwinTargetIntent::HoldVeknilash:
        case TwinTargetIntent::Veknilash:
            if (Unit* veknilash = FindTwinBoss(botAI, units, Aq40TwinEncounter::TwinBoss::Veknilash))
                return veknilash;

            return FindTwinBossForPrePull(botAI, Aq40TwinEncounter::TwinBoss::Veknilash);

        case TwinTargetIntent::HoldReserve:
        case TwinTargetIntent::None:
            break;
    }

    return nullptr;
}

bool IsStrictTwinDualPullOpeningAssignment(Aq40TwinEncounter::TwinEncounterState const& state,
                                           Aq40TwinEncounter::TwinRoleAssignment const* assignment)
{
    if (!assignment || state.phase != Aq40TwinEncounter::TwinEncounterPhase::DualPullWindow)
        return false;

    return (IsTwinOpeningWarlockTankAssignment(*assignment) && IsTwinWarlockTankController(state, *assignment)) ||
           (IsTwinOpeningMeleeTankAssignment(*assignment) && IsTwinMeleeTankController(state, *assignment));
}

TwinTargetIntent GetTwinTargetIntent(Player* bot, PlayerbotAI* botAI,
                                     Aq40TwinEncounter::TwinEncounterState const& state,
                                     Aq40TwinEncounter::TwinRoleAssignment const* assignment)
{
    if (assignment)
    {
        switch (assignment->cohort)
        {
            case Aq40TwinEncounter::TwinRoleCohort::WarlockTank:
                if (ShouldHoldTwinReserveTankAssignmentNow(state, *assignment))
                    return TwinTargetIntent::HoldReserve;

                if (!IsTwinWarlockTankController(state, *assignment))
                    return TwinTargetIntent::None;

                return TwinTargetIntent::Veklor;

            case Aq40TwinEncounter::TwinRoleCohort::MeleeTank:
                if (ShouldHoldTwinReserveTankAssignmentNow(state, *assignment))
                    return TwinTargetIntent::HoldReserve;

                if (!IsTwinMeleeTankController(state, *assignment))
                    return TwinTargetIntent::None;

                return TwinTargetIntent::Veknilash;

            case Aq40TwinEncounter::TwinRoleCohort::SideHealer:
            case Aq40TwinEncounter::TwinRoleCohort::RaidHealer:
                return TwinTargetIntent::None;

            case Aq40TwinEncounter::TwinRoleCohort::Hunter:
            case Aq40TwinEncounter::TwinRoleCohort::MeleeDps:
                return TwinTargetIntent::Veknilash;

            case Aq40TwinEncounter::TwinRoleCohort::RangedDps:
                if (state.phase == Aq40TwinEncounter::TwinEncounterPhase::DualPullWindow)
                    return TwinTargetIntent::HoldVeknilash;

                if (state.phase == Aq40TwinEncounter::TwinEncounterPhase::Stable &&
                    !Aq40TwinEncounter::IsSwapPrepActive(state) &&
                    !HasTwinCredibleStableController(state, Aq40TwinEncounter::TwinBoss::Veklor))
                {
                    return TwinTargetIntent::HoldVeknilash;
                }

                if (ShouldSuppressTwinWarlockVeklorThreat(bot, state, assignment))
                    return TwinTargetIntent::HoldVeknilash;

                if (Aq40TwinEncounter::IsThreatHoldWindowActive(state, Aq40TwinEncounter::TwinBoss::Veklor))
                    return TwinTargetIntent::HoldVeknilash;

                return TwinTargetIntent::Veklor;

            case Aq40TwinEncounter::TwinRoleCohort::None:
                break;
        }
    }

    if (ShouldSuppressTwinWarlockVeklorThreat(bot, state, assignment))
        return TwinTargetIntent::HoldVeknilash;

    if (state.phase == Aq40TwinEncounter::TwinEncounterPhase::Stable &&
        !Aq40TwinEncounter::IsSwapPrepActive(state) &&
        !HasTwinCredibleStableController(state, Aq40TwinEncounter::TwinBoss::Veklor) &&
        !IsTwinWarlockProfile(bot))
    {
        return TwinTargetIntent::HoldVeknilash;
    }

    if (Aq40TwinEncounter::IsThreatHoldWindowActive(state, Aq40TwinEncounter::TwinBoss::Veklor) &&
        !IsTwinWarlockProfile(bot))
    {
        return TwinTargetIntent::HoldVeknilash;
    }

    if (state.phase == Aq40TwinEncounter::TwinEncounterPhase::DualPullWindow &&
        !IsTwinWarlockProfile(bot))
    {
        return TwinTargetIntent::HoldVeknilash;
    }

    return IsTwinMeleeProfile(bot, botAI) ? TwinTargetIntent::Veknilash : TwinTargetIntent::Veklor;
}

char const* GetTwinTargetReason(TwinTargetIntent intent)
{
    switch (intent)
    {
        case TwinTargetIntent::Veklor:
            return "veklor";
        case TwinTargetIntent::Veknilash:
            return "veknilash";
        case TwinTargetIntent::HoldReserve:
            return "hold_reserve";
        case TwinTargetIntent::HoldVeknilash:
            return "hold_veknilash";
        case TwinTargetIntent::None:
            return "none";
    }

    return "none";
}

Aq40TwinEncounter::TwinRoleCohort GetTwinEffectiveBugServiceCohort(
    Aq40TwinEncounter::TwinRoleAssignment const* assignment, Player* bot, PlayerbotAI* botAI)
{
    if (assignment)
        return assignment->cohort;

    if (bot && bot->getClass() == CLASS_HUNTER)
        return Aq40TwinEncounter::TwinRoleCohort::Hunter;

    return IsTwinMeleeProfile(bot, botAI) ? Aq40TwinEncounter::TwinRoleCohort::MeleeDps
                                          : Aq40TwinEncounter::TwinRoleCohort::RangedDps;
}

bool IsTwinBugServiceRole(Aq40TwinEncounter::TwinRoleAssignment const* assignment, Player* bot,
                          PlayerbotAI* botAI)
{
    switch (GetTwinEffectiveBugServiceCohort(assignment, bot, botAI))
    {
        case Aq40TwinEncounter::TwinRoleCohort::RangedDps:
        case Aq40TwinEncounter::TwinRoleCohort::Hunter:
            return true;

        default:
            return false;
    }
}

bool IsTwinBugServiceWindow(Player* bot, Aq40TwinEncounter::TwinEncounterState const& state)
{
    return state.phase == Aq40TwinEncounter::TwinEncounterPhase::Stable &&
           state.recovery.splitBand == Aq40TwinEncounter::TwinSplitBand::Stable &&
           !Aq40TwinEncounter::IsSwapPrepActive(state) &&
           !Aq40TwinEncounter::IsAnyThreatHoldWindowActive(state) &&
           !Aq40TwinEncounter::HasActiveLockedPickupAnchor(bot);
}

float GetTwinBugServiceRange(Player* bot, PlayerbotAI* botAI,
                             Aq40TwinEncounter::TwinRoleAssignment const* assignment,
                             TwinBugPriority priority)
{
    switch (GetTwinEffectiveBugServiceCohort(assignment, bot, botAI))
    {
        case Aq40TwinEncounter::TwinRoleCohort::RangedDps:
            return priority == TwinBugPriority::Hostile ? kTwinRangedGenericBugServiceRange
                                                        : kTwinRangedBugServiceRange;

        case Aq40TwinEncounter::TwinRoleCohort::Hunter:
            return priority == TwinBugPriority::Hostile ? kTwinRangedGenericBugServiceRange
                                                        : kTwinHunterMarkedBugServiceRange;

        default:
            return 0.0f;
    }
}

float GetTwinBugArcaneSafeRadius(Player* bot, PlayerbotAI* botAI,
                                 Aq40TwinEncounter::TwinRoleAssignment const* assignment)
{
    switch (GetTwinEffectiveBugServiceCohort(assignment, bot, botAI))
    {
        case Aq40TwinEncounter::TwinRoleCohort::Hunter:
            return kTwinHunterBugArcaneSafeRadius;

        case Aq40TwinEncounter::TwinRoleCohort::MeleeDps:
            return kTwinMeleeBugArcaneSafeRadius;

        case Aq40TwinEncounter::TwinRoleCohort::RangedDps:
            return kTwinRangedBugArcaneSafeRadius;

        default:
            return kTwinArcaneBurstDangerRadius;
    }
}

bool DoesTwinRoleAllowBugPriority(Player* bot, PlayerbotAI* botAI,
                                  Aq40TwinEncounter::TwinRoleAssignment const* assignment,
                                  TwinBugPriority priority)
{
    switch (GetTwinEffectiveBugServiceCohort(assignment, bot, botAI))
    {
        case Aq40TwinEncounter::TwinRoleCohort::RangedDps:
            return priority != TwinBugPriority::None;

        case Aq40TwinEncounter::TwinRoleCohort::Hunter:
            return priority != TwinBugPriority::None;

        default:
            return false;
    }
}

bool IsTwinBugMarkedBySpell(PlayerbotAI* botAI, Unit* bug, uint32 spellId)
{
    if (!botAI || !bug)
        return false;

    Spell* spell = bug->GetCurrentSpell(CURRENT_GENERIC_SPELL);
    return (spell && Aq40SpellIds::MatchesAnySpellId(spell->GetSpellInfo(), { spellId })) ||
           Aq40SpellIds::HasAnyAura(botAI, bug, { spellId });
}

TwinBugPriority GetTwinBugPriority(Player* bot, PlayerbotAI* botAI,
                                   Aq40TwinEncounter::TwinEncounterState const& state, Unit* bug,
                                   bool allowScriptedStickyPriority = false)
{
    if (!bot || !botAI || !bug || !bug->IsAlive() || !Aq40SpellIds::IsTwinBugEntry(bug->GetEntry()))
        return TwinBugPriority::None;

    if (IsTwinBugMarkedBySpell(botAI, bug, Aq40SpellIds::TwinExplodeBug))
        return TwinBugPriority::Explode;

    if (IsTwinBugMarkedBySpell(botAI, bug, Aq40SpellIds::TwinMutateBug))
        return TwinBugPriority::Mutate;

    if (allowScriptedStickyPriority)
    {
        if (Aq40TwinEncounter::IsScriptedEventActive(
                state, Aq40TwinEncounter::TwinScriptedEvent::ExplodeBug, kTwinExplodeBugWindowMs))
        {
            return TwinBugPriority::Explode;
        }

        if (Aq40TwinEncounter::IsScriptedEventActive(
                state, Aq40TwinEncounter::TwinScriptedEvent::MutateBug, kTwinMutateBugWindowMs))
        {
            return TwinBugPriority::Mutate;
        }
    }

    return TwinBugPriority::Hostile;
}

char const* GetTwinBugReason(TwinBugPriority priority)
{
    switch (priority)
    {
        case TwinBugPriority::Explode:
            return "explode_bug";
        case TwinBugPriority::Mutate:
            return "mutate_bug";
        case TwinBugPriority::Hostile:
            return "bug_cleanup";
        case TwinBugPriority::None:
            return "none";
    }

    return "none";
}

Position GetTwinBugServiceOrigin(Player* bot, Aq40TwinEncounter::TwinEncounterState const& state,
                                 Aq40TwinEncounter::TwinRoleAssignment const* assignment)
{
    if (assignment)
        return GetTwinStableAnchorChoice(state, *assignment).anchor.position;

    Position origin;
    if (bot)
    {
        origin.Relocate(bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ());
    }

    return origin;
}

Aq40TwinEncounter::TwinSide GetTwinBugServiceSide(Player* bot,
                                                  Aq40TwinEncounter::TwinRoleAssignment const* assignment,
                                                  Position const& origin)
{
    if (assignment && Aq40TwinEncounter::IsKnownSide(assignment->stableSide))
        return assignment->stableSide;

    if (!bot)
        return Aq40TwinEncounter::TwinSide::Unknown;

    return GetTwinSideForPosition(origin.GetPositionX(), origin.GetPositionY());
}

bool IsTwinBugSafeForService(Player* bot, PlayerbotAI* botAI,
                             Aq40TwinEncounter::TwinRoleAssignment const* assignment, Unit* veklor,
                             Unit* bug, TwinBugPriority priority, Position const& origin,
                             Aq40TwinEncounter::TwinSide serviceSide)
{
    if (!bot || !botAI || !bug || !bug->IsAlive() || priority == TwinBugPriority::None)
        return false;

    if (!DoesTwinRoleAllowBugPriority(bot, botAI, assignment, priority))
        return false;

    if (!bot->IsWithinLOSInMap(bug))
        return false;

    float const maxServiceDistance = GetTwinBugServiceRange(bot, botAI, assignment, priority);
    if (maxServiceDistance <= 0.0f)
        return false;

    float const serviceDistance = origin.GetExactDist2d(bug->GetPositionX(), bug->GetPositionY());
    if (serviceDistance > maxServiceDistance)
        return false;

    Aq40TwinEncounter::TwinSide const bugSide =
        GetTwinSideForPosition(bug->GetPositionX(), bug->GetPositionY());
    if (Aq40TwinEncounter::IsKnownSide(serviceSide) && bugSide != serviceSide)
        return false;

    if (veklor && bug->GetDistance2d(veklor) < GetTwinBugArcaneSafeRadius(bot, botAI, assignment))
        return false;

    if (priority == TwinBugPriority::Explode && bot->GetDistance2d(bug) < kTwinExplodeBugServiceSafeRadius)
        return false;

    return true;
}

Unit* FindTwinBugServiceTarget(Player* bot, PlayerbotAI* botAI,
                               Aq40TwinEncounter::TwinEncounterState const& state,
                               Aq40TwinEncounter::TwinRoleAssignment const* assignment,
                               GuidVector const& units, Unit* currentTarget, Unit* currentVictim,
                               char const*& outReason)
{
    if (!IsTwinBugServiceWindow(bot, state) || !IsTwinBugServiceRole(assignment, bot, botAI))
        return nullptr;

    Unit* veklor = FindTwinBoss(botAI, units, Aq40TwinEncounter::TwinBoss::Veklor);
    Position const serviceOrigin = GetTwinBugServiceOrigin(bot, state, assignment);
    Aq40TwinEncounter::TwinSide const serviceSide =
        GetTwinBugServiceSide(bot, assignment, serviceOrigin);

    TwinBugSelection best;
    for (ObjectGuid const guid : units)
    {
        Unit* bug = botAI->GetUnit(guid);
        TwinBugPriority const priority = GetTwinBugPriority(bot, botAI, state, bug);
        if (!IsTwinBugSafeForService(
            bot, botAI, assignment, veklor, bug, priority, serviceOrigin, serviceSide))
        {
            continue;
        }

        float const serviceDistance =
            serviceOrigin.GetExactDist2d(bug->GetPositionX(), bug->GetPositionY());
        if (!best.target || priority < best.priority ||
            (priority == best.priority && serviceDistance < best.serviceDistance))
        {
            best.target = bug;
            best.priority = priority;
            best.serviceDistance = serviceDistance;
        }
    }

    Unit* stickyTarget = nullptr;
    if (currentVictim && Aq40SpellIds::IsTwinBugEntry(currentVictim->GetEntry()))
        stickyTarget = currentVictim;
    else if (currentTarget && Aq40SpellIds::IsTwinBugEntry(currentTarget->GetEntry()))
        stickyTarget = currentTarget;

    if (stickyTarget)
    {
        TwinBugPriority const stickyPriority =
            GetTwinBugPriority(bot, botAI, state, stickyTarget, true);
        if (IsTwinBugSafeForService(bot, botAI, assignment, veklor, stickyTarget, stickyPriority,
            serviceOrigin, serviceSide) &&
            (!best.target || stickyPriority < best.priority ||
                (stickyPriority == best.priority &&
                    serviceOrigin.GetExactDist2d(
                        stickyTarget->GetPositionX(), stickyTarget->GetPositionY()) <=
                        best.serviceDistance + 3.0f)))
        {
            best.target = stickyTarget;
            best.priority = stickyPriority;
        }
    }

    if (!best.target)
        return nullptr;

    outReason = GetTwinBugReason(best.priority);
    return best.target;
}

void BindTwinPetControlState(Aq40TwinEncounter::TwinPetControlState& controlState, Pet* pet)
{
    if (!pet)
        return;

    if (controlState.petGuid == pet->GetGUID())
        return;

    controlState.petGuid = pet->GetGUID();
    controlState.forcedPassive = false;
    controlState.previousReactStateCaptured = false;
    controlState.previousReactState = static_cast<uint8>(pet->GetReactState());
    controlState.disabledAutocastSpellIds.clear();
}

bool IsTwinPetAutocastEnabled(Pet const* pet, uint32 spellId)
{
    if (!pet)
        return false;

    return std::find(pet->m_autospells.begin(), pet->m_autospells.end(), spellId) != pet->m_autospells.end();
}

bool SetTwinPetReactState(Pet* pet, ReactStates reactState)
{
    if (!pet || pet->GetReactState() == reactState)
        return false;

    pet->SetReactState(reactState);
    if (CharmInfo* charmInfo = pet->GetCharmInfo())
        charmInfo->SetPlayerReactState(reactState);

    return true;
}

bool IsTwinPetTauntSpell(SpellInfo const* spellInfo)
{
    if (!spellInfo)
        return false;

    std::string const spellToken = Aq40Helpers::GetAq40LogToken(spellInfo->SpellName[0]);
    return spellToken == "growl" || spellToken == "torment" || spellToken == "suffering";
}

bool DisableTwinPetTauntAutocast(Player* bot, Pet* pet)
{
    if (!bot || !pet)
        return false;

    Aq40TwinEncounter::TwinPetControlState& controlState = Aq40TwinEncounter::EnsurePetControlState(bot);
    BindTwinPetControlState(controlState, pet);

    bool changed = false;
    for (PetSpellMap::const_iterator itr = pet->m_spells.begin(); itr != pet->m_spells.end(); ++itr)
    {
        if (itr->second.state == PETSPELL_REMOVED)
            continue;

        uint32 const spellId = itr->first;
        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
        if (!spellInfo || !spellInfo->IsAutocastable() || !IsTwinPetTauntSpell(spellInfo) ||
            !IsTwinPetAutocastEnabled(pet, spellId))
        {
            continue;
        }

        pet->ToggleAutocast(spellInfo, false);
        if (std::find(controlState.disabledAutocastSpellIds.begin(), controlState.disabledAutocastSpellIds.end(),
                spellId) == controlState.disabledAutocastSpellIds.end())
        {
            controlState.disabledAutocastSpellIds.push_back(spellId);
        }

        changed = true;
    }

    if (changed)
    {
        Aq40Helpers::LogAq40Info(bot, "twin_pet", "twin:pet:disable_taunts",
            std::string("boss=twin pet=") + Aq40Helpers::GetAq40LogUnit(pet), 1000);
    }

    return changed;
}

bool HasTwinPetTauntAutocastEnabled(Pet* pet)
{
    if (!pet)
        return false;

    for (PetSpellMap::const_iterator itr = pet->m_spells.begin(); itr != pet->m_spells.end(); ++itr)
    {
        if (itr->second.state == PETSPELL_REMOVED)
            continue;

        uint32 const spellId = itr->first;
        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
        if (spellInfo && spellInfo->IsAutocastable() && IsTwinPetTauntSpell(spellInfo) &&
            IsTwinPetAutocastEnabled(pet, spellId))
        {
            return true;
        }
    }

    return false;
}

bool NeedsTwinPetPassiveCleanup(Pet* pet)
{
    if (!pet)
        return false;

    if (pet->GetReactState() != REACT_PASSIVE || pet->GetVictim() || pet->IsInCombat())
        return true;

    if (CharmInfo* charmInfo = pet->GetCharmInfo())
    {
        if (charmInfo->IsCommandAttack() || !charmInfo->IsReturning())
            return true;
    }

    return HasTwinPetTauntAutocastEnabled(pet);
}

bool ForceTwinPetPassiveFollow(Player* bot, PlayerbotAI* botAI, Pet* pet)
{
    if (!bot || !botAI || !pet)
        return false;

    Aq40TwinEncounter::TwinPetControlState& controlState = Aq40TwinEncounter::EnsurePetControlState(bot);
    BindTwinPetControlState(controlState, pet);

    if (!controlState.previousReactStateCaptured)
    {
        controlState.previousReactState = static_cast<uint8>(pet->GetReactState());
        controlState.previousReactStateCaptured = true;
    }

    bool changed = false;
    bool needsFollow = pet->GetVictim() || pet->IsInCombat();
    if (CharmInfo* charmInfo = pet->GetCharmInfo())
        needsFollow = needsFollow || charmInfo->IsCommandAttack() || !charmInfo->IsReturning();

    if (SetTwinPetReactState(pet, REACT_PASSIVE))
    {
        changed = true;
        needsFollow = true;
    }

    controlState.forcedPassive = true;
    if (needsFollow)
    {
        botAI->PetFollow();
        changed = true;
    }

    return changed;
}

bool ReleaseTwinPetPassive(Player* bot, Pet* pet)
{
    if (!bot || !pet)
        return false;

    Aq40TwinEncounter::TwinPetControlState* controlState = Aq40TwinEncounter::GetPetControlState(bot);
    if (!controlState || controlState->petGuid != pet->GetGUID() || !controlState->forcedPassive)
        return false;

    ReactStates const reactState = controlState->previousReactStateCaptured
        ? static_cast<ReactStates>(controlState->previousReactState)
        : REACT_DEFENSIVE;
    bool const changed = SetTwinPetReactState(pet, reactState);
    controlState->forcedPassive = false;
    controlState->previousReactStateCaptured = false;
    controlState->previousReactState = static_cast<uint8>(reactState);
    return changed;
}

bool ShouldApplyTwinPetPolicy(Player* bot, Aq40TwinEncounter::TwinEncounterState const& state)
{
    if (!bot)
        return false;

    if (Aq40TwinEncounter::IsTwinApproachWindow(state, bot) ||
        Aq40TwinEncounter::IsTwinPrePullStageWindow(state, bot))
        return true;

    if (!Aq40TwinEncounter::IsTwinEncounterParticipant(bot))
        return false;

    return Aq40TwinEncounter::IsActivePhase(state.phase) || Aq40TwinEncounter::IsTerminalPhase(state.phase) ||
           Aq40TwinEncounter::IsAnyThreatHoldWindowActive(state) ||
           Aq40TwinEncounter::HasActiveLockedPickupAnchor(bot);
}

bool IsTwinHunterPetSafeTarget(Player* bot, PlayerbotAI* botAI,
                               Aq40TwinEncounter::TwinEncounterState const& state,
                               Aq40TwinEncounter::TwinRoleAssignment const* assignment,
                               Unit* target, GuidVector const& units)
{
    if (!bot || !botAI || bot->getClass() != CLASS_HUNTER || !assignment ||
        assignment->cohort != Aq40TwinEncounter::TwinRoleCohort::Hunter || !target || !target->IsAlive() ||
        target->GetEntry() != Aq40SpellIds::TwinVeknilashNpcEntry)
    {
        return false;
    }

    if (state.phase != Aq40TwinEncounter::TwinEncounterPhase::Stable ||
        state.recovery.splitBand != Aq40TwinEncounter::TwinSplitBand::Stable ||
        Aq40TwinEncounter::IsSwapPrepActive(state) || Aq40TwinEncounter::HasActiveLockedPickupAnchor(bot) ||
        Aq40TwinEncounter::IsAnyThreatHoldWindowActive(state) ||
        Aq40TwinEncounter::IsTerminalPhase(state.phase))
    {
        return false;
    }

    Aq40TwinEncounter::TwinSide const targetSide =
        GetTwinSideForPosition(target->GetPositionX(), target->GetPositionY());
    if (assignment->stableSide != targetSide)
        return false;

    if (GetTwinSideForPosition(bot->GetPositionX(), bot->GetPositionY()) != targetSide)
        return false;

    if (Pet* pet = bot->GetPet())
    {
        if (GetTwinSideForPosition(pet->GetPositionX(), pet->GetPositionY()) != targetSide)
            return false;
    }

    Unit* veklor = FindTwinBoss(botAI, units, Aq40TwinEncounter::TwinBoss::Veklor);
    if (!veklor)
        return true;

    float constexpr kTwinHunterPetSafeRadius = kTwinArcaneBurstLooseRadius + 2.0f;
    if (bot->GetDistance2d(veklor) <= kTwinHunterPetSafeRadius)
        return false;

    if (Pet* pet = bot->GetPet())
    {
        if (pet->GetDistance2d(veklor) <= kTwinHunterPetSafeRadius)
            return false;
    }

    return true;
}

char const* GetTwinPetHoldReason(Player* bot, Aq40TwinEncounter::TwinEncounterState const& state, Unit* target)
{
    if (!bot)
        return "invalid";

    if (state.phase == Aq40TwinEncounter::TwinEncounterPhase::PrePull)
        return "prepull_stage";
    if (state.phase == Aq40TwinEncounter::TwinEncounterPhase::DualPullWindow)
        return "dual_pull";
    if (Aq40TwinEncounter::IsSwapPrepActive(state))
        return "swap_prep";
    if (state.phase == Aq40TwinEncounter::TwinEncounterPhase::TeleportWindow)
        return "teleport_window";
    if (state.phase == Aq40TwinEncounter::TwinEncounterPhase::PickupRecovery)
        return "pickup_recovery";
    if (Aq40TwinEncounter::HasActiveLockedPickupAnchor(bot) || Aq40TwinEncounter::IsAnyThreatHoldWindowActive(state))
        return "post_swap_hold";
    if (Aq40TwinEncounter::IsTerminalPhase(state.phase))
        return "terminal_failure";
    if (state.recovery.splitBand != Aq40TwinEncounter::TwinSplitBand::Stable)
        return "split_risk";
    if (!target)
        return "no_safe_target";
    if (Aq40SpellIds::IsTwinBugEntry(target->GetEntry()))
        return "bug_target";
    if (bot->getClass() != CLASS_HUNTER)
        return "non_hunter_pet";
    return "unsafe_target";
}

bool ShouldTwinHoldPetPassive(Player* bot, PlayerbotAI* botAI,
                              Aq40TwinEncounter::TwinEncounterState const& state,
                              Aq40TwinEncounter::TwinRoleAssignment const* assignment,
                              Unit* target, GuidVector const& units)
{
    if (!ShouldApplyTwinPetPolicy(bot, state))
        return false;

    if (state.phase == Aq40TwinEncounter::TwinEncounterPhase::PrePull ||
        state.phase == Aq40TwinEncounter::TwinEncounterPhase::DualPullWindow ||
        state.phase == Aq40TwinEncounter::TwinEncounterPhase::TeleportWindow ||
        state.phase == Aq40TwinEncounter::TwinEncounterPhase::PickupRecovery ||
        Aq40TwinEncounter::IsSwapPrepActive(state) ||
        Aq40TwinEncounter::HasActiveLockedPickupAnchor(bot) ||
        Aq40TwinEncounter::IsAnyThreatHoldWindowActive(state) ||
        Aq40TwinEncounter::IsTerminalPhase(state.phase) ||
        state.recovery.splitBand != Aq40TwinEncounter::TwinSplitBand::Stable)
    {
        return true;
    }

    if (!target || Aq40SpellIds::IsTwinBugEntry(target->GetEntry()))
        return true;

    if (bot->getClass() != CLASS_HUNTER)
        return true;

    return !IsTwinHunterPetSafeTarget(bot, botAI, state, assignment, target, units);
}

bool SyncTwinEncounterPetPolicy(Player* bot, PlayerbotAI* botAI,
                                Aq40TwinEncounter::TwinEncounterState const& state,
                                Aq40TwinEncounter::TwinRoleAssignment const* assignment,
                                Unit* target, GuidVector const& units)
{
    if (!bot || !botAI)
        return false;

    Pet* pet = bot->GetPet();
    if (!pet)
        return Aq40TwinEncounter::RestorePetControlState(bot);

    if (!ShouldApplyTwinPetPolicy(bot, state))
        return Aq40TwinEncounter::RestorePetControlState(bot);

    bool changed = DisableTwinPetTauntAutocast(bot, pet);
    if (ShouldTwinHoldPetPassive(bot, botAI, state, assignment, target, units))
    {
        bool const passiveChanged = ForceTwinPetPassiveFollow(bot, botAI, pet);
        if (passiveChanged)
        {
            Aq40Helpers::LogAq40Info(bot, "twin_pet", "twin:pet:hold_passive",
                std::string("boss=twin reason=") + GetTwinPetHoldReason(bot, state, target) +
                    " pet=" + Aq40Helpers::GetAq40LogUnit(pet) +
                    " target=" + Aq40Helpers::GetAq40LogUnit(target),
                1000);
        }

        return passiveChanged || changed;
    }

    bool const released = ReleaseTwinPetPassive(bot, pet);
    if (released)
    {
        Aq40Helpers::LogAq40Info(bot, "twin_pet", "twin:pet:release_safe_target",
            std::string("boss=twin pet=") + Aq40Helpers::GetAq40LogUnit(pet) +
                " target=" + Aq40Helpers::GetAq40LogUnit(target),
            1000);
    }

    return released || changed;
}

bool HasTwinLocalCombatStateToClear(Player* bot, PlayerbotAI* botAI)
{
    if (!bot || !botAI || !botAI->GetAiObjectContext())
        return false;

    auto* context = botAI->GetAiObjectContext();
    if (context->GetValue<Unit*>("old target")->Get() || context->GetValue<Unit*>("current target")->Get() ||
        !context->GetValue<GuidVector>("prioritized targets")->Get().empty() ||
        !context->GetValue<ObjectGuid>("pull target")->Get().IsEmpty() ||
        !context->GetValue<ObjectGuid>("pull strategy target")->Get().IsEmpty() || bot->GetTarget() ||
        !context->GetValue<std::list<ObjectGuid>>("focus heal targets")->Get().empty() ||
        botAI->HasStrategy("focus heal targets", BOT_STATE_COMBAT) ||
        !context->GetValue<std::string>("rti")->Get().empty() ||
        !context->GetValue<std::string>("rti cc")->Get().empty() ||
        context->GetValue<Unit*>("rti target")->Get() || context->GetValue<Unit*>("rti cc target")->Get())
    {
        return true;
    }

    return false;
}

bool NeedsTwinApproachCleanup(Player* bot, PlayerbotAI* botAI)
{
    if (!bot || !botAI)
        return false;

    if (!Aq40TwinEncounter::HasTwinLocalCleanupState(bot))
        return true;

    if (HasTwinLocalCombatStateToClear(bot, botAI))
        return true;

    if (Pet* pet = bot->GetPet())
    {
        if (NeedsTwinPetPassiveCleanup(pet))
            return true;
    }

    return bot->getClass() == CLASS_WARLOCK &&
           botAI->HasStrategy("tank", BOT_STATE_COMBAT) &&
           !Aq40TwinEncounter::ShouldUseTwinWarlockTankStrategy(bot);
}

Aq40TwinEncounter::TwinSide GetTwinAssignedSide(Aq40TwinEncounter::TwinEncounterState const& state,
                                                ObjectGuid memberGuid)
{
    Aq40TwinEncounter::TwinRoleAssignment const* assignment =
        Aq40TwinEncounter::GetAssignmentForMember(state, memberGuid);
    return assignment ? assignment->stableSide : Aq40TwinEncounter::TwinSide::Unknown;
}

Aq40TwinEncounter::TwinSide GetTwinExpectedOwnerSide(Aq40TwinEncounter::TwinEncounterState const& state,
                                                     Aq40TwinEncounter::TwinBoss boss)
{
    return GetTwinAssignedSide(state, Aq40TwinEncounter::GetOwnership(state, boss).expectedOwner);
}

Aq40TwinEncounter::TwinSide GetTwinMeleeDpsExpectedSide(Aq40TwinEncounter::TwinEncounterState const& state,
                                                        Aq40TwinEncounter::TwinRoleAssignment const& assignment)
{
    Aq40TwinEncounter::TwinSide const expectedSide =
        GetTwinExpectedOwnerSide(state, Aq40TwinEncounter::TwinBoss::Veknilash);
    return Aq40TwinEncounter::IsKnownSide(expectedSide) ? expectedSide : assignment.stableSide;
}

uint8 GetTwinCrossSideSlotIndex(Aq40TwinEncounter::TwinRoleAssignment const& assignment)
{
    if (!Aq40TwinEncounter::IsKnownSide(assignment.stableSide))
        return assignment.slotIndex;

    uint32 const combinedSlot = static_cast<uint32>(assignment.slotIndex) * 2u + ToSideIndex(assignment.stableSide);
    return combinedSlot <= std::numeric_limits<uint8>::max() ? static_cast<uint8>(combinedSlot)
                                                              : assignment.slotIndex;
}

ObjectGuid GetTwinCurrentControllerGuidForValidation(Aq40TwinEncounter::TwinEncounterState const& state,
                                                     Aq40TwinEncounter::TwinBoss boss)
{
    ObjectGuid controllerGuid = Aq40TwinEncounter::GetPickupOwner(state, boss);
    if (!controllerGuid.IsEmpty())
        return controllerGuid;

    Aq40TwinEncounter::TwinStableOwnership const& ownership = Aq40TwinEncounter::GetOwnership(state, boss);
    if (!ownership.stableOwner.IsEmpty())
        return ownership.stableOwner;
    if (!ownership.candidateOwner.IsEmpty())
        return ownership.candidateOwner;

    return ownership.expectedOwner;
}

bool HasTwinCredibleStableController(Aq40TwinEncounter::TwinEncounterState const& state,
                                     Aq40TwinEncounter::TwinBoss boss)
{
    if (state.phase != Aq40TwinEncounter::TwinEncounterPhase::Stable ||
        Aq40TwinEncounter::IsSwapPrepActive(state))
    {
        return true;
    }

    ObjectGuid const controllerGuid = GetTwinCurrentControllerGuidForValidation(state, boss);
    if (controllerGuid.IsEmpty())
        return false;

    Aq40TwinEncounter::TwinRoleAssignment const* assignment =
        Aq40TwinEncounter::GetAssignmentForMember(state, controllerGuid);
    if (!assignment)
        return false;

    Aq40TwinEncounter::TwinRoleCohort const expectedCohort =
        boss == Aq40TwinEncounter::TwinBoss::Veklor ? Aq40TwinEncounter::TwinRoleCohort::WarlockTank
                                                    : Aq40TwinEncounter::TwinRoleCohort::MeleeTank;
    if (assignment->cohort != expectedCohort)
        return false;

    Aq40TwinEncounter::TwinSide const expectedSide = GetTwinExpectedOwnerSide(state, boss);
    if (Aq40TwinEncounter::IsKnownSide(expectedSide) && assignment->stableSide != expectedSide)
        return false;

    Aq40TwinEncounter::TwinStableOwnership const& ownership = Aq40TwinEncounter::GetOwnership(state, boss);
    if (!ownership.lastValidConfirmationMs)
        return false;

    return Aq40TwinEncounter::GetTimeSinceOwnershipConfirmationMs(state, boss) <=
           kTwinStableControllerConfirmationWindowMs;
}

bool DoesTwinAssignmentAllowBossTarget(Aq40TwinEncounter::TwinEncounterState const& state,
                                       Aq40TwinEncounter::TwinRoleAssignment const* assignment,
                                       Unit const* target)
{
    if (!target || !IsTwinEmperorTarget(target))
        return true;

    if (!assignment)
        return false;

    auto const isStableSideOwnedVeknilashWindow = [&]() -> bool
    {
        if (!IsTwinVeknilashTarget(target) || Aq40TwinEncounter::IsSwapPrepActive(state) ||
            state.phase == Aq40TwinEncounter::TwinEncounterPhase::TeleportWindow ||
            state.phase == Aq40TwinEncounter::TwinEncounterPhase::PickupRecovery)
        {
            return false;
        }

        bool const openingOrStableWindow =
            state.phase == Aq40TwinEncounter::TwinEncounterPhase::DualPullWindow ||
            (state.phase == Aq40TwinEncounter::TwinEncounterPhase::Stable &&
             state.recovery.splitBand == Aq40TwinEncounter::TwinSplitBand::Stable);
        if (!openingOrStableWindow)
            return false;

        return GetTwinSideForPosition(target->GetPositionX(), target->GetPositionY()) == assignment->stableSide;
    };

    auto const isExpectedMeleeVeknilashWindow = [&]() -> bool
    {
        if (!IsTwinVeknilashTarget(target) || Aq40TwinEncounter::IsSwapPrepActive(state) ||
            state.phase == Aq40TwinEncounter::TwinEncounterPhase::TeleportWindow ||
            state.phase == Aq40TwinEncounter::TwinEncounterPhase::PickupRecovery)
        {
            return false;
        }

        bool const openingOrStableWindow =
            state.phase == Aq40TwinEncounter::TwinEncounterPhase::DualPullWindow ||
            (state.phase == Aq40TwinEncounter::TwinEncounterPhase::Stable &&
             state.recovery.splitBand == Aq40TwinEncounter::TwinSplitBand::Stable);
        if (!openingOrStableWindow)
            return false;

        Aq40TwinEncounter::TwinSide const expectedSide = GetTwinMeleeDpsExpectedSide(state, *assignment);
        return !Aq40TwinEncounter::IsKnownSide(expectedSide) ||
               GetTwinSideForPosition(target->GetPositionX(), target->GetPositionY()) == expectedSide;
    };

    switch (assignment->cohort)
    {
        case Aq40TwinEncounter::TwinRoleCohort::WarlockTank:
            return IsTwinVeklorTarget(target) && IsTwinWarlockTankController(state, *assignment);

        case Aq40TwinEncounter::TwinRoleCohort::MeleeTank:
            return IsTwinVeknilashTarget(target);

        case Aq40TwinEncounter::TwinRoleCohort::SideHealer:
        case Aq40TwinEncounter::TwinRoleCohort::RaidHealer:
            return false;

        case Aq40TwinEncounter::TwinRoleCohort::Hunter:
            return isStableSideOwnedVeknilashWindow();

        case Aq40TwinEncounter::TwinRoleCohort::MeleeDps:
            return isExpectedMeleeVeknilashWindow();

        case Aq40TwinEncounter::TwinRoleCohort::RangedDps:
            if (!IsTwinVeklorTarget(target) || state.phase != Aq40TwinEncounter::TwinEncounterPhase::Stable ||
                state.recovery.splitBand != Aq40TwinEncounter::TwinSplitBand::Stable ||
                Aq40TwinEncounter::IsSwapPrepActive(state) ||
                state.phase == Aq40TwinEncounter::TwinEncounterPhase::TeleportWindow ||
                state.phase == Aq40TwinEncounter::TwinEncounterPhase::PickupRecovery ||
                !HasTwinCredibleStableController(state, Aq40TwinEncounter::TwinBoss::Veklor))
            {
                return false;
            }

            return true;

        case Aq40TwinEncounter::TwinRoleCohort::None:
        default:
            return false;
    }
}

bool IsTwinPendingSwapPrepExpectedOwner(Aq40TwinEncounter::TwinEncounterState const& state,
                                        Aq40TwinEncounter::TwinBoss boss,
                                        Aq40TwinEncounter::TwinRoleAssignment const& assignment)
{
    return Aq40TwinEncounter::IsSwapPrepActive(state) &&
           Aq40TwinEncounter::GetOwnership(state, boss).expectedOwner == assignment.memberGuid;
}

bool IsTwinPostTeleportVeklorPickupWindow(Aq40TwinEncounter::TwinEncounterState const& state)
{
    return !Aq40TwinEncounter::IsPickupEstablished(state, Aq40TwinEncounter::TwinBoss::Veklor) &&
           (state.phase == Aq40TwinEncounter::TwinEncounterPhase::TeleportWindow ||
            state.phase == Aq40TwinEncounter::TwinEncounterPhase::PickupRecovery ||
            Aq40TwinEncounter::IsThreatHoldWindowActive(state, Aq40TwinEncounter::TwinBoss::Veklor));
}

bool ShouldStageTwinIncomingVeklorWarlock(Aq40TwinEncounter::TwinEncounterState const& state,
                                          Aq40TwinEncounter::TwinRoleAssignment const& assignment)
{
    return assignment.cohort == Aq40TwinEncounter::TwinRoleCohort::WarlockTank &&
           IsTwinPendingSwapPrepExpectedOwner(state, Aq40TwinEncounter::TwinBoss::Veklor, assignment);
}

bool ShouldUseTwinVeklorProxyBridge(Aq40TwinEncounter::TwinEncounterState const& state,
                                    Aq40TwinEncounter::TwinRoleAssignment const& assignment)
{
    if (assignment.cohort != Aq40TwinEncounter::TwinRoleCohort::MeleeTank)
        return false;

    Aq40TwinEncounter::TwinSide const veklorExpectedSide =
        GetTwinExpectedOwnerSide(state, Aq40TwinEncounter::TwinBoss::Veklor);
    if (assignment.stableSide != veklorExpectedSide)
        return false;

    return Aq40TwinEncounter::IsSwapPrepActive(state) || IsTwinPostTeleportVeklorPickupWindow(state);
}

Aq40TwinEncounter::TwinRoleAssignment const* GetTwinAssignmentForSideAndCohort(
    Aq40TwinEncounter::TwinEncounterState const& state, Aq40TwinEncounter::TwinSide side,
    Aq40TwinEncounter::TwinRoleCohort cohort)
{
    for (Aq40TwinEncounter::TwinRoleAssignment const& assignment : state.assignments)
    {
        if (assignment.stableSide == side && assignment.cohort == cohort)
            return &assignment;
    }

    return nullptr;
}

bool IsTwinHealerDegradedFallback(Aq40TwinEncounter::TwinEncounterState const& state,
                                  Aq40TwinEncounter::TwinRoleAssignment const* assignment)
{
    return assignment && assignment->cohort == Aq40TwinEncounter::TwinRoleCohort::SideHealer &&
           (state.mode == Aq40TwinEncounter::TwinStrategyMode::Degraded ||
            state.phase == Aq40TwinEncounter::TwinEncounterPhase::Degraded);
}

bool IsTwinSideHealerMode(Aq40TwinEncounter::TwinEncounterState const& state,
                          Aq40TwinEncounter::TwinRoleAssignment const* assignment)
{
    return assignment && assignment->cohort == Aq40TwinEncounter::TwinRoleCohort::SideHealer &&
           !IsTwinHealerDegradedFallback(state, assignment);
}

Aq40TwinEncounter::TwinAnchor MakeMidpointAnchor(Aq40TwinEncounter::TwinAnchor const& first,
                                                 Aq40TwinEncounter::TwinAnchor const& second,
                                                 Position const& facingTarget)
{
    Aq40TwinEncounter::TwinAnchor anchor;
    anchor.position.Relocate((first.position.GetPositionX() + second.position.GetPositionX()) * 0.5f,
        (first.position.GetPositionY() + second.position.GetPositionY()) * 0.5f,
        (first.position.GetPositionZ() + second.position.GetPositionZ()) * 0.5f);
    anchor.preferredRange = std::max(first.preferredRange, second.preferredRange);
    anchor.facing = ComputeFacing(anchor.position, facingTarget);
    return anchor;
}

Aq40TwinEncounter::TwinAnchor GetTwinSideHealerLocalTankSupportAnchor(
    Aq40TwinEncounter::TwinEncounterState const& state, Aq40TwinEncounter::TwinSide side)
{
    Aq40TwinEncounter::TwinEncounterGeometry const& geometry = Aq40TwinEncounter::GetGeometry();
    size_t const sideIndex = Aq40TwinEncounter::IsKnownSide(side) ? ToSideIndex(side) : 0u;

    Aq40TwinEncounter::TwinRoleAssignment const* warlockAssignment =
        GetTwinAssignmentForSideAndCohort(state, side, Aq40TwinEncounter::TwinRoleCohort::WarlockTank);
    Aq40TwinEncounter::TwinRoleAssignment const* meleeAssignment =
        GetTwinAssignmentForSideAndCohort(state, side, Aq40TwinEncounter::TwinRoleCohort::MeleeTank);

    if (warlockAssignment && meleeAssignment)
    {
        Aq40TwinEncounter::TwinAnchor const warlockAnchor =
            GetTwinStableAnchorChoice(state, *warlockAssignment).anchor;
        Aq40TwinEncounter::TwinAnchor const meleeAnchor =
            GetTwinStableAnchorChoice(state, *meleeAssignment).anchor;
        return MakeMidpointAnchor(warlockAnchor, meleeAnchor, geometry.bossPark[sideIndex].position);
    }

    if (warlockAssignment)
        return GetTwinStableAnchorChoice(state, *warlockAssignment).anchor;

    if (meleeAssignment)
        return GetTwinStableAnchorChoice(state, *meleeAssignment).anchor;

    return geometry.sidePrep[sideIndex];
}

TwinPrePullAnchorChoice GetTwinSideHealerRecoveryAnchorChoice(
    Aq40TwinEncounter::TwinEncounterState const& state,
    Aq40TwinEncounter::TwinRoleAssignment const& assignment, Player* bot)
{
    Aq40TwinEncounter::TwinEncounterGeometry const& geometry = Aq40TwinEncounter::GetGeometry();
    size_t const sideIndex = Aq40TwinEncounter::IsKnownSide(assignment.stableSide) ? ToSideIndex(assignment.stableSide) : 0u;
    Aq40TwinEncounter::TwinAnchor const& sideAnchor = geometry.sideHealer[sideIndex];
    Aq40TwinEncounter::TwinAnchor const supportAnchor =
        GetTwinSideHealerLocalTankSupportAnchor(state, assignment.stableSide);
    Aq40TwinEncounter::TwinAnchor const steppedAnchor =
        MakeMidpointAnchor(sideAnchor, supportAnchor, geometry.bossPark[sideIndex].position);
    Aq40TwinEncounter::TwinAnchor const reentryAnchor = BuildOffsetAnchor(
        geometry.roomCenter, sideAnchor.position, kTwinHealerCenterReentryDistance, 0.0f,
        geometry.bossPark[sideIndex].position);

    if (!bot)
        return { sideAnchor, "side_healer_anchor" };

    float const distanceToSide =
        bot->GetExactDist2d(sideAnchor.position.GetPositionX(), sideAnchor.position.GetPositionY());
    float const distanceToStepped =
        bot->GetExactDist2d(steppedAnchor.position.GetPositionX(), steppedAnchor.position.GetPositionY());
    float const distanceToReentry =
        bot->GetExactDist2d(reentryAnchor.position.GetPositionX(), reentryAnchor.position.GetPositionY());
    float const distanceToSupport =
        bot->GetExactDist2d(supportAnchor.position.GetPositionX(), supportAnchor.position.GetPositionY());

    if (distanceToSide <= kTwinHealerAnchorNearDistance)
        return { sideAnchor, "side_healer_anchor" };

    if (distanceToStepped <= kTwinHealerStepRecoveryDistance)
        return { steppedAnchor, "stepped_side_anchor" };

    if (distanceToReentry <= distanceToSupport)
        return { reentryAnchor, "center_side_reentry" };

    return { supportAnchor, "local_tank_support" };
}

std::list<ObjectGuid> BuildTwinSideHealerFocusTargets(Aq40TwinEncounter::TwinEncounterState const& state,
                                                      Aq40TwinEncounter::TwinSide side)
{
    std::list<ObjectGuid> focusTargets;
    Aq40TwinEncounter::TwinRoleAssignment const* warlockAssignment =
        GetTwinAssignmentForSideAndCohort(state, side, Aq40TwinEncounter::TwinRoleCohort::WarlockTank);
    Aq40TwinEncounter::TwinRoleAssignment const* meleeAssignment =
        GetTwinAssignmentForSideAndCohort(state, side, Aq40TwinEncounter::TwinRoleCohort::MeleeTank);
    ObjectGuid const warlockGuid = warlockAssignment ? warlockAssignment->memberGuid : ObjectGuid::Empty;
    ObjectGuid const meleeGuid = meleeAssignment ? meleeAssignment->memberGuid : ObjectGuid::Empty;

    auto const pushTarget = [&focusTargets](ObjectGuid guid)
    {
        if (!guid.IsEmpty() && std::find(focusTargets.begin(), focusTargets.end(), guid) == focusTargets.end())
            focusTargets.push_back(guid);
    };

    if (side == GetTwinExpectedOwnerSide(state, Aq40TwinEncounter::TwinBoss::Veklor))
    {
        pushTarget(warlockGuid);
        if (Aq40TwinEncounter::IsSwapPrepActive(state) && !warlockGuid.IsEmpty())
            return focusTargets;

        pushTarget(meleeGuid);
        return focusTargets;
    }

    pushTarget(meleeGuid);
    pushTarget(warlockGuid);
    return focusTargets;
}

bool SyncTwinHealerFocusTargets(Player* bot, PlayerbotAI* botAI,
                                Aq40TwinEncounter::TwinEncounterState const& state,
                                std::list<ObjectGuid> const& desiredTargets, char const* reason)
{
    if (!bot || !botAI || !botAI->GetAiObjectContext())
        return false;

    auto* context = botAI->GetAiObjectContext();
    auto* focusValue = context->GetValue<std::list<ObjectGuid>>("focus heal targets");
    bool changed = false;

    if (focusValue->Get() != desiredTargets)
    {
        focusValue->Set(desiredTargets);
        changed = true;
    }

    bool const shouldEnable = !desiredTargets.empty();
    if (shouldEnable && !botAI->HasStrategy("focus heal targets", BOT_STATE_COMBAT))
    {
        botAI->ChangeStrategy("+focus heal targets", BOT_STATE_COMBAT);
        changed = true;
    }
    else if (!shouldEnable && botAI->HasStrategy("focus heal targets", BOT_STATE_COMBAT))
    {
        botAI->ChangeStrategy("-focus heal targets", BOT_STATE_COMBAT);
        changed = true;
    }

    if (changed)
    {
        Aq40TwinEncounter::MarkTwinLocalCleanupState(bot);
        std::ostringstream fields;
        fields << "boss=twin phase=" << Aq40TwinEncounter::ToString(state.phase)
               << " mode=" << Aq40TwinEncounter::ToString(state.mode)
               << " reason=" << reason
               << " targets=" << desiredTargets.size();
        Aq40Helpers::LogAq40Info(bot, "twin_strategy",
            shouldEnable ? "twin:side_healer:focus_sync" : "twin:side_healer:focus_clear",
            fields.str(), 1000);
    }

    return changed;
}

Player* GetTwinSwapPrepHealerPreloadTarget(PlayerbotAI* botAI,
                                           Aq40TwinEncounter::TwinEncounterState const& state,
                                           Aq40TwinEncounter::TwinRoleAssignment const& assignment)
{
    if (!botAI || !Aq40TwinEncounter::IsSwapPrepActive(state) ||
        assignment.stableSide != GetTwinExpectedOwnerSide(state, Aq40TwinEncounter::TwinBoss::Veklor))
    {
        return nullptr;
    }

    Aq40TwinEncounter::TwinRoleAssignment const* warlockAssignment =
        GetTwinAssignmentForSideAndCohort(state, assignment.stableSide, Aq40TwinEncounter::TwinRoleCohort::WarlockTank);
    if (!warlockAssignment)
        return nullptr;

    Unit* target = botAI->GetUnit(warlockAssignment->memberGuid);
    Player* playerTarget = target ? target->ToPlayer() : nullptr;
    return playerTarget && playerTarget->IsAlive() && playerTarget->IsInWorld() ? playerTarget : nullptr;
}

bool TryTwinHealerPreloadSpell(Player* bot, PlayerbotAI* botAI, Player* target,
                               Aq40TwinEncounter::TwinEncounterState const& state, char const* spellName,
                               bool requireMissingAura, bool requireInjuredTarget)
{
    if (!bot || !botAI || !target)
        return false;

    if (requireMissingAura && botAI->HasAura(spellName, target))
        return false;

    if (requireInjuredTarget && target->IsFullHealth())
        return false;

    if (!botAI->CanCastSpell(spellName, target) || !botAI->CastSpell(spellName, target))
        return false;

    std::ostringstream fields;
    fields << "boss=twin phase=" << Aq40TwinEncounter::ToString(state.phase)
           << " mode=" << Aq40TwinEncounter::ToString(state.mode)
           << " reason=swap_prep_preload"
           << " spell=" << spellName
           << " target=" << Aq40Helpers::GetAq40LogUnit(target);
    Aq40Helpers::LogAq40Info(bot, "twin_strategy", "twin:side_healer:preload_cast", fields.str(), 1000);
    return true;
}

bool TryTwinSwapPrepHealerPreload(Player* bot, PlayerbotAI* botAI,
                                  Aq40TwinEncounter::TwinEncounterState const& state,
                                  Aq40TwinEncounter::TwinRoleAssignment const& assignment)
{
    if (!bot || !botAI || bot->GetCurrentSpell(CURRENT_GENERIC_SPELL) || bot->GetCurrentSpell(CURRENT_CHANNELED_SPELL))
        return false;

    Player* target = GetTwinSwapPrepHealerPreloadTarget(botAI, state, assignment);
    if (!target)
        return false;

    switch (bot->getClass())
    {
        case CLASS_PRIEST:
            return TryTwinHealerPreloadSpell(bot, botAI, target, state, "power word: shield", true, false) ||
                   TryTwinHealerPreloadSpell(bot, botAI, target, state, "renew", true, false);
        case CLASS_DRUID:
            return TryTwinHealerPreloadSpell(bot, botAI, target, state, "rejuvenation", true, false) ||
                   TryTwinHealerPreloadSpell(bot, botAI, target, state, "regrowth", true, false);
        case CLASS_SHAMAN:
            return TryTwinHealerPreloadSpell(bot, botAI, target, state, "earth shield", true, false) ||
                   TryTwinHealerPreloadSpell(bot, botAI, target, state, "riptide", true, false);
        case CLASS_PALADIN:
            return TryTwinHealerPreloadSpell(bot, botAI, target, state, "sacred shield", true, false) ||
                   TryTwinHealerPreloadSpell(bot, botAI, target, state, "holy shock", false, true) ||
                   TryTwinHealerPreloadSpell(bot, botAI, target, state, "flash of light", false, true);
        default:
            break;
    }

    return false;
}

bool FaceTwinAnchorIfNeeded(Player* bot, Aq40TwinEncounter::TwinAnchor const& anchor)
{
    if (!bot)
        return false;

    if (GetFacingDelta(anchor.facing, bot->GetOrientation()) <= kTwinFacingTolerance)
        return false;

    bot->SetFacingTo(anchor.facing);
    return true;
}

bool HasTwinBossAggroLead(Player* bot, PlayerbotAI* botAI,
                          Aq40TwinEncounter::TwinEncounterState const& state,
                          Aq40TwinEncounter::TwinBoss boss, Unit* target)
{
    if (!bot || !botAI)
        return false;

    if (target && (target->GetVictim() == bot || botAI->HasAggro(target)))
        return true;

    ObjectGuid const botGuid = bot->GetGUID();
    Aq40TwinEncounter::TwinStableOwnership const& ownership = Aq40TwinEncounter::GetOwnership(state, boss);
    if (ownership.candidateOwner == botGuid || ownership.stableOwner == botGuid)
        return true;

    return Aq40TwinEncounter::GetPickupOwner(state, boss) == botGuid;
}

bool ReleaseTwinPinnedTarget(Player* bot, PlayerbotAI* botAI, Unit* target,
                             Aq40TwinEncounter::TwinEncounterState const& state,
                             char const* eventKey, char const* reason,
                             Aq40TwinEncounter::TwinRoleAssignment const* assignment = nullptr,
                             TwinTargetIntent intent = TwinTargetIntent::None)
{
    if (!bot || !botAI || !target)
        return false;

    if (GetTwinCurrentTarget(botAI) != target && bot->GetTarget() != target->GetGUID() &&
        bot->GetVictim() != target)
    {
        return false;
    }

    Aq40TwinEncounter::RequestImmediateMovementInterrupt(bot);
    bot->AttackStop();
    if (botAI->GetAiObjectContext())
        botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Set(nullptr);
    bot->SetTarget(ObjectGuid::Empty);
    bot->SetSelection(ObjectGuid());

    std::ostringstream fields;
    fields << "boss=twin phase=" << Aq40TwinEncounter::ToString(state.phase)
           << " mode=" << Aq40TwinEncounter::ToString(state.mode)
           << " reason=" << reason
           << " target=" << Aq40Helpers::GetAq40LogUnit(target)
           << " target_side=" << Aq40TwinEncounter::ToString(
                  GetTwinSideForPosition(target->GetPositionX(), target->GetPositionY()))
           << " phase_elapsed_ms=" << Aq40TwinEncounter::GetPhaseElapsedMs(state)
           << " mode_elapsed_ms=" << GetTwinModeElapsedMs(state)
           << " teleport_elapsed_ms=" << GetTwinTeleportElapsedMs(state)
           << " swap_prep=" << (Aq40TwinEncounter::IsSwapPrepActive(state) ? 1 : 0);
    if (assignment)
    {
        fields << " cohort=" << Aq40TwinEncounter::ToString(assignment->cohort)
               << " side=" << Aq40TwinEncounter::ToString(assignment->stableSide)
               << " slot=" << static_cast<uint32>(assignment->slotIndex)
               << " intent=" << GetTwinTargetReason(intent)
               << " primary_controller=" << (IsTwinPrimaryTankController(state, assignment) ? 1 : 0);
    }
    AppendTwinOpeningOwnershipFields(fields, bot, state);
    Aq40Helpers::LogAq40Info(bot, "twin_strategy", eventKey, fields.str(), 1000);
    return true;
}

bool DoesTwinIntentAllowBossTarget(TwinTargetIntent intent, Unit const* target)
{
    if (!IsTwinEmperorTarget(target))
        return true;

    switch (intent)
    {
        case TwinTargetIntent::Veklor:
            return IsTwinVeklorTarget(target);

        case TwinTargetIntent::Veknilash:
        case TwinTargetIntent::HoldVeknilash:
            return IsTwinVeknilashTarget(target);

        case TwinTargetIntent::HoldReserve:
        case TwinTargetIntent::None:
            return false;
    }

    return false;
}

bool ReleaseTwinPinnedBossTargetForIntent(Player* bot, PlayerbotAI* botAI,
                                          Aq40TwinEncounter::TwinEncounterState const& state,
                                          Aq40TwinEncounter::TwinRoleAssignment const* assignment,
                                          TwinTargetIntent intent, char const* eventKey)
{
    if (!bot || !botAI)
        return false;

    Unit* const currentTarget = GetTwinCurrentTarget(botAI);
    Unit* const selectionTarget = bot->GetTarget().IsEmpty() ? nullptr : botAI->GetUnit(bot->GetTarget());
    std::array<Unit*, 3> const pinnedTargets = { bot->GetVictim(), currentTarget, selectionTarget };
    for (Unit* pinnedTarget : pinnedTargets)
    {
        if (!IsTwinEmperorTarget(pinnedTarget))
            continue;

        bool const allowedByIntent = DoesTwinIntentAllowBossTarget(intent, pinnedTarget);
        bool const allowedByAssignment = DoesTwinAssignmentAllowBossTarget(state, assignment, pinnedTarget);
        if (allowedByIntent && allowedByAssignment)
            continue;

        return ReleaseTwinPinnedTarget(
            bot, botAI, pinnedTarget, state, eventKey,
            allowedByIntent ? "assignment_guard" : GetTwinTargetReason(intent), assignment, intent);
    }

    return false;
}

bool HoldTwinReserveTankAtAnchor(Player* bot, PlayerbotAI* botAI,
                                 Aq40TwinEncounter::TwinEncounterState const& state,
                                 Aq40TwinEncounter::TwinRoleAssignment const& assignment)
{
    TwinPrePullAnchorChoice const anchorChoice = GetTwinPrePullAnchorChoice(state, assignment);
    Aq40TwinEncounter::TwinAnchor const& anchor = anchorChoice.anchor;

    if (bot->GetExactDist2d(anchor.position.GetPositionX(), anchor.position.GetPositionY()) > kTwinAnchorTolerance)
    {
        Aq40TwinEncounter::RequestImmediateMovementInterrupt(bot);
        std::ostringstream fields;
        fields << "boss=twin phase=" << Aq40TwinEncounter::ToString(state.phase)
               << " cohort=" << Aq40TwinEncounter::ToString(assignment.cohort)
               << " side=" << Aq40TwinEncounter::ToString(assignment.stableSide)
               << " slot=" << static_cast<uint32>(assignment.slotIndex);
        AppendTwinAnchorLogFields(fields, bot, anchor, anchorChoice.label);
        Aq40Helpers::LogAq40Info(bot, "twin_position", "twin:reserve_hold", fields.str(), 1000);
        return MoveTwinInside(botAI, bot->GetMapId(), anchor.position.GetPositionX(),
            anchor.position.GetPositionY(), anchor.position.GetPositionZ(), kTwinAnchorTolerance,
            MovementPriority::MOVEMENT_COMBAT);
    }

    return FaceTwinAnchorIfNeeded(bot, anchor);
}

Unit* ResolveTwinTarget(Player* bot, PlayerbotAI* botAI, Aq40TwinEncounter::TwinEncounterState const& state,
                        Aq40TwinEncounter::TwinRoleAssignment const* assignment,
                        TwinTargetIntent intent, GuidVector const& units, char const*& outReason)
{
    if (!bot || !botAI || IsTwinHealerProfile(bot, botAI))
        return nullptr;

    Unit* veklor = FindTwinBoss(botAI, units, Aq40TwinEncounter::TwinBoss::Veklor);
    Unit* veknilash = FindTwinBoss(botAI, units, Aq40TwinEncounter::TwinBoss::Veknilash);
    Unit* currentTarget = botAI->GetAiObjectContext()
        ? botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Get()
        : nullptr;
    Unit* currentVictim = bot->GetVictim();
    outReason = GetTwinTargetReason(intent);

    if (Unit* openingTarget = FindTwinDualPullOpeningBossTarget(botAI, state, assignment, intent, units))
    {
        outReason = openingTarget->GetEntry() == Aq40SpellIds::TwinVeklorNpcEntry ? "veklor" : "veknilash";
        return openingTarget;
    }

    if (IsStrictTwinDualPullOpeningAssignment(state, assignment))
        return nullptr;

    if (intent != TwinTargetIntent::HoldReserve && intent != TwinTargetIntent::None)
    {
        if (Unit* bugTarget = FindTwinBugServiceTarget(
                bot, botAI, state, assignment, units, currentTarget, currentVictim, outReason))
        {
            return bugTarget;
        }
    }

    Unit* resolvedTarget = nullptr;
    switch (intent)
    {
        case TwinTargetIntent::HoldVeknilash:
        case TwinTargetIntent::Veknilash:
            resolvedTarget = veknilash;
            break;

        case TwinTargetIntent::Veklor:
            resolvedTarget = veklor;
            break;

        case TwinTargetIntent::HoldReserve:
        case TwinTargetIntent::None:
            break;
    }

    if (resolvedTarget && !DoesTwinAssignmentAllowBossTarget(state, assignment, resolvedTarget))
    {
        outReason = "assignment_guard";
        return nullptr;
    }

    return resolvedTarget;
}

Aq40TwinEncounter::TwinAnchor const& GetCenterSpreadAnchor(uint8 slotIndex)
{
    Aq40TwinEncounter::TwinEncounterGeometry const& geometry = Aq40TwinEncounter::GetGeometry();
    return geometry.centerSpread[slotIndex % geometry.centerSpread.size()].anchor;
}

Aq40TwinEncounter::TwinAnchor const& GetCenterSpreadAnchor(Player* bot)
{
    Aq40TwinEncounter::TwinEncounterGeometry const& geometry = Aq40TwinEncounter::GetGeometry();
    size_t const slotIndex = bot ? (bot->GetGUID().GetRawValue() % geometry.centerSpread.size()) : 0u;
    return GetCenterSpreadAnchor(static_cast<uint8>(slotIndex));
}

Aq40TwinEncounter::TwinAnchor const& GetVeknilashSideAnchor(PlayerbotAI* botAI, GuidVector const& units)
{
    Aq40TwinEncounter::TwinEncounterGeometry const& geometry = Aq40TwinEncounter::GetGeometry();
    Unit* veknilash = FindTwinBoss(botAI, units, Aq40TwinEncounter::TwinBoss::Veknilash);
    Aq40TwinEncounter::TwinSide const side =
        veknilash ? GetTwinSideForPosition(veknilash->GetPositionX(), veknilash->GetPositionY())
                  : Aq40TwinEncounter::GetInitialSideForBoss(Aq40TwinEncounter::TwinBoss::Veknilash);
    return geometry.bossPark[ToSideIndex(side)];
}

TwinPrePullAnchorChoice GetTwinHazardRecoveryAnchorChoice(
    Player* bot, PlayerbotAI* botAI, Aq40TwinEncounter::TwinEncounterState const& state,
    Aq40TwinEncounter::TwinRoleAssignment const* assignment, GuidVector const& units)
{
    if (IsTwinSideHealerMode(state, assignment))
        return GetTwinSideHealerRecoveryAnchorChoice(state, *assignment, bot);

    if (assignment)
        return GetTwinStableAnchorChoice(state, *assignment);

    if (IsTwinMeleeProfile(bot, botAI))
        return { GetVeknilashSideAnchor(botAI, units), "veknilash_side" };

    return { GetCenterSpreadAnchor(bot), "center_spread" };
}

Aq40TwinEncounter::TwinAnchor GetTwinMeleeDpsStageAnchor(Aq40TwinEncounter::TwinSide side, uint8 slotIndex)
{
    static std::array<TwinAnchorOffsetPattern, 4> const kPatterns = {
        TwinAnchorOffsetPattern{ 3.5f, -2.5f },
        TwinAnchorOffsetPattern{ 4.5f, 2.5f },
        TwinAnchorOffsetPattern{ 6.0f, -4.5f },
        TwinAnchorOffsetPattern{ 7.5f, 4.5f },
    };

    Aq40TwinEncounter::TwinEncounterGeometry const& geometry = Aq40TwinEncounter::GetGeometry();
    TwinAnchorOffsetPattern const& pattern = kPatterns[slotIndex % kPatterns.size()];
    Aq40TwinEncounter::TwinAnchor const& bossPark = geometry.bossPark[ToSideIndex(side)];
    return BuildOffsetAnchor(bossPark, geometry.roomCenter.position, pattern.forward, pattern.lateral,
        bossPark.position);
}

TwinPrePullAnchorChoice GetTwinMeleeDpsCombatAnchorChoice(
    Aq40TwinEncounter::TwinEncounterState const& state,
    Aq40TwinEncounter::TwinRoleAssignment const& assignment)
{
    Aq40TwinEncounter::TwinSide const meleeSide = GetTwinMeleeDpsExpectedSide(state, assignment);
    uint8 const slotIndex = GetTwinCrossSideSlotIndex(assignment);
    char const* anchorLabel =
        Aq40TwinEncounter::IsSwapPrepActive(state) && Aq40TwinEncounter::IsKnownSide(meleeSide) &&
                meleeSide != assignment.stableSide
            ? "veknilash_melee_cross"
            : "veknilash_melee_pack";
    return { GetTwinMeleeDpsStageAnchor(meleeSide, slotIndex), anchorLabel };
}

Aq40TwinEncounter::TwinAnchor GetTwinHunterStageAnchor(Aq40TwinEncounter::TwinSide side, uint8 slotIndex)
{
    static std::array<TwinAnchorOffsetPattern, 4> const kPatterns = {
        TwinAnchorOffsetPattern{ 12.0f, -4.0f },
        TwinAnchorOffsetPattern{ 14.0f, 4.0f },
        TwinAnchorOffsetPattern{ 16.0f, -7.0f },
        TwinAnchorOffsetPattern{ 18.0f, 7.0f },
    };

    Aq40TwinEncounter::TwinEncounterGeometry const& geometry = Aq40TwinEncounter::GetGeometry();
    TwinAnchorOffsetPattern const& pattern = kPatterns[slotIndex % kPatterns.size()];
    Aq40TwinEncounter::TwinAnchor const& bossPark = geometry.bossPark[ToSideIndex(side)];
    return BuildOffsetAnchor(bossPark, geometry.roomCenter.position, pattern.forward, pattern.lateral,
        bossPark.position);
}

Aq40TwinEncounter::TwinAnchor GetTwinRangedStageAnchor(Aq40TwinEncounter::TwinSide side, uint8 slotIndex)
{
    static std::array<TwinAnchorOffsetPattern, 5> const kPatterns = {
        TwinAnchorOffsetPattern{ -2.0f, -4.0f },
        TwinAnchorOffsetPattern{ -1.0f, 4.0f },
        TwinAnchorOffsetPattern{ 2.0f, -8.0f },
        TwinAnchorOffsetPattern{ 3.0f, 8.0f },
        TwinAnchorOffsetPattern{ 5.0f, 0.0f },
    };

    Aq40TwinEncounter::TwinEncounterGeometry const& geometry = Aq40TwinEncounter::GetGeometry();
    TwinAnchorOffsetPattern const& pattern = kPatterns[slotIndex % kPatterns.size()];
    Aq40TwinEncounter::TwinAnchor const& base = geometry.stableVeklorWarlock[ToSideIndex(side)];
    Aq40TwinEncounter::TwinAnchor const& bossPark = geometry.bossPark[ToSideIndex(side)];
    return BuildOffsetAnchor(base, geometry.roomCenter.position, pattern.forward, pattern.lateral,
        bossPark.position, kTwinWarlockPreferredRange);
}

TwinPrePullAnchorChoice GetTwinPrePullAnchorChoice(Aq40TwinEncounter::TwinEncounterState const& state,
                                                   Aq40TwinEncounter::TwinRoleAssignment const& assignment)
{
    Aq40TwinEncounter::TwinEncounterGeometry const& geometry = Aq40TwinEncounter::GetGeometry();
    size_t const sideIndex = Aq40TwinEncounter::IsKnownSide(assignment.stableSide) ? ToSideIndex(assignment.stableSide) : 0u;
    bool const seededDualPullOwnership = HasTwinSeededDualPullOwnership(state);

    switch (assignment.cohort)
    {
        case Aq40TwinEncounter::TwinRoleCohort::WarlockTank:
            if (assignment.stableSide == Aq40TwinEncounter::GetInitialSideForBoss(Aq40TwinEncounter::TwinBoss::Veklor))
                return { geometry.sidePrep[sideIndex], "side_prep" };
            return seededDualPullOwnership ? TwinPrePullAnchorChoice{ geometry.reserveWarlockPrep[sideIndex],
                                                                      "reserve_warlock_prep" }
                                           : TwinPrePullAnchorChoice{ geometry.sidePrep[sideIndex], "side_prep" };

        case Aq40TwinEncounter::TwinRoleCohort::MeleeTank:
            if (assignment.stableSide ==
                Aq40TwinEncounter::GetInitialSideForBoss(Aq40TwinEncounter::TwinBoss::Veknilash))
            {
                return { geometry.sidePrep[sideIndex], "side_prep" };
            }
            return seededDualPullOwnership ? TwinPrePullAnchorChoice{ geometry.reserveMeleeProxy[sideIndex],
                                                                      "reserve_melee_proxy" }
                                           : TwinPrePullAnchorChoice{ geometry.sidePrep[sideIndex], "side_prep" };

        case Aq40TwinEncounter::TwinRoleCohort::SideHealer:
            return { geometry.sideHealer[sideIndex], "side_healer_anchor" };

        case Aq40TwinEncounter::TwinRoleCohort::RaidHealer:
            return { GetCenterSpreadAnchor(assignment.slotIndex), "center_spread" };

        case Aq40TwinEncounter::TwinRoleCohort::RangedDps:
            return { GetTwinRangedStageAnchor(assignment.stableSide, assignment.slotIndex),
                "stable_veklor_warlock" };

        case Aq40TwinEncounter::TwinRoleCohort::Hunter:
            return { GetTwinHunterStageAnchor(assignment.stableSide, assignment.slotIndex),
                "veknilash_hunter_bias" };

        case Aq40TwinEncounter::TwinRoleCohort::MeleeDps:
            return { GetTwinMeleeDpsStageAnchor(assignment.stableSide, assignment.slotIndex),
                "veknilash_melee_pack" };

        case Aq40TwinEncounter::TwinRoleCohort::None:
            break;
    }

    return { geometry.roomCenter, "room_center" };
}

TwinPrePullAnchorChoice GetTwinStableAnchorChoice(Aq40TwinEncounter::TwinEncounterState const& state,
                                                  Aq40TwinEncounter::TwinRoleAssignment const& assignment)
{
    Aq40TwinEncounter::TwinEncounterGeometry const& geometry = Aq40TwinEncounter::GetGeometry();
    size_t const sideIndex = Aq40TwinEncounter::IsKnownSide(assignment.stableSide) ? ToSideIndex(assignment.stableSide) : 0u;

    switch (assignment.cohort)
    {
        case Aq40TwinEncounter::TwinRoleCohort::WarlockTank:
            if (ShouldStageTwinIncomingVeklorWarlock(state, assignment))
                return { geometry.stableVeklorWarlock[sideIndex], "stable_veklor_warlock" };
            if (ShouldHoldTwinReserveTankAssignmentNow(state, assignment))
                return { geometry.reserveWarlockPrep[sideIndex], "reserve_warlock_prep" };
            return { geometry.stableVeklorWarlock[sideIndex], "stable_veklor_warlock" };

        case Aq40TwinEncounter::TwinRoleCohort::MeleeTank:
            if (ShouldUseTwinVeklorProxyBridge(state, assignment))
                return { geometry.reserveMeleeProxy[sideIndex], "reserve_melee_proxy" };

            if (ShouldHoldTwinReserveTankAssignmentNow(state, assignment))
                return { geometry.reserveMeleeProxy[sideIndex], "reserve_melee_proxy" };
            return { geometry.bossPark[sideIndex], "boss_park" };

        case Aq40TwinEncounter::TwinRoleCohort::SideHealer:
            return { geometry.sideHealer[sideIndex], "side_healer_anchor" };

        case Aq40TwinEncounter::TwinRoleCohort::RaidHealer:
            return { GetCenterSpreadAnchor(assignment.slotIndex), "center_spread" };

        case Aq40TwinEncounter::TwinRoleCohort::RangedDps:
            return { GetTwinRangedStageAnchor(assignment.stableSide, assignment.slotIndex),
                "stable_veklor_warlock" };

        case Aq40TwinEncounter::TwinRoleCohort::Hunter:
            return { GetTwinHunterStageAnchor(assignment.stableSide, assignment.slotIndex),
                "veknilash_hunter_bias" };

        case Aq40TwinEncounter::TwinRoleCohort::MeleeDps:
            return GetTwinMeleeDpsCombatAnchorChoice(state, assignment);

        case Aq40TwinEncounter::TwinRoleCohort::None:
            break;
    }

    return { geometry.roomCenter, "room_center" };
}

bool SyncTwinPendingVeklorPickupAnchor(Player* bot,
                                       Aq40TwinEncounter::TwinEncounterState const& state,
                                       Aq40TwinEncounter::TwinRoleAssignment const* assignment)
{
    if (!bot || !assignment)
        return false;

    bool const postTeleportPickupWindow = IsTwinPostTeleportVeklorPickupWindow(state);
    bool const shouldLockWarlock =
        postTeleportPickupWindow &&
        assignment->cohort == Aq40TwinEncounter::TwinRoleCohort::WarlockTank &&
        Aq40TwinEncounter::GetOwnership(state, Aq40TwinEncounter::TwinBoss::Veklor).expectedOwner ==
            assignment->memberGuid;
    bool const shouldLockBridge =
        postTeleportPickupWindow && ShouldUseTwinVeklorProxyBridge(state, *assignment) &&
        !IsTwinMeleeTankController(state, *assignment);
    bool const shouldLock = shouldLockWarlock || shouldLockBridge;

    if (shouldLock)
    {
        Aq40TwinEncounter::TwinEncounterGeometry const& geometry = Aq40TwinEncounter::GetGeometry();
        size_t const sideIndex =
            Aq40TwinEncounter::IsKnownSide(assignment->stableSide) ? ToSideIndex(assignment->stableSide) : 0u;
        Aq40TwinEncounter::TwinAnchor const& anchor =
            shouldLockWarlock ? geometry.stableVeklorWarlock[sideIndex] : geometry.reserveMeleeProxy[sideIndex];
        char const* anchorLabel = shouldLockWarlock ? "stable_veklor_warlock" : "reserve_melee_proxy";
        uint32 const nowMs = getMSTime();
        uint32 const durationMs =
            std::max(Aq40TwinEncounter::GetThreatHoldRemainingMs(
                         state, Aq40TwinEncounter::TwinBoss::Veklor, nowMs),
                1000u);
        bool const changed = Aq40TwinEncounter::SetLockedPickupAnchor(
            bot, Aq40TwinEncounter::TwinBoss::Veklor, assignment->stableSide, anchor, durationMs, nowMs);
        if (changed)
        {
            std::ostringstream fields;
            fields << "boss=twin phase=" << Aq40TwinEncounter::ToString(state.phase)
                   << " mode=" << Aq40TwinEncounter::ToString(state.mode)
                   << " phase_elapsed_ms=" << Aq40TwinEncounter::GetPhaseElapsedMs(state, nowMs)
                   << " mode_elapsed_ms=" << GetTwinModeElapsedMs(state, nowMs)
                   << " teleport_elapsed_ms=" << GetTwinTeleportElapsedMs(state, nowMs)
                   << " cohort=" << Aq40TwinEncounter::ToString(assignment->cohort)
                   << " side=" << Aq40TwinEncounter::ToString(assignment->stableSide)
                   << " reason=" << (shouldLockWarlock ? "post_teleport_veklor_controller"
                                                       : "post_teleport_veklor_bridge")
                   << " threat_hold_remaining_ms="
                   << Aq40TwinEncounter::GetThreatHoldRemainingMs(
                          state, Aq40TwinEncounter::TwinBoss::Veklor, nowMs);
            AppendTwinAnchorLogFields(fields, bot, anchor, anchorLabel);
            AppendTwinOpeningOwnershipFields(fields, bot, state, nowMs);
            Aq40Helpers::LogAq40Info(bot, "twin_position",
                "twin:post_swap:lock_pickup_anchor", fields.str(), 1000);
        }

        if (GetTwinTeleportElapsedMs(state, nowMs) > 1000 &&
            !Aq40TwinEncounter::IsPickupEstablished(state, Aq40TwinEncounter::TwinBoss::Veklor))
        {
            LogTwinPostTeleportPickupPending(bot, state, *assignment,
                shouldLockWarlock ? "post_teleport_veklor_controller" : "post_teleport_veklor_bridge");
        }

        return changed;
    }

    Aq40TwinEncounter::TwinLockedPickupAnchor const* lockedAnchor = Aq40TwinEncounter::GetLockedPickupAnchor(bot);
    if (!lockedAnchor || lockedAnchor->boss != Aq40TwinEncounter::TwinBoss::Veklor ||
        Aq40TwinEncounter::GetPickupOwner(state, Aq40TwinEncounter::TwinBoss::Veklor) == bot->GetGUID())
    {
        return false;
    }

    std::string const reason =
        Aq40TwinEncounter::IsPickupEstablished(state, Aq40TwinEncounter::TwinBoss::Veklor)
            ? "pickup_established"
            : "pickup_window_closed";
    Aq40TwinEncounter::ClearLockedPickupAnchor(bot);
    std::ostringstream fields;
    fields << "boss=twin phase=" << Aq40TwinEncounter::ToString(state.phase)
           << " mode=" << Aq40TwinEncounter::ToString(state.mode)
           << " reason=" << reason
           << " phase_elapsed_ms=" << Aq40TwinEncounter::GetPhaseElapsedMs(state)
           << " mode_elapsed_ms=" << GetTwinModeElapsedMs(state)
           << " teleport_elapsed_ms=" << GetTwinTeleportElapsedMs(state);
    AppendTwinOpeningOwnershipFields(fields, bot, state);
    Aq40Helpers::LogAq40Info(bot, "twin_position", "twin:post_swap:release_pickup_anchor",
        fields.str(), 1000);
    return true;
}

Aq40TwinEncounter::TwinAnchor GetTwinSplitHoldAnchor(Player* bot, PlayerbotAI* botAI,
                                                     Aq40TwinEncounter::TwinEncounterState const& state,
                                                     GuidVector const& units)
{
    if (bot)
    {
        if (Aq40TwinEncounter::TwinRoleAssignment const* assignment =
                Aq40TwinEncounter::GetAssignmentForMember(state, bot->GetGUID()))
        {
            switch (assignment->cohort)
            {
                case Aq40TwinEncounter::TwinRoleCohort::MeleeTank:
                case Aq40TwinEncounter::TwinRoleCohort::Hunter:
                case Aq40TwinEncounter::TwinRoleCohort::MeleeDps:
                    return GetTwinStableAnchorChoice(state, *assignment).anchor;

                default:
                    break;
            }
        }
    }

    return GetVeknilashSideAnchor(botAI, units);
}

Aq40TwinEncounter::TwinSplitBand GetTwinSplitRiskBand(Unit const* veklor, Unit const* veknilash,
                                                      Aq40TwinEncounter::TwinAnchor const& anchor)
{
    if (!veknilash)
        return Aq40TwinEncounter::TwinSplitBand::Stable;

    float const emperorDistance = veklor ? veklor->GetDistance2d(veknilash) : std::numeric_limits<float>::max();
    float const bossParkError =
        veknilash->GetExactDist2d(anchor.position.GetPositionX(), anchor.position.GetPositionY());

    if (emperorDistance <= kTwinSplitUrgentDistance || bossParkError >= kTwinBossParkUrgentError)
        return Aq40TwinEncounter::TwinSplitBand::Urgent;

    if (emperorDistance <= kTwinSplitWarningDistance || bossParkError >= kTwinBossParkWarningError)
        return Aq40TwinEncounter::TwinSplitBand::Warning;

    return Aq40TwinEncounter::TwinSplitBand::Stable;
}

void UpdateTwinSplitBandForMeleeTank(Player* bot, Aq40TwinEncounter::TwinEncounterState& state,
                                     Unit* veklor, Unit* veknilash,
                                     Aq40TwinEncounter::TwinRoleAssignment const& assignment)
{
    if (!bot || assignment.cohort != Aq40TwinEncounter::TwinRoleCohort::MeleeTank ||
        !IsTwinMeleeTankController(state, assignment))
    {
        return;
    }

    if (state.phase != Aq40TwinEncounter::TwinEncounterPhase::DualPullWindow &&
        state.phase != Aq40TwinEncounter::TwinEncounterPhase::Stable)
    {
        return;
    }

    Aq40TwinEncounter::TwinAnchor const anchor = GetTwinStableAnchorChoice(state, assignment).anchor;
    Aq40TwinEncounter::TwinSplitBand const band = GetTwinSplitRiskBand(veklor, veknilash, anchor);
    if (!Aq40TwinEncounter::SetSplitBand(state, band))
        return;

    float const emperorDistance = veklor && veknilash ? veklor->GetDistance2d(veknilash) : 0.0f;
    float const bossParkError =
        veknilash ? veknilash->GetExactDist2d(anchor.position.GetPositionX(), anchor.position.GetPositionY()) : 0.0f;

    std::ostringstream fields;
    fields << "boss=twin phase=" << Aq40TwinEncounter::ToString(state.phase)
           << " side=" << Aq40TwinEncounter::ToString(assignment.stableSide)
           << " controller=" << Aq40Helpers::GetAq40LogUnit(bot)
           << " emperor_distance=" << emperorDistance
           << " veknilash_anchor_error=" << bossParkError
           << " split_band=" << Aq40TwinEncounter::ToString(band);

    if (band == Aq40TwinEncounter::TwinSplitBand::Stable)
    {
        Aq40Helpers::LogAq40Info(bot, "twin_split_band", "twin:split_band:stable", fields.str(), 1000);
        return;
    }

    Aq40Helpers::LogAq40Warn(bot, "twin_split_band",
        std::string("twin:split_band:") + Aq40TwinEncounter::ToString(band), fields.str(), 1000);
}

bool MaintainTwinAssignedAnchor(Player* bot, PlayerbotAI* botAI,
                                Aq40TwinEncounter::TwinEncounterState const& state,
                                Aq40TwinEncounter::TwinRoleAssignment const& assignment,
                                Aq40TwinEncounter::TwinAnchor const& anchor,
                                char const* eventKey, char const* anchorLabel)
{
    if (!bot)
        return false;

    if (bot->GetExactDist2d(anchor.position.GetPositionX(), anchor.position.GetPositionY()) >
        kTwinAnchorTolerance)
    {
        Aq40TwinEncounter::RequestImmediateMovementInterrupt(bot);
        std::ostringstream fields;
        fields << "boss=twin phase=" << Aq40TwinEncounter::ToString(state.phase)
               << " cohort=" << Aq40TwinEncounter::ToString(assignment.cohort)
               << " side=" << Aq40TwinEncounter::ToString(assignment.stableSide)
               << " slot=" << static_cast<uint32>(assignment.slotIndex);
        AppendTwinAnchorLogFields(fields, bot, anchor, anchorLabel);
        Aq40Helpers::LogAq40Info(bot, "twin_position", eventKey, fields.str(), 1000);
        return MoveTwinInside(botAI, bot->GetMapId(), anchor.position.GetPositionX(),
            anchor.position.GetPositionY(), anchor.position.GetPositionZ(), kTwinAnchorTolerance,
            MovementPriority::MOVEMENT_COMBAT);
    }

    return FaceTwinAnchorIfNeeded(bot, anchor);
}

bool MoveTwinToHazardAnchor(Player* bot, PlayerbotAI* botAI,
                            Aq40TwinEncounter::TwinEncounterState const& state,
                            Aq40TwinEncounter::TwinRoleAssignment const* assignment,
                            TwinPrePullAnchorChoice const& anchorChoice,
                            char const* eventKey, char const* reason)
{
    if (!bot)
        return false;

    if (assignment)
    {
        return MaintainTwinAssignedAnchor(
            bot, botAI, state, *assignment, anchorChoice.anchor, eventKey, anchorChoice.label);
    }

    if (bot->GetExactDist2d(anchorChoice.anchor.position.GetPositionX(),
            anchorChoice.anchor.position.GetPositionY()) <= kTwinAnchorTolerance)
    {
        return false;
    }

    Aq40TwinEncounter::RequestImmediateMovementInterrupt(bot);

    std::ostringstream fields;
    fields << "boss=twin phase=" << Aq40TwinEncounter::ToString(state.phase)
           << " reason=" << reason;
    AppendTwinAnchorLogFields(fields, bot, anchorChoice.anchor, anchorChoice.label);
    Aq40Helpers::LogAq40Info(bot, "avoid_hazard", eventKey, fields.str(), 1000);

    return MoveTwinInside(botAI, bot->GetMapId(), anchorChoice.anchor.position.GetPositionX(),
        anchorChoice.anchor.position.GetPositionY(), anchorChoice.anchor.position.GetPositionZ(),
        kTwinAnchorTolerance, MovementPriority::MOVEMENT_COMBAT);
}

Aura* GetTwinBlizzardAura(Player* bot, PlayerbotAI* botAI)
{
    if (!bot || !botAI)
        return nullptr;

    if (Aura* aura = Aq40SpellIds::GetAnyAura(bot, { Aq40SpellIds::TwinBlizzard }))
        return aura;

    return botAI->GetAura("blizzard", bot);
}

bool SyncTwinWarlockTankOverlay(Player* bot, PlayerbotAI* botAI)
{
    if (!bot || !botAI || bot->getClass() != CLASS_WARLOCK)
        return false;

    bool const shouldUseTankOverlay = Aq40TwinEncounter::ShouldUseTwinWarlockTankStrategy(bot);
    if (!Aq40TwinEncounter::SyncTwinWarlockTankStrategy(bot))
        return false;

    if (shouldUseTankOverlay)
        Aq40TwinEncounter::MarkTwinLocalCleanupState(bot);

    Aq40TwinEncounter::TwinEncounterState const* state = Aq40TwinEncounter::GetEncounterState(bot);
    std::ostringstream fields;
    fields << "boss=twin strategy=tank action=" << (shouldUseTankOverlay ? "enable" : "disable");
    if (state)
    {
        fields << " phase=" << Aq40TwinEncounter::ToString(state->phase)
               << " mode=" << Aq40TwinEncounter::ToString(state->mode);
    }

    Aq40Helpers::LogAq40Info(bot, "twin_strategy",
        shouldUseTankOverlay ? "twin:warlock_tank_overlay:enable" : "twin:warlock_tank_overlay:disable",
        fields.str(), 1000);
    return true;
}

size_t CountTwinAssignedMembersAtPrePullAnchors(Player* bot,
                                                Aq40TwinEncounter::TwinEncounterState const& state,
                                                float tolerance)
{
    if (!bot || state.phase != Aq40TwinEncounter::TwinEncounterPhase::PrePull)
        return 0u;

    Group* group = bot->GetGroup();
    if (!group)
        return 0u;

    size_t readyCount = 0u;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || !member->IsInWorld() || !Aq40BossHelper::IsSameInstance(bot, member))
            continue;

        Aq40TwinEncounter::TwinRoleAssignment const* assignment =
            Aq40TwinEncounter::GetAssignmentForMember(state, member->GetGUID());
        if (!assignment)
            continue;

        TwinPrePullAnchorChoice const anchorChoice = GetTwinPrePullAnchorChoice(state, *assignment);
        if (member->GetExactDist2d(anchorChoice.anchor.position.GetPositionX(),
                anchorChoice.anchor.position.GetPositionY()) <= tolerance)
        {
            ++readyCount;
        }
    }

    return readyCount;
}

void LogTwinStrictReady(Player* bot, Aq40TwinEncounter::TwinEncounterState const& state)
{
    if (!bot)
        return;

    std::ostringstream fields;
    fields << "boss=twin state=ready"
           << " mode=" << Aq40TwinEncounter::ToString(state.mode)
           << " approach=" << state.approachMemberCount
           << " staged=" << state.stagedMemberCount
           << " center_committed=" << state.centerCommittedMemberCount
           << " strict_ready=" << state.strictReadyMemberCount
           << " assigned=" << state.assignments.size();
    AppendTwinOpeningOwnershipFields(fields, bot, state);
    Aq40Helpers::LogAq40Info(bot, "twin_assignments", "twin:ready", fields.str(), 1000);
}

void LogTwinPrePullStageWait(Player* bot,
                             Aq40TwinEncounter::TwinEncounterState const& state,
                             Aq40TwinEncounter::TwinRoleAssignment const& assignment,
                             TwinPrePullAnchorChoice const& anchorChoice,
                             char const* waitReason,
                             Unit* target = nullptr)
{
    if (!bot)
        return;

    std::ostringstream fields;
    fields << "boss=twin phase=prepull mode=" << Aq40TwinEncounter::ToString(state.mode)
            << " mode_elapsed_ms=" << GetTwinModeElapsedMs(state)
           << " wait=" << waitReason
           << " cohort=" << Aq40TwinEncounter::ToString(assignment.cohort)
           << " side=" << Aq40TwinEncounter::ToString(assignment.stableSide)
           << " slot=" << static_cast<uint32>(assignment.slotIndex);
    AppendTwinAnchorLogFields(fields, bot, anchorChoice.anchor, anchorChoice.label);
    fields
           << " approach=" << state.approachMemberCount
           << " staged=" << state.stagedMemberCount
           << " center_committed=" << state.centerCommittedMemberCount
           << " strict_ready=" << state.strictReadyMemberCount
           << " assigned=" << state.assignments.size()
           << " unsupported_reason=" << (state.unsupportedReason.empty() ? "none" : state.unsupportedReason);
    AppendTwinOpeningOwnershipFields(fields, bot, state);
    if (target)
        fields << " target=" << Aq40Helpers::GetAq40LogUnit(target);

    Aq40Helpers::LogAq40Info(bot, "twin_prepull",
        "twin:prepull:wait:" + std::string(waitReason) + ":" + anchorChoice.label,
        fields.str(), 1000);
}

void LogTwinApproachStageWait(Player* bot,
                              Aq40TwinEncounter::TwinEncounterState const& state,
                              Aq40TwinEncounter::TwinRoleAssignment const& assignment,
                              TwinPrePullAnchorChoice const& anchorChoice,
                              char const* waitReason)
{
    if (!bot)
        return;

    std::ostringstream fields;
    fields << "boss=twin phase=prepull mode=" << Aq40TwinEncounter::ToString(state.mode)
           << " stage=approach"
            << " mode_elapsed_ms=" << GetTwinModeElapsedMs(state)
           << " wait=" << waitReason
           << " cohort=" << Aq40TwinEncounter::ToString(assignment.cohort)
           << " side=" << Aq40TwinEncounter::ToString(assignment.stableSide)
           << " slot=" << static_cast<uint32>(assignment.slotIndex);
    AppendTwinAnchorLogFields(fields, bot, anchorChoice.anchor, anchorChoice.label);
    fields
           << " authority=cleanup_only"
           << " movement=generic_follow"
           << " approach=" << state.approachMemberCount
           << " staged=" << state.stagedMemberCount
           << " center_committed=" << state.centerCommittedMemberCount
           << " strict_ready=" << state.strictReadyMemberCount
           << " assigned=" << state.assignments.size()
           << " unsupported_reason=" << (state.unsupportedReason.empty() ? "none" : state.unsupportedReason);
    AppendTwinOpeningOwnershipFields(fields, bot, state);

    Aq40Helpers::LogAq40Info(bot, "twin_prepull",
        "twin:approach:wait:" + std::string(waitReason) + ":" + anchorChoice.label,
        fields.str(), 1000);
}
}    // namespace

bool Aq40TwinApproachStageAction::isUseful()
{
    Aq40TwinEncounter::TwinEncounterState const* state = Aq40TwinEncounter::GetEncounterState(bot);
    if (!state || !Aq40TwinEncounter::IsTwinApproachWindow(*state, bot))
        return false;

    return NeedsTwinApproachCleanup(bot, botAI);
}

bool Aq40TwinApproachStageAction::Execute(Event /*event*/)
{
    Aq40TwinEncounter::TwinEncounterState const* state = Aq40TwinEncounter::GetEncounterState(bot);
    bool const overlayChanged = SyncTwinWarlockTankOverlay(bot, botAI);
    if (!state || !Aq40TwinEncounter::IsTwinApproachWindow(*state, bot))
        return overlayChanged;

    Aq40TwinEncounter::TwinRoleAssignment const* assignment =
        Aq40TwinEncounter::GetAssignmentForMember(*state, bot->GetGUID());
    if (!assignment)
        return overlayChanged;

    Unit* pendingTankAttackTarget = IsTwinTankAssignment(*assignment) ? GetTwinPendingAttackIntentTarget(bot, botAI)
                                                                      : nullptr;
    TwinTargetIntent const intent = GetTwinTargetIntent(bot, botAI, *state, assignment);
    bool const releasedPinnedBoss = ReleaseTwinPinnedBossTargetForIntent(
        bot, botAI, *state, assignment, intent, "twin:approach:release_wrong_target");
    Aq40TwinEncounter::MarkTwinLocalCleanupState(bot);
    bool const clearedIntent = Aq40TwinEncounter::ClearTwinLocalCombatState(bot, botAI, false);
    bool const petChanged = SyncTwinEncounterPetPolicy(bot, botAI, *state, assignment, nullptr, GuidVector());
    TwinPrePullAnchorChoice const anchorChoice = GetTwinPrePullAnchorChoice(*state, *assignment);

    if (pendingTankAttackTarget)
    {
        LogTwinTankAttackIntentDecision(bot, *state, *assignment, anchorChoice, pendingTankAttackTarget, nullptr,
            "approach", "hold_center_commit");
    }

    LogTwinApproachStageWait(bot, *state, *assignment, anchorChoice, "cleanup_only");
    return overlayChanged || releasedPinnedBoss || clearedIntent || petChanged;
}

bool Aq40TwinPrePullStageAction::Execute(Event /*event*/)
{
    Aq40TwinEncounter::TwinEncounterState* state = Aq40TwinEncounter::GetEncounterState(bot);
    bool const overlayChanged = SyncTwinWarlockTankOverlay(bot, botAI);
    if (!state || !Aq40TwinEncounter::IsTwinPrePullStageWindow(*state, bot) ||
        state->assignments.empty())
    {
        return overlayChanged;
    }

    Aq40TwinEncounter::TwinRoleAssignment const* assignment = Aq40TwinEncounter::GetAssignmentForMember(*state, bot->GetGUID());
    if (!assignment)
        return overlayChanged;

    Unit* pendingTankAttackTarget = IsTwinTankAssignment(*assignment) ? GetTwinPendingAttackIntentTarget(bot, botAI)
                                                                      : nullptr;
    TwinTargetIntent const intent = GetTwinTargetIntent(bot, botAI, *state, assignment);
    bool const releasedPinnedBoss = ReleaseTwinPinnedBossTargetForIntent(
        bot, botAI, *state, assignment, intent, "twin:prepull:release_wrong_target");
    Aq40TwinEncounter::MarkTwinLocalCleanupState(bot);
    bool const clearedIntent = Aq40TwinEncounter::ClearTwinLocalCombatState(bot, botAI, false);
    bool const petChanged = SyncTwinEncounterPetPolicy(bot, botAI, *state, assignment, nullptr, GuidVector());
    TwinPrePullAnchorChoice const anchorChoice = GetTwinPrePullAnchorChoice(*state, *assignment);
    Aq40TwinEncounter::TwinAnchor const& anchor = anchorChoice.anchor;
    size_t const strictReadyCount = CountTwinAssignedMembersAtPrePullAnchors(
        bot, *state, kTwinStrictReadyAnchorTolerance);
    state->strictReadyMemberCount = static_cast<uint16>(
        std::min(strictReadyCount, state->assignments.size()));
    bool const strictReady = state->strictReadyMemberCount >= state->assignments.size();
    bool const readyLost = state->mode == Aq40TwinEncounter::TwinStrategyMode::StandardCompReady && !strictReady;

    if (readyLost)
    {
        Aq40TwinEncounter::SetMode(*state, Aq40TwinEncounter::TwinStrategyMode::CenterCommitted, getMSTime());
    }
    bool const readyLostOverlayChanged = readyLost && SyncTwinWarlockTankOverlay(bot, botAI);

    LogTwinCenterCommitHandoff(bot, *state, *assignment, anchorChoice);

    float const distance = bot->GetExactDist2d(anchor.position.GetPositionX(), anchor.position.GetPositionY());
    if (distance > kTwinAnchorTolerance)
    {
        Aq40TwinEncounter::RequestImmediateMovementInterrupt(bot);
        std::ostringstream fields;
        fields << "boss=twin phase=prepull cohort=" << Aq40TwinEncounter::ToString(assignment->cohort)
               << " mode=" << Aq40TwinEncounter::ToString(state->mode)
               << " mode_elapsed_ms=" << GetTwinModeElapsedMs(*state)
               << " side=" << Aq40TwinEncounter::ToString(assignment->stableSide)
               << " slot=" << static_cast<uint32>(assignment->slotIndex);
        AppendTwinAnchorLogFields(fields, bot, anchor, anchorChoice.label);
        fields
               << " wait=distance"
               << " approach=" << state->approachMemberCount
               << " staged=" << state->stagedMemberCount
               << " center_committed=" << state->centerCommittedMemberCount
               << " strict_ready=" << state->strictReadyMemberCount
               << " assigned=" << state->assignments.size()
               << " unsupported_reason=" << (state->unsupportedReason.empty() ? "none" : state->unsupportedReason);
         AppendTwinOpeningOwnershipFields(fields, bot, *state);
        Aq40Helpers::LogAq40Info(bot, "twin_position", "twin:prepull:stage", fields.str(), 1000);
        return MoveInside(bot->GetMapId(), anchor.position.GetPositionX(), anchor.position.GetPositionY(),
            anchor.position.GetPositionZ(), kTwinAnchorTolerance, MovementPriority::MOVEMENT_NORMAL) ||
               overlayChanged || readyLostOverlayChanged || releasedPinnedBoss || clearedIntent || petChanged;
    }

    if (GetFacingDelta(anchor.facing, bot->GetOrientation()) > kTwinFacingTolerance)
    {
        bot->SetFacingTo(anchor.facing);
        return true;
    }

    if (!strictReady)
    {
        LogTwinPrePullStageWait(bot, *state, *assignment, anchorChoice, "strict_ready_pending");
        return overlayChanged || readyLostOverlayChanged || releasedPinnedBoss || clearedIntent || petChanged;
    }

    bool const readyChanged = Aq40TwinEncounter::SetMode(
        *state, Aq40TwinEncounter::TwinStrategyMode::StandardCompReady, getMSTime());
    if (readyChanged)
        LogTwinStrictReady(bot, *state);
    bool const readyOverlayChanged = readyChanged && SyncTwinWarlockTankOverlay(bot, botAI);

    if (ShouldHoldTwinReserveTankAssignmentNow(*state, *assignment))
    {
        if (pendingTankAttackTarget)
        {
            LogTwinTankAttackIntentDecision(bot, *state, *assignment, anchorChoice, pendingTankAttackTarget, nullptr,
                "prepull", "hold_reserve");
        }
        LogTwinPrePullStageWait(bot, *state, *assignment, anchorChoice, "reserve_hold");
        return overlayChanged || readyLostOverlayChanged || readyOverlayChanged || releasedPinnedBoss ||
               clearedIntent || petChanged;
    }

    if (ShouldStartTwinOpeningPullFromPrePull(*assignment) && !bot->IsInCombat())
    {
        TwinOpeningTargets const openingTargets = FindTwinOpeningTargetsForPrePull(botAI);
        Unit* target = GetTwinOpeningTargetForAssignment(*assignment, openingTargets);
        if (target && AreTwinOpeningTargetsDiscoverable(openingTargets))
        {
            if (pendingTankAttackTarget)
            {
                LogTwinTankAttackIntentDecision(bot, *state, *assignment, anchorChoice, pendingTankAttackTarget,
                    target, "prepull", "redirect_opener");
            }
            bool const armedDualPull = TryArmTwinOpeningDualPullFromPrePull(bot, *state, *assignment, openingTargets);
            Aq40Helpers::LogAq40Target(bot, "twin",
                armedDualPull ? "prepull_arm_dual_pull" : "prepull_dual_pull", target, 1000);
            return AttackTwinTarget(botAI, target) || armedDualPull || overlayChanged ||
                   readyLostOverlayChanged || readyOverlayChanged || releasedPinnedBoss || clearedIntent ||
                   petChanged;
        }

        LogTwinPrePullStageWait(bot, *state, *assignment, anchorChoice, "opener_target_unavailable");
        return overlayChanged || readyLostOverlayChanged || readyOverlayChanged || releasedPinnedBoss ||
               clearedIntent || petChanged;
    }

    if (!bot->IsInCombat())
        LogTwinPrePullStageWait(bot, *state, *assignment, anchorChoice, "opener_authorization");

    return overlayChanged || readyLostOverlayChanged || readyOverlayChanged || releasedPinnedBoss ||
           clearedIntent || petChanged;
}

bool Aq40TwinDualPullEngageAction::Execute(Event /*event*/)
{
    Aq40TwinEncounter::TwinEncounterState* state = Aq40TwinEncounter::GetEncounterState(bot);
    bool const overlayChanged = SyncTwinWarlockTankOverlay(bot, botAI);
    if (!state || state->phase != Aq40TwinEncounter::TwinEncounterPhase::DualPullWindow ||
        IsTwinHealerProfile(bot, botAI))
    {
        return overlayChanged;
    }

    Aq40TwinEncounter::TwinRoleAssignment const* assignment = Aq40TwinEncounter::GetAssignmentForMember(*state, bot->GetGUID());
    if (!assignment || !Aq40TwinEncounter::IsTwinEncounterParticipant(bot))
        return overlayChanged;

    TwinTargetIntent const intent = GetTwinTargetIntent(bot, botAI, *state, assignment);
    bool const releasedPinnedBoss = ReleaseTwinPinnedBossTargetForIntent(
        bot, botAI, *state, assignment, intent, "twin:dual_pull:release_wrong_target");
    GuidVector const encounterUnits = GetTwinEncounterUnits(botAI);
    bool const petChanged = SyncTwinEncounterPetPolicy(bot, botAI, *state, assignment, nullptr, encounterUnits);
    if (ShouldHoldTwinReserveTankAssignmentNow(*state, *assignment))
        return HoldTwinReserveTankAtAnchor(bot, botAI, *state, *assignment) || releasedPinnedBoss || overlayChanged ||
               petChanged;
    Unit* veklor = FindTwinBoss(botAI, encounterUnits, Aq40TwinEncounter::TwinBoss::Veklor);
    Unit* veknilash = FindTwinBoss(botAI, encounterUnits, Aq40TwinEncounter::TwinBoss::Veknilash);
    if (assignment)
        UpdateTwinSplitBandForMeleeTank(bot, *state, veklor, veknilash, *assignment);

    char const* reason = "dual_pull";
    Unit* target = ResolveTwinTarget(bot, botAI, *state, assignment, intent, encounterUnits, reason);
    if (!target)
        return releasedPinnedBoss || overlayChanged || petChanged;

    bool const targetPetChanged = SyncTwinEncounterPetPolicy(bot, botAI, *state, assignment, target, encounterUnits);
    bool const petSyncChanged = petChanged || targetPetChanged;

    if (assignment && IsTwinOpeningWarlockTankAssignment(*assignment) &&
        target->GetEntry() == Aq40SpellIds::TwinVeklorNpcEntry)
    {
        Aq40TwinEncounter::TwinEncounterGeometry const& geometry = Aq40TwinEncounter::GetGeometry();
        size_t const sideIndex = ToSideIndex(assignment->stableSide);
        Aq40TwinEncounter::TwinAnchor const& holdAnchor = geometry.sidePrep[sideIndex];
        Aq40TwinEncounter::TwinAnchor const& settleAnchor = geometry.stableVeklorWarlock[sideIndex];
        bool const hasThreatLead = HasTwinBossAggroLead(
            bot, botAI, *state, Aq40TwinEncounter::TwinBoss::Veklor, target);
        bool const shouldRotateInward = hasThreatLead &&
            (Aq40TwinEncounter::GetPhaseElapsedMs(*state) >= kTwinWarlockThreatLeadMs ||
             bot->GetDistance2d(target) < kTwinWarlockMinRange);

        if (AI_VALUE(Unit*, "current target") != target || bot->GetTarget() != target->GetGUID())
        {
            Aq40Helpers::LogAq40Target(bot, "twin", "dual_pull_veklor", target, 1000);
            return Attack(target) || releasedPinnedBoss || overlayChanged || petSyncChanged;
        }

        Aq40TwinEncounter::TwinAnchor const& anchor = shouldRotateInward ? settleAnchor : holdAnchor;
        if (bot->GetExactDist2d(anchor.position.GetPositionX(), anchor.position.GetPositionY()) > kTwinAnchorTolerance)
        {
            Aq40TwinEncounter::RequestImmediateMovementInterrupt(bot);
            std::ostringstream fields;
            fields << "boss=twin phase=dual_pull_window side="
                   << Aq40TwinEncounter::ToString(assignment->stableSide);
            AppendTwinAnchorLogFields(fields, bot, anchor,
                shouldRotateInward ? "stable_veklor_warlock" : "side_prep");
            Aq40Helpers::LogAq40Info(bot, "twin_position",
                shouldRotateInward ? "twin:dual_pull:veklor_rotate_inward" : "twin:dual_pull:veklor_hold_cast",
                fields.str(), 1000);
            return MoveInside(bot->GetMapId(), anchor.position.GetPositionX(), anchor.position.GetPositionY(),
                anchor.position.GetPositionZ(), kTwinAnchorTolerance, MovementPriority::MOVEMENT_COMBAT) ||
                   petSyncChanged;
        }

        if (FaceTwinAnchorIfNeeded(bot, anchor))
            return true;

        float const distance = bot->GetDistance2d(target);
        if (!shouldRotateInward && distance > kTwinWarlockMaxRange + 2.0f)
        {
            Aq40TwinEncounter::RequestImmediateMovementInterrupt(bot);
            Aq40Helpers::LogAq40Info(bot, "twin_position",
                "twin:dual_pull:veklor_stepin",
                "boss=twin phase=dual_pull_window reason=searing_pain_pull_range",
                1000);
            return MoveNear(target, kTwinWarlockPreferredRange, MovementPriority::MOVEMENT_COMBAT) ||
                   petSyncChanged;
        }

        if (shouldRotateInward && distance < kTwinWarlockMinRange)
        {
            Aq40TwinEncounter::RequestImmediateMovementInterrupt(bot);
            Aq40Helpers::LogAq40Info(bot, "twin_position",
                "twin:dual_pull:warlock_backstep",
                "boss=twin phase=dual_pull_window reason=stable_veklor_warlock target=" +
                    Aq40Helpers::GetAq40LogUnit(target),
                1000);
            return MoveAway(target, kTwinWarlockPreferredRange - distance) || petSyncChanged;
        }

        if (shouldRotateInward && distance > kTwinWarlockMaxRange)
        {
            Aq40TwinEncounter::RequestImmediateMovementInterrupt(bot);
            Aq40Helpers::LogAq40Info(bot, "twin_position",
                "twin:dual_pull:warlock_stepin",
                "boss=twin phase=dual_pull_window reason=stable_veklor_warlock target=" +
                    Aq40Helpers::GetAq40LogUnit(target),
                1000);
            return MoveNear(target, kTwinWarlockPreferredRange, MovementPriority::MOVEMENT_COMBAT) ||
                   petSyncChanged;
        }

        return releasedPinnedBoss || overlayChanged || petSyncChanged;
    }

    if (assignment && IsTwinOpeningMeleeTankAssignment(*assignment) &&
        target->GetEntry() == Aq40SpellIds::TwinVeknilashNpcEntry)
    {
        Aq40TwinEncounter::TwinAnchor const& bossPark =
            Aq40TwinEncounter::GetGeometry().bossPark[ToSideIndex(assignment->stableSide)];
        if (AI_VALUE(Unit*, "current target") != target || bot->GetTarget() != target->GetGUID())
        {
            Aq40Helpers::LogAq40Target(bot, "twin", "dual_pull_veknilash", target, 1000);
            return Attack(target) || releasedPinnedBoss || overlayChanged || petSyncChanged;
        }

        if (HasTwinBossAggroLead(bot, botAI, *state, Aq40TwinEncounter::TwinBoss::Veknilash, target))
        {
            if (bot->GetExactDist2d(bossPark.position.GetPositionX(), bossPark.position.GetPositionY()) >
                kTwinAnchorTolerance)
            {
                Aq40TwinEncounter::RequestImmediateMovementInterrupt(bot);
                std::ostringstream fields;
                fields << "boss=twin phase=dual_pull_window side="
                       << Aq40TwinEncounter::ToString(assignment->stableSide);
                AppendTwinAnchorLogFields(fields, bot, bossPark, "boss_park");
                Aq40Helpers::LogAq40Info(bot, "twin_position", "twin:dual_pull:veknilash_park", fields.str(),
                    1000);
                return MoveInside(bot->GetMapId(), bossPark.position.GetPositionX(), bossPark.position.GetPositionY(),
                    bossPark.position.GetPositionZ(), kTwinAnchorTolerance, MovementPriority::MOVEMENT_COMBAT) ||
                       petSyncChanged;
            }

            if (FaceTwinAnchorIfNeeded(bot, bossPark))
                return true;

            return releasedPinnedBoss || overlayChanged || petSyncChanged;
        }

        if (bot->GetDistance2d(target) > 8.0f)
        {
            Aq40TwinEncounter::RequestImmediateMovementInterrupt(bot);
            Aq40Helpers::LogAq40Info(bot, "twin_position",
                "twin:dual_pull:melee_stepin",
                "boss=twin phase=dual_pull_window reason=veknilash_pickup target=" +
                    Aq40Helpers::GetAq40LogUnit(target),
                1000);
            return MoveNear(target, kTwinMeleeContactRange, MovementPriority::MOVEMENT_COMBAT) ||
                   petSyncChanged;
        }

        return releasedPinnedBoss || overlayChanged || petSyncChanged;
    }

    if (target->GetEntry() == Aq40SpellIds::TwinVeknilashNpcEntry && IsTwinMeleeProfile(bot, botAI) &&
        bot->GetDistance2d(target) > 8.0f)
    {
        Aq40TwinEncounter::RequestImmediateMovementInterrupt(bot);
        Aq40Helpers::LogAq40Info(bot, "twin_position",
            "twin:dual_pull:melee_stepin",
            "boss=twin phase=dual_pull_window reason=melee_engage target=" + Aq40Helpers::GetAq40LogUnit(target),
            1000);
        return MoveNear(target, kTwinMeleeContactRange, MovementPriority::MOVEMENT_COMBAT) || petSyncChanged;
    }

    if (AI_VALUE(Unit*, "current target") == target && bot->GetVictim() == target)
        return releasedPinnedBoss || overlayChanged || petSyncChanged;

    Aq40Helpers::LogAq40Target(bot, "twin", reason, target, 1000);
    return Attack(target) || releasedPinnedBoss || overlayChanged || petSyncChanged;
}

bool Aq40TwinSwapPrepStageAction::Execute(Event /*event*/)
{
    Aq40TwinEncounter::TwinEncounterState* state = Aq40TwinEncounter::GetEncounterState(bot);
    bool const overlayChanged = SyncTwinWarlockTankOverlay(bot, botAI);
    if (!state || !IsTwinEncounterActive(state) || !Aq40TwinEncounter::IsSwapPrepActive(*state))
        return overlayChanged;

    uint32 const nowMs = getMSTime();
    bool const armedSwapPrep = Aq40TwinEncounter::ArmSwapPrep(*state, nowMs);
    if (armedSwapPrep)
    {
        std::ostringstream fields;
        fields << "boss=twin phase=" << Aq40TwinEncounter::ToString(state->phase)
               << " next_teleport_earliest_ms="
               << (state->nextTeleportEarliestAtMs > nowMs ? state->nextTeleportEarliestAtMs - nowMs : 0u)
               << " next_teleport_latest_ms="
               << (state->nextTeleportLatestAtMs > nowMs ? state->nextTeleportLatestAtMs - nowMs : 0u)
               << " veklor_expected_side="
               << Aq40TwinEncounter::ToString(GetTwinExpectedOwnerSide(*state, Aq40TwinEncounter::TwinBoss::Veklor))
               << " veknilash_expected_side="
               << Aq40TwinEncounter::ToString(GetTwinExpectedOwnerSide(*state, Aq40TwinEncounter::TwinBoss::Veknilash));
        Aq40Helpers::LogAq40Info(bot, "twin_swap_prep", "twin:swap_prep:arm", fields.str(), 1000);
    }

    Aq40TwinEncounter::TwinRoleAssignment const* assignment =
        Aq40TwinEncounter::GetAssignmentForMember(*state, bot->GetGUID());
    if (!assignment || IsTwinHealerProfile(bot, botAI))
        return armedSwapPrep || overlayChanged;

    if (assignment->cohort == Aq40TwinEncounter::TwinRoleCohort::WarlockTank)
    {
        GuidVector const encounterUnits = GetTwinEncounterUnits(botAI);
        if (Unit* veklor = FindTwinBoss(botAI, encounterUnits, Aq40TwinEncounter::TwinBoss::Veklor))
        {
            if (ShouldSuppressTwinWarlockVeklorThreat(bot, *state, assignment) &&
                ReleaseTwinPinnedTarget(
                    bot, botAI, veklor, *state, "twin:swap_prep:release_veklor", "swap_prep_hold",
                    assignment, TwinTargetIntent::HoldReserve))
            {
                return true;
            }
        }
    }

    TwinPrePullAnchorChoice const anchorChoice = GetTwinStableAnchorChoice(*state, *assignment);
    char const* eventKey = "twin:swap_prep:hold";
    switch (assignment->cohort)
    {
        case Aq40TwinEncounter::TwinRoleCohort::WarlockTank:
            eventKey = "twin:swap_prep:warlock_hold";
            break;
        case Aq40TwinEncounter::TwinRoleCohort::MeleeTank:
            eventKey = "twin:swap_prep:melee_proxy";
            break;
        case Aq40TwinEncounter::TwinRoleCohort::Hunter:
        case Aq40TwinEncounter::TwinRoleCohort::RangedDps:
            eventKey = "twin:swap_prep:ranged_hold";
            break;
        case Aq40TwinEncounter::TwinRoleCohort::MeleeDps:
            eventKey = "twin:swap_prep:melee_stage";
            break;
        default:
            break;
    }

    return MaintainTwinAssignedAnchor(
        bot, botAI, *state, *assignment, anchorChoice.anchor, eventKey, anchorChoice.label) ||
           armedSwapPrep || overlayChanged;
}

bool Aq40TwinHealerSupportAction::Execute(Event /*event*/)
{
    Aq40TwinEncounter::TwinEncounterState const* state = Aq40TwinEncounter::GetEncounterState(bot);
    if (!IsTwinEncounterActive(state) || !IsTwinHealerProfile(bot, botAI))
    {
        return false;
    }

    Aq40TwinEncounter::TwinRoleAssignment const* assignment =
        Aq40TwinEncounter::GetAssignmentForMember(*state, bot->GetGUID());
    GuidVector const encounterUnits = GetTwinEncounterUnits(botAI);
    bool const releasedPinnedBoss = ReleaseTwinPinnedBossTargetForIntent(
        bot, botAI, *state, assignment, TwinTargetIntent::None, "twin:healer:release_boss_target");
    bool const petChanged = SyncTwinEncounterPetPolicy(bot, botAI, *state, assignment, nullptr, encounterUnits);
    if (IsTwinSideHealerMode(*state, assignment))
    {
        bool const focusChanged = SyncTwinHealerFocusTargets(
            bot, botAI, *state, BuildTwinSideHealerFocusTargets(*state, assignment->stableSide),
            Aq40TwinEncounter::IsSwapPrepActive(*state) ? "swap_prep_preload" : "side_tank_package");
        TwinPrePullAnchorChoice const anchorChoice =
            GetTwinSideHealerRecoveryAnchorChoice(*state, *assignment, bot);
        bool const repositioned = MaintainTwinAssignedAnchor(
            bot, botAI, *state, *assignment, anchorChoice.anchor, "twin:side_healer:hold", anchorChoice.label);
        if (repositioned)
            return true;

        return TryTwinSwapPrepHealerPreload(bot, botAI, *state, *assignment) || focusChanged || petChanged ||
               releasedPinnedBoss;
    }

    bool const focusChanged = SyncTwinHealerFocusTargets(bot, botAI, *state, std::list<ObjectGuid>(),
        IsTwinHealerDegradedFallback(*state, assignment) ? "degraded_fallback" : "raid_healer_clear");
    if (assignment)
    {
        if (assignment->cohort == Aq40TwinEncounter::TwinRoleCohort::RaidHealer)
        {
            TwinPrePullAnchorChoice const anchorChoice = GetTwinStableAnchorChoice(*state, *assignment);
            return MaintainTwinAssignedAnchor(
                       bot, botAI, *state, *assignment, anchorChoice.anchor, "twin:raid_healer:hold",
                       anchorChoice.label) ||
                   focusChanged || petChanged || releasedPinnedBoss;
        }

        if (IsTwinHealerDegradedFallback(*state, assignment))
        {
            return MaintainTwinAssignedAnchor(bot, botAI, *state, *assignment,
                       GetCenterSpreadAnchor(assignment->slotIndex),
                       "twin:side_healer:degraded_fallback", "center_spread") ||
                   focusChanged || petChanged || releasedPinnedBoss;
        }
    }

    if (!state->assignments.empty())
        return focusChanged || petChanged || releasedPinnedBoss;

    Aq40TwinEncounter::TwinAnchor const& anchor = GetCenterSpreadAnchor(bot);
    if (bot->GetExactDist2d(anchor.position.GetPositionX(), anchor.position.GetPositionY()) <= kTwinAnchorTolerance)
        return focusChanged || petChanged || releasedPinnedBoss;

    Aq40TwinEncounter::RequestImmediateMovementInterrupt(bot);
    Aq40Helpers::LogAq40Info(bot, "spread_position",
        "twin:healer_support",
        "boss=twin phase=" + std::string(Aq40TwinEncounter::ToString(state->phase)) +
            " reason=center_spread",
        1000);
    return MoveInside(bot->GetMapId(), anchor.position.GetPositionX(), anchor.position.GetPositionY(),
        anchor.position.GetPositionZ(), kTwinAnchorTolerance, MovementPriority::MOVEMENT_COMBAT) ||
           focusChanged || petChanged || releasedPinnedBoss;
}

bool Aq40TwinChooseTargetAction::Execute(Event /*event*/)
{
    Aq40TwinEncounter::TwinEncounterState* state = Aq40TwinEncounter::GetEncounterState(bot);
    if (!IsTwinEncounterActive(state) || IsTwinHealerProfile(bot, botAI))
        return false;

    Aq40TwinEncounter::TwinRoleAssignment const* assignment =
        Aq40TwinEncounter::GetAssignmentForMember(*state, bot->GetGUID());
    GuidVector const encounterUnits = GetTwinEncounterUnits(botAI);
    bool const pickupAnchorChanged = SyncTwinPendingVeklorPickupAnchor(bot, *state, assignment);
    TwinTargetIntent const intent = GetTwinTargetIntent(bot, botAI, *state, assignment);
    bool const releasedPinnedBoss = ReleaseTwinPinnedBossTargetForIntent(
        bot, botAI, *state, assignment, intent, "twin:target:release_wrong_target");
    bool const petChanged = SyncTwinEncounterPetPolicy(bot, botAI, *state, assignment, nullptr, encounterUnits);
    if (assignment && ShouldHoldTwinReserveTankAssignmentNow(*state, *assignment))
    {
        return HoldTwinReserveTankAtAnchor(bot, botAI, *state, *assignment) || releasedPinnedBoss || petChanged ||
               pickupAnchorChanged;
    }

    Unit* veklor = FindTwinBoss(botAI, encounterUnits, Aq40TwinEncounter::TwinBoss::Veklor);
    Unit* veknilash = FindTwinBoss(botAI, encounterUnits, Aq40TwinEncounter::TwinBoss::Veknilash);
    if (assignment && assignment->cohort == Aq40TwinEncounter::TwinRoleCohort::MeleeTank &&
        !IsTwinMeleeTankController(*state, *assignment))
    {
        return (veknilash ? ReleaseTwinPinnedTarget(bot, botAI, veknilash, *state,
                                 "twin:melee_tank:release_veknilash", "ownership_gate", assignment,
                                 TwinTargetIntent::None)
                          : false) ||
               releasedPinnedBoss || petChanged || pickupAnchorChanged;
    }

    if (assignment)
        UpdateTwinSplitBandForMeleeTank(bot, *state, veklor, veknilash, *assignment);

    char const* reason = "boss";
    Unit* target = ResolveTwinTarget(bot, botAI, *state, assignment, intent, encounterUnits, reason);
    auto const holdStableAnchorWithoutTarget = [&]() -> bool
    {
        if (state->phase != Aq40TwinEncounter::TwinEncounterPhase::Stable || !assignment ||
            Aq40TwinEncounter::HasActiveLockedPickupAnchor(bot))
        {
            return false;
        }

        TwinPrePullAnchorChoice const anchorChoice = GetTwinStableAnchorChoice(*state, *assignment);
        switch (assignment->cohort)
        {
            case Aq40TwinEncounter::TwinRoleCohort::RangedDps:
                return MaintainTwinAssignedAnchor(bot, botAI, *state, *assignment, anchorChoice.anchor,
                    "twin:stable:ranged_hold", anchorChoice.label);

            case Aq40TwinEncounter::TwinRoleCohort::Hunter:
                return MaintainTwinAssignedAnchor(bot, botAI, *state, *assignment, anchorChoice.anchor,
                    "twin:stable:hunter_hold", anchorChoice.label);

            case Aq40TwinEncounter::TwinRoleCohort::MeleeDps:
                return MaintainTwinAssignedAnchor(bot, botAI, *state, *assignment, anchorChoice.anchor,
                    "twin:stable:melee_hold", anchorChoice.label);

            default:
                break;
        }

        return false;
    };
    if (!target)
        return holdStableAnchorWithoutTarget() || releasedPinnedBoss || petChanged || pickupAnchorChanged;

    bool const targetPetChanged = SyncTwinEncounterPetPolicy(bot, botAI, *state, assignment, target, encounterUnits);
    bool const petSyncChanged = petChanged || targetPetChanged;

    if (AI_VALUE(Unit*, "current target") != target || bot->GetTarget() != target->GetGUID())
    {
        Aq40Helpers::LogAq40Target(bot, "twin", reason, target, 1000);
        return Attack(target) || releasedPinnedBoss || petSyncChanged || pickupAnchorChanged;
    }

    if (state->phase == Aq40TwinEncounter::TwinEncounterPhase::Stable && assignment &&
        !Aq40TwinEncounter::HasActiveLockedPickupAnchor(bot))
    {
        switch (assignment->cohort)
        {
            case Aq40TwinEncounter::TwinRoleCohort::MeleeTank:
                if (target->GetEntry() == Aq40SpellIds::TwinVeknilashNpcEntry)
                {
                    if (HasTwinBossAggroLead(bot, botAI, *state, Aq40TwinEncounter::TwinBoss::Veknilash, target))
                    {
                        TwinPrePullAnchorChoice const anchorChoice = GetTwinStableAnchorChoice(*state, *assignment);
                        if (MaintainTwinAssignedAnchor(bot, botAI, *state, *assignment, anchorChoice.anchor,
                                "twin:stable:veknilash_park", anchorChoice.label))
                        {
                            return true;
                        }
                    }
                    else if (bot->GetDistance2d(target) > 8.0f)
                    {
                        Aq40TwinEncounter::RequestImmediateMovementInterrupt(bot);
                        Aq40Helpers::LogAq40Info(bot, "twin_position",
                            "twin:stable:melee_stepin",
                            "boss=twin phase=stable reason=veknilash_hold target=" +
                                Aq40Helpers::GetAq40LogUnit(target),
                            1000);
                        return MoveNear(target, kTwinMeleeContactRange, MovementPriority::MOVEMENT_COMBAT);
                    }
                }
                break;

            case Aq40TwinEncounter::TwinRoleCohort::RangedDps:
            case Aq40TwinEncounter::TwinRoleCohort::Hunter:
                case Aq40TwinEncounter::TwinRoleCohort::MeleeDps:
            {
                TwinPrePullAnchorChoice const anchorChoice = GetTwinStableAnchorChoice(*state, *assignment);
                char const* eventKey = assignment->cohort == Aq40TwinEncounter::TwinRoleCohort::RangedDps
                                           ? "twin:stable:ranged_hold"
                               : (assignment->cohort == Aq40TwinEncounter::TwinRoleCohort::Hunter
                                   ? "twin:stable:hunter_hold"
                                   : "twin:stable:melee_hold");
                if (MaintainTwinAssignedAnchor(bot, botAI, *state, *assignment, anchorChoice.anchor,
                        eventKey, anchorChoice.label))
                {
                    return true;
                }
                break;
            }

            default:
                break;
        }
    }

    if (bot->GetVictim() == target)
        return releasedPinnedBoss || petSyncChanged || pickupAnchorChanged;

    Aq40Helpers::LogAq40Target(bot, "twin", reason, target, 1000);
    return Attack(target) || releasedPinnedBoss || petSyncChanged || pickupAnchorChanged;
}

bool Aq40TwinHoldSplitAction::Execute(Event /*event*/)
{
    Aq40TwinEncounter::TwinEncounterState const* state = Aq40TwinEncounter::GetEncounterState(bot);
    if (!state || Aq40TwinEncounter::IsTerminalPhase(state->phase))
        return false;

    Aq40TwinEncounter::TwinRoleAssignment const* assignment =
        Aq40TwinEncounter::GetAssignmentForMember(*state, bot->GetGUID());
    bool const assignedParticipant = assignment && Aq40TwinEncounter::IsTwinEncounterParticipant(bot);
    if (!assignedParticipant && !Aq40TwinEncounter::HasActiveLockedPickupAnchor(bot))
        return false;

    if (Aq40TwinEncounter::TwinLockedPickupAnchor const* lockedAnchor = Aq40TwinEncounter::GetLockedPickupAnchor(bot))
    {
        if (Aq40TwinEncounter::IsLockedPickupAnchorExpired(*lockedAnchor) ||
            bot->GetExactDist2d(lockedAnchor->anchor.position.GetPositionX(),
                lockedAnchor->anchor.position.GetPositionY()) <= kTwinAnchorTolerance)
        {
            return false;
        }

        Aq40TwinEncounter::RequestImmediateMovementInterrupt(bot);
        Aq40Helpers::LogAq40Info(bot, "twin_position",
            "twin:hold_split:locked_anchor",
            "boss=twin phase=" + std::string(Aq40TwinEncounter::ToString(state->phase)) +
                " reason=locked_pickup_anchor",
            1000);
        return MoveInside(bot->GetMapId(), lockedAnchor->anchor.position.GetPositionX(),
            lockedAnchor->anchor.position.GetPositionY(), lockedAnchor->anchor.position.GetPositionZ(),
            kTwinAnchorTolerance, MovementPriority::MOVEMENT_COMBAT);
    }

    if (!assignedParticipant)
        return false;

    if (state->recovery.splitBand != Aq40TwinEncounter::TwinSplitBand::Warning &&
        state->recovery.splitBand != Aq40TwinEncounter::TwinSplitBand::Urgent)
    {
        return false;
    }

    GuidVector const encounterUnits = GetTwinEncounterUnits(botAI);
    Aq40TwinEncounter::TwinAnchor safeAnchor;
    if (IsTwinSideHealerMode(*state, assignment))
        safeAnchor = GetTwinSideHealerRecoveryAnchorChoice(*state, *assignment, bot).anchor;
    else if (IsTwinMeleeProfile(bot, botAI))
        safeAnchor = GetTwinSplitHoldAnchor(bot, botAI, *state, encounterUnits);
    else
        safeAnchor = GetCenterSpreadAnchor(bot);

    if (bot->GetExactDist2d(safeAnchor.position.GetPositionX(), safeAnchor.position.GetPositionY()) <=
        kTwinAnchorTolerance)
    {
        return false;
    }

    Aq40TwinEncounter::RequestImmediateMovementInterrupt(bot);
    Aq40Helpers::LogAq40Info(bot, "twin_position",
        "twin:hold_split:safe_anchor",
        "boss=twin phase=" + std::string(Aq40TwinEncounter::ToString(state->phase)) +
            " reason=split_risk_reanchor",
        1000);
    return MoveInside(bot->GetMapId(), safeAnchor.position.GetPositionX(), safeAnchor.position.GetPositionY(),
        safeAnchor.position.GetPositionZ(), kTwinAnchorTolerance, MovementPriority::MOVEMENT_COMBAT);
}

bool Aq40TwinWarlockTankAction::Execute(Event /*event*/)
{
    Aq40TwinEncounter::TwinEncounterState const* state = Aq40TwinEncounter::GetEncounterState(bot);
    bool const overlayChanged = SyncTwinWarlockTankOverlay(bot, botAI);
    if (!IsTwinEncounterActive(state) || !Aq40TwinEncounter::ShouldUseTwinWarlockTankStrategy(bot) ||
        IsTwinHealerProfile(bot, botAI))
    {
        return overlayChanged;
    }

    Aq40TwinEncounter::TwinRoleAssignment const* assignment =
        Aq40TwinEncounter::GetAssignmentForMember(*state, bot->GetGUID());
    bool const pickupAnchorChanged = SyncTwinPendingVeklorPickupAnchor(bot, *state, assignment);
    if (assignment && ShouldHoldTwinReserveTankAssignmentNow(*state, *assignment))
    {
        TwinTargetIntent const intent = GetTwinTargetIntent(bot, botAI, *state, assignment);
        bool const releasedPinnedBoss = ReleaseTwinPinnedBossTargetForIntent(
            bot, botAI, *state, assignment, intent, "twin:warlock_tank:release_wrong_target");
        return HoldTwinReserveTankAtAnchor(bot, botAI, *state, *assignment) || releasedPinnedBoss ||
               overlayChanged || pickupAnchorChanged;
    }

    GuidVector const encounterUnits = GetTwinEncounterUnits(botAI);
    Unit* veklor = FindTwinBoss(botAI, encounterUnits, Aq40TwinEncounter::TwinBoss::Veklor);
    if (!veklor)
        return overlayChanged || pickupAnchorChanged;

    if (assignment && !IsTwinWarlockTankController(*state, *assignment))
    {
        return ReleaseTwinPinnedTarget(bot, botAI, veklor, *state,
                   "twin:warlock_tank:release_veklor", "ownership_gate", assignment,
                   TwinTargetIntent::None) ||
               overlayChanged || pickupAnchorChanged;
    }

    if (AI_VALUE(Unit*, "current target") != veklor || bot->GetTarget() != veklor->GetGUID())
    {
        Aq40Helpers::LogAq40Target(bot, "twin", "warlock_pin", veklor, 1000);
        return Attack(veklor) || overlayChanged || pickupAnchorChanged;
    }

    if (state->phase == Aq40TwinEncounter::TwinEncounterPhase::Stable && assignment &&
        !Aq40TwinEncounter::HasActiveLockedPickupAnchor(bot) &&
        HasTwinBossAggroLead(bot, botAI, *state, Aq40TwinEncounter::TwinBoss::Veklor, veklor))
    {
        TwinPrePullAnchorChoice const anchorChoice = GetTwinStableAnchorChoice(*state, *assignment);
        if (MaintainTwinAssignedAnchor(bot, botAI, *state, *assignment, anchorChoice.anchor,
                "twin:stable:veklor_hold", anchorChoice.label))
        {
            return true;
        }
    }

    float const distance = bot->GetDistance2d(veklor);
    if (distance < kTwinWarlockMinRange)
    {
        Aq40TwinEncounter::RequestImmediateMovementInterrupt(bot);
        Aq40Helpers::LogAq40Info(bot, "twin_position",
            "twin:warlock_tank:backstep",
            "boss=twin phase=" + std::string(Aq40TwinEncounter::ToString(state->phase)) +
                " reason=veklor_range_hold",
            1000);
        return MoveAway(veklor, kTwinWarlockPreferredRange - distance) || overlayChanged ||
               pickupAnchorChanged;
    }

    if (distance > kTwinWarlockMaxRange)
    {
        Aq40TwinEncounter::RequestImmediateMovementInterrupt(bot);
        Aq40Helpers::LogAq40Info(bot, "twin_position",
            "twin:warlock_tank:stepin",
            "boss=twin phase=" + std::string(Aq40TwinEncounter::ToString(state->phase)) +
                " reason=veklor_range_hold",
            1000);
        return MoveNear(veklor, kTwinWarlockPreferredRange, MovementPriority::MOVEMENT_COMBAT) ||
               overlayChanged ||
               pickupAnchorChanged;
    }

    return overlayChanged || pickupAnchorChanged;
}

bool Aq40TwinDodgeBlizzardAction::Execute(Event /*event*/)
{
    Aq40TwinEncounter::TwinEncounterState const* state = Aq40TwinEncounter::GetEncounterState(bot);
    if (!state)
        return false;

    bool const scriptedWindow = Aq40TwinEncounter::IsScriptedEventActive(
        *state, Aq40TwinEncounter::TwinScriptedEvent::Blizzard, kTwinBlizzardWindowMs);
    if (!IsTwinEncounterActive(state) && !scriptedWindow)
        return false;

    Aura* blizzardAura = GetTwinBlizzardAura(bot, botAI);
    DynamicObject* blizzardDynObj = blizzardAura ? blizzardAura->GetDynobjOwner() : nullptr;
    bool const hasBlizzardAura = blizzardAura != nullptr;
    bool const hasBlizzardDynObj = blizzardDynObj && blizzardDynObj->IsInWorld();
    if (!hasBlizzardAura && !hasBlizzardDynObj && !scriptedWindow)
        return false;

    Aq40TwinEncounter::RequestImmediateMovementInterrupt(bot);
    if (hasBlizzardDynObj)
    {
        float const hazardRadius = blizzardDynObj->GetRadius();
        if (hazardRadius > 0.0f && bot->GetDistance(blizzardDynObj) <= hazardRadius &&
            FleePosition(blizzardDynObj->GetPosition(), hazardRadius, 250U))
        {
            Aq40Helpers::LogAq40Info(bot, "avoid_hazard",
                "twin:blizzard:dynamic_object",
                "boss=twin hazard=blizzard phase=" + std::string(Aq40TwinEncounter::ToString(state->phase)) +
                    " reason=dynamic_object",
                1000);
            return true;
        }
    }

    if (botAI->DoSpecificAction("avoid aoe", Event(), true))
    {
        Aq40Helpers::LogAq40Info(bot, "avoid_hazard",
            "twin:blizzard:avoid_aoe",
            "boss=twin hazard=blizzard phase=" + std::string(Aq40TwinEncounter::ToString(state->phase)) +
                " reason=" + (hasBlizzardDynObj ? std::string("dynamic_object")
                                                 : (hasBlizzardAura ? std::string("aura")
                                                                    : std::string("scripted_window"))),
            1000);
        return true;
    }

    GuidVector const encounterUnits = GetTwinEncounterUnits(botAI);
    Aq40TwinEncounter::TwinRoleAssignment const* assignment =
        Aq40TwinEncounter::GetAssignmentForMember(*state, bot->GetGUID());
    TwinPrePullAnchorChoice const anchorChoice =
        GetTwinHazardRecoveryAnchorChoice(bot, botAI, *state, assignment, encounterUnits);
    return MoveTwinToHazardAnchor(bot, botAI, *state, assignment, anchorChoice,
        "twin:blizzard:fallback_anchor",
        hasBlizzardDynObj ? "dynamic_object" : (hasBlizzardAura ? "aura" : "scripted_window"));
}

bool Aq40TwinDodgeExplodeBugAction::Execute(Event /*event*/)
{
    Aq40TwinEncounter::TwinEncounterState const* state = Aq40TwinEncounter::GetEncounterState(bot);
    if (!state || !IsTwinEncounterActive(state))
        return false;

    Aq40TwinEncounter::TwinRoleAssignment const* assignment =
        Aq40TwinEncounter::GetAssignmentForMember(*state, bot->GetGUID());
    if (IsTwinPrimaryTankController(*state, assignment))
        return false;

    bool const scriptedWindow = Aq40TwinEncounter::IsScriptedEventActive(
        *state, Aq40TwinEncounter::TwinScriptedEvent::ExplodeBug, kTwinExplodeBugWindowMs);
    GuidVector const encounterUnits = GetTwinEncounterUnits(botAI);
    Unit* explodeBug = nullptr;
    Position explodeSourcePosition;
    if (!TryGetTwinExplodeBugHazard(bot, botAI, *state, encounterUnits, kTwinExplodeBugDangerRadius,
            scriptedWindow, explodeBug, explodeSourcePosition))
    {
        return false;
    }

    Spell* spell = explodeBug ? explodeBug->GetCurrentSpell(CURRENT_GENERIC_SPELL) : nullptr;
    bool const castingExplosion = IsTwinExplodeBugCast(spell);
    if (!scriptedWindow && !castingExplosion)
        return false;

    Aq40TwinEncounter::RequestImmediateMovementInterrupt(bot);
    Aq40Helpers::LogAq40Info(bot, "avoid_hazard",
        "twin:explode_bug:flee",
        "boss=twin hazard=explode_bug source=" +
            (explodeBug ? Aq40Helpers::GetAq40LogUnit(explodeBug) : std::string("tracked_script_source")),
        1000);
    if (FleePosition(explodeSourcePosition, kTwinExplodeBugDangerRadius, 250U))
        return true;

    if (explodeBug)
    {
        float const distance = bot->GetDistance2d(explodeBug);
        if (distance < kTwinExplodeBugDangerRadius &&
            MoveAway(explodeBug, kTwinExplodeBugDangerRadius - distance))
        {
            return true;
        }
    }

    TwinPrePullAnchorChoice const anchorChoice =
        GetTwinHazardRecoveryAnchorChoice(bot, botAI, *state, assignment, encounterUnits);
    return MoveTwinToHazardAnchor(bot, botAI, *state, assignment, anchorChoice,
        "twin:explode_bug:recover", "explode_bug_ring");
}

bool Aq40TwinAvoidVeklorAction::Execute(Event /*event*/)
{
    Aq40TwinEncounter::TwinEncounterState const* state = Aq40TwinEncounter::GetEncounterState(bot);
    if (!state || Aq40TwinEncounter::IsTerminalPhase(state->phase))
        return false;

    GuidVector const encounterUnits = GetTwinEncounterUnits(botAI);
    Unit* veklor = FindTwinBoss(botAI, encounterUnits, Aq40TwinEncounter::TwinBoss::Veklor);
    if (!veklor)
        return false;

    Aq40TwinEncounter::TwinRoleAssignment const* assignment =
        Aq40TwinEncounter::GetAssignmentForMember(*state, bot->GetGUID());
    bool const isActiveVeklorController = assignment &&
                                          assignment->cohort == Aq40TwinEncounter::TwinRoleCohort::WarlockTank &&
                                          IsTwinWarlockTankController(*state, *assignment);
    if (isActiveVeklorController)
        return false;

    float const distance = bot->GetDistance2d(veklor);
    bool const arcaneWindow = Aq40TwinEncounter::IsScriptedEventActive(
        *state, Aq40TwinEncounter::TwinScriptedEvent::ArcaneBurst, kTwinArcaneBurstWindowMs);
    if (distance > kTwinArcaneBurstDangerRadius && !(arcaneWindow && distance <= kTwinArcaneBurstLooseRadius))
        return false;

    TwinPrePullAnchorChoice const anchorChoice =
        GetTwinHazardRecoveryAnchorChoice(bot, botAI, *state, assignment, encounterUnits);
    return MoveTwinToHazardAnchor(bot, botAI, *state, assignment, anchorChoice,
        "twin:veklor:avoid", arcaneWindow ? "arcane_burst_window" : "veklor_proximity");
}

bool Aq40TwinPostSwapHoldAction::Execute(Event /*event*/)
{
    Aq40TwinEncounter::TwinEncounterState const* state = Aq40TwinEncounter::GetEncounterState(bot);
    if (!state || Aq40TwinEncounter::IsTerminalPhase(state->phase) ||
        (state->phase != Aq40TwinEncounter::TwinEncounterPhase::TeleportWindow &&
         state->phase != Aq40TwinEncounter::TwinEncounterPhase::PickupRecovery &&
         !Aq40TwinEncounter::IsAnyThreatHoldWindowActive(*state)))
    {
        return false;
    }

    Aq40TwinEncounter::TwinRoleAssignment const* assignment =
        Aq40TwinEncounter::GetAssignmentForMember(*state, bot->GetGUID());
    bool const pickupAnchorChanged = SyncTwinPendingVeklorPickupAnchor(bot, *state, assignment);
    bool const assignedParticipant = assignment && Aq40TwinEncounter::IsTwinEncounterParticipant(bot);
    if (!assignedParticipant && !Aq40TwinEncounter::HasActiveLockedPickupAnchor(bot))
        return pickupAnchorChanged;

    if (Aq40TwinEncounter::TwinLockedPickupAnchor const* lockedAnchor = Aq40TwinEncounter::GetLockedPickupAnchor(bot))
    {
        if (!Aq40TwinEncounter::IsLockedPickupAnchorExpired(*lockedAnchor) &&
            bot->GetExactDist2d(lockedAnchor->anchor.position.GetPositionX(),
                lockedAnchor->anchor.position.GetPositionY()) > kTwinAnchorTolerance)
        {
            Aq40TwinEncounter::RequestImmediateMovementInterrupt(bot);
            Aq40Helpers::LogAq40Info(bot, "twin_position",
                "twin:post_swap:locked_anchor",
                "boss=twin phase=" + std::string(Aq40TwinEncounter::ToString(state->phase)) +
                    " reason=locked_pickup_anchor",
                1000);
            return MoveInside(bot->GetMapId(), lockedAnchor->anchor.position.GetPositionX(),
                lockedAnchor->anchor.position.GetPositionY(), lockedAnchor->anchor.position.GetPositionZ(),
                kTwinAnchorTolerance, MovementPriority::MOVEMENT_COMBAT) ||
                   pickupAnchorChanged;
        }

        return pickupAnchorChanged;
    }

    if (!assignedParticipant)
        return pickupAnchorChanged;

    GuidVector const encounterUnits = GetTwinEncounterUnits(botAI);
    Aq40TwinEncounter::TwinAnchor holdAnchor;
    if (IsTwinSideHealerMode(*state, assignment))
        holdAnchor = GetTwinSideHealerRecoveryAnchorChoice(*state, *assignment, bot).anchor;
    else if (IsTwinMeleeProfile(bot, botAI))
        holdAnchor = GetVeknilashSideAnchor(botAI, encounterUnits);
    else
        holdAnchor = GetCenterSpreadAnchor(bot);

    if (bot->GetExactDist2d(holdAnchor.position.GetPositionX(), holdAnchor.position.GetPositionY()) <=
        kTwinAnchorTolerance)
    {
        return pickupAnchorChanged;
    }

    Aq40TwinEncounter::RequestImmediateMovementInterrupt(bot);
    Aq40Helpers::LogAq40Info(bot, "twin_position",
        "twin:post_swap:hold_anchor",
        "boss=twin phase=" + std::string(Aq40TwinEncounter::ToString(state->phase)) +
            " reason=post_swap_hold",
        1000);
    return MoveInside(bot->GetMapId(), holdAnchor.position.GetPositionX(), holdAnchor.position.GetPositionY(),
        holdAnchor.position.GetPositionZ(), kTwinAnchorTolerance, MovementPriority::MOVEMENT_COMBAT) ||
           pickupAnchorChanged;
}
