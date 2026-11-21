/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "SWPEncounter_KJ.h"
#include "EncounterHelpers.h"
#include "PlayerbotAI.h"
#include "PlayerbotTextMgr.h"
#include "Playerbots.h"
#include "SWPInstanceState.h"
#include "SWPShared.h"
#include "Spell.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include <list>
#include <map>
#include <optional>
#include <string>

using namespace EncounterHelpers;

namespace SwpHelpers
{

// Note: Kil'jaeden's CombatReach is 15.0f

namespace
{

bool IsDragonGroupTarget(Player* bot, Player* member)
{
    return member && member != bot && member->IsAlive() &&
        member->GetMapId() == SWP_MAP_ID && !PlayerbotAI::IsTank(member);
}

uint32 GetDragonAppliedAuraSpell(uint32 spellId)
{
    switch (spellId)
    {
        case Id(SwpSpells::SPELL_DRAGON_BREATH_HASTE):
            return Id(SwpSpells::SPELL_DRAGON_BREATH_HASTE);
        case Id(SwpSpells::SPELL_DRAGON_BREATH_REVITALIZE):
            return Id(SwpSpells::SPELL_DRAGON_BREATH_REVITALIZE);
        default:
            return 0;
    }
}

bool HasAuraFromDragon(Player* member, uint32 spellId)
{
    uint32 const auraSpellId = GetDragonAppliedAuraSpell(spellId);
    return auraSpellId && member && member->HasAura(auraSpellId);
}

bool IsOutsideArmageddons(
    Position const& position, std::vector<KiljaedenArmageddon> const& armageddons)
{
    for (KiljaedenArmageddon const& armageddon : armageddons)
    {
        if (position.GetExactDist2d(armageddon.destination) < armageddon.safeDistance)
            return false;
    }

    return true;
}

float GetNearestArmageddonDistance(
    Position const& position, std::vector<KiljaedenArmageddon> const& armageddons)
{
    float nearestDistance = std::numeric_limits<float>::max();

    for (KiljaedenArmageddon const& armageddon : armageddons)
    {
        nearestDistance = std::min(
            nearestDistance, position.GetExactDist2d(armageddon.destination));
    }

    return nearestDistance;
}

bool ShouldRebuildKiljaedenAssignments(uint32& lastRebuildMs, uint32 intervalMs)
{
    uint32 const now = getMSTime();
    if (lastRebuildMs && getMSTimeDiff(lastRebuildMs, now) < intervalMs)
        return false;

    lastRebuildMs = now;
    return true;
}

// A point on the ring round Kil'jaeden to aim for when the straight line to the destination would
// pass through his Knockback: a little past the bot's tangent to the ring, on the side nearer the
// destination.
bool TryGetKiljaedenDetourPoint(Player* bot, Position const& destination, Position& detourPoint)
{
    if (!IsLineThroughKiljaedenKnockback(*bot, destination))
        return false;

    Position const& center = SUNWELL_CENTER_POSITION;
    float const radius = KILJAEDEN_KNOCKBACK_CLEARANCE;
    float const botDistance = std::max(bot->GetExactDist2d(center), radius);
    float const botAngle = center.GetAngle(bot);
    float const arc = Position::NormalizeOrientation(center.GetAngle(&destination) - botAngle);
    float const turn = arc <= static_cast<float>(M_PI) ? 1.0f : -1.0f;
    float const angle =
        botAngle + turn * (std::acos(radius / botDistance) + KILJAEDEN_ROUTE_LEAD_ANGLE);

    detourPoint = Position(
        center.GetPositionX() + std::cos(angle) * radius,
        center.GetPositionY() + std::sin(angle) * radius, bot->GetPositionZ());
    return true;
}

bool SayKiljaedenDragonOrbUser(PlayerbotAI* botAI, ClassRoleChoice const& choice, bool isHandover)
{
    std::string text;
    if (choice.guid.IsEmpty())
    {
        text = PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "kiljaeden_no_designated_dragon_orb_user",
            "No bot has an assistant flag, and therefore a player must control the dragons. If you "
            "would like a bot to control the dragons, please set the assistant flag for a bot.",
            {});
    }
    else
    {
        Player* orbUser = ObjectAccessor::GetPlayer(*botAI->GetBot(), choice.guid);
        if (!orbUser)
            return false;

        std::map<std::string, std::string> placeholders = { { "%bot", orbUser->GetName() } };
        if (isHandover)
        {
            text = PlayerbotTextMgr::instance().GetBotTextOrDefault(
                "kiljaeden_dragon_orb_user_handover", "%bot takes over as the dragon orb user.",
                placeholders);
        }
        else
        {
            text = PlayerbotTextMgr::instance().GetBotTextOrDefault(
                "kiljaeden_designated_dragon_orb_user",
                "%bot is the first assistant bot and the designated dragon orb user. If you would "
                "like only players to control dragons, please remove assistant flags from all "
                "bots.",
                placeholders);
        }
    }

