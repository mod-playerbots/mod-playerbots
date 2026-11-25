/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "SWPEncounter_Kalec.h"
#include "EncounterHelpers.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "PlayerbotTextMgr.h"
#include "SWPInstanceState.h"
#include "ThreatHelpers.h"
#include <algorithm>
#include <map>
#include <string>
#include <vector>

namespace SwpHelpers
{

// Note: Kalecgos's CombatReach is 10.5f, and Sathrovarr's CombatReach is 4.5f.
// Note: Kalecgos remains on player threat lists for the duration of the encounter, even for
// players in the Spectral Realm and after he turns friendly after Sathrovarr is killed.

namespace
{

bool IsActiveRiftOpen(KalecgosEncounterState const& state)
{
    return state.activeRiftOpenedMs &&
        getMSTimeDiff(state.activeRiftOpenedMs, getMSTime()) <= SPECTRAL_RIFT_ACTIVE_WINDOW_MS;
}

bool ClearExpiredActiveRift(KalecgosEncounterState& state)
{
    if (!state.activeRiftOpenedMs || IsActiveRiftOpen(state))
        return false;

    state.activeRiftOpenedMs = 0;
    state.activeRiftGroup = KALECGOS_INVALID_GROUP;
    state.blastedPlayerGuid = ObjectGuid::Empty;
    state.activeRiftOutgoingTankGuid = ObjectGuid::Empty;
    return true;
}

uint8 GetAssignedGroup(KalecgosEncounterState const& state, ObjectGuid playerGuid)
{
    auto const assignment = state.playerToGroup.find(playerGuid);
    return assignment != state.playerToGroup.end() ? assignment->second : KALECGOS_INVALID_GROUP;
}

// For the spell listener, which can't wait for the tracker's next update.
KalecgosEncounterState& GetPreparedEncounterState(Player* player)
{
    UpdateKalecgosEncounterState(player);
    return KalecgosState(player->GetInstanceId());
}

// Exhaustion must end before what is left of the rift's life, not its whole life.
bool CanReachPortalBeforeExpiry(Player* player, KalecgosEncounterState const& state)
{
    Aura const* exhaustion = player->GetAura(Id(SwpSpells::SPELL_SPECTRAL_EXHAUSTION));
    if (!exhaustion)
        return true;

    uint32 const riftAgeMs = state.activeRiftOpenedMs ?
        getMSTimeDiff(state.activeRiftOpenedMs, getMSTime()) : 0;
    int32 const riftRemainingMs = static_cast<int32>(SPECTRAL_RIFT_ACTIVE_WINDOW_MS) -
        static_cast<int32>(riftAgeMs) - SPECTRAL_RIFT_ENTRY_MARGIN_MS;
    return exhaustion->GetDuration() <= riftRemainingMs;
}

bool IsPortalCandidate(Player* player)
{
    if (!player->IsAlive() || player->GetMapId() != SWP_MAP_ID || !GET_PLAYERBOT_AI(player))
        return false;

    return !IsInSpectralRealm(player);
}

bool IsPortalEligibleCandidate(Player* player, KalecgosEncounterState const& state)
{
    return IsPortalCandidate(player) && CanReachPortalBeforeExpiry(player, state);
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

    ObjectGuid const mainTankGuid = PlayerbotAI::GetMainTankGuid(group);

    Player* mainTank = nullptr;
    std::vector<Player*> assistTanks;
    std::vector<Player*> otherTanks;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member)
            continue;

        if (member->GetGUID() == mainTankGuid)
        {
            mainTank = member;
            continue;
        }

        if (!PlayerbotAI::IsTank(member))
            continue;

        if (group->IsAssistant(member->GetGUID()))
            assistTanks.push_back(member);
        else
            otherTanks.push_back(member);
    }

    assistTanks.insert(assistTanks.end(), otherTanks.begin(), otherTanks.end());

    auto const assignSlot = [&tankGuids](uint8 slot, Player* tank)
    {
        if (tank && tank->GetMapId() == SWP_MAP_ID)
            tankGuids[slot] = tank->GetGUID();
    };

