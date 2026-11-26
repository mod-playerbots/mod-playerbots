/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <span>
#include <vector>

#define LOG_INFO(...)

using uint32 = unsigned;
constexpr float CornerArrival = 1.0f;
constexpr float MovementClearance = 2.0f;
constexpr float DirectionTolerance = 1.0f;
constexpr uint32 WayOutRetryMs = 500;
enum CurrentSpellTypes
{
    CURRENT_GENERIC_SPELL,
    CURRENT_CHANNELED_SPELL
};

struct Position
{
    float x{}, y{}, z{};
    Position operator-(Position const& other) const { return {x - other.x, y - other.y, z - other.z}; }
    float length() const { return std::hypot(x, y, z); }
    float GetExactDist(Position const& other) const { return (*this - other).length(); }
};

namespace G3D
{
using Vector3 = Position;
}
namespace Movement
{
using PointsArray = std::vector<Position>;
}

Position const TransitionCenter{30, 0, 0};
std::array<Position, 2> const Waypoints{{{10, 0, 0}, {20, 0, 0}}};
Position const Blocked{};
Position const& Point(Position const& position) { return position; }
bool InsideHorsemenEntrance(Position const&) { return false; }
bool InsideHorsemenRoom(Position const&) { return true; }
uint32 getMSTime() { return 100; }
uint32 getMSTimeDiff(uint32 then, uint32 now) { return now - then; }
bool StartsToward(Movement::PointsArray const&, Position const&, Position const&) { return true; }

struct Spell
{
    Spell const* GetSpellInfo() const { return this; }
    bool IsPositive() const { return true; }
};

struct Player
{
    uint32 id = 42;
    unsigned stops = 0;
    unsigned launches = 0;
    Movement::PointsArray route{{}, TransitionCenter};
    Player* movespline = this;
    uint32 GetId() const { return id; }
    bool isMoving() const { return !route.empty(); }
    void StopMoving()
    {
        ++stops;
        route.clear();
    }
    Position GetPosition() const { return {}; }
    float GetExactDist(Position const& goal) const { return GetPosition().GetExactDist(goal); }
    Player* GetMotionMaster() { return this; }
    void Clear() {}
    Spell const* GetCurrentSpell(int) const { return nullptr; }
};

Position CurrentPoint(Player const& bot) { return bot.GetPosition(); }
Movement::PointsArray RemainingSpline(Player const& bot, uint32 id)
{
    return bot.GetId() == id ? bot.route : Movement::PointsArray{};
}

struct PlayerbotAI
{
    static bool IsHeal(Player const*) { return false; }
    bool CanMove() const { return true; }
};

struct WalkCheck
{
    WalkCheck(PlayerbotAI*, Player&) {}
    bool Pulls(Movement::PointsArray const&) const { return false; }
};

struct FourHorsemen40Hazards
{
    PlayerbotAI ai;
    PlayerbotAI* _ai = &ai;
    uint32 _fixedPointSpline = 42;
    uint32 _centerRetreatSpline = 0;
    uint32 _fixedPointRetryAt = 0;
    bool _blockedFixedCenterward = false;
    bool _fixedPointCenterward = true;
    Position _blockedFixedGoal{};
    bool unsafeDirect = false;
    bool safeDetour = false;
    bool inExitBranch = false;
    bool triedEscape = false;

