/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_MOVEMENTACTIONS_H
#define PLAYERBOTS_MOVEMENTACTIONS_H

#include <cmath>

#include "Action.h"
#include "LastMovementValue.h"
#include "PathGenerator.h"
#include "PlayerbotAIConfig.h"

class Player;
class PlayerbotAI;
class Unit;
class WorldObject;
class Position;

// ============================================================================
// MOVEMENT DESIGN INTENT
//
// The goal is the most natural bot movement achievable while maintaining
// responsiveness and low server cost — any approach must scale across
// thousands of bots. Cheating (teleports, skipped travel) is minimized, and
// generates a debug log to identify areas for improvement.
//
// The pipeline is a port of the cmangos playerbot movement code (MoveTo /
// MoveTo2 / DispatchMovement / ResolveMovePath). Throughout this header and
// MovementActions.cpp, "the reference" means that implementation — those
// comments record where this port deliberately follows it or diverges.
//
// --- MoveTo ----------------------------------------------------
//  1. CAN WE MOVE AT ALL?  UpdateMovementState + IsMovingAllowed — dead,
//     CC'd, mid-flight, uncontrolled.
//  2. WHO IS THE MOVER?  A bot in a controlling vehicle seat moves the
//     vehicle base — dispatching on the seated Player would move a body the
//     encounter ignores while the vehicle stands still. A passenger without
//     control refuses: its legs aren't its own.
//  3. IS THIS ACTION ALLOWED RIGHT NOW?  MovementPriority dictates
//     replacement: while the bot is still executing a walk, a
//     lower-priority action does not replace it; higher or equal does
//     (newest wins). Then exact-duplicate dedup so identical re-issues
//     cost nothing.
//  4. LITERAL OR ROUTED?  Some moves must be taken exactly as given — exact
//     waypoints (formations), flying/swimming movers, backward steps,
//     vehicles (the route planner has no notion of a non-bot mover), and the
//     disableMoveSplinePath config — those go straight to a plain point
//     dispatch (DoMovePoint) and never enter MoveTo2. Everything else runs
//     the routing pipeline.
//
// --- MoveTo2: the routing pipeline ------------------------------------------
//  5. ALREADY RIDING?  (WaitForTransport) On a recorded transport ride the
//     only decisions are "stay aboard" or "this is my stop" — walking logic
//     is meaningless on a moving deck, so the ride is resolved before any
//     of it runs.
//  6. UNWATCHED-BOT ECONOMY.  A config-gated cheat that lets bots no player
//     can see advance by teleport instead of walking, cutting server cost.
//  7. CLOSE ENOUGH TO DESTINATION? (short-stop, targetPosRecalcDistance) Arriving is a
//     decision too: within the threshold the journey ends — stop, clear state.
//  8. GET A ROUTE.  (ResolveMovePath)
//       - reuse the cached path while the destination has barely moved
//         (<10% of the journey). Avoid re-planning every tick;
//       - near + same map (within sightDistance): direct navmesh query —
//         no mental map needed to cross a field;
//       - far or cross-map: the travel-node graph (roads, boats, flights, portals),
//         plans the trip leg by leg. The graph is built at server start and is IMMUTABLE at
//         runtime (map threads read it concurrently; live mutation is a
//         data race). No route on the map? Probe in bounded stretches
//         toward the goal, give up LOUDLY the first stretch that gains
//         nothing — quitting fast and visibly beats flailing forever;
//       - regression guard: a fresh plan that ends no closer to the goal
//         than the cached one loses to the cached one. A re-plan has to earn
//         its replacement;
//       - last resort: a single-point beeline, so something is always
//         attempted and failure surfaces as "no progress", never silence.
//     Planning only — ResolveMovePath reads lastMove and dispatches nothing.
//  9. TRIM IT.  (makeShortCut) Drop waypoints the bot can already walk
//     straight to; if trimming ate the whole path, bridge back to the
//     nearest reachable on-path anchor — a plan you can't stand on is not
//     a plan.
// 10. SPECIAL FIRST LEG?  (UpcomingSpecialMovement / HandleSpecialMovement)
//     When the next path element isn't plain ground it needs an
//     interaction, not a walk: click the portal, step through the area
//     trigger, board or leave the transport, talk to the flight master and
//     take the taxi. Each converts one special leg into game actions, then
//     the pipeline resumes on the far side. UpcomingSpecialMovement runs
//     first and cuts the path so the head IS the special leg —
//     HandleSpecialMovement only ever looks at the head, so the two are
//     always called as a pair.
// 11. SHIP LOGISTICS.  Approaching a dock: ship present → board; absent →
//     walk to the water's edge and hold. Ship schedules are slow, so
//     waiting is legitimate player behavior — indefinitely, and NEVER
//     "stuck".
// 12. DANGER ON THE PATH.  (ClipPath) Cut the walk short before hostile
//     camps unless the caller explicitly accepts the risk — stopping short
//     beats delivering the bot into a pull it didn't choose.
// 13. WATER SURFACE.  (surfaceSnapWaypoints) Swimmable waypoints snap to
//     the surface so swimming bots travel on top of the water, not along
//     floor-level path points.
// 14. DISPATCH.  (DispatchMovement) The only place legs actually move:
//     exactly one motion generator per dispatch (multi-point spline, or a
//     single-point fallback), the target + priority are recorded for the
//     dedup/replacement gates, and unwatched bots may teleport-advance
//     (logged). The AI is never put to sleep for movement.
//
// --- The rules the pipeline enforces ----------------------------------------
//
// REPLACEMENT (stage 3). Movement replacement is dictated by MovementPriority.
// A lower-priority action never replaces a walk still in progress — or a
// position hold (SetNextMovementDelay) still running; a higher or equal
// one does (newest wins). A hold is movement-only: the brain stays awake
// and keeps fighting. Movement never sleeps the brain; RPG idles pace
// themselves with awake rpgInfo timestamps instead.
//
// ALWAYS AWAKE. Walk movement never sleeps the AI. An early-out in the
// action absorbs re-issues and lower-priority calls instead: still moving
// toward a destination that hasn't meaningfully changed → nothing to do.
// Combat callers pass react=true to opt out of that early-out, so a chase
// re-aims every tick.
//
// CROSS-TRANSPORT FOLLOW. Following someone onto a different deck is not a
// walk — there is no navmesh spanning two independently moving transports.
// When bot and target are on different transports (or one is off-transport)
// and within sight, the bot disembarks, teleports to the target, and boards
// the target's transport, all in one tick, ahead of the engine-level follow.
//
// STUCK & RESCUE. Stuck = ~90 seconds of honest no-progress while NOT
// legitimately waiting. A botched map crossing teleports the
// bot to its intended arrival. Teleports are logged.
//
// MOVENEAR SEMANTICS. MoveNear(WorldObject*, dist) means TOUCH RANGE — dist
// is clamped to GetRange("follow"), so "near a thing" is walking-up-to-it
// distance regardless of what the caller passes. Callers needing a real
// standoff (spread mechanics: "stand 22y from the marked player") must use
// the coordinate overload MoveNear(mapId, x, y, z, dist), which applies the
// raw offset. One primitive per meaning, instead of one primitive whose
// behavior depends on a clamp the caller can't see.
// ============================================================================

