/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include <algorithm>
#include <cmath>
#include <unordered_set>
#include <unordered_map>

#include "SWPEncounter_KJ.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "Timer.h"

namespace SunwellHelpers
{

// Note: Kil'jaeden's CombatReach is 15.0f

namespace
{

float GetCenteredArcSlotAngleOffset(uint8 slotIndex, uint8 slotCount, float arcWidth)
{
    if (slotCount <= 1)
        return 0.0f;

    const float angleStep = arcWidth / static_cast<float>(slotCount - 1);
    if (slotCount % 2 == 1)
    {
        if (slotIndex == 0)
            return 0.0f;

        const uint8 stepIndex = (slotIndex + 1) / 2;
        float angleOffset = angleStep * stepIndex;
        if (slotIndex % 2 == 0)
            angleOffset = -angleOffset;

        return angleOffset;
    }

    const float halfStep = angleStep / 2.0f;
    const uint8 pairIndex = slotIndex / 2;
    float angleOffset = halfStep + angleStep * pairIndex;
    if (slotIndex % 2 == 1)
        angleOffset = -angleOffset;

    return angleOffset;
}

uint32 GetKiljaedenDragonManualCooldown(uint32 spellId)
{
    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
    if (!spellInfo)
        return 1000;

    uint32 cooldownMs = spellInfo->GetRecoveryTime();
    if (spellInfo->CategoryRecoveryTime > cooldownMs)
        cooldownMs = spellInfo->CategoryRecoveryTime;
    if (spellInfo->StartRecoveryTime > cooldownMs)
        cooldownMs = spellInfo->StartRecoveryTime;

    return cooldownMs ? cooldownMs : 1000;
}

bool IsKiljaedenDragonGroupTarget(PlayerbotAI* botAI, Player* bot, Player* member)
{
    return member && member->IsAlive() && member != bot && !botAI->IsTank(member) &&
        member->GetMapId() == SUNWELL_MAP_ID;
}

uint32 GetKiljaedenDragonAppliedAuraSpell(uint32 spellId)
{
    switch (spellId)
    {
        case static_cast<uint32>(SunwellSpells::SPELL_DRAGON_BREATH_HASTE):
            return static_cast<uint32>(SunwellSpells::SPELL_DRAGON_BREATH_HASTE);

        case static_cast<uint32>(SunwellSpells::SPELL_DRAGON_BREATH_REVITALIZE):
            return static_cast<uint32>(SunwellSpells::SPELL_DRAGON_BREATH_REVITALIZE);

        default:
            return 0;
    }
}

bool HasKiljaedenDragonApplicableAura(Player* member, uint32 spellId)
{
    uint32 const auraSpellId = GetKiljaedenDragonAppliedAuraSpell(spellId);
    return auraSpellId && member && member->HasAura(auraSpellId);
}

float GetKiljaedenRangedSlotAngle(uint8 slotIndex)
{
    Position position;
    if (!TryGetKiljaedenRangedSlotPosition(slotIndex, position))
        return 0.0f;

    return Position::NormalizeOrientation(std::atan2(
        position.GetPositionY() - KILJAEDEN_CENTER_POSITION.GetPositionY(),
        position.GetPositionX() - KILJAEDEN_CENTER_POSITION.GetPositionX()));
}

bool IsKiljaedenRangedSlotSafe(
    Position const& position, std::vector<KiljaedenArmageddon> const& armageddons)
{
    for (KiljaedenArmageddon const& armageddon : armageddons)
    {
        if (position.GetExactDist2d(
                armageddon.destination.GetPositionX(),
                armageddon.destination.GetPositionY()) < armageddon.safeDistance)
        {
            return false;
        }
    }

    return true;
}

float GetKiljaedenNearestArmageddonDistance(
    Position const& position, std::vector<KiljaedenArmageddon> const& armageddons)
{
    float nearestDistance = std::numeric_limits<float>::max();

    for (KiljaedenArmageddon const& armageddon : armageddons)
    {
        nearestDistance = std::min(nearestDistance, position.GetExactDist2d(
            armageddon.destination.GetPositionX(),
            armageddon.destination.GetPositionY()));
    }

    return nearestDistance;
}

} // end anonymous namespace

const Position KILJAEDEN_TANK_POSITION =     { 1704.729f, 634.891f, 27.787f };
const Position KILJAEDEN_S_MELEE_POSITION =  { 1689.487f, 632.119f, 27.823f };
const Position KILJAEDEN_E_MELEE_POSITION =  { 1700.542f, 619.589f, 27.786f };
const Position KILJAEDEN_DARKNESS_POSITION = { 1709.768f, 642.241f, 27.706f };
const Position KILJAEDEN_CENTER_POSITION =   { 1698.450f, 628.030f, 28.199f };

uint32 const KILJAEDEN_DRAGON_ORB_ENTRIES[4] =
{
    static_cast<uint32>(SunwellObjects::GO_DRAGON_ORB_1),
    static_cast<uint32>(SunwellObjects::GO_DRAGON_ORB_2),
    static_cast<uint32>(SunwellObjects::GO_DRAGON_ORB_3),
    static_cast<uint32>(SunwellObjects::GO_DRAGON_ORB_4)
};

std::unordered_set<ObjectGuid> kiljaedenTrackedArmageddonTargets;

std::unordered_map<uint32, KiljaedenEncounterState> kiljaedenEncounterStates;

std::unordered_map<ObjectGuid::LowType, uint32> kiljaedenDragonOrbUseTimes;

void AddKiljaedenArmageddon(
    uint32 instanceId, Position const& destination, uint32 durationMs, float safeDistance)
{
    if (!durationMs || safeDistance <= 0.0f)
        return;

    const uint32 now = getMSTime();
    PruneExpiredKiljaedenArmageddons(instanceId);

    KiljaedenArmageddon armageddon;
    armageddon.destination = destination;
    armageddon.expireMs = now + durationMs;
    armageddon.safeDistance = safeDistance;
    kiljaedenEncounterStates[instanceId].armageddons.push_back(armageddon);
}

bool TryGetKiljaedenNearestArmageddon(Player* bot, KiljaedenArmageddon& armageddon)
{
    PruneExpiredKiljaedenArmageddons(bot->GetInstanceId());
    auto const stateItr = kiljaedenEncounterStates.find(bot->GetInstanceId());
    if (stateItr == kiljaedenEncounterStates.end())
        return false;

    bool foundArmageddon = false;
    float bestDistance = 0.0f;

    for (KiljaedenArmageddon const& candidate : stateItr->second.armageddons)
    {
        const float distance = bot->GetExactDist2d(
            candidate.destination.GetPositionX(), candidate.destination.GetPositionY());
        if (distance >= candidate.safeDistance)
            continue;

        if (!foundArmageddon || distance < bestDistance)
        {
            armageddon = candidate;
            bestDistance = distance;
            foundArmageddon = true;
        }
    }

    return foundArmageddon;
}

void PruneExpiredKiljaedenArmageddons(uint32 instanceId)
{
    auto const stateItr = kiljaedenEncounterStates.find(instanceId);
    if (stateItr == kiljaedenEncounterStates.end())
        return;

    const uint32 now = getMSTime();
    std::vector<KiljaedenArmageddon>& armageddons = stateItr->second.armageddons;
    armageddons.erase(std::remove_if(armageddons.begin(), armageddons.end(),
        [now](KiljaedenArmageddon const& armageddon) {
            return !armageddon.expireMs || armageddon.expireMs <= now;
        }), armageddons.end());

    if (armageddons.empty())
        kiljaedenEncounterStates.erase(stateItr);
}

bool TryGetKiljaedenRangedSlotPosition(uint8 slotIndex, Position& position)
{
    if (slotIndex >= KILJAEDEN_TOTAL_RANGED_SLOT_COUNT)
        return false;

    float radius = KILJAEDEN_OUTER_RANGED_RADIUS;
    uint8 localSlotIndex = slotIndex;
    uint8 slotCount = KILJAEDEN_OUTER_RANGED_SLOT_COUNT;

    if (slotIndex < KILJAEDEN_INNER_RANGED_SLOT_COUNT)
    {
        radius = KILJAEDEN_INNER_RANGED_RADIUS;
        slotCount = KILJAEDEN_INNER_RANGED_SLOT_COUNT;
    }
    else
    {
        localSlotIndex -= KILJAEDEN_INNER_RANGED_SLOT_COUNT;
    }

    const float angleOffset = GetCenteredArcSlotAngleOffset(localSlotIndex, slotCount, M_PI);
    const float angle =
        Position::NormalizeOrientation(KILJAEDEN_RANGED_ARC_ORIENTATION + angleOffset);
    const float positionX = KILJAEDEN_CENTER_POSITION.GetPositionX() + std::cos(angle) * radius;
    const float positionY = KILJAEDEN_CENTER_POSITION.GetPositionY() + std::sin(angle) * radius;

    position = Position{ positionX, positionY, KILJAEDEN_CENTER_POSITION.GetPositionZ() };
    return true;
}

void EnsureKiljaedenRangedAssignments(PlayerbotAI* botAI, Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group || bot->GetMapId() != SUNWELL_MAP_ID)
        return;

