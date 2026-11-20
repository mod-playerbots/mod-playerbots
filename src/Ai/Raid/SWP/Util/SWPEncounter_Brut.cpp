/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "SWPEncounter_Brut.h"
#include "EncounterHelpers.h"
#include "Playerbots.h"
#include "SWPInstanceState.h"
#include "SWPShared.h"
#include <algorithm>
#include <cmath>
#include <optional>
#include <vector>

using namespace EncounterHelpers;

namespace SwpHelpers
{

// Note: Brutallus's CombatReach is 18.0f

namespace
{

bool IsBurnPadActive(BrutallusEncounterState const& state, ObjectGuid ownerGuid)
{
    auto const burnStateItr = state.rangedBurnStates.find(ownerGuid);
    return burnStateItr != state.rangedBurnStates.end() &&
        burnStateItr->second != BrutallusRangedBurnState::None;
}

bool TryGetBurnPadIndex(
    BrutallusEncounterState& state, Player* bot, uint8 rangedIndex, uint8& padIndex)
{
    auto& assignments = state.rangedBurnPadAssignments;
    for (auto itr = assignments.begin(); itr != assignments.end();)
    {
        if (itr->first != bot->GetGUID() && !IsBurnPadActive(state, itr->first))
        {
            itr = assignments.erase(itr);
            continue;
        }

        ++itr;
    }

    auto const existingItr = assignments.find(bot->GetGUID());
    if (existingItr != assignments.end() &&
        existingItr->second < BRUTALLUS_TOTAL_BURN_PADS)
    {
        padIndex = existingItr->second;
        return true;
    }

    std::array<bool, BRUTALLUS_TOTAL_BURN_PADS> usedPads = {};
    for (auto const& assignment : assignments)
    {
        if (assignment.second < BRUTALLUS_TOTAL_BURN_PADS)
            usedPads[assignment.second] = true;
    }

    static_assert(BRUTALLUS_TOTAL_BURN_PADS == 8,
        "Burn pad order tables and angle offsets assume exactly 8 pads");

    static constexpr std::array<uint8, BRUTALLUS_BURN_PADS_PER_GROUP>
        mainGroupPriority = { 0, 1, 2, 3 };
    static constexpr std::array<uint8, BRUTALLUS_BURN_PADS_PER_GROUP>
        mainGroupOverflow = { 4, 5, 6, 7 };
    static constexpr std::array<uint8, BRUTALLUS_BURN_PADS_PER_GROUP>
        assistGroupPriority = { 7, 6, 5, 4 };
    static constexpr std::array<uint8, BRUTALLUS_BURN_PADS_PER_GROUP>
        assistGroupOverflow = { 3, 2, 1, 0 };

    auto const assignFromOrder = [&](std::array<uint8, BRUTALLUS_BURN_PADS_PER_GROUP> const& order)
    {
        for (uint8 candidate : order)
        {
            if (usedPads[candidate])
                continue;

            assignments[bot->GetGUID()] = candidate;
            padIndex = candidate;
            return true;
        }
        return false;
    };

    if (rangedIndex % 2 == 0)
        return assignFromOrder(mainGroupPriority) || assignFromOrder(mainGroupOverflow);

    return assignFromOrder(assistGroupPriority) || assignFromOrder(assistGroupOverflow);
}

bool ShouldRebuildAssignments(uint32& lastRebuildMs)
{
    uint32 const now = getMSTime();
    if (lastRebuildMs &&
        getMSTimeDiff(lastRebuildMs, now) < BRUTALLUS_ASSIGNMENT_REBUILD_INTERVAL_MS)
    {
        return false;
    }

    lastRebuildMs = now;
    return true;
}

void PruneAssignments(
    std::unordered_map<ObjectGuid, uint8>& assignments,
    std::vector<ObjectGuid> const& eligibleGuids)
{
    for (auto itr = assignments.begin(); itr != assignments.end();)
    {
        if (std::find(eligibleGuids.begin(), eligibleGuids.end(), itr->first) ==
            eligibleGuids.end())
        {
            itr = assignments.erase(itr);
            continue;
        }

        ++itr;
    }
}

void EnsureRangedAssignments(Group* group, BrutallusEncounterState& state)
{
    if (!ShouldRebuildAssignments(state.rangedAssignmentRebuildMs))
        return;

    auto& assignments = state.rangedAssignments;

    std::vector<ObjectGuid> eligibleGuids;
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

        eligibleGuids.push_back(member->GetGUID());

        if (assignments.find(member->GetGUID()) != assignments.end())
            continue;

        if (PlayerbotAI::IsHeal(member))
            healers.push_back(member);
        else
            rangedDamage.push_back(member);
    }