    return botAI->SayToRaid(text);
}

} // end anonymous namespace

GuidVector FindKiljaedenHandGuids(Player* bot)
{
    GuidVector guids;

    std::list<Creature*> creatures;
    bot->GetCreatureListWithEntryInGrid(
        creatures, Id(SwpNpcs::NPC_HAND_OF_THE_DECEIVER), HAND_SEARCH_RADIUS);

    for (Creature* creature : creatures)
    {
        if (creature && creature->IsAlive() && creature->IsInCombat())
            guids.push_back(creature->GetGUID());
    }

    std::sort(guids.begin(), guids.end());

    return guids;
}

std::vector<Unit*> GetKiljaedenHands(PlayerbotAI* botAI)
{
    std::vector<Unit*> hands;

    for (auto const& guid :
         botAI->GetAiObjectContext()->GetValue<GuidVector>("kiljaeden hands")->RefGet())
    {
        Unit* hand = botAI->GetUnit(guid);
        if (hand && hand->IsAlive())
            hands.push_back(hand);
    }

    return hands;
}

bool IsKiljaedenHandControlClaimed(Unit* hand)
{
    KiljaedenEncounterState* state = FindKiljaedenState(hand->GetInstanceId());
    if (!state)
        return false;

    auto const claimItr = state->handControlClaims.find(hand->GetGUID());
    if (claimItr == state->handControlClaims.end())
        return false;

    if (claimItr->second > getMSTime())
        return true;

    state->handControlClaims.erase(claimItr);
    return false;
}

void ClaimKiljaedenHandControl(Unit* hand)
{
    KiljaedenState(hand->GetInstanceId()).handControlClaims[hand->GetGUID()] =
        getMSTime() + HAND_CONTROL_CLAIM_MS;
}

void AddKiljaedenArmageddon(
    uint32 instanceId, Position const& destination, uint32 durationMs, float safeDistance)
{
    if (!durationMs || safeDistance <= 0.0f)
        return;

    uint32 const now = getMSTime();
    PruneExpiredKiljaedenArmageddons(instanceId);

    KiljaedenArmageddon armageddon;
    armageddon.destination = destination;
    armageddon.expireMs = now + durationMs;
    armageddon.safeDistance = safeDistance;
    KiljaedenState(instanceId).armageddons.push_back(armageddon);
}

// Callers prune expired Armageddons first (PruneExpiredKiljaedenArmageddons).
bool IsOutsideKiljaedenArmageddons(uint32 instanceId, Position const& position)
{
    KiljaedenEncounterState const* state = FindKiljaedenState(instanceId);
    return !state || IsOutsideArmageddons(position, state->armageddons);
}

void PruneExpiredKiljaedenArmageddons(uint32 instanceId)
{
    KiljaedenEncounterState* state = FindKiljaedenState(instanceId);
    if (!state)
        return;

    uint32 const now = getMSTime();
    std::vector<KiljaedenArmageddon>& armageddons = state->armageddons;
    armageddons.erase(std::remove_if(armageddons.begin(), armageddons.end(),
        [now](KiljaedenArmageddon const& armageddon) {
            return !armageddon.expireMs || armageddon.expireMs <= now;
        }), armageddons.end());
}

