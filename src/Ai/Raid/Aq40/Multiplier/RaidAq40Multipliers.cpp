#include "RaidAq40Multipliers.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <sstream>
#include <unordered_map>

#include "Action.h"
#include "AttackAction.h"
#include "ChooseTargetActions.h"
#include "DKActions.h"
#include "DruidBearActions.h"
#include "FollowActions.h"
#include "GenericActions.h"
#include "GenericSpellActions.h"
#include "HunterActions.h"
#include "MageActions.h"
#include "MovementActions.h"
#include "ObjectGuid.h"
#include "PaladinActions.h"
#include "Pet.h"
#include "Playerbots.h"
#include "ReachTargetActions.h"
#include "WarriorActions.h"
#include "../Action/RaidAq40Actions.h"
#include "../RaidAq40BossHelper.h"
#include "../RaidAq40SpellIds.h"
#include "../Util/RaidAq40Helpers_Cthun.h"
#include "../Util/RaidAq40Helpers_Shared.h"
#include "../Util/RaidAq40Helpers_Skeram.h"
#include "../Util/RaidAq40TwinEncounter.h"

namespace
{
uint32 constexpr kTwinStableControllerConfirmationWindowMs = 6000;
float constexpr kTwinExplodeBugDangerRadius = 17.0f;
float constexpr kTwinArcaneBurstDangerRadius = 18.0f;
float constexpr kTwinArcaneBurstLooseRadius = 24.0f;
float constexpr kTwinRangedBugServiceRange = 32.0f;
float constexpr kTwinRangedGenericBugServiceRange = 22.0f;
float constexpr kTwinHunterMarkedBugServiceRange = 26.0f;
float constexpr kTwinRangedBugArcaneSafeRadius = kTwinArcaneBurstDangerRadius + 2.0f;
float constexpr kTwinHunterBugArcaneSafeRadius = kTwinArcaneBurstLooseRadius + 4.0f;
float constexpr kTwinExplodeBugServiceSafeRadius = kTwinExplodeBugDangerRadius + 2.0f;
float constexpr kTwinHunterPetSafeRadius = kTwinArcaneBurstLooseRadius + 2.0f;
std::unordered_map<uint64, bool> sTwinRegistrationCandidateByBot;

struct TwinAnchorOffsetPattern
{
    float forward = 0.0f;
    float lateral = 0.0f;
};

struct Direction2d
{
    float x = 0.0f;
    float y = 0.0f;
    float length = 0.0f;
};

enum class TwinBugPriority : uint8
{
    Explode = 0,
    Mutate = 1,
    Hostile = 2,
    None = 255,
};

// Returns true only for Defender Thunderclap (ranged/healers within 24y).
bool IsAq40TrashMovementCase(PlayerbotAI* botAI, Player* bot, GuidVector const& encounterUnits)
{
    if (!botAI || !bot)
        return false;

    if (!PlayerbotAI::IsRanged(bot) && !botAI->IsHeal(bot))
        return false;

    for (ObjectGuid const guid : encounterUnits)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit)
            continue;

        Spell* spell = unit->GetCurrentSpell(CURRENT_GENERIC_SPELL);
        if (spell &&
            Aq40SpellIds::MatchesAnySpellId(spell->GetSpellInfo(), { Aq40SpellIds::Aq40DefenderThunderclap }) &&
            bot->GetDistance2d(unit) < 24.0f)
            return true;
    }

    return false;
}

// IsSarturaMob / IsSarturaSpinning now live in Aq40BossHelper.

bool IsTwinRegistrationCandidate(Player const* bot)
{
    return bot && bot->IsAlive() && bot->IsInWorld() && Aq40BossHelper::IsInAq40(bot) &&
           (Aq40TwinEncounter::HasActiveLockedPickupAnchor(bot) || Aq40TwinEncounter::IsTwinEncounterParticipant(bot));
}

Unit* GetTwinCurrentTarget(PlayerbotAI* botAI)
{
    if (!botAI || !botAI->GetAiObjectContext())
        return nullptr;

    return botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Get();
}

Unit* GetTwinSelectionTarget(Player* bot, PlayerbotAI* botAI)
{
    if (!bot || !botAI || bot->GetTarget().IsEmpty())
        return nullptr;

    return botAI->GetUnit(bot->GetTarget());
}

Unit* GetTwinPetTarget(Player* bot)
{
    if (!bot)
        return nullptr;

    Pet* pet = bot->GetPet();
    return pet ? pet->GetVictim() : nullptr;
}

Unit* GetTwinObservedTarget(Player* bot, PlayerbotAI* botAI)
{
    if (!bot || !botAI)
        return nullptr;

    if (Unit* currentTarget = GetTwinCurrentTarget(botAI))
        return currentTarget;

    if (Unit* victim = bot->GetVictim())
        return victim;

    return GetTwinSelectionTarget(bot, botAI);
}

Player* GetTwinFollowLeader(PlayerbotAI* botAI)
{
    if (!botAI)
        return nullptr;

    if (Player* master = botAI->GetMaster())
        return master;

    return botAI->GetGroupLeader();
}

bool IsTwinFollowUseful(PlayerbotAI* botAI)
{
    if (!botAI)
        return false;

    FollowAction followAction(botAI);
    return followAction.isUseful();
}

bool IsTwinTrueCasterProfile(Player* bot, PlayerbotAI* botAI)
{
    return bot && botAI && !botAI->IsHeal(bot) && bot->getClass() != CLASS_HUNTER &&
           PlayerbotAI::IsRanged(bot);
}

bool WasTwinRegistrationCandidate(Player* bot)
{
    if (!bot)
        return false;

    auto const itr = sTwinRegistrationCandidateByBot.find(bot->GetGUID().GetRawValue());
    return itr != sTwinRegistrationCandidateByBot.end() && itr->second;
}

void SetTwinRegistrationCandidate(Player* bot, bool candidate)
{
    if (!bot)
        return;

    uint64 const botKey = bot->GetGUID().GetRawValue();
    if (!candidate)
    {
        sTwinRegistrationCandidateByBot.erase(botKey);
        return;
    }

    sTwinRegistrationCandidateByBot[botKey] = true;
}

char const* GetTwinRegistrationMovementReason(Aq40TwinEncounter::TwinEncounterState const& state, Player* bot,
                                              bool assignedParticipant, bool hasLockedPickupAnchor,
                                              bool approachTwin, bool prepullStage, bool activeTwin,
                                              bool postSwapHold, bool terminalTwin)
{
    if (terminalTwin)
        return "terminal_cleanup";

    if (postSwapHold)
        return hasLockedPickupAnchor ? "locked_pickup_hold" : "post_swap_hold";

    if (activeTwin)
    {
        if (Aq40TwinEncounter::IsSwapPrepActive(state))
            return "swap_prep";

        return Aq40TwinEncounter::ToString(state.phase);
    }

    if (prepullStage)
    {
        return state.mode == Aq40TwinEncounter::TwinStrategyMode::StandardCompReady
            ? "side_owned_stage"
            : "strict_ready_pending";
    }

    if (approachTwin)
        return "cleanup_only";

    if (assignedParticipant && !Aq40TwinEncounter::IsTwinEncounterParticipant(bot, false))
        return "approach_range_only";

    return "registration_pending";
}

