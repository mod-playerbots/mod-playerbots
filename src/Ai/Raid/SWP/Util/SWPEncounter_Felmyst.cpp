/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "SWPEncounter_Felmyst.h"
#include "EncounterHelpers.h"
#include "Playerbots.h"
#include "SWPInstanceState.h"
#include "SWPShared.h"
#include <algorithm>
#include <cmath>
#include <list>

using namespace EncounterHelpers;

namespace SwpHelpers
{

// Note: Felmyst's CombatReach is 10.0f

namespace
{

FelmystDemonicVaporGuids const& GetCachedDemonicVapors(Player* bot)
{
    static FelmystDemonicVaporGuids const none;
    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
    if (!botAI)
        return none;

    return botAI->GetAiObjectContext()
        ->GetValue<FelmystDemonicVaporGuids>("felmyst demonic vapors")->RefGet();
}

Creature* GetLivingCreature(Player* bot, ObjectGuid const& guid)
{
    Creature* creature = ObjectAccessor::GetCreature(*bot, guid);
    return creature && creature->IsAlive() ? creature : nullptr;
}

bool TryGetGroundStackCenter(
    Player* bot, Unit* felmyst, FelmystGroundStack stack, float& positionX, float& positionY)
{
    if (!felmyst)
        return false;

    switch (stack)
    {
        case FelmystGroundStack::Melee:
        {
            constexpr float behindDistance = 12.5f;
            float const behindAngle = Position::NormalizeOrientation(
                GetFelmystFrontAngle(bot, felmyst) + M_PI);
            positionX = felmyst->GetPositionX() + behindDistance * std::cos(behindAngle);
            positionY = felmyst->GetPositionY() + behindDistance * std::sin(behindAngle);
            return true;
        }
        case FelmystGroundStack::Left:
        case FelmystGroundStack::Right:
        {
            constexpr float sideDistance = 24.0f;
            float const frontAngle = GetFelmystFrontAngle(bot, felmyst);
            float const sideAngle = frontAngle +
                (stack == FelmystGroundStack::Left ? M_PI_2 : -M_PI_2);
            positionX = felmyst->GetPositionX() + std::cos(sideAngle) * sideDistance;
            positionY = felmyst->GetPositionY() + std::sin(sideAngle) * sideDistance;
            return true;
        }
        default:
            return false;
    }
}

FogLocation GetFogLocationFromLanePointIndex(uint8 laneIndex, bool useLeftPoint)
{
    switch (laneIndex)
    {
        case 0:
            return useLeftPoint ? FogLocation::LeftTop : FogLocation::RightTop;
        case 1:
            return useLeftPoint ? FogLocation::LeftMiddle : FogLocation::RightMiddle;
        case 2:
            return useLeftPoint ? FogLocation::LeftBottom : FogLocation::RightBottom;
        default:
            return FogLocation::None;
    }
}

FogLane GetFogLaneFromLocation(FogLocation location)
{
    switch (location)
    {
        case FogLocation::LeftTop:
        case FogLocation::RightTop:
            return FogLane::Top;
        case FogLocation::LeftMiddle:
        case FogLocation::RightMiddle:
            return FogLane::Middle;
        case FogLocation::LeftBottom:
        case FogLocation::RightBottom:
            return FogLane::Bottom;
        default:
            return FogLane::None;
    }
}

bool IsFogSideLocation(FogLocation location)
{
    return location == FogLocation::LeftSide || location == FogLocation::RightSide;
}

// The unit normal of a lane's threshold line, pointing to its safe side.
bool TryGetFogThresholdNormal(FogLane lane, float& normalX, float& normalY)
{
    uint8 const laneIndex = static_cast<uint8>(lane);
    if (lane == FogLane::None || laneIndex >= FOG_SAFE_THRESHOLDS.size())
        return false;

    FogSafeThreshold const& threshold = FOG_SAFE_THRESHOLDS[laneIndex];
    float const alongX = threshold.b.GetPositionX() - threshold.a.GetPositionX();
    float const alongY = threshold.b.GetPositionY() - threshold.a.GetPositionY();
    float const length = std::hypot(alongX, alongY);
    if (length <= 0.0f)
        return false;

    normalX = (threshold.safeSideIsNorth ? -alongY : alongY) / length;
    normalY = (threshold.safeSideIsNorth ? alongX : -alongX) / length;
    return true;
}

// How far a position is past a lane's threshold line, extended past its ends; negative on the
// danger side.
float GetDistancePastFogThreshold(float positionX, float positionY, FogLane lane)
{
    float normalX = 0.0f;
    float normalY = 0.0f;
    if (!TryGetFogThresholdNormal(lane, normalX, normalY))
        return std::numeric_limits<float>::lowest();

    Position const& a = FOG_SAFE_THRESHOLDS[static_cast<uint8>(lane)].a;
    return (positionX - a.GetPositionX()) * normalX + (positionY - a.GetPositionY()) * normalY;
}

FogLocation GetFogLocationFromPosition(float positionX, float positionY, float matchDistance)
{
    float bestDistance = matchDistance;
    FogLocation bestLocation = FogLocation::None;

    float const leftSideDistance = std::hypot(
        positionX - FOG_LEFT_SIDE.GetPositionX(), positionY - FOG_LEFT_SIDE.GetPositionY());

    if (leftSideDistance <= bestDistance)
    {
        bestDistance = leftSideDistance;
        bestLocation = FogLocation::LeftSide;
    }

    float const rightSideDistance = std::hypot(
        positionX - FOG_RIGHT_SIDE.GetPositionX(), positionY - FOG_RIGHT_SIDE.GetPositionY());

    if (rightSideDistance <= bestDistance)
    {
        bestDistance = rightSideDistance;
        bestLocation = FogLocation::RightSide;
    }

    for (uint8 laneIndex = 0; laneIndex < FOG_LEFT_LANES.size(); ++laneIndex)
    {
        float const leftDistance = std::hypot(
            positionX - FOG_LEFT_LANES[laneIndex].GetPositionX(),
            positionY - FOG_LEFT_LANES[laneIndex].GetPositionY());

        if (leftDistance <= bestDistance)
        {
            bestDistance = leftDistance;
            bestLocation = GetFogLocationFromLanePointIndex(laneIndex, true);
        }

        float const rightDistance = std::hypot(
            positionX - FOG_RIGHT_LANES[laneIndex].GetPositionX(),
            positionY - FOG_RIGHT_LANES[laneIndex].GetPositionY());

        if (rightDistance <= bestDistance)
        {
            bestDistance = rightDistance;
            bestLocation = GetFogLocationFromLanePointIndex(laneIndex, false);
        }
    }

    return bestLocation;
}

bool TryGetFlightDestination(Unit* felmyst, Position& destination)
{
    if (!felmyst)
        return false;

    float destinationX = 0.0f;
    float destinationY = 0.0f;
    float destinationZ = 0.0f;

    if (!felmyst->GetMotionMaster()->GetDestination(destinationX, destinationY, destinationZ))
        return false;

    destination = Position{ destinationX, destinationY, destinationZ };
    return true;
}

bool IsSweeping(Unit* felmyst)
{
    return felmyst && felmyst->HasAura(Id(SwpSpells::SPELL_FELMYST_SPEED_BURST));
}

FogLocation GetCurrentFogLocation(Unit* felmyst)
{
    if (!felmyst)
        return FogLocation::None;

    return GetFogLocationFromPosition(
        felmyst->GetPositionX(), felmyst->GetPositionY(), FELMYST_LOCATION_MATCH_DISTANCE);
}

FogLocation GetDestinationFogLocation(Unit* felmyst)
{
    Position destination;
    if (!TryGetFlightDestination(felmyst, destination))
        return FogLocation::None;

    return GetFogLocationFromPosition(
        destination.GetPositionX(), destination.GetPositionY(), FELMYST_LOCATION_MATCH_DISTANCE);
}

bool IsNearLandingPosition(Position const& destination)
{
    bool const nearRight = destination.GetExactDist2d(FELMYST_RIGHT_LANDING_POSITION) <=
        FELMYST_LOCATION_MATCH_DISTANCE;
    bool const nearLeft = destination.GetExactDist2d(FELMYST_LEFT_LANDING_POSITION) <=
        FELMYST_LOCATION_MATCH_DISTANCE;

    return nearRight || nearLeft;
}

bool IsNearFelmystCrate(float positionX, float positionY)
{
    return FELMYST_CRATE_POSITION.GetExactDist2d(positionX, positionY) < FELMYST_CRATE_CLEARANCE;
}

bool IsNearDemonicVaporHazard(
    float positionX, float positionY, std::vector<Creature*> const& hazards, float clearance)
{
    for (Creature* hazard : hazards)
    {
        if (hazard->GetExactDist2d(positionX, positionY) < clearance)
            return true;
    }

    return false;
}

Position ClosestPointOnSegment(Position const& p, Position const& segA, Position const& segB)
{
    float const abX = segB.GetPositionX() - segA.GetPositionX();
    float const abY = segB.GetPositionY() - segA.GetPositionY();
    float const lenSq = abX * abX + abY * abY;
    if (lenSq <= 0.0f)
        return segA;

    float const t = std::clamp(
        ((p.GetPositionX() - segA.GetPositionX()) * abX +
         (p.GetPositionY() - segA.GetPositionY()) * abY) / lenSq, 0.0f, 1.0f);

    return Position(
        segA.GetPositionX() + t * abX, segA.GetPositionY() + t * abY, segA.GetPositionZ());
}

} // end anonymous namespace

FelmystDemonicVaporGuids FindFelmystDemonicVaporGuids(Player* bot)
{
    FelmystDemonicVaporGuids guids;

    auto const addGuids = [bot](uint32 entry, GuidVector& out)
    {
        std::list<Creature*> creatures;
        bot->GetCreatureListWithEntryInGrid(creatures, entry, DEMONIC_VAPOR_SEARCH_RADIUS);
        for (Creature* creature : creatures)
        {
            if (creature && creature->IsAlive())
                out.push_back(creature->GetGUID());
        }
    };

    addGuids(Id(SwpNpcs::NPC_DEMONIC_VAPOR), guids.heads);
    addGuids(Id(SwpNpcs::NPC_DEMONIC_VAPOR_TRAIL), guids.trails);
    return guids;
}

std::vector<Creature*> GetDemonicVaporHazards(Player* bot)
{
    FelmystDemonicVaporGuids const& guids = GetCachedDemonicVapors(bot);

    std::vector<Creature*> hazards;
    hazards.reserve(guids.heads.size() + guids.trails.size());

    for (ObjectGuid const& guid : guids.heads)
    {
        Creature* head = GetLivingCreature(bot, guid);
        if (head && head->GetSummonerGUID() != bot->GetGUID())
            hazards.push_back(head);
    }

    for (ObjectGuid const& guid : guids.trails)
    {
        if (Creature* trail = GetLivingCreature(bot, guid))
            hazards.push_back(trail);
    }

    return hazards;
}

// Any living head the bot summoned, which is the head chasing it.
Creature* GetFelmystOwnDemonicVapor(Player* bot)
{
    for (ObjectGuid const& guid : GetCachedDemonicVapors(bot).heads)
    {
        Creature* head = GetLivingCreature(bot, guid);
        if (head && head->GetSummonerGUID() == bot->GetGUID())
            return head;
    }

    return nullptr;
}

bool IsNearFelmystDemonicVaporHazard(Player* bot)
{
    return IsNearDemonicVaporHazard(
        bot->GetPositionX(), bot->GetPositionY(), GetDemonicVaporHazards(bot),
        DEMONIC_VAPOR_CLEARANCE);
}

// Called by the summon hook for every vapor, before the head appears.
void RecordFelmystDemonicVaporSummon(Player* target)
{
    if (!target)
        return;

    FelmystEncounterState& state = FelmystState(target->GetInstanceId());
    if (state.demonicVaporCount < std::numeric_limits<uint8>::max())
        ++state.demonicVaporCount;

    if (state.demonicVaporCount == 1)
        state.firstDemonicVaporTargetGuid = target->GetGUID();
}

namespace
{

bool TryFindSpotPastFogThreshold(
    Player* bot, FogLane lane, Position const& projectFrom, Position& destination)
{
    if (lane == FogLane::None)
        return false;

    uint8 const laneIndex = static_cast<uint8>(lane);
    if (laneIndex >= FOG_SAFE_THRESHOLDS.size())
        return false;

    Position const bestProjection = ClosestPointOnSegment(
        projectFrom, FOG_SAFE_THRESHOLDS[laneIndex].a, FOG_SAFE_THRESHOLDS[laneIndex].b);

    FogSafeThreshold const& threshold = FOG_SAFE_THRESHOLDS[laneIndex];

    float const perpX = threshold.safeSideIsNorth ?
        -(threshold.b.GetPositionY() - threshold.a.GetPositionY()) :
         (threshold.b.GetPositionY() - threshold.a.GetPositionY());
    float const perpY = threshold.safeSideIsNorth ?
        (threshold.b.GetPositionX() - threshold.a.GetPositionX()) :
        -(threshold.b.GetPositionX() - threshold.a.GetPositionX());

    float const segmentLength = std::hypot(perpX, perpY);
    if (segmentLength <= 0.0f)
        return false;

    float const unitX = perpX / segmentLength;
    float const unitY = perpY / segmentLength;

    float const alongX = (threshold.b.GetPositionX() - threshold.a.GetPositionX()) / segmentLength;
    float const alongY = (threshold.b.GetPositionY() - threshold.a.GetPositionY()) / segmentLength;

    std::vector<Creature*> const hazards = GetDemonicVaporHazards(bot);
    auto const isClearOfVapor = [&hazards](float x, float y)
    {
        for (Creature* hazard : hazards)
        {
            if (hazard && hazard->GetExactDist2d(x, y) < FOG_DESTINATION_VAPOR_CLEARANCE)
                return false;
        }

        return true;
    };

    constexpr float minThresholdClearance = 3.0f;
    constexpr float maxClearance = 30.0f;
    constexpr float clearanceStep = 3.0f;
    constexpr float maxLateralOffset = 20.0f;
    constexpr float lateralStep = 5.0f;

    struct FogCandidate
    {
        float x;
        float y;
        float distance;
    };

    std::vector<FogCandidate> candidates;
    float const projectionAlong =
        (bestProjection.GetPositionX() - threshold.a.GetPositionX()) * alongX +
        (bestProjection.GetPositionY() - threshold.a.GetPositionY()) * alongY;

    constexpr uint32 clearanceStepCount =
        static_cast<uint32>((maxClearance - minThresholdClearance) / clearanceStep);
    constexpr uint32 lateralStepCount =
        static_cast<uint32>(2.0f * maxLateralOffset / lateralStep);

    for (uint32 clearanceIndex = 0; clearanceIndex <= clearanceStepCount; ++clearanceIndex)
    {
        float const clearance =
            minThresholdClearance + static_cast<float>(clearanceIndex) * clearanceStep;

        for (uint32 lateralIndex = 0; lateralIndex <= lateralStepCount; ++lateralIndex)
        {
            float const lateral =
                -maxLateralOffset + static_cast<float>(lateralIndex) * lateralStep;

            float const along = projectionAlong + lateral;
            if (along < 0.0f || along > segmentLength)
                continue;

            float const x = bestProjection.GetPositionX() + unitX * clearance + alongX * lateral;
            float const y = bestProjection.GetPositionY() + unitY * clearance + alongY * lateral;
            candidates.push_back({ x, y, projectFrom.GetExactDist2d(x, y) });
        }
    }

    std::sort(candidates.begin(), candidates.end(),
        [](FogCandidate const& first, FogCandidate const& second)
        {
            return first.distance < second.distance;
        });

    for (FogCandidate const& candidate : candidates)
    {
        if (!isClearOfVapor(candidate.x, candidate.y))
            continue;

        // No raycast clamp from the bot: it held bots at obstacles. The air step goes around.
        float z = bot->GetMapWaterOrGroundLevel(candidate.x, candidate.y, bot->GetPositionZ());
        if (z <= INVALID_HEIGHT)
            z = bot->GetPositionZ();

        destination = Position(candidate.x, candidate.y, z);
        return true;
    }

    return false;
}

} // end anonymous namespace 2

bool TryGetFelmystLandingApproachDestination(
    Player* bot, FogLane lastCompletedLane, Unit* felmyst, Position& destination)
{
    if (!felmyst)
        return false;

    Position const projectFrom(
        felmyst->GetPositionX(), felmyst->GetPositionY(), felmyst->GetPositionZ());

    return TryFindSpotPastFogThreshold(bot, lastCompletedLane, projectFrom, destination);
}

Position const& GetFelmystMainTankGroundPosition(Player* bot)
{
    Position const* bestPosition = &FELMYST_TANK_POSITIONS[0];
    float bestDistance = std::numeric_limits<float>::max();

    for (Position const& position : FELMYST_TANK_POSITIONS)
    {
        float const distance = bot->GetExactDist2d(position);
        if (distance < bestDistance)
        {
            bestDistance = distance;
            bestPosition = &position;
        }
    }

    return *bestPosition;
}

bool TryGetFelmystGroundStackPosition(
    Player* bot, Unit* felmyst, FelmystGroundStack stack, Position& position)
{
    float destinationX = 0.0f;
    float destinationY = 0.0f;

    if (!TryGetGroundStackCenter(bot, felmyst, stack, destinationX, destinationY))
        return false;

    float destinationZ = bot->GetMapWaterOrGroundLevel(
        destinationX, destinationY, bot->GetPositionZ());

    if (destinationZ <= INVALID_HEIGHT)
        destinationZ = bot->GetPositionZ();

    // No raycast clamp from the bot: it froze bots behind a tree. The path step goes around.
    position = Position{ destinationX, destinationY, destinationZ };
    return true;
}

FelmystGroundStack GetClosestFelmystGroundStack(Player* bot, Unit* felmyst, Player* reference)
{
    if (!felmyst || !reference)
        return FelmystGroundStack::None;

    FelmystGroundStack bestStack = FelmystGroundStack::None;
    float bestDistance = std::numeric_limits<float>::max();
    for (FelmystGroundStack stack : {
             FelmystGroundStack::Melee, FelmystGroundStack::Left, FelmystGroundStack::Right })
    {
        float stackX = 0.0f;
        float stackY = 0.0f;
        if (!TryGetGroundStackCenter(bot, felmyst, stack, stackX, stackY))
            continue;

        float const stackDistance = reference->GetExactDist2d(stackX, stackY);
        if (stackDistance < bestDistance)
        {
            bestDistance = stackDistance;
            bestStack = stack;
        }
    }

    return bestStack;
}

float GetFelmystFrontAngle(Player* bot, Unit* felmyst)
{
    Position const& defaultTankPosition = GetFelmystMainTankGroundPosition(bot);
    float frontX = defaultTankPosition.GetPositionX();
    float frontY = defaultTankPosition.GetPositionY();

    Player* mainTank = GetGroupMainTank(bot);
    if (mainTank && mainTank->IsAlive() && mainTank->GetMapId() == felmyst->GetMapId())
    {
        frontX = mainTank->GetPositionX();
        frontY = mainTank->GetPositionY();
    }
    else if (Unit* victim = felmyst->GetVictim())
    {
        frontX = victim->GetPositionX();
        frontY = victim->GetPositionY();
    }

    return std::atan2(frontY - felmyst->GetPositionY(), frontX - felmyst->GetPositionX());
}

bool TryGetFelmystRangedPosition(Player* bot, Unit* felmyst, Position& position)
{
    if (!felmyst || bot->GetMapId() != SWP_MAP_ID || !PlayerbotAI::IsRanged(bot))
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    bool const botIsHealer = PlayerbotAI::IsHeal(bot);
    ObjectGuid const botGuid = bot->GetGUID();
    uint32 stackIndex = 0;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member->GetMapId() != SWP_MAP_ID || !GET_PLAYERBOT_AI(member) ||
            !PlayerbotAI::IsRanged(member))
        {
            continue;
        }