    assignSlot(0, mainTank);
    for (uint8 index = 0; index < KALECGOS_TANK_COUNT - 1 && index < assistTanks.size(); ++index)
        assignSlot(index + 1, assistTanks[index]);

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
    if (!group || guid == ObjectGuid::Empty)
        return nullptr;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member->GetGUID() != guid)
            continue;

        if (!member->IsAlive() || member->GetMapId() != SWP_MAP_ID || IsInSpectralRealm(member))
            return nullptr;

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
    Group* group, KalecgosEncounterState const& state,
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

// The next tank in the rotation is the surface tank that has been out of the Spectral Realm
// longest.
Player* GetReplacementSurfaceTank(
    Group* group, KalecgosEncounterState const& state, ObjectGuid departingGuid,
    ObjectGuid excludedGuid = ObjectGuid::Empty)
{
    if (Player* replacement = GetFirstResolvedSurfaceTank(
            group, state.tankPortalRotationGuids, departingGuid, excludedGuid))
    {
        return replacement;
    }

    return GetFirstResolvedSurfaceTank(
        group, state.tankAssignmentGuids, departingGuid, excludedGuid);
}

Player* GetSurfaceTankAfterCurrentHandOff(Group* group, KalecgosEncounterState const& state)
{
    ObjectGuid const currentTankGuid = state.currentTankGuid;
    if (currentTankGuid == ObjectGuid::Empty)
        return nullptr;

    if (currentTankGuid != state.activeRiftOutgoingTankGuid &&
        currentTankGuid != state.blastedPlayerGuid)
    {
        return nullptr;
    }

    return GetReplacementSurfaceTank(group, state, currentTankGuid);
}

Player* GetKalecgosCurrentVictimTank(
    Player* player, Group* group, KalecgosEncounterState const& state)
{
    Unit* kalecgos = GetSwpBoss(player, SwpBosses::DATA_KALECGOS);

    if (!kalecgos)
        kalecgos = GetKalecgosUnit(player, SwpNpcs::NPC_KALECGOS_DRAGON);

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

Player* ResolveKalecgosDesignatedTank(
    Player* player, Group* group, KalecgosEncounterState const& state)
{
    if (Player* tank = ResolveSurfaceTank(group, state.currentTankGuid))
    {
        if (Player* replacementTank = GetSurfaceTankAfterCurrentHandOff(group, state))
            return replacementTank;

        return tank;
    }

    return GetKalecgosCurrentVictimTank(player, group, state);
}

Player* SelectOutgoingTankForRift(Group* group, KalecgosEncounterState const& state)
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

bool GroupHasEligibleEntrant(Group* group, KalecgosEncounterState const& state, uint8 groupIndex)
{
    if (!group || groupIndex >= KALECGOS_GROUP_COUNT)
        return false;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || GetAssignedGroup(state, member->GetGUID()) != groupIndex)
            continue;

        if (IsPortalEligibleCandidate(member, state) &&
            state.blastedPlayerGuid != member->GetGUID())
            return true;
    }

    return false;
}

uint8 GetNextAvailablePortalGroup(Group* group, KalecgosEncounterState const& state)
{
    for (uint8 groupIndex = 0; groupIndex < KALECGOS_GROUP_COUNT; ++groupIndex)
    {
        if (GroupHasEligibleEntrant(group, state, groupIndex))
            return groupIndex;
    }

    return KALECGOS_INVALID_GROUP;
}

uint8 ResolveActivePortalGroup(Group* group, KalecgosEncounterState const& state)
{
    if (state.blastedPlayerGuid == ObjectGuid::Empty)
        return KALECGOS_INVALID_GROUP;

    uint8 const blastedGroup = GetAssignedGroup(state, state.blastedPlayerGuid);
    if (blastedGroup != KALECGOS_INVALID_GROUP &&
        GroupHasEligibleEntrant(group, state, blastedGroup))
    {
        return blastedGroup;
    }

    return GetNextAvailablePortalGroup(group, state);
}

