/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RAID_TOCANUBARAKKITEPLANNER_H
#define PLAYERBOTS_RAID_TOCANUBARAKKITEPLANNER_H

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <vector>

#include "ToCAnubarakPursuit.h"

// One kite's path and how it ends, picked from a few hundred candidates each judged against the spike's
// pure pursuit
namespace AnubarakKitePlanner
{
using AnubarakPursuit::DEG_TO_RAD;
using AnubarakPursuit::Dist;
using AnubarakPursuit::Pace;
using AnubarakPursuit::PatchMove;
using AnubarakPursuit::Point;
using AnubarakPursuit::Spike;
using AnubarakPursuit::TICK_S;

enum class Ending : uint8_t
{
    None,
    Patch,
    // Hand of Protection at the catch, then into the patch under it
    PatchHand,
    Hand,
    // Divine Shield, Ice Block, Vanish or Feign Death: each hands the spike on at its speed
    Own
};

// Worst first when no path is clean
enum class Hazard : uint8_t
{
    None,
    Tank,
    Kite,
    Raid,
    Catch
};

enum class Shape : uint8_t
{
    Ring,
    Arc,
    Held,
    Motion
};

struct Patch
{
    Point pos;
    bool tank = false;
    // The kite may end on it
    bool usable = false;
    // Still coming down: the spike fails nearer, and nothing is slowed yet
    bool falling = false;
    uint64_t id = 0;
};

struct HandOffer
{
    uint64_t id = 0;
    Point pos;
    // The kiter's own, which lands anywhere at once
    bool self = false;
    // Back in time for a later submerge that needs it
    bool spare = true;
};

struct KiteWorld
{
    Point kiter;
    Pace pace;
    // A speed boost the kiter can fire, the plan saying when: its run speed and seconds, 0 for none
    float readyBoostSpeed = 0.0f;
    float readyBoostS = 0.0f;
    Spike spike;
    // Seconds to the emerge, which despawns the spike
    float lifeS = 60.0f;
    std::vector<Patch> patches;
    bool gather = false;
    Point gatherPos;
    // Players outside the gather, the kiter left out
    std::vector<Point> bystanders;
    float slowFactor = 0.2f;
    // No catch counts before this: the base speed spike's first test
    float catchFromS = 0.0f;
    // Hand of Protection on the kiter now: no catch counts while it holds, and no other Hand lands
    float handLeftS = 0.0f;
    // Paladins that can land a Hand, spare ones first, then the one to keep
    std::vector<HandOffer> hands;
    // The spare offers
    int spareHands = 0;
    // Patches the spike's life can still be ended on, this kite's included
    int patchBudget = 0;
    // Heroic: a kite patch ending costs costReservedPatch, since a later submerge needs it
    bool reservePatch = false;
    bool own = false;
    // False for a paladin: a Hand's Forbearance blocks its Divine Shield
    bool ownAfterHand = true;
    bool heroic = true;
    // Normal: a burrower alive makes a tank patch the last resort, since it submerges back to full off it
    bool burrowers = false;
    // A warlock's Demonic Circle: Teleport ready, and where its circle stands
    bool teleport = false;
    Point circle;
};

struct Rules
{
    Point centre = {745.0f, 135.0f};
    // Walkable all round
    float floorRadius = 45.0f;
    float ring = 40.0f;
    float rim = 45.0f;
    int headings = 16;
    float arcStraightS = 1.5f;
    std::array<float, 7> arcTurns = {-40.0f, -24.0f, -12.0f, 0.0f, 12.0f, 24.0f, 40.0f};
    float horizonS = 25.0f;
    float maxHorizonS = 30.0f;
    // The spike's 8 yd sphere test plus both sizes
    float patchReach = 11.5f;
    float fallingReach = 10.5f;
    float impaleReach = 7.5f;
    float slowReach = 7.5f;
    float edgeRadius = 8.5f;
    float edgeRunOn = 15.0f;
    float gatherClearance = 20.0f;
    float bystanderClearance = 10.0f;
    // Something the spike starts inside of passes while it gets no closer than this past the start
    float startSlack = 0.5f;
    // Its patch test comes every 1.5, 0.8 and 0.4 s by speed step, so a spike restarted inside a
    // patch's reach can still be led out before it fails
    std::array<float, 3> testS = {1.5f, 0.8f, 0.4f};
    int endingShapes = 6;
    // Per usable patch, the shapes passing within patchNear of it latest get its endings too
    int nearShapes = 3;
    float patchNear = 15.0f;
    // Heroic: a kite patch spent before the spike has chased this long since its last restart counts as
    // spent early, planned or not, since its reset buys little
    float earlyPatchS = 12.0f;
    float patchWindowS = 15.0f;
    float edgeWindowS = 6.0f;
    float turnStepS = 1.0f;
    float handLeadS = 1.0f;
    float ownLeadS = 0.6f;
    float handS = 10.0f;
    // Plans count a Hand as running out this much early and Impale as reaching this much farther, so a
    // patch walk planned to the tick still makes it with the game's timing off a little
    float handMarginS = 1.0f;
    float catchMargin = 1.0f;
    // Impale doesn't hand the spike on, so it keeps after a caught kiter: a patch it fails on this soon
    // after the catch still counts against the run, the planned one only before the second Impale
    float postCatchS = 3.0f;
    float handRange = 30.0f;
    float paladinSpeed = 7.0f;
    // The spike idles this long after a patch, then marks someone fresh at its base speed
    float idleS = 4.0f;
    // Its last chase spline keeps it walking on through the idle, up to this far, so a patch that close to the
    // walk is spent before the next mark starts
    float driftReach = 17.0f;
    // A fresh kite runs about this long, one handed a spike already fast this long
    float freshS = 15.0f;
    float fastS = 6.0f;
    // A ready boost fires once a spike faster than the kiter's run comes this close
    float boostRange = 15.0f;
    // An immunity hands the spike on at its speed, so it costs costOwnMid unless the spike's life is
    // nearly out by then
    float ownLateS = 6.0f;
    float costOwn = 1.0f;
    float costOwnMid = 3.0f;
    float costPatch = 2.0f;
    // Over a Hand and an own immunity together: only a catch or a hazard loses to it
    float costReservedPatch = 8.0f;
    float costHand = 2.0f;
    float costLastHand = 3.0f;
    float costNormalPatch = 0.5f;
    float costNormalTankPatch = 10.0f;
    float hazardBandS = 1.0f;
    float uncoveredBandS = 1.0f;
    float endBandS = 1.0f;
    float gatherBand = 5.0f;
    float marginCap = 20.0f;
    float marginBand = 1.0f;
    // The held plan stays unless another beats it on hazards, coverage or cost, or ends this much later
    float keepEndS = 1.0f;
    // A turn this sharp waits until the held plan is this old: re-picked freely, the kiter stood
    // between two headings at the rim
    float turnWaitDegrees = 90.0f;
    float turnWaitS = 1.0f;
    // Demonic Circle: Teleport's 40 yd less a margin. It only pays onto a circle this much farther from the spike
    // than the kiter.
    float teleportRange = 38.0f;
    float teleportGain = 15.0f;
    // With the teleport ready, a ring this far round the centre keeps a run in range
    float teleportRing = 36.0f;
    // After the jump the kiter runs on away from the spike, or turned off that by these
    std::array<float, 5> teleportHeadings = {0.0f, 45.0f, -45.0f, 90.0f, -90.0f};
    std::array<float, 3> teleportArcTurns = {-24.0f, 0.0f, 24.0f};
    // Per base, the teleport runs given endings
    int teleportShapes = 2;
};

struct Candidate
{
    Shape shape = Shape::Ring;
    float headingDeg = 0.0f;
    // Ring: the circle's radius and the way round, +1 counter-clockwise; arc: the turn in deg/s
    float radius = 0.0f;
    float turn = 0.0f;
    Ending ending = Ending::None;
    // Index into the world's patches, -1 for none
    int patch = -1;
    PatchMove move = PatchMove::In;
    // Tick the walk onto the patch starts
    int turnTick = -1;
    // Tick the ready boost fires, -1 for none
    int boostTick = -1;
    // Tick the kiter teleports to the circle, there the tick after, -1 for none
    int teleportTick = -1;
    bool hand = false;
    bool own = false;
    // Where the kiter is every TICK_S from now
    std::vector<Point> track;
};

struct Outcome
{
    // What ended it: a planned Hand or immunity unused shows as the ending reached instead
    Ending ending = Ending::None;
    // Heroic: it ends on the kite patch it planned
    bool kitePatch = false;
    // It spends the Hand kept for a later submerge
    bool lastHand = false;
    float endS = 0.0f;
    // First second of each hazard, negative for none
    float tankS = -1.0f;
    float kiteS = -1.0f;
    float raidS = -1.0f;
    bool caught = false;
    // Of the spike's life after this kite, what the patches and Hands left can't cover
    float uncoveredS = 0.0f;
    float cost = 0.0f;
    float margin = 0.0f;
    Point endPoint;
    // From the gather, negative unless the kite ends on an immunity or a catch
    float endGather = -1.0f;
    // The catch the Hand or immunity is timed on, negative for none
    float catchS = -1.0f;
    Point catchPoint;
    float handS = -1.0f;
    uint64_t handId = 0;
    float ownS = -1.0f;
    float turnS = -1.0f;
    // The ready boost fires by the end, negative when it doesn't
    float boostS = -1.0f;
    float teleportS = -1.0f;
};

struct Plan
{
    bool valid = false;
    bool kept = false;
    int judged = 0;
    Candidate candidate;
    Outcome outcome;
};

namespace Detail
{
inline Point Lerp(Point a, Point b, float f) { return {a.x + (b.x - a.x) * f, a.y + (b.y - a.y) * f}; }

inline Point OnFloor(Point p, Rules const& r)
{
    float const d = Dist(p, r.centre);
    if (d <= r.floorRadius)
        return p;

    return Lerp(r.centre, p, r.floorRadius / d);
}

inline float SlowAt(Point at, KiteWorld const& w, Rules const& r)
{
    if (w.slowFactor >= 1.0f)
        return 1.0f;

    for (Patch const& patch : w.patches)
        if (!patch.falling && Dist(at, patch.pos) < r.slowReach)
            return w.slowFactor;

    return 1.0f;
}

// To the spike's despawn when that comes by maxHorizonS, else to horizonS
inline int Ticks(KiteWorld const& w, Rules const& r)
{
    float const span = w.lifeS <= r.maxHorizonS ? std::max(w.lifeS, TICK_S) : r.horizonS;
    return static_cast<int>(std::ceil(span / TICK_S - 0.001f));
}

// The kiter's pace with the ready boost fired at boostTick, the world's own for -1
inline Pace CandidatePace(KiteWorld const& w, int boostTick)
{
    if (boostTick < 0)
        return w.pace;

    return Pace(w.pace.speed, w.readyBoostSpeed, w.readyBoostS, static_cast<float>(boostTick) * TICK_S);
}

// Extends track to ticks + 1 points, each step its length by step(at, length, t)
template <typename Step>
void Walk(std::vector<Point>& track, int ticks, Pace const& pace, KiteWorld const& w, Rules const& r, Step&& step)
{
    track.reserve(static_cast<size_t>(ticks) + 1);
    Point at = track.back();
    for (int i = static_cast<int>(track.size()) - 1; i < ticks; ++i)
    {
        float const t = static_cast<float>(i) * TICK_S;
        float const length = pace.SpeedAt(t) * TICK_S * SlowAt(at, w, r);
        at = OnFloor(step(at, length, t), r);
        track.push_back(at);
    }
}

inline Point AtLength(AnubarakPursuit::KiterPath const& path, float s)
{
    if (s <= path.straight || path.radius <= 0.0f)
        return {path.start.x + path.dir.x * s, path.start.y + path.dir.y * s};

    Point const hit = {path.start.x + path.dir.x * path.straight, path.start.y + path.dir.y * path.straight};
    float const angle =
        std::atan2(hit.y - path.centre.y, hit.x - path.centre.x) + path.turn * (s - path.straight) / path.radius;
    return {path.centre.x + path.radius * std::cos(angle), path.centre.y + path.radius * std::sin(angle)};
}

// Straight on a heading, turning turnDeg a second after straightS; at the rim it slides along it the way it
// turns, or the way nearer its heading
struct ArcStep
{
    Point heading;
    float turnDeg = 0.0f;
    float straightS = 0.0f;
    Rules const* rules = nullptr;