        if (PlayerbotAI::IsHeal(member) == botIsHealer && member->GetGUID() < botGuid)
            ++stackIndex;
    }

    // Healers spread across all three stacks; ranged dps alternates between the two sides
    FelmystGroundStack const stack = botIsHealer ?
        static_cast<FelmystGroundStack>(stackIndex % 3) :
        (stackIndex % 2 == 0 ? FelmystGroundStack::Left : FelmystGroundStack::Right);

    return TryGetFelmystGroundStackPosition(bot, felmyst, stack, position);
}

bool IsFelmystRangedStackActive(Player* bot, Unit* felmyst)
{
    if (!felmyst || felmyst->IsFlying() || !PlayerbotAI::IsRanged(bot) ||
        felmyst->GetVictim() == bot)
    {
        return false;
    }

    // On initial landing, let the MT get aggro before assuming positions
    Player* mainTank = GetGroupMainTank(bot);
    if (mainTank && felmyst->GetVictim() != mainTank &&
        felmyst->GetHealthPct() > BOSS_ENGAGED_HEALTH_PCT)
    {
        return false;
    }

    // Priests may leave the stack to Mass Dispel Gas Nova beyond their reach.
    if (bot->getClass() == CLASS_PRIEST && IsFelmystMassDispelReady(bot) &&
        GetFelmystGasNovaDispelTarget(bot))
    {
        return false;
    }

    return !GetFelmystEncapsulateTarget(bot) && !DidEncapsulateOccurThisGroundPhase(bot);
}