void AssignPlayerToGroup(
    KalecgosEncounterState& state, std::array<size_t, KALECGOS_GROUP_COUNT>& groupSizes,
    std::array<bool, KALECGOS_GROUP_COUNT>& groupHasDecurser,
    std::array<bool, KALECGOS_GROUP_COUNT>& groupHasHealer, Player* member, uint8 groupIndex)
{
    if (!member || groupIndex >= KALECGOS_GROUP_COUNT)
        return;

    state.playerToGroup[member->GetGUID()] = groupIndex;
    ++groupSizes[groupIndex];

    if (GET_PLAYERBOT_AI(member))
        groupHasDecurser[groupIndex] = groupHasDecurser[groupIndex] || IsKalecgosDecurser(member);

    // Resto shamans and druids are placed as decursers, so they count here too.
    groupHasHealer[groupIndex] = groupHasHealer[groupIndex] || PlayerbotAI::IsHeal(member);
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
    return bot->HasAura(Id(SwpSpells::SPELL_SPECTRAL_EXHAUSTION));
}

bool IsInSpectralRealm(Player* bot)
{
    return bot->HasAura(Id(SwpSpells::SPELL_SPECTRAL_REALM));
}

bool IsKalecgosDecurser(Player* bot)
{
    switch (bot->getClass())
    {
        case CLASS_MAGE:
        case CLASS_SHAMAN:
            break;
        case CLASS_DRUID:
        {
            if (!PlayerbotAI::IsRanged(bot))
                return false;
            break;
        }
        default:
            return false;
    }

    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
    return botAI->HasStrategy("cure", BOT_STATE_COMBAT);
}

bool EnsureKalecgosRaidAssignments(Player* player)
{
    Group* group = player->GetGroup();
    if (!group)
        return false;

    KalecgosEncounterState& state = KalecgosState(player->GetInstanceId());
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
        return false;

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
    std::array<bool, KALECGOS_GROUP_COUNT> groupHasHealer = { false, false, false, false };

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
            state, groupSizes, groupHasDecurser, groupHasHealer, decurser, groupIndex);
    }

    for (Player* healer : healers)
    {
        uint8 groupIndex = GetLeastFilledGroup(groupSizes, &groupHasHealer, true);
        AssignPlayerToGroup(
            state, groupSizes, groupHasDecurser, groupHasHealer, healer, groupIndex);
    }

    for (Player* ranged : rangedDps)
    {
        AssignPlayerToGroup(
            state, groupSizes, groupHasDecurser, groupHasHealer, ranged,
            GetLeastFilledGroup(groupSizes));
    }

    for (Player* melee : meleeDps)
    {
        AssignPlayerToGroup(
            state, groupSizes, groupHasDecurser, groupHasHealer, melee,
            GetLeastFilledGroup(groupSizes));
    }

    for (Player* other : others)
    {
        AssignPlayerToGroup(
            state, groupSizes, groupHasDecurser, groupHasHealer, other,
            GetLeastFilledGroup(groupSizes));
    }

    if (state.activeRiftGroup == KALECGOS_INVALID_GROUP)
        state.activeRiftGroup = ResolveActivePortalGroup(group, state);

    return true;
}

bool UpdateKalecgosEncounterState(Player* player)
{
    KalecgosEncounterState& state = KalecgosState(player->GetInstanceId());
    bool const isStarted = EncounterHelpers::EmplaceIfUnset(state.encounterStartMs, getMSTime());
    bool const isRiftCleared = ClearExpiredActiveRift(state);
    bool const isRebuilt = EnsureKalecgosRaidAssignments(player);
    return isStarted || isRiftCleared || isRebuilt;
}

// Read-only companion to GetKalecgosDesignatedTank below.
Player* FindKalecgosDesignatedTank(Player* player)
{
    Group* group = player->GetGroup();
    if (!group)
        return nullptr;

    KalecgosEncounterState* state = FindKalecgosState(player->GetInstanceId());
    if (!state)
        return nullptr;

    return ResolveKalecgosDesignatedTank(player, group, *state);
}

Player* GetKalecgosDesignatedTank(Player* player)
{
    Group* group = player->GetGroup();
    if (!group)
        return nullptr;

    KalecgosEncounterState& state = KalecgosState(player->GetInstanceId());
    Player* tank = ResolveKalecgosDesignatedTank(player, group, state);

    if (!ResolveSurfaceTank(group, state.currentTankGuid))
        state.currentTankGuid = tank ? tank->GetGUID() : ObjectGuid::Empty;

    return tank;
}

