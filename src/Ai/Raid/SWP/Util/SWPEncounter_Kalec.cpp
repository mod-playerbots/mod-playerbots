/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "SWPEncounter_Kalec.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "PlayerbotTextMgr.h"
#include "RaidBossHelpers.h"
#include "Timer.h"
#include <algorithm>
#include <map>
#include <vector>

namespace SwpHelpers
{

// Note: Kalecgos's CombatReach is 10.5f, and Sathrovarr's CombatReach is 4.0f
// Note: Kalecgos remains on player threat lists for the duration of the encounter, even for
// players in the Spectral Realm and after he turns friendly after Sathrovarr is killed.

Position const KALECGOS_TANK_POSITION =           { 1703.584f, 895.626f, 53.076f };
Position const KALECGOS_INITIAL_RANGED_POSITION = { 1704.634f, 938.080f, 53.076f };

std::unordered_map<uint32, KalecgosEncounterState> kalecgosEncounterStates;
std::unordered_map<ObjectGuid, KalecgosRealmState> kalecgosRealmStates;

namespace
{

void ClearExpiredKalecgosActiveRift(KalecgosEncounterState& state, uint32 now)
{
    if (!state.activeRiftOpenedMs)
        return;

    constexpr uint32 riftEntryWindowMs = 10000;
    if (getMSTimeDiff(state.activeRiftOpenedMs, now) <= riftEntryWindowMs)
        return;

    state.activeRiftOpenedMs = 0;
    state.activeRiftGroup = KALECGOS_INVALID_GROUP;
    state.blastedPlayerGuid = ObjectGuid::Empty;
    state.firstEntrantGuid = ObjectGuid::Empty;
    state.activeRiftOutgoingTankGuid = ObjectGuid::Empty;
}

uint8 GetKalecgosAssignedGroup(const KalecgosEncounterState& state, ObjectGuid playerGuid)
{
    auto const assignment = state.playerToGroup.find(playerGuid);
    return assignment != state.playerToGroup.end() ?
        assignment->second : KALECGOS_INVALID_GROUP;
}

Player* FindKalecgosGroupMember(Group* group, ObjectGuid playerGuid)
{
    if (playerGuid == ObjectGuid::Empty || !group)
        return nullptr;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member->GetGUID() != playerGuid)
            continue;

        if (member->GetMapId() != SWP_MAP_ID)
            return nullptr;

        return member;
    }

    return nullptr;
}

KalecgosEncounterState& GetPreparedKalecgosEncounterState(Player* player)
{
    KalecgosEncounterState& state = kalecgosEncounterStates[player->GetInstanceId()];
    if (!state.encounterStartMs)
        state.encounterStartMs = getMSTime();

    ClearExpiredKalecgosActiveRift(state, getMSTime());
    EnsureKalecgosGroupAssignments(player);
    return state;
}

bool IsKalecgosActiveRiftCandidate(Player* candidate, const KalecgosEncounterState& state)
{
    if (!candidate || !candidate->IsAlive() || candidate->GetMapId() != SWP_MAP_ID ||
        !state.activeRiftOpenedMs || state.activeRiftGroup == KALECGOS_INVALID_GROUP)
    {
        return false;
    }

    if (state.blastedPlayerGuid == candidate->GetGUID())
        return true;

    return GetKalecgosAssignedGroup(state, candidate->GetGUID()) == state.activeRiftGroup;
}

bool IsKalecgosPortalEligibleCandidate(Player* candidate)
{
    return candidate && candidate->IsAlive() && GET_PLAYERBOT_AI(candidate) &&
        candidate->GetMapId() == SWP_MAP_ID && !IsExhausted(candidate) &&
        !IsInSpectralRealm(candidate);
}

void AnnounceKalecgosTankTransition(
    PlayerbotAI* botAI, std::string const& textId, std::string const& defaultText,
    std::map<std::string, std::string> const& placeholders)
{
    std::string const text = PlayerbotTextMgr::instance().GetBotTextOrDefault(
        textId, defaultText, placeholders);

    if (botAI)
        botAI->SayToRaid(text);
}

