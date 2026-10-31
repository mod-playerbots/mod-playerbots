/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include <cmath>
#include <functional>

#include "JumpTrajectory.h"
#include "gtest/gtest.h"

using namespace JumpTrajectory;

namespace
{
constexpr float RUN_SPEED = 7.0f;
constexpr float BACKOFF = 0.3f;

// Terrain from a height function, plus optional flat platforms, a wall plane at x = wallX, a ceiling plane and a
// liquid surface. Platforms are found only within the search distance, like VMAP floors; terrain is found from
// anywhere above it, like the heightmap.
struct FakeWorld : CollisionProbe
{
    struct Platform
    {
        float minX, maxX, z;
    };

    std::function<float(float, float)> terrain = [](float, float) { return 0.0f; };
    std::vector<Platform> platforms;
    float wallX = 1e9f;
    float ceilingZ = 1e9f;
    float liquidZ = NO_HEIGHT;
    float liquidFromX = -1e9f;
    float voidZ = -1e9f;
    mutable uint32 rays = 0;

    bool RayHit(Point const& from, Point const& to, Point& hit) const override
    {
        ++rays;
        float best = 2.0f;
        if ((from.x - wallX) * (to.x - wallX) < 0.0f)
            best = (wallX - from.x) / (to.x - from.x);
        if ((from.z - ceilingZ) * (to.z - ceilingZ) < 0.0f)
            best = std::min(best, (ceilingZ - from.z) / (to.z - from.z));
        if (best > 1.0f)
            return false;

        float const length = std::hypot(to.x - from.x, to.y - from.y, to.z - from.z);
        float const pulled = std::max(0.0f, best - BACKOFF / length);
        hit = {from.x + (to.x - from.x) * pulled, from.y + (to.y - from.y) * pulled, from.z + (to.z - from.z) * pulled};
        return true;
    }

    float TerrainZ(float x, float y) const override { return terrain(x, y); }

    float FloorZ(float x, float y, float zFrom, float maxSearch) const override
    {
        float best = NO_HEIGHT;
        float const ground = terrain(x, y);
        if (ground > NO_HEIGHT && ground <= zFrom)
            best = ground;
        for (Platform const& platform : platforms)
            if (x >= platform.minX && x <= platform.maxX && platform.z <= zFrom && zFrom - platform.z <= maxSearch)
                best = std::max(best, platform.z);
        return best;
    }

    // Like the core, no water below the bed.
    float LiquidZ(float x, float y, float z) const override
    {
        return x >= liquidFromX && z <= liquidZ && z >= terrain(x, y) ? liquidZ : NO_HEIGHT;
    }

    float MinZ(float, float) const override { return voidZ; }
};

Launch RunningJump(Point from)
{
    Launch launch;
    launch.position = from;
    launch.velocity.x = RUN_SPEED;
    launch.velocity.z = JUMP_VELOCITY;
    return launch;
}

uint32 TotalMs(Trajectory const& trajectory)
{
    uint32 total = 0;
    for (Segment const& segment : trajectory.segments)
        total += segment.durationMs;
    return total;
}

constexpr float FLAT_FLIGHT_SECONDS = 2.0f * JUMP_VELOCITY / GRAVITY;
}  // namespace

TEST(JumpTrajectory, FlatJumpIsOneExactArc)
{
    FakeWorld world;
    Trajectory trajectory = Sweep(RunningJump({0.0f, 0.0f, 0.0f}), world);

    ASSERT_EQ(trajectory.outcome, Outcome::Landed);
    ASSERT_EQ(trajectory.segments.size(), 1u);
    EXPECT_NEAR(TotalMs(trajectory), FLAT_FLIGHT_SECONDS * 1000.0f, 2.0f);
    EXPECT_NEAR(trajectory.landing.x, RUN_SPEED * FLAT_FLIGHT_SECONDS, 0.02f);
    EXPECT_FLOAT_EQ(trajectory.landing.z, 0.0f);
    EXPECT_EQ(trajectory.segments[0].kind, SegmentKind::Parabolic);
}