#define ANGLE_90_DEG M_PI_2
#define ANGLE_120_DEG (2.f * static_cast<float>(M_PI) / 3.f)

// Default acceptable path types for GeneratePath
constexpr uint32 DEFAULT_PATH_ACCEPT_MASK = PATHFIND_NORMAL | PATHFIND_INCOMPLETE;
constexpr uint32 RELAXED_PATH_ACCEPT_MASK = PATHFIND_NORMAL | PATHFIND_INCOMPLETE | PATHFIND_FARFROMPOLY;

struct PathResult
{
    Movement::PointsArray points;
    G3D::Vector3 actualEnd;
    G3D::Vector3 end;
    PathType pathType;
    bool reachable;
};

class MovementAction : public Action
{
public:
    MovementAction(PlayerbotAI* botAI, std::string const name);

protected:
    // Emit a one-line trace describing the imminent movement. No-op
    // unless the bot has the "debug move" non-combat strategy.
    // Subclasses (e.g. NewRpgBaseAction) may override to append richer
    // context such as RPG status and target name. Optional `extra`
    // is appended verbatim (use it to attach hop labels like
    // "node:Stormwind innkeeper" or fallback reasons).
    virtual void EmitDebugMove(char const* method, char const* generator, float x, float y, float z, char const* extra = nullptr);

