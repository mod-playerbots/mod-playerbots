/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "JumpTrajectory.h"

#include <algorithm>
#include <cmath>
#include <utility>

namespace JumpTrajectory
{
namespace
{
constexpr float EPSILON = 0.0001f;
constexpr uint32 CONTACT_ITERATIONS = 8;  // halvings that place a contact along a chord

enum class Contact : uint8
{
    None,
    Wall,
    Ceiling
};

Point Lerp(Point const& from, Point const& to, float fraction)
{
    return {from.x + (to.x - from.x) * fraction, from.y + (to.y - from.y) * fraction,
            from.z + (to.z - from.z) * fraction};
}

// How far along a chord a hit lies: by horizontal travel, or by height when the chord is vertical.
float ChordFraction(Point const& from, Point const& to, Point const& hit)
{
    float const dx = to.x - from.x;
    float const dy = to.y - from.y;
    float const lengthSquared = dx * dx + dy * dy;
    if (lengthSquared >= EPSILON)
        return std::clamp(((hit.x - from.x) * dx + (hit.y - from.y) * dy) / lengthSquared, 0.0f, 1.0f);

    float const dz = to.z - from.z;
    return std::abs(dz) < EPSILON ? 0.0f : std::clamp((hit.z - from.z) / dz, 0.0f, 1.0f);
}

// Where along a chord `touches` starts to hold, given that it holds at the end: the last fraction clear of it and the
// first touching it.
template <typename Touches>
std::pair<float, float> FirstContact(Touches const& touches)
{
    float clear = 0.0f;
    float touching = 1.0f;
    for (uint32 i = 0; i < CONTACT_ITERATIONS; ++i)
    {
        float const middle = (clear + touching) * 0.5f;
        if (touches(middle))
            touching = middle;
        else
            clear = middle;
    }

    return {clear, touching};
}

float Seconds(uint32 ms) { return static_cast<float>(ms) / 1000.0f; }
uint32 Milliseconds(float seconds) { return static_cast<uint32>(seconds * 1000.0f + 0.5f); }

// The height below which the bot swims in `liquid`, NO_HEIGHT when it is too shallow to swim in. Shallower liquid
// lands the bot on its bed, as the client and core decide.
float SwimHeight(CollisionProbe const& probe, Point const& end, float liquid, float collisionHeight)
{
    float const swimDepth = SWIM_DEPTH * collisionHeight;
    float const bed = probe.FloorZ(end.x, end.y, liquid, swimDepth);
    return bed <= NO_HEIGHT || liquid - bed > swimDepth ? liquid - swimDepth : NO_HEIGHT;
}
}  // namespace

Trajectory Sweep(Launch const& launch, CollisionProbe const& probe)
{
    Trajectory trajectory;
    float const halfHeight = launch.collisionHeight * 0.5f;
    float const terminal = launch.terminalVelocity;

    Point position = launch.position;
    float vx = launch.velocity.x;
    float vy = launch.velocity.y;
    float vz = std::max(launch.velocity.z, -terminal);

    // A takeoff a hair below the floor it stands on would land at once.
    float const takeoffFloor =
        probe.FloorZ(position.x, position.y, position.z + halfHeight, halfHeight + FLOOR_SEARCH_MARGIN);
    float const standZ = takeoffFloor > NO_HEIGHT ? takeoffFloor + launch.landingOffset : NO_HEIGHT;
    if (standZ > position.z && standZ - position.z <= TAKEOFF_SNAP)
        position.z = standZ;

    trajectory.takeoff = position;

    // Not rising off the floor it stands on: it lands where it is.
    if (takeoffFloor > NO_HEIGHT && vz <= 0.0f && position.z <= standZ)
    {
        trajectory.outcome = Outcome::Landed;
        trajectory.landing = {position.x, position.y, standZ};
        return trajectory;
    }

    // Under the heightmap (caves, building interiors) the terrain is not a wall. Only horizontal travel meets walls.
    bool underTerrain = true;
    if (vx * vx + vy * vy > EPSILON)
    {
        float const takeoffTerrain = probe.TerrainZ(position.x, position.y);
        underTerrain = takeoffTerrain > NO_HEIGHT && position.z < takeoffTerrain - TAKEOFF_SNAP;
    }

    Point segmentStart = position;
    float segmentStartVz = vz;
    float segmentTime = 0.0f;
    SegmentKind segmentKind = SegmentKind::Parabolic;

    auto openSegment = [&]()
    {
        segmentStart = position;
        segmentStartVz = vz;
        segmentTime = 0.0f;
        segmentKind = vz <= -terminal + EPSILON ? SegmentKind::Linear : SegmentKind::Parabolic;
    };
    auto closeSegment = [&]()
    {
        uint32 const durationMs = Milliseconds(segmentTime);
        if (durationMs > 0)
            trajectory.segments.push_back({segmentStart, position, durationMs, segmentStartVz, segmentKind});
    };
    openSegment();

    // The floor under a point of the chord, searched from the body's middle at the chord's higher end.
    auto floorAt = [&](Point const& point)
    {
        float const searchFrom = std::max(position.z, point.z) + halfHeight;
        return probe.FloorZ(point.x, point.y, searchFrom, searchFrom - point.z + FLOOR_SEARCH_MARGIN);
    };
    auto onFloor = [&](Point const& point)
    {
        float const floor = floorAt(point);
        return floor > NO_HEIGHT && point.z <= floor + launch.landingOffset;
    };

    for (uint32 step = 0; step < MAX_STEPS; ++step)
    {
        bool const capped = vz <= -terminal + EPSILON;
        bool const horizontal = vx * vx + vy * vy > EPSILON;
        float dt = Seconds(capped ? CAPPED_STEP_MS : STEP_MS);
        bool reachesTerminal = false;
        bool reachesApex = false;
        if (!capped)
        {
            float const toTerminal = (vz + terminal) / GRAVITY;
            if (toTerminal < dt)
            {
                dt = toTerminal;
                reachesTerminal = true;
            }

            // Straight up and back down is a spline with no length; split it at the apex.
            if (!horizontal && vz > 0.0f && vz / GRAVITY < dt)
            {
                dt = vz / GRAVITY;
                reachesApex = true;
                reachesTerminal = false;
            }
        }

        auto arcAt = [&](float t)
        {
            return Point{position.x + vx * t, position.y + vy * t,
                         capped ? position.z - terminal * t : position.z + vz * t - 0.5f * GRAVITY * t * t};
        };
        Point const end = arcAt(dt);

        float fraction = 1.0f;
        Contact contact = Contact::None;
        Point hit;
        if (horizontal)
        {
            // Terrain rising into the body is a wall where it starts to, not at the chord's start.
            auto intoTerrain = [&](float t)
            {
                Point const point = arcAt(dt * t);
                float const terrain = probe.TerrainZ(point.x, point.y);
                return terrain > NO_HEIGHT && terrain > std::max(position.z, point.z) + halfHeight;
            };
            if (!underTerrain && intoTerrain(1.0f))
            {
                fraction = FirstContact(intoTerrain).first;
                contact = Contact::Wall;
            }
            else if (probe.RayHit({position.x, position.y, position.z + halfHeight}, {end.x, end.y, end.z + halfHeight},
                                  hit))
            {
                fraction = ChordFraction(position, end, hit);
                contact = Contact::Wall;
            }
        }

        Point const head{position.x, position.y, position.z + launch.collisionHeight};
        Point const endHead{end.x, end.y, end.z + launch.collisionHeight};
        if (contact == Contact::None && vz > 0.0f && probe.RayHit(head, endHead, hit))
        {
            fraction = ChordFraction(head, endHead, hit);
            contact = Contact::Ceiling;
        }

        float chordTime = dt * fraction;
        Point chordEnd = contact == Contact::None ? end : arcAt(chordTime);

        auto finish = [&](Point const& landing, float time, Outcome outcome)
        {
            position = landing;
            segmentTime += time;
            closeSegment();
            trajectory.outcome = outcome;
            trajectory.landing = landing;
            return trajectory;
        };

        // Lands where the chord sinks to a level surface, or at its end when it rises onto it.
        auto land = [&](float surface, Outcome outcome)
        {
            float const startClearance = position.z - surface;
            float crossing = vz > 0.0f ? 1.0f : 0.0f;
            if (startClearance > 0.0f)
                crossing = startClearance / (startClearance - (chordEnd.z - surface));

            Point landing = Lerp(position, chordEnd, crossing);
            landing.z = surface;
            return finish(landing, chordTime * crossing, outcome);
        };

        float const floor = floorAt(chordEnd);
        bool const reachesFloor = floor > NO_HEIGHT && chordEnd.z <= floor + launch.landingOffset;

        // A long chord can pass through liquid and end below its bed, so ask at the floor: the core finds no liquid
        // below the bed.
        float const chordEndVz = capped ? -terminal : vz - GRAVITY * chordTime;
        float const liquid =
            chordEndVz < 0.0f ? probe.LiquidZ(chordEnd.x, chordEnd.y, reachesFloor ? floor : chordEnd.z) : NO_HEIGHT;
        if (liquid > NO_HEIGHT)
        {
            // A water walker comes down on the surface as on a floor.
            float const surface = liquid + launch.landingOffset;
            if (launch.waterWalking && chordEnd.z <= surface && position.z >= surface - TAKEOFF_SNAP)
                return land(surface, Outcome::Landed);

            // Past the apex a client swims, and stops falling, once it sinks to swim depth.
            float const swimZ = SwimHeight(probe, chordEnd, liquid, launch.collisionHeight);
            if (swimZ > NO_HEIGHT && chordEnd.z <= swimZ)
            {
                if (position.z > swimZ)
                    return land(swimZ, Outcome::Liquid);

                // Launched under the surface: it swims on from the apex.
                float const apexTime = capped || vz <= 0.0f ? 0.0f : vz / GRAVITY;
                return finish(arcAt(apexTime), apexTime, Outcome::Liquid);
            }
        }

        if (reachesFloor)
        {
            auto const [clear, touching] = FirstContact([&](float t) { return onFloor(arcAt(chordTime * t)); });
            Point landing = arcAt(chordTime * touching);
            float const landingFloor = floorAt(landing) + launch.landingOffset;
            if (!horizontal || landingFloor - landing.z <= launch.stepHeight)
            {
                landing.z = landingFloor;
                return finish(landing, chordTime * touching, Outcome::Landed);
            }

            // A ledge above the step height meets the body's side, as a wall.
            chordTime *= clear;
            chordEnd = arcAt(chordTime);
            contact = Contact::Wall;
        }
        else if (chordEnd.z < launch.minZ)
        {
            trajectory.outcome = Outcome::DropCap;
            return trajectory;
        }
        else
        {
            float const voidZ = probe.MinZ(chordEnd.x, chordEnd.y);
            if (chordEnd.z < voidZ)
                return land(voidZ, Outcome::Void);
        }

        position = chordEnd;
        segmentTime += chordTime;
        vz = capped ? -terminal : vz - GRAVITY * chordTime;

        if (contact != Contact::None || reachesTerminal || reachesApex)
        {
            closeSegment();
            if (contact == Contact::Wall)
            {
                vx = 0.0f;
                vy = 0.0f;
            }
            else if (contact == Contact::Ceiling)
                vz = std::min(vz, 0.0f);
            else
                vz = reachesTerminal ? -terminal : 0.0f;

            openSegment();
        }
    }

    trajectory.outcome = Outcome::StepCap;
    return trajectory;
}

Trajectory WalkOff(Launch const& launch, CollisionProbe const& probe, float maxWalk)
{
    float const speed = std::hypot(launch.velocity.x, launch.velocity.y);
    if (speed < EPSILON)
        return {};

    float const halfHeight = launch.collisionHeight * 0.5f;
    float const stepX = launch.velocity.x / speed * WALK_STEP;
    float const stepY = launch.velocity.y / speed * WALK_STEP;
    Point position = launch.position;
    uint32 const steps = static_cast<uint32>(maxWalk / WALK_STEP);
    for (uint32 step = 1; step <= steps; ++step)
    {
        Point const next{position.x + stepX, position.y + stepY, position.z};
        Point hit;
        if (probe.RayHit({position.x, position.y, position.z + halfHeight}, {next.x, next.y, next.z + halfHeight}, hit))
            return {};

        // Searched as the sweep searches at takeoff, so the floor the walk steps off is one the fall leaves behind.
        float const floor = probe.FloorZ(next.x, next.y, next.z + halfHeight, halfHeight + FLOOR_SEARCH_MARGIN);
        if (floor > NO_HEIGHT && floor + launch.landingOffset >= next.z - FLOOR_SEARCH_MARGIN)
        {
            position = {next.x, next.y, floor + launch.landingOffset};
            continue;
        }

        // The floor drops away: the walk carries the bot this one step past the edge, and it falls from there.
        Launch fall = launch;
        fall.position = next;
        fall.velocity.z = 0.0f;
        Trajectory trajectory = Sweep(fall, probe);
        float const walked = static_cast<float>(step) * WALK_STEP;
        trajectory.segments.insert(trajectory.segments.begin(),
                                   {launch.position, next, Milliseconds(walked / speed), 0.0f, SegmentKind::Linear});
        return trajectory;
    }

    return {};
}

float ParabolicAmplitude(uint32 durationMs)
{
    float const duration = Seconds(durationMs);
    return GRAVITY * duration * duration / 8.0f;
}

float SegmentVelocity(Segment const& segment)
{
    if (!segment.durationMs)
        return 0.0f;

    float const dx = segment.end.x - segment.start.x;
    float const dy = segment.end.y - segment.start.y;
    float const dz = segment.end.z - segment.start.z;
    return std::sqrt(dx * dx + dy * dy + dz * dz) / Seconds(segment.durationMs);
}

Point PositionAt(Segment const& segment, uint32 elapsedMs)
{
    if (!segment.durationMs)
        return segment.end;

    elapsedMs = std::min(elapsedMs, segment.durationMs);
    Point position =
        Lerp(segment.start, segment.end, static_cast<float>(elapsedMs) / static_cast<float>(segment.durationMs));
    if (segment.kind == SegmentKind::Parabolic)
    {
        float const elapsed = Seconds(elapsedMs);
        position.z += 0.5f * GRAVITY * elapsed * (Seconds(segment.durationMs) - elapsed);
    }

    return position;
}

float VerticalSpeedAt(Segment const& segment, uint32 elapsedMs)
{
    if (!segment.durationMs)
        return segment.startVz;

    float const duration = Seconds(segment.durationMs);
    float const elapsed = Seconds(std::min(elapsedMs, segment.durationMs));
    float speed = (segment.end.z - segment.start.z) / duration;
    if (segment.kind == SegmentKind::Parabolic)
        speed += 0.5f * GRAVITY * (duration - 2.0f * elapsed);

    return speed;
}

Segment Remainder(Segment const& segment, uint32 elapsedMs)
{
    elapsedMs = std::min(elapsedMs, segment.durationMs);
    return {PositionAt(segment, elapsedMs), segment.end, segment.durationMs - elapsedMs,
            VerticalSpeedAt(segment, elapsedMs), segment.kind};
}
}  // namespace JumpTrajectory