bool TryGetKiljaedenRangedSlotPosition(uint8 slotIndex, Position& position)
{
    if (slotIndex >= KILJAEDEN_TOTAL_RANGED_SLOT_COUNT)
        return false;

    position = KILJAEDEN_RANGED_POSITIONS[slotIndex];
    return true;
}

void EnsureKiljaedenRangedAssignments(Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group)
        return;

    KiljaedenEncounterState& state = KiljaedenState(bot->GetInstanceId());
    if (!ShouldRebuildKiljaedenAssignments(
            state.rangedAssignmentRebuildMs, KILJAEDEN_RANGED_ASSIGNMENT_REBUILD_INTERVAL_MS))
    {
        return;
    }

    auto& assignments = state.rangedAssignments;

    std::vector<ObjectGuid> invalidAssignments;
    for (auto const& assignment : assignments)
    {
        bool found = false;
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || member->GetGUID() != assignment.first)
                continue;

            found = member->GetMapId() == SWP_MAP_ID && GET_PLAYERBOT_AI(member) &&
                PlayerbotAI::IsRanged(member);

            break;
        }

        if (!found)
            invalidAssignments.push_back(assignment.first);
    }

    for (ObjectGuid const& guid : invalidAssignments)
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
        if (!member || member->GetMapId() != SWP_MAP_ID || !GET_PLAYERBOT_AI(member) ||
            !PlayerbotAI::IsRanged(member))
        {
            continue;
        }

        if (assignments.find(member->GetGUID()) != assignments.end())
            continue;

        if (PlayerbotAI::IsHeal(member))
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

void EnsureKiljaedenRangedArmageddonAssignments(Player* bot)
{
    struct CandidateSlotScore
    {
        uint8 slotIndex = 0;
        float moveDistance = 0.0f;
        uint8 occupancy = 0;
        float armageddonDistance = 0.0f;
    };

    uint32 const instanceId = bot->GetInstanceId();
    PruneExpiredKiljaedenArmageddons(instanceId);

    KiljaedenEncounterState* foundState = FindKiljaedenState(instanceId);
    if (!foundState)
        return;

    KiljaedenEncounterState& state = *foundState;

    Group* group = bot->GetGroup();
    if (state.armageddons.empty() || !group)
    {
        state.rangedArmageddonAssignments.clear();
        return;
    }

    // For bots to return to their normal positions once Armageddons stop.
    if (!ShouldRebuildKiljaedenAssignments(
            state.rangedArmageddonRebuildMs, ARMAGEDDON_ASSIGNMENT_REBUILD_INTERVAL_MS))
    {
        return;
    }

    auto const& armageddons = state.armageddons;
    auto const& canonicalAssignments = state.rangedAssignments;

    std::vector<KiljaedenRangedBotAssignment> rangedBots;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member->GetMapId() != SWP_MAP_ID || !GET_PLAYERBOT_AI(member) ||
            !PlayerbotAI::IsRanged(member))
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
    std::array<float, KILJAEDEN_TOTAL_RANGED_SLOT_COUNT> nearestArmageddonDistances = {};

    for (uint8 slotIndex = 0; slotIndex < KILJAEDEN_TOTAL_RANGED_SLOT_COUNT; ++slotIndex)
    {
        Position const& slotPosition = KILJAEDEN_RANGED_POSITIONS[slotIndex];
        safeSlots[slotIndex] = IsOutsideArmageddons(slotPosition, armageddons);
        nearestArmageddonDistances[slotIndex] =
            GetNearestArmageddonDistance(slotPosition, armageddons);
    }

    std::array<uint8, KILJAEDEN_TOTAL_RANGED_SLOT_COUNT> plannedOccupancy = {};
    auto& tempAssignments = state.rangedArmageddonAssignments;
    tempAssignments.clear();

    auto const getCandidateScore =
        [&](KiljaedenRangedBotAssignment const& rangedBot, uint8 candidateSlotIndex)
    {
        CandidateSlotScore score;
        score.slotIndex = candidateSlotIndex;
        score.moveDistance = KILJAEDEN_RANGED_POSITIONS[rangedBot.slotIndex].GetExactDist2d(
            KILJAEDEN_RANGED_POSITIONS[candidateSlotIndex]);
        score.occupancy = plannedOccupancy[candidateSlotIndex];
        score.armageddonDistance = nearestArmageddonDistances[candidateSlotIndex];
        return score;
    };

    auto const shouldTakeCandidate =
        [&](CandidateSlotScore const& candidate, CandidateSlotScore const& best, bool bestFound)
    {
        if (!bestFound)
            return true;

        if (candidate.moveDistance != best.moveDistance)
            return candidate.moveDistance < best.moveDistance;

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
            if (!safeSlots[candidateSlotIndex] ||
                plannedOccupancy[candidateSlotIndex] >= KILJAEDEN_MAX_BOTS_PER_RANGED_SLOT)
            {
                continue;
            }

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
}