    PruneAssignments(assignments, eligibleGuids);

    std::array<bool, BRUTALLUS_TOTAL_RANGED_POSITIONS> usedPositions = {};
    for (auto const& assignment : assignments)
    {
        if (assignment.second < BRUTALLUS_TOTAL_RANGED_POSITIONS)
            usedPositions[assignment.second] = true;
    }

    auto const assignNextOpenSlot = [&](Player* member)
    {
        for (uint8 slotIndex = 0; slotIndex < BRUTALLUS_TOTAL_RANGED_POSITIONS; ++slotIndex)
        {
            if (usedPositions[slotIndex])
                continue;

            assignments[member->GetGUID()] = slotIndex;
            usedPositions[slotIndex] = true;
            return;
        }

        assignments[member->GetGUID()] =
            static_cast<uint8>(assignments.size() % BRUTALLUS_TOTAL_RANGED_POSITIONS);
    };

    for (Player* member : healers)
        assignNextOpenSlot(member);

    for (Player* member : rangedDamage)
        assignNextOpenSlot(member);
}

void EnsureMeleeAssignments(Group* group, BrutallusEncounterState& state)
{
    if (!ShouldRebuildAssignments(state.meleeAssignmentRebuildMs))
        return;

    auto& assignments = state.meleeAssignments;

    std::vector<ObjectGuid> eligibleGuids;
    std::vector<Player*> unassigned;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member->GetMapId() != SWP_MAP_ID || !GET_PLAYERBOT_AI(member))
            continue;

        if (!PlayerbotAI::IsMelee(member) || PlayerbotAI::IsMainTank(member) ||
            PlayerbotAI::IsAssistTankOfIndex(member, 0, true))
        {
            continue;
        }

        eligibleGuids.push_back(member->GetGUID());

        if (assignments.find(member->GetGUID()) == assignments.end())
            unassigned.push_back(member);
    }

    PruneAssignments(assignments, eligibleGuids);

    std::array<bool, BRUTALLUS_TOTAL_MELEE_POSITIONS> usedPositions = {};
    for (auto const& assignment : assignments)
    {
        if (assignment.second < BRUTALLUS_TOTAL_MELEE_POSITIONS)
            usedPositions[assignment.second] = true;
    }

    auto const assignNextOpenSlot = [&](Player* member)
    {
        for (uint8 slotIndex = 0; slotIndex < BRUTALLUS_TOTAL_MELEE_POSITIONS; ++slotIndex)
        {
            if (usedPositions[slotIndex])
                continue;

            assignments[member->GetGUID()] = slotIndex;
            usedPositions[slotIndex] = true;
            return;
        }

        assignments[member->GetGUID()] =
            static_cast<uint8>(assignments.size() % BRUTALLUS_TOTAL_MELEE_POSITIONS);
    };

    for (Player* member : unassigned)
        assignNextOpenSlot(member);
}

float GetTankAngle(Unit* brutallus, Player* tank, float fallbackAngle)
{
    if (!brutallus || !tank)
        return Position::NormalizeOrientation(fallbackAngle);

    return Position::NormalizeOrientation(std::atan2(
        tank->GetPositionY() - brutallus->GetPositionY(),
        tank->GetPositionX() - brutallus->GetPositionX()));
}

// The signed difference between two world angles, from -pi to pi.
float GetAngleOffset(float angle, float reference)
{
    float offset = Position::NormalizeOrientation(angle - reference);
    if (offset > static_cast<float>(M_PI))
        offset -= 2.0f * static_cast<float>(M_PI);

    return offset;
}

// Where the main tank stands once Brutallus is on the anchor: on its own bearing from him if
// that is within tolerance of BRUTALLUS_MAIN_TANK_BEARING, otherwise on the nearer limit.
Position GetMainTankStandInPosition(Player* bot, Unit& brutallus)
{
    float const offset = std::clamp(
        GetAngleOffset(brutallus.GetAbsoluteAngle(bot), BRUTALLUS_MAIN_TANK_BEARING),
        -BRUTALLUS_MAIN_TANK_BEARING_TOLERANCE, BRUTALLUS_MAIN_TANK_BEARING_TOLERANCE);

    return GetBrutallusPositionAtAngle(
        bot, brutallus, BRUTALLUS_MAIN_TANK_BEARING + offset, BRUTALLUS_TANK_POSITION_RADIUS);
}