bool IsFelmystLanding(Unit* felmyst)
{
    Position destination;
    if (!TryGetFlightDestination(felmyst, destination))
        return false;

    return IsNearLandingPosition(destination);
}

namespace
{

// A pass is recognized by watching Felmyst's flight destination.
void AdvanceFelmystFogPassTracker(Unit* felmyst, FogPassState& tracker)
{
    if (IsFelmystLanding(felmyst))
    {
        tracker = FogPassState{};
        return;
    }

    const FogLocation currentLocation = GetCurrentFogLocation(felmyst);
    const FogLane currentLane = GetFogLaneFromLocation(currentLocation);

    const FogLocation destinationLocation = GetDestinationFogLocation(felmyst);
    const FogLane destinationLane = GetFogLaneFromLocation(destinationLocation);

    const FogLocation previousDestinationLocation = tracker.lastDestinationLocation;
    const FogLane previousDestinationLane = GetFogLaneFromLocation(previousDestinationLocation);

    if (IsSweeping(felmyst))
    {
        FogLane sweepLane = destinationLane;
        if (sweepLane == FogLane::None)
            sweepLane = currentLane;

        tracker.armedSweepLane = sweepLane;
    }

    if (destinationLocation != FogLocation::None &&
        destinationLocation != previousDestinationLocation)
    {
        if (tracker.armedSweepLane != FogLane::None &&
            previousDestinationLane == tracker.armedSweepLane &&
            IsFogSideLocation(destinationLocation))
        {
            ++tracker.completedPassCount;
            tracker.lastCompletedLane = tracker.armedSweepLane;
            tracker.armedSweepLane = FogLane::None;

            constexpr uint32 thirdPassWindowMs = 10 * IN_MILLISECONDS;
            if (tracker.completedPassCount >= FOG_PASSES_PER_AIR_PHASE)
                tracker.thirdPassWindowExpireMs = getMSTime() + thirdPassWindowMs;
        }

        tracker.lastDestinationLocation = destinationLocation;
    }
}

void AdvanceFelmystFogTracker(Unit* felmyst, FelmystEncounterState& state)
{
    uint32 const now = getMSTime();
    constexpr uint32 fogWindupGraceMs = 7 * IN_MILLISECONDS;
    constexpr uint32 fogRecoveryGraceMs = 2500;

    FogOfCorruptionState& tracker = state.fogOfCorruption;
    bool const hasTracker = tracker.phase != FogPhase::None;

    const FogLocation currentLocation = GetCurrentFogLocation(felmyst);
    const FogLocation destinationLocation = GetDestinationFogLocation(felmyst);
    const FogLane currentLane = GetFogLaneFromLocation(currentLocation);
    const FogLane destinationLane = GetFogLaneFromLocation(destinationLocation);

    auto const readLane = [&](FogLane lane)
    {
        tracker.lane = lane;
        tracker.phase = FogPhase::Windup;
        tracker.expireMs = now + fogWindupGraceMs;
        state.laneReadThisPhase = true;
    };

    if (currentLane != FogLane::None)
    {
        readLane(currentLane);
        return;
    }

    if (IsSweeping(felmyst))
    {
        if (tracker.lane == FogLane::None)
            return;

        tracker.phase = FogPhase::Sweep;
        tracker.expireMs = now + fogRecoveryGraceMs;
        return;
    }

    // The lane is known as soon as Felmyst leaves the side point for its start. The departure
    // after the last pass is the landing.
    if (destinationLane != FogLane::None &&
        state.fogPass.completedPassCount < FOG_PASSES_PER_AIR_PHASE &&
        felmyst->GetExactDist2d(FOG_LEFT_SIDE) >= FELMYST_LANE_READ_DISTANCE &&
        felmyst->GetExactDist2d(FOG_RIGHT_SIDE) >= FELMYST_LANE_READ_DISTANCE)
    {
        readLane(destinationLane);
        return;
    }

    if (hasTracker && tracker.expireMs > now && tracker.lane != FogLane::None &&
        tracker.phase == FogPhase::Windup &&
        !IsFogSideLocation(currentLocation) && !IsFogSideLocation(destinationLocation))
    {
        return;
    }

    if (hasTracker && tracker.expireMs > now && tracker.lane != FogLane::None &&
        (tracker.phase == FogPhase::Sweep || tracker.phase == FogPhase::Recovery ||
         IsFogSideLocation(currentLocation) || IsFogSideLocation(destinationLocation)))
    {
        tracker.phase = FogPhase::Recovery;
        return;
    }

    tracker = FogOfCorruptionState{};
}

void ClearFelmystAirPhaseState(FelmystEncounterState& state)
{
    state.airPhaseEnd = FelmystAirEnd::None;
    state.demonicVaporCount = 0;
    state.firstDemonicVaporTargetGuid = ObjectGuid::Empty;
    state.laneReadThisPhase = false;
    state.fogOfCorruption = FogOfCorruptionState{};
    state.fogPass = FogPassState{};
}

bool IsPostThirdPassWindowOpen(FogPassState const& tracker)
{
    return tracker.completedPassCount >= FOG_PASSES_PER_AIR_PHASE &&
        tracker.lastCompletedLane != FogLane::None &&
        tracker.thirdPassWindowExpireMs > getMSTime();
}

FelmystEncounterState const* GetFelmystAirborneState(Unit* felmyst)
{
    if (!felmyst || !felmyst->IsFlying())
        return nullptr;

    return FindFelmystState(felmyst->GetInstanceId());
}

bool TryGetFelmystFogOfCorruptionStageState(Unit* felmyst, FogOfCorruptionState& state)
{
    state = FogOfCorruptionState();

    FelmystEncounterState const* encounterState = GetFelmystAirborneState(felmyst);
    if (!encounterState)
        return false;

    FogOfCorruptionState const& tracker = encounterState->fogOfCorruption;
    if (tracker.phase == FogPhase::None || tracker.lane == FogLane::None ||
        tracker.expireMs <= getMSTime())
    {
        return false;
    }

    state = tracker;
    return true;
}

} // end anonymous namespace

