/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "JumpTrajectory.h"

#include <algorithm>
#include <cmath>

namespace JumpTrajectory
{
namespace
{
constexpr float EPSILON = 0.0001f;

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

// How far along the chord's horizontal travel a hit lies.
float HorizontalFraction(Point const& from, Point const& to, Point const& hit)
{
    float const dx = to.x - from.x;
    float const dy = to.y - from.y;
    float const lengthSquared = dx * dx + dy * dy;
    if (lengthSquared < EPSILON)
        return 0.0f;

    return std::clamp(((hit.x - from.x) * dx + (hit.y - from.y) * dy) / lengthSquared, 0.0f, 1.0f);
}

float Seconds(uint32 ms) { return static_cast<float>(ms) / 1000.0f; }
uint32 Milliseconds(float seconds) { return static_cast<uint32>(seconds * 1000.0f + 0.5f); }
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
    if (takeoffFloor > NO_HEIGHT)
    {
        float const standZ = takeoffFloor + launch.landingOffset;
        if (standZ > position.z && standZ - position.z <= TAKEOFF_SNAP)
            position.z = standZ;
    }

    trajectory.takeoff = position;
    float floorBelow = takeoffFloor;  // under the chord's start, NO_HEIGHT when the last search found none

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
        if (horizontal)
        {
            float const terrain = underTerrain ? NO_HEIGHT : probe.TerrainZ(end.x, end.y);
            Point hit;
            if (terrain > NO_HEIGHT && terrain > std::max(position.z, end.z) + halfHeight)
            {
                fraction = 0.0f;
                contact = Contact::Wall;
            }
            else if (probe.RayHit({position.x, position.y, position.z + halfHeight}, {end.x, end.y, end.z + halfHeight},
                                  hit))
            {
                fraction = HorizontalFraction(position, end, hit);
                contact = Contact::Wall;
            }
            else if (vz > 0.0f && probe.RayHit({position.x, position.y, position.z + launch.collisionHeight},
                                               {end.x, end.y, end.z + launch.collisionHeight}, hit))
            {
                fraction = HorizontalFraction(position, end, hit);
                contact = Contact::Ceiling;
            }
        }

        float const chordTime = dt * fraction;
        Point const chordEnd = contact == Contact::None ? end : arcAt(chordTime);
        float const chordEndVz = capped ? -terminal : vz - GRAVITY * chordTime;

        // Lands where the chord crosses the surface, or at its end when it steps up onto a ledge on the way up.
        // With the surface known under both ends the crossing is exact on a planar slope.
        auto land = [&](float startSurface, float endSurface, Outcome outcome)
        {
            if (startSurface <= NO_HEIGHT)
                startSurface = endSurface;

            float const startClearance = position.z - startSurface;
            float const endClearance = chordEnd.z - endSurface;
            float crossing = vz > 0.0f ? 1.0f : 0.0f;
            if (startClearance > 0.0f)
                crossing = startClearance / (startClearance - endClearance);

            Point landing = Lerp(position, chordEnd, crossing);
            landing.z = startSurface + (endSurface - startSurface) * crossing;
            position = landing;
            segmentTime += chordTime * crossing;
            closeSegment();
            trajectory.outcome = outcome;
            trajectory.landing = landing;
            return trajectory;
        };

        float const searchFrom = std::max(position.z, chordEnd.z) + halfHeight;
        float const floor =
            probe.FloorZ(chordEnd.x, chordEnd.y, searchFrom, searchFrom - chordEnd.z + FLOOR_SEARCH_MARGIN);
        float const floorSurface = floor + launch.landingOffset;
        bool const reachesFloor = floor > NO_HEIGHT && chordEnd.z <= floorSurface;

        // A long chord can pass through water and end below its bed; the water catches the fall first. The core finds
        // no water below the bed, so ask at the floor. Water too shallow to swim in lands the bot on the bed.
        if (chordEndVz < 0.0f)
        {
            float const liquid = probe.LiquidZ(chordEnd.x, chordEnd.y, reachesFloor ? floor : chordEnd.z);
            if (liquid > NO_HEIGHT && chordEnd.z <= liquid)
            {
                float const swimDepth = SWIM_DEPTH * launch.collisionHeight;
                float const bed = probe.FloorZ(chordEnd.x, chordEnd.y, liquid, swimDepth);
                if (bed <= NO_HEIGHT || liquid - bed > swimDepth)
                    return land(liquid, liquid, Outcome::Liquid);
            }
        }

        if (reachesFloor)
        {
            float const startSurface = floorBelow > NO_HEIGHT ? floorBelow + launch.landingOffset : NO_HEIGHT;
            return land(startSurface, floorSurface, Outcome::Landed);
        }

        if (chordEnd.z < launch.minZ)
        {
            trajectory.outcome = Outcome::DropCap;
            return trajectory;
        }

        position = chordEnd;
        segmentTime += chordTime;
        vz = chordEndVz;
        floorBelow = floor;

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
    for (float walked = WALK_STEP; walked <= maxWalk; walked += WALK_STEP)
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