std::array<ObjectGuid, KALECGOS_TANK_COUNT> GetExpectedKalecgosTankAssignmentGuids(
    Player* player)
{
    std::array<ObjectGuid, KALECGOS_TANK_COUNT> tankGuids =
    {
        ObjectGuid::Empty, ObjectGuid::Empty, ObjectGuid::Empty
    };

    Group* group = player->GetGroup();
    if (!group)
        return tankGuids;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member)
            continue;

        if (PlayerbotAI::IsMainTank(member))
            tankGuids[0] = member->GetGUID();
        else if (PlayerbotAI::IsAssistTankOfIndex(member, 0))
            tankGuids[1] = member->GetGUID();
        else if (PlayerbotAI::IsAssistTankOfIndex(member, 1))
            tankGuids[2] = member->GetGUID();
    }

    return tankGuids;
}

std::array<ObjectGuid, KALECGOS_TANK_COUNT> BuildInitialKalecgosTankPortalRotationGuids(
    std::array<ObjectGuid, KALECGOS_TANK_COUNT> const& tankAssignmentGuids)
{
    std::array<ObjectGuid, KALECGOS_TANK_COUNT> rotationGuids =
    {
        ObjectGuid::Empty, ObjectGuid::Empty, ObjectGuid::Empty
    };
    uint8 nextIndex = 0;

    auto const appendGuid = [&](ObjectGuid guid)
    {
        if (guid == ObjectGuid::Empty || nextIndex >= KALECGOS_TANK_COUNT)
            return;

        if (std::find(rotationGuids.begin(), rotationGuids.end(), guid) == rotationGuids.end())
            rotationGuids[nextIndex++] = guid;
    };

    appendGuid(tankAssignmentGuids[2]);
    appendGuid(tankAssignmentGuids[1]);
    appendGuid(tankAssignmentGuids[0]);

    return rotationGuids;
}

bool HasKalecgosTankAssignment(
    std::array<ObjectGuid, KALECGOS_TANK_COUNT> const& tankAssignmentGuids, ObjectGuid guid)
{
    if (guid == ObjectGuid::Empty)
        return false;

    return std::find(tankAssignmentGuids.begin(), tankAssignmentGuids.end(), guid) !=
        tankAssignmentGuids.end();
}

std::array<ObjectGuid, KALECGOS_TANK_COUNT> RebuildKalecgosTankPortalRotationGuids(
    std::array<ObjectGuid, KALECGOS_TANK_COUNT> const& existingRotationGuids,
    std::array<ObjectGuid, KALECGOS_TANK_COUNT> const& tankAssignmentGuids)
{
    std::array<ObjectGuid, KALECGOS_TANK_COUNT> rotationGuids =
    {
        ObjectGuid::Empty, ObjectGuid::Empty, ObjectGuid::Empty
    };
    uint8 nextIndex = 0;

    auto const appendGuid = [&](ObjectGuid guid)
    {
        if (!HasKalecgosTankAssignment(tankAssignmentGuids, guid) ||
            nextIndex >= KALECGOS_TANK_COUNT)
        {
            return;
        }

        if (std::find(rotationGuids.begin(), rotationGuids.end(), guid) == rotationGuids.end())
            rotationGuids[nextIndex++] = guid;
    };

    for (ObjectGuid guid : existingRotationGuids)
        appendGuid(guid);

    for (ObjectGuid guid : BuildInitialKalecgosTankPortalRotationGuids(tankAssignmentGuids))
        appendGuid(guid);

    return rotationGuids;
}

Player* GetKalecgosSurfaceAssignedTank(Group* group, ObjectGuid guid)
{
    Player* tank = FindKalecgosGroupMember(group, guid);
    if (!tank || !tank->IsAlive() || tank->GetMapId() != SWP_MAP_ID || IsInSpectralRealm(tank))
        return nullptr;

    return tank;
}

Player* GetFirstKalecgosSurfaceTankInOrder(
    Group* group, std::array<ObjectGuid, KALECGOS_TANK_COUNT> const& orderedGuids,
    ObjectGuid firstExcludedGuid = ObjectGuid::Empty,
    ObjectGuid secondExcludedGuid = ObjectGuid::Empty)
{
    for (ObjectGuid guid : orderedGuids)
    {
        if (guid == firstExcludedGuid || guid == secondExcludedGuid)
            continue;

        if (Player* tank = GetKalecgosSurfaceAssignedTank(group, guid))
            return tank;
    }

    return nullptr;
}

