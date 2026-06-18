#include "RaidAq40TwinEncounter.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <list>
#include <sstream>
#include <unordered_map>

#include "CharmInfo.h"
#include "Pet.h"
#include "SpellInfo.h"
#include "SpellMgr.h"

#include "../RaidAq40BossHelper.h"
#include "RaidAq40Helpers_Shared.h"
#include "Playerbots.h"
#include "Timer.h"

namespace Aq40TwinEncounter
{
namespace
{
float constexpr kPi = 3.14159265358979323846f;
float constexpr kRoomCenterX = -8954.043f;
float constexpr kRoomCenterY = 1236.379f;
float constexpr kRoomCenterZ = -112.619f;
float constexpr kBossParkSide0X = -9027.082f;
float constexpr kBossParkSide0Y = 1260.673f;
float constexpr kBossParkSide0Z = -112.295f;
float constexpr kBossParkSide1X = -8895.334f;
float constexpr kBossParkSide1Y = 1284.836f;
float constexpr kBossParkSide1Z = -112.293f;
float constexpr kSidePrepSide0X = -9000.572f;
float constexpr kSidePrepSide0Y = 1196.334f;
float constexpr kSidePrepSide0Z = -112.304f;
float constexpr kSidePrepSide1X = -8897.111f;
float constexpr kSidePrepSide1Y = 1215.582f;
float constexpr kSidePrepSide1Z = -112.306f;
uint32 constexpr kInitialVeklorSideIndex = 1u;
uint32 constexpr kInitialVeknilashSideIndex = 0u;
float constexpr kStableVeklorWarlockDistance = 23.0f;
float constexpr kReserveMeleeProxyDistance = 8.0f;
float constexpr kReserveWarlockPrepDistance = 8.0f;
float constexpr kSideHealerTowardBossParkDistance = 14.0f;
float constexpr kSideHealerLateralDistance = 8.0f;
float constexpr kTwinRoomReadyRadius = 180.0f;
float constexpr kTwinRoomExtendedRadius = 220.0f;
float constexpr kTwinRoomApproachRadius = kTwinRoomExtendedRadius;
float constexpr kTwinCenterCommitRadius = 32.0f;
uint32 constexpr kTwinTeleportCadenceEarliestMs = 30000;
uint32 constexpr kTwinTeleportCadenceLatestMs = 35000;
uint32 constexpr kTwinSwapPrepLeadMs = 5000;
uint32 constexpr kTwinClosestTargetGrantDelayMs = 1000;
size_t constexpr kTwinRequiredWarlockTanks = 2u;
size_t constexpr kTwinRequiredMeleeTanks = 2u;
size_t constexpr kTwinRequiredSideHealers = 2u;
size_t constexpr kTwinRequiredRaidHealers = 1u;
size_t constexpr kTwinRequiredHunters = 1u;
size_t constexpr kTwinRequiredRangedDps = 1u;
size_t constexpr kTwinRequiredMeleeDps = 1u;

struct TwinAssignmentBuildResult
{
    std::vector<TwinRoleAssignment> assignments;
    std::string unsupportedReason;
    std::array<ObjectGuid, 2> warlockTankBySide = { ObjectGuid::Empty, ObjectGuid::Empty };
    std::array<ObjectGuid, 2> meleeTankBySide = { ObjectGuid::Empty, ObjectGuid::Empty };
    std::array<ObjectGuid, 2> sideHealerBySide = { ObjectGuid::Empty, ObjectGuid::Empty };
    size_t approachCount = 0u;
    size_t stagedCount = 0u;
    size_t raidHealerCount = 0u;
    size_t hunterCount = 0u;
    size_t rangedCount = 0u;
    size_t meleeCount = 0u;
};

struct TwinManagedWarlockTankOverlay
{
    uint32 instanceId = 0;
    bool addedByTwin = false;
};

std::unordered_map<uint32, TwinEncounterState> sTwinStateByInstance;
std::unordered_map<uint64, TwinLockedPickupAnchor> sTwinPickupAnchorByBot;
std::unordered_map<uint64, TwinManagedWarlockTankOverlay> sTwinWarlockTankOverlayByBot;
std::unordered_map<uint64, TwinPetControlState> sTwinPetControlByBot;
std::unordered_map<uint64, uint32> sTwinLocalCleanupByBot;

size_t ToIndex(TwinBoss boss)
{
    return boss == TwinBoss::Veknilash ? 1u : 0u;
}

size_t ToSideIndex(TwinSide side)
{
    return side == TwinSide::Side1 ? 1u : 0u;
}

uint32 ResolveNow(uint32 nowMs)
{
    return nowMs ? nowMs : getMSTime();
}

uint32 GetElapsedSince(uint32 startMs, uint32 nowMs)
{
    return startMs ? getMSTimeDiff(startMs, nowMs) : 0;
}

bool IsActiveUntil(uint32 expiresAtMs, uint32 nowMs)
{
    return expiresAtMs && nowMs < expiresAtMs;
}

bool HasMeaningfulOwnership(TwinStableOwnership const& ownership)
{
    return !ownership.expectedOwner.IsEmpty() || !ownership.reserveOwner.IsEmpty() ||
           !ownership.candidateOwner.IsEmpty() || !ownership.stableOwner.IsEmpty() || ownership.stableSinceMs ||
           ownership.reservePromotionUsed || ownership.lastValidConfirmationMs;
}

bool HasOnlyPrePullAssignmentOwnership(TwinStableOwnership const& ownership)
{
    return ownership.candidateOwner.IsEmpty() && ownership.stableOwner.IsEmpty() && !ownership.stableSinceMs &&
           !ownership.reservePromotionUsed && !ownership.lastValidConfirmationMs;
}

bool HasMeaningfulRecovery(TwinBossRecoveryState const& recovery)
{
    return recovery.threatHoldUntilMs || recovery.pickupEstablished || !recovery.pickupOwner.IsEmpty() ||
           recovery.pickupEstablishedAtMs || recovery.pickupLostAtMs;
}

bool HasMeaningfulHazards(TwinScriptedHazardWindows const& hazards)
{
    return hazards.teleportAtMs || hazards.blizzardAtMs || hazards.arcaneBurstAtMs || hazards.healBrotherAtMs ||
           hazards.explodeBugAtMs || !hazards.explodeBugSourceGuid.IsEmpty() || hazards.mutateBugAtMs || hazards.uppercutAtMs ||
           hazards.unbalancingStrikeAtMs;
}

bool IsSamePosition(Position const& left, Position const& right)
{
    return std::fabs(left.GetPositionX() - right.GetPositionX()) < 0.01f &&
           std::fabs(left.GetPositionY() - right.GetPositionY()) < 0.01f &&
           std::fabs(left.GetPositionZ() - right.GetPositionZ()) < 0.01f;
}

bool IsPetSpellAutocastEnabled(Pet const* pet, uint32 spellId)
{
    if (!pet)
        return false;

    return std::find(pet->m_autospells.begin(), pet->m_autospells.end(), spellId) != pet->m_autospells.end();
}

bool HasMeaningfulCadence(TwinEncounterState const& state)
{
    return state.lastTeleportAtMs || state.nextTeleportEarliestAtMs || state.nextTeleportLatestAtMs ||
           state.swapPrepStartAtMs || state.swapPrepArmedAtMs ||
           state.closestTargetGrantDelayMs != kTwinClosestTargetGrantDelayMs;
}

bool HasOnlyPrePullAssignmentScaffold(TwinEncounterState const& state)
{
    if (state.phase != TwinEncounterPhase::PrePull ||
        (state.mode != TwinStrategyMode::Inactive && state.mode != TwinStrategyMode::StandardCompReady) ||
        state.recovery.splitBand != TwinSplitBand::Stable || state.recovery.splitBandEnteredAtMs ||
        HasMeaningfulHazards(state.scriptedHazards) || HasMeaningfulCadence(state))
    {
        return false;
    }

    for (size_t index = 0; index < state.ownership.size(); ++index)
    {
        if (!HasOnlyPrePullAssignmentOwnership(state.ownership[index]) ||
            HasMeaningfulRecovery(state.recovery.boss[index]))
        {
            return false;
        }
    }

    return true;
}

void ResetTeleportCadence(TwinEncounterState& state)
{
    state.lastTeleportAtMs = 0;
    state.nextTeleportEarliestAtMs = 0;
    state.nextTeleportLatestAtMs = 0;
    state.swapPrepStartAtMs = 0;
    state.closestTargetGrantDelayMs = kTwinClosestTargetGrantDelayMs;
    state.swapPrepArmedAtMs = 0;
}

void ScheduleNextTeleportCadence(TwinEncounterState& state, uint32 referenceMs)
{
    state.nextTeleportEarliestAtMs = referenceMs + kTwinTeleportCadenceEarliestMs;
    state.nextTeleportLatestAtMs = referenceMs + kTwinTeleportCadenceLatestMs;
    state.swapPrepStartAtMs = state.nextTeleportEarliestAtMs > kTwinSwapPrepLeadMs
        ? state.nextTeleportEarliestAtMs - kTwinSwapPrepLeadMs
        : referenceMs;
    state.swapPrepArmedAtMs = 0;
    if (!state.closestTargetGrantDelayMs)
        state.closestTargetGrantDelayMs = kTwinClosestTargetGrantDelayMs;
}

bool HasMeaningfulEncounterState(TwinEncounterState const& state)
{
    if (HasOnlyPrePullAssignmentScaffold(state))
        return false;

    if ((state.mode != TwinStrategyMode::Inactive && state.mode != TwinStrategyMode::StandardCompReady) ||
        state.phase != TwinEncounterPhase::PrePull ||
        state.recovery.splitBand != TwinSplitBand::Stable || state.recovery.splitBandEnteredAtMs ||
        HasMeaningfulHazards(state.scriptedHazards) || HasMeaningfulCadence(state))
    {
        return true;
    }

    for (TwinBoss boss : { TwinBoss::Veklor, TwinBoss::Veknilash })
    {
        if (HasMeaningfulOwnership(GetOwnership(state, boss)) || HasMeaningfulRecovery(GetRecoveryState(state, boss)))
            return true;
    }

    return false;
}

uint64 GetBotKey(Player const* bot)
{
    return bot ? bot->GetGUID().GetRawValue() : 0;
}

bool IsNearTwinRoom(Player const* bot, float radius)
{
    if (!bot || !bot->IsInWorld() || !Aq40BossHelper::IsInAq40(bot))
        return false;

    Position const& center = GetGeometry().roomCenter.position;
    return const_cast<Player*>(bot)->GetExactDist2d(center.GetPositionX(), center.GetPositionY()) <= radius;
}

bool IsNearTwinApproach(Player const* bot)
{
    return IsNearTwinRoom(bot, kTwinRoomApproachRadius);
}

std::string BuildUnsupportedReason(char const* cohortToken, size_t requiredCount, size_t availableCount)
{
    std::ostringstream out;
    out << "missing_" << cohortToken << "_need_" << requiredCount << "_have_" << availableCount;
    return out.str();
}

bool AreAssignmentsEqual(std::vector<TwinRoleAssignment> const& left, std::vector<TwinRoleAssignment> const& right)
{
    if (left.size() != right.size())
        return false;

    for (size_t index = 0; index < left.size(); ++index)
    {
        TwinRoleAssignment const& leftAssignment = left[index];
        TwinRoleAssignment const& rightAssignment = right[index];
        if (leftAssignment.memberGuid != rightAssignment.memberGuid ||
            leftAssignment.cohort != rightAssignment.cohort ||
            leftAssignment.stableSide != rightAssignment.stableSide ||
            leftAssignment.slotIndex != rightAssignment.slotIndex)
        {
            return false;
        }
    }

    return true;
}

void SortMembersByGuid(std::vector<Player*>& members)
{
    std::stable_sort(members.begin(), members.end(), [](Player* left, Player* right)
    {
        return left->GetGUID().GetRawValue() < right->GetGUID().GetRawValue();
    });
}

uint32 GetTankAssignmentPriority(Player* member, Group const* group)
{
    if (!member)
        return 99u;

    if (PlayerbotAI::IsMainTank(member))
        return 0u;
    if (PlayerbotAI::IsAssistTankOfIndex(member, 0, true))
        return 1u;
    if (PlayerbotAI::IsAssistTankOfIndex(member, 1, true))
        return 2u;
    if (group && group->IsAssistant(member->GetGUID()))
        return 3u;
    return 10u;
}

template <typename PriorityFn>
void SortMembersByPriority(std::vector<Player*>& members, PriorityFn&& priorityFn)
{
    std::stable_sort(members.begin(), members.end(), [&](Player* left, Player* right)
    {
        uint32 const leftPriority = priorityFn(left);
        uint32 const rightPriority = priorityFn(right);
        if (leftPriority != rightPriority)
            return leftPriority < rightPriority;

        return left->GetGUID().GetRawValue() < right->GetGUID().GetRawValue();
    });
}

std::vector<Player*> CollectTwinMembers(Player* bot, float radius)
{
    std::vector<Player*> members;
    if (!bot || !Aq40BossHelper::IsInAq40(bot))
        return members;

    Group* group = bot->GetGroup();
    if (!group)
        return members;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || !member->IsInWorld())
            continue;
        if (!Aq40BossHelper::IsSameInstance(bot, member))
            continue;
        if (!IsNearTwinRoom(member, radius))
            continue;