// The only function that writes Felmyst's air-phase state, apart from the summon hook's count.
void AdvanceFelmystEncounterState(Unit* felmyst)
{
    if (!felmyst)
        return;

    FelmystEncounterState& state = FelmystState(felmyst->GetInstanceId());
    bool const flying = felmyst->IsFlying();

    // Seen in the air before she was ever seen on the ground is the pull, which starts nothing.
    if (state.lastSeenFlightState == FelmystFlightState::Flying && !flying)
    {
        state.touchdownMs = getMSTime();
        ClearFelmystAirPhaseState(state);
    }
    else if (state.lastSeenFlightState == FelmystFlightState::Grounded && flying)
    {
        state.airPhaseEnd = GetNearestFelmystAirEnd(felmyst);
        state.touchdownMs = 0;
    }

    state.lastSeenFlightState = flying ? FelmystFlightState::Flying : FelmystFlightState::Grounded;

    if (!flying)
        return;

    AdvanceFelmystFogPassTracker(felmyst, state.fogPass);
    AdvanceFelmystFogTracker(felmyst, state);
}

// The end whose G0 is nearer Felmyst.
FelmystAirEnd GetNearestFelmystAirEnd(Unit* felmyst)
{
    if (!felmyst)
        return FelmystAirEnd::None;

    return felmyst->GetExactDist2d(FELMYST_WEST_GROUP_POSITIONS[0]) <
        felmyst->GetExactDist2d(FELMYST_EAST_GROUP_POSITIONS[0]) ?
        FelmystAirEnd::West : FelmystAirEnd::East;
}

