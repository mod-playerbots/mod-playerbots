/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "AirborneController.h"

#include <algorithm>
#include <cmath>

#include "AiObjectContext.h"
#include "DetourNavMeshQuery.h"
#include "GameTime.h"
#include "LastMovementValue.h"
#include "Map.h"
#include "MotionMaster.h"
#include "MoveSpline.h"
#include "MoveSplineInit.h"
#include "PathGenerator.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "PlayerbotAIConfig.h"
#include "Timer.h"
#include "World.h"

using namespace JumpTrajectory;

namespace
{
constexpr float DELIBERATE_DROP_CAP = 6.0f;  // well under the fall damage threshold
constexpr float NO_DROP_CAP = 100000.0f;     // falls with no depth limit; the sweep's step cap bounds how far it looks
constexpr uint32 DROP_DOWN_RETRY_MS = 2000;  // while the bot waits at an edge its target dropped from
constexpr float DROP_DOWN_WALK = 10.0f;      // how far the bot walks looking for the edge
constexpr float LEDGE_HEIGHT = 2.0f;         // a target less far below is down a step the paths take
constexpr float SURVIVAL_MARGIN = 0.05f;     // share of max health a drop down leaves the bot
// Player::HandleFall: the damage share of max health is FALL_DMG_SLOPE * height + FALL_DMG_INTERCEPT.
constexpr float FALL_DMG_SLOPE = 0.018f;
constexpr float FALL_DMG_INTERCEPT = -0.2426f;
constexpr float WALL_BACKOFF = 0.3f;
constexpr float FACING_TOLERANCE = 0.2f;  // a smaller turn is not worth relaunching the arc and broadcasting it
constexpr float ARRIVAL_TOLERANCE = 0.5f;
constexpr float GROUND_CONTACT = 1.0f;
constexpr float MIN_SPLINE_VELOCITY = 0.01f;  // MoveSplineInitArgs::Validate refuses anything slower

float Normalized(float height) { return height > INVALID_HEIGHT ? height : NO_HEIGHT; }

class MapProbe : public CollisionProbe
{
public:
    MapProbe(Map* map, uint32 phaseMask, float collisionHeight)
        : _map(map), _phaseMask(phaseMask), _collisionHeight(collisionHeight)
    {
    }

    bool RayHit(Point const& from, Point const& to, Point& hit) const override
    {
        MapCollisionData const& collision = _map->GetMapCollisionData();
        Point nearest = to;
        bool found = false;
        float x, y, z;
        if (collision.GetStaticTree().GetObjectHitPos(from.x, from.y, from.z, nearest.x, nearest.y, nearest.z, x, y, z,
                                                      -WALL_BACKOFF))
        {
            nearest = {x, y, z};
            found = true;
        }

        if (collision.GetDynamicTree().GetObjectHitPos(_phaseMask, from.x, from.y, from.z, nearest.x, nearest.y,
                                                       nearest.z, x, y, z, -WALL_BACKOFF))
        {
            nearest = {x, y, z};
            found = true;
        }

        if (found)
            hit = nearest;

        return found;
    }

    float TerrainZ(float x, float y) const override { return Normalized(_map->GetGridHeight(x, y)); }

    float FloorZ(float x, float y, float zFrom, float maxSearch) const override
    {
        return Normalized(_map->GetHeight(_phaseMask, x, y, zFrom, true, maxSearch));
    }

    float LiquidZ(float x, float y, float z) const override
    {
        LiquidData const liquid = _map->GetLiquidData(_phaseMask, x, y, z, _collisionHeight, {});
        return (liquid.Status & MAP_LIQUID_STATUS_SWIMMING) ? liquid.Level : NO_HEIGHT;
    }

private:
    Map* _map;
    uint32 _phaseMask;
    float _collisionHeight;
};
}  // namespace

bool AirborneController::CanLeaveGround() const
{
    return _bot && _bot->IsInWorld() && _bot->IsAlive() && !IsSelfBot(_bot) && _bot->m_mover == _bot &&
           !_bot->IsBeingTeleported() && !_bot->GetVehicle() && !_bot->GetTransport() &&
           !_bot->HasUnitMovementFlag(MOVEMENTFLAG_ONTRANSPORT | MOVEMENTFLAG_SWIMMING) && !_bot->IsInFlight() &&
           !_bot->IsFlying() && !_bot->IsCharmed();
}

bool AirborneController::CanJump() const { return CanLeaveGround() && _botAI->CanMove(); }