    auto& assignments = kiljaedenEncounterStates[bot->GetInstanceId()].rangedAssignments;
    const bool isRanged = botAI->IsRanged(bot);

    std::vector<ObjectGuid> invalidAssignments;
    for (auto const& assignment : assignments)
    {
        bool found = false;
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || member->GetGUID() != assignment.first)
                continue;

            found = member->GetMapId() == SUNWELL_MAP_ID && GET_PLAYERBOT_AI(member) && isRanged;

            break;
        }

        if (!found)
            invalidAssignments.push_back(assignment.first);
    }

    for (const ObjectGuid& guid : invalidAssignments)
        assignments.erase(guid);

    std::array<bool, KILJAEDEN_TOTAL_RANGED_SLOT_COUNT> usedSlots = {};
    for (auto const& assignment : assignments)
    {
        if (assignment.second < KILJAEDEN_TOTAL_RANGED_SLOT_COUNT)
            usedSlots[assignment.second] = true;
    }

    auto const assignNextOpenSlot = [&](Player* member)
    {
        for (uint8 slotIndex = 0; slotIndex < KILJAEDEN_TOTAL_RANGED_SLOT_COUNT;
             ++slotIndex)
        {
            if (usedSlots[slotIndex])
                continue;

            assignments[member->GetGUID()] = slotIndex;
            usedSlots[slotIndex] = true;
            return;
        }
    };

    std::vector<Player*> healers;
    std::vector<Player*> rangedDamage;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member->GetMapId() != SUNWELL_MAP_ID || !isRanged ||
            !GET_PLAYERBOT_AI(member))
        {
            continue;
        }

        if (assignments.find(member->GetGUID()) != assignments.end())
            continue;

        if (botAI->IsHeal(member))
            healers.push_back(member);
        else
            rangedDamage.push_back(member);
    }

    auto const sortByGuid = [](std::vector<Player*>& members)
    {
        std::sort(members.begin(), members.end(),
            [](Player* left, Player* right) { return left->GetGUID() < right->GetGUID(); });
    };

    sortByGuid(healers);
    sortByGuid(rangedDamage);

    for (Player* member : healers)
        assignNextOpenSlot(member);

    for (Player* member : rangedDamage)
        assignNextOpenSlot(member);
}

