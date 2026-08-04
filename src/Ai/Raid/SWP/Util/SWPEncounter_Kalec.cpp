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

std::unordered_map<uint32, KalecgosEncounterState> kalecgosEncounterStates;
std::unordered_map<ObjectGuid, KalecgosRealmState> kalecgosRealmStates;

namespace
{

void ClearExpiredActiveRift(KalecgosEncounterState& state, uint32 now)
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

uint8 GetAssignedGroup(const KalecgosEncounterState& state, ObjectGuid playerGuid)
{
    auto const assignment = state.playerToGroup.find(playerGuid);
    return assignment != state.playerToGroup.end() ? assignment->second : KALECGOS_INVALID_GROUP;
}

KalecgosEncounterState& GetPreparedEncounterState(Player* player)
{
    KalecgosEncounterState& state = kalecgosEncounterStates[player->GetInstanceId()];
    if (!state.encounterStartMs)
        state.encounterStartMs = getMSTime();

    ClearExpiredActiveRift(state, getMSTime());
    EnsureKalecgosRaidAssignments(player);
    return state;
}

bool IsActivePortalCandidate(Player* bot, const KalecgosEncounterState& state)
{
    if (!state.activeRiftOpenedMs || state.activeRiftGroup == KALECGOS_INVALID_GROUP)
        return false;

    if (state.blastedPlayerGuid == bot->GetGUID())
        return true;

    return GetAssignedGroup(state, bot->GetGUID()) == state.activeRiftGroup;
}

bool IsPortalEligibleCandidate(Player* bot)
{
    if (!bot->IsAlive() || !GET_PLAYERBOT_AI(bot) || bot->GetMapId() != SWP_MAP_ID)
        return false;

    return !IsExhausted(bot) && !IsInSpectralRealm(bot);
}

void AnnounceTankTransition(
    PlayerbotAI* botAI, std::string const& textId, std::string const& defaultText,
    std::map<std::string, std::string> const& placeholders)
{
    std::string const text = PlayerbotTextMgr::instance().GetBotTextOrDefault(
        textId, defaultText, placeholders);

    if (botAI)
        botAI->SayToRaid(text);
}

std::array<ObjectGuid, KALECGOS_TANK_COUNT> GetExpectedTankAssignmentGuids(Player* player)
{
    std::array<ObjectGuid, KALECGOS_TANK_COUNT> tankGuids = {
        ObjectGuid::Empty, ObjectGuid::Empty, ObjectGuid::Empty };

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

std::array<ObjectGuid, KALECGOS_TANK_COUNT> BuildInitialTankPortalRotationGuids(
    std::array<ObjectGuid, KALECGOS_TANK_COUNT> const& tankAssignmentGuids)
{
    std::array<ObjectGuid, KALECGOS_TANK_COUNT> rotationGuids = {
        ObjectGuid::Empty, ObjectGuid::Empty, ObjectGuid::Empty };

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

std::array<ObjectGuid, KALECGOS_TANK_COUNT> RebuildTankPortalRotationGuids(
    std::array<ObjectGuid, KALECGOS_TANK_COUNT> const& existingRotationGuids,
    std::array<ObjectGuid, KALECGOS_TANK_COUNT> const& tankAssignmentGuids)
{
    std::array<ObjectGuid, KALECGOS_TANK_COUNT> rotationGuids = {
        ObjectGuid::Empty, ObjectGuid::Empty, ObjectGuid::Empty };

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

    for (ObjectGuid guid : BuildInitialTankPortalRotationGuids(tankAssignmentGuids))
        appendGuid(guid);

    return rotationGuids;
}

Player* ResolveSurfaceTank(Group* group, ObjectGuid guid)
{
    if (guid == ObjectGuid::Empty || !group)
        return nullptr;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member->GetGUID() != guid)
            continue;

        if (member->GetMapId() != SWP_MAP_ID || !member->IsAlive() ||
            IsInSpectralRealm(member))
        {
            return nullptr;
        }

        return member;
    }

    return nullptr;
}

Player* GetFirstResolvedSurfaceTank(
    Group* group, std::array<ObjectGuid, KALECGOS_TANK_COUNT> const& orderedGuids,
    ObjectGuid firstExcludedGuid = ObjectGuid::Empty,
    ObjectGuid secondExcludedGuid = ObjectGuid::Empty)
{
    for (ObjectGuid guid : orderedGuids)
    {
        if (guid == firstExcludedGuid || guid == secondExcludedGuid)
            continue;

        if (Player* tank = ResolveSurfaceTank(group, guid))
            return tank;
    }

    return nullptr;
}

Player* GetNextSurfaceTankForPortal(
    Group* group, const KalecgosEncounterState& state,
    ObjectGuid firstExcludedGuid = ObjectGuid::Empty,
    ObjectGuid secondExcludedGuid = ObjectGuid::Empty)
{
    if (Player* tank = GetFirstResolvedSurfaceTank(
            group, state.tankPortalRotationGuids, firstExcludedGuid, secondExcludedGuid))
    {
        return tank;
    }

    return GetFirstResolvedSurfaceTank(
        group, state.tankAssignmentGuids, firstExcludedGuid, secondExcludedGuid);
}

Player* GetSurfaceTankAfterCurrentHandOff(Group* group, KalecgosEncounterState const& state)
{
    ObjectGuid const currentTankGuid = state.currentTankGuid;
    if (currentTankGuid == ObjectGuid::Empty)
        return nullptr;

    if (currentTankGuid != state.activeRiftOutgoingTankGuid &&
        (currentTankGuid != state.blastedPlayerGuid ||
         !HasKalecgosTankAssignment(state.tankAssignmentGuids, state.blastedPlayerGuid)))
    {
        return nullptr;
    }

    return GetNextSurfaceTankInOrder(
        group, state.tankAssignmentGuids, currentTankGuid, ObjectGuid::Empty, true);
}

Player* GetKalecgosCurrentVictimTank(
    Player* player, Group* group, const KalecgosEncounterState& state)
{
    Unit* kalecgos = nullptr;

    if (PlayerbotAI* botAI = GET_PLAYERBOT_AI(player))
        kalecgos = botAI->GetAiObjectContext()->GetValue<Unit*>("find target", "kalecgos")->Get();

    if (!kalecgos)
        kalecgos = player->FindNearestCreature(Id(SwpNpcs::NPC_KALECGOS_DRAGON), 200.0f, true);

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

    return GetFirstResolvedSurfaceTank(group, state.tankAssignmentGuids);
}

Player* SelectOutgoingTankForRift(Group* group, const KalecgosEncounterState& state)
{
    if (!state.activeRiftOpenedMs ||
        HasKalecgosTankAssignment(state.tankAssignmentGuids, state.blastedPlayerGuid))
    {
        return nullptr;
    }

    uint8 surfaceTankCount = 0;
    for (ObjectGuid guid : state.tankAssignmentGuids)
    {
        if (ResolveSurfaceTank(group, guid))
            ++surfaceTankCount;
    }
    if (surfaceTankCount <= 1)
        return nullptr;

    if (Player* nextTank = GetNextSurfaceTankForPortal(group, state);
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
    Player* outgoingTank = SelectOutgoingTankForRift(group, state);

    state.activeRiftOutgoingTankGuid = outgoingTank ?
        outgoingTank->GetGUID() : ObjectGuid::Empty;

    if (!currentTank)
    {
        currentTank = GetNextSurfaceTankForPortal(
            group, state, state.activeRiftOutgoingTankGuid);
    }

    state.currentTankGuid = currentTank ? currentTank->GetGUID() : ObjectGuid::Empty;
}

void AdvanceKalecgosTankPortalRotation(KalecgosEncounterState& state, ObjectGuid tankGuid)
{
    if (!HasKalecgosTankAssignment(state.tankAssignmentGuids, tankGuid))
        return;

    std::array<ObjectGuid, KALECGOS_TANK_COUNT> rotationGuids = {
        ObjectGuid::Empty, ObjectGuid::Empty, ObjectGuid::Empty };

    uint8 nextIndex = 0;

    for (ObjectGuid guid : state.tankPortalRotationGuids)
    {
        if (guid == ObjectGuid::Empty || guid == tankGuid)
            continue;

        rotationGuids[nextIndex++] = guid;
    }

    if (nextIndex < KALECGOS_TANK_COUNT)
        rotationGuids[nextIndex] = tankGuid;

    state.tankPortalRotationGuids = RebuildTankPortalRotationGuids(
        rotationGuids, state.tankAssignmentGuids);
}

uint8 GetNextAvailablePortalGroup(Group* group, const KalecgosEncounterState& state)
{
    if (!group)
        return KALECGOS_INVALID_GROUP;

    for (uint8 groupIndex = 0; groupIndex < KALECGOS_GROUP_COUNT; ++groupIndex)
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || GetAssignedGroup(state, member->GetGUID()) != groupIndex)
                continue;

            if (IsPortalEligibleCandidate(member) &&
                state.blastedPlayerGuid != member->GetGUID())
            {
                return groupIndex;
            }
        }
    }

    return KALECGOS_INVALID_GROUP;
}