Position const& GetFelmystGroupPosition(FelmystAirEnd end, uint8 demonicVaporCount)
{
    auto const& positions = end == FelmystAirEnd::West ?
        FELMYST_WEST_GROUP_POSITIONS : FELMYST_EAST_GROUP_POSITIONS;

    return positions[std::min<size_t>(demonicVaporCount, positions.size() - 1)];
}

std::span<Position const> GetFelmystDemonicVaporRoute(FelmystAirEnd end, uint8 demonicVaporCount)
{
    bool const second = demonicVaporCount >= 2;
    if (end == FelmystAirEnd::West)
        return second ? FELMYST_WEST_SECOND_VAPOR_ROUTE : FELMYST_WEST_FIRST_VAPOR_ROUTE;

    return second ? FELMYST_EAST_SECOND_VAPOR_ROUTE : FELMYST_EAST_FIRST_VAPOR_ROUTE;
}

// In the air, the group position for this air phase's end and vapor count; on the ground (the
// early move), G0 of the end nearer Felmyst.
bool TryGetFelmystAirGroupPosition(Unit* felmyst, Position& position)
{
    if (!felmyst)
        return false;

    if (!felmyst->IsFlying())
    {
        position = GetFelmystGroupPosition(GetNearestFelmystAirEnd(felmyst), 0);
        return true;
    }

    FelmystEncounterState const* state = FindFelmystState(felmyst->GetInstanceId());
    if (!state || state->airPhaseEnd == FelmystAirEnd::None)
        return false;

    position = GetFelmystGroupPosition(state->airPhaseEnd, state->demonicVaporCount);
    return true;
}