// If the straight line from the bot to the destination passes inside the crate's clearance,
// the destination becomes a point just outside it, on the side of the crate the line passes.
void RouteAroundCrate(Player* bot, Position& destination)
{
    Position const& crate = BRUTALLUS_CRATE_POSITION;
    if (crate.GetExactDist2d(destination) < BRUTALLUS_CRATE_CLEARANCE)
        return;

    float const botX = bot->GetPositionX();
    float const botY = bot->GetPositionY();
    float const lineX = destination.GetPositionX() - botX;
    float const lineY = destination.GetPositionY() - botY;
    float const lineLength = std::hypot(lineX, lineY);
    if (lineLength < 0.1f)
        return;

    float const along = std::clamp(
        ((crate.GetPositionX() - botX) * lineX + (crate.GetPositionY() - botY) * lineY) /
        (lineLength * lineLength), 0.0f, 1.0f);

    float const nearestX = botX + lineX * along;
    float const nearestY = botY + lineY * along;
    float const nearestDist = crate.GetExactDist2d(nearestX, nearestY);
    if (nearestDist >= BRUTALLUS_CRATE_CLEARANCE)
        return;

    // A line through the crate's center has no side, so take the line's left.
    float sideX = -lineY / lineLength;
    float sideY = lineX / lineLength;
    if (nearestDist > 0.1f)
    {
        sideX = (nearestX - crate.GetPositionX()) / nearestDist;
        sideY = (nearestY - crate.GetPositionY()) / nearestDist;
    }

    constexpr float waypointMargin = 1.5f;
    float const waypointDist = BRUTALLUS_CRATE_CLEARANCE + waypointMargin;
    destination.Relocate(
        crate.GetPositionX() + sideX * waypointDist, crate.GetPositionY() + sideY * waypointDist,
        destination.GetPositionZ());
}

} // end anonymous namespace

float GetBrutallusMainTankAngle(Unit* brutallus, Player* mainTank)
{
    return GetTankAngle(brutallus, mainTank, BRUTALLUS_MAIN_TANK_BEARING);
}

float GetBrutallusAssistTankAngle(Unit* brutallus, Player* assistTank, float mainTankAngle)
{
    return GetTankAngle(brutallus, assistTank, Position::NormalizeOrientation(
        mainTankAngle + BRUTALLUS_ASSIST_TANK_ANGLE_OFFSET));
}

Position GetBrutallusPositionAtAngle(Player* bot, Unit& brutallus, float angle, float radius)
{
    float const x = brutallus.GetPositionX() + std::cos(angle) * radius;
    float const y = brutallus.GetPositionY() + std::sin(angle) * radius;
    return Position(x, y, bot->GetPositionZ());
}

// Where the main tank is heading while it places Brutallus. With him off the anchor, it first
// lines up on the far side of him from the anchor, then backs out past the anchor so that he
// follows onto it. With him on the anchor, it stands in.
Position GetBrutallusMainTankDestination(Player* bot, Unit& brutallus)
{
    Position const& anchor = BRUTALLUS_ANCHOR_POSITION;
    Position destination;

    if (brutallus.GetExactDist2d(anchor) > BRUTALLUS_ANCHOR_TOLERANCE)
    {
        float const anchorAngle = brutallus.GetAbsoluteAngle(anchor);
        float const lineUpOffset = GetAngleOffset(brutallus.GetAbsoluteAngle(bot), anchorAngle);
        if (std::fabs(lineUpOffset) > BRUTALLUS_TANK_LINE_UP_ANGLE)
        {
            destination = GetBrutallusPositionAtAngle(
                bot, brutallus, anchorAngle, BRUTALLUS_TANK_LINE_UP_RADIUS);
        }
        else
        {
            destination.Relocate(
                anchor.GetPositionX() + std::cos(anchorAngle) * BRUTALLUS_DRAG_DISTANCE,
                anchor.GetPositionY() + std::sin(anchorAngle) * BRUTALLUS_DRAG_DISTANCE,
                bot->GetPositionZ());
        }
    }
    else
        destination = GetMainTankStandInPosition(bot, brutallus);

    RouteAroundCrate(bot, destination);
    return destination;
}