uint8 ResolveActivePortalGroup(Group* group, const KalecgosEncounterState& state)
{
    if (state.blastedPlayerGuid != ObjectGuid::Empty)
    {
        uint8 const blastedGroup = GetAssignedGroup(state, state.blastedPlayerGuid);
        if (blastedGroup != KALECGOS_INVALID_GROUP)
            return blastedGroup;

        return GetNextAvailablePortalGroup(group, state);
    }

    if (state.firstEntrantGuid != ObjectGuid::Empty)
    {
        uint8 const entrantGroup = GetAssignedGroup(state, state.firstEntrantGuid);
        if (entrantGroup != KALECGOS_INVALID_GROUP)
            return entrantGroup;

        return GetNextAvailablePortalGroup(group, state);
    }

    return KALECGOS_INVALID_GROUP;
}

void AssignPlayerToGroup(
    KalecgosEncounterState& state, std::array<size_t, KALECGOS_GROUP_COUNT>& groupSizes,
    std::array<bool, KALECGOS_GROUP_COUNT>& groupHasDecurser,
    Player* member, uint8 groupIndex)
{
    if (!member || groupIndex >= KALECGOS_GROUP_COUNT)
        return;

    state.playerToGroup[member->GetGUID()] = groupIndex;
    ++groupSizes[groupIndex];

    if (GET_PLAYERBOT_AI(member))
        groupHasDecurser[groupIndex] = groupHasDecurser[groupIndex] || IsKalecgosDecurser(member);
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

Player* GetNextSurfaceTankInOrder(
    Group* group, std::array<ObjectGuid, KALECGOS_TANK_COUNT> const& orderedGuids,
    ObjectGuid afterGuid, ObjectGuid excludedGuid, bool fallbackToFirst)
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
            return GetFirstResolvedSurfaceTank(group, orderedGuids, excludedGuid);

        return nullptr;
    }

    for (uint8 offset = 0; offset < KALECGOS_TANK_COUNT; ++offset)
    {
        ObjectGuid const guid = orderedGuids[(startIndex + offset) % KALECGOS_TANK_COUNT];
        if (guid == ObjectGuid::Empty || guid == afterGuid || guid == excludedGuid)
            continue;

        if (Player* tank = ResolveSurfaceTank(group, guid))
            return tank;
    }

    return nullptr;
}