TEST(JumpTrajectory, InclineLandsEarly)
{
    FakeWorld world;
    world.terrain = [](float x, float) { return 0.3f * x; };
    Trajectory trajectory = Sweep(RunningJump({0.0f, 0.0f, 0.0f}), world);

    ASSERT_EQ(trajectory.outcome, Outcome::Landed);
    EXPECT_LT(trajectory.landing.x, RUN_SPEED * FLAT_FLIGHT_SECONDS - 0.5f);
    EXPECT_NEAR(trajectory.landing.z, 0.3f * trajectory.landing.x, 0.05f);
}

TEST(JumpTrajectory, DeclineLandsLate)
{
    FakeWorld world;
    world.terrain = [](float x, float) { return -0.3f * x; };
    Trajectory trajectory = Sweep(RunningJump({0.0f, 0.0f, 0.0f}), world);

    ASSERT_EQ(trajectory.outcome, Outcome::Landed);
    EXPECT_GT(trajectory.landing.x, RUN_SPEED * FLAT_FLIGHT_SECONDS + 0.5f);
    EXPECT_NEAR(trajectory.landing.z, -0.3f * trajectory.landing.x, 0.05f);
}

TEST(JumpTrajectory, WallStopsHorizontalTravelThenFallsStraightDown)
{
    FakeWorld world;
    world.platforms.push_back({-10.0f, 2.0f, 5.0f});
    world.wallX = 4.0f;
    Trajectory trajectory = Sweep(RunningJump({0.0f, 0.0f, 5.0f}), world);

    ASSERT_EQ(trajectory.outcome, Outcome::Landed);
    ASSERT_EQ(trajectory.segments.size(), 2u);
    Segment const& fall = trajectory.segments[1];
    EXPECT_FLOAT_EQ(fall.start.x, fall.end.x);
    EXPECT_LT(trajectory.landing.x, 4.0f);
    EXPECT_GT(trajectory.landing.x, 4.0f - 2.0f * BACKOFF);
    EXPECT_FLOAT_EQ(trajectory.landing.z, 0.0f);
}

TEST(JumpTrajectory, CeilingBonkKillsLiftButKeepsMomentum)
{
    FakeWorld world;
    world.ceilingZ = 3.0f;
    Trajectory trajectory = Sweep(RunningJump({0.0f, 0.0f, 0.0f}), world);

    ASSERT_EQ(trajectory.outcome, Outcome::Landed);
    ASSERT_EQ(trajectory.segments.size(), 2u);
    EXPECT_FLOAT_EQ(trajectory.segments[1].startVz, 0.0f);
    EXPECT_LT(trajectory.segments[0].end.z + 2.03f, 3.0f);
    EXPECT_GT(trajectory.landing.x, trajectory.segments[0].end.x);
}

TEST(JumpTrajectory, VerticalJumpSplitsAtTheApex)
{
    FakeWorld world;
    Launch launch;
    launch.velocity.z = JUMP_VELOCITY;
    Trajectory trajectory = Sweep(launch, world);

    ASSERT_EQ(trajectory.outcome, Outcome::Landed);
    ASSERT_EQ(trajectory.segments.size(), 2u);
    EXPECT_NEAR(trajectory.segments[0].end.z, JUMP_VELOCITY * JUMP_VELOCITY / (2.0f * GRAVITY), 0.01f);
    EXPECT_FLOAT_EQ(trajectory.landing.x, 0.0f);
    EXPECT_FLOAT_EQ(trajectory.landing.z, 0.0f);
}

TEST(JumpTrajectory, FallFromTheGroundLandsInPlace)
{
    FakeWorld world;
    Launch launch;
    launch.position = {1.0f, 2.0f, -0.2f};
    Trajectory trajectory = Sweep(launch, world);

    ASSERT_EQ(trajectory.outcome, Outcome::Landed);
    EXPECT_TRUE(trajectory.segments.empty());
    EXPECT_FLOAT_EQ(trajectory.landing.z, 0.0f);
}