    bool JumpTo(uint32 mapId, float x, float y, float z, MovementPriority priority = MovementPriority::MOVEMENT_NORMAL);
    bool MoveNear(uint32 mapId, float x, float y, float z, float distance = sPlayerbotAIConfig.contactDistance,
                  MovementPriority priority = MovementPriority::MOVEMENT_NORMAL);
    bool MoveToLOS(WorldObject* target, bool ranged = false);
    bool MoveTo(uint32 mapId, float x, float y, float z, bool idle = false, bool react = false,
                bool normal_only = false, bool exact_waypoint = false,
                MovementPriority priority = MovementPriority::MOVEMENT_NORMAL, bool lessDelay = false,
                bool backwards = false, bool ignoreEnemyTargets = false);

    // Path-aware funnel mirroring the reference movement implementation.
    // Runs UpdateMovementState + IsMovingAllowed + WaitForTransport gates,
    // applies the targetPosRecalcDistance short-stop, resolves a TravelPath
    // via ResolveMovePath (which gates graph A* by sightDistance), trims
    // with makeShortCut, handles special head segments
    // (portal/area-trigger/transport/flight) via HandleSpecialMovement,
    // clips at hostile creatures via ClipPath (unless ignoreEnemyTargets),
    // and dispatches the resulting walk via DispatchMovement.
    // MoveTo(mapId,...) delegates here unless an intentional bypass
    // (exact_waypoint / disableMoveSplinePath / flying / swimming /
    // backwards) routes the move straight to DoMovePoint.
    // `react=true` opts the move out of the end-of-dispatch
    // WaitForReach AI-loop block — combat callers should set this so the
    // bot can keep re-evaluating mid-chase. Default false matches the
    // reference's MoveTo2 default.
    bool MoveTo2(WorldPosition endPos,
                 bool idle = false, bool react = false,
                 bool noPath = false, bool ignoreEnemyTargets = false,
                 MovementPriority priority = MovementPriority::MOVEMENT_NORMAL,
                 bool lessDelay = false);