void EnsureKiljaedenRangedArmageddonAssignments(PlayerbotAI* botAI, Player* bot)
{
    struct CandidateSlotScore
    {
        uint8 slotIndex = 0;
        bool sameRow = false;
        float angleDistance = 0.0f;
        uint8 occupancy = 0;
        float armageddonDistance = 0.0f;
    };

    const uint32 instanceId = bot->GetInstanceId();
    PruneExpiredKiljaedenArmageddons(instanceId);

    auto const armageddonItr = kiljaedenEncounterStates.find(instanceId);
    if (armageddonItr == kiljaedenEncounterStates.end() || armageddonItr->second.armageddons.empty())
    {
        kiljaedenEncounterStates[instanceId].rangedArmageddonAssignments.clear();
        return;
    }

    Group* group = bot->GetGroup();
    if (!group || !botAI->IsRanged(bot))
    {
        kiljaedenEncounterStates[instanceId].rangedArmageddonAssignments.clear();
        return;
    }

    auto const canonicalItr = kiljaedenEncounterStates.find(instanceId);
    if (canonicalItr == kiljaedenEncounterStates.end())
    {
        kiljaedenEncounterStates[instanceId].rangedArmageddonAssignments.clear();
        return;
    }

    auto const& armageddons = armageddonItr->second.armageddons;
    auto const& canonicalAssignments = canonicalItr->second.rangedAssignments;

    std::vector<KiljaedenRangedBotAssignment> rangedBots;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member->GetMapId() != SUNWELL_MAP_ID ||
            !GET_PLAYERBOT_AI(member) || !botAI->IsRanged(member))
        {
            continue;
        }

        auto const assignmentItr = canonicalAssignments.find(member->GetGUID());
        if (assignmentItr == canonicalAssignments.end() ||
            assignmentItr->second >= KILJAEDEN_TOTAL_RANGED_SLOT_COUNT)
        {
            continue;
        }

        rangedBots.push_back({ member->GetGUID(), assignmentItr->second });
    }

    std::sort(rangedBots.begin(), rangedBots.end(),
        [](KiljaedenRangedBotAssignment const& left, KiljaedenRangedBotAssignment const& right)
        {
            if (left.slotIndex != right.slotIndex)
                return left.slotIndex < right.slotIndex;

            return left.guid < right.guid;
        });

    std::array<bool, KILJAEDEN_TOTAL_RANGED_SLOT_COUNT> safeSlots = {};
    std::array<float, KILJAEDEN_TOTAL_RANGED_SLOT_COUNT> slotAngles = {};
    std::array<float, KILJAEDEN_TOTAL_RANGED_SLOT_COUNT> nearestArmageddonDistances = {};

    for (uint8 slotIndex = 0; slotIndex < KILJAEDEN_TOTAL_RANGED_SLOT_COUNT; ++slotIndex)
    {
        Position slotPosition;
        if (!TryGetKiljaedenRangedSlotPosition(slotIndex, slotPosition))
            continue;

        safeSlots[slotIndex] = IsKiljaedenRangedSlotSafe(slotPosition, armageddons);
        slotAngles[slotIndex] = GetKiljaedenRangedSlotAngle(slotIndex);
        nearestArmageddonDistances[slotIndex] =
            GetKiljaedenNearestArmageddonDistance(slotPosition, armageddons);
    }

    std::array<uint8, KILJAEDEN_TOTAL_RANGED_SLOT_COUNT> plannedOccupancy = {};
    auto& tempAssignments = kiljaedenEncounterStates[instanceId].rangedArmageddonAssignments;
    tempAssignments.clear();

    auto const getCandidateScore =
        [&](KiljaedenRangedBotAssignment const& rangedBot, uint8 candidateSlotIndex)
    {
        CandidateSlotScore score;
        score.slotIndex = candidateSlotIndex;
        score.sameRow =
            (candidateSlotIndex < KILJAEDEN_INNER_RANGED_SLOT_COUNT) ==
            (rangedBot.slotIndex < KILJAEDEN_INNER_RANGED_SLOT_COUNT);

        float angleDistance = Position::NormalizeOrientation(
            slotAngles[candidateSlotIndex] - slotAngles[rangedBot.slotIndex]);
        if (angleDistance > static_cast<float>(M_PI))
            angleDistance -= 2.0f * static_cast<float>(M_PI);

        score.angleDistance = std::fabs(angleDistance);
        score.occupancy = plannedOccupancy[candidateSlotIndex];
        score.armageddonDistance = nearestArmageddonDistances[candidateSlotIndex];
        return score;
    };

    auto const shouldTakeCandidate =
        [&](CandidateSlotScore const& candidate, CandidateSlotScore const& best, bool bestFound)
    {
        if (!bestFound)
            return true;

        if (candidate.sameRow != best.sameRow)
            return candidate.sameRow;

        if (candidate.angleDistance != best.angleDistance)
            return candidate.angleDistance < best.angleDistance;

        if (candidate.occupancy != best.occupancy)
            return candidate.occupancy < best.occupancy;

        if (candidate.armageddonDistance != best.armageddonDistance)
            return candidate.armageddonDistance > best.armageddonDistance;

        return candidate.slotIndex < best.slotIndex;
    };

    std::vector<KiljaedenRangedBotAssignment> displacedBots;
    for (KiljaedenRangedBotAssignment const& rangedBot : rangedBots)
    {
        if (!safeSlots[rangedBot.slotIndex])
        {
            displacedBots.push_back(rangedBot);
            continue;
        }

        tempAssignments[rangedBot.guid] = rangedBot.slotIndex;
        ++plannedOccupancy[rangedBot.slotIndex];
    }

    for (KiljaedenRangedBotAssignment const& rangedBot : displacedBots)
    {
        bool bestFound = false;
        CandidateSlotScore bestCandidate;
        bestCandidate.slotIndex = rangedBot.slotIndex;

        for (uint8 candidateSlotIndex = 0;
             candidateSlotIndex < KILJAEDEN_TOTAL_RANGED_SLOT_COUNT; ++candidateSlotIndex)
        {
            if (!safeSlots[candidateSlotIndex] || plannedOccupancy[candidateSlotIndex] >= 2)
                continue;

            const CandidateSlotScore candidate = getCandidateScore(rangedBot, candidateSlotIndex);
            if (!shouldTakeCandidate(candidate, bestCandidate, bestFound))
                continue;

            bestFound = true;
            bestCandidate = candidate;
        }

        tempAssignments[rangedBot.guid] = bestCandidate.slotIndex;
        if (bestFound)
            ++plannedOccupancy[bestCandidate.slotIndex];
    }

    if (tempAssignments.empty())
        kiljaedenEncounterStates[instanceId].rangedArmageddonAssignments.clear();
}