// From takeoff to the first lane read, for every bot not chased by a head.
bool IsFelmystAirGroupMoveActive(Player* bot, Unit* felmyst)
{
    if (!felmyst)
        return false;

    FelmystEncounterState const* state = FindFelmystState(felmyst->GetInstanceId());
    return state && state->airPhaseEnd != FelmystAirEnd::None && !state->laneReadThisPhase &&
        !GetFelmystOwnDemonicVapor(bot);
}

// Takeoff is no further away than the time the bot needs to cover its distance to G0 beyond
// FELMYST_EARLY_MOVE_FREE_DISTANCE.
bool IsFelmystEarlyMoveDue(Player* bot, Unit* felmyst)
{
    if (!felmyst || felmyst->IsFlying())
        return false;

    FelmystEncounterState const* state = FindFelmystState(felmyst->GetInstanceId());
    if (!state || !state->touchdownMs)
        return false;

    Position const& groupPosition = GetFelmystGroupPosition(GetNearestFelmystAirEnd(felmyst), 0);
    float const excessDistance =
        bot->GetExactDist2d(groupPosition) - FELMYST_EARLY_MOVE_FREE_DISTANCE;

    if (excessDistance <= 0.0f)
        return false;

    uint32 const leadMs =
        static_cast<uint32>(excessDistance / FELMYST_EARLY_MOVE_SPEED * IN_MILLISECONDS);

    return GetMSTimeDiffToNow(state->touchdownMs) + leadMs >= FELMYST_TAKEOFF_AFTER_TOUCHDOWN_MS;
}

// One step toward the destination, or at 30, 60, or 90 degrees to either side of it. A landing
// point too close to a hazard or inside refusedLane is refused but may be crossed (PatchesOnly
// when that is all that refuses the direct step); one blocked or too close to the crate never is.
FelmystAirStep TryGetFelmystAirStep(
    Player* bot, Position const& destination, std::vector<Creature*> const& hazards,
    float hazardClearance, FogLane refusedLane, float& stepX, float& stepY)
{
    float const botX = bot->GetPositionX();
    float const botY = bot->GetPositionY();
    float const stepDistance = std::min(PATH_STEP_DISTANCE, bot->GetExactDist2d(destination));
    float const directAngle = std::atan2(
        destination.GetPositionY() - botY, destination.GetPositionX() - botX);

    auto const isRefused = [&](float x, float y)
    {
        if (refusedLane != FogLane::None && !IsFelmystClearOfFogLane(x, y, refusedLane))
            return true;

        return IsNearDemonicVaporHazard(x, y, hazards, hazardClearance);
    };

    auto const canStep = [&](float x, float y)
    {
        float stepZ = 0.0f;
        return CanTakeStepTowards(bot, x, y, stepDistance, stepX, stepY, stepZ) &&
            !IsNearFelmystCrate(stepX, stepY);
    };

    constexpr float headingStep = static_cast<float>(M_PI) / 6.0f;
    constexpr std::array headingOffsets = { 0, 1, -1, 2, -2, 3, -3 };
    bool directStepRefused = false;

    for (int const offset : headingOffsets)
    {
        float const angle = directAngle + offset * headingStep;
        float const x = botX + std::cos(angle) * stepDistance;
        float const y = botY + std::sin(angle) * stepDistance;

        if (IsNearFelmystCrate(x, y))
            continue;

        if (isRefused(x, y))
        {
            directStepRefused |= offset == 0;
            continue;
        }

        if (canStep(x, y) && !isRefused(stepX, stepY))
            return FelmystAirStep::Step;
    }

    if (directStepRefused &&
        canStep(botX + std::cos(directAngle) * stepDistance,
            botY + std::sin(directAngle) * stepDistance))
    {
        return FelmystAirStep::PatchesOnly;
    }

    return FelmystAirStep::Blocked;
}