    // Centralized walk dispatch. Mirrors the reference's DispatchMovement
    // shape: takes a TravelPath, builds the PointsArray internally,
    // applies inactive-bot teleport carve-out, masterWalking mode,
    // pre-dispatch state cleanup (clear emote, stand, interrupt cast),
    // transport-passenger coordinate sandwich
    // (CalculatePassengerPosition → UpdateAllowedPositionZ → Offset)
    // around the per-point Z snap, mm.Clear → MovePoint(last) →
    // MoveSplinePath. Caches the destination + duration on lastMove.
    //
    // Divergence from reference: reference ends with WaitForReach(size)
    // which blocks the AI loop until the move completes. AC's combat
    // callers (ReachCombatTo) currently funnel through MoveTo → MoveTo2
    // → DispatchMovement; blocking the AI loop here would suspend combat
    // re-evaluation for the full move duration. Until combat dispatch is
    // restructured to bypass MoveTo2, the WaitForReach is deliberately
    // omitted.
    // `react=true` skips the end-of-dispatch WaitForReach so the AI
    // loop isn't blocked while the spline plays — combat callers use
    // this to keep re-evaluating mid-chase.
    bool DispatchMovement(TravelPath path,
                          WorldPosition dest,
                          char const* label,
                          MovementPriority priority = MovementPriority::MOVEMENT_NORMAL,
                          bool lessDelay = false,
                          bool react = false);
    bool MoveTo(WorldObject* target, float distance = 0.0f,
                MovementPriority priority = MovementPriority::MOVEMENT_NORMAL);
    bool MoveNear(WorldObject* target, float distance = sPlayerbotAIConfig.contactDistance,
                  MovementPriority priority = MovementPriority::MOVEMENT_NORMAL);
    float GetFollowAngle();
    bool Follow(Unit* target, float distance = sPlayerbotAIConfig.followDistance);
    bool Follow(Unit* target, float distance, float angle);
    // Handles the cross-transport follow case: when bot and target are
    // on different transports (or one is off-transport) and within
    // sight, this disembarks the bot from its current transport (if
    // any), teleports it to the target's position, and boards the
    // target's transport (if any). Returns true if the transport
    // transition was performed this tick (caller should skip the
    // engine-level follow for this tick).
    bool FollowOnTransport(Unit* target);
    bool ChaseTo(WorldObject* obj, float distance = 0.0f);
    bool ReachCombatTo(Unit* target, float distance = 0.0f);
    float MoveDelay(float distance, bool backwards = false);
    void WaitForReach(float distance);
    // PointsArray overload: sums segment distances and calls the float
    // version. Matches the reference's WaitForReach(PointsArray) used at
    // the end of DispatchMovement.
    void WaitForReach(Movement::PointsArray const& path);
    void SetNextMovementDelay(float delayMillis);
    bool IsMovingAllowed(WorldObject* target);
    bool IsDuplicateMove(float x, float y, float z);
    bool IsMovingAllowed();
    bool Flee(Unit* target);
    void ClearIdleState();
    void UpdateMovementState();
    bool MoveAway(Unit* target, float distance = sPlayerbotAIConfig.fleeDistance, bool backwards = false);
    bool MoveFromGroup(float distance);
    bool Move(float angle, float distance);
    bool MoveInside(uint32 mapId, float x, float y, float z, float distance = sPlayerbotAIConfig.followDistance,
                    MovementPriority priority = MovementPriority::MOVEMENT_NORMAL);
    void CreateWp(Player* wpOwner, float x, float y, float z, float o, uint32 entry, bool important = false);
    Position BestPositionForMeleeToFlee(Position pos, float radius);
    Position BestPositionForRangedToFlee(Position pos, float radius);
    bool FleePosition(Position pos, float radius, uint32 minInterval = 1000);
    bool CheckLastFlee(float curAngle, std::list<FleeInfo>& infoList);

    PathResult GeneratePath(float x, float y, float z, uint32 acceptMask = DEFAULT_PATH_ACCEPT_MASK, bool forceDestination = false);

    // Returns a unified TravelPath for the move. Mirror of the reference
    // ResolveMovePath shape: 10% lastPath reuse short-circuit, choose
    // graph (cross-map / >sightDistance) or live mmap probe, regression
    // guard preferring cached path when no better, fall back to a
    // single-point path on dest. Stateless — does not dispatch.
    TravelPath ResolveMovePath(WorldPosition startPos,
                               WorldPosition endPos,
                               LastMovement& lastMove);

    // Dispatches the head-of-path special segment (portal interact /
    // area-trigger marker / transport boarding / flight master taxi).
    // Caller is expected to first call TravelPath::UpcommingSpecialMovement
    // which cuts the path so the head is the special segment. Returns
    // true if a movement-consuming action was dispatched this tick.
    // Returns false for AREA_TRIGGER-with-entry (caller still dispatches
    // the walk into the trigger volume).
    bool HandleSpecialMovement(TravelPath& path);

    // Top-of-MoveFarTo gate that keeps a bot riding a transport across
    // ticks. Returns true if the bot is still on the transport we last
    // boarded (caller should skip the rest of MoveFarTo this tick).
    // Clears lastTransportEntry and returns false if the bot has
    // disembarked or is no longer on the expected transport.
    bool WaitForTransport();