bool AirborneController::IsSlowFalling() const
{
    // The core sets the movement flags only on units the server moves; for a player it asks the client, which a bot
    // lacks. The auras are what Player::HandleFall checks.
    return _bot->HasFeatherFallAura() || _bot->HasHoverAura();
}

bool AirborneController::IsFallHarmless() const
{
    return !sPlayerbotAIConfig.FallDamage || sWorld->getRate(RATE_DAMAGE_FALL) <= 0.0f || IsSlowFalling();
}

float AirborneController::SurvivableDrop() const
{
    if (IsFallHarmless())
        return NO_DROP_CAP;

    float const rate = sWorld->getRate(RATE_DAMAGE_FALL);

    // Without health to spare it still takes the drops that do no damage.
    float const damageShare = std::max(0.0f, (_bot->GetHealthPct() / 100.0f - SURVIVAL_MARGIN) / rate);
    float const safeFall = static_cast<float>(_bot->GetTotalAuraModifier(SPELL_AURA_SAFE_FALL));
    return (damageShare - FALL_DMG_INTERCEPT) / FALL_DMG_SLOPE + safeFall;
}

Point AirborneController::BotPoint() const
{
    return {_bot->GetPositionX(), _bot->GetPositionY(), _bot->GetPositionZ()};
}

Trajectory AirborneController::SweepFrom(Point const& position, Point const& velocity, float minZ, float maxWalk) const
{
    Launch launch;
    launch.position = position;
    launch.velocity = velocity;
    launch.terminalVelocity = IsSlowFalling() ? SAFE_FALL_TERMINAL_VELOCITY : TERMINAL_VELOCITY;
    launch.collisionHeight = _bot->GetCollisionHeight();
    launch.landingOffset = _bot->GetHoverHeight();
    launch.minZ = minZ;
    MapProbe const probe(_bot->GetMap(), _bot->GetPhaseMask(), launch.collisionHeight);
    return maxWalk > 0.0f ? WalkOff(launch, probe, maxWalk) : Sweep(launch, probe);
}

bool AirborneController::IsOnNavMesh(Point const& point) const
{
    dtNavMeshQuery const* query = _bot->GetMap()->GetMapCollisionData().GetMMapData().GetNavMeshQuery();
    if (!query)
        return true;  // no nav mesh on this map to check against

    dtQueryFilter const filter;
    float const center[3] = {point.y, point.z, point.x};  // Detour axis order
    float const extents[3] = {1.0f, 2.0f, 1.0f};
    float nearest[3];
    dtPolyRef polyRef = 0;
    return dtStatusSucceed(query->findNearestPoly(center, extents, &filter, &polyRef, nearest)) && polyRef;
}

bool AirborneController::IsSafeLanding(Trajectory const& trajectory) const
{
    return trajectory.Landed() && (trajectory.outcome == Outcome::Liquid || IsOnNavMesh(trajectory.landing));
}

bool AirborneController::Jump(float heading, float speedXY)
{
    if (!CanJump())
        return false;

    Point const takeoff = BotPoint();
    float const minZ = takeoff.z - (IsFallHarmless() ? NO_DROP_CAP : DELIBERATE_DROP_CAP);
    Point const velocity{std::cos(heading) * speedXY, std::sin(heading) * speedXY, JUMP_VELOCITY};
    Trajectory trajectory = SweepFrom(takeoff, velocity, minZ);
    if (!IsSafeLanding(trajectory))
        return false;

    Begin(std::move(trajectory), takeoff.z);
    return true;
}

bool AirborneController::DropDownTo(Unit const* target)
{
    uint32 const now = getMSTime();
    if (!target || !target->IsInWorld() || target->GetMapId() != _bot->GetMapId() || !_bot->movespline->Finalized() ||
        target->GetPositionZ() > _bot->GetPositionZ() - LEDGE_HEIGHT ||
        getMSTimeDiff(_lastDropDownMs, now) < DROP_DOWN_RETRY_MS || !CanJump())
        return false;

    _lastDropDownMs = now;

    // A path reaches the target; the bot walks it.
    PathGenerator path(_bot);
    path.CalculatePath(target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), false);
    if (path.GetPathType() & PATHFIND_NORMAL)
        return false;

    Point const start = BotPoint();
    float const heading = _bot->GetAngle(target);
    float const speedXY = _bot->GetSpeed(MOVE_RUN);
    Point const velocity{std::cos(heading) * speedXY, std::sin(heading) * speedXY, 0.0f};
    float const maxDrop = SurvivableDrop();
    Trajectory trajectory = SweepFrom(start, velocity, start.z - maxDrop, DROP_DOWN_WALK);
    Point const& landing = trajectory.landing;
    // Fall damage counts from the edge, which the walk may climb to, and a sweep step can land past its depth limit.
    if (!IsSafeLanding(trajectory) ||
        (trajectory.outcome != Outcome::Liquid && trajectory.takeoff.z - landing.z > maxDrop) ||
        target->GetExactDist(landing.x, landing.y, landing.z) >= target->GetExactDist(start.x, start.y, start.z))
        return false;

    // The splines keep the facing they start with.
    _bot->SetOrientation(heading);
    float const takeoffZ = trajectory.takeoff.z;
    Begin(std::move(trajectory), takeoffZ);
    return true;
}

