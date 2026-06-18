#include "RaidAq40Multipliers.h"

#include <array>
#include <sstream>

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
        return false;

    Aq40TwinEncounter::TwinEncounterState const* state = Aq40TwinEncounter::GetEncounterState(bot);
    if (!state)
        return false;

    bool const assignedParticipant = Aq40TwinEncounter::IsTwinAssignedParticipant(*state, bot);
    bool const hasLockedPickupAnchor = Aq40TwinEncounter::HasActiveLockedPickupAnchor(bot);
    bool const approachTwin = Aq40TwinEncounter::IsTwinApproachWindow(*state, bot);
    bool const prepullStage = Aq40TwinEncounter::IsTwinPrePullStageWindow(*state, bot);
    bool const activeTwin = assignedParticipant && Aq40TwinEncounter::IsActivePhase(state->phase) &&
                            !Aq40TwinEncounter::IsTerminalPhase(state->phase);
    bool const postSwapHold = !Aq40TwinEncounter::IsTerminalPhase(state->phase) &&
                              (hasLockedPickupAnchor ||
                               (assignedParticipant && Aq40TwinEncounter::IsAnyThreatHoldWindowActive(*state)));
    bool const terminalTwin = Aq40TwinEncounter::IsTerminalPhase(state->phase) &&
                              (hasLockedPickupAnchor || assignedParticipant);
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

bool IsTwinHunterSafePetAttackWindow(Player* bot, Aq40TwinEncounter::TwinEncounterState const& state,
                                     Aq40TwinEncounter::TwinRoleAssignment const* assignment,
                                     Unit const* currentTarget)
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
    return assignment->stableSide == targetSide &&
           GetTwinSideForPosition(bot->GetPositionX(), bot->GetPositionY()) == targetSide;
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
    if (!assignment && !hasLockedPickupAnchor)
        return 1.0f;

    bool const assignedParticipant = Aq40TwinEncounter::IsTwinAssignedParticipant(*state, bot);
    bool const approachTwin = Aq40TwinEncounter::IsTwinApproachWindow(*state, bot);
    bool const prepullStage = Aq40TwinEncounter::IsTwinPrePullStageWindow(*state, bot);
    bool const activeTwin = assignedParticipant && Aq40TwinEncounter::IsActivePhase(state->phase) &&
                            !Aq40TwinEncounter::IsTerminalPhase(state->phase);
    bool const terminalTwin = Aq40TwinEncounter::IsTerminalPhase(state->phase) &&
                              (hasLockedPickupAnchor || assignedParticipant);
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
        if (isTwinAction || actionName == "aq40 choose target")
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

    if (dynamic_cast<PetAttackAction*>(action))
    {
        Unit* const petTarget = AI_VALUE(Unit*, "current target");
        return IsTwinHunterSafePetAttackWindow(bot, *state, assignment, petTarget) ? 1.0f : 0.0f;
    }

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
    bool const usesCurrentTarget = attackOrReachAction || offensiveSpellAction;
    bool const targetlessShootAction = actionName == "shoot";
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
    bool const suppressAssignmentGuardedBossPressure =
        assignment && guardedBossTarget && !DoesTwinAssignmentAllowBossTarget(*state, assignment, guardedBossTarget);

    if (pendingSwapPrepVeklorWarlock && targetlessShootAction)
        return 0.0f;

    if (pendingPostTeleportVeklorWarlock && targetlessShootAction && !targetsVeklor)
        return 0.0f;

    if (suppressReserveTankBossTargeting && targetsTwinBoss &&
        (attackOrReachAction || offensiveSpellAction))
    {
        return 0.0f;
    }

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