    // Transport boarding helpers (shared by FollowAction and travel plan)
    static Transport* GetTransportForPosTolerant(Map* map, WorldObject* ref,
        uint32 phaseMask, float x, float y, float z);
    static bool FindBoardingPointOnTransport(Map* map, Transport* transport,
        WorldObject* ref, float refX, float refY, float refZ,
        float botX, float botY, float botZ,
        float& outX, float& outY, float& outZ);
    bool BoardTransport(Transport* transport);

protected:
    struct CheckAngle
    {
        float angle;
        bool strict;
    };

private:
    bool wasMovementRestricted = false;
    void DoMovePoint(Unit* unit, float x, float y, float z, bool generatePath, bool backwards);
};

class FleeAction : public MovementAction
{
public:
    FleeAction(PlayerbotAI* botAI, float distance = sPlayerbotAIConfig.spellDistance)
        : MovementAction(botAI, "flee"), distance(distance)
    {
    }

    bool Execute(Event event) override;
    bool isUseful() override;

private:
    float distance;
};

class FleeWithPetAction : public MovementAction
{
public:
    FleeWithPetAction(PlayerbotAI* botAI) : MovementAction(botAI, "flee with pet") {}

    bool Execute(Event event) override;
};

class AvoidAoeAction : public MovementAction
{
public:
    AvoidAoeAction(PlayerbotAI* botAI, int moveInterval = 1000)
        : MovementAction(botAI, "avoid aoe"), moveInterval(moveInterval)
    {
    }

    bool isUseful() override;
    bool Execute(Event event) override;

protected:
    bool AvoidAuraWithDynamicObj();
    bool AvoidGameObjectWithDamage();
    bool AvoidUnitWithDamageAura();
    time_t lastTellTimer = 0;
    int lastMoveTimer = 0;
    int moveInterval;
};

class CombatFormationMoveAction : public MovementAction
{
public:
    CombatFormationMoveAction(PlayerbotAI* botAI, std::string name = "combat formation move", int moveInterval = 1000)
        : MovementAction(botAI, name), moveInterval(moveInterval)
    {
    }

    bool isUseful() override;
    bool Execute(Event event) override;

protected:
    Position AverageGroupPos(float dis = sPlayerbotAIConfig.sightDistance, bool ranged = false, bool self = false);
    Player* NearestGroupMember(float dis = sPlayerbotAIConfig.sightDistance);
    float AverageGroupAngle(Unit* from, bool ranged = false, bool self = false);
    Position GetNearestPosition(const std::vector<Position>& positions);
    int lastMoveTimer = 0;
    int moveInterval;
};

class TankFaceAction : public CombatFormationMoveAction
{
public:
    TankFaceAction(PlayerbotAI* botAI) : CombatFormationMoveAction(botAI, "tank face") {}

    bool Execute(Event event) override;
};

class RearFlankAction : public MovementAction
{
    // 90 degree minimum angle prevents any frontal cleaves/breaths and avoids parry-hasting the boss.
    // 120 degree maximum angle leaves a 120 degree symmetrical cone at the tail end which is usually enough to avoid
    // tail swipes. Some dragons or mobs may have different danger zone angles, override if needed.
public:
    RearFlankAction(PlayerbotAI* botAI, float distance = 0.0f, float minAngle = ANGLE_90_DEG,
                    float maxAngle = ANGLE_120_DEG)
        : MovementAction(botAI, "rear flank")
    {
        this->distance = distance;
        this->minAngle = minAngle;
        this->maxAngle = maxAngle;
    }

    bool Execute(Event event) override;
    bool isUseful() override;

protected:
    float distance, minAngle, maxAngle;
};

class DisperseSetAction : public Action
{
public:
    DisperseSetAction(PlayerbotAI* botAI, std::string const name = "disperse set") : Action(botAI, name) {}

    bool Execute(Event event) override;
    float DEFAULT_DISPERSE_DISTANCE_RANGED = 5.0f;
    float DEFAULT_DISPERSE_DISTANCE_MELEE = 2.0f;
};