TEST(JumpTrajectory, LiquidCatchesTheFall)
{
    FakeWorld world;
    world.terrain = [](float, float) { return -20.0f; };
    world.platforms.push_back({-10.0f, 1.0f, 10.0f});
    world.liquidZ = 0.0f;
    world.liquidFromX = 1.0f;
    Launch const launch = RunningJump({0.0f, 0.0f, 10.0f});
    Trajectory trajectory = Sweep(launch, world);

    // A client swims, and stops falling, once it sinks to swim depth.
    ASSERT_EQ(trajectory.outcome, Outcome::Liquid);
    EXPECT_FLOAT_EQ(trajectory.landing.z, -SWIM_DEPTH * launch.collisionHeight);
}

// Long falls take steps of many yards; one that passes the surface and ends below the bed still lands in the water.
TEST(JumpTrajectory, LiquidCatchesALongFall)
{
    FakeWorld world;
    world.liquidZ = 5.0f;
    for (uint32 height = 60; height <= 250; ++height)
    {
        Launch launch;
        launch.position = {0.0f, 0.0f, static_cast<float>(height)};
        Trajectory trajectory = Sweep(launch, world);

        ASSERT_EQ(trajectory.outcome, Outcome::Liquid) << "falling from " << height;
        EXPECT_FLOAT_EQ(trajectory.landing.z, 5.0f - SWIM_DEPTH * launch.collisionHeight);
    }
}

// A client launched under the surface rises through the water and swims on from the apex.
TEST(JumpTrajectory, LaunchUnderWaterSwimsOnFromTheApex)
{
    FakeWorld world;
    world.terrain = [](float, float) { return -50.0f; };
    world.liquidZ = 0.0f;
    Launch launch;
    launch.position = {0.0f, 0.0f, -10.0f};
    launch.velocity = {5.0f, 0.0f, 7.0f};
    Trajectory trajectory = Sweep(launch, world);

    float const apexTime = 7.0f / GRAVITY;
    ASSERT_EQ(trajectory.outcome, Outcome::Liquid);
    EXPECT_NEAR(trajectory.landing.x, 5.0f * apexTime, 0.001f);
    EXPECT_NEAR(trajectory.landing.z, -10.0f + 0.5f * 7.0f * apexTime, 0.001f);
}

// A fall that starts under the surface, such as a dismount while swimming, ends where it starts.
TEST(JumpTrajectory, FallUnderWaterSwimsAtOnce)
{
    FakeWorld world;
    world.terrain = [](float, float) { return -50.0f; };
    world.liquidZ = 0.0f;
    Launch launch;
    launch.position = {0.0f, 0.0f, -10.0f};
    Trajectory trajectory = Sweep(launch, world);

    ASSERT_EQ(trajectory.outcome, Outcome::Liquid);
    EXPECT_TRUE(trajectory.segments.empty());
    EXPECT_FLOAT_EQ(trajectory.landing.z, -10.0f);
}

// Water too shallow to swim in does not catch a fall, short or long.
TEST(JumpTrajectory, ShallowWaterLandsOnTheBed)
{
    FakeWorld world;
    world.liquidZ = 1.0f;
    for (uint32 height = 2; height <= 200; ++height)
    {
        Launch launch;
        launch.position = {0.0f, 0.0f, static_cast<float>(height)};
        Trajectory trajectory = Sweep(launch, world);

        ASSERT_EQ(trajectory.outcome, Outcome::Landed) << "falling from " << height;
        EXPECT_FLOAT_EQ(trajectory.landing.z, 0.0f);
    }
}

// Water under a floor does not catch a fall that lands on the floor.
TEST(JumpTrajectory, FloorAboveTheWaterCatchesTheFall)
{
    FakeWorld world;
    world.terrain = [](float, float) { return -20.0f; };
    world.platforms.push_back({-10.0f, 10.0f, 3.0f});
    world.liquidZ = 0.0f;
    Launch launch;
    launch.position = {0.0f, 0.0f, 40.0f};
    Trajectory trajectory = Sweep(launch, world);

    ASSERT_EQ(trajectory.outcome, Outcome::Landed);
    EXPECT_FLOAT_EQ(trajectory.landing.z, 3.0f);
}

TEST(JumpTrajectory, RefusesADropPastTheCap)
{
    FakeWorld world;
    world.platforms.push_back({-10.0f, 1.0f, 10.0f});
    Launch launch = RunningJump({0.0f, 0.0f, 10.0f});
    launch.minZ = 4.0f;

    EXPECT_EQ(Sweep(launch, world).outcome, Outcome::DropCap);
}

