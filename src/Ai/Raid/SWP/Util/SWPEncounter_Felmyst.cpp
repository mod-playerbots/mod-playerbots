/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "SWPEncounter_Felmyst.h"
#include "Playerbots.h"
#include "RaidBossHelpers.h"
#include <algorithm>
#include <cmath>
#include <list>
#include <vector>

namespace SwpHelpers
{

// Note: Felmyst's CombatReach is 10.0f

std::unordered_map<uint32, FelmystEncounterState> felmystEncounterStates;

namespace
{

std::array<Position, 3> const TANK_POSITIONS =
{{
    { 1460.145f, 598.290f, 21.869f },
    { 1480.587f, 636.805f, 21.713f },
    { 1479.524f, 584.069f, 23.231f },
}};

std::array<Position, 3> const FOG_LEFT_LANES =
{{
    { 1494.745f, 704.000f, 50.085f, 4.747f },
    { 1469.923f, 703.239f, 50.086f, 4.747f },
    { 1446.515f, 701.518f, 50.085f, 4.747f },
}};

std::array<Position, 3> const FOG_RIGHT_LANES =
{{
    { 1492.820f, 515.668f, 50.083f, 1.449f },
    { 1466.732f, 515.595f, 50.572f, 1.449f },
    { 1441.640f, 520.520f, 50.083f, 1.449f },
}};

std::array<std::array<Position, 3>, 3> const FOG_SAFE_SPOTS =
{{
    {{ // Top lane safe spots
        { 1466.877f, 562.297f, 22.231f },
        { 1466.718f, 602.838f, 22.834f },
        { 1466.896f, 641.309f, 20.496f },
    }},
    {{ // Middle lane safe spots
        { 1500.352f, 570.684f, 24.830f },
        { 1500.372f, 602.543f, 26.305f },
        { 1500.275f, 639.854f, 24.744f },
    }},
    {{ // Bottom lane safe spots
        { 1484.481f, 568.884f, 23.328f },
        { 1484.491f, 602.682f, 24.015f },
        { 1484.395f, 635.028f, 22.242f },
    }}
}};

Position const FOG_LEFT_SIDE =  { 1469.064f, 729.585f, 59.824f, 4.677f };
Position const FOG_RIGHT_SIDE = { 1458.556f, 502.200f, 59.900f, 1.606f };

Position const LEFT_LANDING_POSITION =   { 1476.770f, 665.094f, 20.642f };
Position const RIGHT_LANDING_POSITION =  { 1469.930f, 557.009f, 22.632f };
Position const CENTER_GROUND_REFERENCE = { 1473.350f, 611.052f, 21.637f };

struct DemonicVaporAnchor
{
    Position position;
    FogLane lane;
    uint8 sideMask;
};

constexpr uint8 DEMONIC_VAPOR_LEFT_SIDE = 0x1;
constexpr uint8 DEMONIC_VAPOR_RIGHT_SIDE = 0x2;

// Use the fog-lane X bands projected onto each grounded side landing.
std::array<DemonicVaporAnchor, 6> const DEMONIC_VAPOR_KITE_ANCHORS =
{{
    {
        { 1492.820f, RIGHT_LANDING_POSITION.GetPositionY(),
          RIGHT_LANDING_POSITION.GetPositionZ() },
        FogLane::Top, DEMONIC_VAPOR_RIGHT_SIDE,
    },
    {
        { 1494.745f, LEFT_LANDING_POSITION.GetPositionY(),
          LEFT_LANDING_POSITION.GetPositionZ() },
        FogLane::Top, DEMONIC_VAPOR_LEFT_SIDE,
    },
    {
        { 1466.732f, RIGHT_LANDING_POSITION.GetPositionY(),
          RIGHT_LANDING_POSITION.GetPositionZ() },
        FogLane::Middle, DEMONIC_VAPOR_RIGHT_SIDE,
    },
    {
        { 1469.923f, LEFT_LANDING_POSITION.GetPositionY(),
          LEFT_LANDING_POSITION.GetPositionZ() },
        FogLane::Middle, DEMONIC_VAPOR_LEFT_SIDE,
    },
    {
        { 1441.640f, RIGHT_LANDING_POSITION.GetPositionY(),
          RIGHT_LANDING_POSITION.GetPositionZ() },
        FogLane::Bottom, DEMONIC_VAPOR_RIGHT_SIDE,
    },
    {
        { 1446.515f, LEFT_LANDING_POSITION.GetPositionY(),
          LEFT_LANDING_POSITION.GetPositionZ() },
        FogLane::Bottom, DEMONIC_VAPOR_LEFT_SIDE,
    }
}};

std::array<Position, 3> const DEMONIC_VAPOR_LANE_REFERENCES =
{{
    { 1493.783f, 609.834f, 50.084f },
    { 1468.328f, 609.417f, 50.329f },
    { 1444.078f, 611.019f, 50.084f },
}};

void ResetDemonicVaporFlightState(uint32 instanceId)
{
    auto const stateItr = felmystEncounterStates.find(instanceId);
    if (stateItr == felmystEncounterStates.end())
        return;

    stateItr->second.demonicVaporRegionIndices.clear();
    stateItr->second.demonicVaporUsedRegionMask = 0;
    stateItr->second.demonicVaporFirstRegionIndex = 0;
}

void ResetDemonicVaporFlightStateIfGrounded(Player* bot)
{
    constexpr float searchRadius = 250.0f;
    Creature* felmyst = bot->FindNearestCreature(
        static_cast<uint32>(SwpNpcs::NPC_FELMYST), searchRadius, true);

    if (!felmyst || !felmyst->IsFlying())
        ResetDemonicVaporFlightState(bot->GetInstanceId());
}

bool TryGetFelmystGroundStackCenter(
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

bool IsNearFogSafeSpot(Player* bot, FogLane dangerLane, float& closestDistance)
{
    closestDistance = std::numeric_limits<float>::max();
    if (dangerLane == FogLane::None)
        return false;

    uint8 const laneIndex = static_cast<uint8>(dangerLane);
    if (laneIndex >= FOG_SAFE_SPOTS.size())
        return false;

    for (Position const& safeSpot : FOG_SAFE_SPOTS[laneIndex])
    {
        float const distance = bot->GetExactDist2d(
            safeSpot.GetPositionX(), safeSpot.GetPositionY());

        if (distance < closestDistance)
            closestDistance = distance;
    }

    constexpr float safeSpotArrivalDistance = 8.0f;
    return closestDistance <= safeSpotArrivalDistance;
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

bool TryGetFelmystMovementDestination(Unit* felmyst, Position& destination)
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

FogLocation GetFelmystCurrentFogLocation(Unit* felmyst)
{
    if (!felmyst)
        return FogLocation::None;

    return GetFogLocationFromPosition(
        felmyst->GetPositionX(), felmyst->GetPositionY(), FELMYST_FOG_LOCATION_MATCH_DISTANCE);
}

FogLocation GetFelmystDestinationFogLocation(Unit* felmyst)
{
    Position destination;
    if (!TryGetFelmystMovementDestination(felmyst, destination))
        return FogLocation::None;

    return GetFogLocationFromPosition(
        destination.GetPositionX(), destination.GetPositionY(), FELMYST_FOG_LOCATION_MATCH_DISTANCE);
}

bool IsNearFelmystLandingPosition(Position const& destination)
{
    bool const nearRight = destination.GetExactDist2d(
        RIGHT_LANDING_POSITION.GetPositionX(),
        RIGHT_LANDING_POSITION.GetPositionY()) <= FELMYST_FOG_LOCATION_MATCH_DISTANCE;
    bool const nearLeft = destination.GetExactDist2d(
        LEFT_LANDING_POSITION.GetPositionX(),
        LEFT_LANDING_POSITION.GetPositionY()) <= FELMYST_FOG_LOCATION_MATCH_DISTANCE;

    return nearRight || nearLeft;
}

FogLane GetNearestDemonicVaporLane(Player* bot)
{
    FogLane bestLane = FogLane::Middle;
    float bestDistance = std::numeric_limits<float>::max();

    for (uint8 laneIndex = 0; laneIndex < DEMONIC_VAPOR_LANE_REFERENCES.size(); ++laneIndex)
    {
        Position const& reference = DEMONIC_VAPOR_LANE_REFERENCES[laneIndex];
        float const distance = bot->GetExactDist2d(
            reference.GetPositionX(), reference.GetPositionY());
        if (distance < bestDistance)
        {
            bestDistance = distance;
            bestLane = static_cast<FogLane>(laneIndex);
        }
    }

    return bestLane;
}

uint8 GetDemonicVaporAllowedSides(Player* bot)
{
    float const centerDistance = bot->GetExactDist2d(
        CENTER_GROUND_REFERENCE.GetPositionX(), CENTER_GROUND_REFERENCE.GetPositionY());
    float const rightDistance = bot->GetExactDist2d(
        RIGHT_LANDING_POSITION.GetPositionX(), RIGHT_LANDING_POSITION.GetPositionY());
    float const leftDistance = bot->GetExactDist2d(
        LEFT_LANDING_POSITION.GetPositionX(), LEFT_LANDING_POSITION.GetPositionY());

    if (centerDistance <= rightDistance && centerDistance <= leftDistance)
        return DEMONIC_VAPOR_LEFT_SIDE | DEMONIC_VAPOR_RIGHT_SIDE;

    return rightDistance <= leftDistance ?
        DEMONIC_VAPOR_LEFT_SIDE : DEMONIC_VAPOR_RIGHT_SIDE;
}

uint8 GetDemonicVaporAnchorMask(uint8 anchorIndex)
{
    if (anchorIndex >= DEMONIC_VAPOR_KITE_ANCHORS.size())
        return 0;

    return static_cast<uint8>(1u << anchorIndex);
}

uint8 FlipVaporSide(uint8 sideMask)
{
    return sideMask == DEMONIC_VAPOR_LEFT_SIDE ? DEMONIC_VAPOR_RIGHT_SIDE : DEMONIC_VAPOR_LEFT_SIDE;
}

uint8 SelectPreferredDemonicVaporSide(Player* bot, FogLane lane, uint8 allowedSides)
{
    if (allowedSides == DEMONIC_VAPOR_LEFT_SIDE || allowedSides == DEMONIC_VAPOR_RIGHT_SIDE)
        return allowedSides;

    float bestLeftDistance = std::numeric_limits<float>::max();
    float bestRightDistance = std::numeric_limits<float>::max();

    for (uint8 anchorIndex = 0;
         anchorIndex < DEMONIC_VAPOR_KITE_ANCHORS.size();
         ++anchorIndex)
    {
        DemonicVaporAnchor const& anchor =
            DEMONIC_VAPOR_KITE_ANCHORS[anchorIndex];
        if (anchor.lane != lane)
            continue;

        Position const& anchorPos = DEMONIC_VAPOR_KITE_ANCHORS[anchorIndex].position;
        float const distance = bot->GetExactDist2d(
            anchorPos.GetPositionX(), anchorPos.GetPositionY());

        if (anchor.sideMask == DEMONIC_VAPOR_LEFT_SIDE)
            bestLeftDistance = distance;
        else if (anchor.sideMask == DEMONIC_VAPOR_RIGHT_SIDE)
            bestRightDistance = distance;
    }

    return bestLeftDistance <= bestRightDistance ?
        DEMONIC_VAPOR_LEFT_SIDE : DEMONIC_VAPOR_RIGHT_SIDE;
}

std::array<FogLane, 3> GetFelmystDemonicVaporLanePriority(FogLane lane)
{
    switch (lane)
    {
        case FogLane::Top:
            return {{ FogLane::Top, FogLane::Middle, FogLane::Bottom }};
        case FogLane::Bottom:
            return {{ FogLane::Bottom, FogLane::Middle, FogLane::Top }};
        default:
            return {{ FogLane::Middle, FogLane::Top, FogLane::Bottom }};
    }
}

void PushUniqueDemonicVaporAnchor(std::vector<uint8>& anchorIndices, uint8 anchorIndex)
{
    if (std::find(anchorIndices.begin(), anchorIndices.end(), anchorIndex) == anchorIndices.end())
        anchorIndices.push_back(anchorIndex);
}

void AppendDemonicVaporAnchorsForSide(
    std::vector<uint8>& anchorIndices, FogLane preferredLane, uint8 sideMask)
{
    auto const lanePriority = GetFelmystDemonicVaporLanePriority(preferredLane);
    for (FogLane lane : lanePriority)
    {
        for (uint8 anchorIndex = 0; anchorIndex < DEMONIC_VAPOR_KITE_ANCHORS.size();
             ++anchorIndex)
        {
            DemonicVaporAnchor const& anchor = DEMONIC_VAPOR_KITE_ANCHORS[anchorIndex];
            if (anchor.sideMask == sideMask && anchor.lane == lane)
                PushUniqueDemonicVaporAnchor(anchorIndices, anchorIndex);
        }
    }
}

std::vector<Unit*> GetDemonicVaporHazards(Player* bot)
{
    std::vector<Unit*> hazards;
    constexpr float searchRadius = 75.0f;

    auto const addHazards = [&](uint32 entry)
    {
        std::list<Creature*> creatures;
        bot->GetCreatureListWithEntryInGrid(creatures, entry, searchRadius);
        for (Creature* creature : creatures)
        {
            if (!creature || !creature->IsAlive())
                continue;

            if (entry == static_cast<uint32>(SwpNpcs::NPC_DEMONIC_VAPOR) &&
                creature->GetSummonerGUID() == bot->GetGUID())
            {
                continue;
            }

            hazards.push_back(creature);
        }
    };

    addHazards(static_cast<uint32>(SwpNpcs::NPC_DEMONIC_VAPOR));
    addHazards(static_cast<uint32>(SwpNpcs::NPC_DEMONIC_VAPOR_TRAIL));
    return hazards;
}

float GetMinDistanceToOtherPlayers(Player* bot, float x, float y)
{
    float minDistance = std::numeric_limits<float>::max();
    Map::PlayerList const& players = bot->GetMap()->GetPlayers();
    for (Map::PlayerList::const_iterator it = players.begin(); it != players.end(); ++it)
    {
        Player* member = it->GetSource();
        if (!member || member == bot || !member->IsAlive() || member->GetMapId() != SWP_MAP_ID)
            continue;

        float const distance = std::hypot(x - member->GetPositionX(), y - member->GetPositionY());
        if (distance < minDistance)
            minDistance = distance;
    }

    return minDistance;
}

float GetMinDistanceToHazards(float x, float y, std::vector<Unit*> const& hazards)
{
    float minDistance = std::numeric_limits<float>::max();
    for (Unit* hazard : hazards)
    {
        if (!hazard)
            continue;

        float const distance = std::hypot(x - hazard->GetPositionX(), y - hazard->GetPositionY());
        if (distance < minDistance)
            minDistance = distance;
    }

    return minDistance;
}

bool IsDemonicVaporPathSafe(
    Player* bot, Position const& start, Position const& target, std::vector<Unit*> const& hazards)
{
    constexpr float pathStepSize = 2.0f;
    constexpr float playerPathClearance = 7.0f;
    constexpr float hazardPathClearance = 10.0f;
    float const totalDistance = start.GetExactDist2d(target.GetPositionX(), target.GetPositionY());
    if (totalDistance <= 0.0f)
        return true;

    Map::PlayerList const& players = bot->GetMap()->GetPlayers();
    uint32 const stepCount = static_cast<uint32>(totalDistance / pathStepSize) + 1;
    for (uint32 step = 0; step <= stepCount; ++step)
    {
        float const t = std::min(
            static_cast<float>(step * pathStepSize) / totalDistance, 1.0f);
        float const checkX = start.GetPositionX() +
            (target.GetPositionX() - start.GetPositionX()) * t;
        float const checkY = start.GetPositionY() +
            (target.GetPositionY() - start.GetPositionY()) * t;

        for (Map::PlayerList::const_iterator it = players.begin(); it != players.end(); ++it)
        {
            Player* member = it->GetSource();
            if (!member || member == bot || !member->IsAlive() || member->GetMapId() != SWP_MAP_ID)
                continue;

            if (std::hypot(checkX - member->GetPositionX(), checkY - member->GetPositionY()) <
                playerPathClearance)
            {
                return false;
            }
        }

        for (Unit* hazard : hazards)
        {
            if (!hazard)
                continue;

            if (std::hypot(checkX - hazard->GetPositionX(), checkY - hazard->GetPositionY()) <
                hazardPathClearance)
            {
                return false;
            }
        }
    }

    return true;
}

bool TryGetDemonicVaporAnchorDestination(
    Player* bot, uint8 anchorIndex, std::vector<Unit*> const& hazards,
    bool requireSafePath, bool requireSafeEndpoint, Position& destination)
{
    if (anchorIndex >= DEMONIC_VAPOR_KITE_ANCHORS.size())
        return false;

    constexpr float minPlayerEndpointClearance = 8.0f;
    constexpr float minHazardEndpointClearance = 12.0f;

    float destinationX = DEMONIC_VAPOR_KITE_ANCHORS[anchorIndex].position.GetPositionX();
    float destinationY = DEMONIC_VAPOR_KITE_ANCHORS[anchorIndex].position.GetPositionY();
    float destinationZ = DEMONIC_VAPOR_KITE_ANCHORS[anchorIndex].position.GetPositionZ();

    if (!bot->GetMap()->CheckCollisionAndGetValidCoords(
            bot, bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(),
            destinationX, destinationY, destinationZ, true))
    {
        return false;
    }

    float const minPlayerDistance = GetMinDistanceToOtherPlayers(bot, destinationX, destinationY);
    float const minHazardDistance = GetMinDistanceToHazards(destinationX, destinationY, hazards);

    if (requireSafeEndpoint &&
        (minPlayerDistance < minPlayerEndpointClearance ||
         minHazardDistance < minHazardEndpointClearance))
    {
        return false;
    }

    Position const candidate(destinationX, destinationY, destinationZ, bot->GetOrientation());
    Position const origin(
        bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(), bot->GetOrientation());
    if (requireSafePath && !IsDemonicVaporPathSafe(bot, origin, candidate, hazards))
        return false;

    destination = candidate;
    return true;
}

bool TryGetFelmystDemonicVaporStepDestination(
    Player* bot, Position const& anchorDestination, Position& destination)
{
    constexpr float stepDistance = 10.0f;
    float const distanceToAnchor = bot->GetExactDist2d(
        anchorDestination.GetPositionX(), anchorDestination.GetPositionY());
    if (distanceToAnchor <= stepDistance)
    {
        destination = anchorDestination;
        return true;
    }

    float const directionX =
        (anchorDestination.GetPositionX() - bot->GetPositionX()) / distanceToAnchor;
    float const directionY =
        (anchorDestination.GetPositionY() - bot->GetPositionY()) / distanceToAnchor;

    float destinationX = bot->GetPositionX() + directionX * stepDistance;
    float destinationY = bot->GetPositionY() + directionY * stepDistance;
    float destinationZ = bot->GetMapWaterOrGroundLevel(
        destinationX, destinationY, anchorDestination.GetPositionZ());

    if (destinationZ <= INVALID_HEIGHT)
        destinationZ = bot->GetPositionZ();

    if (!bot->GetMap()->CheckCollisionAndGetValidCoords(
            bot, bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(),
            destinationX, destinationY, destinationZ, true))
    {
        return false;
    }

    if (bot->GetExactDist2d(destinationX, destinationY) <= 0.01f)
        return false;

    destination = Position(destinationX, destinationY, destinationZ, bot->GetOrientation());

    return true;
}

} // end anonymous namespace

Position const& GetFelmystMainTankGroundPosition(Player* bot)
{
    Position const* bestPosition = &TANK_POSITIONS[0];
    float bestDistance = std::numeric_limits<float>::max();

    for (Position const& position : TANK_POSITIONS)
    {
        float const distance = bot->GetExactDist2d(
            position.GetPositionX(), position.GetPositionY());
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

    if (!TryGetFelmystGroundStackCenter(bot, felmyst, stack, destinationX, destinationY))
        return false;

    float destinationZ = bot->GetMapWaterOrGroundLevel(
        destinationX, destinationY, bot->GetPositionZ());

    if (destinationZ <= INVALID_HEIGHT)
        destinationZ = bot->GetPositionZ();

    bot->GetMap()->CheckCollisionAndGetValidCoords(
        bot, bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(),
        destinationX, destinationY, destinationZ, false);

    position = Position{ destinationX, destinationY, destinationZ };
    return true;
}

FelmystGroundStack GetClosestFelmystGroundStack(Player* bot, Unit* felmyst, Unit* unit)
{
    if (!unit || !felmyst)
        return FelmystGroundStack::None;

    FelmystGroundStack bestStack = FelmystGroundStack::None;
    float bestDistance = std::numeric_limits<float>::max();
    for (FelmystGroundStack stack : {
             FelmystGroundStack::Melee, FelmystGroundStack::Left, FelmystGroundStack::Right })
    {
        float stackX = 0.0f;
        float stackY = 0.0f;
        if (!TryGetFelmystGroundStackCenter(bot, felmyst, stack, stackX, stackY))
            continue;

        float const stackDistance = unit->GetExactDist2d(stackX, stackY);
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

    Player* mainTank = GetGroupMainTank(GET_PLAYERBOT_AI(bot), bot);
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

void EnsureFelmystRangedAssignments(Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group)
        return;

    auto& assignments = felmystEncounterStates[bot->GetInstanceId()].rangedAssignments;
    std::vector<Player*> healers;
    std::vector<Player*> rangedDamage;

    assignments.clear();

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !PlayerbotAI::IsRanged(member))
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

    for (uint32 index = 0; index < healers.size(); ++index)
    {
        auto const stack = static_cast<FelmystGroundStack>(index % 3);
        assignments[healers[index]->GetGUID()] = static_cast<uint8>(stack);
    }

    for (uint32 index = 0; index < rangedDamage.size(); ++index)
    {
        auto const stack = index % 2 == 0 ? FelmystGroundStack::Left : FelmystGroundStack::Right;
        assignments[rangedDamage[index]->GetGUID()] = static_cast<uint8>(stack);
    }
}

bool TryGetFelmystRangedPosition(Player* bot, Unit* felmyst, Position& position)
{
    if (!felmyst)
        return false;

    EnsureFelmystRangedAssignments(bot);

    auto const instanceItr = felmystEncounterStates.find(bot->GetInstanceId());
    if (instanceItr == felmystEncounterStates.end())
        return false;

    auto const assignmentItr = instanceItr->second.rangedAssignments.find(bot->GetGUID());
    if (assignmentItr == instanceItr->second.rangedAssignments.end())
        return false;

    return TryGetFelmystGroundStackPosition(
        bot, felmyst, static_cast<FelmystGroundStack>(assignmentItr->second), position);
}

Creature* GetFelmystDemonicVaporSummonedByBot(Player* bot)
{
    constexpr float searchRadius = 50.0f;
    std::list<Creature*> vapors;
    bot->GetCreatureListWithEntryInGrid(
        vapors, static_cast<uint32>(SwpNpcs::NPC_DEMONIC_VAPOR), searchRadius);

    for (Creature* creature : vapors)
    {
        if (creature && creature->IsAlive() &&
            creature->GetSummonerGUID() == bot->GetGUID())
        {
            return creature;
        }
    }

    return nullptr;
}

bool IsFelmystDemonicVaporHeadNearBot(Player* bot)
{
    constexpr float kiteDistanceThreshold = 15.0f;
    Creature* vapor = GetFelmystDemonicVaporSummonedByBot(bot);
    return vapor && bot->GetDistance2d(vapor) <= kiteDistanceThreshold;
}

bool TryGetFelmystLandingDestination(Unit* felmyst, Position& destination)
{
    if (!TryGetFelmystMovementDestination(felmyst, destination))
        return false;

    return IsNearFelmystLandingPosition(destination);
}

bool TryGetFelmystPostThirdPassWindow(Unit* felmyst, FogLane& lane)
{
    lane = FogLane::None;

    if (!felmyst)
        return false;

    uint32 const instanceId = felmyst->GetInstanceId();
    if (!felmyst->IsFlying())
    {
        felmystEncounterStates[instanceId].fogPass = FogPassState{};
        return false;
    }

    Position landingDestination;
    if (TryGetFelmystLandingDestination(felmyst, landingDestination))
    {
        felmystEncounterStates[instanceId].fogPass = FogPassState{};
        return false;
    }

    FogPassState& tracker = felmystEncounterStates[instanceId].fogPass;
    uint32 const now = getMSTime();
    constexpr uint32 thirdPassWindowMs = 10000;

    const FogLocation currentLocation = GetFelmystCurrentFogLocation(felmyst);
    const FogLane currentLane = GetFogLaneFromLocation(currentLocation);

    const FogLocation destinationLocation = GetFelmystDestinationFogLocation(felmyst);
    const FogLane destinationLane = GetFogLaneFromLocation(destinationLocation);

    const FogLocation previousDestinationLocation = tracker.lastDestinationLocation;
    const FogLane previousDestinationLane = GetFogLaneFromLocation(previousDestinationLocation);

    bool const isSweeping = felmyst->HasAura(
        static_cast<uint32>(SwpSpells::SPELL_FELMYST_SPEED_BURST));

    if (isSweeping)
    {
        const FogLane sweepLane =
            destinationLane != FogLane::None ? destinationLane : currentLane;
        if (sweepLane != FogLane::None)
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
            if (tracker.completedPassCount >= 3)
                tracker.thirdPassWindowExpireMs = now + thirdPassWindowMs;
        }

        tracker.lastDestinationLocation = destinationLocation;
    }

    if (tracker.completedPassCount >= 3 && tracker.thirdPassWindowExpireMs > now &&
        tracker.lastCompletedLane != FogLane::None)
    {
        lane = tracker.lastCompletedLane;
        return true;
    }

    return false;
}

bool IsFelmystAirPhaseTargetSuppressed(Unit* felmyst)
{
    if (!felmyst || !felmyst->IsFlying())
        return false;

    // The HP threshold is to preserve melee targeting during the initial airborne pull
    if (felmyst->GetHealthPct() > 90.0f)
        return false;

    Position destination;
    if (!TryGetFelmystMovementDestination(felmyst, destination))
        return true;

    return !IsNearFelmystLandingPosition(destination);
}

void ClearFelmystDemonicVaporKiteState(Player* bot)
{
    uint32 const instanceId = bot->GetInstanceId();
    ObjectGuid const guid = bot->GetGUID();

    auto const stateItr = felmystEncounterStates.find(instanceId);
    if (stateItr != felmystEncounterStates.end())
        stateItr->second.demonicVaporRegionIndices.erase(guid);

    ResetDemonicVaporFlightStateIfGrounded(bot);
}

bool TryGetFelmystDemonicVaporKiteDestination(Player* bot, Position& destination)
{
    uint32 const instanceId = bot->GetInstanceId();
    ObjectGuid const guid = bot->GetGUID();

    ResetDemonicVaporFlightStateIfGrounded(bot);

    if (!IsFelmystDemonicVaporHeadNearBot(bot))
    {
        ClearFelmystDemonicVaporKiteState(bot);
        return false;
    }

    auto& regionIndices = felmystEncounterStates[instanceId].demonicVaporRegionIndices;
    uint8& usedRegionMask = felmystEncounterStates[instanceId].demonicVaporUsedRegionMask;
    auto const regionItr = regionIndices.find(guid);
    std::vector<uint8> preferredAnchors;

    if (regionItr != regionIndices.end() &&
        regionItr->second < DEMONIC_VAPOR_KITE_ANCHORS.size())
    {
        PushUniqueDemonicVaporAnchor(preferredAnchors, regionItr->second);
        AppendDemonicVaporAnchorsForSide(
            preferredAnchors,
            DEMONIC_VAPOR_KITE_ANCHORS[regionItr->second].lane,
            DEMONIC_VAPOR_KITE_ANCHORS[regionItr->second].sideMask);
    }
    else
    {
        const FogLane preferredLane = GetNearestDemonicVaporLane(bot);
        uint8 const allowedSides = GetDemonicVaporAllowedSides(bot);
        uint8 preferredSide = SelectPreferredDemonicVaporSide(
            bot, preferredLane, allowedSides);

        auto const firstRegionStateItr = felmystEncounterStates.find(instanceId);
        if (allowedSides == (DEMONIC_VAPOR_LEFT_SIDE | DEMONIC_VAPOR_RIGHT_SIDE) &&
            firstRegionStateItr != felmystEncounterStates.end() &&
            firstRegionStateItr->second.demonicVaporFirstRegionIndex <
            DEMONIC_VAPOR_KITE_ANCHORS.size())
        {
            uint8 const firstAnchorIndex =
                firstRegionStateItr->second.demonicVaporFirstRegionIndex;
            preferredSide = FlipVaporSide(
                DEMONIC_VAPOR_KITE_ANCHORS[firstAnchorIndex].sideMask);
        }

        AppendDemonicVaporAnchorsForSide(preferredAnchors, preferredLane, preferredSide);

        uint8 const alternateSide = FlipVaporSide(preferredSide);
        if (allowedSides & alternateSide)
            AppendDemonicVaporAnchorsForSide(preferredAnchors, preferredLane, alternateSide);
    }

    std::vector<Unit*> const hazards = GetDemonicVaporHazards(bot);
    auto const tryAnchors = [&](bool requireSafePath, bool requireSafeEndpoint)
    {
        for (uint8 anchorIndex : preferredAnchors)
        {
            if (regionItr == regionIndices.end() &&
                (usedRegionMask & GetDemonicVaporAnchorMask(anchorIndex)) != 0)
            {
                continue;
            }

            Position anchorDestination;
            if (!TryGetDemonicVaporAnchorDestination(
                    bot, anchorIndex, hazards, requireSafePath,
                    requireSafeEndpoint, anchorDestination))
            {
                continue;
            }

            if (!TryGetFelmystDemonicVaporStepDestination(bot, anchorDestination, destination))
                continue;

            regionIndices[guid] = anchorIndex;
            usedRegionMask |= GetDemonicVaporAnchorMask(anchorIndex);
            felmystEncounterStates[instanceId].demonicVaporFirstRegionIndex = anchorIndex;
            return true;
        }

        return false;
    };

    return tryAnchors(true, true) || tryAnchors(false, true) || tryAnchors(false, false);
}

bool TryGetFelmystFogOfCorruptionStageState(Unit* felmyst, FogOfCorruptionState& state)
{
    state = FogOfCorruptionState();
    uint32 const now = getMSTime();
    constexpr uint32 fogRecoveryGraceMs = 2500;

    if (!felmyst)
        return false;

    uint32 const instanceId = felmyst->GetInstanceId();
    if (!felmyst->IsFlying())
    {
        ResetDemonicVaporFlightState(instanceId);
        felmystEncounterStates[instanceId].fogPass = FogPassState{};
        felmystEncounterStates[instanceId].fogOfCorruption = FogOfCorruptionState{};
        return false;
    }

    FogLane ignoredPostThirdPassLane = FogLane::None;
    TryGetFelmystPostThirdPassWindow(felmyst, ignoredPostThirdPassLane);

    FogOfCorruptionState& tracker = felmystEncounterStates[instanceId].fogOfCorruption;
    bool const hasTracker = tracker.phase != FogPhase::None;

    const FogLocation currentLocation = GetFelmystCurrentFogLocation(felmyst);
    const FogLocation destinationLocation = GetFelmystDestinationFogLocation(felmyst);
    const FogLane currentLane = GetFogLaneFromLocation(currentLocation);
    const FogLane destinationLane = GetFogLaneFromLocation(destinationLocation);

    bool const isSweeping = felmyst->HasAura(
        static_cast<uint32>(SwpSpells::SPELL_FELMYST_SPEED_BURST));

    if (currentLane != FogLane::None)
    {
        constexpr uint32 fogWindupGraceMs = 7000;
        tracker.lane = currentLane;
        tracker.phase = FogPhase::Windup;
        tracker.expireMs = now + fogWindupGraceMs;
        state = tracker;
        return true;
    }

    if (isSweeping)
    {
        if (tracker.lane == FogLane::None)
            return false;

        tracker.phase = FogPhase::Sweep;
        tracker.expireMs = now + fogRecoveryGraceMs;
        state = tracker;
        return true;
    }

    if (hasTracker && tracker.expireMs > now && tracker.lane != FogLane::None &&
        tracker.phase == FogPhase::Windup &&
        !IsFogSideLocation(currentLocation) && !IsFogSideLocation(destinationLocation))
    {
        state = tracker;
        return true;
    }

    if (hasTracker && tracker.expireMs > now && tracker.lane != FogLane::None &&
        (tracker.phase == FogPhase::Sweep ||
         tracker.phase == FogPhase::Recovery ||
         IsFogSideLocation(currentLocation) || IsFogSideLocation(destinationLocation)))
    {
        tracker.phase = FogPhase::Recovery;
        state = tracker;
        return true;
    }

    felmystEncounterStates[instanceId].fogOfCorruption = FogOfCorruptionState{};
    return false;
}

bool TryGetActiveFogOfCorruptionState(
    Player* bot, Unit* felmyst, FogOfCorruptionState& state)
{
    if (!TryGetFelmystFogOfCorruptionStageState(felmyst, state))
        return false;

    if (state.phase == FogPhase::Recovery)
        return false;

    float safeSpotDistance = std::numeric_limits<float>::max();
    if (IsNearFogSafeSpot(bot, state.lane, safeSpotDistance))
        return false;

    return state.lane != FogLane::None;
}

bool TryGetFelmystFogSafeDestinations(
    Player* bot, FogLane dangerLane, std::array<Position, 3>& destinations,
    uint8& destinationCount)
{
    destinationCount = 0;
    if (dangerLane == FogLane::None)
        return false;

    uint8 const laneIndex = static_cast<uint8>(dangerLane);
    if (laneIndex >= FOG_SAFE_SPOTS.size())
        return false;

    auto const& safeSpots = FOG_SAFE_SPOTS[laneIndex];
    std::array<uint8, 3> candidateOrder = { 0, 1, 2 };
    std::list<Creature*> vaporHazards;
    auto const addVaporHazards = [&](uint32 entry)
    {
        constexpr float searchRadius = 150.0f;
        std::list<Creature*> creatures;
        bot->GetCreatureListWithEntryInGrid(creatures, entry, searchRadius);
        for (Creature* creature : creatures)
        {
            if (creature && creature->IsAlive())
                vaporHazards.push_back(creature);
        }
    };

    addVaporHazards(static_cast<uint32>(SwpNpcs::NPC_DEMONIC_VAPOR));
    addVaporHazards(static_cast<uint32>(SwpNpcs::NPC_DEMONIC_VAPOR_TRAIL));

    auto const isSafeSpotBlockedByVapor = [&](Position const& safeSpot)
    {
        constexpr float safeDistanceFromVapor = 10.0f;
        for (Creature* hazard : vaporHazards)
        {
            if (!hazard)
                continue;

            if (hazard->GetExactDist2d(
                    safeSpot.GetPositionX(), safeSpot.GetPositionY()) < safeDistanceFromVapor)
            {
                return true;
            }
        }

        return false;
    };

    std::sort(candidateOrder.begin(), candidateOrder.end(),
        [&](uint8 leftIndex, uint8 rightIndex)
        {
            Position const& left = safeSpots[leftIndex];
            Position const& right = safeSpots[rightIndex];
            return bot->GetExactDist2d(left.GetPositionX(), left.GetPositionY()) <
                   bot->GetExactDist2d(right.GetPositionX(), right.GetPositionY());
        });

    for (uint8 candidateIndex : candidateOrder)
    {
        Position const& safeSpot = safeSpots[candidateIndex];
        if (isSafeSpotBlockedByVapor(safeSpot))
            continue;

        float destinationX = safeSpot.GetPositionX();
        float destinationY = safeSpot.GetPositionY();
        float destinationZ = safeSpot.GetPositionZ();
        if (!bot->GetMap()->CheckCollisionAndGetValidCoords(
                bot, bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(),
                destinationX, destinationY, destinationZ, false))
        {
            continue;
        }

        destinations[destinationCount++] = Position{ destinationX, destinationY, destinationZ };
    }

    return destinationCount > 0;
}

void RecordFelmystIncomingEncapsulateTarget(Player* target, uint32 durationMs)
{
    if (!target)
        return;

    uint32 const now = getMSTime();
    IncomingEncapsulateState& state =
        felmystEncounterStates[target->GetInstanceId()].incomingEncapsulate;

    constexpr uint32 encapsulateDelayMs = 500;
    if (state.targetGuid != target->GetGUID())
        state.delayMs = now + encapsulateDelayMs;

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
    auto const incomingItr = felmystEncounterStates.find(bot->GetInstanceId());
    if (incomingItr != felmystEncounterStates.end())
    {
        auto& incomingState = incomingItr->second.incomingEncapsulate;
        Player* incomingTarget = nullptr;
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || member->GetGUID() != incomingState.targetGuid)
                continue;

            incomingTarget = member;
            break;
        }

        if (incomingTarget &&
            incomingTarget->HasAura(static_cast<uint32>(SwpSpells::SPELL_ENCAPSULATE)))
        {
            incomingState.auraObserved = true;
            felmystEncounterStates[bot->GetInstanceId()].encapsulateOccurredThisGroundPhase = true;
            return incomingTarget;
        }

        if (!incomingTarget || incomingState.auraObserved || incomingState.expireMs <= now)
            incomingItr->second.incomingEncapsulate = IncomingEncapsulateState{};
        else
            return incomingState.delayMs <= now ? incomingTarget : nullptr;
    }