void LogTwinRegistrationCandidateTransition(Player* bot, Aq40TwinEncounter::TwinEncounterState const& state,
                                            Aq40TwinEncounter::TwinRoleAssignment const* assignment,
                                            bool assignedParticipant, bool hasLockedPickupAnchor,
                                            bool approachTwin, bool prepullStage, bool activeTwin,
                                            bool postSwapHold, bool terminalTwin)
{
    if (!bot || WasTwinRegistrationCandidate(bot))
        return;

    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
    Unit* const currentTarget = botAI ? GetTwinCurrentTarget(botAI) : nullptr;
    Unit* const selectionTarget = botAI ? GetTwinSelectionTarget(bot, botAI) : nullptr;
    Unit* const observedTarget = botAI ? GetTwinObservedTarget(bot, botAI) : bot->GetVictim();
    Unit* const petTarget = GetTwinPetTarget(bot);
    Player* const followLeader = botAI ? GetTwinFollowLeader(botAI) : nullptr;
    char const* movementReason = GetTwinRegistrationMovementReason(
        state, bot, assignedParticipant, hasLockedPickupAnchor, approachTwin, prepullStage, activeTwin,
        postSwapHold, terminalTwin);

    std::ostringstream fields;
    fields << "boss=twin registration=candidate"
           << " phase=" << Aq40TwinEncounter::ToString(state.phase)
           << " mode=" << Aq40TwinEncounter::ToString(state.mode)
           << " movement_reason=" << movementReason
           << " cohort=" << (assignment ? Aq40TwinEncounter::ToString(assignment->cohort) : "none")
           << " side=" << (assignment ? Aq40TwinEncounter::ToString(assignment->stableSide) : "unknown")
           << " slot=" << (assignment ? static_cast<uint32>(assignment->slotIndex) : 0u)
           << " target=" << Aq40Helpers::GetAq40LogUnit(observedTarget)
           << " current_target=" << Aq40Helpers::GetAq40LogUnit(currentTarget)
           << " selection_target=" << Aq40Helpers::GetAq40LogUnit(selectionTarget)
           << " victim=" << Aq40Helpers::GetAq40LogUnit(bot->GetVictim())
           << " pet_target=" << Aq40Helpers::GetAq40LogUnit(petTarget)
           << " follow_target=" << Aq40Helpers::GetAq40LogUnit(followLeader)
           << " follow_strategy_nc=" << (botAI && botAI->HasStrategy("follow", BOT_STATE_NON_COMBAT) ? 1 : 0)
           << " follow_strategy_combat=" << (botAI && botAI->HasStrategy("follow", BOT_STATE_COMBAT) ? 1 : 0)
           << " follow_useful=" << (botAI && IsTwinFollowUseful(botAI) ? 1 : 0)
           << " stay_strategy_nc=" << (botAI && botAI->HasStrategy("stay", BOT_STATE_NON_COMBAT) ? 1 : 0)
           << " stay_strategy_combat=" << (botAI && botAI->HasStrategy("stay", BOT_STATE_COMBAT) ? 1 : 0)
           << " approach=" << state.approachMemberCount
           << " staged=" << state.stagedMemberCount
           << " center_committed=" << state.centerCommittedMemberCount
           << " strict_ready=" << state.strictReadyMemberCount
           << " assigned=" << state.assignments.size()
           << " locked_pickup=" << (hasLockedPickupAnchor ? 1 : 0)
           << " assigned_participant=" << (assignedParticipant ? 1 : 0);
    Aq40Helpers::LogAq40Info(bot, "twin_registration",
        "twin:registration:candidate:" + std::string(movementReason), fields.str(), 1000);
    SetTwinRegistrationCandidate(bot, true);
}

void LogTwinRegistrationDecision(Player* bot, Aq40TwinEncounter::TwinEncounterState const& state,
                                 bool registrationActive)
{
    if (!bot || registrationActive || state.phase != Aq40TwinEncounter::TwinEncounterPhase::PrePull ||
        !Aq40TwinEncounter::HasDeterministicAssignments(state))
    {
        return;
    }

    Aq40TwinEncounter::TwinRoleAssignment const* assignment =
        Aq40TwinEncounter::GetAssignmentForMember(state, bot->GetGUID());
    if (!assignment)
        return;

    bool const distanceWait = !Aq40TwinEncounter::IsTwinEncounterParticipant(bot, false);
    char const* waitReason = distanceWait ? "distance"
                                          : (Aq40TwinEncounter::IsTwinCenterCommitted(state)
                                                  ? "strict_ready_pending"
                                                  : "center_commit_pending");
    std::ostringstream fields;
    fields << "boss=twin registration=inactive"
           << " mode=" << Aq40TwinEncounter::ToString(state.mode)
           << " wait=" << waitReason
           << " cohort=" << Aq40TwinEncounter::ToString(assignment->cohort)
           << " side=" << Aq40TwinEncounter::ToString(assignment->stableSide)
           << " slot=" << static_cast<uint32>(assignment->slotIndex)
           << " approach=" << state.approachMemberCount
           << " staged=" << state.stagedMemberCount
           << " center_committed=" << state.centerCommittedMemberCount
           << " strict_ready=" << state.strictReadyMemberCount
           << " assigned=" << state.assignments.size()
           << " unsupported_reason=" << (state.unsupportedReason.empty() ? "none" : state.unsupportedReason);
    Aq40Helpers::LogAq40Info(bot, "twin_registration",
        "twin:registration:" + std::string(waitReason),
        fields.str(), 2000);
}

bool IsTwinRegistrationWindow(Player* bot)
{
    if (!IsTwinRegistrationCandidate(bot))
    {
        SetTwinRegistrationCandidate(bot, false);
        return false;
    }

    Aq40TwinEncounter::TwinEncounterState const* state = Aq40TwinEncounter::GetEncounterState(bot);
    if (!state)
    {
        SetTwinRegistrationCandidate(bot, false);
        return false;
    }

    bool const assignedParticipant = Aq40TwinEncounter::IsTwinAssignedParticipant(*state, bot);
    bool const hasLockedPickupAnchor = Aq40TwinEncounter::HasActiveLockedPickupAnchor(bot);
    bool const approachTwin = Aq40TwinEncounter::IsTwinApproachWindow(*state, bot);
    bool const prepullStage = Aq40TwinEncounter::IsTwinPrePullStageWindow(*state, bot);
    bool const activeTwin = assignedParticipant && Aq40TwinEncounter::IsTwinCombatAuthorized(*state);
    bool const postSwapHold = !Aq40TwinEncounter::IsTerminalPhase(state->phase) &&
                              (hasLockedPickupAnchor ||
                               (assignedParticipant && Aq40TwinEncounter::IsAnyThreatHoldWindowActive(*state)));
    bool const terminalTwin = Aq40TwinEncounter::IsTerminalPhase(state->phase) &&
                              (hasLockedPickupAnchor || assignedParticipant ||
                               Aq40TwinEncounter::IsTwinEncounterParticipant(bot));
    Aq40TwinEncounter::TwinRoleAssignment const* assignment =
        Aq40TwinEncounter::GetAssignmentForMember(*state, bot->GetGUID());
    LogTwinRegistrationCandidateTransition(bot, *state, assignment, assignedParticipant, hasLockedPickupAnchor,
                                           approachTwin, prepullStage, activeTwin, postSwapHold, terminalTwin);
    bool const registrationActive = approachTwin || prepullStage || activeTwin || postSwapHold || terminalTwin;
    LogTwinRegistrationDecision(bot, *state, registrationActive);
    return registrationActive;
}

bool IsTwinSharedAq40Action(std::string const& actionName)
{
    return actionName == "aq40 manage resistance strategies" || actionName == "aq40 erase timers and trackers";
}

bool IsTwinGenericTargetAction(Action* action)
{
    return dynamic_cast<DpsAoeAction*>(action) ||
           dynamic_cast<DpsAssistAction*>(action) ||
           dynamic_cast<TankAssistAction*>(action) ||
           dynamic_cast<AggressiveTargetAction*>(action) ||
           dynamic_cast<AttackAnythingAction*>(action) ||
           dynamic_cast<AttackLeastHpTargetAction*>(action) ||
           dynamic_cast<AttackRtiTargetAction*>(action) ||
           dynamic_cast<DropTargetAction*>(action);
}

bool IsTwinMovementDriftAction(Action* action)
{
    return dynamic_cast<CombatFormationMoveAction*>(action) ||
           dynamic_cast<FollowAction*>(action) ||
           dynamic_cast<FleeAction*>(action);
}

bool IsTwinQueuedEscapeAction(Action* action)
{
    return dynamic_cast<AvoidAoeAction*>(action) ||
           dynamic_cast<MoveFromGroupAction*>(action) ||
           dynamic_cast<RunAwayAction*>(action) ||
           dynamic_cast<MoveOutOfEnemyContactAction*>(action) ||
           dynamic_cast<CastDisengageAction*>(action) ||
           dynamic_cast<CastBlinkBackAction*>(action);
}