bool IsKiljaedenCastingDarknessOfAThousandSouls(Unit* kiljaeden)
{
    return kiljaeden &&
        kiljaeden->FindCurrentSpellBySpellId(Id(SwpSpells::SPELL_DARKNESS_OF_A_THOUSAND_SOULS));
}

// Milliseconds until Darkness of a Thousand Souls lands, or -1 if he isn't casting it. Its cast bar
// comes before its channel, and GetCastTimeRemaining reads only the one in progress.
int32 GetKiljaedenDarknessTimeLeftMs(Unit* kiljaeden)
{
    if (!kiljaeden)
        return -1;

    Spell* darkness =
        kiljaeden->FindCurrentSpellBySpellId(Id(SwpSpells::SPELL_DARKNESS_OF_A_THOUSAND_SOULS));
    if (!darkness)
        return -1;

    int32 timeLeftMs = darkness->GetCastTimeRemaining();
    if (darkness->getState() == SPELL_STATE_PREPARING)
        timeLeftMs += darkness->GetSpellInfo()->GetDuration();

    return timeLeftMs;
}

bool IsKiljaedenStartDpsHoldActive(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    if (PlayerbotAI::IsMainTank(bot))
        return false;

    KiljaedenEncounterState const* state = FindKiljaedenState(bot->GetInstanceId());
    if (state && state->dpsHoldStartMs)
        return getMSTimeDiff(*state->dpsHoldStartMs, getMSTime()) < KILJAEDEN_DPS_HOLD_MS;

    // He is found once he engages the raid, which is when he becomes attackable.
    return GetSwpBoss(bot, SwpBosses::DATA_KILJAEDEN) != nullptr;
}

// Whether a straight line from one point to another passes too close to Kil'jaeden's center. A line
// whose nearest point to him is either end doesn't, so a bot moving away from him, or onto a
// position on the ring, goes straight.
bool IsLineThroughKiljaedenKnockback(Position const& from, Position const& to)
{
    Position const& center = SUNWELL_CENTER_POSITION;
    float const fromX = from.GetPositionX();
    float const fromY = from.GetPositionY();
    float const deltaX = to.GetPositionX() - fromX;
    float const deltaY = to.GetPositionY() - fromY;
    float const lengthSq = deltaX * deltaX + deltaY * deltaY;
    if (lengthSq <= 0.0f)
        return false;

    float const t = ((center.GetPositionX() - fromX) * deltaX +
        (center.GetPositionY() - fromY) * deltaY) / lengthSq;
    if (t <= 0.0f || t >= 1.0f)
        return false;

    return center.GetExactDist2d(fromX + deltaX * t, fromY + deltaY * t) <
        KILJAEDEN_ROUTE_LINE_CLEARANCE;
}

// Whether meleeing a unit means entering Kil'jaeden's Knockback: the unit stands inside the ring
// the route keeps clear, or the straight line to it crosses the Knockback.
bool IsMeleeBlockedByKiljaedenKnockback(Position const& from, Position const& target)
{
    return SUNWELL_CENTER_POSITION.GetExactDist2d(target) < KILJAEDEN_ROUTE_LINE_CLEARANCE ||
        IsLineThroughKiljaedenKnockback(from, target);
}