Player* GetNextKalecgosSurfaceTankInOrder(
    Group* group, std::array<ObjectGuid, KALECGOS_TANK_COUNT> const& orderedGuids,
    ObjectGuid afterGuid, ObjectGuid excludedGuid = ObjectGuid::Empty, bool fallbackToFirst = false)
{
    uint8 startIndex = 0;
    bool foundAfterGuid = false;

    for (uint8 index = 0; index < KALECGOS_TANK_COUNT; ++index)
    {
        if (orderedGuids[index] == afterGuid)
        {
            startIndex = (index + 1) % KALECGOS_TANK_COUNT;
            foundAfterGuid = true;
            break;
        }
    }

    if (!foundAfterGuid)
    {
        if (fallbackToFirst)
            return GetFirstKalecgosSurfaceTankInOrder(group, orderedGuids, excludedGuid);

        return nullptr;
    }

    for (uint8 offset = 0; offset < KALECGOS_TANK_COUNT; ++offset)
    {
        ObjectGuid const guid = orderedGuids[(startIndex + offset) % KALECGOS_TANK_COUNT];
        if (guid == ObjectGuid::Empty || guid == afterGuid || guid == excludedGuid)
            continue;

        if (Player* tank = GetKalecgosSurfaceAssignedTank(group, guid))
            return tank;
    }

    return nullptr;
}

Player* GetFirstKalecgosSurfaceTankInPortalRotation(
    Group* group, const KalecgosEncounterState& state,
    ObjectGuid firstExcludedGuid = ObjectGuid::Empty,
    ObjectGuid secondExcludedGuid = ObjectGuid::Empty)
{
    if (Player* tank = GetFirstKalecgosSurfaceTankInOrder(
            group, state.tankPortalRotationGuids, firstExcludedGuid, secondExcludedGuid))
    {
        return tank;
    }

    return GetFirstKalecgosSurfaceTankInOrder(
        group, state.tankAssignmentGuids, firstExcludedGuid, secondExcludedGuid);
}

bool ShouldKalecgosCurrentTankHandOff(KalecgosEncounterState const& state)
{
    ObjectGuid const currentTankGuid = state.currentTankGuid;
    if (currentTankGuid == ObjectGuid::Empty)
        return false;

    if (currentTankGuid == state.activeRiftOutgoingTankGuid)
        return true;

    return currentTankGuid == state.blastedPlayerGuid &&
        HasKalecgosTankAssignment(state.tankAssignmentGuids, state.blastedPlayerGuid);
}

Player* GetKalecgosSurfaceTankAfterCurrentHandOff(
    Group* group, KalecgosEncounterState const& state)
{
    if (!ShouldKalecgosCurrentTankHandOff(state))
        return nullptr;

    return GetNextKalecgosSurfaceTankInOrder(
        group, state.tankAssignmentGuids, state.currentTankGuid, ObjectGuid::Empty, true);
}

Player* GetKalecgosBlastAnnouncementCurrentTank(
    Group* group, const KalecgosEncounterState& state)
{
    if (Player* replacementTank = GetKalecgosSurfaceTankAfterCurrentHandOff(group, state))
        return replacementTank;

    return GetKalecgosSurfaceAssignedTank(group, state.currentTankGuid);
}

uint8 CountKalecgosSurfaceAssignedTanks(Group* group, const KalecgosEncounterState& state)
{
    uint8 count = 0;
    for (ObjectGuid guid : state.tankAssignmentGuids)
    {
        if (GetKalecgosSurfaceAssignedTank(group, guid))
            ++count;
    }

    return count;
}

Player* GetKalecgosCurrentVictimTank(
    Player* player, Group* group, const KalecgosEncounterState& state)
{
    Unit* kalecgos = nullptr;

    if (PlayerbotAI* botAI = GET_PLAYERBOT_AI(player))
        kalecgos = botAI->GetAiObjectContext()->GetValue<Unit*>("find target", "kalecgos")->Get();

    if (!kalecgos)
    {
        kalecgos = player->FindNearestCreature(
            static_cast<uint32>(SwpNpcs::NPC_KALECGOS_DRAGON), 200.0f, true);
    }

    if (kalecgos)
    {
        Unit* victim = kalecgos->GetVictim();
        if (victim && victim->IsPlayer())
        {
            Player* currentVictim = victim->ToPlayer();
            if (HasKalecgosTankAssignment(state.tankAssignmentGuids, currentVictim->GetGUID()))
                return currentVictim;
        }
    }

    return GetFirstKalecgosSurfaceTankInOrder(group, state.tankAssignmentGuids);
}