bool IsTwinStableAnchorCohort(Aq40TwinEncounter::TwinRoleCohort cohort)
{
    switch (cohort)
    {
        case Aq40TwinEncounter::TwinRoleCohort::SideHealer:
        case Aq40TwinEncounter::TwinRoleCohort::RaidHealer:
        case Aq40TwinEncounter::TwinRoleCohort::RangedDps:
        case Aq40TwinEncounter::TwinRoleCohort::Hunter:
            return true;

        default:
            return false;
    }
}

bool IsTwinVeklorTarget(Unit const* unit)
{
    return unit && unit->GetEntry() == Aq40SpellIds::TwinVeklorNpcEntry;
}

bool IsTwinVeknilashTarget(Unit const* unit)
{
    return unit && unit->GetEntry() == Aq40SpellIds::TwinVeknilashNpcEntry;
}

bool IsTwinBossTarget(Unit const* unit)
{
    return IsTwinVeklorTarget(unit) || IsTwinVeknilashTarget(unit);
}

bool IsTwinBugTarget(Unit const* unit)
{
    return unit && Aq40SpellIds::IsTwinBugEntry(unit->GetEntry());
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

bool IsTwinUnsafePickupWindow(Aq40TwinEncounter::TwinEncounterState const& state, Player const* bot)
{
    return Aq40TwinEncounter::IsSwapPrepActive(state) ||
           state.phase == Aq40TwinEncounter::TwinEncounterPhase::TeleportWindow ||
           state.phase == Aq40TwinEncounter::TwinEncounterPhase::PickupRecovery ||
           Aq40TwinEncounter::IsAnyThreatHoldWindowActive(state) ||
           Aq40TwinEncounter::HasActiveLockedPickupAnchor(bot);
}

Aq40TwinEncounter::TwinSide GetTwinSideForPosition(float x, float y)
{
    Aq40TwinEncounter::TwinEncounterGeometry const& geometry = Aq40TwinEncounter::GetGeometry();
    float const side0Distance = geometry.bossPark[0].position.GetExactDist2d(x, y);
    float const side1Distance = geometry.bossPark[1].position.GetExactDist2d(x, y);
    return side0Distance <= side1Distance ? Aq40TwinEncounter::TwinSide::Side0
                                          : Aq40TwinEncounter::TwinSide::Side1;
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

size_t GetTwinSideIndex(Aq40TwinEncounter::TwinSide side)
{
    return side == Aq40TwinEncounter::TwinSide::Side1 ? 1u : 0u;
}

Direction2d GetTwinDirection2d(Position const& from, Position const& toward)
{
    float const dx = toward.GetPositionX() - from.GetPositionX();
    float const dy = toward.GetPositionY() - from.GetPositionY();
    Direction2d direction;
    direction.length = std::sqrt(dx * dx + dy * dy);
    if (direction.length >= 0.01f)
    {
        direction.x = dx / direction.length;
        direction.y = dy / direction.length;
    }

    return direction;
}

Position BuildTwinOffsetPosition(Position const& origin, Position const& toward, float forwardDistance,
                                 float lateralDistance)
{
    Direction2d const direction = GetTwinDirection2d(origin, toward);
    Position position;
    if (direction.length < 0.01f)
    {
        position.Relocate(origin.GetPositionX(), origin.GetPositionY(), origin.GetPositionZ());
        return position;
    }

    float const rightX = direction.y;
    float const rightY = -direction.x;
    float const zRatio = std::min(std::fabs(forwardDistance) / direction.length, 1.0f);
    position.Relocate(origin.GetPositionX() + direction.x * forwardDistance + rightX * lateralDistance,
        origin.GetPositionY() + direction.y * forwardDistance + rightY * lateralDistance,
        origin.GetPositionZ() + (toward.GetPositionZ() - origin.GetPositionZ()) * zRatio);
    return position;
}

Position GetTwinHunterStagePosition(Aq40TwinEncounter::TwinSide side, uint8 slotIndex)
{
    static std::array<TwinAnchorOffsetPattern, 4> const kPatterns = {
        TwinAnchorOffsetPattern{ 12.0f, -4.0f },
        TwinAnchorOffsetPattern{ 14.0f, 4.0f },
        TwinAnchorOffsetPattern{ 16.0f, -7.0f },
        TwinAnchorOffsetPattern{ 18.0f, 7.0f },
    };

    Aq40TwinEncounter::TwinEncounterGeometry const& geometry = Aq40TwinEncounter::GetGeometry();
    Aq40TwinEncounter::TwinAnchor const& bossPark = geometry.bossPark[GetTwinSideIndex(side)];
    TwinAnchorOffsetPattern const& pattern = kPatterns[slotIndex % kPatterns.size()];
    return BuildTwinOffsetPosition(
        bossPark.position, geometry.roomCenter.position, pattern.forward, pattern.lateral);
}

Position GetTwinRangedStagePosition(Aq40TwinEncounter::TwinSide side, uint8 slotIndex)
{
    static std::array<TwinAnchorOffsetPattern, 5> const kPatterns = {
        TwinAnchorOffsetPattern{ -2.0f, -4.0f },
        TwinAnchorOffsetPattern{ -1.0f, 4.0f },
        TwinAnchorOffsetPattern{ 2.0f, -8.0f },
        TwinAnchorOffsetPattern{ 3.0f, 8.0f },
        TwinAnchorOffsetPattern{ 5.0f, 0.0f },
    };

    Aq40TwinEncounter::TwinEncounterGeometry const& geometry = Aq40TwinEncounter::GetGeometry();
    Aq40TwinEncounter::TwinAnchor const& base = geometry.stableVeklorWarlock[GetTwinSideIndex(side)];
    TwinAnchorOffsetPattern const& pattern = kPatterns[slotIndex % kPatterns.size()];
    return BuildTwinOffsetPosition(
        base.position, geometry.roomCenter.position, pattern.forward, pattern.lateral);
}

Aq40TwinEncounter::TwinRoleCohort GetTwinEffectiveBugServiceCohort(
    Aq40TwinEncounter::TwinRoleAssignment const* assignment, Player* bot, PlayerbotAI* botAI)
{
    if (assignment)
        return assignment->cohort;

    if (bot && bot->getClass() == CLASS_HUNTER)
        return Aq40TwinEncounter::TwinRoleCohort::Hunter;

    return bot && botAI && !PlayerbotAI::IsRanged(bot) && !botAI->IsHeal(bot)
        ? Aq40TwinEncounter::TwinRoleCohort::MeleeDps
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
    if (priority == TwinBugPriority::None)
        return false;

    return IsTwinBugServiceRole(assignment, bot, botAI);
}

bool IsTwinBugMarkedBySpell(PlayerbotAI* botAI, Unit* bug, uint32 spellId)
{
    if (!botAI || !bug)
        return false;

    Spell* spell = bug->GetCurrentSpell(CURRENT_GENERIC_SPELL);
    return (spell && Aq40SpellIds::MatchesAnySpellId(spell->GetSpellInfo(), { spellId })) ||
           Aq40SpellIds::HasAnyAura(botAI, bug, { spellId });
}

TwinBugPriority GetTwinBugPriority(Player* bot, PlayerbotAI* botAI, Unit* bug)
{
    if (!bot || !botAI || !bug || !bug->IsAlive() || !Aq40SpellIds::IsTwinBugEntry(bug->GetEntry()))
        return TwinBugPriority::None;

    if (IsTwinBugMarkedBySpell(botAI, bug, Aq40SpellIds::TwinExplodeBug))
        return TwinBugPriority::Explode;

    if (IsTwinBugMarkedBySpell(botAI, bug, Aq40SpellIds::TwinMutateBug))
        return TwinBugPriority::Mutate;

    return TwinBugPriority::Hostile;
}

Position GetTwinBugServiceOriginPosition(Player* bot,
                                         Aq40TwinEncounter::TwinRoleAssignment const* assignment)
{
    Position origin;
    if (!bot)
        return origin;

    origin.Relocate(bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ());
    if (!assignment)
        return origin;

    switch (assignment->cohort)
    {
        case Aq40TwinEncounter::TwinRoleCohort::RangedDps:
            return GetTwinRangedStagePosition(assignment->stableSide, assignment->slotIndex);

        case Aq40TwinEncounter::TwinRoleCohort::Hunter:
            return GetTwinHunterStagePosition(assignment->stableSide, assignment->slotIndex);

        default:
            return origin;
    }
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

bool HasTwinBugPriorityTarget(Player* bot, PlayerbotAI* botAI,
                              Aq40TwinEncounter::TwinEncounterState const& state,
                              Aq40TwinEncounter::TwinRoleAssignment const* assignment,
                              GuidVector const& units, Unit* veklor)
{
    if (!bot || !botAI || !IsTwinBugServiceWindow(bot, state) || !IsTwinBugServiceRole(assignment, bot, botAI))
        return false;

    Position const serviceOrigin = GetTwinBugServiceOriginPosition(bot, assignment);
    Aq40TwinEncounter::TwinSide const serviceSide =
        GetTwinBugServiceSide(bot, assignment, serviceOrigin);

    for (ObjectGuid const guid : units)
    {
        Unit* bug = botAI->GetUnit(guid);
        TwinBugPriority const priority = GetTwinBugPriority(bot, botAI, bug);
        if (IsTwinBugSafeForService(bot, botAI, assignment, veklor, bug, priority, serviceOrigin, serviceSide))
            return true;
    }

    return false;
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
    if (!target || !IsTwinBossTarget(target))
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
            return IsTwinVeklorTarget(target) &&
                   Aq40TwinEncounter::IsPrimaryController(state, Aq40TwinEncounter::TwinBoss::Veklor,
                       assignment->memberGuid);

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

bool IsTwinMeleeDpsVeklorProximityRisk(Player* bot, Unit* veklor,
                                       Aq40TwinEncounter::TwinRoleAssignment const* assignment)
{
    return bot && veklor && assignment &&
           assignment->cohort == Aq40TwinEncounter::TwinRoleCohort::MeleeDps &&
           bot->GetDistance2d(veklor) <= kTwinArcaneBurstLooseRadius;
}

bool IsTwinHunterPetVeklorProximityRisk(Player* bot, Unit* veklor)
{
    if (!bot || !veklor)
        return false;

    Pet* pet = bot->GetPet();
    return pet && pet->IsAlive() && pet->IsInWorld() &&
           pet->GetDistance2d(veklor) <= kTwinHunterPetSafeRadius;
}

bool IsTwinHunterSafePetAttackWindow(Player* bot, Aq40TwinEncounter::TwinEncounterState const& state,
                                     Aq40TwinEncounter::TwinRoleAssignment const* assignment,
                                     Unit const* currentTarget, Unit* veklor)
{
    if (!bot || bot->getClass() != CLASS_HUNTER || !assignment ||
        assignment->cohort != Aq40TwinEncounter::TwinRoleCohort::Hunter || !currentTarget ||
        currentTarget->GetEntry() != Aq40SpellIds::TwinVeknilashNpcEntry)
    {
        return false;
    }

    if (state.phase != Aq40TwinEncounter::TwinEncounterPhase::Stable ||
        state.recovery.splitBand != Aq40TwinEncounter::TwinSplitBand::Stable ||
        Aq40TwinEncounter::IsSwapPrepActive(state) || IsTwinUnsafePickupWindow(state, bot))
    {
        return false;
    }

    Aq40TwinEncounter::TwinSide const targetSide =
        GetTwinSideForPosition(currentTarget->GetPositionX(), currentTarget->GetPositionY());
    if (assignment->stableSide != targetSide ||
        GetTwinSideForPosition(bot->GetPositionX(), bot->GetPositionY()) != targetSide)
    {
        return false;
    }

    if (Pet* pet = bot->GetPet())
    {
        if (!pet->IsAlive() || !pet->IsInWorld() ||
            GetTwinSideForPosition(pet->GetPositionX(), pet->GetPositionY()) != targetSide)
        {
            return false;
        }
    }

    if (!veklor)
        return true;

    return bot->GetDistance2d(veklor) > kTwinHunterPetSafeRadius &&
           !IsTwinHunterPetVeklorProximityRisk(bot, veklor);
}
}    // namespace

float Aq40GenericMultiplier::GetValue(Action* action)
{
    if (!action || !Aq40BossHelper::IsInAq40(bot))
        return 1.0f;

    std::string const actionName = action->getName();
    if (Aq40SpellIds::HasAnyAura(botAI, bot, { Aq40SpellIds::Aq40DefenderPlague }))
    {
        if (actionName == "aq40 trash avoid dangerous aoe")
            return 4.0f;

        // Suppress movement that could break plague separation for all roles.
        if (dynamic_cast<CombatFormationMoveAction*>(action) ||
            dynamic_cast<FollowAction*>(action) ||
            dynamic_cast<FleeAction*>(action))
            return 0.0f;

        // Melee bots cannot attack during plague — closing distance violates separation.
        if (!PlayerbotAI::IsRanged(bot) && !botAI->IsHeal(bot))
        {
            if (dynamic_cast<AttackAction*>(action) ||
                dynamic_cast<ReachTargetAction*>(action) ||
                dynamic_cast<CastReachTargetSpellAction*>(action) ||
                dynamic_cast<MovementAction*>(action))
                return 0.0f;
        }

        // Ranged and healers can keep casting/healing from their current position.
        return 1.0f;
    }

    if (!Aq40BossHelper::IsEncounterTank(bot, bot))
    {
        GuidVector encounterUnits = Aq40BossHelper::GetEncounterUnits(botAI, AI_VALUE(GuidVector, "attackers"));
        if (!encounterUnits.empty() &&
            !Aq40BossHelper::IsBossEncounterActive(botAI, encounterUnits) &&
            Aq40BossHelper::IsTrashEncounterActive(botAI, encounterUnits) &&
            IsAq40TrashMovementCase(botAI, bot, encounterUnits))
        {
            if (actionName == "aq40 trash avoid dangerous aoe")
                return 3.5f;

            if (dynamic_cast<CombatFormationMoveAction*>(action) ||
                dynamic_cast<FollowAction*>(action) ||
                dynamic_cast<FleeAction*>(action) ||
                (dynamic_cast<MovementAction*>(action) && actionName != "aq40 trash avoid dangerous aoe"))
                return 0.0f;

            if (dynamic_cast<AttackAction*>(action) ||
                dynamic_cast<ReachTargetAction*>(action) ||
                dynamic_cast<CastReachTargetSpellAction*>(action))
                return 0.0f;
        }
    }

    return 1.0f;
}

float Aq40SkeramMultiplier::GetValue(Action* action)
{
    if (!action || !Aq40BossHelper::IsInAq40(bot))
        return 1.0f;

    GuidVector const attackers = AI_VALUE(GuidVector, "attackers");
    if (!Aq40Helpers::IsSkeramEncounterLive(bot, botAI, attackers))
        return 1.0f;

    std::string const actionName = action->getName();

    // Whitelist Skeram-specific actions.
    bool isSkeramControlAction =
        actionName == "aq40 skeram acquire platform target" ||
        actionName == "aq40 skeram interrupt" ||
        actionName == "aq40 skeram focus real boss" ||
        actionName == "aq40 skeram control mind control" ||
        actionName == "aq40 choose target";
    if (isSkeramControlAction)
        return 1.0f;

    if (actionName.compare(0, 11, "aq40 trash ") == 0)
        return 0.0f;

    // Suppress generic assist actions that scatter DPS across split copies
    // (pattern used by all reference raids for complex encounters).
    if (dynamic_cast<DpsAssistAction*>(action) || dynamic_cast<TankAssistAction*>(action))
        return 0.0f;

    return 1.0f;
}

float Aq40BugTrioMultiplier::GetValue(Action* action)
{
    if (!action || !Aq40BossHelper::IsInAq40(bot))
        return 1.0f;

    GuidVector activeUnits = Aq40BossHelper::GetActiveCombatUnits(botAI, AI_VALUE(GuidVector, "attackers"));
    if (!Aq40BossHelper::HasAnyNamedUnit(botAI, activeUnits, { "lord kri", "princess yauj", "vem", "yauj brood" }))
        return 1.0f;

    GuidVector encounterUnits = Aq40BossHelper::GetEncounterUnits(botAI, AI_VALUE(GuidVector, "attackers"));
    Unit* kri = Aq40BossHelper::FindUnitByAnyName(botAI, encounterUnits, { "lord kri" });
    if (!kri)
        return 1.0f;

    bool poisonCloudWindow = kri->GetHealthPct() <= 5.0f ||
                             Aq40SpellIds::HasAnyAura(botAI, kri, { Aq40SpellIds::BugTrioPoisonCloud });
    if (!poisonCloudWindow || Aq40BossHelper::IsEncounterTank(bot, bot))
        return 1.0f;

    if (bot->GetDistance2d(kri) > 12.0f)
        return 1.0f;

    std::string const actionName = action->getName();
    if (actionName == "aq40 bug trio avoid poison cloud")
        return 3.5f;

    if (dynamic_cast<MovementAction*>(action) &&
        !dynamic_cast<Aq40BugTrioAvoidPoisonCloudAction*>(action))
        return 0.0f;

    if (dynamic_cast<AttackAction*>(action) ||
        dynamic_cast<ReachTargetAction*>(action) ||
        dynamic_cast<CastReachTargetSpellAction*>(action))
        return 0.0f;

    return 1.0f;
}

float Aq40SarturaMultiplier::GetValue(Action* action)
{
    if (!action || !Aq40BossHelper::IsInAq40(bot) || Aq40BossHelper::IsEncounterTank(bot, bot))
        return 1.0f;

    GuidVector encounterUnits = Aq40BossHelper::GetEncounterUnits(botAI, AI_VALUE(GuidVector, "attackers"));
    bool whirlwindRisk = false;
    bool const isBackline = botAI->IsRanged(bot) || botAI->IsHeal(bot);
    for (ObjectGuid const guid : encounterUnits)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!Aq40BossHelper::IsSarturaSpinning(botAI, unit))
            continue;

        float const distance = bot->GetDistance2d(unit);
        bool const isClosingOnBot = unit->GetVictim() == bot || unit->GetTarget() == bot->GetGUID();
        if (distance <= 18.0f || (isBackline && isClosingOnBot && distance <= 24.0f))
        {
            whirlwindRisk = true;
            break;
        }
    }

    if (!whirlwindRisk)
        return 1.0f;

    if (dynamic_cast<Aq40SarturaAvoidWhirlwindAction*>(action))
        return 3.5f;

    if (dynamic_cast<CastReachTargetSpellAction*>(action) ||
        (dynamic_cast<MovementAction*>(action) &&
         !dynamic_cast<Aq40SarturaAvoidWhirlwindAction*>(action)))
        return 0.0f;

    return 1.0f;
}

float Aq40FankrissMultiplier::GetValue(Action* action)
{
    if (!action || !Aq40BossHelper::IsInAq40(bot))
        return 1.0f;

    GuidVector activeUnits = Aq40BossHelper::GetActiveCombatUnits(botAI, AI_VALUE(GuidVector, "attackers"));
    if (!Aq40BossHelper::HasAnyNamedUnit(botAI, activeUnits, { "fankriss the unyielding" }))
        return 1.0f;

    std::string const actionName = action->getName();

    // Whitelist Fankriss-specific actions.
    if (actionName.compare(0, 14, "aq40 fankriss ") == 0)
        return 1.0f;

    // Suppress trash actions during boss encounter.
    if (actionName.compare(0, 11, "aq40 trash ") == 0)
        return 0.0f;

    // Suppress generic assist actions that scatter DPS away from boss-assigned targets.
    if (dynamic_cast<DpsAssistAction*>(action) || dynamic_cast<TankAssistAction*>(action))
        return 0.0f;

    // Suppress flee/formation movement for non-tanks to keep the raid stable.
    if (!Aq40BossHelper::IsEncounterTank(bot, bot))
    {
        if (dynamic_cast<FleeAction*>(action) || dynamic_cast<CombatFormationMoveAction*>(action))
            return 0.0f;
    }

    return 1.0f;
}

float Aq40HuhuranMultiplier::GetValue(Action* action)
{
    if (!action || !Aq40BossHelper::IsInAq40(bot))
        return 1.0f;

    GuidVector activeUnits = Aq40BossHelper::GetActiveCombatUnits(botAI, AI_VALUE(GuidVector, "attackers"));
    Unit* huhuran = Aq40BossHelper::FindUnitByAnyName(botAI, activeUnits, { "princess huhuran" });
    if (!huhuran || Aq40BossHelper::IsEncounterTank(bot, bot))
        return 1.0f;

    bool const isBackline = botAI->IsRanged(bot) || botAI->IsHeal(bot);
    if (!isBackline)
        return 1.0f;

    bool const poisonPhase = huhuran->GetHealthPct() <= 32.0f ||
                             Aq40SpellIds::HasAnyAura(botAI, huhuran, { Aq40SpellIds::HuhuranFrenzy });
    if (!poisonPhase)
        return 1.0f;

    if (dynamic_cast<Aq40HuhuranPoisonSpreadAction*>(action))
        return 3.0f;

    if (dynamic_cast<CombatFormationMoveAction*>(action) ||
        dynamic_cast<FollowAction*>(action) ||
        dynamic_cast<FleeAction*>(action) ||
        (dynamic_cast<MovementAction*>(action) &&
         !dynamic_cast<Aq40HuhuranPoisonSpreadAction*>(action)))
        return 0.0f;

    return 1.0f;
}

float Aq40TwinMultiplier::GetValue(Action* action)
{
    if (!action || !Aq40BossHelper::IsInAq40(bot) || !IsTwinRegistrationWindow(bot))
        return 1.0f;

    Aq40TwinEncounter::TwinEncounterState const* state = Aq40TwinEncounter::GetEncounterState(bot);
    if (!state)
        return 1.0f;

    Aq40TwinEncounter::TwinRoleAssignment const* assignment =
        Aq40TwinEncounter::GetAssignmentForMember(*state, bot->GetGUID());
    bool const hasLockedPickupAnchor = Aq40TwinEncounter::HasActiveLockedPickupAnchor(bot);
    bool const terminalRoomParticipant = Aq40TwinEncounter::IsTerminalPhase(state->phase) &&
                                         Aq40TwinEncounter::IsTwinEncounterParticipant(bot);
    if (!assignment && !hasLockedPickupAnchor && !terminalRoomParticipant)
        return 1.0f;

    bool const assignedParticipant = Aq40TwinEncounter::IsTwinAssignedParticipant(*state, bot);
    bool const approachTwin = Aq40TwinEncounter::IsTwinApproachWindow(*state, bot);
    bool const prepullStage = Aq40TwinEncounter::IsTwinPrePullStageWindow(*state, bot);
    bool const activeTwin = assignedParticipant && Aq40TwinEncounter::IsTwinCombatAuthorized(*state);
    bool const terminalTwin = Aq40TwinEncounter::IsTerminalPhase(state->phase) &&
                              (hasLockedPickupAnchor || assignedParticipant || terminalRoomParticipant);
    bool const postSwapHold = !Aq40TwinEncounter::IsTerminalPhase(state->phase) &&
                              (hasLockedPickupAnchor ||
                               (assignedParticipant && Aq40TwinEncounter::IsAnyThreatHoldWindowActive(*state)));
    bool const nonDegradedTwin = state->phase != Aq40TwinEncounter::TwinEncounterPhase::Degraded;
    bool const activeNonDegradedTwin = activeTwin && nonDegradedTwin;
    bool const immediateRepositionWindow = Aq40TwinEncounter::IsImmediateRepositionWindow(*state);
    bool const authoritativeMovementWindow =
        activeNonDegradedTwin &&
        (immediateRepositionWindow || hasLockedPickupAnchor);
    bool const unsafePickupWindow = nonDegradedTwin && IsTwinUnsafePickupWindow(*state, bot);
    bool const stableVeklorControllerInvalid =
        state->phase == Aq40TwinEncounter::TwinEncounterPhase::Stable &&
        !Aq40TwinEncounter::IsSwapPrepActive(*state) &&
        !HasTwinCredibleStableController(*state, Aq40TwinEncounter::TwinBoss::Veklor);
    bool const strictVeklorSuppressionWindow =
        state->phase == Aq40TwinEncounter::TwinEncounterPhase::DualPullWindow || unsafePickupWindow ||
        stableVeklorControllerInvalid;
    bool const isPrimaryVeklorController = assignment &&
                                           assignment->cohort == Aq40TwinEncounter::TwinRoleCohort::WarlockTank &&
                                           Aq40TwinEncounter::IsPrimaryController(
                                               *state, Aq40TwinEncounter::TwinBoss::Veklor, assignment->memberGuid);
    bool const isPrimaryVeknilashController = assignment &&
                                              assignment->cohort == Aq40TwinEncounter::TwinRoleCohort::MeleeTank &&
                                              Aq40TwinEncounter::IsPrimaryController(
                                                  *state, Aq40TwinEncounter::TwinBoss::Veknilash,
                                                  assignment->memberGuid);
    bool const hasVeklorLockedPickupAnchor =
        Aq40TwinEncounter::HasLockedPickupAnchor(bot, Aq40TwinEncounter::TwinBoss::Veklor);
    bool const pendingSwapPrepVeklorWarlock =
        assignment && assignment->cohort == Aq40TwinEncounter::TwinRoleCohort::WarlockTank &&
        Aq40TwinEncounter::IsSwapPrepActive(*state) &&
        Aq40TwinEncounter::GetOwnership(*state, Aq40TwinEncounter::TwinBoss::Veklor).expectedOwner ==
            assignment->memberGuid &&
        !isPrimaryVeklorController;
    bool const pendingPostTeleportVeklorWarlock =
        assignment && assignment->cohort == Aq40TwinEncounter::TwinRoleCohort::WarlockTank &&
        !Aq40TwinEncounter::IsPickupEstablished(*state, Aq40TwinEncounter::TwinBoss::Veklor) &&
        (state->phase == Aq40TwinEncounter::TwinEncounterPhase::TeleportWindow ||
         state->phase == Aq40TwinEncounter::TwinEncounterPhase::PickupRecovery ||
         Aq40TwinEncounter::IsThreatHoldWindowActive(*state, Aq40TwinEncounter::TwinBoss::Veklor)) &&
        Aq40TwinEncounter::GetOwnership(*state, Aq40TwinEncounter::TwinBoss::Veklor).expectedOwner ==
            assignment->memberGuid &&
        isPrimaryVeklorController;
    bool const suppressNonControllerWarlockVeklor = bot->getClass() == CLASS_WARLOCK &&
                                                    !isPrimaryVeklorController &&
                                                    (!Aq40TwinEncounter::IsPickupEstablished(
                                                         *state, Aq40TwinEncounter::TwinBoss::Veklor) ||
                                                     unsafePickupWindow);
    bool const suppressUnsafeRangedVeklorThreat = unsafePickupWindow && !isPrimaryVeklorController && assignment &&
                                                  (assignment->cohort == Aq40TwinEncounter::TwinRoleCohort::RangedDps ||
                                                   assignment->cohort == Aq40TwinEncounter::TwinRoleCohort::Hunter);
    bool const suppressNonControllerVeklorWindowTargeting =
        strictVeklorSuppressionWindow && !isPrimaryVeklorController && !hasVeklorLockedPickupAnchor;
    bool const suppressReserveTankBossTargeting =
        strictVeklorSuppressionWindow && assignment &&
        (assignment->cohort == Aq40TwinEncounter::TwinRoleCohort::MeleeTank ||
         assignment->cohort == Aq40TwinEncounter::TwinRoleCohort::WarlockTank) &&
        !hasLockedPickupAnchor && !isPrimaryVeklorController && !isPrimaryVeknilashController;

    std::string const actionName = action->getName();
    bool const isTwinAction = actionName.compare(0, 10, "aq40 twin ") == 0;
    if (terminalTwin)
    {
        if (isTwinAction || actionName == "aq40 choose target" || actionName == "shoot")
            return 0.0f;

        if (dynamic_cast<PetAttackAction*>(action) || dynamic_cast<SetPetStanceAction*>(action) ||
            dynamic_cast<TogglePetSpellAutoCastAction*>(action))
        {
            return 0.0f;
        }

        if (actionName.compare(0, 5, "aq40 ") == 0 && actionName.compare(0, 10, "aq40 twin ") != 0 &&
            !IsTwinSharedAq40Action(actionName))
        {
            return 0.0f;
        }

        if (dynamic_cast<CastReachTargetSpellAction*>(action) || dynamic_cast<MovementAction*>(action))
            return 0.0f;

        return 1.0f;
    }

    if (isTwinAction)
    {
        if (actionName == "aq40 twin approach stage")
            return approachTwin ? 2.5f : 1.0f;
        if (actionName == "aq40 twin prepull stage")
            return prepullStage ? 3.0f : 1.0f;
        if (actionName == "aq40 twin dual pull engage")
            return state->phase == Aq40TwinEncounter::TwinEncounterPhase::DualPullWindow ? 3.5f : 1.0f;
        if (actionName == "aq40 twin swap prep stage")
            return Aq40TwinEncounter::IsSwapPrepActive(*state) ? 4.0f : 1.0f;
        if (actionName == "aq40 twin post swap hold")
            return postSwapHold ? 4.0f : 1.0f;
        if (actionName == "aq40 twin hold split")
            return (postSwapHold || state->recovery.splitBand == Aq40TwinEncounter::TwinSplitBand::Warning ||
                       state->recovery.splitBand == Aq40TwinEncounter::TwinSplitBand::Urgent)
                       ? 3.0f
                       : 1.0f;
        if (actionName == "aq40 twin dodge explode bug")
            return Aq40TwinEncounter::IsScriptedEventActive(
                       *state, Aq40TwinEncounter::TwinScriptedEvent::ExplodeBug, 2500)
                       ? 4.0f
                       : 1.0f;
        if (actionName == "aq40 twin dodge blizzard")
            return Aq40TwinEncounter::IsScriptedEventActive(
                       *state, Aq40TwinEncounter::TwinScriptedEvent::Blizzard, 5000)
                       ? 3.5f
                       : 1.0f;
        if (actionName == "aq40 twin avoid veklor")
            return activeTwin ? 3.0f : 1.0f;
        if (actionName == "aq40 twin warlock tank")
            return Aq40TwinEncounter::ShouldUseTwinWarlockTankStrategy(bot) ? 2.5f : 0.0f;
        if (actionName == "aq40 twin choose target")
            return activeTwin ? 2.0f : 1.0f;
        return 1.0f;
    }

    if (actionName == "aq40 choose target")
        return 0.0f;

    if (actionName.compare(0, 5, "aq40 ") == 0 && actionName.compare(0, 10, "aq40 twin ") != 0 &&
        !IsTwinSharedAq40Action(actionName))
    {
        return 0.0f;
    }

    if (IsTwinGenericTargetAction(action))
        return 0.0f;

    if (dynamic_cast<SetPetStanceAction*>(action) || dynamic_cast<TogglePetSpellAutoCastAction*>(action))
        return 0.0f;

    if (authoritativeMovementWindow && IsTwinQueuedEscapeAction(action))
        return 0.0f;

    if (assignment)
    {
        bool const isTwinTankAssignment = assignment->cohort == Aq40TwinEncounter::TwinRoleCohort::MeleeTank ||
                                          assignment->cohort == Aq40TwinEncounter::TwinRoleCohort::WarlockTank;
        bool const isStableAnchorCohort = IsTwinStableAnchorCohort(assignment->cohort);

        if (assignment->cohort == Aq40TwinEncounter::TwinRoleCohort::MeleeTank &&
            !immediateRepositionWindow &&
            (dynamic_cast<TankFaceAction*>(action) || actionName == "set facing"))
        {
            return 0.0f;
        }

        if (nonDegradedTwin && isTwinTankAssignment &&
            (dynamic_cast<ReachTargetAction*>(action) || dynamic_cast<CastReachTargetSpellAction*>(action)))
        {
            return 0.0f;
        }

        if (activeNonDegradedTwin && isStableAnchorCohort &&
            (dynamic_cast<ReachTargetAction*>(action) || dynamic_cast<CastReachTargetSpellAction*>(action)))
        {
            return 0.0f;
        }

        bool const suppressMeleeDpsUnsafeReach =
            assignment->cohort == Aq40TwinEncounter::TwinRoleCohort::MeleeDps &&
            (Aq40TwinEncounter::IsSwapPrepActive(*state) ||
             state->phase == Aq40TwinEncounter::TwinEncounterPhase::TeleportWindow ||
             state->phase == Aq40TwinEncounter::TwinEncounterPhase::PickupRecovery ||
             Aq40TwinEncounter::IsAnyThreatHoldWindowActive(*state) || hasLockedPickupAnchor);
        if (activeNonDegradedTwin && suppressMeleeDpsUnsafeReach &&
            (dynamic_cast<ReachTargetAction*>(action) || dynamic_cast<CastReachTargetSpellAction*>(action)))
        {
            return 0.0f;
        }
    }

    if (approachTwin && dynamic_cast<FollowAction*>(action))
    {
        // Before center commit, Twin only owns cleanup; let the normal leader-follow path keep travel player-driven.
        return 1.0f;
    }

    if (IsTwinMovementDriftAction(action))
    {
        return 0.0f;
    }

    Unit* const actionTarget = action->GetTarget();
    Unit* const currentTarget = AI_VALUE(Unit*, "current target");
    Unit* const selectionTarget = bot->GetTarget().IsEmpty() ? nullptr : botAI->GetUnit(bot->GetTarget());
    Unit* const currentVictim = bot->GetVictim();
    bool const attackOrReachAction = dynamic_cast<AttackAction*>(action) ||
                                     dynamic_cast<ReachTargetAction*>(action) ||
                                     dynamic_cast<CastReachTargetSpellAction*>(action);
    bool const offensiveSpellAction = dynamic_cast<CastSpellAction*>(action) &&
                                      !dynamic_cast<CastHealingSpellAction*>(action);
    bool const targetlessShootAction = actionName == "shoot";
    bool const usesCurrentTarget = attackOrReachAction || offensiveSpellAction || targetlessShootAction;
    std::array<Unit*, 4> const guardedTargets = {
        actionTarget,
        usesCurrentTarget ? currentTarget : nullptr,
        selectionTarget,
        currentVictim,
    };
    bool targetsTwinBoss = false;
    bool targetsVeklor = false;
    bool targetsTwinBug = false;
    Unit* guardedBossTarget = nullptr;
    for (Unit* candidate : guardedTargets)
    {
        if (!candidate)
            continue;

        if (IsTwinBossTarget(candidate))
        {
            targetsTwinBoss = true;
            if (!guardedBossTarget)
                guardedBossTarget = candidate;
        }

        if (IsTwinVeklorTarget(candidate))
            targetsVeklor = true;

        if (IsTwinBugTarget(candidate))
            targetsTwinBug = true;
    }
    bool const suppressTrueCasterVeknilashPressure =
        activeTwin && guardedBossTarget && IsTwinVeknilashTarget(guardedBossTarget) &&
        IsTwinTrueCasterProfile(bot, botAI);
    bool const suppressAssignmentGuardedBossPressure =
        assignment && guardedBossTarget && !DoesTwinAssignmentAllowBossTarget(*state, assignment, guardedBossTarget);
    bool const suppressNonHunterVeknilashShoot =
        targetlessShootAction && guardedBossTarget && IsTwinVeknilashTarget(guardedBossTarget) &&
        bot->getClass() != CLASS_HUNTER;
    bool const twinBugPriorityRole = IsTwinBugServiceRole(assignment, bot, botAI);
    bool const needsTwinVeklorSafetyCheck =
        dynamic_cast<PetAttackAction*>(action) || twinBugPriorityRole ||
        (assignment && assignment->cohort == Aq40TwinEncounter::TwinRoleCohort::MeleeDps);
    GuidVector encounterUnits;
    bool encounterUnitsLoaded = false;
    auto const getEncounterUnits = [&]() -> GuidVector const&
    {
        if (!encounterUnitsLoaded)
        {
            encounterUnits = Aq40BossHelper::GetEncounterUnits(botAI, AI_VALUE(GuidVector, "attackers"));
            encounterUnitsLoaded = true;
        }

        return encounterUnits;
    };
    auto const getTwinVeklor = [&]() -> Unit*
    {
        for (Unit* candidate : guardedTargets)
        {
            if (IsTwinVeklorTarget(candidate))
                return candidate;
        }

        return FindTwinBoss(botAI, getEncounterUnits(), Aq40TwinEncounter::TwinBoss::Veklor);
    };
    Unit* const liveVeklor = needsTwinVeklorSafetyCheck ? getTwinVeklor() : nullptr;
    bool const meleeDpsVeklorProximityRisk =
        IsTwinMeleeDpsVeklorProximityRisk(bot, liveVeklor, assignment);
    bool const suppressBugPriorityBossPressure =
        activeTwin && guardedBossTarget && twinBugPriorityRole &&
        HasTwinBugPriorityTarget(bot, botAI, *state, assignment, getEncounterUnits(), liveVeklor);
    bool const designatedWarlockTank = assignment &&
                                       assignment->cohort == Aq40TwinEncounter::TwinRoleCohort::WarlockTank;
    bool const veklorPickupEstablished =
        Aq40TwinEncounter::IsPickupEstablished(*state, Aq40TwinEncounter::TwinBoss::Veklor);
    bool const warlockTankPrePickupWindow =
        designatedWarlockTank && !veklorPickupEstablished &&
        (prepullStage || approachTwin || activeTwin || Aq40TwinEncounter::IsAnyThreatHoldWindowActive(*state));

    if (warlockTankPrePickupWindow && targetlessShootAction)
        return 0.0f;

    if (suppressNonHunterVeknilashShoot)
        return 0.0f;

    if (warlockTankPrePickupWindow && offensiveSpellAction && actionName != "searing pain" &&
        actionName != "shadow ward" && (targetsVeklor || guardedBossTarget))
    {
        return 0.0f;
    }

    if (pendingSwapPrepVeklorWarlock && targetlessShootAction)
        return 0.0f;

    if (pendingPostTeleportVeklorWarlock && targetlessShootAction && !targetsVeklor)
        return 0.0f;

    if (dynamic_cast<PetAttackAction*>(action))
    {
        return IsTwinHunterSafePetAttackWindow(bot, *state, assignment, currentTarget, liveVeklor) ? 1.0f : 0.0f;
    }

    if (suppressReserveTankBossTargeting && targetsTwinBoss &&
        (attackOrReachAction || offensiveSpellAction))
    {
        return 0.0f;
    }

    if (meleeDpsVeklorProximityRisk && (attackOrReachAction || offensiveSpellAction))
        return 0.0f;

    if (suppressBugPriorityBossPressure && targetsTwinBoss &&
        (attackOrReachAction || offensiveSpellAction))
    {
        return 0.0f;
    }

    if (suppressTrueCasterVeknilashPressure && (attackOrReachAction || offensiveSpellAction))
        return 0.0f;

    if (suppressAssignmentGuardedBossPressure && (attackOrReachAction || offensiveSpellAction))
        return 0.0f;

    if ((suppressNonControllerWarlockVeklor || suppressUnsafeRangedVeklorThreat ||
            suppressNonControllerVeklorWindowTargeting) && targetsVeklor &&
        (attackOrReachAction || offensiveSpellAction))
    {
        return 0.0f;
    }

    if (unsafePickupWindow && targetsTwinBug &&
        (attackOrReachAction || dynamic_cast<CastSpellAction*>(action)))
    {
        return 0.0f;
    }

    if (postSwapHold && !Aq40BossHelper::IsEncounterTank(bot, bot))
    {
        if (dynamic_cast<ReachTargetAction*>(action) || dynamic_cast<CastReachTargetSpellAction*>(action))
            return 0.0f;
    }

    return 1.0f;
}

float Aq40OuroMultiplier::GetValue(Action* action)
{
    if (!action || !Aq40BossHelper::IsInAq40(bot))
        return 1.0f;

    GuidVector activeUnits = Aq40BossHelper::GetActiveCombatUnits(botAI, AI_VALUE(GuidVector, "attackers"));
    Unit* ouro = Aq40BossHelper::FindUnitByAnyName(botAI, activeUnits, { "ouro" });
    if (!ouro)
        return 1.0f;

    std::string const actionName = action->getName();
    bool const isEncounterTank = Aq40BossHelper::IsEncounterTank(bot, bot);

    // Suppress generic targeting during Ouro — encounter-specific
    // targeting handles scarabs, dirt mounds, and submerge phases.
    if (actionName == "aq40 choose target")
        return 0.0f;

    // Tank melee contact priority
    if (isEncounterTank && bot->GetDistance2d(ouro) > 8.0f)
    {
        if (actionName == "aq40 ouro hold melee contact")
            return 3.0f;

        if (!dynamic_cast<MovementAction*>(action))
            return 0.5f;
    }

    // Non-tanks in frontal arc need to get behind ASAP (Sand Blast avoidance).
    // Pattern from Sartura whirlwind multiplier: boost the avoidance action,
    // suppress competing movement.
    if (!isEncounterTank && ouro->isInFront(bot, 10.0f) && bot->GetDistance2d(ouro) <= 15.0f)
    {
        if (dynamic_cast<Aq40OuroAvoidSandBlastAction*>(action))
            return 3.5f;

        if (dynamic_cast<CombatFormationMoveAction*>(action) ||
            dynamic_cast<FollowAction*>(action) ||
            dynamic_cast<FleeAction*>(action) ||
            (dynamic_cast<MovementAction*>(action) &&
             !dynamic_cast<Aq40OuroAvoidSandBlastAction*>(action) &&
             !dynamic_cast<Aq40OuroAvoidSweepAction*>(action) &&
             !dynamic_cast<Aq40OuroAvoidSubmergeAction*>(action)))
            return 0.0f;
    }

    return 1.0f;
}

float Aq40ViscidusMultiplier::GetValue(Action* action)
{
    if (!action || !Aq40BossHelper::IsInAq40(bot))
        return 1.0f;

    GuidVector activeUnits = Aq40BossHelper::GetActiveCombatUnits(botAI, AI_VALUE(GuidVector, "attackers"));
    Unit* viscidus = Aq40BossHelper::FindUnitByAnyName(botAI, activeUnits, { "viscidus" });
    if (!viscidus)
        return 1.0f;

    bool frozen = Aq40SpellIds::HasAnyAura(botAI, viscidus,
        { Aq40SpellIds::ViscidusFreeze });
    std::string const actionName = action->getName();

    // Suppress generic targeting during Viscidus — encounter-specific
    // targeting handles globs, frost priority, shatter windows, etc.
    if (actionName == "aq40 choose target")
        return 0.0f;

    if (frozen)
    {
        if (actionName == "aq40 viscidus shatter")
            return 2.8f;
        if (actionName == "aq40 viscidus use frost")
            return 0.0f;
    }
    else
    {
        if (actionName == "aq40 viscidus use frost" && !botAI->IsHeal(bot) &&
            !Aq40BossHelper::IsEncounterTank(bot, bot))
            return 2.2f;
        if (actionName == "aq40 viscidus shatter")
            return 0.4f;
    }

    return 1.0f;
}

float Aq40CthunMultiplier::GetValue(Action* action)
{
    if (!action || !Aq40BossHelper::IsInAq40(bot))
        return 1.0f;

    GuidVector activeUnits = Aq40BossHelper::GetActiveCombatUnits(botAI, AI_VALUE(GuidVector, "attackers"));
    if (!Aq40BossHelper::HasAnyNamedUnit(botAI, activeUnits,
                                         { "c'thun", "eye of c'thun", "eye tentacle", "claw tentacle",
                                           "giant eye tentacle", "giant claw tentacle", "flesh tentacle" }))
        return 1.0f;

    GuidVector encounterUnits = Aq40BossHelper::GetEncounterUnits(botAI, AI_VALUE(GuidVector, "attackers"));

    std::string const actionName = action->getName();
    bool isCthunControlAction =
        actionName == "aq40 cthun choose target" ||
        actionName == "aq40 cthun avoid dark glare" ||
        actionName == "aq40 cthun stomach dps" ||
        actionName == "aq40 cthun stomach exit" ||
        actionName == "aq40 cthun phase2 add priority" ||
        actionName == "aq40 cthun vulnerable burst" ||
        actionName == "aq40 cthun interrupt eye";

    if (isCthunControlAction)
        return 1.0f;

    // Suppress generic targeting during C'Thun — encounter-specific
    // targeting handles add priority, stomach, vulnerable burst, etc.
    if (actionName == "aq40 choose target")
        return 0.0f;

    bool const inStomach = Aq40Helpers::IsCthunInStomach(bot, botAI);

    // Spread is an outside-room action; suppress it for stomach bots so
    // they stay locked to flesh-tentacle/exit behavior.
    if (actionName == "aq40 cthun maintain spread")
        return inStomach ? 0.0f : 1.0f;
    bool const darkGlare = [&]()
    {
        if (inStomach)
            return false;

        Unit* eye = Aq40BossHelper::FindUnitByAnyName(botAI, encounterUnits, { "eye of c'thun" });
        if (!eye)
            return false;

        Spell* spell = eye->GetCurrentSpell(CURRENT_GENERIC_SPELL);
        return (spell && Aq40SpellIds::MatchesAnySpellId(spell->GetSpellInfo(), { Aq40SpellIds::CthunDarkGlare })) ||
               Aq40SpellIds::HasAnyAura(botAI, eye, { Aq40SpellIds::CthunDarkGlare }) ||
               botAI->HasAura("dark glare", eye);
    }();
    bool const vulnerable = Aq40Helpers::IsCthunVulnerableNow(botAI, encounterUnits);

    if (inStomach)
    {
        if (dynamic_cast<Aq40CthunStomachExitAction*>(action))
        {
            Aura* acid = Aq40SpellIds::GetAnyAura(bot, { Aq40SpellIds::CthunDigestiveAcid });
            if (!acid)
                acid = botAI->GetAura("digestive acid", bot, false, true);

            uint32 exitStacks = 10;
            if (Aq40BossHelper::IsEncounterPrimaryTank(bot, bot))
                exitStacks = 1;
            else if (botAI->IsHeal(bot))
                exitStacks = 5;

            if (acid && acid->GetStackAmount() >= exitStacks)
                return 4.0f;
        }

        if (dynamic_cast<Aq40CthunStomachDpsAction*>(action))
            return 3.0f;

        if (dynamic_cast<MovementAction*>(action) &&
            !dynamic_cast<Aq40CthunStomachExitAction*>(action))
            return 0.0f;
    }

    if (darkGlare)
    {
        if (dynamic_cast<Aq40CthunAvoidDarkGlareAction*>(action))
            return 4.0f;

        if (dynamic_cast<CastReachTargetSpellAction*>(action) ||
            (dynamic_cast<MovementAction*>(action) &&
             !dynamic_cast<Aq40CthunAvoidDarkGlareAction*>(action)))
            return 0.0f;
    }

    if (!inStomach && !darkGlare && !vulnerable &&
        !Aq40BossHelper::IsEncounterTank(bot, bot))
    {
        if (dynamic_cast<Aq40CthunMaintainSpreadAction*>(action))
            return 2.5f;

        if (dynamic_cast<CombatFormationMoveAction*>(action) ||
            dynamic_cast<FollowAction*>(action) ||
            dynamic_cast<FleeAction*>(action) ||
            (dynamic_cast<MovementAction*>(action) &&
             !dynamic_cast<Aq40CthunMaintainSpreadAction*>(action)))
            return 0.0f;
    }

    if (dynamic_cast<CombatFormationMoveAction*>(action) ||
        dynamic_cast<FollowAction*>(action) ||
        dynamic_cast<FleeAction*>(action))
        return 0.0f;

    return 1.0f;
}