ObjectGuid FindKalecgosSpectralRiftGuid(Player* bot)
{
    GameObject* rift = bot->FindNearestGameObject(
        Id(SwpObjects::GO_SPECTRAL_RIFT), SPECTRAL_RIFT_SEARCH_RADIUS, true);

    return rift ? rift->GetGUID() : ObjectGuid::Empty;
}

GuidVector FindKalecgosUnitGuids(Player* bot)
{
    return EncounterHelpers::FindCreatureGuidsAround(
        bot, KALECGOS_SPAWN_POSITION, KALECGOS_UNIT_SEARCH_RADIUS,
        { Id(SwpNpcs::NPC_KALECGOS_DRAGON), Id(SwpNpcs::NPC_KALECGOS_HUMANOID) });
}

Creature* GetKalecgosUnit(Player* bot, SwpNpcs npc)
{
    GuidVector const& guids = EncounterHelpers::GetInstanceCreatureList(
        SwpState(bot->GetInstanceId()).kalecgosUnits, KALECGOS_UNIT_CACHE_INTERVAL_MS, bot,
        FindKalecgosUnitGuids);
    for (ObjectGuid const& guid : guids)
    {
        if (guid.GetEntry() != Id(npc))
            continue;

        Creature* unit = ObjectAccessor::GetCreature(*bot, guid);
        if (unit && unit->IsAlive())
            return unit;
    }

    return nullptr;
}

// Reads only, so a multiplier may call it. An expired rift not yet cleared counts as closed.
bool ShouldEnterKalecgosPortal(Player* bot)
{
    if (!IsPortalCandidate(bot))
        return false;

    KalecgosEncounterState const* state = FindKalecgosState(bot->GetInstanceId());
    if (!state || !IsActiveRiftOpen(*state) || !CanReachPortalBeforeExpiry(bot, *state))
        return false;

    if (HasKalecgosTankAssignment(state->tankAssignmentGuids, bot->GetGUID()))
    {
        if (state->sathrovarrBanishCalled)
            return false;

        Group* group = bot->GetGroup();
        if (!group)
            return false;

        return ResolveSurfaceTank(group, state->activeRiftOutgoingTankGuid) == bot &&
            state->blastedPlayerGuid != bot->GetGUID();
    }

    if (state->activeRiftGroup == KALECGOS_INVALID_GROUP ||
        GetAssignedGroup(*state, bot->GetGUID()) != state->activeRiftGroup)
    {
        return false;
    }

    // Once Sathrovarr is banished, only healers go down, so a blasted player isn't left alone with
    // Curse of Boundless Agony.
    if (state->sathrovarrBanishCalled && !PlayerbotAI::IsHeal(bot))
        return false;

    return state->blastedPlayerGuid != bot->GetGUID();
}