Player* SelectKalecgosOutgoingTankForRift(
    Group* group, const KalecgosEncounterState& state)
{
    if (!state.activeRiftOpenedMs ||
        HasKalecgosTankAssignment(state.tankAssignmentGuids, state.blastedPlayerGuid) ||
        CountKalecgosSurfaceAssignedTanks(group, state) <= 2)
    {
        return nullptr;
    }

    if (Player* nextTank = GetFirstKalecgosSurfaceTankInPortalRotation(group, state);
        nextTank && !IsExhausted(nextTank))
    {
        return nextTank;
    }

    return nullptr;
}

void AssignKalecgosTankTargetsForActiveRift(
    Player* player, Group* group, KalecgosEncounterState& state)
{
    Player* currentTank = GetKalecgosCurrentVictimTank(player, group, state);
    Player* outgoingTank = SelectKalecgosOutgoingTankForRift(group, state);

    state.activeRiftOutgoingTankGuid = outgoingTank ?
        outgoingTank->GetGUID() : ObjectGuid::Empty;

    if (!currentTank)
    {
        currentTank = GetFirstKalecgosSurfaceTankInPortalRotation(
            group, state, state.activeRiftOutgoingTankGuid);
    }

    state.currentTankGuid = currentTank ? currentTank->GetGUID() : ObjectGuid::Empty;
}

void AdvanceKalecgosTankPortalRotation(KalecgosEncounterState& state, ObjectGuid tankGuid)
{
    if (!HasKalecgosTankAssignment(state.tankAssignmentGuids, tankGuid))
        return;

    std::array<ObjectGuid, KALECGOS_TANK_COUNT> rotationGuids =
    {
        ObjectGuid::Empty, ObjectGuid::Empty, ObjectGuid::Empty
    };
    uint8 nextIndex = 0;

    for (ObjectGuid guid : state.tankPortalRotationGuids)
    {
        if (guid == ObjectGuid::Empty || guid == tankGuid)
            continue;

        rotationGuids[nextIndex++] = guid;
    }

    if (nextIndex < KALECGOS_TANK_COUNT)
        rotationGuids[nextIndex] = tankGuid;

    state.tankPortalRotationGuids = RebuildKalecgosTankPortalRotationGuids(
        rotationGuids, state.tankAssignmentGuids);
}

Player* GetKalecgosOutgoingTank(Group* group, const KalecgosEncounterState& state)
{
    if (!state.activeRiftOpenedMs || state.activeRiftOutgoingTankGuid == ObjectGuid::Empty)
        return nullptr;

    return GetKalecgosSurfaceAssignedTank(group, state.activeRiftOutgoingTankGuid);
}

uint8 GetNextAvailableKalecgosGroup(Group* group, const KalecgosEncounterState& state)
{
    if (!group)
        return KALECGOS_INVALID_GROUP;

    for (uint8 groupIndex = 0; groupIndex < KALECGOS_GROUP_COUNT; ++groupIndex)
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || GetKalecgosAssignedGroup(state, member->GetGUID()) != groupIndex)
                continue;

            if (IsKalecgosPortalEligibleCandidate(member) &&
                state.blastedPlayerGuid != member->GetGUID())
            {
                return groupIndex;
            }
        }
    }

    return KALECGOS_INVALID_GROUP;
}

uint8 ResolveKalecgosActiveRiftGroup(Group* group, const KalecgosEncounterState& state)
{
    if (state.blastedPlayerGuid != ObjectGuid::Empty)
    {
        uint8 const blastedGroup = GetKalecgosAssignedGroup(state, state.blastedPlayerGuid);
        if (blastedGroup != KALECGOS_INVALID_GROUP)
            return blastedGroup;

        return GetNextAvailableKalecgosGroup(group, state);
    }

    if (state.firstEntrantGuid != ObjectGuid::Empty)
    {
        uint8 const entrantGroup = GetKalecgosAssignedGroup(state, state.firstEntrantGuid);
        if (entrantGroup != KALECGOS_INVALID_GROUP)
            return entrantGroup;

        return GetNextAvailableKalecgosGroup(group, state);
    }

    return KALECGOS_INVALID_GROUP;
}