    Point operator()(Point at, float length, float t)
    {
        if (t >= straightS && turnDeg != 0.0f)
        {
            float const w = turnDeg * DEG_TO_RAD * TICK_S;
            float const c = std::cos(w);
            float const s = std::sin(w);
            heading = {heading.x * c - heading.y * s, heading.x * s + heading.y * c};
        }

        Point next = {at.x + heading.x * length, at.y + heading.y * length};
        Point const centre = rules->centre;
        float const out = Dist(at, centre);
        if (Dist(next, centre) <= rules->floorRadius || out < 0.01f)
            return next;

        Point const u = {(at.x - centre.x) / out, (at.y - centre.y) / out};
        Point const ccw = {-u.y, u.x};
        Point const cw = {u.y, -u.x};
        if (turnDeg > 0.0f)
            heading = ccw;
        else if (turnDeg < 0.0f)
            heading = cw;
        else
            heading = ccw.x * heading.x + ccw.y * heading.y >= cw.x * heading.x + cw.y * heading.y ? ccw : cw;

        return {at.x + heading.x * length, at.y + heading.y * length};
    }
};

// Through each leg's end in turn, then stands on the last
struct LegStep
{
    std::vector<Point> const* legs = nullptr;
    size_t next = 0;

    Point operator()(Point at, float length, float)
    {
        while (next < legs->size() && length > 0.0f)
        {
            Point const to = (*legs)[next];
            float const d = Dist(at, to);
            if (length < d)
                return Lerp(at, to, length / d);

            length -= d;
            at = to;
            ++next;
        }

        return at;
    }
};

inline float Band(float value, float band) { return std::floor(value / band); }

// What a run passes and where its spike is each tick
struct Watch
{
    std::vector<float> reach;
    std::vector<uint8_t> startInside;
    int graceTicks = 0;
    float gatherNeed = 0.0f;
    std::vector<float> bystanderNeed;
};

inline Watch MakeWatch(KiteWorld const& w, Rules const& r)
{
    Watch watch;
    for (Patch const& patch : w.patches)
    {
        float const reach = patch.falling ? r.fallingReach : r.patchReach;
        watch.reach.push_back(reach);
        watch.startInside.push_back(Dist(w.spike.pos, patch.pos) <= reach ? 1 : 0);
    }

    int const step = std::clamp(w.spike.step, 0, AnubarakPursuit::LAST_STEP);
    watch.graceTicks = static_cast<int>(std::ceil(r.testS[static_cast<size_t>(step)] / TICK_S - 0.001f));
    if (w.gather)
        watch.gatherNeed = std::min(r.gatherClearance, Dist(w.spike.pos, w.gatherPos) - r.startSlack);

    for (Point const& player : w.bystanders)
        watch.bystanderNeed.push_back(std::min(r.bystanderClearance, Dist(w.spike.pos, player) - r.startSlack));

    return watch;
}

struct Run
{
    std::vector<Spike> spikes;
    // The first patch the spike fails on, and the tick
    int failTick = -1;
    int failPatch = -1;
    bool failPlanned = false;
    int raidTick = -1;
    // Per patch it started inside, the tick it first left, -1 while it hasn't
    std::vector<int> leftTick;
    // Per tick from tick 0, the closest the spike came to a patch or the gather past its clearance
    std::vector<float> margin;
};

// One tick of the chase, from tick i to i + 1, track holding i + 1 already. planned fails from turnTick
// on end the kite as planned. False once the spike fails on a patch.
inline bool ChaseOne(Run& run, std::vector<Point> const& track, int i, int planned, int turnTick, KiteWorld const& w,
                     Rules const& r, Watch const& watch)
{
    Spike spike = run.spikes.back();
    AnubarakPursuit::ChaseTick(spike, track[static_cast<size_t>(i)], static_cast<float>(i) * TICK_S);
    int const tick = i + 1;
    run.spikes.push_back(spike);

    float margin = std::min(run.margin.back(), r.marginCap);
    for (size_t k = 0; k < w.patches.size(); ++k)
    {
        float const d = Dist(spike.pos, w.patches[k].pos);
        bool const inside = d <= watch.reach[k];
        if (static_cast<int>(k) != planned || tick < turnTick)
            margin = std::min(margin, d - watch.reach[k]);

        if (watch.startInside[k] && !inside && run.leftTick[k] < 0)
            run.leftTick[k] = tick;

        bool const counts = !watch.startInside[k] || run.leftTick[k] >= 0 || tick >= watch.graceTicks;
        if (inside && counts && run.failTick < 0)
        {
            run.failTick = tick;
            run.failPatch = static_cast<int>(k);
            run.failPlanned = static_cast<int>(k) == planned && tick >= turnTick;
        }
    }

    if (w.gather)
    {
        float const d = Dist(spike.pos, w.gatherPos);
        margin = std::min(margin, d - r.gatherClearance);
        if (run.raidTick < 0 && d < watch.gatherNeed)
            run.raidTick = tick;
    }

    for (size_t k = 0; run.raidTick < 0 && k < w.bystanders.size(); ++k)
        if (Dist(spike.pos, w.bystanders[k]) < watch.bystanderNeed[k])
            run.raidTick = tick;

    run.margin.push_back(margin);
    return run.failTick < 0;
}

// The spike along track from tick from up to the first patch it fails on or ticks
inline void Chase(Run& run, std::vector<Point> const& track, int from, int ticks, int planned, int turnTick,
                  KiteWorld const& w, Rules const& r, Watch const& watch)
{
    for (int i = from; i < ticks && ChaseOne(run, track, i, planned, turnTick, w, r, watch); ++i)
    {
    }
}

inline Run StartRun(KiteWorld const& w, Rules const& r)
{
    Run run;
    run.spikes.push_back(w.spike);
    run.leftTick.assign(w.patches.size(), -1);
    run.margin.push_back(r.marginCap);
    return run;
}

inline int ToTick(float s) { return static_cast<int>(std::lround(s / TICK_S)); }

// Which ticks a catch counts on: from the first test, outside the Hand on the kiter now and a planned one
struct CatchWindow
{
    int from = 0;
    int handTo = -1;
    int skipFrom = -1;
    int skipTo = -2;

