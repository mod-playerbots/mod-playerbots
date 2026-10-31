/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_AIRBORNECONTROLLER_H
#define PLAYERBOTS_AIRBORNECONTROLLER_H

#include <optional>

#include "JumpTrajectory.h"

class Player;
class PlayerbotAI;
class Unit;

// What a bot's client would do while it is off the ground: the jump, knockback and fall arcs, landing with fall
// damage, and turning mid-air. Bots have no client, so without this they take no fall damage and keep a
// stale falling flag. Self bots have a real client and are left to it.
class AirborneController
{
public:
    AirborneController(PlayerbotAI* botAI, Player* bot) : _botAI(botAI), _bot(bot) {}

    bool IsAirborne() const { return _airborne; }
    bool CanJump() const;
    // A deliberate jump at the client's takeoff speed. Refused when the landing is off the walkable area or more than
    // a few yards below the takeoff, unless the fall is harmless.
    bool Jump(float heading, float speedXY);
    // Once the bot stands still above a target that no path reaches, walks off the ledge after it, as a player
    // following it would. Refused when the fall could kill the bot, or the landing is off the walkable area or no
    // nearer the target.
    bool DropDownTo(Unit const* target);
    void Knockback(float vcos, float vsin, float speedXY, float speedZ);
    // Falls from where the bot is on its next update, such as after a flying dismount or when flight ends. The core
    // reports both while the flight aura is still on, so the bot only knows it can fall once the aura is gone.
    void Fall() { _fallPending = true; }
    // Turns mid-air without changing the path. False when not airborne.
    bool Face(float orientation);
    // A root that arrives mid-air waits for the landing, as it does for a client.
    bool DeferRoot(bool root);
    // Every bot update: flies the arc while airborne, and drops a bot whose movement ends over nothing.
    void Update();
    // Drops the airborne state without landing, such as on a teleport.
    void Clear();

private:
    using Point = JumpTrajectory::Point;
    using Trajectory = JumpTrajectory::Trajectory;

    bool CanLeaveGround() const;
    bool IsSwimming() const;
    bool IsSlowFalling() const;
    // No fall damage on landing: slow fall, or fall damage turned off.
    bool IsFallHarmless() const;
    // The deepest landing below the takeoff the bot survives with some health to spare.
    float SurvivableDrop() const;
    Point BotPoint() const;
    // A walk of up to maxWalk to the edge first when set.
    Trajectory SweepFrom(Point const& position, Point const& velocity, float minZ, float maxWalk = 0.0f) const;
    bool IsOnNavMesh(Point const& point) const;
    // Lands, and on the walkable area unless in liquid.
    bool IsSafeLanding(Trajectory const& trajectory) const;
    bool IsOwnSplineRunning() const;
    // A core effect (Death Grip, fear, a charm) moves the bot as it would a client's; the fall waits for it.
    bool IsCoreControlled() const;
    // Where the bot is and how fast it moves now, from the active segment.
    Point CurrentPosition() const;
    Point CurrentVelocity() const;
    uint32 ElapsedInSegment() const;
    // Stops the bot at its current point on the path, the active segment shortened to the rest of it. False while a
    // core effect moves the bot.
    bool Interrupt(float orientation);
    void StartFall();
    // Leaves the ground at takeoffZ, or carries on from mid-air without one, and flies the trajectory.
    void Begin(Trajectory&& trajectory, std::optional<float> takeoffZ);
    void Start(Trajectory&& trajectory);
    void LaunchSegment();
    // Sweeps again from the current state, keeping the velocity, and drops any movement the bot's AI started;
    // settles where it is when no floor is in reach.
    void Resweep();
    // The core's knockback, which stops at the edge of the walkable area.
    Trajectory ProtectedKnockback(float vcos, float vsin, float speedXY, float speedZ) const;
    void Land();
    // What the movement handler does to a client that falls below the map: death, or the battleground's rescue.
    void FallToVoid();
    void Settle();

    PlayerbotAI* _botAI;
    Player* _bot;
    bool _airborne = false;
    bool _rootPending = false;
    bool _fallPending = false;
    bool _wasMoving = false;
    uint32 _lastDropDownMs = 0;
    bool _slowFall = false;
    std::vector<JumpTrajectory::Segment> _segments;
    size_t _segmentIndex = 0;
    uint32 _splineId = 0;
    uint32 _segmentStartMs = 0;
    uint32 _fallStartMs = 0;
    uint32 _nextHeartbeatMs = 0;  // fall time of the next position report
    Point _landing;
    JumpTrajectory::Outcome _landingOutcome = JumpTrajectory::Outcome::Landed;
    bool _swimOnLanding = false;  // the swim flag a knockback cleared, restored on landing in liquid
};

#endif