void AirborneController::Knockback(float vcos, float vsin, float speedXY, float speedZ)
{
    // A self bot's client moves it.
    if (IsSelfBot(_bot))
        return;

    _bot->CastStop();
    if (!CanLeaveGround())
    {
        // Vehicles, transports and water keep the core's knockback; the sweep works in world coordinates on land.
        _bot->StopMoving();
        _bot->GetMotionMaster()->Clear();
        _bot->GetMotionMaster()->MoveKnockbackFrom(_bot->GetPositionX() - vcos, _bot->GetPositionY() - vsin, speedXY,
                                                   speedZ, true);
        return;
    }

    bool const wasAirborne = _airborne;
    if (wasAirborne && !Interrupt(_bot->GetOrientation()))
        _bot->StopMoving();

    Point const start = BotPoint();
    Trajectory trajectory = SweepFrom(start, {vcos * speedXY, vsin * speedXY, speedZ}, start.z - NO_DROP_CAP);
    if (sPlayerbotAIConfig.KnockbackEdgeProtection &&
        (!IsSafeLanding(trajectory) || trajectory.landing.z < start.z - DELIBERATE_DROP_CAP))
        trajectory = ProtectedKnockback(vcos, vsin, speedXY, speedZ);

    if (!trajectory.Landed())
    {
        if (wasAirborne)
            Settle();

        return;
    }

    Begin(std::move(trajectory), wasAirborne ? std::nullopt : std::optional<float>(start.z));
}

Trajectory AirborneController::ProtectedKnockback(float vcos, float vsin, float speedXY, float speedZ) const
{
    // The core's knockback: a nav-mesh raycast stops at the edge of the walkable area.
    Position destination = _bot->GetPosition();
    float const flightTime = speedZ > 0.0f ? 2.0f * speedZ / GRAVITY : 0.0f;
    _bot->MovePositionToFirstCollision(
        destination, speedXY * flightTime,
        _bot->GetRelativeAngle(destination.GetPositionX() + vcos, destination.GetPositionY() + vsin));

    Trajectory trajectory;
    trajectory.outcome = Outcome::Landed;
    trajectory.landing = {destination.GetPositionX(), destination.GetPositionY(), destination.GetPositionZ()};
    uint32 const durationMs = static_cast<uint32>(flightTime * 1000.0f);
    if (!durationMs)
        return trajectory;

    Segment segment;
    segment.start = BotPoint();
    segment.end = trajectory.landing;
    segment.durationMs = durationMs;
    segment.startVz = VerticalSpeedAt(segment, 0);
    trajectory.segments.push_back(segment);
    return trajectory;
}

void AirborneController::StartFall()
{
    if (_airborne || !_bot || !_bot->IsInWorld() || IsSelfBot(_bot) || _bot->HasFlyAura() ||
        _bot->HasIncreaseMountedFlightSpeedAura())
        return;

    // A forced dismount leaves the flight flags behind, and a flying spline ignores gravity.
    _bot->RemoveUnitMovementFlag(MOVEMENTFLAG_FLYING | MOVEMENTFLAG_DISABLE_GRAVITY | MOVEMENTFLAG_CAN_FLY);

    // Every dismount and every stop ends up here, so the floor cached at the last relocation settles the usual case
    // and a probe the rest: on the ground there is nothing to fall, and a moving bot's spline Z can sit a little off
    // the floor; leave its movement alone.
    Point const start = BotPoint();
    bool onFloor = start.z - _bot->GetFloorZ() < GROUND_CONTACT;
    if (!onFloor)
    {
        float const halfHeight = _bot->GetCollisionHeight() * 0.5f;
        float const floor = _bot->GetMap()->GetHeight(_bot->GetPhaseMask(), start.x, start.y, start.z + halfHeight,
                                                      true, halfHeight + GROUND_CONTACT);
        onFloor = floor > INVALID_HEIGHT && start.z - floor < GROUND_CONTACT;
    }

    if (onFloor)
    {
        _bot->RemoveUnitMovementFlag(MOVEMENTFLAG_FALLING | MOVEMENTFLAG_FALLING_FAR);
        return;
    }

    if (!CanLeaveGround())
        return;

    Trajectory trajectory = SweepFrom(start, {}, start.z - NO_DROP_CAP);
    if (trajectory.Landed())
        Begin(std::move(trajectory), start.z);
}