void AssignPlayerToGroup(
    KalecgosEncounterState& state, std::array<size_t, KALECGOS_GROUP_COUNT>& groupSizes,
    std::array<bool, KALECGOS_GROUP_COUNT>& groupHasTank,
    std::array<bool, KALECGOS_GROUP_COUNT>& groupHasDecurser,
    Player* member, uint8 groupIndex)
{
    if (!member || groupIndex >= KALECGOS_GROUP_COUNT)
        return;

    state.playerToGroup[member->GetGUID()] = groupIndex;
    ++groupSizes[groupIndex];

    if (GET_PLAYERBOT_AI(member))
    {
        groupHasTank[groupIndex] = groupHasTank[groupIndex] || PlayerbotAI::IsTank(member);
        groupHasDecurser[groupIndex] = groupHasDecurser[groupIndex] || IsKalecgosDecurser(member);
    }
}

uint8 GetLeastFilledGroup(
    std::array<size_t, KALECGOS_GROUP_COUNT> const& groupSizes,
    std::array<bool, KALECGOS_GROUP_COUNT> const* requiredFlags = nullptr,
    bool preferMissingFlag = false)
{
    uint8 bestGroup = KALECGOS_INVALID_GROUP;
    size_t smallestSize = std::numeric_limits<size_t>::max();

    for (uint8 groupIndex = 0; groupIndex < KALECGOS_GROUP_COUNT; ++groupIndex)
    {
        if (requiredFlags && preferMissingFlag && (*requiredFlags)[groupIndex])
            continue;

        if (groupSizes[groupIndex] < smallestSize)
        {
            bestGroup = groupIndex;
            smallestSize = groupSizes[groupIndex];
        }
    }

    if (bestGroup != KALECGOS_INVALID_GROUP || !requiredFlags || !preferMissingFlag)
        return bestGroup;

    return GetLeastFilledGroup(groupSizes);
}

} // end anonymous namespace

bool IsExhausted(Player* bot)
{
    return bot->HasAura(static_cast<uint32>(SwpSpells::SPELL_SPECTRAL_EXHAUSTION));
}

bool IsInSpectralRealm(Player* bot)
{
    return bot->HasAura(static_cast<uint32>(SwpSpells::SPELL_SPECTRAL_REALM));
}

bool IsKalecgosDecurser(Player* bot)
{
    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
    switch (bot->getClass())
    {
        case CLASS_MAGE:
        case CLASS_SHAMAN:
            break;
        case CLASS_DRUID:
            if (!botAI->IsRanged(bot))
                return false;
            break;
        default:
            return false;
    }

    return botAI->HasStrategy("cure", BOT_STATE_COMBAT);
}