bool IsKiljaedenCastingDarknessOfAThousandSouls(Unit* kiljaeden)
{
    return kiljaeden && kiljaeden->HasUnitState(UNIT_STATE_CASTING) &&
        kiljaeden->FindCurrentSpellBySpellId(
            static_cast<uint32>(SunwellSpells::SPELL_DARKNESS_OF_A_THOUSAND_SOULS));
}

Player* GetKiljaedenDragonOrbUser(Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || !GET_PLAYERBOT_AI(member) ||
            member->GetMapId() != SUNWELL_MAP_ID)
        {
            continue;
        }

        if (group->IsAssistant(member->GetGUID()))
            return member;
    }

    return nullptr;
}

bool ResetKiljaedenDragonOrbUserAnnouncement(uint32 instanceId)
{
    auto const stateItr = kiljaedenEncounterStates.find(instanceId);
    if (stateItr == kiljaedenEncounterStates.end() || !stateItr->second.dragonOrbAnnouncementMs)
        return false;

    constexpr uint32 announcementResetDelayMs = 10000;
    if (getMSTimeDiff(stateItr->second.dragonOrbAnnouncementMs, getMSTime()) < announcementResetDelayMs)
        return false;

    stateItr->second.dragonOrbAnnouncementMs = 0;
    return true;
}