    bool Counts(int tick) const { return tick >= from && tick > handTo && (tick < skipFrom || tick > skipTo); }
};

inline CatchWindow MakeCatchWindow(KiteWorld const& w, Rules const& r)
{
    CatchWindow window;
    window.from = static_cast<int>(std::ceil(w.catchFromS / TICK_S - 0.001f));
    if (w.handLeftS > r.handMarginS)
        window.handTo = ToTick(w.handLeftS - r.handMarginS);

    return window;
}

// A Hand landed at tick land covers the catch to a margin short of its end
inline void PlanHand(CatchWindow& window, int land, Rules const& r)
{
    window.skipFrom = land;
    window.skipTo = land + ToTick(r.handS - r.handMarginS);
}

inline bool Caught(Point spike, Point kiter, Rules const& r) { return Dist(spike, kiter) <= r.impaleReach + r.catchMargin; }

// Whether run's fail still counts after a catch at catchTick, atCatch the spike then. It Impales a caught kiter
// again each testS, so the planned patch only saves the kite before the second hit
inline bool FailCountsAfterCatch(Run const& run, int catchTick, Spike const& atCatch, Rules const& r)
{
    int const late = run.failTick - catchTick;
    int const step = std::clamp(atCatch.step, 0, AnubarakPursuit::LAST_STEP);
    return late <= ToTick(r.postCatchS) && (!run.failPlanned || late < ToTick(r.testS[static_cast<size_t>(step)]));
}

// First tick from from to before last the catch counts on
inline int FirstCatch(Run const& run, std::vector<Point> const& track, int from, int last, CatchWindow const& window,
                      Rules const& r)
{
    for (int i = std::max(from, 0); i < last && i < static_cast<int>(run.spikes.size()); ++i)
        if (window.Counts(i) && Caught(run.spikes[static_cast<size_t>(i)].pos, track[static_cast<size_t>(i)], r))
            return i;

    return -1;
}

// Seconds since the spike last restarted at its base speed, read off its speed step
inline float SpikeAgeS(Spike const& spike)
{
    float const step = AnubarakPursuit::STEP_S;
    if (spike.step >= AnubarakPursuit::LAST_STEP)
        return step * static_cast<float>(AnubarakPursuit::LAST_STEP);

    return std::max(0.0f, step * static_cast<float>(spike.step + 1) - spike.nextStepS);
}

inline float Uncovered(float restS, bool fast, int patches, int hands, Rules const& r)
{
    if (restS <= 0.0f)
        return 0.0f;

    float const cover = (fast ? r.fastS : r.freshS) + static_cast<float>(std::max(patches, 0)) * (r.idleS + r.freshS) +
                        static_cast<float>(std::max(hands, 0)) * r.handS;
    return std::max(0.0f, restS - cover);
}

inline float PatchCost(Patch const& patch, KiteWorld const& w, Rules const& r)
{
    if (w.heroic)
        return w.reservePatch && !patch.tank ? r.costReservedPatch : r.costPatch;

    return patch.tank && w.burrowers ? r.costNormalTankPatch : r.costNormalPatch;
}

// The first offer that can land by tick land: a paladin 30 yd off the kiter's spot then at a run
inline HandOffer const* HandFor(KiteWorld const& w, Rules const& r, std::vector<Point> const& track, int land)
{
    Point const at = track[static_cast<size_t>(std::clamp(land, 0, static_cast<int>(track.size()) - 1))];
    for (HandOffer const& offer : w.hands)
    {
        float const earliest =
            offer.self ? 0.0f : std::max(0.0f, (Dist(offer.pos, at) - r.handRange) / r.paladinSpeed);
        if (earliest <= static_cast<float>(land) * TICK_S + 0.001f)
            return &offer;
    }

    return nullptr;
}

// How a run ended, for Finish
struct RunEnd
{
    int endTick = 0;
    Point endPoint;
    float margin = 0.0f;
    int raidTick = -1;
    // The patch the spike failed on and when, when that ended it
    int failTick = -1;
    int failPatch = -1;
    bool planned = false;
    bool caught = false;
    bool handUsed = false;
    bool handSpare = false;
    bool ownUsed = false;
    int boostTick = -1;
    int teleportTick = -1;
    // The spike where it failed, else at the end, and which way it was going then
    Spike spike;
    Point heading;
};

inline Point Toward(Point from, Point to)
{
    float const d = Dist(from, to);
    return d > 0.001f ? Point{(to.x - from.x) / d, (to.y - from.y) / d} : Point{0.0f, 0.0f};
}

inline Point Ahead(Point from, Point dir, float length, Rules const& r)
{
    return OnFloor({from.x + dir.x * length, from.y + dir.y * length}, r);
}

// How far the spike runs in leftS from t seconds into the plan, its speed stepping up on time
inline float RunLength(Spike spike, float t, float leftS)
{
    float length = 0.0f;
    while (leftS > 0.0f)
    {
        float const span =
            spike.step < AnubarakPursuit::LAST_STEP ? std::clamp(spike.nextStepS - t, 0.0f, leftS) : leftS;
        length += spike.speed * span;
        leftS -= span;
        t += span;
        if (leftS > 0.0f && spike.step < AnubarakPursuit::LAST_STEP)
        {
            ++spike.step;
            spike.speed = AnubarakPursuit::SPEEDS[spike.step];
            spike.nextStepS += AnubarakPursuit::STEP_S;
        }
    }

    return length;
}

// An immunity or a catch hands the spike on at its speed, mostly to someone at the gather: a raid pass at once
// when it gets within the gather's clearance before it despawns
inline bool HandedOnToGather(Outcome const& out, RunEnd const& end, KiteWorld const& w, Rules const& r)
{
    float const leftS = w.lifeS - out.endS;
    float const full = Dist(end.spike.pos, w.gatherPos);
    return leftS > 0.0f &&
           (full <= r.gatherClearance || RunLength(end.spike, out.endS, leftS) >= full - r.gatherClearance);
}

// A Hand or immunity only pays against a spike the kiter can't outrun there
inline bool OutrunsKiter(Spike const& spike, Point at, int tick, int boostTick, KiteWorld const& w, Rules const& r)
{
    float const t = static_cast<float>(tick) * TICK_S;
    return spike.speed > CandidatePace(w, boostTick).SpeedAt(t) * SlowAt(at, w, r);
}

// The spike's leg after the kite, toward the gather: at its speed, or after a patch at its base speed from where
// the idle drift leaves it. A patch it passes on either is the kite's hazard, at the kite's end.
inline void NextLeg(Outcome& out, RunEnd const& end, KiteWorld const& w, Rules const& r)
{
    bool const spent = end.failTick >= 0;
    auto const passes = [&](Point from, Point to)
    {
        for (size_t k = 0; k < w.patches.size(); ++k)
        {
            Patch const& patch = w.patches[k];
            if ((spent && static_cast<int>(k) == end.failPatch) || (!patch.tank && !patch.usable) ||
                AnubarakPursuit::SegmentDist(from, to, patch.pos) > r.patchReach)
                continue;

            float& hazard = patch.tank ? out.tankS : out.kiteS;
            if (hazard < 0.0f)
                hazard = out.endS;
        }
    };

    Spike spike = end.spike;
    float t = out.endS;
    float leftS = w.lifeS - out.endS;
    if (spent)
    {
        t = 0.0f;
        leftS = w.lifeS - static_cast<float>(end.failTick) * TICK_S - r.idleS;
        spike = Spike();
        spike.pos = Ahead(end.spike.pos, end.heading, r.idleS * AnubarakPursuit::SPEEDS[0], r);
    }

    if (leftS <= 0.0f)
        return;

    if (spent)
        passes(end.spike.pos, Ahead(end.spike.pos, end.heading, r.driftReach, r));

    float const full = Dist(spike.pos, w.gatherPos);
    if (full < 0.01f)
        return;

    passes(spike.pos, Lerp(spike.pos, w.gatherPos, std::min(1.0f, RunLength(spike, t, leftS) / full)));
}

// The hazards, ending, coverage and cost of a run that ended as end says. A run still going at the horizon
// with life left counts as caught there: by then the spike runs at its top speed.
inline void Finish(Outcome& out, RunEnd const& end, KiteWorld const& w, Rules const& r)
{
    bool const open = !end.caught && !end.ownUsed && end.failTick < 0 && end.endTick >= Ticks(w, r);
    out.caught = end.caught || (open && w.lifeS > static_cast<float>(end.endTick) * TICK_S + TICK_S);
    out.endS = static_cast<float>(end.endTick) * TICK_S;
    out.endPoint = end.endPoint;
    out.margin = end.margin;
    if (end.raidTick >= 0 && end.raidTick <= end.endTick)
        out.raidS = static_cast<float>(end.raidTick) * TICK_S;

    // A fail after a catch is still a hazard, but the kite ended at the catch
    bool const spent = end.failTick >= 0;
    bool const failed = spent && !end.caught;
    bool const planned = failed && end.planned;
    if (spent)
    {
        float const s = static_cast<float>(end.failTick) * TICK_S;
        bool const tank = w.patches[static_cast<size_t>(end.failPatch)].tank;
        if (tank && !planned)
            out.tankS = s;
        else if (!tank && (!end.planned || (w.heroic && SpikeAgeS(w.spike) + s < r.earlyPatchS)))
            out.kiteS = s;
    }

    out.ending = planned        ? (end.handUsed ? Ending::PatchHand : Ending::Patch)
                 : end.ownUsed  ? Ending::Own
                 : end.handUsed ? Ending::Hand
                                : Ending::None;
    out.kitePatch = planned && w.heroic && !w.patches[static_cast<size_t>(end.failPatch)].tank;

    int patchesLeft = w.patchBudget;
    if (spent && w.patches[static_cast<size_t>(end.failPatch)].usable)
        --patchesLeft;

    // Still running at the horizon counts as handed on fast there
    float restS = w.lifeS - out.endS;
    bool fast = true;
    if (failed)
    {
        restS -= r.idleS;
        fast = false;
    }

    out.uncoveredS = Uncovered(restS, fast, patchesLeft, w.spareHands - (end.handSpare ? 1 : 0), r);
    if (planned)
        out.cost += PatchCost(w.patches[static_cast<size_t>(end.failPatch)], w, r);

    if (end.handUsed)
        out.cost += end.handSpare ? r.costHand : r.costLastHand;

    out.lastHand = end.handUsed && !end.handSpare;

    if (end.ownUsed)
        out.cost += w.lifeS - out.endS <= r.ownLateS ? r.costOwn : r.costOwnMid;

    if (end.boostTick >= 0 && end.boostTick <= end.endTick)
        out.boostS = static_cast<float>(end.boostTick) * TICK_S;

    if (end.teleportTick >= 0 && end.teleportTick <= end.endTick)
        out.teleportS = static_cast<float>(end.teleportTick) * TICK_S;

    if (w.heroic && w.gather && !spent && (end.ownUsed || out.caught) && HandedOnToGather(out, end, w, r))
        out.raidS = 0.0f;

    if (w.heroic && w.gather && (spent || end.ownUsed || out.caught))
        NextLeg(out, end, w, r);

    if (w.gather && (end.ownUsed || out.caught))
        out.endGather = Dist(out.endPoint, w.gatherPos);
}

// A run's outcome, stopped at the first patch the spike fails on. hand lands a Hand handLeadS before the catch,
// own the immunity ownLeadS before it, after the Hand when both, each only on a spike faster than the kiter.
inline Outcome Score(std::vector<Point> const& track, int turnTick, int boostTick, bool hand, bool own,
                     Run const& run, KiteWorld const& w, Rules const& r, int teleportTick = -1)
{
    Outcome out;
    out.turnS = turnTick >= 0 ? static_cast<float>(turnTick) * TICK_S : -1.0f;
    int const last = run.failTick >= 0 ? run.failTick : static_cast<int>(run.spikes.size()) - 1;
    int const scanTo = run.failTick >= 0 ? run.failTick : last + 1;
    CatchWindow window = MakeCatchWindow(w, r);
    int catchTick = FirstCatch(run, track, 0, scanTo, window, r);
    out.catchS = catchTick >= 0 ? static_cast<float>(catchTick) * TICK_S : -1.0f;
    if (catchTick >= 0)
        out.catchPoint = track[static_cast<size_t>(catchTick)];

    auto const outruns = [&](int tick)
    {
        return OutrunsKiter(run.spikes[static_cast<size_t>(tick)], track[static_cast<size_t>(tick)], tick, boostTick,
                            w, r);
    };

    RunEnd end;
    if (hand && catchTick >= 0 && w.handLeftS <= 0.0f && outruns(catchTick))
    {
        int const land = std::max(catchTick - ToTick(r.handLeadS), 0);
        if (HandOffer const* offer = HandFor(w, r, track, land))
        {
            end.handUsed = true;
            end.handSpare = offer->spare;
            out.handS = static_cast<float>(land) * TICK_S;
            out.handId = offer->id;
            PlanHand(window, land, r);
            catchTick = FirstCatch(run, track, land, scanTo, window, r);
        }
    }

    end.endTick = last;
    if (catchTick >= 0)
    {
        end.endTick = catchTick;
        if (own && w.own && (!end.handUsed || w.ownAfterHand) && outruns(catchTick))
        {
            end.ownUsed = true;
            end.endTick = std::max(catchTick - ToTick(r.ownLeadS), 0);
            out.ownS = static_cast<float>(end.endTick) * TICK_S;
        }
        else
            end.caught = true;
    }

    if (run.failTick >= 0 &&
        (catchTick < 0 ||
         (end.caught && FailCountsAfterCatch(run, catchTick, run.spikes[static_cast<size_t>(catchTick)], r))))
    {
        end.failTick = run.failTick;
        end.failPatch = run.failPatch;
        end.planned = run.failPlanned;
    }

    end.endPoint = track[static_cast<size_t>(end.endTick)];
    end.margin = run.margin[static_cast<size_t>(end.endTick)];
    end.raidTick = run.raidTick;
    end.boostTick = boostTick;
    end.teleportTick = teleportTick;
    size_t const spikeTick = static_cast<size_t>(end.failTick >= 0 ? end.failTick : end.endTick);
    end.spike = run.spikes[spikeTick];
    if (spikeTick > 0)
        end.heading = Toward(run.spikes[spikeTick - 1].pos, end.spike.pos);

    Finish(out, end, w, r);
    return out;
}

inline constexpr size_t HEADING_TICKS = 10;

inline Point Heading(std::vector<Point> const& track)
{
    Point const from = track.front();
    Point const to = track[std::min<size_t>(HEADING_TICKS, track.size() - 1)];
    float const d = Dist(from, to);
    return d > 0.5f ? Point{(to.x - from.x) / d, (to.y - from.y) / d} : Point{0.0f, 0.0f};
}

inline float TurnDegrees(Point a, Point b)
{
    if ((a.x == 0.0f && a.y == 0.0f) || (b.x == 0.0f && b.y == 0.0f))
        return 0.0f;

    return std::acos(std::clamp(a.x * b.x + a.y * b.y, -1.0f, 1.0f)) / DEG_TO_RAD;
}
}  // namespace Detail

// Negative when a ranks first. Rows past `rows` aren't looked at: the held plan only has to match the
// best on the first five.
inline int Compare(Outcome const& a, Outcome const& b, Rules const& r, int rows = 8)
{
    auto const hazard = [&r](float x, float y)
    {
        if ((x >= 0.0f) != (y >= 0.0f))
            return x >= 0.0f ? 1 : -1;

        if (x < 0.0f)
            return 0;

        float const bx = Detail::Band(x, r.hazardBandS);
        float const by = Detail::Band(y, r.hazardBandS);
        return bx == by ? 0 : (bx > by ? -1 : 1);
    };

    if (int const c = hazard(a.tankS, b.tankS))
        return c;

    if (int const c = hazard(a.kiteS, b.kiteS))
        return c;

    if (int const c = hazard(a.raidS, b.raidS))
        return c;

    if (a.caught != b.caught)
        return a.caught ? 1 : -1;

    auto const uncovered = [&r](float s) { return s <= 0.0f ? -1.0f : Detail::Band(s, r.uncoveredBandS); };
    if (uncovered(a.uncoveredS) != uncovered(b.uncoveredS))
        return uncovered(a.uncoveredS) < uncovered(b.uncoveredS) ? -1 : 1;

    if (rows <= 4)
        return 0;

    // Both onto a kite patch: the later first, so a spare Hand or the teleport that carries the kite on to
    // its patch beats the plain walk onto it. The Hand kept back still goes by cost.
    float const endA = Detail::Band(a.endS, r.endBandS);
    float const endB = Detail::Band(b.endS, r.endBandS);
    bool const bothKite = a.kitePatch && b.kitePatch && !a.lastHand && !b.lastHand;
    if (bothKite && endA != endB)
        return endA > endB ? -1 : 1;

    if (std::fabs(a.cost - b.cost) > 0.01f)
        return a.cost < b.cost ? -1 : 1;

    if (rows <= 5)
        return 0;

    if (endA != endB)
        return endA > endB ? -1 : 1;

    // A boost or the teleport costs nothing but its cooldown: kept unless it ends the kite later, so it's
    // there next submerge
    if ((a.boostS >= 0.0f) != (b.boostS >= 0.0f))
        return a.boostS < 0.0f ? -1 : 1;

    if ((a.teleportS >= 0.0f) != (b.teleportS >= 0.0f))
        return a.teleportS < 0.0f ? -1 : 1;

    if (a.endGather >= 0.0f && b.endGather >= 0.0f)
    {
        float const ga = Detail::Band(a.endGather, r.gatherBand);
        float const gb = Detail::Band(b.endGather, r.gatherBand);
        if (ga != gb)
            return ga > gb ? -1 : 1;
    }

    float const ma = Detail::Band(std::min(a.margin, r.marginCap), r.marginBand);
    float const mb = Detail::Band(std::min(b.margin, r.marginCap), r.marginBand);
    if (ma != mb)
        return ma > mb ? -1 : 1;

    return 0;
}

// The candidate's run and outcome
inline Outcome Judge(Candidate const& c, KiteWorld const& w, Rules const& r)
{
    int const ticks = Detail::Ticks(w, r);
    Detail::Watch const watch = Detail::MakeWatch(w, r);
    Detail::Run run = Detail::StartRun(w, r);
    Detail::Chase(run, c.track, 0, std::min(ticks, static_cast<int>(c.track.size()) - 1), c.patch, c.turnTick, w, r,
                  watch);
    return Detail::Score(c.track, c.turnTick, c.boostTick, c.hand, c.own, run, w, r, c.teleportTick);
}

// A ring member: straight on headingDeg until radius off the centre, then round it, turn +1 counter-clockwise
inline Candidate RingCandidate(KiteWorld const& w, Rules const& r, float headingDeg, float radius, float turn,
                               int boostTick = -1)
{
    Candidate c;
    c.shape = Shape::Ring;
    c.headingDeg = headingDeg;
    c.radius = radius;
    c.turn = turn;
    c.boostTick = boostTick;
    Pace const pace = Detail::CandidatePace(w, boostTick);
    float const a = headingDeg * DEG_TO_RAD;
    AnubarakPursuit::KiterPath const path = AnubarakPursuit::MakeKiterPath(
        w.kiter, {std::cos(a), std::sin(a)}, pace, r.centre, AnubarakPursuit::PathMember{radius, turn, 1e9f});
    c.track.push_back(w.kiter);
    float s = 0.0f;
    Detail::Walk(c.track, Detail::Ticks(w, r), pace, w, r, [&path, &s](Point, float length, float)
                 {
                     s += length;
                     return Detail::AtLength(path, s);
                 });
    return c;
}

inline Candidate ArcCandidate(KiteWorld const& w, Rules const& r, float headingDeg, float turnDeg,
                              int boostTick = -1)
{
    Candidate c;
    c.shape = Shape::Arc;
    c.headingDeg = headingDeg;
    c.turn = turnDeg;
    c.boostTick = boostTick;
    float const a = headingDeg * DEG_TO_RAD;
    c.track.push_back(w.kiter);
    Detail::Walk(c.track, Detail::Ticks(w, r), Detail::CandidatePace(w, boostTick), w, r,
                 Detail::ArcStep{{std::cos(a), std::sin(a)}, turnDeg, r.arcStraightS, &r});
    return c;
}

// Straight on at velocity, sliding along the rim: a human kiter's run
inline Candidate MotionCandidate(KiteWorld const& w, Rules const& r, Point velocity)
{
    Candidate c;
    c.shape = Shape::Motion;
    float const speed = std::hypot(velocity.x, velocity.y);
    c.track.push_back(w.kiter);
    Point const dir = speed > 0.01f ? Point{velocity.x / speed, velocity.y / speed} : Point{1.0f, 0.0f};
    Detail::Walk(c.track, Detail::Ticks(w, r), Pace(speed), w, r, Detail::ArcStep{dir, 0.0f, 0.0f, &r});
    return c;
}

// The tick on track nearest at, searched span ticks either side of from
inline int NearestTick(std::vector<Point> const& track, Point at, int from, int span)
{
    int const last = static_cast<int>(track.size()) - 1;
    int best = std::clamp(from, 0, last);
    for (int i = std::max(from - span, 0); i <= std::min(last, from + span); ++i)
        if (Dist(track[static_cast<size_t>(i)], at) < Dist(track[static_cast<size_t>(best)], at))
            best = i;

    return best;
}

// The first tick past from that's yards along the track, at most maxTicks past it and never past its end
inline int AheadTick(std::vector<Point> const& track, int from, int maxTicks, float yards)
{
    int const last = std::min(static_cast<int>(track.size()) - 1, from + maxTicks);
    float walked = 0.0f;
    int i = std::clamp(from, 0, std::max(last, 0));
    while (i < last && walked < yards)
    {
        walked += Dist(track[static_cast<size_t>(i)], track[static_cast<size_t>(i) + 1]);
        ++i;
    }

    return i;
}

// The held plan on from its track point nearest the kiter, searched slackTicks round elapsedS. Past its end a
// patch walk stands on the patch and a run goes straight on. A ready boost goes now once its tick has passed.
inline Candidate HeldCandidate(Candidate const& held, float elapsedS, KiteWorld const& w, Rules const& r,
                               int slackTicks = 15)
{
    Candidate c;
    c.shape = Shape::Held;
    c.headingDeg = held.headingDeg;
    c.radius = held.radius;
    c.turn = held.turn;
    c.ending = held.ending;
    c.patch = held.patch;
    c.move = held.move;
    c.hand = held.hand;
    c.own = held.own;
    c.track.push_back(w.kiter);
    if (held.track.size() < 2)
        return c;

    int const ticks = Detail::Ticks(w, r);
    int const at = NearestTick(held.track, w.kiter, static_cast<int>(std::lround(elapsedS / TICK_S)), slackTicks);
    for (size_t i = static_cast<size_t>(at) + 1; i < held.track.size() && static_cast<int>(c.track.size()) <= ticks; ++i)
        c.track.push_back(held.track[i]);

    c.turnTick = held.turnTick >= 0 ? std::max(held.turnTick - at, 0) : -1;
    c.boostTick = held.boostTick >= 0 && w.readyBoostS > 0.0f ? std::max(held.boostTick - at, 0) : -1;
    // Nearest a tick past the jump, the kiter is already at the circle
    c.teleportTick = held.teleportTick >= at && w.teleport ? held.teleportTick - at : -1;
    if (c.patch >= 0 && c.turnTick >= 0)
    {
        Point const last = c.track.back();
        while (static_cast<int>(c.track.size()) <= ticks)
            c.track.push_back(last);

        return c;
    }

    Point dir = {1.0f, 0.0f};
    size_t const n = c.track.size();
    if (n >= 2 && Dist(c.track[n - 2], c.track[n - 1]) > 0.01f)
    {
        float const d = Dist(c.track[n - 2], c.track[n - 1]);
        dir = {(c.track[n - 1].x - c.track[n - 2].x) / d, (c.track[n - 1].y - c.track[n - 2].y) / d};
    }

    Detail::Walk(c.track, ticks, Detail::CandidatePace(w, c.boostTick), w, r, Detail::ArcStep{dir, 0.0f, 0.0f, &r});
    return c;
}

namespace Detail
{
struct Base
{
    Candidate candidate;
    Run run;
    Outcome none;
};

// A judged candidate: a base shape with a Hand or immunity on its end, or one of the patch endings
struct Entry
{
    int base = -1;
    int ending = -1;
    bool hand = false;
    bool own = false;
    Outcome outcome;
};

inline AnubarakPursuit::PatchRules LegRules(Rules const& r)
{
    AnubarakPursuit::PatchRules legs;
    legs.edgeRadius = r.edgeRadius;
    legs.edgeRunOn = r.edgeRunOn;
    return legs;
}

inline bool CanBoost(KiteWorld const& w) { return w.readyBoostS > 0.0f && w.readyBoostSpeed > w.pace.speed; }

// The first tick of an unboosted base's run with the spike faster than the kiter's run and within boostRange,
// -1 when none comes by the run's end
inline int BoostTick(Base const& base, KiteWorld const& w, Rules const& r)
{
    int const last = std::min({ToTick(base.none.endS), static_cast<int>(base.run.spikes.size()) - 1,
                               static_cast<int>(base.candidate.track.size()) - 1});
    for (int k = 0; k <= last; ++k)
    {
        Spike const& spike = base.run.spikes[static_cast<size_t>(k)];
        if (spike.speed > w.pace.speed + 0.01f &&
            Dist(spike.pos, base.candidate.track[static_cast<size_t>(k)]) <= r.boostRange)
            return k;
    }

    return -1;
}

inline Candidate Boosted(Candidate const& c, KiteWorld const& w, Rules const& r, int boostTick)
{
    if (c.shape == Shape::Ring)
        return RingCandidate(w, r, c.headingDeg, c.radius, c.turn, boostTick);

    return ArcCandidate(w, r, c.headingDeg, c.turn, boostTick);
}

// The base run to ownLeadS before a catch it can't outrun, the teleport, then the best teleportShapes runs on
// from the circle. None unless the circle is in range then and teleportGain farther from the spike.
inline std::vector<Base> TeleportBases(Base const& base, KiteWorld const& w, Rules const& r, Watch const& watch)
{
    std::vector<Base> out;
    std::vector<Point> const& track = base.candidate.track;
    int const catchTick = base.none.catchS >= 0.0f ? ToTick(base.none.catchS) : -1;
    if (!w.teleport || base.candidate.teleportTick >= 0 || catchTick < 0 ||
        catchTick >= static_cast<int>(std::min(track.size(), base.run.spikes.size())))
        return out;

    int const jump = std::max(catchTick - ToTick(r.ownLeadS), 0);
    Point const at = track[static_cast<size_t>(jump)];
    Point const spike = base.run.spikes[static_cast<size_t>(jump)].pos;
    if (!OutrunsKiter(base.run.spikes[static_cast<size_t>(catchTick)], track[static_cast<size_t>(catchTick)],
                      catchTick, base.candidate.boostTick, w, r) ||
        Dist(at, w.circle) > r.teleportRange || Dist(w.circle, spike) < Dist(at, spike) + r.teleportGain)
        return out;

    // The kiter as it lands on the circle, the world's clock moved on to the jump
    float const jumpS = static_cast<float>(jump) * TICK_S;
    KiteWorld from = w;
    from.kiter = w.circle;
    from.lifeS = w.lifeS - jumpS;
    from.pace = CandidatePace(w, base.candidate.boostTick).After(jumpS);
    Point const away = Toward(spike, w.circle);
    float const awayDeg = std::atan2(away.y, away.x) / DEG_TO_RAD;
    int const ticks = Ticks(w, r);

    std::vector<Base> runs;
    auto const add = [&](Candidate const& member)
    {
        Base jumped;
        Candidate& c = jumped.candidate;
        c.shape = base.candidate.shape;
        c.headingDeg = base.candidate.headingDeg;
        c.radius = base.candidate.radius;
        c.turn = base.candidate.turn;
        c.boostTick = base.candidate.boostTick;
        c.teleportTick = jump;
        c.track.assign(track.begin(), track.begin() + jump + 1);
        c.track.insert(c.track.end(), member.track.begin(), member.track.end());
        c.track.resize(std::min(c.track.size(), static_cast<size_t>(ticks) + 1));
        while (static_cast<int>(c.track.size()) <= ticks)
            c.track.push_back(c.track.back());

        Run& run = jumped.run;
        run.spikes.assign(base.run.spikes.begin(), base.run.spikes.begin() + jump + 1);
        run.margin.assign(base.run.margin.begin(), base.run.margin.begin() + jump + 1);
        run.raidTick = base.run.raidTick >= 0 && base.run.raidTick <= jump ? base.run.raidTick : -1;
        run.leftTick = base.run.leftTick;
        for (int& left : run.leftTick)
            if (left > jump)
                left = -1;

        Chase(run, c.track, jump, ticks, -1, -1, w, r, watch);
        jumped.none = Score(c.track, -1, c.boostTick, false, false, run, w, r, jump);
        runs.push_back(std::move(jumped));
    };

    for (float offset : r.teleportHeadings)
    {
        float const deg = awayDeg + offset;
        for (float radius : {r.ring, r.rim})
            for (float turn : {1.0f, -1.0f})
                add(RingCandidate(from, r, deg, radius, turn));

        for (float turn : r.teleportArcTurns)
            add(ArcCandidate(from, r, deg, turn));
    }

    std::stable_sort(runs.begin(), runs.end(),
                     [&r](Base const& a, Base const& b) { return Compare(a.none, b.none, r) < 0; });
    for (size_t i = 0; i < runs.size() && i < static_cast<size_t>(r.teleportShapes); ++i)
        out.push_back(std::move(runs[i]));

    return out;
}

// The base shape to tick turn, then the move onto patch p, under a Hand before the base's catch when hand. The
// run stops at the first patch fail or counted catch, so c.track ends there. False when it can't be planned.
inline bool RunEnding(Base const& base, int turn, int p, PatchMove move, bool hand, KiteWorld const& w,
                      Rules const& r, Watch const& watch, Candidate& c, Outcome& out)
{
    int const ticks = Ticks(w, r);
    int const baseCatch = base.none.catchS >= 0.0f ? ToTick(base.none.catchS) : -1;
    std::vector<Point> legs;
    if (turn < 0 || turn >= ticks || turn <= base.candidate.teleportTick ||
        (base.run.failTick >= 0 && turn >= base.run.failTick) ||
        (!hand && baseCatch >= 0 && baseCatch <= turn) ||
        !AnubarakPursuit::PatchMoveLegs(move, base.candidate.track[static_cast<size_t>(turn)],
                                        w.patches[static_cast<size_t>(p)].pos, LegRules(r), legs))
        return false;

    out = Outcome();
    CatchWindow window = MakeCatchWindow(w, r);
    RunEnd end;
    if (hand)
    {
        int const land = baseCatch >= 0 ? std::max(baseCatch - ToTick(r.handLeadS), 0) : -1;
        HandOffer const* offer = land >= 0 ? HandFor(w, r, base.candidate.track, land) : nullptr;
        if (!offer || turn < land ||
            !OutrunsKiter(base.run.spikes[static_cast<size_t>(baseCatch)],
                          base.candidate.track[static_cast<size_t>(baseCatch)], baseCatch, base.candidate.boostTick,
                          w, r))
            return false;

        end.handUsed = true;
        end.handSpare = offer->spare;
        out.handS = static_cast<float>(land) * TICK_S;
        out.handId = offer->id;
        out.catchS = base.none.catchS;
        out.catchPoint = base.none.catchPoint;
        PlanHand(window, land, r);
    }

    c = Candidate();
    c.shape = base.candidate.shape;
    c.headingDeg = base.candidate.headingDeg;
    c.radius = base.candidate.radius;
    c.turn = base.candidate.turn;
    c.ending = hand ? Ending::PatchHand : Ending::Patch;
    c.patch = p;
    c.move = move;
    c.turnTick = turn;
    c.boostTick = base.candidate.boostTick;
    c.teleportTick = base.candidate.teleportTick;
    c.hand = hand;
    c.track.assign(base.candidate.track.begin(), base.candidate.track.begin() + turn + 1);
    Pace const pace = CandidatePace(w, c.boostTick);

    Run run;
    run.spikes.push_back(base.run.spikes[static_cast<size_t>(turn)]);
    run.margin.push_back(base.run.margin[static_cast<size_t>(turn)]);
    run.raidTick = base.run.raidTick >= 0 && base.run.raidTick <= turn ? base.run.raidTick : -1;
    run.leftTick = base.run.leftTick;
    for (int& left : run.leftTick)
        if (left > turn)
            left = -1;

    LegStep step{&legs, 0};
    Point at = c.track.back();
    int catchTick = -1;
    float catchMargin = 0.0f;
    int const postCatch = ToTick(r.postCatchS);
    for (int i = turn; i < ticks && (catchTick < 0 || i < catchTick + postCatch); ++i)
    {
        float const t = static_cast<float>(i) * TICK_S;
        at = OnFloor(step(at, pace.SpeedAt(t) * TICK_S * SlowAt(at, w, r), t), r);
        c.track.push_back(at);
        if (!ChaseOne(run, c.track, i, p, turn, w, r, watch))
            break;

        if (catchTick < 0 && window.Counts(i + 1) && Caught(run.spikes.back().pos, at, r))
        {
            catchTick = i + 1;
            catchMargin = run.margin.back();
        }
    }

    end.endTick = catchTick >= 0 ? catchTick : static_cast<int>(c.track.size()) - 1;
    end.margin = catchTick >= 0 ? catchMargin : run.margin.back();
    if (catchTick >= 0)
    {
        end.caught = true;
        if (out.catchS < 0.0f)
        {
            out.catchS = static_cast<float>(catchTick) * TICK_S;
            out.catchPoint = c.track[static_cast<size_t>(catchTick)];
        }
    }

    if (run.failTick >= 0 &&
        (catchTick < 0 || FailCountsAfterCatch(run, catchTick, run.spikes[static_cast<size_t>(catchTick - turn)], r)))
    {
        end.failTick = run.failTick;
        end.failPatch = run.failPatch;
        end.planned = run.failPlanned;
    }

    end.endPoint = c.track[static_cast<size_t>(end.endTick)];
    end.raidTick = run.raidTick;
    end.boostTick = c.boostTick;
    end.teleportTick = c.teleportTick;
    size_t const spikeTick = static_cast<size_t>((end.failTick >= 0 ? end.failTick : end.endTick) - turn);
    end.spike = run.spikes[spikeTick];
    if (spikeTick > 0)
        end.heading = Toward(run.spikes[spikeTick - 1].pos, end.spike.pos);
    else if (turn > 0)
        end.heading = Toward(base.run.spikes[static_cast<size_t>(turn - 1)].pos, end.spike.pos);

    out.turnS = static_cast<float>(turn) * TICK_S;
    Finish(out, end, w, r);
    return true;
}

// An ending's track carried on to the horizon, standing on the patch once its legs run out
inline void FullTrack(Candidate& c, KiteWorld const& w, Rules const& r)
{
    int const ticks = Ticks(w, r);
    if (c.patch < 0 || c.turnTick < 0 || static_cast<int>(c.track.size()) > ticks)
        return;

    std::vector<Point> legs;
    c.track.resize(static_cast<size_t>(c.turnTick) + 1);
    if (AnubarakPursuit::PatchMoveLegs(c.move, c.track.back(), w.patches[static_cast<size_t>(c.patch)].pos,
                                       LegRules(r), legs))
        Walk(c.track, ticks, CandidatePace(w, c.boostTick), w, r, LegStep{&legs, 0});

    while (static_cast<int>(c.track.size()) <= ticks)
        c.track.push_back(c.track.back());
}

// The patch endings on one base shape: onto each usable patch from each turn second, every move in the last
// 6 s before the base's end and straight in every 2 s before that, alone or under a Hand landed at the catch
inline void PatchEndings(Base const& base, int baseIndex, KiteWorld const& w, Rules const& r, Watch const& watch,
                         std::vector<Candidate>& endings, std::vector<Entry>& entries)
{
    int const ticks = Ticks(w, r);
    int const failTick = base.run.failTick >= 0 ? base.run.failTick : ticks;
    int const catchTick = base.none.catchS >= 0.0f ? ToTick(base.none.catchS) : ticks;
    int const endTick = std::min(catchTick, failTick) - 1;
    int const step = std::max(1, ToTick(r.turnStepS));
    int const allMoves = endTick - ToTick(r.edgeWindowS);
    int const first = std::max(0, endTick - ToTick(r.patchWindowS));
    bool const underHand = w.handLeftS > 0.0f;

    Candidate c;
    Outcome out;
    auto const add = [&]()
    {
        Entry entry;
        entry.base = baseIndex;
        entry.ending = static_cast<int>(endings.size());
        entry.outcome = out;
        entries.push_back(entry);
        endings.push_back(std::move(c));
    };

    for (size_t p = 0; p < w.patches.size(); ++p)
    {
        if (!w.patches[p].usable)
            continue;

        int const patch = static_cast<int>(p);
        for (int turn = endTick; turn >= first; turn -= turn > allMoves ? step : 2 * step)
            for (PatchMove move : {PatchMove::In, PatchMove::EdgeLeft, PatchMove::EdgeRight})
                if ((move == PatchMove::In || (!underHand && turn > allMoves)) &&
                    RunEnding(base, turn, patch, move, false, w, r, watch, c, out))
                    add();

        if (!underHand && !w.hands.empty() && catchTick < ticks)
        {
            int const land = std::max(catchTick - ToTick(r.handLeadS), 0);
            for (int turn = land; turn < land + ToTick(r.handS - r.handMarginS); turn += step)
                if (RunEnding(base, turn, patch, PatchMove::In, true, w, r, watch, c, out))
                    add();
        }
    }
}
}  // namespace Detail

// Every candidate judged, the best kept. held is the last re-plan's choice, held heldAgeS so far, its track
// started heldElapsedS ago.
inline Plan PlanKite(KiteWorld const& w, Candidate const* held, float heldAgeS, float heldElapsedS,
                     Rules const& r = Rules())
{
    Plan plan;
    int const ticks = Detail::Ticks(w, r);
    Detail::Watch const watch = Detail::MakeWatch(w, r);

    std::vector<Detail::Base> bases;
    size_t const shapes = static_cast<size_t>(r.headings) * (4 + r.arcTurns.size());
    bases.reserve(Detail::CanBoost(w) ? shapes * 2 : shapes);
    auto const addBase = [&](Candidate&& c)
    {
        Detail::Base base;
        base.candidate = std::move(c);
        base.run = Detail::StartRun(w, r);
        Detail::Chase(base.run, base.candidate.track, 0, ticks, -1, -1, w, r, watch);
        base.none =
            Detail::Score(base.candidate.track, -1, base.candidate.boostTick, false, false, base.run, w, r);
        bases.push_back(std::move(base));
    };

    std::vector<float> radii = {r.ring, r.rim};
    if (w.teleport)
        radii.push_back(r.teleportRing);

    for (int h = 0; h < r.headings; ++h)
    {
        float const deg = 360.0f * static_cast<float>(h) / static_cast<float>(r.headings);
        for (float radius : radii)
            for (float turn : {1.0f, -1.0f})
                addBase(RingCandidate(w, r, deg, radius, turn));

        for (float turn : r.arcTurns)
            addBase(ArcCandidate(w, r, deg, turn));
    }

    std::vector<size_t> order(bases.size());
    for (size_t i = 0; i < order.size(); ++i)
        order[i] = i;

    std::stable_sort(order.begin(), order.end(),
                     [&](size_t a, size_t b) { return Compare(bases[a].none, bases[b].none, r) < 0; });
    std::vector<uint8_t> ended(bases.size(), 0);
    size_t const top = std::min(order.size(), static_cast<size_t>(r.endingShapes));
    for (size_t i = 0; i < top; ++i)
        ended[order[i]] = 1;

    // The longest runs often end far from every patch, so each patch also gets the shapes that pass it latest
    for (Patch const& patch : w.patches)
    {
        if (!patch.usable)
            continue;

        std::vector<std::pair<int, size_t>> nearby;
        for (size_t i = 0; i < bases.size(); ++i)
        {
            int const last = static_cast<int>(bases[i].none.endS / TICK_S);
            for (int k = std::min(last, static_cast<int>(bases[i].candidate.track.size()) - 1); k > 0; --k)
                if (Dist(bases[i].candidate.track[static_cast<size_t>(k)], patch.pos) <= r.patchNear)
                {
                    nearby.emplace_back(k, i);
                    break;
                }
        }

        std::sort(nearby.begin(), nearby.end(), [](auto const& a, auto const& b) { return a.first > b.first; });
        for (size_t n = 0; n < nearby.size() && n < static_cast<size_t>(r.nearShapes); ++n)
            ended[nearby[n].second] = 1;
    }

    // The shapes given endings again with the ready boost fired where their own run first wants it
    if (Detail::CanBoost(w))
        for (size_t i = 0, plain = bases.size(); i < plain; ++i)
        {
            int const boostTick = ended[i] ? Detail::BoostTick(bases[i], w, r) : -1;
            if (boostTick < 0)
                continue;

            addBase(Detail::Boosted(bases[i].candidate, w, r, boostTick));
            ended.push_back(1);
        }

    // And again with the teleport before their catch, every run on the ring that stays in range among them
    if (w.teleport)
        for (size_t i = 0, plain = bases.size(); i < plain; ++i)
        {
            Candidate const& c = bases[i].candidate;
            if (!ended[i] && !(c.shape == Shape::Ring && c.radius == r.teleportRing))
                continue;

            std::vector<Detail::Base> jumps = Detail::TeleportBases(bases[i], w, r, watch);
            for (Detail::Base& jump : jumps)
            {
                bases.push_back(std::move(jump));
                ended.push_back(1);
            }
        }

    std::vector<Detail::Entry> entries;
    std::vector<Candidate> endings;
    entries.reserve(bases.size() * 4);
    for (size_t i = 0; i < bases.size(); ++i)
    {
        Detail::Base const& base = bases[i];
        auto const add = [&](bool hand, bool own, Outcome const& outcome)
        {
            Detail::Entry entry;
            entry.base = static_cast<int>(i);
            entry.hand = hand;
            entry.own = own;
            entry.outcome = outcome;
            entries.push_back(entry);
        };

        add(false, false, base.none);
        if (base.none.catchS < 0.0f)
            continue;

        std::vector<Point> const& track = base.candidate.track;
        int const boostTick = base.candidate.boostTick;
        int const teleportTick = base.candidate.teleportTick;
        if (w.own)
            add(false, true, Detail::Score(track, -1, boostTick, false, true, base.run, w, r, teleportTick));

        if (!w.hands.empty() && w.handLeftS <= 0.0f)
        {
            add(true, false, Detail::Score(track, -1, boostTick, true, false, base.run, w, r, teleportTick));
            if (w.own)
                add(true, true, Detail::Score(track, -1, boostTick, true, true, base.run, w, r, teleportTick));
        }
    }

    for (size_t i = 0; i < bases.size(); ++i)
        if (ended[i])
            Detail::PatchEndings(bases[i], static_cast<int>(i), w, r, watch, endings, entries);

    plan.judged = static_cast<int>(entries.size());
    if (entries.empty())
        return plan;

    size_t best = 0;
    for (size_t i = 1; i < entries.size(); ++i)
        if (Compare(entries[i].outcome, entries[best].outcome, r) < 0)
            best = i;

    Detail::Entry const& pick = entries[best];
    if (pick.ending >= 0)
    {
        plan.candidate = std::move(endings[static_cast<size_t>(pick.ending)]);
        if (pick.outcome.boostS < 0.0f)
            plan.candidate.boostTick = -1;

        Detail::FullTrack(plan.candidate, w, r);
    }
    else
    {
        plan.candidate = std::move(bases[static_cast<size_t>(pick.base)].candidate);
        plan.candidate.hand = pick.hand;
        plan.candidate.own = pick.own;
        plan.candidate.ending = pick.hand ? Ending::Hand : pick.own ? Ending::Own : Ending::None;
        if (pick.outcome.boostS < 0.0f)
            plan.candidate.boostTick = -1;
    }

    plan.valid = true;
    plan.outcome = pick.outcome;
    if (!held || held->track.size() < 2)
        return plan;

    Candidate keep = HeldCandidate(*held, heldElapsedS, w, r);
    Outcome const keepOut = Judge(keep, w, r);
    ++plan.judged;
    int const head = Compare(plan.outcome, keepOut, r, 5);
    bool const beaten = head < 0 || (head == 0 && plan.outcome.endS > keepOut.endS + r.keepEndS);
    float const turn = Detail::TurnDegrees(Detail::Heading(keep.track), Detail::Heading(plan.candidate.track));
    bool const waits = turn > r.turnWaitDegrees && heldAgeS < r.turnWaitS;
    if (!beaten || waits)
    {
        plan.candidate = std::move(keep);
        plan.outcome = keepOut;
        plan.kept = true;
    }

    return plan;
}

// A human kiter: its current motion held, judged once for the catch the Hand and the escort serve
inline Plan PlanMotion(KiteWorld const& w, Point velocity, Rules const& r = Rules())
{
    Plan plan;
    Candidate c = MotionCandidate(w, r, velocity);
    if (!w.hands.empty() && w.handLeftS <= 0.0f)
    {
        c.ending = Ending::Hand;
        c.hand = true;
    }

    plan.outcome = Judge(c, w, r);
    plan.candidate = std::move(c);
    plan.valid = true;
    plan.judged = 1;
    return plan;
}
}  // namespace AnubarakKitePlanner

#endif