    bool WalkToFixedPoint(Player&, Position const&, float, bool, std::span<Position const> = {});
    bool ContinueCenterRetreat(Player&, bool, bool&) { return false; }
    bool HasActiveRearExchangeExit(Player&) { return inExitBranch; }
    bool IsSafeFromVoidDamage(Player&, Position const&) { return true; }
    bool IsEscapingRearExchangeVoidZone(Player&) { return false; }
    bool IsSafeFromVoidZones(Player&, Position const&) { return true; }
    bool EscapeRearExchangeVoidZone(Player&, Position const*)
    {
        triedEscape = true;
        return false;
    }
    bool WalkRear(Player&, Position const&, float, bool, float, std::span<int const>, Position const*, bool,
                  std::span<Position const>)
    {
        return false;
    }
    void RefreshVoidZones(Player&) {}
    float Clearance(Position const&) { return MovementClearance; }
    bool GroundLeg(Player&, Position const& from, Position const& goal, Movement::PointsArray& path, bool)
    {
        if (goal.x != TransitionCenter.x)
            return false;
        path = {from, goal};
        return true;
    }
    Position const* FirstBlockedZone(Player&, Movement::PointsArray const& path)
    {
        return unsafeDirect && path.back().x == TransitionCenter.x && path.size() == 2 ? &Blocked : nullptr;
    }
    bool FindVoidDetour(Player&, Position const&, Position const&, Position const&, std::span<int const>,
                        Movement::PointsArray&, bool)
    {
        return false;
    }
    bool FindCenterwardDetour(Player&, Position const& from, Position const& goal, float, Movement::PointsArray& path)
    {
        if (!safeDetour || goal.x != TransitionCenter.x)
            return false;
        path = {from, {15, 5, 0}, goal};
        return true;
    }
    std::array<int, 1> RearExchangeCrossingBlock(Player&, Movement::PointsArray const&, std::array<bool, 1>, bool&)
    {
        return {1};
    }
    bool FindCenterRetreatStep(Player&, Position const&, Movement::PointsArray&) { return false; }
    bool Launch(Player& bot, Movement::PointsArray const& path, uint32& id)
    {
        ++bot.launches;
        id = ++bot.id;
        bot.route = path;
        return true;
    }
};

// FIXED_POINT

struct CenterMovement
{
    Player player;
    Player* bot = &player;
    FourHorsemen40Hazards _hazards;
    bool _centerRouteBlocked = false;
    struct
    {
        bool centerRetreat = true;
        bool returningFromZeliek = true;
        std::array<Position, 2> const* zeliekLane = &Waypoints;
    } _tick;

    std::span<Position const> ZeliekRetreatLane() const { return Waypoints; }

    // CENTER_DECISION
};

int main()
{
    int failures = 0;
    auto const check = [&](bool pass, char const* label)
    {
        if (!pass)
            ++failures;
        std::cout << (pass ? "PASS " : "FAIL ") << label << '\n';
    };

    for (bool activeExit : {false, true})
    {
        std::cout << (activeExit ? "Active exit branch\n" : "Normal retreat branch\n");
        CenterMovement direct;
        direct._hazards.inExitBranch = activeExit;
        for (int decision = 0; decision < 3; ++decision)
            check(direct.MoveThroughCenter() && direct.player.stops == 0 && direct.player.launches == 0 &&
                      direct.player.GetId() == 42,
                  "blocked waypoints preserve the direct spline ID");

        CenterMovement alternate;
        alternate._hazards.inExitBranch = activeExit;
        alternate.player.route.back() = Waypoints.back();
        check(alternate.MoveThroughCenter() && alternate.player.stops == 0 && alternate.player.launches == 0 &&
                  alternate.player.GetId() == 42,
              "blocked first waypoint preserves the active second waypoint");

        CenterMovement unsafe;
        unsafe._hazards.inExitBranch = activeExit;
        check(unsafe.MoveThroughCenter(), "direct spline is initially usable");
        uint32 const previousSpline = unsafe.player.GetId();
        unsigned const previousStops = unsafe.player.stops;
        unsigned const previousLaunches = unsafe.player.launches;
        unsafe._hazards.unsafeDirect = unsafe._hazards.safeDetour = true;
        check(unsafe.MoveThroughCenter() && unsafe.player.stops == previousStops + 1 &&
                  unsafe.player.launches == previousLaunches + 1 && unsafe.player.GetId() != previousSpline &&
                  unsafe.player.route.size() == 3,
              "new hazard stops the old spline and allows a safe detour");

        CenterMovement blocked;
        blocked._hazards.inExitBranch = activeExit;
        blocked._hazards.unsafeDirect = true;
        check(!blocked.MoveThroughCenter() && blocked.player.stops == 1 && blocked.player.launches == 0 &&
                  !blocked.player.isMoving() && blocked._hazards.triedEscape == activeExit,
              "no safe route stops movement and follows the exit fallback when applicable");
    }
    return failures ? 1 : 0;
}