// One step of the main tank toward its destination that keeps Brutallus in front of it. A step
// sideways to him is replaced by a diagonal: in toward him while there is room, otherwise out.
// A step that is blocked or ends at the crate is refused, never shortened, and the headings to
// either side are tried in its place.
bool GetBrutallusMainTankStep(
    Player* bot, Unit* brutallus, Position const& destination, float& stepX, float& stepY,
    float& stepZ, bool& backwards)
{
    if (!brutallus)
        return false;

    float const distance = bot->GetExactDist2d(destination);
    float const brutallusDist = bot->GetExactDist2d(brutallus);
    float const brutallusAngle = bot->GetAbsoluteAngle(brutallus);

    // Headings are measured from the line to Brutallus: 0 is straight at him.
    float const heading = GetAngleOffset(bot->GetAbsoluteAngle(destination), brutallusAngle);
    float const awayLimit = static_cast<float>(M_PI) - BRUTALLUS_TANK_STEER_ANGLE;
    float steer = std::fabs(heading);
    if (brutallusDist <= BRUTALLUS_TANK_MIN_RADIUS)
    {
        // Too close to go further in, so out on the diagonal, on the destination's side.
        steer = std::max(steer, awayLimit);
    }
    else if (steer > BRUTALLUS_TANK_STEER_ANGLE && steer < awayLimit)
    {
        constexpr float roomToStepIn = 3.0f;
        steer = brutallusDist > BRUTALLUS_TANK_MIN_RADIUS + roomToStepIn ?
            BRUTALLUS_TANK_STEER_ANGLE : awayLimit;
    }

    if (heading < 0.0f)
        steer = -steer;

    constexpr float degreeToRadian = M_PI / 180.0f;
    static constexpr std::array turns = {
        0.0f,
        20.0f * degreeToRadian,
        -20.0f * degreeToRadian,
        40.0f * degreeToRadian,
        -40.0f * degreeToRadian,
        60.0f * degreeToRadian,
        -60.0f * degreeToRadian,
        80.0f * degreeToRadian,
        -80.0f * degreeToRadian,
    };

    // Slack for float error, so a heading right on a limit is not lost.
    constexpr float angleSlack = 0.01f;

    for (float turn : turns)
    {
        float const candidate = GetAngleOffset(steer + turn, 0.0f);
        float const candidateSize = std::fabs(candidate);

        bool forwards;
        if (candidateSize <= BRUTALLUS_TANK_STEER_ANGLE + angleSlack)
            forwards = true;
        else if (candidateSize >= awayLimit - angleSlack)
            forwards = false;
        else
            continue;

        if (forwards && brutallusDist <= BRUTALLUS_TANK_MIN_RADIUS)
            continue;

        float const stepDistance = std::min(
            forwards ? PATH_STEP_DISTANCE : PATH_BACKWARD_STEP_DISTANCE, distance);

        float const angle = brutallusAngle + candidate;
        float const x = bot->GetPositionX() + std::cos(angle) * stepDistance;
        float const y = bot->GetPositionY() + std::sin(angle) * stepDistance;

        if (BRUTALLUS_CRATE_POSITION.GetExactDist2d(x, y) < BRUTALLUS_CRATE_CLEARANCE ||
            !CanTakeStepTowards(bot, x, y, stepDistance, stepX, stepY, stepZ))
        {
            continue;
        }

        backwards = !forwards;
        return true;
    }

    return false;
}

bool IsBrutallusMainTankInPlace(Player* bot, Unit* brutallus)
{
    if (!brutallus ||
        brutallus->GetExactDist2d(BRUTALLUS_ANCHOR_POSITION) > BRUTALLUS_ANCHOR_TOLERANCE)
    {
        return false;
    }

    return bot->GetExactDist2d(GetMainTankStandInPosition(bot, *brutallus)) <=
        BRUTALLUS_TANK_ARRIVAL_DISTANCE;
}

bool TryGetBrutallusAssignedPositionIndex(Player* bot, uint8& positionIndex)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    bool const isRanged = PlayerbotAI::IsRanged(bot);
    auto& state = BrutallusState(bot->GetInstanceId());

    if (isRanged)
        EnsureRangedAssignments(group, state);
    else
        EnsureMeleeAssignments(group, state);

    auto& assignments = isRanged ? state.rangedAssignments : state.meleeAssignments;
    auto const assignmentItr = assignments.find(bot->GetGUID());
    if (assignmentItr == assignments.end())
        return false;

    positionIndex = assignmentItr->second;
    return true;
}