// The nearest point clear of every patch, other player's head, and the crate, in eight directions
// out to FELMYST_SIDESTEP_SEARCH_RADIUS, that the bot can reach in a straight line.
bool TryGetFelmystSidestepPosition(
    Player* bot, std::vector<Creature*> const& hazards, Position& destination)
{
    constexpr uint8 directionCount = 8;
    constexpr float directionStep = 2.0f * static_cast<float>(M_PI) / directionCount;
    constexpr float ringStep = 1.0f;
    constexpr uint8 ringCount = static_cast<uint8>(FELMYST_SIDESTEP_SEARCH_RADIUS / ringStep);

    auto const isClear = [&hazards](float x, float y)
    {
        return !IsNearFelmystCrate(x, y) &&
            !IsNearDemonicVaporHazard(x, y, hazards, DEMONIC_VAPOR_CLEARANCE);
    };

    float const botX = bot->GetPositionX();
    float const botY = bot->GetPositionY();

    for (uint8 ring = 1; ring <= ringCount; ++ring)
    {
        float const distance = ring * ringStep;
        for (uint8 direction = 0; direction < directionCount; ++direction)
        {
            float const angle = direction * directionStep;
            float const x = botX + std::cos(angle) * distance;
            float const y = botY + std::sin(angle) * distance;
            if (!isClear(x, y))
                continue;

            float stepX = 0.0f;
            float stepY = 0.0f;
            float stepZ = 0.0f;
            if (!CanTakeStepTowards(bot, x, y, distance, stepX, stepY, stepZ) ||
                !isClear(stepX, stepY))
            {
                continue;
            }

            destination = Position(stepX, stepY, bot->GetPositionZ());
            return true;
        }
    }

    return false;
}

bool IsFelmystClearOfFogLane(float positionX, float positionY, FogLane lane)
{
    return GetDistancePastFogThreshold(positionX, positionY, lane) >= FOG_CLEAR_PAST_THRESHOLD;
}

// The nearest point FOG_STOP_PAST_THRESHOLD past the lane's threshold line, extended past its ends.
Position GetFelmystFogStopPosition(Player* bot, FogLane lane)
{
    float normalX = 0.0f;
    float normalY = 0.0f;
    if (!TryGetFogThresholdNormal(lane, normalX, normalY))
        return bot->GetPosition();

    float const shortfall = FOG_STOP_PAST_THRESHOLD -
        GetDistancePastFogThreshold(bot->GetPositionX(), bot->GetPositionY(), lane);

    return Position(
        bot->GetPositionX() + normalX * shortfall, bot->GetPositionY() + normalY * shortfall,
        bot->GetPositionZ());
}

bool TryGetFelmystPostThirdPassWindow(Unit* felmyst, FogLane& lane)
{
    lane = FogLane::None;

    FelmystEncounterState const* state = GetFelmystAirborneState(felmyst);
    if (!state || !IsPostThirdPassWindowOpen(state->fogPass))
        return false;

    lane = state->fogPass.lastCompletedLane;
    return true;
}

bool IsFelmystFogActiveForBot(Player* bot, Unit* felmyst)
{
    FelmystEncounterState const* state = GetFelmystAirborneState(felmyst);
    if (!state)
        return false;

    FogOfCorruptionState const& fog = state->fogOfCorruption;
    if (fog.phase == FogPhase::None || fog.phase == FogPhase::Recovery ||
        fog.lane == FogLane::None)
    {
        return false;
    }

    return !IsFelmystClearOfFogLane(bot->GetPositionX(), bot->GetPositionY(), fog.lane);
}

bool IsFelmystFogMovementSuppressed(Unit* felmyst)
{
    FelmystEncounterState const* state = GetFelmystAirborneState(felmyst);
    if (!state)
        return false;

    return state->fogOfCorruption.phase != FogPhase::None ||
        IsPostThirdPassWindowOpen(state->fogPass);
}

bool IsFelmystAirPhaseTargetSuppressed(Unit* felmyst)
{
    if (!felmyst || !felmyst->IsFlying())
        return false;

    // HP threshold to preserve melee targeting during the initial airborne pull
    if (felmyst->GetHealthPct() > BOSS_ENGAGED_HEALTH_PCT)
        return false;

    Position destination;
    if (!TryGetFlightDestination(felmyst, destination))
        return true;

    return !IsNearLandingPosition(destination);
}

bool IsFelmystLandingDpsHoldActive(PlayerbotAI* botAI)
{
    if (PlayerbotAI::IsMainTank(botAI->GetBot()))
        return false;

    Unit* felmyst = GetSwpBoss(botAI->GetBot(), SwpBosses::DATA_FELMYST);
    if (!felmyst)
        return false;

    FelmystEncounterState const* state = FindFelmystState(felmyst->GetInstanceId());
    return state && state->landingDpsWaitStartMs;
}