void EnsureKalecgosGroupAssignments(Player* player)
{
    Group* group = player->GetGroup();
    if (!group || player->GetMapId() != SWP_MAP_ID)
        return;

    KalecgosEncounterState& state = kalecgosEncounterStates[player->GetInstanceId()];
    std::vector<Player*> botMembers;
    std::array<ObjectGuid, KALECGOS_TANK_COUNT> const expectedTankAssignmentGuids =
        GetExpectedKalecgosTankAssignmentGuids(player);

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member->GetMapId() != SWP_MAP_ID)
            continue;

        if (GET_PLAYERBOT_AI(member) && !PlayerbotAI::IsTank(member))
            botMembers.push_back(member);
    }

    bool needsRebuild = state.playerToGroup.size() != botMembers.size() ||
        state.tankAssignmentGuids != expectedTankAssignmentGuids;
    if (!needsRebuild)
    {
        for (Player* member : botMembers)
        {
            if (state.playerToGroup.find(member->GetGUID()) == state.playerToGroup.end())
            {
                needsRebuild = true;
                break;
            }
        }
    }

    if (!needsRebuild)
        return;

    state.playerToGroup.clear();
    state.tankAssignmentGuids = expectedTankAssignmentGuids;
    state.tankPortalRotationGuids = RebuildKalecgosTankPortalRotationGuids(
        state.tankPortalRotationGuids, state.tankAssignmentGuids);

    if (!HasKalecgosTankAssignment(state.tankAssignmentGuids, state.currentTankGuid))
    {
        if (Player* fallbackTank = GetKalecgosCurrentVictimTank(player, group, state))
            state.currentTankGuid = fallbackTank->GetGUID();
        else
            state.currentTankGuid = ObjectGuid::Empty;
    }

    if (!HasKalecgosTankAssignment(state.tankAssignmentGuids, state.activeRiftOutgoingTankGuid))
        state.activeRiftOutgoingTankGuid = ObjectGuid::Empty;

    std::array<size_t, KALECGOS_GROUP_COUNT> groupSizes = { 0, 0, 0, 0 };
    std::array<bool, KALECGOS_GROUP_COUNT> groupHasTank = { false, false, false, false };
    std::array<bool, KALECGOS_GROUP_COUNT> groupHasDecurser = { false, false, false, false };

    std::vector<Player*> decursers;
    std::vector<Player*> healers;
    std::vector<Player*> rangedDps;
    std::vector<Player*> meleeDps;
    std::vector<Player*> others;

    for (Player* member : botMembers)
    {
        if (state.playerToGroup.find(member->GetGUID()) != state.playerToGroup.end())
            continue;

        if (IsKalecgosDecurser(member))
            decursers.push_back(member);
        else if (PlayerbotAI::IsHeal(member))
            healers.push_back(member);
        else if (PlayerbotAI::IsRangedDps(member))
            rangedDps.push_back(member);
        else if (PlayerbotAI::IsMelee(member) && PlayerbotAI::IsDps(member))
            meleeDps.push_back(member);
        else
            others.push_back(member);
    }

    for (Player* decurser : decursers)
    {
        uint8 groupIndex = GetLeastFilledGroup(groupSizes, &groupHasDecurser, true);
        AssignPlayerToGroup(
            state, groupSizes, groupHasTank, groupHasDecurser, decurser, groupIndex);
    }

    for (Player* healer : healers)
        AssignPlayerToGroup(
            state, groupSizes, groupHasTank, groupHasDecurser,
            healer, GetLeastFilledGroup(groupSizes));

    for (Player* ranged : rangedDps)
        AssignPlayerToGroup(
            state, groupSizes, groupHasTank, groupHasDecurser,
            ranged, GetLeastFilledGroup(groupSizes));

    for (Player* melee : meleeDps)
        AssignPlayerToGroup(
            state, groupSizes, groupHasTank, groupHasDecurser,
            melee, GetLeastFilledGroup(groupSizes));

    for (Player* other : others)
        AssignPlayerToGroup(
            state, groupSizes, groupHasTank, groupHasDecurser,
            other, GetLeastFilledGroup(groupSizes));

    if (state.activeRiftGroup == KALECGOS_INVALID_GROUP)
        state.activeRiftGroup = ResolveKalecgosActiveRiftGroup(group, state);
}

Player* GetKalecgosCurrentTank(Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    KalecgosEncounterState& state = GetPreparedKalecgosEncounterState(bot);

    if (Player* tank = GetKalecgosSurfaceAssignedTank(group, state.currentTankGuid))
    {
        if (Player* replacementTank = GetKalecgosSurfaceTankAfterCurrentHandOff(group, state))
            return replacementTank;

        return tank;
    }

    if (Player* fallbackTank = GetKalecgosCurrentVictimTank(bot, group, state))
    {
        state.currentTankGuid = fallbackTank->GetGUID();
        return fallbackTank;
    }

    state.currentTankGuid = ObjectGuid::Empty;
    return nullptr;
}

Player* GetKalecgosReplacementTank(Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    KalecgosEncounterState& state = GetPreparedKalecgosEncounterState(bot);
    Player* currentTank = GetKalecgosSurfaceAssignedTank(group, state.currentTankGuid);
    if (!currentTank)
        currentTank = GetKalecgosCurrentVictimTank(bot, group, state);

    if (!currentTank)
        return nullptr;

    return GetNextKalecgosSurfaceTankInOrder(
        group, state.tankAssignmentGuids, currentTank->GetGUID(), ObjectGuid::Empty, true);
}