bool TryGetBrutallusRangedPosition(
    Player* bot, Unit* brutallus, Player* mainTank, Player* assistTank, uint8 rangedIndex,
    float radius, Position& position)
{
    if (!brutallus || rangedIndex >= BRUTALLUS_TOTAL_RANGED_POSITIONS)
        return false;

    bool const isMainTankGroup = rangedIndex % 2 == 0;
    uint8 const arcPositionIndex =
        static_cast<uint8>((rangedIndex / 2) % BRUTALLUS_RANGED_POSITIONS_PER_GROUP);

    float const mainTankAngle = GetBrutallusMainTankAngle(brutallus, mainTank);
    float const assistTankAngle =
        GetBrutallusAssistTankAngle(brutallus, assistTank, mainTankAngle);

    float const tankAngle = isMainTankGroup ? mainTankAngle : assistTankAngle;
    float const angleOffset = GetCenteredArcSlotAngleOffset(
        arcPositionIndex, BRUTALLUS_RANGED_POSITIONS_PER_GROUP,
        BRUTALLUS_RANGED_GROUP_ARC_WIDTH);

    float const angle = Position::NormalizeOrientation(tankAngle + angleOffset);

    position = GetBrutallusPositionAtAngle(bot, *brutallus, angle, radius);
    return true;
}

bool TryGetBrutallusBurnPadPosition(
    Player* bot, Unit* brutallus, Player* mainTank, uint8 rangedIndex, float radius,
    Position& position)
{
    if (!brutallus || rangedIndex >= BRUTALLUS_TOTAL_RANGED_POSITIONS)
        return false;

    uint8 padIndex = 0;
    if (!TryGetBurnPadIndex(BrutallusState(bot->GetInstanceId()), bot, rangedIndex, padIndex))
        return false;

    constexpr float degreeToRadian = M_PI / 180.0f;
    static constexpr std::array burnPadAngleOffsets = {
        70.0f * degreeToRadian,
        83.3f * degreeToRadian,
        96.7f * degreeToRadian,
        110.0f * degreeToRadian,
        130.0f * degreeToRadian,
        143.3f * degreeToRadian,
        156.7f * degreeToRadian,
        170.0f * degreeToRadian
    };

    float const mainTankAngle = GetBrutallusMainTankAngle(brutallus, mainTank);
    float const angle = Position::NormalizeOrientation(
        mainTankAngle + burnPadAngleOffsets[padIndex]);

    position = GetBrutallusPositionAtAngle(bot, *brutallus, angle, radius);
    return true;
}

bool TryGetBrutallusLaneTraversalPosition(
    Player* bot, Unit* brutallus, float targetX, float targetY, float radius, float currentX,
    float currentY, Position& position)
{
    if (!brutallus)
        return false;

    float const targetAngle = Position::NormalizeOrientation(std::atan2(
        targetY - brutallus->GetPositionY(), targetX - brutallus->GetPositionX()));

    float const currentAngle = Position::NormalizeOrientation(
        std::atan2(currentY - brutallus->GetPositionY(), currentX - brutallus->GetPositionX()));

    float remainingAngle = Position::NormalizeOrientation(targetAngle - currentAngle);
    if (remainingAngle > static_cast<float>(M_PI))
        remainingAngle -= 2.0f * static_cast<float>(M_PI);

    constexpr float stepDistance = 3.0f;
    float const stepRatio = stepDistance / (2.0f * radius);
    float const clampedStepRatio = std::clamp(stepRatio, 0.0f, 1.0f);
    float const stepAngle = 2.0f * std::asin(clampedStepRatio);
    float nextAngle = targetAngle;

    if (std::fabs(remainingAngle) > stepAngle)
    {
        nextAngle = Position::NormalizeOrientation(
            currentAngle + std::copysign(stepAngle, remainingAngle));
    }

    position = GetBrutallusPositionAtAngle(bot, *brutallus, nextAngle, radius);
    return true;
}

bool ReleaseBrutallusBurnPad(Player* bot)
{
    BrutallusEncounterState* state = FindBrutallusState(bot->GetInstanceId());
    if (!state)
        return false;

    return state->rangedBurnPadAssignments.erase(bot->GetGUID()) > 0;
}

bool HasBrutallusBurn(Player* bot)
{
    return bot->HasAura(Id(SwpSpells::SPELL_BURN));
}

// Shared encounter state

BrutallusEncounterState& BrutallusState(uint32 instanceId)
{
    std::optional<BrutallusEncounterState>& state = SwpState(instanceId).brutallus;
    if (!state)
        state.emplace();

    return *state;
}

BrutallusEncounterState* FindBrutallusState(uint32 instanceId)
{
    SwpInstanceState* state = FindSwpState(instanceId);
    return state && state->brutallus ? &*state->brutallus : nullptr;
}

}