        members.push_back(member);
    }

    return members;
}

std::vector<Player*> CollectTwinApproachMembers(Player* bot)
{
    return CollectTwinMembers(bot, kTwinRoomApproachRadius);
}

std::vector<Player*> CollectTwinStagedMembers(Player* bot)
{
    return CollectTwinMembers(bot, kTwinRoomReadyRadius);
}

bool AnyTwinMemberInCombat(std::vector<Player*> const& members)
{
    return std::any_of(members.begin(), members.end(), [](Player* member)
    {
        return member && member->IsInCombat();
    });
}

bool IsReadyPrePullState(TwinEncounterState const& state)
{
    return state.phase == TwinEncounterPhase::PrePull &&
           state.mode == TwinStrategyMode::StandardCompReady &&
           HasDeterministicAssignments(state);
}

bool HasAssignedMemberInCombat(Player* bot, TwinEncounterState const& state)
{
    if (!bot || state.assignments.empty())
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsInWorld() || !Aq40BossHelper::IsSameInstance(bot, member))
            continue;

        if (!GetAssignmentForMember(state, member->GetGUID()))
            continue;

        if (member->IsInCombat())
            return true;
    }

    return false;
}

bool ClearWarlockTankOverlaysForInstance(Player* bot)
{
    if (!bot || !bot->GetMap())
        return false;

    uint32 const instanceId = GetInstanceId(bot);
    if (!instanceId)
        return false;

    bool cleared = false;
    Map::PlayerList const& players = bot->GetMap()->GetPlayers();
    for (Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
    {
        Player* member = itr->GetSource();
        if (!member || !member->IsInWorld() || GetInstanceId(member) != instanceId)
            continue;

        uint64 const botKey = GetBotKey(member);
        auto const overlayItr = sTwinWarlockTankOverlayByBot.find(botKey);
        if (overlayItr == sTwinWarlockTankOverlayByBot.end() || overlayItr->second.instanceId != instanceId)
            continue;

        ClearTwinWarlockTankStrategy(member);
        Aq40Helpers::LogAq40Info(member, "twin_strategy",
            "twin:warlock_tank_overlay:instance_reset",
            "boss=twin strategy=tank action=instance_reset", 1000);
        cleared = true;
    }

    for (auto itr = sTwinWarlockTankOverlayByBot.begin(); itr != sTwinWarlockTankOverlayByBot.end();)
    {
        if (itr->second.instanceId == instanceId)
        {
            itr = sTwinWarlockTankOverlayByBot.erase(itr);
            cleared = true;
            continue;
        }

        ++itr;
    }

    return cleared;
}

bool ClearPetControlStateForInstance(Player* bot)
{
    if (!bot || !bot->GetMap())
        return false;

    uint32 const instanceId = GetInstanceId(bot);
    if (!instanceId)
        return false;

    bool cleared = false;
    Map::PlayerList const& players = bot->GetMap()->GetPlayers();
    for (Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
    {
        Player* member = itr->GetSource();
        if (!member || !member->IsInWorld() || GetInstanceId(member) != instanceId)
            continue;

        uint64 const botKey = GetBotKey(member);
        auto const controlItr = sTwinPetControlByBot.find(botKey);
        if (controlItr == sTwinPetControlByBot.end() || controlItr->second.instanceId != instanceId)
            continue;

        RestorePetControlState(member);
        cleared = true;
    }

    for (auto itr = sTwinPetControlByBot.begin(); itr != sTwinPetControlByBot.end();)
    {
        if (itr->second.instanceId == instanceId)
        {
            itr = sTwinPetControlByBot.erase(itr);
            cleared = true;
            continue;
        }

        ++itr;
    }

    return cleared;
}

void PushAssignment(std::vector<TwinRoleAssignment>& assignments, Player* member, TwinRoleCohort cohort,
                    TwinSide side, uint8 slotIndex)
{
    if (!member)
        return;

    TwinRoleAssignment assignment;
    assignment.memberGuid = member->GetGUID();
    assignment.cohort = cohort;
    assignment.stableSide = side;
    assignment.slotIndex = slotIndex;
    assignments.push_back(assignment);
}

void AssignBalancedSideCohort(std::vector<Player*> const& members, TwinRoleCohort cohort,
                              std::array<size_t, 2>& sideLoad,
                              std::vector<TwinRoleAssignment>& assignments)
{
    uint8 slotIndex = 0;
    for (Player* member : members)
    {
        TwinSide const side = sideLoad[0] <= sideLoad[1] ? TwinSide::Side0 : TwinSide::Side1;
        PushAssignment(assignments, member, cohort, side, slotIndex++);
        ++sideLoad[ToSideIndex(side)];
    }
}

void ConfigurePrePullOwnership(TwinEncounterState& state, TwinAssignmentBuildResult const& buildResult)
{
    state.ownership[ToIndex(TwinBoss::Veklor)] = TwinStableOwnership();
    state.ownership[ToIndex(TwinBoss::Veknilash)] = TwinStableOwnership();

    TwinStableOwnership& veklorOwnership = state.ownership[ToIndex(TwinBoss::Veklor)];
    veklorOwnership.expectedOwner = buildResult.warlockTankBySide[ToSideIndex(GetInitialSideForBoss(TwinBoss::Veklor))];
    veklorOwnership.reserveOwner =
        buildResult.warlockTankBySide[ToSideIndex(GetOppositeSide(GetInitialSideForBoss(TwinBoss::Veklor)))];

    TwinStableOwnership& veknilashOwnership = state.ownership[ToIndex(TwinBoss::Veknilash)];
    veknilashOwnership.expectedOwner =
        buildResult.meleeTankBySide[ToSideIndex(GetInitialSideForBoss(TwinBoss::Veknilash))];
    veknilashOwnership.reserveOwner =
        buildResult.meleeTankBySide[ToSideIndex(GetOppositeSide(GetInitialSideForBoss(TwinBoss::Veknilash)))];

    state.recovery = TwinRecoveryState();
    state.scriptedHazards = TwinScriptedHazardWindows();
    ResetTeleportCadence(state);
}

void ClearPrePullAssignments(TwinEncounterState& state, uint32 nowMs, bool clearReason)
{
    if (state.phase != TwinEncounterPhase::PrePull)
        return;

    state.assignments.clear();
    state.centerCommittedMemberCount = 0;
    state.strictReadyMemberCount = 0;
    state.ownership[ToIndex(TwinBoss::Veklor)] = TwinStableOwnership();
    state.ownership[ToIndex(TwinBoss::Veknilash)] = TwinStableOwnership();
    state.recovery = TwinRecoveryState();
    state.scriptedHazards = TwinScriptedHazardWindows();
    ResetTeleportCadence(state);
    if (clearReason)
        state.unsupportedReason.clear();

    SetMode(state, TwinStrategyMode::Inactive, nowMs);
}

Player* ChooseTwinLogBot(Player* fallbackBot, std::vector<Player*> const& stagedMembers)
{
    if (fallbackBot && GET_PLAYERBOT_AI(fallbackBot) && IsNearTwinApproach(fallbackBot))
        return fallbackBot;

    for (Player* member : stagedMembers)
    {
        if (member && GET_PLAYERBOT_AI(member))
            return member;
    }

    return fallbackBot;
}

Player* FindTwinStagedMember(std::vector<Player*> const& stagedMembers, ObjectGuid guid)
{
    if (guid.IsEmpty())
        return nullptr;

    auto const itr = std::find_if(stagedMembers.begin(), stagedMembers.end(), [guid](Player* member)
    {
        return member && member->GetGUID() == guid;
    });

    return itr != stagedMembers.end() ? *itr : nullptr;
}

size_t CountAssignedMembersNearTwinCenter(std::vector<Player*> const& members,
                                          std::vector<TwinRoleAssignment> const& assignments)
{
    Position const& roomCenter = GetGeometry().roomCenter.position;
    size_t committedCount = 0u;
    for (TwinRoleAssignment const& assignment : assignments)
    {
        Player* member = FindTwinStagedMember(members, assignment.memberGuid);
        if (!member)
            continue;

        if (member->GetExactDist2d(roomCenter.GetPositionX(), roomCenter.GetPositionY()) <= kTwinCenterCommitRadius)
            ++committedCount;
    }

    return committedCount;
}

std::string BuildTwinShadowResistanceUnsupportedReason(std::vector<Player*> const& stagedMembers,
                                                       std::vector<TwinRoleAssignment> const& assignments)
{
    size_t priestCount = 0u;
    for (Player* member : stagedMembers)
    {
        if (member && member->getClass() == CLASS_PRIEST)
            ++priestCount;
    }

    if (priestCount > 0u)
        return {};

    std::array<bool, 2> paladinSideCoverage = { false, false };
    for (TwinRoleAssignment const& assignment : assignments)
    {
        if (!IsKnownSide(assignment.stableSide))
            continue;

        Player* member = FindTwinStagedMember(stagedMembers, assignment.memberGuid);
        if (!member || member->getClass() != CLASS_PALADIN)
            continue;

        paladinSideCoverage[ToSideIndex(assignment.stableSide)] = true;
    }

    if (paladinSideCoverage[0] && paladinSideCoverage[1])
        return {};

    size_t const sidePaladinCount = (paladinSideCoverage[0] ? 1u : 0u) + (paladinSideCoverage[1] ? 1u : 0u);
    std::ostringstream out;
    out << "missing_shadow_resistance_support_need_1_priest_or_2_side_paladins"
        << "_have_priests_" << priestCount
        << "_have_side_paladins_" << sidePaladinCount;
    return out.str();
}

void LogTwinMemberScan(Player* logBot, size_t approachCount, size_t stagedCount)
{
    if (!logBot)
        return;

    std::ostringstream fields;
    fields << "boss=twin state=member_scan"
           << " approach=" << approachCount
           << " staged=" << stagedCount;
    Aq40Helpers::LogAq40Info(logBot, "twin_prepull",
        "twin:member_scan:approach_" + std::to_string(approachCount) +
            "_staged_" + std::to_string(stagedCount),
        fields.str(), 1000);
}

TwinAssignmentBuildResult BuildTwinAssignments(Player* bot, std::vector<Player*> const& approachMembers,
                                               std::vector<Player*> const& stagedMembers)
{
    TwinAssignmentBuildResult buildResult;
    PlayerbotAI* referenceAI = GET_PLAYERBOT_AI(bot);
    if (!bot || !referenceAI)
        return buildResult;

    Group* group = bot->GetGroup();
    if (!group)
        return buildResult;

    buildResult.approachCount = approachMembers.size();
    buildResult.stagedCount = stagedMembers.size();
    if (approachMembers.empty() || AnyTwinMemberInCombat(approachMembers))
        return buildResult;

    std::vector<Player*> warlockCandidates;
    std::vector<Player*> meleeTankCandidates;
    std::vector<Player*> healerCandidates;
    std::vector<Player*> hunterCandidates;
    std::vector<Player*> rangedDpsCandidates;
    std::vector<Player*> meleeDpsCandidates;

    for (Player* member : approachMembers)
    {
        bool const isHealer = referenceAI->IsHeal(member);
        bool const isTank = PlayerbotAI::IsTank(member, true);
        bool const isRanged = PlayerbotAI::IsRanged(member);

        if (member->getClass() == CLASS_WARLOCK)
        {
            warlockCandidates.push_back(member);
            continue;
        }

        if (isTank)
        {
            meleeTankCandidates.push_back(member);
            continue;
        }

        if (isHealer)
        {
            healerCandidates.push_back(member);
            continue;
        }

        if (member->getClass() == CLASS_HUNTER)
        {
            hunterCandidates.push_back(member);
            continue;
        }

        if (isRanged)
        {
            rangedDpsCandidates.push_back(member);
            continue;
        }

        meleeDpsCandidates.push_back(member);
    }

    SortMembersByPriority(warlockCandidates, [&](Player* member)
    {
        return GetTankAssignmentPriority(member, group);
    });
    SortMembersByPriority(meleeTankCandidates, [&](Player* member)
    {
        return GetTankAssignmentPriority(member, group);
    });
    SortMembersByGuid(healerCandidates);
    SortMembersByGuid(hunterCandidates);
    SortMembersByGuid(rangedDpsCandidates);
    SortMembersByGuid(meleeDpsCandidates);

    if (warlockCandidates.size() < kTwinRequiredWarlockTanks)
    {
        buildResult.unsupportedReason =
            BuildUnsupportedReason("warlock_tanks", kTwinRequiredWarlockTanks, warlockCandidates.size());
        return buildResult;
    }

    if (meleeTankCandidates.size() < kTwinRequiredMeleeTanks)
    {
        buildResult.unsupportedReason =
            BuildUnsupportedReason("melee_tanks", kTwinRequiredMeleeTanks, meleeTankCandidates.size());
        return buildResult;
    }

    if (healerCandidates.size() < kTwinRequiredSideHealers)
    {
        buildResult.unsupportedReason =
            BuildUnsupportedReason("side_healers", kTwinRequiredSideHealers, healerCandidates.size());
        return buildResult;
    }

    size_t const raidHealerCount = healerCandidates.size() - kTwinRequiredSideHealers;
    if (raidHealerCount < kTwinRequiredRaidHealers)
    {
        buildResult.unsupportedReason =
            BuildUnsupportedReason("raid_healers", kTwinRequiredRaidHealers, raidHealerCount);
        return buildResult;
    }

    rangedDpsCandidates.insert(rangedDpsCandidates.end(),
        warlockCandidates.begin() + kTwinRequiredWarlockTanks, warlockCandidates.end());
    meleeDpsCandidates.insert(meleeDpsCandidates.end(),
        meleeTankCandidates.begin() + kTwinRequiredMeleeTanks, meleeTankCandidates.end());

    SortMembersByGuid(rangedDpsCandidates);
    SortMembersByGuid(meleeDpsCandidates);

    if (rangedDpsCandidates.size() < kTwinRequiredRangedDps)
    {
        buildResult.unsupportedReason =
            BuildUnsupportedReason("ranged_dps", kTwinRequiredRangedDps, rangedDpsCandidates.size());
        return buildResult;
    }

    if (hunterCandidates.size() < kTwinRequiredHunters)
    {
        buildResult.unsupportedReason =
            BuildUnsupportedReason("hunters", kTwinRequiredHunters, hunterCandidates.size());
        return buildResult;
    }

    if (meleeDpsCandidates.size() < kTwinRequiredMeleeDps)
    {
        buildResult.unsupportedReason =
            BuildUnsupportedReason("melee_dps", kTwinRequiredMeleeDps, meleeDpsCandidates.size());
        return buildResult;
    }

    std::array<size_t, 2> sideLoad = { 0u, 0u };

    buildResult.warlockTankBySide[0] = warlockCandidates[0]->GetGUID();
    buildResult.warlockTankBySide[1] = warlockCandidates[1]->GetGUID();
    buildResult.meleeTankBySide[0] = meleeTankCandidates[0]->GetGUID();
    buildResult.meleeTankBySide[1] = meleeTankCandidates[1]->GetGUID();
    buildResult.sideHealerBySide[0] = healerCandidates[0]->GetGUID();
    buildResult.sideHealerBySide[1] = healerCandidates[1]->GetGUID();

    PushAssignment(buildResult.assignments, warlockCandidates[0], TwinRoleCohort::WarlockTank, TwinSide::Side0, 0);
    PushAssignment(buildResult.assignments, warlockCandidates[1], TwinRoleCohort::WarlockTank, TwinSide::Side1, 1);
    ++sideLoad[0];
    ++sideLoad[1];

    PushAssignment(buildResult.assignments, meleeTankCandidates[0], TwinRoleCohort::MeleeTank, TwinSide::Side0, 0);
    PushAssignment(buildResult.assignments, meleeTankCandidates[1], TwinRoleCohort::MeleeTank, TwinSide::Side1, 1);
    ++sideLoad[0];
    ++sideLoad[1];

    PushAssignment(buildResult.assignments, healerCandidates[0], TwinRoleCohort::SideHealer, TwinSide::Side0, 0);
    PushAssignment(buildResult.assignments, healerCandidates[1], TwinRoleCohort::SideHealer, TwinSide::Side1, 1);
    ++sideLoad[0];
    ++sideLoad[1];

    for (size_t index = kTwinRequiredSideHealers; index < healerCandidates.size(); ++index)
        PushAssignment(buildResult.assignments, healerCandidates[index], TwinRoleCohort::RaidHealer,
                       TwinSide::Unknown, static_cast<uint8>(index - kTwinRequiredSideHealers));

    buildResult.raidHealerCount = raidHealerCount;
    buildResult.rangedCount = rangedDpsCandidates.size();
    buildResult.hunterCount = hunterCandidates.size();
    buildResult.meleeCount = meleeDpsCandidates.size();

    AssignBalancedSideCohort(rangedDpsCandidates, TwinRoleCohort::RangedDps, sideLoad, buildResult.assignments);
    AssignBalancedSideCohort(hunterCandidates, TwinRoleCohort::Hunter, sideLoad, buildResult.assignments);
    AssignBalancedSideCohort(meleeDpsCandidates, TwinRoleCohort::MeleeDps, sideLoad, buildResult.assignments);

    buildResult.unsupportedReason =
        BuildTwinShadowResistanceUnsupportedReason(approachMembers, buildResult.assignments);
    if (!buildResult.unsupportedReason.empty())
    {
        buildResult.assignments.clear();
        return buildResult;
    }

    return buildResult;
}

void RefreshPrePullAssignments(Player* bot, TwinEncounterState& state)
{
    if (!bot || state.phase != TwinEncounterPhase::PrePull)
        return;

    uint32 const now = ResolveNow(0);
    size_t const previousApproachCount = state.approachMemberCount;
    size_t const previousStagedCount = state.stagedMemberCount;
    size_t const previousCenterCommittedCount = state.centerCommittedMemberCount;
    std::vector<Player*> const approachMembers = CollectTwinApproachMembers(bot);
    std::vector<Player*> const stagedMembers = CollectTwinStagedMembers(bot);
    state.approachMemberCount = static_cast<uint16>(approachMembers.size());
    state.stagedMemberCount = static_cast<uint16>(stagedMembers.size());

    Player* logBot = ChooseTwinLogBot(bot, !approachMembers.empty() ? approachMembers : stagedMembers);
    bool const countsChanged = previousApproachCount != approachMembers.size() ||
                               previousStagedCount != stagedMembers.size();
    if (countsChanged)
        LogTwinMemberScan(logBot, approachMembers.size(), stagedMembers.size());

    bool const wasReady = IsReadyPrePullState(state);
    bool const wasCenterCommitted = IsTwinCenterCommitted(state);
    bool const preserveReadyState = wasReady && HasAssignedMemberInCombat(bot, state);

    if (preserveReadyState)
    {
        if (logBot)
        {
            std::ostringstream fields;
            fields << "boss=twin state=retain_ready reason=first_contact_pending"
                   << " assignments=" << state.assignments.size()
                   << " approach=" << state.approachMemberCount
                   << " staged=" << state.stagedMemberCount
                   << " center_committed=" << state.centerCommittedMemberCount
                   << " strict_ready=" << state.strictReadyMemberCount;
            Aq40Helpers::LogAq40Info(logBot, "twin_prepull", "twin:retain_ready:first_contact",
                                     fields.str(), 5000);
        }

        return;
    }

    if (approachMembers.empty() || AnyTwinMemberInCombat(approachMembers))
    {
        bool const hadTrackedState = wasReady || !state.assignments.empty() || !state.unsupportedReason.empty();
        std::string const clearReason = approachMembers.empty() ? "no_approach_members" : "approach_member_in_combat";
        ClearPrePullAssignments(state, now, true);

        if ((countsChanged || hadTrackedState) && logBot)
        {
            std::ostringstream fields;
            fields << "boss=twin state=inactive reason=" << clearReason
                   << " approach=" << approachMembers.size()
                   << " staged=" << stagedMembers.size();
            Aq40Helpers::LogAq40Info(logBot, "twin_prepull", "twin:inactive:" + clearReason,
                                     fields.str(), 1000);
        }

        return;
    }

    TwinAssignmentBuildResult const buildResult = BuildTwinAssignments(bot, approachMembers, stagedMembers);

    bool const assignmentsChanged = !AreAssignmentsEqual(state.assignments, buildResult.assignments);
    bool const reasonChanged = state.unsupportedReason != buildResult.unsupportedReason;

    if (!buildResult.unsupportedReason.empty())
    {
        ClearPrePullAssignments(state, now, false);
        state.unsupportedReason = buildResult.unsupportedReason;

        if ((assignmentsChanged || reasonChanged || wasReady || countsChanged) && logBot)
        {
            std::ostringstream fields;
            fields << "boss=twin state=unsupported reason=" << buildResult.unsupportedReason
                   << " approach=" << buildResult.approachCount
                   << " staged=" << buildResult.stagedCount;
            Aq40Helpers::LogAq40Warn(logBot, "twin_prepull", "twin:unsupported:" + buildResult.unsupportedReason,
                                     fields.str(), 1000);
        }

        return;
    }

    size_t const centerCommittedCount = CountAssignedMembersNearTwinCenter(approachMembers, buildResult.assignments);
    bool const centerCommitted = !buildResult.assignments.empty() &&
                                 centerCommittedCount >= buildResult.assignments.size();
    bool const centerCommitChanged = previousCenterCommittedCount != centerCommittedCount;
    bool const keepStrictReady = centerCommitted && state.mode == TwinStrategyMode::StandardCompReady &&
                                 !assignmentsChanged && !reasonChanged && !countsChanged && !centerCommitChanged;

    state.assignments = buildResult.assignments;
    state.unsupportedReason.clear();
    state.centerCommittedMemberCount = static_cast<uint16>(centerCommittedCount);
    if (!keepStrictReady)
        state.strictReadyMemberCount = 0;
    ConfigurePrePullOwnership(state, buildResult);
    SetPhase(state, TwinEncounterPhase::PrePull, now);
    if (centerCommitted)
    {
        SetMode(state, keepStrictReady ? TwinStrategyMode::StandardCompReady : TwinStrategyMode::CenterCommitted, now);

        if (!keepStrictReady && (assignmentsChanged || reasonChanged || countsChanged || centerCommitChanged ||
                                 wasReady || !wasCenterCommitted) && logBot)
        {
            std::ostringstream fields;
            fields << "boss=twin state=center_committed"
                   << " approach=" << buildResult.approachCount
                   << " staged=" << buildResult.stagedCount
                   << " center_committed=" << centerCommittedCount
                   << " assigned=" << buildResult.assignments.size()
                   << " handoff=side_owned_stage"
                   << " wait=strict_ready";
            Aq40Helpers::LogAq40Info(logBot, "twin_prepull", "twin:center_commit", fields.str(), 1000);
        }

        return;
    }

    SetMode(state, TwinStrategyMode::Inactive, now);

    if ((assignmentsChanged || reasonChanged || countsChanged || centerCommitChanged || wasReady ||
         wasCenterCommitted) && logBot)
    {
        std::ostringstream fields;
        fields << "boss=twin state=approach_pending"
               << " approach=" << buildResult.approachCount
               << " staged=" << buildResult.stagedCount
               << " center_committed=" << centerCommittedCount
               << " assigned=" << buildResult.assignments.size()
               << " authority=cleanup_only"
               << " movement=player_controlled"
               << " wait=center_commit";
        Aq40Helpers::LogAq40Info(logBot, "twin_prepull", "twin:approach_pending", fields.str(), 1000);
    }
}

TwinAnchor MakeAnchor(float x, float y, float z, float preferredRange = 0.0f, float facing = 0.0f)
{
    TwinAnchor anchor;
    anchor.position.Relocate(x, y, z);
    anchor.preferredRange = preferredRange;
    anchor.facing = facing;
    return anchor;
}

float ComputeFacing(Position const& from, Position const& to)
{
    return std::atan2(to.GetPositionY() - from.GetPositionY(), to.GetPositionX() - from.GetPositionX());
}

struct Direction2d
{
    float x = 0.0f;
    float y = 0.0f;
    float length = 0.0f;
};

Position MakePosition(float x, float y, float z)
{
    Position position;
    position.Relocate(x, y, z);
    return position;
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

Position TranslatePosition(Position const& origin, Position const& toward, float forwardDistance,
                           float lateralDistance = 0.0f)
{
    Direction2d const direction = GetDirection2d(origin, toward);
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

TwinAnchor BuildDerivedAnchor(Position const& origin, Position const& toward, float forwardDistance,
                              Position const& facingTarget, float preferredRange = 0.0f,
                              float lateralDistance = 0.0f)
{
    Position const position = TranslatePosition(origin, toward, forwardDistance, lateralDistance);
    float const range = preferredRange > 0.0f ? preferredRange : std::fabs(forwardDistance);
    return MakeAnchor(position.GetPositionX(), position.GetPositionY(), position.GetPositionZ(),
        range, ComputeFacing(position, facingTarget));
}

float ComputeOutwardFacing(Position const& from, Position const& center)
{
    return Position::NormalizeOrientation(ComputeFacing(from, center) + kPi);
}

TwinCenterSpreadSlot BuildCenterSpreadSlot(uint8 slotIndex, float radius, float angleDegrees)
{
    float const angleRadians = angleDegrees * kPi / 180.0f;
    TwinCenterSpreadSlot slot;
    slot.slotIndex = slotIndex;
    slot.anchor = MakeAnchor(kRoomCenterX + std::cos(angleRadians) * radius,
                             kRoomCenterY + std::sin(angleRadians) * radius,
                             kRoomCenterZ,
                             radius,
                             angleRadians + kPi);
    return slot;
}

TwinEncounterGeometry BuildGeometry()
{
    TwinEncounterGeometry geometry;

    Position const roomCenter = MakePosition(kRoomCenterX, kRoomCenterY, kRoomCenterZ);
    Position const bossParkSide0 = MakePosition(kBossParkSide0X, kBossParkSide0Y, kBossParkSide0Z);
    Position const bossParkSide1 = MakePosition(kBossParkSide1X, kBossParkSide1Y, kBossParkSide1Z);
    Position const sidePrepSide0 = MakePosition(kSidePrepSide0X, kSidePrepSide0Y, kSidePrepSide0Z);
    Position const sidePrepSide1 = MakePosition(kSidePrepSide1X, kSidePrepSide1Y, kSidePrepSide1Z);

    geometry.roomCenter = MakeAnchor(kRoomCenterX, kRoomCenterY, kRoomCenterZ);

    geometry.bossPark[kInitialVeknilashSideIndex] =
        MakeAnchor(kBossParkSide0X, kBossParkSide0Y, kBossParkSide0Z, 0.0f,
            ComputeOutwardFacing(bossParkSide0, roomCenter));
    geometry.bossPark[kInitialVeklorSideIndex] =
        MakeAnchor(kBossParkSide1X, kBossParkSide1Y, kBossParkSide1Z, 0.0f,
            ComputeOutwardFacing(bossParkSide1, roomCenter));

    geometry.sidePrep[kInitialVeknilashSideIndex] =
        MakeAnchor(kSidePrepSide0X, kSidePrepSide0Y, kSidePrepSide0Z, 0.0f,
            ComputeFacing(sidePrepSide0, bossParkSide0));
    geometry.sidePrep[kInitialVeklorSideIndex] =
        MakeAnchor(kSidePrepSide1X, kSidePrepSide1Y, kSidePrepSide1Z, 0.0f,
            ComputeFacing(sidePrepSide1, bossParkSide1));

    geometry.stableVeklorWarlock[kInitialVeknilashSideIndex] =
        BuildDerivedAnchor(bossParkSide0, roomCenter, kStableVeklorWarlockDistance, bossParkSide0,
            kStableVeklorWarlockDistance);
    geometry.stableVeklorWarlock[kInitialVeklorSideIndex] =
        BuildDerivedAnchor(bossParkSide1, roomCenter, kStableVeklorWarlockDistance, bossParkSide1,
            kStableVeklorWarlockDistance);

    geometry.reserveMeleeProxy[kInitialVeknilashSideIndex] =
        BuildDerivedAnchor(bossParkSide0, roomCenter, kReserveMeleeProxyDistance, bossParkSide0,
            kReserveMeleeProxyDistance);
    geometry.reserveMeleeProxy[kInitialVeklorSideIndex] =
        BuildDerivedAnchor(bossParkSide1, roomCenter, kReserveMeleeProxyDistance, bossParkSide1,
            kReserveMeleeProxyDistance);

    geometry.reserveWarlockPrep[kInitialVeknilashSideIndex] =
        BuildDerivedAnchor(bossParkSide0, sidePrepSide0, kReserveWarlockPrepDistance, bossParkSide0,
            kReserveWarlockPrepDistance);
    geometry.reserveWarlockPrep[kInitialVeklorSideIndex] =
        BuildDerivedAnchor(bossParkSide1, sidePrepSide1, kReserveWarlockPrepDistance, bossParkSide1,
            kReserveWarlockPrepDistance);

    geometry.sideHealer[kInitialVeknilashSideIndex] =
        BuildDerivedAnchor(sidePrepSide0, bossParkSide0, kSideHealerTowardBossParkDistance, bossParkSide0, 0.0f,
            -kSideHealerLateralDistance);
    geometry.sideHealer[kInitialVeklorSideIndex] =
        BuildDerivedAnchor(sidePrepSide1, bossParkSide1, kSideHealerTowardBossParkDistance, bossParkSide1, 0.0f,
            kSideHealerLateralDistance);

    geometry.centerSpread = {
        BuildCenterSpreadSlot(0, 18.0f, 20.0f),
        BuildCenterSpreadSlot(1, 26.0f, 80.0f),
        BuildCenterSpreadSlot(2, 18.0f, 140.0f),
        BuildCenterSpreadSlot(3, 26.0f, 200.0f),
        BuildCenterSpreadSlot(4, 18.0f, 260.0f),
        BuildCenterSpreadSlot(5, 26.0f, 320.0f),
    };

    return geometry;
}

TwinEncounterGeometry const sTwinGeometry = BuildGeometry();
}    // namespace

TwinBoss GetOtherBoss(TwinBoss boss)
{
    switch (boss)
    {
        case TwinBoss::Veklor: return TwinBoss::Veknilash;
        case TwinBoss::Veknilash: return TwinBoss::Veklor;
    }

    return TwinBoss::Veklor;
}

TwinSide GetInitialSideForBoss(TwinBoss boss)
{
    return boss == TwinBoss::Veklor ? TwinSide::Side1 : TwinSide::Side0;
}

TwinSide GetOppositeSide(TwinSide side)
{
    switch (side)
    {
        case TwinSide::Side0: return TwinSide::Side1;
        case TwinSide::Side1: return TwinSide::Side0;
        case TwinSide::Unknown: return TwinSide::Unknown;
    }

    return TwinSide::Unknown;
}

bool IsKnownSide(TwinSide side)
{
    return side == TwinSide::Side0 || side == TwinSide::Side1;
}

bool IsTwinEncounterParticipant(Player const* bot, bool allowExtendedRoom)
{
    if (!bot || !bot->IsAlive() || !bot->IsInWorld() || !Aq40BossHelper::IsInAq40(bot))
        return false;

    if (allowExtendedRoom && HasActiveLockedPickupAnchor(bot))
        return true;

    return allowExtendedRoom ? IsNearTwinApproach(bot) : IsNearTwinRoom(bot, kTwinRoomReadyRadius);
}

TwinEncounterGeometry const& GetGeometry()
{
    return sTwinGeometry;
}

TwinRoleAssignment const* GetAssignmentForMember(TwinEncounterState const& state, ObjectGuid memberGuid)
{
    if (memberGuid.IsEmpty())
        return nullptr;

    auto const itr = std::find_if(state.assignments.begin(), state.assignments.end(),
        [memberGuid](TwinRoleAssignment const& assignment)
        {
            return assignment.memberGuid == memberGuid;
        });

    return itr != state.assignments.end() ? &(*itr) : nullptr;
}

TwinRoleAssignment const* GetAssignmentForMember(Player const* bot)
{
    if (!bot)
        return nullptr;

    TwinEncounterState const* state = GetEncounterState(bot);
    return state ? GetAssignmentForMember(*state, bot->GetGUID()) : nullptr;
}

bool HasTwinAssignmentForMember(TwinEncounterState const& state, Player const* bot)
{
    return bot && GetAssignmentForMember(state, bot->GetGUID()) != nullptr;
}

bool IsTwinAssignedParticipant(TwinEncounterState const& state, Player const* bot, bool allowExtendedRoom)
{
    return HasTwinAssignmentForMember(state, bot) && IsTwinEncounterParticipant(bot, allowExtendedRoom);
}

bool IsAssignedToCohort(TwinEncounterState const& state, ObjectGuid memberGuid, TwinRoleCohort cohort)
{
    TwinRoleAssignment const* assignment = GetAssignmentForMember(state, memberGuid);
    return assignment && assignment->cohort == cohort;
}

bool HasDeterministicAssignments(TwinEncounterState const& state)
{
    return state.unsupportedReason.empty() && !state.assignments.empty();
}

std::string const& GetUnsupportedReason(TwinEncounterState const& state)
{
    return state.unsupportedReason;
}

bool IsTwinApproachWindow(TwinEncounterState const& state, Player const* bot)
{
    return bot && HasDeterministicAssignments(state) && state.phase == TwinEncounterPhase::PrePull &&
           state.mode == TwinStrategyMode::Inactive && IsTwinAssignedParticipant(state, bot);
}

bool IsTwinApproachWindow(Player const* bot)
{
    TwinEncounterState const* state = GetEncounterState(bot);
    return state && IsTwinApproachWindow(*state, bot);
}

bool IsTwinCenterCommitted(TwinEncounterState const& state)
{
    return HasDeterministicAssignments(state) && state.phase == TwinEncounterPhase::PrePull &&
           (state.mode == TwinStrategyMode::CenterCommitted ||
            state.mode == TwinStrategyMode::StandardCompReady);
}

bool IsTwinCenterCommitted(Player const* bot)
{
    TwinEncounterState const* state = GetEncounterState(bot);
    return state && IsTwinCenterCommitted(*state);
}

bool IsTwinPrePullStageWindow(TwinEncounterState const& state, Player const* bot)
{
    return bot && IsTwinCenterCommitted(state) && IsTwinAssignedParticipant(state, bot, false);
}

bool IsTwinPrePullStageWindow(Player const* bot)
{
    TwinEncounterState const* state = GetEncounterState(bot);
    return state && IsTwinPrePullStageWindow(*state, bot);
}

bool IsTwinPrePullReady(Player const* bot)
{
    TwinEncounterState const* state = GetEncounterState(bot);
    if (!state || !HasDeterministicAssignments(*state) || state->phase != TwinEncounterPhase::PrePull)
        return false;

    bool const isStrictReadyParticipant = IsTwinAssignedParticipant(*state, bot, false);
    bool const isReady = state->mode == TwinStrategyMode::StandardCompReady && isStrictReadyParticipant;
    if (!isReady)
    {
        TwinRoleAssignment const* assignment = GetAssignmentForMember(*state, bot->GetGUID());
        if (assignment)
        {
            char const* waitReason = "distance";
            if (isStrictReadyParticipant)
            {
                waitReason = IsTwinCenterCommitted(*state) ? "strict_ready_pending" : "center_commit_pending";
            }

            std::ostringstream fields;
            fields << "boss=twin prepull=not_ready"
                   << " mode=" << ToString(state->mode)
                   << " wait=" << waitReason
                   << " cohort=" << ToString(assignment->cohort)
                   << " side=" << ToString(assignment->stableSide)
                   << " slot=" << static_cast<uint32>(assignment->slotIndex)
                   << " approach=" << state->approachMemberCount
                   << " staged=" << state->stagedMemberCount
                   << " center_committed=" << state->centerCommittedMemberCount
                   << " strict_ready=" << state->strictReadyMemberCount
                   << " assigned=" << state->assignments.size()
                   << " unsupported_reason=" << (state->unsupportedReason.empty() ? "none" : state->unsupportedReason);
            Aq40Helpers::LogAq40Info(const_cast<Player*>(bot), "twin_prepull",
                "twin:prepull:not_ready:" + std::string(waitReason),
                fields.str(), 2000);
        }
    }

    return isReady;
}

bool IsTwinDesignatedWarlockTank(Player const* bot)
{
    if (!bot || bot->getClass() != CLASS_WARLOCK)
        return false;

    TwinEncounterState const* state = GetEncounterState(bot);
    return state && HasDeterministicAssignments(*state) &&
           IsAssignedToCohort(*state, bot->GetGUID(), TwinRoleCohort::WarlockTank);
}

bool ShouldUseTwinWarlockTankStrategy(Player const* bot)
{
    TwinEncounterState const* state = GetEncounterState(bot);
    if (!state || IsTerminalPhase(state->phase) || !IsTwinDesignatedWarlockTank(bot))
        return false;

    return IsTwinPrePullReady(bot) || (IsActivePhase(state->phase) && IsTwinAssignedParticipant(*state, bot));
}

bool SyncTwinWarlockTankStrategy(Player* bot)
{
    if (!bot)
        return false;

    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
    if (!botAI)
        return false;

    uint64 const botKey = GetBotKey(bot);
    uint32 const instanceId = GetInstanceId(bot);
    auto managedItr = sTwinWarlockTankOverlayByBot.find(botKey);
    if (managedItr != sTwinWarlockTankOverlayByBot.end() && managedItr->second.instanceId != instanceId)
    {
        sTwinWarlockTankOverlayByBot.erase(managedItr);
        managedItr = sTwinWarlockTankOverlayByBot.end();
    }

    if (!ShouldUseTwinWarlockTankStrategy(bot))
        return ClearTwinWarlockTankStrategy(bot);

    if (botAI->HasStrategy("tank", BOT_STATE_COMBAT))
        return false;

    botAI->ChangeStrategy("+tank", BOT_STATE_COMBAT);

    TwinManagedWarlockTankOverlay& overlay = sTwinWarlockTankOverlayByBot[botKey];
    overlay.instanceId = instanceId;
    overlay.addedByTwin = true;
    return true;
}

bool ClearTwinWarlockTankStrategy(Player* bot)
{
    if (!bot)
        return false;

    uint64 const botKey = GetBotKey(bot);
    auto managedItr = sTwinWarlockTankOverlayByBot.find(botKey);
    if (managedItr == sTwinWarlockTankOverlayByBot.end())
        return false;

    bool removed = false;
    if (managedItr->second.addedByTwin)
    {
        if (PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot))
        {
            if (botAI->HasStrategy("tank", BOT_STATE_COMBAT))
            {
                botAI->ChangeStrategy("-tank", BOT_STATE_COMBAT);
                removed = true;
            }
        }
    }

    sTwinWarlockTankOverlayByBot.erase(managedItr);
    return removed;
}

bool MarkTwinLocalCleanupState(Player* bot)
{
    if (!bot)
        return false;

    uint32 const instanceId = GetInstanceId(bot);
    if (!instanceId)
        return false;

    uint64 const botKey = GetBotKey(bot);
    auto itr = sTwinLocalCleanupByBot.find(botKey);
    if (itr != sTwinLocalCleanupByBot.end() && itr->second == instanceId)
        return false;

    sTwinLocalCleanupByBot[botKey] = instanceId;
    return true;
}

bool HasTwinLocalCleanupState(Player const* bot)
{
    if (!bot)
        return false;

    uint32 const instanceId = GetInstanceId(bot);
    if (!instanceId)
        return false;

    auto const itr = sTwinLocalCleanupByBot.find(GetBotKey(bot));
    return itr != sTwinLocalCleanupByBot.end() && itr->second == instanceId;
}

bool ClearTwinLocalCombatState(Player* bot, PlayerbotAI* botAI, bool clearCleanupState)
{
    if (!bot || !botAI || !botAI->GetAiObjectContext())
        return false;

    auto* context = botAI->GetAiObjectContext();
    bool changed = false;

    if (context->GetValue<Unit*>("old target")->Get())
    {
        context->GetValue<Unit*>("old target")->Set(nullptr);
        changed = true;
    }

    if (context->GetValue<Unit*>("current target")->Get())
    {
        context->GetValue<Unit*>("current target")->Set(nullptr);
        changed = true;
    }

    if (!context->GetValue<GuidVector>("prioritized targets")->Get().empty())
    {
        context->GetValue<GuidVector>("prioritized targets")->Reset();
        changed = true;
    }

    if (!context->GetValue<ObjectGuid>("pull target")->Get().IsEmpty())
    {
        context->GetValue<ObjectGuid>("pull target")->Set(ObjectGuid::Empty);
        changed = true;
    }

    if (!context->GetValue<ObjectGuid>("pull strategy target")->Get().IsEmpty())
    {
        context->GetValue<ObjectGuid>("pull strategy target")->Set(ObjectGuid::Empty);
        changed = true;
    }

    if (bot->GetTarget())
    {
        bot->SetTarget();
        bot->SetSelection(ObjectGuid());
        changed = true;
    }

    std::list<ObjectGuid> const& focusHealTargets = context->GetValue<std::list<ObjectGuid>>("focus heal targets")->Get();
    if (!focusHealTargets.empty())
    {
        context->GetValue<std::list<ObjectGuid>>("focus heal targets")->Set(std::list<ObjectGuid>());
        changed = true;
    }

    if (botAI->HasStrategy("focus heal targets", BOT_STATE_COMBAT))
    {
        botAI->ChangeStrategy("-focus heal targets", BOT_STATE_COMBAT);
        changed = true;
    }

    if (!context->GetValue<std::string>("rti")->Get().empty())
    {
        context->GetValue<std::string>("rti")->Set("");
        changed = true;
    }

    if (!context->GetValue<std::string>("rti cc")->Get().empty())
    {
        context->GetValue<std::string>("rti cc")->Set("");
        changed = true;
    }

    if (context->GetValue<Unit*>("rti target")->Get())
    {
        context->GetValue<Unit*>("rti target")->Set(nullptr);
        changed = true;
    }

    if (context->GetValue<Unit*>("rti cc target")->Get())
    {
        context->GetValue<Unit*>("rti cc target")->Set(nullptr);
        changed = true;
    }

    if (Pet* pet = bot->GetPet())
    {
        bool petNeedsFollow = pet->GetVictim() || pet->IsInCombat();
        if (CharmInfo* charmInfo = pet->GetCharmInfo())
            petNeedsFollow = petNeedsFollow || charmInfo->IsCommandAttack() || !charmInfo->IsReturning();

        if (petNeedsFollow)
        {
            botAI->PetFollow();
            changed = true;
        }
    }

    if (!clearCleanupState)
        return changed;

    auto itr = sTwinLocalCleanupByBot.find(GetBotKey(bot));
    if (itr == sTwinLocalCleanupByBot.end() || itr->second != GetInstanceId(bot))
        return changed;

    sTwinLocalCleanupByBot.erase(itr);
    return true;
}

TwinStableOwnership& GetOwnership(TwinEncounterState& state, TwinBoss boss)
{
    return state.ownership[ToIndex(boss)];
}

TwinStableOwnership const& GetOwnership(TwinEncounterState const& state, TwinBoss boss)
{
    return state.ownership[ToIndex(boss)];
}

TwinBossRecoveryState& GetRecoveryState(TwinEncounterState& state, TwinBoss boss)
{
    return state.recovery.boss[ToIndex(boss)];
}

TwinBossRecoveryState const& GetRecoveryState(TwinEncounterState const& state, TwinBoss boss)
{
    return state.recovery.boss[ToIndex(boss)];
}

bool SetMode(TwinEncounterState& state, TwinStrategyMode mode, uint32 nowMs)
{
    uint32 const now = ResolveNow(nowMs);
    if (state.mode == mode)
    {
        if (!state.modeEnteredAtMs)
        {
            state.modeEnteredAtMs = now;
            return true;
        }

        return false;
    }

    state.mode = mode;
    state.modeEnteredAtMs = now;
    return true;
}

bool CanTransitionPhase(TwinEncounterPhase from, TwinEncounterPhase to)
{
    if (from == to || to == TwinEncounterPhase::PrePull || to == TwinEncounterPhase::TerminalFailure ||
        to == TwinEncounterPhase::Degraded)
    {
        return true;
    }

    switch (from)
    {
        case TwinEncounterPhase::PrePull:
            return to == TwinEncounterPhase::DualPullWindow || to == TwinEncounterPhase::Stable;
        case TwinEncounterPhase::DualPullWindow:
            return to == TwinEncounterPhase::Stable || to == TwinEncounterPhase::TeleportWindow ||
                   to == TwinEncounterPhase::PickupRecovery;
        case TwinEncounterPhase::Stable:
            return to == TwinEncounterPhase::TeleportWindow || to == TwinEncounterPhase::PickupRecovery;
        case TwinEncounterPhase::TeleportWindow:
            return to == TwinEncounterPhase::PickupRecovery || to == TwinEncounterPhase::Stable;
        case TwinEncounterPhase::PickupRecovery:
            return to == TwinEncounterPhase::Stable || to == TwinEncounterPhase::TeleportWindow;
        case TwinEncounterPhase::TerminalFailure:
            return false;
        case TwinEncounterPhase::Degraded:
            return to == TwinEncounterPhase::PickupRecovery || to == TwinEncounterPhase::Stable ||
                   to == TwinEncounterPhase::TeleportWindow;
    }

    return false;
}

bool SetPhase(TwinEncounterState& state, TwinEncounterPhase phase, uint32 nowMs)
{
    uint32 const now = ResolveNow(nowMs);
    if (state.phase == phase)
    {
        if (!state.phaseEnteredAtMs)
        {
            state.phaseEnteredAtMs = now;
            return true;
        }

        return false;
    }

    if (!CanTransitionPhase(state.phase, phase))
        return false;

    state.phase = phase;
    state.phaseEnteredAtMs = now;
    return true;
}

bool IsActivePhase(TwinEncounterPhase phase)
{
    return phase != TwinEncounterPhase::PrePull;
}

bool IsRecoveryPhase(TwinEncounterPhase phase)
{
    return phase == TwinEncounterPhase::TeleportWindow || phase == TwinEncounterPhase::PickupRecovery ||
           phase == TwinEncounterPhase::Degraded;
}

bool IsTerminalPhase(TwinEncounterPhase phase)
{
    return phase == TwinEncounterPhase::TerminalFailure;
}

uint32 GetPhaseElapsedMs(TwinEncounterState const& state, uint32 nowMs)
{
    return GetElapsedSince(state.phaseEnteredAtMs, ResolveNow(nowMs));
}

bool HasTeleportCadence(TwinEncounterState const& state)
{
    return state.nextTeleportEarliestAtMs && state.nextTeleportLatestAtMs && state.swapPrepStartAtMs;
}

void ArmTeleportCadence(TwinEncounterState& state, uint32 nowMs)
{
    uint32 const now = ResolveNow(nowMs);
    if (!state.closestTargetGrantDelayMs)
        state.closestTargetGrantDelayMs = kTwinClosestTargetGrantDelayMs;

    if (HasTeleportCadence(state) && now < state.nextTeleportLatestAtMs)
        return;

    ScheduleNextTeleportCadence(state, now);
}

bool IsSwapPrepActive(TwinEncounterState const& state, uint32 nowMs)
{
    if (state.phase == TwinEncounterPhase::PrePull || state.phase == TwinEncounterPhase::TeleportWindow ||
        state.phase == TwinEncounterPhase::PickupRecovery || IsTerminalPhase(state.phase))
    {
        return false;
    }

    if (!HasTeleportCadence(state))
        return false;

    uint32 const now = ResolveNow(nowMs);
    return now >= state.swapPrepStartAtMs && now <= state.nextTeleportLatestAtMs;
}

bool IsSwapPrepActive(Player const* bot, uint32 nowMs)
{
    TwinEncounterState const* state = GetEncounterState(bot);
    return state && IsSwapPrepActive(*state, nowMs);
}

bool ArmSwapPrep(TwinEncounterState& state, uint32 nowMs)
{
    uint32 const now = ResolveNow(nowMs);
    if (!IsSwapPrepActive(state, now) || state.swapPrepArmedAtMs)
        return false;

    for (TwinBoss boss : { TwinBoss::Veklor, TwinBoss::Veknilash })
    {
        TwinStableOwnership& ownership = GetOwnership(state, boss);
        if (!ownership.expectedOwner.IsEmpty() && !ownership.reserveOwner.IsEmpty())
            std::swap(ownership.expectedOwner, ownership.reserveOwner);
    }

    state.swapPrepArmedAtMs = now;
    return true;
}

uint32 GetScriptedEventAtMs(TwinEncounterState const& state, TwinScriptedEvent event)
{
    switch (event)
    {
        case TwinScriptedEvent::Teleport: return state.scriptedHazards.teleportAtMs;
        case TwinScriptedEvent::Blizzard: return state.scriptedHazards.blizzardAtMs;
        case TwinScriptedEvent::ArcaneBurst: return state.scriptedHazards.arcaneBurstAtMs;
        case TwinScriptedEvent::HealBrother: return state.scriptedHazards.healBrotherAtMs;
        case TwinScriptedEvent::ExplodeBug: return state.scriptedHazards.explodeBugAtMs;
        case TwinScriptedEvent::MutateBug: return state.scriptedHazards.mutateBugAtMs;
        case TwinScriptedEvent::Uppercut: return state.scriptedHazards.uppercutAtMs;
        case TwinScriptedEvent::UnbalancingStrike: return state.scriptedHazards.unbalancingStrikeAtMs;
    }

    return 0;
}

bool IsScriptedEventActive(TwinEncounterState const& state, TwinScriptedEvent event, uint32 windowMs, uint32 nowMs,
                           uint32* outElapsedMs)
{
    if (outElapsedMs)
        *outElapsedMs = 0;

    if (!windowMs)
        return false;

    uint32 const eventAtMs = GetScriptedEventAtMs(state, event);
    if (!eventAtMs)
        return false;

    uint32 const elapsedMs = GetElapsedSince(eventAtMs, ResolveNow(nowMs));
    if (outElapsedMs)
        *outElapsedMs = elapsedMs;

    return elapsedMs <= windowMs;
}

bool IsScriptedEventActive(Player const* bot, TwinScriptedEvent event, uint32 windowMs, uint32 nowMs,
                           uint32* outElapsedMs)
{
    TwinEncounterState const* state = GetEncounterState(bot);
    return state && IsScriptedEventActive(*state, event, windowMs, nowMs, outElapsedMs);
}

ObjectGuid GetExplodeBugSourceGuid(TwinEncounterState const& state)
{
    return state.scriptedHazards.explodeBugSourceGuid;
}

Position const& GetExplodeBugSourcePosition(TwinEncounterState const& state)
{
    return state.scriptedHazards.explodeBugSourcePosition;
}

bool SetExplodeBugSource(TwinEncounterState& state, ObjectGuid sourceGuid, Position const& sourcePosition)
{
    TwinScriptedHazardWindows& hazards = state.scriptedHazards;
    if (hazards.explodeBugSourceGuid == sourceGuid && IsSamePosition(hazards.explodeBugSourcePosition, sourcePosition))
        return false;

    hazards.explodeBugSourceGuid = sourceGuid;
    hazards.explodeBugSourcePosition = sourcePosition;
    return true;
}

void ClearExplodeBugSource(TwinEncounterState& state)
{
    state.scriptedHazards.explodeBugSourceGuid = ObjectGuid::Empty;
    state.scriptedHazards.explodeBugSourcePosition.Relocate(0.0f, 0.0f, 0.0f);
}

bool SetExpectedOwner(TwinEncounterState& state, TwinBoss boss, ObjectGuid ownerGuid)
{
    TwinStableOwnership& ownership = GetOwnership(state, boss);
    if (ownership.expectedOwner == ownerGuid)
        return false;

    ownership.expectedOwner = ownerGuid;
    return true;
}

bool SetReserveOwner(TwinEncounterState& state, TwinBoss boss, ObjectGuid ownerGuid)
{
    TwinStableOwnership& ownership = GetOwnership(state, boss);
    if (ownership.reserveOwner == ownerGuid)
        return false;

    ownership.reserveOwner = ownerGuid;
    return true;
}

bool SetCandidateOwner(TwinEncounterState& state, TwinBoss boss, ObjectGuid ownerGuid)
{
    TwinStableOwnership& ownership = GetOwnership(state, boss);
    if (ownership.candidateOwner == ownerGuid)
        return false;

    ownership.candidateOwner = ownerGuid;
    return true;
}

void ClearCandidateOwner(TwinEncounterState& state, TwinBoss boss)
{
    GetOwnership(state, boss).candidateOwner = ObjectGuid::Empty;
}

bool ConfirmOwner(TwinEncounterState& state, TwinBoss boss, ObjectGuid ownerGuid, uint32 nowMs)
{
    if (ownerGuid.IsEmpty())
        return false;

    TwinStableOwnership& ownership = GetOwnership(state, boss);
    uint32 const now = ResolveNow(nowMs);
    bool changed = false;

    if (ownership.candidateOwner != ownerGuid)
    {
        ownership.candidateOwner = ownerGuid;
        changed = true;
    }

    if (ownership.lastValidConfirmationMs != now)
    {
        ownership.lastValidConfirmationMs = now;
        changed = true;
    }

    return changed;
}

bool SetStableOwner(TwinEncounterState& state, TwinBoss boss, ObjectGuid ownerGuid, uint32 nowMs)
{
    if (ownerGuid.IsEmpty())
        return false;

    TwinStableOwnership& ownership = GetOwnership(state, boss);
    uint32 const now = ResolveNow(nowMs);
    bool changed = ConfirmOwner(state, boss, ownerGuid, now);

    if (ownership.stableOwner != ownerGuid)
    {
        ownership.stableOwner = ownerGuid;
        ownership.stableSinceMs = now;
        changed = true;
    }
    else if (!ownership.stableSinceMs)
    {
        ownership.stableSinceMs = now;
        changed = true;
    }

    return changed;
}

void ClearStableOwner(TwinEncounterState& state, TwinBoss boss)
{
    TwinStableOwnership& ownership = GetOwnership(state, boss);
    ownership.stableOwner = ObjectGuid::Empty;
    ownership.stableSinceMs = 0;
}

void ResetStableOwnership(TwinEncounterState& state, TwinBoss boss, bool keepAssignments)
{
    TwinStableOwnership& ownership = GetOwnership(state, boss);
    if (!keepAssignments)
    {
        ownership = TwinStableOwnership();
        return;
    }

    ObjectGuid const expectedOwner = ownership.expectedOwner;
    ObjectGuid const reserveOwner = ownership.reserveOwner;
    bool const reservePromotionUsed = ownership.reservePromotionUsed;

    ownership = TwinStableOwnership();
    ownership.expectedOwner = expectedOwner;
    ownership.reserveOwner = reserveOwner;
    ownership.reservePromotionUsed = reservePromotionUsed;
}

void ResetAllStableOwnership(TwinEncounterState& state, bool keepAssignments)
{
    ResetStableOwnership(state, TwinBoss::Veklor, keepAssignments);
    ResetStableOwnership(state, TwinBoss::Veknilash, keepAssignments);
}

bool HasStableOwner(TwinEncounterState const& state, TwinBoss boss)
{
    return !GetOwnership(state, boss).stableOwner.IsEmpty();
}

bool HasCandidateOwner(TwinEncounterState const& state, TwinBoss boss)
{
    return !GetOwnership(state, boss).candidateOwner.IsEmpty();
}

bool IsStableOwner(TwinEncounterState const& state, TwinBoss boss, ObjectGuid ownerGuid)
{
    return !ownerGuid.IsEmpty() && GetOwnership(state, boss).stableOwner == ownerGuid;
}

bool IsPrimaryController(TwinEncounterState const& state, TwinBoss boss, ObjectGuid ownerGuid)
{
    if (ownerGuid.IsEmpty())
        return false;

    TwinBossRecoveryState const& recovery = GetRecoveryState(state, boss);
    if (!recovery.pickupOwner.IsEmpty())
        return recovery.pickupOwner == ownerGuid;

    TwinStableOwnership const& ownership = GetOwnership(state, boss);
    if (!ownership.stableOwner.IsEmpty())
        return ownership.stableOwner == ownerGuid;

    if (!ownership.candidateOwner.IsEmpty())
        return ownership.candidateOwner == ownerGuid;

    if (IsSwapPrepActive(state))
        return false;

    return ownership.expectedOwner == ownerGuid;
}

bool CanPromoteReserveOwner(TwinEncounterState const& state, TwinBoss boss)
{
    TwinStableOwnership const& ownership = GetOwnership(state, boss);
    return !ownership.reserveOwner.IsEmpty() && !ownership.reservePromotionUsed;
}

bool PromoteReserveOwner(TwinEncounterState& state, TwinBoss boss, uint32 nowMs)
{
    if (!CanPromoteReserveOwner(state, boss))
        return false;

    TwinStableOwnership& ownership = GetOwnership(state, boss);
    uint32 const now = ResolveNow(nowMs);
    ObjectGuid const promotedOwner = ownership.reserveOwner;

    ownership.expectedOwner = promotedOwner;
    ownership.reserveOwner = ObjectGuid::Empty;
    ownership.candidateOwner = promotedOwner;
    ownership.stableOwner = ObjectGuid::Empty;
    ownership.stableSinceMs = 0;
    ownership.reservePromotionUsed = true;
    ownership.lastValidConfirmationMs = now;
    return true;
}

uint32 GetStableOwnershipAgeMs(TwinEncounterState const& state, TwinBoss boss, uint32 nowMs)
{
    TwinStableOwnership const& ownership = GetOwnership(state, boss);
    return ownership.stableOwner.IsEmpty() ? 0 : GetElapsedSince(ownership.stableSinceMs, ResolveNow(nowMs));
}

uint32 GetTimeSinceOwnershipConfirmationMs(TwinEncounterState const& state, TwinBoss boss, uint32 nowMs)
{
    return GetElapsedSince(GetOwnership(state, boss).lastValidConfirmationMs, ResolveNow(nowMs));
}

void ArmThreatHoldWindow(TwinEncounterState& state, TwinBoss boss, uint32 durationMs, uint32 nowMs)
{
    TwinBossRecoveryState& recovery = GetRecoveryState(state, boss);
    uint32 const now = ResolveNow(nowMs);
    recovery.threatHoldUntilMs = durationMs ? now + durationMs : 0;
}

void ClearThreatHoldWindow(TwinEncounterState& state, TwinBoss boss)
{
    GetRecoveryState(state, boss).threatHoldUntilMs = 0;
}

bool IsThreatHoldWindowActive(TwinEncounterState const& state, TwinBoss boss, uint32 nowMs)
{
    return IsActiveUntil(GetRecoveryState(state, boss).threatHoldUntilMs, ResolveNow(nowMs));
}

uint32 GetThreatHoldRemainingMs(TwinEncounterState const& state, TwinBoss boss, uint32 nowMs)
{
    TwinBossRecoveryState const& recovery = GetRecoveryState(state, boss);
    uint32 const now = ResolveNow(nowMs);
    return IsActiveUntil(recovery.threatHoldUntilMs, now) ? (recovery.threatHoldUntilMs - now) : 0;
}

bool IsAnyThreatHoldWindowActive(TwinEncounterState const& state, uint32 nowMs)
{
    return IsThreatHoldWindowActive(state, TwinBoss::Veklor, nowMs) ||
           IsThreatHoldWindowActive(state, TwinBoss::Veknilash, nowMs);
}

uint32 GetMaxThreatHoldRemainingMs(TwinEncounterState const& state, uint32 nowMs)
{
    return std::max(GetThreatHoldRemainingMs(state, TwinBoss::Veklor, nowMs),
                    GetThreatHoldRemainingMs(state, TwinBoss::Veknilash, nowMs));
}

bool MarkPickupEstablished(TwinEncounterState& state, TwinBoss boss, ObjectGuid ownerGuid, uint32 nowMs)
{
    if (ownerGuid.IsEmpty())
        return false;

    TwinBossRecoveryState& recovery = GetRecoveryState(state, boss);
    uint32 const now = ResolveNow(nowMs);
    bool const ownerChanged = recovery.pickupOwner != ownerGuid;
    bool const newlyEstablished = !recovery.pickupEstablished;
    bool changed = false;

    if (newlyEstablished)
    {
        recovery.pickupEstablished = true;
        changed = true;
    }

    if (ownerChanged)
    {
        recovery.pickupOwner = ownerGuid;
        changed = true;
    }

    if (newlyEstablished || ownerChanged || !recovery.pickupEstablishedAtMs)
    {
        recovery.pickupEstablishedAtMs = now;
        changed = true;
    }

    if (recovery.pickupLostAtMs)
    {
        recovery.pickupLostAtMs = 0;
        changed = true;
    }

    return changed;
}

void ClearPickupEstablished(TwinEncounterState& state, TwinBoss boss, uint32 nowMs)
{
    TwinBossRecoveryState& recovery = GetRecoveryState(state, boss);
    if (!recovery.pickupEstablished && recovery.pickupOwner.IsEmpty() && !recovery.pickupEstablishedAtMs)
        return;

    recovery.pickupEstablished = false;
    recovery.pickupOwner = ObjectGuid::Empty;
    recovery.pickupEstablishedAtMs = 0;
    recovery.pickupLostAtMs = ResolveNow(nowMs);
}

bool IsPickupEstablished(TwinEncounterState const& state, TwinBoss boss)
{
    return GetRecoveryState(state, boss).pickupEstablished;
}

ObjectGuid GetPickupOwner(TwinEncounterState const& state, TwinBoss boss)
{
    return GetRecoveryState(state, boss).pickupOwner;
}

uint32 GetPickupEstablishedAgeMs(TwinEncounterState const& state, TwinBoss boss, uint32 nowMs)
{
    TwinBossRecoveryState const& recovery = GetRecoveryState(state, boss);
    if (!recovery.pickupEstablished)
        return 0;

    return GetElapsedSince(recovery.pickupEstablishedAtMs, ResolveNow(nowMs));
}

bool SetSplitBand(TwinEncounterState& state, TwinSplitBand band, uint32 nowMs)
{
    uint32 const now = ResolveNow(nowMs);
    if (state.recovery.splitBand == band)
    {
        if (!state.recovery.splitBandEnteredAtMs)
        {
            state.recovery.splitBandEnteredAtMs = now;
            return true;
        }

        return false;
    }

    state.recovery.splitBand = band;
    state.recovery.splitBandEnteredAtMs = now;
    return true;
}

uint32 GetSplitBandAgeMs(TwinEncounterState const& state, uint32 nowMs)
{
    return GetElapsedSince(state.recovery.splitBandEnteredAtMs, ResolveNow(nowMs));
}

void EnterDualPullWindow(TwinEncounterState& state, uint32 nowMs)
{
    ArmTeleportCadence(state, nowMs);
    SetPhase(state, TwinEncounterPhase::DualPullWindow, nowMs);
}

void EnterStablePhase(TwinEncounterState& state, uint32 nowMs)
{
    ArmTeleportCadence(state, nowMs);
    SetPhase(state, TwinEncounterPhase::Stable, nowMs);
}

void EnterTeleportWindow(TwinEncounterState& state, uint32 threatHoldDurationMs, uint32 nowMs)
{
    uint32 const now = ResolveNow(nowMs);
    state.lastTeleportAtMs = now;
    ScheduleNextTeleportCadence(state, now);
    SetPhase(state, TwinEncounterPhase::TeleportWindow, now);

    for (TwinBoss boss : { TwinBoss::Veklor, TwinBoss::Veknilash })
    {
        ResetStableOwnership(state, boss, true);
        ArmThreatHoldWindow(state, boss, threatHoldDurationMs, now);
        ClearPickupEstablished(state, boss, now);

        TwinStableOwnership& ownership = GetOwnership(state, boss);
        ownership.candidateOwner = ownership.expectedOwner;
    }
}

void EnterPickupRecovery(TwinEncounterState& state, uint32 nowMs)
{
    SetPhase(state, TwinEncounterPhase::PickupRecovery, nowMs);
}

void EnterTerminalFailure(TwinEncounterState& state, uint32 nowMs)
{
    uint32 const now = ResolveNow(nowMs);
    SetSplitBand(state, TwinSplitBand::Terminal, now);
    SetPhase(state, TwinEncounterPhase::TerminalFailure, now);
}

void EnterDegradedPhase(TwinEncounterState& state, uint32 nowMs)
{
    uint32 const now = ResolveNow(nowMs);
    SetMode(state, TwinStrategyMode::Degraded, now);
    SetPhase(state, TwinEncounterPhase::Degraded, now);
}

uint32 GetInstanceId(Player const* bot)
{
    if (!bot || !bot->GetMap())
        return 0;

    return bot->GetMap()->GetInstanceId();
}

TwinEncounterState* GetEncounterState(Player* bot)
{
    uint32 const instanceId = GetInstanceId(bot);
    if (!instanceId || !Aq40BossHelper::IsInAq40(bot))
        return nullptr;

    TwinEncounterState& state = sTwinStateByInstance[instanceId];
    if (state.instanceId != instanceId)
        ResetEncounterState(state, instanceId);

    RefreshPrePullAssignments(bot, state);
    if (state.phase != TwinEncounterPhase::PrePull && !IsTerminalPhase(state.phase))
        ArmTeleportCadence(state);
    return &state;
}

TwinEncounterState const* GetEncounterState(Player const* bot)
{
    return bot ? GetEncounterState(const_cast<Player*>(bot)) : nullptr;
}

TwinEncounterState& EnsureEncounterState(Player* bot)
{
    return *GetEncounterState(bot);
}

TwinLockedPickupAnchor* GetLockedPickupAnchor(Player* bot)
{
    uint64 const key = GetBotKey(bot);
    if (!key)
        return nullptr;

    auto itr = sTwinPickupAnchorByBot.find(key);
    if (itr != sTwinPickupAnchorByBot.end() && IsLockedPickupAnchorExpired(itr->second))
    {
        sTwinPickupAnchorByBot.erase(itr);
        return nullptr;
    }

    return itr != sTwinPickupAnchorByBot.end() ? &itr->second : nullptr;
}

TwinLockedPickupAnchor const* GetLockedPickupAnchor(Player const* bot)
{
    uint64 const key = GetBotKey(bot);
    if (!key)
        return nullptr;

    auto itr = sTwinPickupAnchorByBot.find(key);
    if (itr != sTwinPickupAnchorByBot.end() && IsLockedPickupAnchorExpired(itr->second))
        return nullptr;

    return itr != sTwinPickupAnchorByBot.end() ? &itr->second : nullptr;
}

TwinLockedPickupAnchor& EnsureLockedPickupAnchor(Player* bot)
{
    uint64 const key = GetBotKey(bot);
    TwinLockedPickupAnchor& state = sTwinPickupAnchorByBot[key];
    if (state.instanceId != GetInstanceId(bot))
        ResetPickupAnchorState(state);

    state.instanceId = GetInstanceId(bot);
    return state;
}

TwinPetControlState* GetPetControlState(Player* bot)
{
    uint64 const key = GetBotKey(bot);
    if (!key)
        return nullptr;

    auto itr = sTwinPetControlByBot.find(key);
    if (itr != sTwinPetControlByBot.end() && itr->second.instanceId != GetInstanceId(bot))
    {
        sTwinPetControlByBot.erase(itr);
        return nullptr;
    }

    return itr != sTwinPetControlByBot.end() ? &itr->second : nullptr;
}

TwinPetControlState const* GetPetControlState(Player const* bot)
{
    uint64 const key = GetBotKey(bot);
    if (!key)
        return nullptr;

    auto itr = sTwinPetControlByBot.find(key);
    if (itr != sTwinPetControlByBot.end() && itr->second.instanceId != GetInstanceId(bot))
        return nullptr;

    return itr != sTwinPetControlByBot.end() ? &itr->second : nullptr;
}

TwinPetControlState& EnsurePetControlState(Player* bot)
{
    uint64 const key = GetBotKey(bot);
    TwinPetControlState& state = sTwinPetControlByBot[key];
    if (state.instanceId != GetInstanceId(bot))
        ResetPetControlState(state);

    state.instanceId = GetInstanceId(bot);
    return state;
}

bool IsLockedPickupAnchorExpired(TwinLockedPickupAnchor const& state, uint32 nowMs)
{
    return !state.instanceId || !state.expiresAtMs || !IsActiveUntil(state.expiresAtMs, ResolveNow(nowMs));
}

bool HasLockedPickupAnchor(Player const* bot, TwinBoss boss, uint32 nowMs)
{
    TwinLockedPickupAnchor const* state = GetLockedPickupAnchor(bot);
    return state && state->boss == boss && !IsLockedPickupAnchorExpired(*state, nowMs);
}

bool SetLockedPickupAnchor(Player* bot, TwinBoss boss, TwinSide side, TwinAnchor const& anchor, uint32 durationMs,
                           uint32 nowMs)
{
    uint32 const instanceId = GetInstanceId(bot);
    if (!bot || !instanceId)
        return false;

    TwinLockedPickupAnchor& state = EnsureLockedPickupAnchor(bot);
    uint32 const now = ResolveNow(nowMs);
    uint32 const expiresAtMs = durationMs ? now + durationMs : 0;
    bool const changed = state.boss != boss || state.side != side ||
                         state.anchor.position.GetPositionX() != anchor.position.GetPositionX() ||
                         state.anchor.position.GetPositionY() != anchor.position.GetPositionY() ||
                         state.anchor.position.GetPositionZ() != anchor.position.GetPositionZ() ||
                         state.anchor.preferredRange != anchor.preferredRange || state.anchor.facing != anchor.facing;

    state.instanceId = instanceId;
    state.boss = boss;
    state.side = side;
    state.lockedAtMs = now;
    state.expiresAtMs = expiresAtMs;
    state.anchor = anchor;
    return changed;
}

bool PruneExpiredLockedPickupAnchor(Player* bot, uint32 nowMs)
{
    uint64 const key = GetBotKey(bot);
    auto itr = sTwinPickupAnchorByBot.find(key);
    if (itr == sTwinPickupAnchorByBot.end() || !IsLockedPickupAnchorExpired(itr->second, nowMs))
        return false;

    sTwinPickupAnchorByBot.erase(itr);
    return true;
}

void ClearLockedPickupAnchor(Player* bot)
{
    uint64 const key = GetBotKey(bot);
    auto itr = sTwinPickupAnchorByBot.find(key);
    if (itr != sTwinPickupAnchorByBot.end())
        sTwinPickupAnchorByBot.erase(itr);
}

bool HasActiveLockedPickupAnchor(Player const* bot, uint32 nowMs)
{
    TwinLockedPickupAnchor const* state = GetLockedPickupAnchor(bot);
    return state && !IsLockedPickupAnchorExpired(*state, nowMs);
}

bool IsImmediateRepositionWindow(TwinEncounterState const& state, uint32 nowMs)
{
    return state.phase == TwinEncounterPhase::DualPullWindow || state.phase == TwinEncounterPhase::TeleportWindow ||
           state.phase == TwinEncounterPhase::PickupRecovery || IsAnyThreatHoldWindowActive(state, nowMs);
}

bool IsImmediateRepositionWindow(Player const* bot, uint32 nowMs)
{
    TwinEncounterState const* state = GetEncounterState(bot);
    return HasActiveLockedPickupAnchor(bot, nowMs) || (state && IsImmediateRepositionWindow(*state, nowMs));
}

bool RequestImmediateMovementInterrupt(Player* bot)
{
    if (!bot)
        return false;

    if (PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot))
    {
        botAI->RequestSpellInterrupt();
        return true;
    }

    return false;
}

void ResetEncounterState(TwinEncounterState& state, uint32 instanceId)
{
    state = TwinEncounterState();
    state.instanceId = instanceId;
}

void ResetPickupAnchorState(TwinLockedPickupAnchor& state)
{
    state = TwinLockedPickupAnchor();
}

void ResetPetControlState(TwinPetControlState& state)
{
    state = TwinPetControlState();
}

bool RestorePetControlState(Player* bot)
{
    if (!bot)
        return false;

    uint64 const key = GetBotKey(bot);
    auto itr = sTwinPetControlByBot.find(key);
    if (itr == sTwinPetControlByBot.end())
        return false;

    TwinPetControlState const controlState = itr->second;
    sTwinPetControlByBot.erase(itr);

    Pet* pet = bot->GetPet();
    if (!pet || pet->GetGUID() != controlState.petGuid)
        return true;

    for (uint32 spellId : controlState.disabledAutocastSpellIds)
    {
        if (!pet->HasSpell(spellId) || IsPetSpellAutocastEnabled(pet, spellId))
            continue;

        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
        if (!spellInfo || !spellInfo->IsAutocastable())
            continue;

        pet->ToggleAutocast(spellInfo, true);
    }

    if (controlState.forcedPassive && controlState.previousReactStateCaptured &&
        pet->GetReactState() == REACT_PASSIVE)
    {
        ReactStates const reactState = static_cast<ReactStates>(controlState.previousReactState);
        pet->SetReactState(reactState);
        if (CharmInfo* charmInfo = pet->GetCharmInfo())
            charmInfo->SetPlayerReactState(reactState);
    }

    return true;
}

bool ResetState(Player* bot)
{
    uint32 const instanceId = GetInstanceId(bot);
    if (!instanceId)
        return false;

    bool erased = ClearWarlockTankOverlaysForInstance(bot);
    erased = ClearPetControlStateForInstance(bot) || erased;
    erased = sTwinStateByInstance.erase(instanceId) > 0 || erased;
    for (auto itr = sTwinPickupAnchorByBot.begin(); itr != sTwinPickupAnchorByBot.end();)
    {
        if (itr->second.instanceId == instanceId)
        {
            itr = sTwinPickupAnchorByBot.erase(itr);
            erased = true;
            continue;
        }

        ++itr;
    }

    return erased;
}

bool HasPersistentState(Player* bot)
{
    uint32 const instanceId = GetInstanceId(bot);
    if (!instanceId)
        return false;

    auto const stateItr = sTwinStateByInstance.find(instanceId);
    if (stateItr != sTwinStateByInstance.end() && HasMeaningfulEncounterState(stateItr->second))
        return true;

    for (auto const& entry : sTwinPickupAnchorByBot)
    {
        if (entry.second.instanceId == instanceId && !IsLockedPickupAnchorExpired(entry.second))
            return true;
    }

    for (auto const& entry : sTwinPetControlByBot)
    {
        if (entry.second.instanceId == instanceId)
            return true;
    }

    return false;
}

char const* ToString(TwinBoss boss)
{
    switch (boss)
    {
        case TwinBoss::Veklor: return "veklor";
        case TwinBoss::Veknilash: return "veknilash";
    }

    return "unknown";
}

char const* ToString(TwinSide side)
{
    switch (side)
    {
        case TwinSide::Side0: return "side0";
        case TwinSide::Side1: return "side1";
        case TwinSide::Unknown: return "unknown";
    }

    return "unknown";
}

char const* ToString(TwinRoleCohort cohort)
{
    switch (cohort)
    {
        case TwinRoleCohort::None: return "none";
        case TwinRoleCohort::WarlockTank: return "warlock_tank";
        case TwinRoleCohort::MeleeTank: return "melee_tank";
        case TwinRoleCohort::SideHealer: return "side_healer";
        case TwinRoleCohort::RaidHealer: return "raid_healer";
        case TwinRoleCohort::RangedDps: return "ranged_dps";
        case TwinRoleCohort::Hunter: return "hunter";
        case TwinRoleCohort::MeleeDps: return "melee_dps";
    }

    return "unknown";
}

char const* ToString(TwinStrategyMode mode)
{
    switch (mode)
    {
        case TwinStrategyMode::Inactive: return "inactive";
        case TwinStrategyMode::CenterCommitted: return "center_committed";
        case TwinStrategyMode::StandardCompReady: return "standard_comp_ready";
        case TwinStrategyMode::Combat: return "combat";
        case TwinStrategyMode::Degraded: return "degraded";
    }

    return "unknown";
}

char const* ToString(TwinEncounterPhase phase)
{
    switch (phase)
    {
        case TwinEncounterPhase::PrePull: return "prepull";
        case TwinEncounterPhase::DualPullWindow: return "dual_pull_window";
        case TwinEncounterPhase::Stable: return "stable";
        case TwinEncounterPhase::TeleportWindow: return "teleport_window";
        case TwinEncounterPhase::PickupRecovery: return "pickup_recovery";
        case TwinEncounterPhase::TerminalFailure: return "terminal_failure";
        case TwinEncounterPhase::Degraded: return "degraded";
    }

    return "unknown";
}

char const* ToString(TwinScriptedEvent event)
{
    switch (event)
    {
        case TwinScriptedEvent::Teleport: return "teleport";
        case TwinScriptedEvent::Blizzard: return "blizzard";
        case TwinScriptedEvent::ArcaneBurst: return "arcane_burst";
        case TwinScriptedEvent::HealBrother: return "heal_brother";
        case TwinScriptedEvent::ExplodeBug: return "explode_bug";
        case TwinScriptedEvent::MutateBug: return "mutate_bug";
        case TwinScriptedEvent::Uppercut: return "uppercut";
        case TwinScriptedEvent::UnbalancingStrike: return "unbalancing_strike";
    }

    return "unknown";
}

char const* ToString(TwinSplitBand band)
{
    switch (band)
    {
        case TwinSplitBand::Stable: return "stable";
        case TwinSplitBand::Warning: return "warning";
        case TwinSplitBand::Urgent: return "urgent";
        case TwinSplitBand::Terminal: return "terminal";
    }

    return "unknown";
}
}    // namespace Aq40TwinEncounter