// One step toward the destination, going round Kil'jaeden's Knockback when the straight line would
// cross it. Returns false once the bot is within arrivalDist (2D) of the destination.
bool GetKiljaedenStepToPosition(
    Player* bot, Position const& destination, float arrivalDist, float& stepX, float& stepY)
{
    if (bot->GetExactDist2d(destination) <= arrivalDist)
        return false;

    Position detourPoint;
    bool const isDetour = TryGetKiljaedenDetourPoint(bot, destination, detourPoint);
    bool backwards;
    return GetStepToPosition(
        bot, isDetour ? detourPoint : destination, isDetour ? 0.0f : arrivalDist, nullptr, stepX,
        stepY, backwards);
}

// Where a bot with Fire Bloom waits during Darkness: FIRE_BLOOM_STANDOFF from the stack toward the
// bot, turned round the stack to just outside the Knockback ring when it would fall inside it.
Position GetKiljaedenFireBloomWaitPosition(Player* bot)
{
    Position const& stack = KILJAEDEN_DARKNESS_POSITION;
    Position const& center = SUNWELL_CENTER_POSITION;
    float const stackDistance = stack.GetExactDist2d(center);
    float const towardCenter = stack.GetAngle(&center);

    // The angle from the stack-to-center line at which the wait circle meets the ring
    float const ringCos = (FIRE_BLOOM_STANDOFF * FIRE_BLOOM_STANDOFF +
        stackDistance * stackDistance -
        KILJAEDEN_KNOCKBACK_CLEARANCE * KILJAEDEN_KNOCKBACK_CLEARANCE) /
        (2.0f * FIRE_BLOOM_STANDOFF * stackDistance);
    float const ringAngle = std::acos(std::clamp(ringCos, -1.0f, 1.0f));

    float angle = stack.GetAngle(bot);
    float const offset = Position::NormalizeOrientation(angle - towardCenter);
    if (offset < ringAngle)
        angle = towardCenter + ringAngle;
    else if (offset > 2.0f * static_cast<float>(M_PI) - ringAngle)
        angle = towardCenter - ringAngle;

    return Position(
        stack.GetPositionX() + std::cos(angle) * FIRE_BLOOM_STANDOFF,
        stack.GetPositionY() + std::sin(angle) * FIRE_BLOOM_STANDOFF, bot->GetPositionZ());
}

// Whether the bot clears Fire Bloom with its immunity (Cloak of Shadows, Ice Block, or Divine
// Shield) as soon as the global cooldown allows, so it need not move away with it. Cloak of Shadows
// has a short cooldown; Ice Block and Divine Shield are saved for phase 4. The global cooldown is
// left out on purpose: all three spells are on it, and a bot that waits out the rest of it should
// not set off for its Fire Bloom position meanwhile.
bool CanClearKiljaedenFireBloomWithImmunity(Player* bot, Unit* kiljaeden)
{
    if (PlayerbotAI::IsTank(bot))
        return false;

    uint32 const spellId = GetSelfImmunitySpell(bot);
    if (!spellId || !bot->HasSpell(spellId) || bot->HasSpellCooldown(spellId))
        return false;

    if (bot->getClass() != CLASS_ROGUE &&
        (!kiljaeden || kiljaeden->GetHealthPct() >= KILJAEDEN_PHASE4_HP_THRESHOLD))
    {
        return false;
    }

    // Forbearance blocks Divine Shield, and Hypothermia Ice Block.
    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
    return spellInfo &&
        !(spellInfo->ExcludeCasterAuraSpell && bot->HasAura(spellInfo->ExcludeCasterAuraSpell));
}

GuidVector FindKiljaedenDragonOrbGuids(Player* bot)
{
    GuidVector guids;
    guids.reserve(KILJAEDEN_DRAGON_ORB_ENTRIES.size());

    for (uint32 const orbEntry : KILJAEDEN_DRAGON_ORB_ENTRIES)
    {
        if (GameObject* orb =
                bot->FindNearestGameObject(orbEntry, DRAGON_ORB_SEARCH_RADIUS, true))
        {
            guids.push_back(orb->GetGUID());
        }
    }

    return guids;
}