    Player* closestTarget = nullptr;
    float closestDistance = 0.0f;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() ||
            !member->HasAura(static_cast<uint32>(SwpSpells::SPELL_ENCAPSULATE)))
        {
            continue;
        }

        felmystEncounterStates[bot->GetInstanceId()].encapsulateOccurredThisGroundPhase = true;

        float distance = bot->GetDistance2d(member);
        if (!closestTarget || distance < closestDistance)
        {
            closestTarget = member;
            closestDistance = distance;
        }
    }

    return closestTarget;
}

bool DidEncapsulateOccurThisGroundPhase(Player* bot)
{
    auto const stateItr = felmystEncounterStates.find(bot->GetInstanceId());
    return stateItr != felmystEncounterStates.end() &&
        stateItr->second.encapsulateOccurredThisGroundPhase;
}

Player* GetFelmystGasNovaDispelTarget(Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    Player* closestTarget = nullptr;
    float closestDistance = 0.0f;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->HasAura(static_cast<uint32>(SwpSpells::SPELL_GAS_NOVA)))
            continue;

        float distance = bot->GetDistance(member);
        if (!closestTarget || distance < closestDistance)
        {
            closestTarget = member;
            closestDistance = distance;
        }
    }

    return closestTarget;
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

        if (!member->HasAura(static_cast<uint32>(SwpSpells::SPELL_FOG_OF_CORRUPTION_CHARM)))
            continue;

        if (PlayerbotAI::IsMelee(bot) && !felmyst->IsFlying() && !bot->IsWithinMeleeRange(member))
            continue;

        if (!PlayerbotAI::IsMelee(bot) && bot->GetDistance2d(member) > 30.0f)
            continue;

        if (member->GetHealth() < lowestHp)
        {
            lowestHp = member->GetHealth();
            lowestHpTarget = member;
        }
    }

    return lowestHpTarget;
}

}