bool IsExhausted(Player* bot)
{
    return bot->HasAura(Id(SwpSpells::SPELL_SPECTRAL_EXHAUSTION));
}

bool IsInSpectralRealm(Player* bot)
{
    return bot->HasAura(Id(SwpSpells::SPELL_SPECTRAL_REALM));
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
            if (!PlayerbotAI::IsRanged(bot))
                return false;
            break;
        default:
            return false;
    }

    return botAI->HasStrategy("cure", BOT_STATE_COMBAT);
}

void EnsureKalecgosRaidAssignments (Player* player)
{
    Group* group = player->GetGroup();
    if (!group || player->GetMapId() != SWP_MAP_ID)
        return;

    KalecgosEncounterState& state = kalecgosEncounterStates[player->GetInstanceId()];
    std::vector<Player*> botMembers;
    std::array<ObjectGuid, KALECGOS_TANK_COUNT> const expectedTankAssignmentGuids =
        GetExpectedTankAssignmentGuids(player);

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
    state.tankPortalRotationGuids = RebuildTankPortalRotationGuids(
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
        AssignPlayerToGroup(state, groupSizes, groupHasDecurser, decurser, groupIndex);
    }

    for (Player* healer : healers)
        AssignPlayerToGroup(
            state, groupSizes, groupHasDecurser, healer, GetLeastFilledGroup(groupSizes));

    for (Player* ranged : rangedDps)
        AssignPlayerToGroup(
            state, groupSizes, groupHasDecurser, ranged, GetLeastFilledGroup(groupSizes));

    for (Player* melee : meleeDps)
        AssignPlayerToGroup(
            state, groupSizes, groupHasDecurser, melee, GetLeastFilledGroup(groupSizes));

    for (Player* other : others)
        AssignPlayerToGroup(
            state, groupSizes, groupHasDecurser, other, GetLeastFilledGroup(groupSizes));

    if (state.activeRiftGroup == KALECGOS_INVALID_GROUP)
        state.activeRiftGroup = ResolveActivePortalGroup(group, state);
}