bool ShouldEnterKalecgosSpectralRift(Player* bot)
{
    if (!IsKalecgosPortalEligibleCandidate(bot))
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    KalecgosEncounterState& state = GetPreparedKalecgosEncounterState(bot);
    if (!state.activeRiftOpenedMs)
        return false;

    if (HasKalecgosTankAssignment(state.tankAssignmentGuids, bot->GetGUID()))
    {
        return GetKalecgosOutgoingTank(group, state) == bot &&
            state.blastedPlayerGuid != bot->GetGUID();
    }

    if (!IsKalecgosActiveRiftCandidate(bot, state))
        return false;

    return state.blastedPlayerGuid != bot->GetGUID();
}

void RecordKalecgosSpectralBlastTarget(Player* player, PlayerbotAI* announcerAI)
{
    Group* group = player->GetGroup();
    if (!group || player->GetMapId() != SWP_MAP_ID)
        return;

    KalecgosEncounterState& state = GetPreparedKalecgosEncounterState(player);
    uint32 const now = getMSTime();

    state.activeRiftOpenedMs = now;
    state.blastedPlayerGuid = player->GetGUID();
    state.firstEntrantGuid = ObjectGuid::Empty;
    state.activeRiftGroup = ResolveKalecgosActiveRiftGroup(group, state);
    AssignKalecgosTankTargetsForActiveRift(player, group, state);

    Player* currentTank = GetKalecgosBlastAnnouncementCurrentTank(group, state);

    if (HasKalecgosTankAssignment(state.tankAssignmentGuids, player->GetGUID()))
    {
        if (!currentTank)
            return;

        AnnounceKalecgosTankTransition(
            announcerAI, "kalecgos_tank_sent_to_spectral_realm",
            "Tank %tank has been sent to the Spectral Realm. The active Kalecgos tank is %current.",
            {
                {"%tank", player->GetName()},
                {"%current", currentTank->GetName()}
            });

        return;
    }

    if (Player* outgoingTank = GetKalecgosOutgoingTank(group, state);
        outgoingTank && currentTank)
    {
        AnnounceKalecgosTankTransition(
            announcerAI, "kalecgos_tank_should_enter_spectral_realm",
            "Tank %tank should enter the Spectral Realm. The active Kalecgos tank is %current.",
            {
                {"%tank", outgoingTank->GetName()},
                {"%current", currentTank->GetName()}
            });
    }
}

void RecordKalecgosSpectralRealmEnter(Player* player)
{
    Group* group = player->GetGroup();
    if (!group || player->GetMapId() != SWP_MAP_ID)
        return;

    KalecgosEncounterState& state = GetPreparedKalecgosEncounterState(player);
    ObjectGuid const guid = player->GetGUID();
    uint32 const now = getMSTime();
    Player* replacementTank = nullptr;
    bool const wasCurrentTank = state.currentTankGuid == guid;

    if (wasCurrentTank)
    {
        replacementTank = GetNextKalecgosSurfaceTankInOrder(
            group, state.tankAssignmentGuids, guid, state.activeRiftOutgoingTankGuid, true);
    }

    UpdateKalecgosRealmState(player, true, now);

    if (state.activeRiftOpenedMs)
    {
        if (state.firstEntrantGuid == ObjectGuid::Empty)
            state.firstEntrantGuid = guid;

        if (state.activeRiftGroup == KALECGOS_INVALID_GROUP)
            state.activeRiftGroup = ResolveKalecgosActiveRiftGroup(group, state);
    }

    if (HasKalecgosTankAssignment(state.tankAssignmentGuids, player->GetGUID()))
    {
        AdvanceKalecgosTankPortalRotation(state, guid);

        if (state.activeRiftOutgoingTankGuid == guid)
            state.activeRiftOutgoingTankGuid = ObjectGuid::Empty;

        if (wasCurrentTank)
        {
            state.currentTankGuid =
                replacementTank ? replacementTank->GetGUID() : ObjectGuid::Empty;
        }
    }
}

void UpdateKalecgosRealmState(Player* bot, bool inSpectralRealm, uint32 timestamp)
{
    KalecgosRealmState& realmState = kalecgosRealmStates[bot->GetGUID()];
    realmState.inSpectralRealm = inSpectralRealm;

    if (inSpectralRealm)
        realmState.lastEnterMs = timestamp;
    else
        realmState.lastExitMs = timestamp;
}

}
