/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_JUMPTRAJECTORY_H
#define PLAYERBOTS_JUMPTRAJECTORY_H

#include <vector>

#include "Define.h"

// Client airborne physics for bots: sweeps a jump, knockback or fall through the world and splits it into spline
// segments. No core types, so it is unit-testable without a server; the world is reached through CollisionProbe.
namespace JumpTrajectory
{
// Client values; the core's Movement::gravity and terminal velocities match them.
constexpr float GRAVITY = 19.29110527f;
constexpr float JUMP_VELOCITY = 7.9555473f;
constexpr float TERMINAL_VELOCITY = 60.148003f;
constexpr float SAFE_FALL_TERMINAL_VELOCITY = 7.0f;

constexpr float NO_HEIGHT = -100000.0f;  // the core's INVALID_HEIGHT

constexpr uint32 STEP_MS = 50;          // chord length while the descent still accelerates
constexpr uint32 CAPPED_STEP_MS = 500;  // at terminal velocity the path is straight, so a long chord is exact
constexpr uint32 MAX_STEPS = 200;
constexpr float FLOOR_SEARCH_MARGIN = 1.0f;
constexpr float TAKEOFF_SNAP = 0.5f;  // a floor this far above the takeoff point is where the bot stands
constexpr float WALK_STEP = 0.5f;     // floor probe spacing on a walk toward an edge
constexpr float SWIM_DEPTH = 0.75f;   // share of collision height; shallower water is stood in, as the core decides
// A void fall ends at least this far below the map's lowest point: the core counts only a body strictly below it as out
// of bounds.
constexpr float VOID_DEPTH = 1.0f;
// The client's body starts this many collision radii above the feet: a ledge lower than that is landed on, a taller one
// is a wall.
constexpr float STEP_HEIGHT_PER_RADIUS = 1.849399f;
constexpr float DEFAULT_COLLISION_RADIUS = 0.33333334f;

struct Point
{
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
};

enum class SegmentKind : uint8
{
    Parabolic,  // exact-gravity arc: the spline's parabola with amplitude ParabolicAmplitude(durationMs)
    Linear      // terminal velocity reached: a straight line
};

struct Segment
{
    Point start;
    Point end;
    uint32 durationMs = 0;
    float startVz = 0.0f;  // up positive
    SegmentKind kind = SegmentKind::Parabolic;
};

enum class Outcome : uint8
{
    Landed,
    Liquid,   // landed on a liquid surface, which takes no fall damage
    Void,     // fell below the map's lowest point, which kills
    DropCap,  // fell below Launch::minZ
    StepCap   // still airborne after MAX_STEPS chords
};

struct Launch
{
    Point position;
    Point velocity;  // z up positive
    float terminalVelocity = TERMINAL_VELOCITY;
    float collisionHeight = 2.03f;
    float landingOffset = 0.0f;  // hover height
    float minZ = NO_HEIGHT;      // refuse a trajectory that drops below this
    float stepHeight = DEFAULT_COLLISION_RADIUS * STEP_HEIGHT_PER_RADIUS;
    bool waterWalking = false;  // liquid surfaces are floors, and landing on one deals fall damage
};

class CollisionProbe
{
public:
    virtual ~CollisionProbe() = default;

    // First hit of static or dynamic geometry from `from` toward `to`, pulled back a little toward `from`.
    virtual bool RayHit(Point const& from, Point const& to, Point& hit) const = 0;
    // Heightmap only, NO_HEIGHT where there is none.
    virtual float TerrainZ(float x, float y) const = 0;
    // Highest floor (terrain, models, gameobjects) at or below zFrom within maxSearch, NO_HEIGHT when none.
    virtual float FloorZ(float x, float y, float zFrom, float maxSearch) const = 0;
    // The liquid surface when the point is in liquid, NO_HEIGHT otherwise.
    virtual float LiquidZ(float x, float y, float z) const = 0;
    // The map's lowest point; a fall below it is a fall into the void.
    virtual float MinZ(float x, float y) const = 0;
};

struct Trajectory
{
    Outcome outcome = Outcome::StepCap;
    std::vector<Segment> segments;  // empty when the bot lands where it is
    Point takeoff;                  // where the bot leaves the ground
    Point landing;

    bool Landed() const { return outcome == Outcome::Landed || outcome == Outcome::Liquid; }
    // Lands, or falls into the void: either way the fall plays out.
    bool Ends() const { return Landed() || outcome == Outcome::Void; }
};

Trajectory Sweep(Launch const& launch, CollisionProbe const& probe);
// Walks at the launch's horizontal speed until the floor drops away, then falls from there, as a client walking off a
// ledge does. StepCap when a wall or maxWalk comes first.
Trajectory WalkOff(Launch const& launch, CollisionProbe const& probe, float maxWalk);

// The parabolic amplitude that makes the spline's vertical acceleration exactly GRAVITY over the duration.
float ParabolicAmplitude(uint32 durationMs);
// The spline velocity that makes its duration equal the segment's.
float SegmentVelocity(Segment const& segment);
// Where the segment's spline puts the bot `elapsedMs` in, and its vertical speed there.
Point PositionAt(Segment const& segment, uint32 elapsedMs);
float VerticalSpeedAt(Segment const& segment, uint32 elapsedMs);
// The rest of a segment after `elapsedMs`, which follows the same path when launched from where the bot is.
Segment Remainder(Segment const& segment, uint32 elapsedMs);
}  // namespace JumpTrajectory

#endif