Player* GetKalecgosDesignatedTank(Player* player)
{
    Group* group = player->GetGroup();
    if (!group)
        return nullptr;

    KalecgosEncounterState& state = GetPreparedEncounterState(player);

    if (Player* tank = ResolveSurfaceTank(group, state.currentTankGuid))
    {
        if (Player* replacementTank = GetSurfaceTankAfterCurrentHandOff(group, state))
            return replacementTank;

        return tank;
    }

    if (Player* fallbackTank = GetKalecgosCurrentVictimTank(player, group, state))
    {
        state.currentTankGuid = fallbackTank->GetGUID();
        return fallbackTank;
    }

    state.currentTankGuid = ObjectGuid::Empty;
    return nullptr;
}

bool ShouldEnterKalecgosPortal(Player* bot)
{
    if (!IsPortalEligibleCandidate(bot))
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    KalecgosEncounterState& state = GetPreparedEncounterState(bot);
    if (!state.activeRiftOpenedMs)
        return false;

    if (HasKalecgosTankAssignment(state.tankAssignmentGuids, bot->GetGUID()))
    {
        return ResolveSurfaceTank(group, state.activeRiftOutgoingTankGuid) == bot &&
            state.blastedPlayerGuid != bot->GetGUID();
    }

    if (!IsActivePortalCandidate(bot, state))
        return false;

    return state.blastedPlayerGuid != bot->GetGUID();
}

void RecordSpectralBlastTarget(Player* player, PlayerbotAI* announcerAI)
{
    Group* group = player->GetGroup();
    if (!group || player->GetMapId() != SWP_MAP_ID)
        return;

    KalecgosEncounterState& state = GetPreparedEncounterState(player);
    uint32 const now = getMSTime();

    state.activeRiftOpenedMs = now;
    state.blastedPlayerGuid = player->GetGUID();
    state.firstEntrantGuid = ObjectGuid::Empty;
    state.activeRiftGroup = ResolveActivePortalGroup(group, state);
    AssignKalecgosTankTargetsForActiveRift(player, group, state);

    Player* currentTank = GetSurfaceTankAfterCurrentHandOff(group, state);
    if (!currentTank)
        currentTank = ResolveSurfaceTank(group, state.currentTankGuid);

    if (HasKalecgosTankAssignment(state.tankAssignmentGuids, player->GetGUID()))
    {
        if (!currentTank)
            return;

        AnnounceTankTransition(
            announcerAI, "kalecgos_tank_sent_to_spectral_realm",
            "Tank %tank has been sent to the Spectral Realm. The active Kalecgos tank is %current.",
            {
                {"%tank", player->GetName()},
                {"%current", currentTank->GetName()}
            });

        return;
    }

    if (Player* outgoingTank = ResolveSurfaceTank(group, state.activeRiftOutgoingTankGuid);
        outgoingTank && currentTank)
    {
        AnnounceTankTransition(
            announcerAI, "kalecgos_tank_should_enter_spectral_realm",
            "Tank %tank should enter the Spectral Realm. The active Kalecgos tank is %current.",
            {
                {"%tank", outgoingTank->GetName()},
                {"%current", currentTank->GetName()}
            });
    }
}

void RecordSpectralRealmEnter(Player* player)
{
    Group* group = player->GetGroup();
    if (!group || player->GetMapId() != SWP_MAP_ID)
        return;

    KalecgosEncounterState& state = GetPreparedEncounterState(player);
    ObjectGuid const guid = player->GetGUID();
    bool const wasCurrentTank = state.currentTankGuid == guid;
    Player* replacementTank = nullptr;

    if (wasCurrentTank)
    {
        replacementTank = GetNextSurfaceTankInOrder(
            group, state.tankAssignmentGuids, guid, state.activeRiftOutgoingTankGuid, true);
    }

    uint32 const now = getMSTime();
    UpdateKalecgosRealmState(player, true, now);

    if (state.activeRiftOpenedMs)
    {
        if (state.firstEntrantGuid == ObjectGuid::Empty)
            state.firstEntrantGuid = guid;

        if (state.activeRiftGroup == KALECGOS_INVALID_GROUP)
            state.activeRiftGroup = ResolveActivePortalGroup(group, state);
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