TEST(JumpTrajectory, RefusesWhenThereIsNoFloor)
{
    FakeWorld world;
    world.terrain = [](float, float) { return NO_HEIGHT; };
    Launch launch = RunningJump({0.0f, 0.0f, 10.0f});
    launch.minZ = -500.0f;
    EXPECT_EQ(Sweep(launch, world).outcome, Outcome::DropCap);

    launch.minZ = NO_HEIGHT;
    EXPECT_EQ(Sweep(launch, world).outcome, Outcome::StepCap);
}

// A client falls on until it passes below the map, where the server kills it.
TEST(JumpTrajectory, FallsIntoTheVoid)
{
    FakeWorld world;
    world.terrain = [](float, float) { return NO_HEIGHT; };
    world.voidZ = -500.0f;
    Trajectory trajectory = Sweep(RunningJump({0.0f, 0.0f, 10.0f}), world);

    ASSERT_EQ(trajectory.outcome, Outcome::Void);
    EXPECT_FALSE(trajectory.Landed());
    EXPECT_TRUE(trajectory.Ends());
    EXPECT_FLOAT_EQ(trajectory.landing.z, -500.0f);
}

// A deliberate drop is refused at its cap before it reaches the void.
TEST(JumpTrajectory, DropCapComesBeforeTheVoid)
{
    FakeWorld world;
    world.terrain = [](float, float) { return NO_HEIGHT; };
    world.voidZ = -500.0f;
    Launch launch = RunningJump({0.0f, 0.0f, 10.0f});
    launch.minZ = 4.0f;
    EXPECT_EQ(Sweep(launch, world).outcome, Outcome::DropCap);
}

// Floors and liquid above the map's lowest point still catch the fall.
TEST(JumpTrajectory, FloorAboveTheVoidCatchesTheFall)
{
    FakeWorld world;
    world.voidZ = -1.0f;
    Trajectory trajectory = Sweep(RunningJump({0.0f, 0.0f, 10.0f}), world);
    ASSERT_EQ(trajectory.outcome, Outcome::Landed);
    EXPECT_FLOAT_EQ(trajectory.landing.z, 0.0f);

    world.terrain = [](float, float) { return -50.0f; };
    world.liquidZ = 0.0f;
    world.voidZ = -10.0f;
    EXPECT_EQ(Sweep(RunningJump({0.0f, 0.0f, 10.0f}), world).outcome, Outcome::Liquid);
}

TEST(JumpTrajectory, LongFallReachesTerminalVelocityOnAStraightLine)
{
    FakeWorld world;
    Launch launch;
    launch.position = {0.0f, 0.0f, 300.0f};
    Trajectory trajectory = Sweep(launch, world);

    ASSERT_EQ(trajectory.outcome, Outcome::Landed);
    ASSERT_EQ(trajectory.segments.size(), 2u);
    EXPECT_EQ(trajectory.segments[0].kind, SegmentKind::Parabolic);
    EXPECT_EQ(trajectory.segments[1].kind, SegmentKind::Linear);
    EXPECT_NEAR(trajectory.segments[0].durationMs, TERMINAL_VELOCITY / GRAVITY * 1000.0f, 2.0f);
    float const terminalDrop = TERMINAL_VELOCITY * TERMINAL_VELOCITY / (2.0f * GRAVITY);
    float const expectedMs = (TERMINAL_VELOCITY / GRAVITY + (300.0f - terminalDrop) / TERMINAL_VELOCITY) * 1000.0f;
    EXPECT_NEAR(TotalMs(trajectory), expectedMs, 5.0f);
}

TEST(JumpTrajectory, SlowFallGlidesFarWithFewRays)
{
    FakeWorld world;
    world.platforms.push_back({-10.0f, 1.0f, 40.0f});
    Launch launch = RunningJump({0.0f, 0.0f, 40.0f});
    launch.terminalVelocity = SAFE_FALL_TERMINAL_VELOCITY;
    Trajectory trajectory = Sweep(launch, world);

    ASSERT_EQ(trajectory.outcome, Outcome::Landed);
    EXPECT_GT(trajectory.landing.x, 40.0f);
    EXPECT_EQ(trajectory.segments.back().kind, SegmentKind::Linear);
    EXPECT_LT(world.rays, 60u);
}