bool AirborneController::Face(float orientation)
{
    if (!_airborne)
        return false;

    float const turn = Position::NormalizeOrientation(orientation - _bot->GetOrientation());
    if (turn < FACING_TOLERANCE || turn > 2.0f * static_cast<float>(M_PI) - FACING_TOLERANCE)
        return true;

    if (!IsOwnSplineRunning())
    {
        _bot->SetOrientation(orientation);
        return true;
    }

    // The spline keeps the orientation it launched with, so turning means relaunching the rest of the segment.
    if (Interrupt(orientation))
        LaunchSegment();

    return true;
}

bool AirborneController::DeferRoot(bool root)
{
    if (!_airborne || !root)
    {
        _rootPending = false;
        return false;
    }

    _rootPending = true;
    return true;
}

void AirborneController::Update()
{
    if (_fallPending)
    {
        _fallPending = false;
        StartFall();
    }

    // A client falls as soon as it stands over nothing, such as where a path to a target below runs off a ledge.
    bool const moving = !_bot->movespline->Finalized();
    if (_wasMoving && !moving && !_airborne && !_bot->IsInWater() && CanLeaveGround())
        StartFall();

    _wasMoving = moving;

    if (!_airborne)
        return;

    if (!_bot->IsInWorld() || _bot->IsBeingTeleported() || _bot->GetVehicle() || _bot->GetTransport() ||
        _bot->IsInFlight())
    {
        Clear();
        return;
    }

    if (IsCoreControlled())
        return;

    // A falling client cannot steer, so a move the bot's AI started mid-air is taken back.
    if (IsSlowFalling() != _slowFall || (!_bot->movespline->Finalized() && !IsOwnSplineRunning()))
    {
        Resweep();
        return;
    }

    if (!_bot->movespline->Finalized())
        return;

    // Off the path, such as when death stops the spline mid-arc; the body carries on as a client's does.
    Segment const& segment = _segments[_segmentIndex];
    if (_bot->GetExactDist(segment.end.x, segment.end.y, segment.end.z) > ARRIVAL_TOLERANCE)
    {
        Resweep();
        return;
    }

    if (++_segmentIndex < _segments.size())
        LaunchSegment();
    else
        Land();
}

void AirborneController::Clear()
{
    _fallPending = false;
    if (_airborne)
        Settle();
}

void AirborneController::Begin(Trajectory&& trajectory, std::optional<float> takeoffZ)
{
    _bot->GetMotionMaster()->Clear();
    if (takeoffZ)
    {
        _airborne = true;
        _bot->AddUnitMovementFlag(MOVEMENTFLAG_FALLING);
        _bot->m_movementInfo.SetFallTime(0);
        _bot->SetFallInformation(GameTime::GetGameTime().count(), *takeoffZ);
    }

    Start(std::move(trajectory));
}

void AirborneController::Start(Trajectory&& trajectory)
{
    _segments = std::move(trajectory.segments);
    _segmentIndex = 0;
    _landing = trajectory.landing;
    _landingInLiquid = trajectory.outcome == Outcome::Liquid;
    _slowFall = IsSlowFalling();
    if (_segments.empty())
        Land();
    else
        LaunchSegment();
}