void RecordSpectralBlastTarget(Player* player, PlayerbotAI* announcerAI)
{
    Group* group = player->GetGroup();
    if (!group)
        return;

    KalecgosEncounterState& state = GetPreparedEncounterState(player);
    uint32 const now = getMSTime();

    state.activeRiftOpenedMs = now;
    state.blastedPlayerGuid = player->GetGUID();
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
    if (!group)
        return;

    KalecgosEncounterState& state = GetPreparedEncounterState(player);
    ObjectGuid const guid = player->GetGUID();
    bool const wasCurrentTank = state.currentTankGuid == guid;
    Player* replacementTank = nullptr;

    if (wasCurrentTank)
    {
        replacementTank =
            GetReplacementSurfaceTank(group, state, guid, state.activeRiftOutgoingTankGuid);
    }

    if (state.activeRiftOpenedMs && state.activeRiftGroup == KALECGOS_INVALID_GROUP)
        state.activeRiftGroup = ResolveActivePortalGroup(group, state);

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

Unit* GetKalecgosSideBoss(Player* bot)
{
    if (IsInSpectralRealm(bot))
        return FindThreatTarget(bot, "sathrovarr the corruptor");

    return GetSwpBoss(bot, SwpBosses::DATA_KALECGOS);
}

KalecgosCall GetDueKalecgosCall(Player* bot)
{
    Unit* boss = GetKalecgosSideBoss(bot);
    if (!boss)
        return KalecgosCall::None;

    KalecgosEncounterState const* state = FindKalecgosState(bot->GetInstanceId());
    bool const isRealmSide = IsInSpectralRealm(bot);
    KalecgosBoss const sideBoss = isRealmSide ? KalecgosBoss::Sathrovarr : KalecgosBoss::Kalecgos;
    bool const isOtherSide = state && state->firstLowBoss && *state->firstLowBoss != sideBoss;
    float const healthPct = boss->GetHealthPct();

    switch (state ? state->healthCall : KalecgosHealthCall::None)
    {
        case KalecgosHealthCall::None:
            if (healthPct < KALECGOS_HEALTH_CALL_PCT)
                return KalecgosCall::FirstUnder;
            break;
        case KalecgosHealthCall::FirstUnder:
            if (isOtherSide)
                return KalecgosCall::OtherHealth;
            break;
        case KalecgosHealthCall::GapHold:
            if (isOtherSide && healthPct < KALECGOS_HEALTH_CALL_PCT)
                return KalecgosCall::GapHoldRelease;
            break;
        default:
            break;
    }

    if (state && state->sathrovarrBanishCalled)
        return KalecgosCall::None;

    if (isRealmSide)
    {
        return boss->HasAura(Id(SwpSpells::SPELL_KALECGOS_BANISH)) ?
            KalecgosCall::SathrovarrBanished : KalecgosCall::None;
    }

    bool const isEndHoldCalled = state && state->endHoldCalled;
    return !isEndHoldCalled && healthPct < KALECGOS_END_HOLD_PCT ?
        KalecgosCall::EndHold : KalecgosCall::None;
}

// Each side of the realms has one enemy, so a held side holds everything.
KalecgosDpsHold GetKalecgosSideHoldWindow(uint32 instanceId, bool isRealmSide)
{
    KalecgosEncounterState const* state = FindKalecgosState(instanceId);
    if (!state)
        return KalecgosDpsHold::None;

    if (state->healthCall == KalecgosHealthCall::GapHold && state->firstLowBoss &&
        (*state->firstLowBoss == KalecgosBoss::Sathrovarr) == isRealmSide)
    {
        return KalecgosDpsHold::HealthGap;
    }

    if (!isRealmSide && state->endHoldCalled && !state->sathrovarrBanishCalled)
        return KalecgosDpsHold::End;

    return KalecgosDpsHold::None;
}

KalecgosDpsHold GetKalecgosDpsHoldWindow(Player* bot)
{
    return GetKalecgosSideHoldWindow(bot->GetInstanceId(), IsInSpectralRealm(bot));
}

KalecgosDpsHold GetKalecgosDpsHold(Player* bot)
{
    return PlayerbotAI::IsTank(bot) ? KalecgosDpsHold::None : GetKalecgosDpsHoldWindow(bot);
}

// The portal unsummons a player's pet and resummons it beside its owner, but a guardian (Feral
// Spirit, Force of Nature, Army of the Dead) has no path through the floor and stays on the
// surface, so each minion's own side decides.
bool IsKalecgosPetHoldActive(Player* bot)
{
    uint32 const instanceId = bot->GetInstanceId();
    bool const isSurfaceHeld =
        GetKalecgosSideHoldWindow(instanceId, false) != KalecgosDpsHold::None;
    bool const isRealmHeld =
        GetKalecgosSideHoldWindow(instanceId, true) != KalecgosDpsHold::None;
    if (!isSurfaceHeld && !isRealmHeld)
        return false;

    return std::any_of(
        bot->m_Controlled.begin(), bot->m_Controlled.end(),
        [isSurfaceHeld, isRealmHeld](Unit* unit)
        {
            Creature* minion = unit ? unit->ToCreature() : nullptr;
            if (!minion || !minion->IsAlive() || !minion->GetCharmInfo())
                return false;

            return minion->GetPositionZ() < SPECTRAL_REALM_MAX_Z ? isRealmHeld : isSurfaceHeld;
        });
}

// Shared encounter state

KalecgosEncounterState& KalecgosState(uint32 instanceId)
{
    std::optional<KalecgosEncounterState>& state = SwpState(instanceId).kalecgos;
    if (!state)
        state.emplace();

    return *state;
}

KalecgosEncounterState* FindKalecgosState(uint32 instanceId)
{
    SwpInstanceState* state = FindSwpState(instanceId);
    return state && state->kalecgos ? &*state->kalecgos : nullptr;
}

}