TEST(JumpTrajectory, HoverLandsAboveTheFloor)
{
    FakeWorld world;
    Launch launch = RunningJump({0.0f, 0.0f, 1.0f});
    launch.landingOffset = 1.0f;
    Trajectory trajectory = Sweep(launch, world);

    ASSERT_EQ(trajectory.outcome, Outcome::Landed);
    EXPECT_FLOAT_EQ(trajectory.landing.z, 1.0f);
}

// A hop that reaches a ledge with its feet more than a step height below the top meets the ledge as a wall.
TEST(JumpTrajectory, LedgeAboveTheStepHeightIsAWall)
{
    FakeWorld world;
    world.platforms.push_back({1.0f, 10.0f, 0.9f});
    Launch launch = RunningJump({0.0f, 0.0f, 0.0f});
    launch.velocity.z = 2.0f;
    Trajectory trajectory = Sweep(launch, world);

    ASSERT_EQ(trajectory.outcome, Outcome::Landed);
    EXPECT_LT(trajectory.landing.x, 1.0f);
    EXPECT_GT(trajectory.landing.x, 0.95f);
    EXPECT_FLOAT_EQ(trajectory.landing.z, 0.0f);
}

TEST(JumpTrajectory, LedgeBelowTheStepHeightIsLandedOn)
{
    FakeWorld world;
    world.platforms.push_back({1.0f, 10.0f, 0.4f});
    Launch launch = RunningJump({0.0f, 0.0f, 0.0f});
    launch.velocity.z = 2.0f;
    Trajectory trajectory = Sweep(launch, world);

    ASSERT_EQ(trajectory.outcome, Outcome::Landed);
    EXPECT_GE(trajectory.landing.x, 1.0f);
    EXPECT_LT(trajectory.landing.x, 1.05f);
    EXPECT_FLOAT_EQ(trajectory.landing.z, 0.4f);
}

TEST(JumpTrajectory, CeilingStopsAStandingJump)
{
    FakeWorld world;
    world.ceilingZ = 2.83f;
    Launch launch;
    launch.velocity.z = JUMP_VELOCITY;
    Trajectory trajectory = Sweep(launch, world);

    ASSERT_EQ(trajectory.outcome, Outcome::Landed);
    ASSERT_EQ(trajectory.segments.size(), 2u);
    float const headAtStop = trajectory.segments[0].end.z + launch.collisionHeight;
    EXPECT_LT(headAtStop, world.ceilingZ);
    EXPECT_GT(headAtStop, world.ceilingZ - 2.0f * BACKOFF);
    EXPECT_FLOAT_EQ(trajectory.segments[1].startVz, 0.0f);
    EXPECT_FLOAT_EQ(trajectory.landing.z, 0.0f);
}

// At terminal velocity a chord is half a second long; the stop still comes at the cliff, not at the chord's start.
TEST(JumpTrajectory, CliffStopsAFastFallWhereItRises)
{
    FakeWorld world;
    world.terrain = [](float x, float) { return x < 5.0f ? 0.0f : 300.0f; };
    Launch launch;
    launch.position = {0.0f, 0.0f, 200.0f};
    launch.velocity = {RUN_SPEED, 0.0f, -TERMINAL_VELOCITY};
    Trajectory trajectory = Sweep(launch, world);

    ASSERT_EQ(trajectory.outcome, Outcome::Landed);
    EXPECT_LT(trajectory.landing.x, 5.0f);
    EXPECT_GT(trajectory.landing.x, 4.95f);
    EXPECT_FLOAT_EQ(trajectory.landing.z, 0.0f);
}

TEST(JumpTrajectory, WaterWalkerLandsOnTheSurface)
{
    FakeWorld world;
    world.terrain = [](float, float) { return -20.0f; };
    world.platforms.push_back({-10.0f, 1.0f, 10.0f});
    world.liquidZ = 0.0f;
    world.liquidFromX = 1.0f;
    Launch launch = RunningJump({0.0f, 0.0f, 10.0f});
    launch.waterWalking = true;
    Trajectory trajectory = Sweep(launch, world);

    // A floor, so it takes fall damage.
    ASSERT_EQ(trajectory.outcome, Outcome::Landed);
    EXPECT_FLOAT_EQ(trajectory.landing.z, 0.0f);
}