bool TryGetActiveFogOfCorruptionState(Player* bot, Unit* felmyst, FogOfCorruptionState& state)
{
    if (!TryGetFelmystFogOfCorruptionStageState(felmyst, state))
        return false;

    if (state.phase == FogPhase::Recovery)
        return false;

    return !IsFelmystClearOfFogLane(bot->GetPositionX(), bot->GetPositionY(), state.lane);
}

void RecordFelmystIncomingEncapsulateTarget(Player* target, uint32 durationMs)
{
    if (!target)
        return;

    uint32 const now = getMSTime();
    IncomingEncapsulateState& state = FelmystState(target->GetInstanceId()).incomingEncapsulate;

    if (state.targetGuid != target->GetGUID())
        state.delayMs = now + ENCAPSULATE_DELAY_MS;

    state.targetGuid = target->GetGUID();
    state.expireMs = now + durationMs;
    state.auraObserved = false;
}

Player* GetFelmystEncapsulateTarget(Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    uint32 const now = getMSTime();
    if (FelmystEncounterState* felmystState = FindFelmystState(bot->GetInstanceId()))
    {
        auto& incomingState = felmystState->incomingEncapsulate;
        Player* incomingTarget = nullptr;
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || member->GetGUID() != incomingState.targetGuid)
                continue;

            incomingTarget = member;
            break;
        }

        if (incomingTarget && incomingTarget->HasAura(Id(SwpSpells::SPELL_ENCAPSULATE)))
        {
            incomingState.auraObserved = true;
            felmystState->encapsulateOccurredThisGroundPhase = true;
            return incomingTarget;
        }

        if (!incomingTarget || incomingState.auraObserved || incomingState.expireMs <= now)
            felmystState->incomingEncapsulate = IncomingEncapsulateState{};
        else
            return incomingState.delayMs <= now ? incomingTarget : nullptr;
    }

    Player* closestTarget = nullptr;
    float closestDistance = std::numeric_limits<float>::max();

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || !member->HasAura(Id(SwpSpells::SPELL_ENCAPSULATE)))
            continue;

        FelmystState(bot->GetInstanceId()).encapsulateOccurredThisGroundPhase = true;

        float distance = bot->GetExactDist(member);
        if (distance < closestDistance)
        {
            closestTarget = member;
            closestDistance = distance;
        }
    }

    return closestTarget;
}

bool ShouldMoveAwayFromFelmystEncapsulateTarget(
    Player* bot, Unit* felmyst, Player* encapsulateTarget)
{
    if (!felmyst || !encapsulateTarget || encapsulateTarget == bot || felmyst->GetVictim() == bot)
        return false;

    return bot->GetExactDist2d(encapsulateTarget) < ENCAPSULATE_CLEARANCE;
}

bool DidEncapsulateOccurThisGroundPhase(Player* bot)
{
    FelmystEncounterState const* state = FindFelmystState(bot->GetInstanceId());
    return state && state->encapsulateOccurredThisGroundPhase;
}

Player* GetFelmystGasNovaDispelTarget(Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    Player* closestTarget = nullptr;
    float closestDistance = std::numeric_limits<float>::max();

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->HasAura(Id(SwpSpells::SPELL_GAS_NOVA)))
            continue;

        float distance = bot->GetExactDist(member);
        if (distance < closestDistance)
        {
            closestTarget = member;
            closestDistance = distance;
        }
    }

    return closestTarget;
}

// CanCastSpell() skips the mana check and refuses a cast-time spell while the bot moves, so it
// can't gate a move toward the cast.
bool IsFelmystMassDispelReady(Player* bot)
{
    uint32 const spellId = Id(SwpSpells::SPELL_MASS_DISPEL);
    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
    return spellInfo && bot->HasSpell(spellId) && !bot->HasSpellCooldown(spellId) &&
        !bot->HasUnitFlag(UNIT_FLAG_SILENCED) &&
        static_cast<int32>(bot->GetPower(POWER_MANA)) >=
            spellInfo->CalcPowerCost(bot, spellInfo->GetSchoolMask());
}

Player* GetFelmystCharmedTarget(Player* bot, Unit* felmyst)
{
    if (!felmyst)
        return nullptr;

    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    Player* lowestHpTarget = nullptr;
    uint32 lowestHp = std::numeric_limits<uint32>::max();

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member == bot || !member->IsAlive() || member->GetMapId() != SWP_MAP_ID)
            continue;

        if (!member->HasAura(Id(SwpSpells::SPELL_FOG_OF_CORRUPTION_CHARM)))
            continue;

        if (PlayerbotAI::IsMelee(bot) && !felmyst->IsFlying() && !bot->IsWithinMeleeRange(member))
            continue;

        if (PlayerbotAI::IsRanged(bot) && bot->GetDistance2d(member) > RANGED_ABILITY_REACH)
            continue;

        if (member->GetHealth() < lowestHp)
        {
            lowestHp = member->GetHealth();
            lowestHpTarget = member;
        }
    }

    return lowestHpTarget;
}

// Shared encounter state

FelmystEncounterState& FelmystState(uint32 instanceId)
{
    std::optional<FelmystEncounterState>& state = SwpState(instanceId).felmyst;
    if (!state)
        state.emplace();

    return *state;
}

FelmystEncounterState* FindFelmystState(uint32 instanceId)
{
    SwpInstanceState* state = FindSwpState(instanceId);
    return state && state->felmyst ? &*state->felmyst : nullptr;
}

}