void AirborneController::LaunchSegment()
{
    Segment const& segment = _segments[_segmentIndex];
    _segmentStartMs = getMSTime();
    _splineId = 0;

    float const velocity = SegmentVelocity(segment);
    if (velocity > MIN_SPLINE_VELOCITY)
    {
        Movement::MoveSplineInit init(_bot);
        init.MoveTo(segment.end.x, segment.end.y, segment.end.z);
        if (segment.kind == SegmentKind::Parabolic)
            init.SetParabolic(ParabolicAmplitude(segment.durationMs), 0.0f);

        // Launch caps the velocity at four times run speed, so a long fall plays out slower than it would fall.
        init.SetVelocity(velocity);
        init.SetOrientationFixed(true);
        if (init.Launch())
        {
            _splineId = _bot->movespline->GetId();
            return;
        }
    }

    // Too short to launch: step to its end so Update moves on.
    _bot->UpdatePosition(segment.end.x, segment.end.y, segment.end.z, _bot->GetOrientation());
}

bool AirborneController::IsOwnSplineRunning() const
{
    return _splineId && !_bot->movespline->Finalized() && _bot->movespline->GetId() == _splineId;
}

bool AirborneController::IsCoreControlled() const
{
    return _bot->IsCharmed() || _bot->GetMotionMaster()->GetMotionSlotType(MOTION_SLOT_CONTROLLED) != NULL_MOTION_TYPE;
}

uint32 AirborneController::ElapsedInSegment() const
{
    Segment const& segment = _segments[_segmentIndex];
    if (!IsOwnSplineRunning())
        return std::min(getMSTimeDiff(_segmentStartMs, getMSTime()), segment.durationMs);

    // The spline may run longer than the segment when Launch capped its velocity.
    int32 const splineDuration = _bot->movespline->Duration();
    if (splineDuration <= 0)
        return segment.durationMs;

    return static_cast<uint32>(static_cast<uint64>(_bot->movespline->timePassed()) * segment.durationMs /
                               static_cast<uint64>(splineDuration));
}

Point AirborneController::CurrentPosition() const
{
    if (!IsOwnSplineRunning())
        return BotPoint();

    Movement::Location const position = _bot->movespline->ComputePosition();
    return {position.x, position.y, position.z};
}

Point AirborneController::CurrentVelocity() const
{
    if (_segments.empty())
        return {};

    Segment const& segment = _segments[_segmentIndex];
    if (!segment.durationMs)
        return {0.0f, 0.0f, segment.startVz};

    float const duration = segment.durationMs / 1000.0f;
    return {(segment.end.x - segment.start.x) / duration, (segment.end.y - segment.start.y) / duration,
            VerticalSpeedAt(segment, ElapsedInSegment())};
}

bool AirborneController::Interrupt(float orientation)
{
    if (IsCoreControlled())
        return false;

    Point position = BotPoint();
    if (IsOwnSplineRunning())
    {
        Segment& segment = _segments[_segmentIndex];
        segment = Remainder(segment, ElapsedInSegment());
        position = segment.start;
    }

    _bot->DisableSpline();
    _bot->UpdatePosition(position.x, position.y, position.z, orientation);
    return true;
}

void AirborneController::Resweep()
{
    Point const position = CurrentPosition();
    Point const velocity = CurrentVelocity();
    Interrupt(_bot->GetOrientation());
    _bot->GetMotionMaster()->Clear();
    Trajectory trajectory = SweepFrom(position, velocity, position.z - NO_DROP_CAP);
    if (!trajectory.Landed())
    {
        Settle();
        return;
    }

    Start(std::move(trajectory));
}

void AirborneController::Land()
{
    MovementInfo landing = _bot->m_movementInfo;
    landing.pos.Relocate(_landing.x, _landing.y, _landing.z);
    if (!_landingInLiquid && sPlayerbotAIConfig.FallDamage)
        _bot->HandleFall(landing);

    // Parachutes and the like end on landing, as the movement handler does for a client.
    _bot->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_LANDING);
    Settle();

    // A knockback leaves the bot away from where its last move was going; let it move again at once.
    _botAI->GetAiObjectContext()->GetValue<LastMovement&>("last movement")->Get().Set(nullptr);
}

void AirborneController::Settle()
{
    _airborne = false;
    _segments.clear();
    _segmentIndex = 0;
    _splineId = 0;
    if (!_bot->IsInWorld())
        return;

    // Player::IsFalling() compares the standing Z to the fall anchor.
    _bot->SetFallInformation(0, _bot->GetPositionZ());
    _bot->RemoveUnitMovementFlag(MOVEMENTFLAG_FALLING | MOVEMENTFLAG_FALLING_FAR);
    if (_rootPending)
    {
        _rootPending = false;
        _botAI->ApplyForceRoot();
        return;
    }

    if (!_bot->IsRooted())
        _bot->SendMovementFlagUpdate();
}