TEST(JumpTrajectory, WaterWalkerJumpsOffTheSurface)
{
    FakeWorld world;
    world.terrain = [](float, float) { return -20.0f; };
    world.liquidZ = 0.0f;
    Launch launch = RunningJump({0.0f, 0.0f, 0.0f});
    launch.waterWalking = true;
    Trajectory trajectory = Sweep(launch, world);

    ASSERT_EQ(trajectory.outcome, Outcome::Landed);
    EXPECT_NEAR(trajectory.landing.x, RUN_SPEED * FLAT_FLIGHT_SECONDS, 0.02f);
    EXPECT_FLOAT_EQ(trajectory.landing.z, 0.0f);

    // Standing on it, it does not fall in.
    launch.velocity = {};
    trajectory = Sweep(launch, world);
    ASSERT_EQ(trajectory.outcome, Outcome::Landed);
    EXPECT_TRUE(trajectory.segments.empty());
    EXPECT_FLOAT_EQ(trajectory.landing.z, 0.0f);
}

TEST(JumpTrajectorySegment, AmplitudeAndVelocityMatchTheSplineMath)
{
    EXPECT_FLOAT_EQ(ParabolicAmplitude(1000), GRAVITY / 8.0f);
    EXPECT_FLOAT_EQ(ParabolicAmplitude(500), GRAVITY / 32.0f);

    Segment segment{{0.0f, 0.0f, 0.0f}, {3.0f, 4.0f, 0.0f}, 500, 0.0f, SegmentKind::Parabolic};
    EXPECT_FLOAT_EQ(SegmentVelocity(segment), 10.0f);
    segment.durationMs = 0;
    EXPECT_FLOAT_EQ(SegmentVelocity(segment), 0.0f);
}

TEST(JumpTrajectorySegment, SplineFollowsGravityExactly)
{
    FakeWorld world;
    Trajectory trajectory = Sweep(RunningJump({0.0f, 0.0f, 0.0f}), world);
    ASSERT_EQ(trajectory.segments.size(), 1u);
    Segment const& segment = trajectory.segments[0];

    for (uint32 ms = 0; ms <= segment.durationMs; ms += 100)
    {
        float const t = ms / 1000.0f;
        EXPECT_NEAR(PositionAt(segment, ms).z, JUMP_VELOCITY * t - 0.5f * GRAVITY * t * t, 0.01f) << ms;
        EXPECT_NEAR(VerticalSpeedAt(segment, ms), JUMP_VELOCITY - GRAVITY * t, 0.02f) << ms;
    }
}

TEST(JumpTrajectorySegment, MidArcRelaunchContinuesTheSameArc)
{
    Segment segment{{0.0f, 0.0f, 10.0f}, {6.0f, 2.0f, 4.0f}, 900, 3.0f, SegmentKind::Parabolic};
    for (uint32 cut : {0u, 150u, 450u, 899u})
    {
        Segment const rest = Remainder(segment, cut);
        EXPECT_EQ(rest.durationMs, segment.durationMs - cut);
        for (uint32 ms = 0; ms <= rest.durationMs; ms += 50)
        {
            Point const expected = PositionAt(segment, cut + ms);
            Point const actual = PositionAt(rest, ms);
            EXPECT_NEAR(actual.x, expected.x, 0.001f);
            EXPECT_NEAR(actual.y, expected.y, 0.001f);
            EXPECT_NEAR(actual.z, expected.z, 0.001f);
        }
    }
}

namespace
{
Launch Walk(Point from)
{
    Launch launch;
    launch.position = from;
    launch.velocity.x = RUN_SPEED;
    return launch;
}

constexpr float WALK_LIMIT = 10.0f;
}  // namespace