bool HasRecentKiljaedenDragonOrbUse(Player* bot, uint32 recentMs)
{
    auto const orbUseTime = kiljaedenDragonOrbUseTimes.find(bot->GetGUID().GetCounter());
    if (orbUseTime == kiljaedenDragonOrbUseTimes.end())
        return false;

    return getMSTimeDiff(orbUseTime->second, getMSTime()) < recentMs;
}

bool HasKiljaedenDragonAura(Player* bot)
{
    return bot->HasAura(static_cast<uint32>(SunwellSpells::SPELL_VENGEANCE_OF_THE_BLUE_FLIGHT));
}

Unit* GetKiljaedenControlledDragon(Player* bot)
{
    Unit* dragon = bot->GetCharm();
    if (!dragon || !dragon->IsAlive() || dragon->GetCharmerGUID() != bot->GetGUID())
        return nullptr;

    return dragon;
}

bool CastKiljaedenDragonSpell(Unit* dragon, uint32 spellId)
{
    if (!dragon || dragon->HasSpellCooldown(spellId))
        return false;

    dragon->CastSpell(dragon, spellId, true);
    dragon->AddSpellCooldown(spellId, 0, GetKiljaedenDragonManualCooldown(spellId));
    return true;
}

Player* FindBestKiljaedenDragonClusterTarget(
    PlayerbotAI* botAI, Player* bot, Unit* dragon, uint32 spellId)
{
    if (!dragon)
        return nullptr;

    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    constexpr uint8 minClusterSize = 3;
    constexpr float clusterRadius = 6.0f;

    Player* bestTarget = nullptr;
    uint32 bestClusterSize = 0;
    uint32 bestTotalClusterSize = 0;
    float bestDistanceToDragon = 0.0f;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* candidate = ref->GetSource();
        if (!IsKiljaedenDragonGroupTarget(botAI, bot, candidate) ||
            HasKiljaedenDragonApplicableAura(candidate, spellId))
        {
            continue;
        }

        uint32 clusterSize = 0;
        uint32 totalClusterSize = 0;
        for (GroupReference* otherRef = group->GetFirstMember();
             otherRef; otherRef = otherRef->next())
        {
            Player* other = otherRef->GetSource();
            if (!IsKiljaedenDragonGroupTarget(botAI, bot, other) ||
                candidate->GetExactDist2d(other) > clusterRadius)
            {
                continue;
            }

            ++totalClusterSize;
            if (!HasKiljaedenDragonApplicableAura(other, spellId))
                ++clusterSize;
        }

        if (clusterSize < minClusterSize)
            continue;

        const float distanceToDragon = dragon->GetExactDist2d(candidate);
        if (!bestTarget || clusterSize > bestClusterSize ||
            (clusterSize == bestClusterSize && totalClusterSize > bestTotalClusterSize) ||
            (clusterSize == bestClusterSize && totalClusterSize == bestTotalClusterSize &&
             distanceToDragon < bestDistanceToDragon) ||
            (clusterSize == bestClusterSize && totalClusterSize == bestTotalClusterSize &&
             distanceToDragon == bestDistanceToDragon && candidate->GetGUID() < bestTarget->GetGUID()))
        {
            bestTarget = candidate;
            bestClusterSize = clusterSize;
            bestTotalClusterSize = totalClusterSize;
            bestDistanceToDragon = distanceToDragon;
        }
    }

    return bestTarget;
}

Player* FindClosestKiljaedenDragonTarget(Player* bot, Unit* dragon, uint32 spellId)
{
    if (!dragon)
        return nullptr;

    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    Player* closestTarget = nullptr;
    float closestDistance = 0.0f;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || member == bot ||
            member->GetMapId() != SUNWELL_MAP_ID ||
            HasKiljaedenDragonApplicableAura(member, spellId))
        {
            continue;
        }

        const float distance = dragon->GetExactDist2d(member);
        if (!closestTarget || distance < closestDistance)
        {
            closestTarget = member;
            closestDistance = distance;
        }
    }

    return closestTarget;
}

}