// The orb user is the first living assistant bot in group order; with none, a player flies. As with
// the class roles, it is picked again out of encounters and, during one, replaced only if it dies
// or leaves, with the handover announced.
bool UpdateKiljaedenDragonOrbUser(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    ClassRoleAssignment& assignment = SwpState(bot->GetInstanceId()).kiljaedenDragonOrbUser;
    bool const isInEncounter = IsEncounterInProgress(bot, SWP_MAP_ID);
    if (isInEncounter && GetClassRolePlayer(bot, assignment))
        return false;

    ClassRoleChoice next;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || member->GetMapId() != SWP_MAP_ID ||
            !GET_PLAYERBOT_AI(member) || !group->IsAssistant(member->GetGUID()))
        {
            continue;
        }

        next = { member->GetGUID(), ClassRoleChoiceReason::Assistant };
        break;
    }

    if (assignment.choice == next)
        return false;

    bool const isHandover = isInEncounter && assignment.choice && !next.guid.IsEmpty();
    assignment.choice = next;

    if (isHandover && SayKiljaedenDragonOrbUser(botAI, next, true))
        assignment.announcedChoice = next;

    return true;
}

// Announces an unannounced orb user once the bot is within the announce distance and below the
// announce Z coordinate, and forgets the announcement beyond the rearm distance. The controller
// dies with Kil'jaeden, so nothing is announced after the kill.
bool UpdateKiljaedenDragonOrbUserAnnouncement(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    ClassRoleAssignment& assignment = SwpState(bot->GetInstanceId()).kiljaedenDragonOrbUser;
    float const distance = bot->GetExactDist2d(SUNWELL_CENTER_POSITION);
    if (distance > KILJAEDEN_ORB_USER_REARM_DISTANCE)
        return ResetIfSet(assignment.announcedChoice);

    if (distance > KILJAEDEN_ORB_USER_ANNOUNCE_DISTANCE ||
        bot->GetPositionZ() > KILJAEDEN_ORB_USER_ANNOUNCE_Z ||
        !assignment.choice || assignment.choice == assignment.announcedChoice)
    {
        return false;
    }

    if (!bot->FindNearestCreature(
            Id(SwpNpcs::NPC_KILJAEDEN_CONTROLLER), KILJAEDEN_ORB_USER_REARM_DISTANCE))
    {
        return false;
    }

    assignment.announcedChoice = assignment.choice;
    SayKiljaedenDragonOrbUser(botAI, *assignment.choice, false);
    return true;
}

// Null if nobody is the orb user, or it is dead or not in the bot's instance.
Player* GetKiljaedenDragonOrbUser(Player* bot)
{
    SwpInstanceState const* state = FindSwpState(bot->GetInstanceId());
    return state ? GetClassRolePlayer(bot, state->kiljaedenDragonOrbUser) : nullptr;
}

bool HasUsedKiljaedenDragonOrb(Player* bot)
{
    KiljaedenEncounterState const* state = FindKiljaedenState(bot->GetInstanceId());
    return state && state->dragonOrbUsers.contains(bot->GetGUID());
}

bool HasKiljaedenDragonAura(Player* bot)
{
    return bot->HasAura(Id(SwpSpells::SPELL_VENGEANCE_OF_THE_BLUE_FLIGHT));
}

Unit* GetKiljaedenControlledDragon(Player* bot)
{
    Unit* dragon = bot->GetCharm();
    if (!dragon || !dragon->IsAlive() ||
        dragon->GetEntry() != Id(SwpNpcs::NPC_POWER_OF_THE_BLUE_FLIGHT))
    {
        return nullptr;
    }

    return dragon;
}