TEST(JumpTrajectoryWalkOff, WalksToTheEdgeThenFalls)
{
    FakeWorld world;
    world.platforms.push_back({-100.0f, 3.0f, 10.0f});
    Trajectory trajectory = WalkOff(Walk({0.0f, 0.0f, 10.0f}), world, WALK_LIMIT);

    ASSERT_EQ(trajectory.outcome, Outcome::Landed);
    ASSERT_GE(trajectory.segments.size(), 2u);
    Segment const& walk = trajectory.segments.front();
    EXPECT_EQ(walk.kind, SegmentKind::Linear);
    EXPECT_FLOAT_EQ(walk.end.x, 3.5f);
    EXPECT_FLOAT_EQ(walk.end.z, 10.0f);
    EXPECT_EQ(walk.durationMs, 500u);
    EXPECT_FLOAT_EQ(trajectory.takeoff.x, 3.5f);
    EXPECT_FLOAT_EQ(trajectory.takeoff.z, 10.0f);

    // Leaves the edge at run speed with no vertical speed.
    float const fallSeconds = std::sqrt(2.0f * 10.0f / GRAVITY);
    EXPECT_FLOAT_EQ(trajectory.landing.z, 0.0f);
    EXPECT_NEAR(trajectory.landing.x, 3.5f + RUN_SPEED * fallSeconds, 0.05f);
}

TEST(JumpTrajectoryWalkOff, FollowsStepsDownToTheEdge)
{
    FakeWorld world;
    world.platforms.push_back({-100.0f, 1.0f, 10.0f});
    world.platforms.push_back({1.0f, 2.0f, 9.6f});
    world.platforms.push_back({2.0f, 3.0f, 9.2f});
    Trajectory trajectory = WalkOff(Walk({0.0f, 0.0f, 10.0f}), world, WALK_LIMIT);

    ASSERT_EQ(trajectory.outcome, Outcome::Landed);
    EXPECT_FLOAT_EQ(trajectory.takeoff.x, 3.5f);
    EXPECT_FLOAT_EQ(trajectory.takeoff.z, 9.2f);
    EXPECT_FLOAT_EQ(trajectory.landing.z, 0.0f);
}

// The fall counts from the edge, above where the walk began.
TEST(JumpTrajectoryWalkOff, ClimbsToTheEdgeThenFalls)
{
    FakeWorld world;
    world.platforms.push_back({-100.0f, 1.0f, 10.0f});
    world.platforms.push_back({1.0f, 2.0f, 10.4f});
    world.platforms.push_back({2.0f, 3.0f, 10.8f});
    Trajectory trajectory = WalkOff(Walk({0.0f, 0.0f, 10.0f}), world, WALK_LIMIT);

    ASSERT_EQ(trajectory.outcome, Outcome::Landed);
    EXPECT_FLOAT_EQ(trajectory.takeoff.x, 3.5f);
    EXPECT_FLOAT_EQ(trajectory.takeoff.z, 10.8f);
    EXPECT_FLOAT_EQ(trajectory.landing.z, 0.0f);
}

TEST(JumpTrajectoryWalkOff, NoEdgeInReachRefuses)
{
    FakeWorld world;
    Trajectory trajectory = WalkOff(Walk({0.0f, 0.0f, 0.0f}), world, WALK_LIMIT);

    EXPECT_EQ(trajectory.outcome, Outcome::StepCap);
    EXPECT_TRUE(trajectory.segments.empty());
}

TEST(JumpTrajectoryWalkOff, WallBeforeTheEdgeRefuses)
{
    FakeWorld world;
    world.platforms.push_back({-100.0f, 3.0f, 10.0f});
    world.wallX = 2.2f;
    Trajectory trajectory = WalkOff(Walk({0.0f, 0.0f, 10.0f}), world, WALK_LIMIT);

    EXPECT_EQ(trajectory.outcome, Outcome::StepCap);
    EXPECT_TRUE(trajectory.segments.empty());
}

TEST(JumpTrajectoryWalkOff, StandingStillRefuses)
{
    FakeWorld world;
    Launch launch = Walk({0.0f, 0.0f, 10.0f});
    launch.velocity.x = 0.0f;
    world.platforms.push_back({-100.0f, 3.0f, 10.0f});

    EXPECT_EQ(WalkOff(launch, world, WALK_LIMIT).outcome, Outcome::StepCap);
}