class RunAwayAction : public MovementAction
{
public:
    RunAwayAction(PlayerbotAI* botAI) : MovementAction(botAI, "runaway") {}

    bool Execute(Event event) override;
};

class MoveToLootAction : public MovementAction
{
public:
    MoveToLootAction(PlayerbotAI* botAI) : MovementAction(botAI, "move to loot") {}

    bool Execute(Event event) override;
};

class MoveOutOfEnemyContactAction : public MovementAction
{
public:
    MoveOutOfEnemyContactAction(PlayerbotAI* botAI) : MovementAction(botAI, "move out of enemy contact") {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

class SetFacingTargetAction : public Action
{
public:
    SetFacingTargetAction(PlayerbotAI* botAI) : Action(botAI, "set facing") {}

    bool Execute(Event event) override;
    bool isUseful() override;
    bool isPossible() override;
};

class SetBehindTargetAction : public CombatFormationMoveAction
{
public:
    SetBehindTargetAction(PlayerbotAI* botAI) : CombatFormationMoveAction(botAI, "set behind") {}

    bool Execute(Event event) override;
};

class MoveOutOfCollisionAction : public MovementAction
{
public:
    MoveOutOfCollisionAction(PlayerbotAI* botAI) : MovementAction(botAI, "move out of collision") {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

class MoveRandomAction : public MovementAction
{
public:
    MoveRandomAction(PlayerbotAI* botAI) : MovementAction(botAI, "move random") {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

class MoveInsideAction : public MovementAction
{
public:
    MoveInsideAction(PlayerbotAI* ai, float x, float y, float distance = 5.0f) : MovementAction(ai, "move inside")
    {
        this->x = x;
        this->y = y;
        this->distance = distance;
    }
    virtual bool Execute(Event event);

protected:
    float x, y, distance;
};

class RotateAroundTheCenterPointAction : public MovementAction
{
public:
    RotateAroundTheCenterPointAction(PlayerbotAI* ai, std::string name, float center_x, float center_y,
                                     float radius = 40.0f, uint32 intervals = 16, bool clockwise = true,
                                     float start_angle = 0)
        : MovementAction(ai, name)
    {
        this->center_x = center_x;
        this->center_y = center_y;
        this->radius = radius;
        this->intervals = intervals;
        this->clockwise = clockwise;
        this->call_counters = 0;
        for (uint32 i = 0; i < intervals; i++)
        {
            float angle = start_angle + 2 * M_PI * i / intervals;
            waypoints.push_back(std::make_pair(center_x + cos(angle) * radius, center_y + sin(angle) * radius));
        }
    }
    virtual bool Execute(Event event);

protected:
    virtual uint32 GetCurrWaypoint() { return 0; }
    uint32 FindNearestWaypoint();
    float center_x, center_y, radius;
    uint32 intervals, call_counters;
    bool clockwise;
    std::vector<std::pair<float, float>> waypoints;
};

class MoveFromGroupAction : public MovementAction
{
public:
    MoveFromGroupAction(PlayerbotAI* botAI, std::string const name = "move from group") : MovementAction(botAI, name) {}

    bool Execute(Event event) override;
};

class MoveAwayFromCreatureAction : public MovementAction
{
public:
    MoveAwayFromCreatureAction(PlayerbotAI* botAI, std::string name, uint32 creatureId, float range, bool alive = true)
        : MovementAction(botAI, name), creatureId(creatureId), range(range), alive(alive)
    {
    }

    bool Execute(Event event) override;
    bool isPossible() override;

private:
    uint32 creatureId;
    float range;
    bool alive;
};

class MoveAwayFromPlayerWithDebuffAction : public MovementAction
{
public:
    MoveAwayFromPlayerWithDebuffAction(PlayerbotAI* botAI, std::string name, uint32 spellId, float range)
        : MovementAction(botAI, name), spellId(spellId), range(range)
    {
    }

    bool Execute(Event event) override;
    bool isPossible() override;

private:
    uint32 spellId;
    float range;
};

#endif