bool CastKiljaedenDragonSpell(Unit* dragon, uint32 spellId)
{
    if (!dragon || dragon->HasSpellCooldown(spellId))
        return false;

    dragon->CastSpell(dragon, spellId, true);
    dragon->AddSpellCooldown(spellId, 0, GetManualCastCooldown(spellId));

    // The engine records no global cooldown for a triggered cast, so hold the dragon's other
    // abilities here. Without it, Haste and Revitalize can cast on consecutive ticks.
    if (uint32 const globalCooldownMs = GetManualCastGlobalCooldown(spellId))
    {
        for (uint32 otherSpellId : KILJAEDEN_DRAGON_SPELLS)
        {
            if (otherSpellId != spellId && !dragon->HasSpellCooldown(otherSpellId))
                dragon->AddSpellCooldown(otherSpellId, 0, globalCooldownMs);
        }
    }

    return true;
}

Player* FindBestKiljaedenDragonClusterTarget(Player* bot, Unit* dragon, uint32 spellId)
{
    if (!dragon)
        return nullptr;

    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    Player* bestTarget = nullptr;
    uint32 bestClusterSize = 0;
    uint32 bestTotalClusterSize = 0;
    float bestDistanceToDragon = 0.0f;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* candidate = ref->GetSource();
        if (!IsDragonGroupTarget(bot, candidate) || HasAuraFromDragon(candidate, spellId))
            continue;

        uint32 clusterSize = 0;
        uint32 totalClusterSize = 0;
        for (GroupReference* otherRef = group->GetFirstMember();
             otherRef; otherRef = otherRef->next())
        {
            Player* other = otherRef->GetSource();
            if (!IsDragonGroupTarget(bot, other) ||
                candidate->GetExactDist2d(other) > KILJAEDEN_DRAGON_CLUSTER_RADIUS)
            {
                continue;
            }

            ++totalClusterSize;
            if (!HasAuraFromDragon(other, spellId))
                ++clusterSize;
        }

        if (clusterSize < KILJAEDEN_DRAGON_MIN_CLUSTER_SIZE)
            continue;

        float const distanceToDragon = dragon->GetExactDist2d(candidate);

        auto const isBetter = [&]() -> bool
        {
            if (!bestTarget)
                return true;
            if (clusterSize != bestClusterSize)
                return clusterSize > bestClusterSize;
            if (totalClusterSize != bestTotalClusterSize)
                return totalClusterSize > bestTotalClusterSize;
            if (distanceToDragon != bestDistanceToDragon)
                return distanceToDragon < bestDistanceToDragon;
            return candidate->GetGUID() < bestTarget->GetGUID();
        };

        if (isBetter())
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
    float closestDistance = std::numeric_limits<float>::max();

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!IsDragonGroupTarget(bot, member) || HasAuraFromDragon(member, spellId))
            continue;

        float const distance = dragon->GetExactDist2d(member);
        if (distance < closestDistance)
        {
            closestTarget = member;
            closestDistance = distance;
        }
    }

    return closestTarget;
}

bool HasStaleRootFlag(Player* bot)
{
    // This helper ensures no genuine roots are cleared because one of the unit states below is
    // always present during a genuine root via Unit::SetRooted.
    return bot->IsRooted() &&
        !bot->HasUnitState(UNIT_STATE_ROOT | UNIT_STATE_STUNNED | UNIT_STATE_LOGOUT_TIMER);
}

bool ReleaseStaleRootFlag(Player* bot)
{
    if (!HasStaleRootFlag(bot))
        return false;

    bot->m_movementInfo.RemoveMovementFlag(MOVEMENTFLAG_ROOT);
    bot->SendMovementFlagUpdate();
    return true;
}

// Shared encounter state

KiljaedenEncounterState& KiljaedenState(uint32 instanceId)
{
    std::optional<KiljaedenEncounterState>& state = SwpState(instanceId).kiljaeden;
    if (!state)
        state.emplace();

    return *state;
}

KiljaedenEncounterState* FindKiljaedenState(uint32 instanceId)
{
    SwpInstanceState* state = FindSwpState(instanceId);
    return state && state->kiljaeden ? &*state->kiljaeden : nullptr;
}

}
