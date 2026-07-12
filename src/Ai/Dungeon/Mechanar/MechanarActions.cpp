#include "Playerbots.h"
#include "AiFactory.h"
#include "MechanarActions.h"
#include "MechanarShared.h"
#include "Group.h"
#include "Timer.h"

#include <algorithm>
#include <cmath>
#include <utility>
#include <vector>

namespace
{
    bool Normalize(float& x, float& y)
    {
        float const len = std::sqrt(x * x + y * y);
        if (len < 0.01f)
            return false;
        x /= len;
        y /= len;
        return true;
    }

    float Dist2d(float ax, float ay, float bx, float by)
    {
        float const dx = ax - bx, dy = ay - by;
        return std::sqrt(dx * dx + dy * dy);
    }

    // Closest approach of segment a->b to point (px,py): how near a hop's path passes a
    // hazard, where the destination-only distance cannot see a straight-through pass.
    float MinSegDist(float ax, float ay, float bx, float by, float px, float py)
    {
        float const dx = bx - ax, dy = by - ay;
        float const len2 = dx * dx + dy * dy;
        float t = (len2 < 1e-6f) ? 0.0f : ((px - ax) * dx + (py - ay) * dy) / len2;
        t = std::max(0.0f, std::min(1.0f, t));
        return Dist2d(ax + t * dx, ay + t * dy, px, py);
    }

    // Average position of nearby living group-mates (excluding this bot). The flame is led
    // away from this point so its Inferno never rakes the stacked raid.
    bool PartyCentroid(Player* bot, float& cx, float& cy)
    {
        Group* group = bot->GetGroup();
        if (!group)
            return false;

        float sx = 0.0f, sy = 0.0f;
        int n = 0;
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || member == bot || !member->IsAlive() ||
                member->GetMapId() != bot->GetMapId())
                continue;
            if (bot->GetExactDist2d(member) > 60.0f)
                continue;
            sx += member->GetPositionX();
            sy += member->GetPositionY();
            ++n;
        }
        if (n == 0)
            return false;
        cx = sx / n;
        cy = sy / n;
        return true;
    }

    // Position of the nearest live tank group-mate: the heal target the fixated healer's
    // kite is leashed to (see HEALER_KITE_LEASH). Nearest covers the two-tank group;
    // recomputed every tick so a tank that is itself kiting is tracked.
    bool TankPosition(Player* bot, float& tx, float& ty)
    {
        Group* group = bot->GetGroup();
        if (!group)
            return false;

        float best = 1e18f;
        bool found = false;
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || member == bot || !member->IsAlive() ||
                member->GetMapId() != bot->GetMapId())
                continue;
            if (!PlayerbotAI::IsTank(member))
                continue;
            float const d = bot->GetExactDist2d(member);
            if (d < best)
            {
                best = d;
                tx = member->GetPositionX();
                ty = member->GetPositionY();
                found = true;
            }
        }
        return found;
    }

    // Candidate flee headings, as signed radian offsets from the ideal (away-from-flame)
    // bearing: straight away first, then fanning ever wider to either side all the way
    // round to a full reversal (+/- pi). The near-away headings win in the open (highest
    // flame distance), but the wide ones matter at a wall: when the bot is kited into a
    // corner or a hall end and every away-heading runs into a wall, the only escape is to
    // turn back past its own (slow, ~4yd/s) flame, a reverse move up the hall. Without the
    // wide headings the bot has no in-room candidate and wedges in the corner. The bot's
    // own fixating flame is deliberately not a hard hazard (see the fan below), so this
    // reverse is allowed to graze its Inferno; that is the "force through when there is no
    // other path" behaviour. The score still prefers opening distance, so a reverse is only
    // ever chosen when the forward options are walled off. The planned end-of-hall turn is
    // separate: it is the committed two-phase maneuver (KITE_TURN_*), which preempts the
    // fan entirely; these wide offsets remain the last-ditch escape for a bot that got
    // cornered anyway (leashed healer, dodge drift, fixate mid-corner).
    constexpr float KITE_FAN_OFFSETS[] = {
        0.0f, 0.35f, -0.35f, 0.70f, -0.70f, 1.05f, -1.05f, 1.40f, -1.40f,
        1.75f, -1.75f, 2.10f, -2.10f, 2.45f, -2.45f, 2.80f, -2.80f, 3.14f };

    // Sepethrea summons two Raging Flames; each has a ~10yd Inferno AoE and lays its own
    // ground-fire trail. A point is a flame hazard if it is inside INFERNO_SAFE_DIST of any
    // of the (pre-collected) elemental centres or inside any (pre-collected) trail patch.
    // `flames` and `patches` are gathered once per Execute so this stays cheap when scored
    // against many candidate hops.
    bool SpotSafe(std::vector<std::pair<float, float>> const& flames,
                  std::vector<MechanarFlames::TrailPatch> const& patches, float x, float y)
    {
        for (auto const& f : flames)
        {
            float const dx = x - f.first, dy = y - f.second;
            if (dx * dx + dy * dy < MechanarFlames::INFERNO_SAFE_DIST * MechanarFlames::INFERNO_SAFE_DIST)
                return false;
        }
        for (MechanarFlames::TrailPatch const& p : patches)
        {
            float const dx = x - p.x, dy = y - p.y;
            float const r = p.radius + MechanarFlames::TRAIL_DANGER_MARGIN;
            if (dx * dx + dy * dy < r * r)
                return false;
        }
        return true;
    }

    // True if the whole hop a->b clears the flame hazards (sampled along the segment), so
    // a bot never kites through the other elemental's Inferno or a trail ribbon.
    bool SegSafe(std::vector<std::pair<float, float>> const& flames,
                 std::vector<MechanarFlames::TrailPatch> const& patches,
                 float ax, float ay, float bx, float by)
    {
        constexpr int N = 5;
        for (int k = 1; k <= N; ++k)
        {
            float const t = static_cast<float>(k) / N;
            if (!SpotSafe(flames, patches, ax + (bx - ax) * t, ay + (by - ay) * t))
                return false;
        }
        return true;
    }

    // Minimum 2D distance from (x,y) to the nearest hazard (elemental centre shrunk by
    // INFERNO_SAFE_DIST, or trail-patch edge). Negative means inside a hazard; larger is
    // safer. Used to pick the least-bad hop when every candidate is compromised.
    float HazardClearance(std::vector<std::pair<float, float>> const& flames,
                          std::vector<MechanarFlames::TrailPatch> const& patches, float x, float y)
    {
        float clear = 1e9f;
        for (auto const& f : flames)
            clear = std::min(clear, Dist2d(x, y, f.first, f.second) - MechanarFlames::INFERNO_SAFE_DIST);
        for (MechanarFlames::TrailPatch const& p : patches)
            clear = std::min(clear, Dist2d(x, y, p.x, p.y) - (p.radius + MechanarFlames::TRAIL_DANGER_MARGIN));
        return clear;
    }
}

// The bot is fixated: kite the flame out of the raid, staying in the room.
bool SepethreaKiteFlameAction::Execute(Event /*event*/)
{
    Unit* flame = MechanarFlames::GetFixatingFlame(bot);
    if (!flame)
        return false;

    float const dist = bot->GetDistance2d(flame);
    float const bx = bot->GetPositionX();
    float const by = bot->GetPositionY();

    // Ideal flee bearing: straight away from the flame (party bias mixed in below).
    float fx = bx - flame->GetPositionX();
    float fy = by - flame->GetPositionY();
    if (!Normalize(fx, fy))
    {
        // Degenerate (standing on the flame): head outward from the fight core.
        fx = bx - MechanarFlames::ROOM_ANCHOR_X;
        fy = by - MechanarFlames::ROOM_ANCHOR_Y;
        if (!Normalize(fx, fy))
        {
            fx = 1.0f;
            fy = 0.0f;
        }
    }

    // Distance to the huddle of the rest of the party (the boss-DPS stack).
    float px, py;
    bool const haveParty = PartyCentroid(bot, px, py);
    float const groupDist =
        haveParty ? Dist2d(bot->GetPositionX(), bot->GetPositionY(), px, py) : 1000.0f;

    // A healer's kite is leashed to the tank (heal range 40yd): find the tank and the
    // current distance to it. Non-healers (and a healer in a tankless group) kite free.
    float tankX = 0.0f, tankY = 0.0f;
    bool const leashed = botAI->IsHeal(bot) && TankPosition(bot, tankX, tankY);
    float const tankDist =
        leashed ? Dist2d(bot->GetPositionX(), bot->GetPositionY(), tankX, tankY) : 0.0f;

    // End-of-hall turn maneuver (see KITE_TURN_* in MechanarShared.h). A leashed healer
    // never runs the hall's ends (the leash keeps it near the tank), so the maneuver is
    // for free kiters only.
    uint32 const nowMs = getMSTime();
    bool turning = _turnUntilMs != 0 && nowMs < _turnUntilMs;
    if (turning && (leashed || flame->GetGUID() != _turnFlame))
    {
        // Fixate moved on (or the bot became leash-bound) mid-maneuver: abandon it.
        _turnUntilMs = 0;
        turning = false;
    }
    if (turning && !_turnLateral && dist > MechanarFlames::KITE_THRESHOLD &&
        (by - flame->GetPositionY()) * -_turnEnd > MechanarFlames::KITE_TURN_EXIT_PAST)
    {
        // Pass complete: clearly on the open side of the flame with the normal kite gap
        // restored, so return to the hold-and-hop rhythm.
        _turnUntilMs = 0;
        turning = false;
    }
    if (!turning && !leashed && std::fabs(fy) >= 0.5f && dist <= MechanarFlames::KITE_TURN_THRESHOLD)
    {
        // Fleeing along the hall: is the end wall coming up?
        float const endSign = fy > 0.0f ? 1.0f : -1.0f;
        float const endDist = fy > 0.0f ? MechanarFlames::ROOM_Y_MAX - by
                                        : by - MechanarFlames::ROOM_Y_MIN;
        if (endDist < MechanarFlames::KITE_TURN_RUNWAY)
        {
            // Enter: commit to a pass side. Prefer a side whose swing-out and run lane
            // both clear the other flame and the trail ribbons; break ties toward the side
            // where the flame leaves more hall width.
            std::vector<std::pair<float, float>> avoidFlames;
            MechanarFlames::CollectAvoidFlames(bot, flame, avoidFlames);
            std::vector<MechanarFlames::TrailPatch> patches;
            MechanarFlames::CollectTrailPatches(bot, 60.0f, patches);

            float bestKey = -1e18f;
            for (float side : { 1.0f, -1.0f })
            {
                float const sideX = side > 0.0f
                                        ? MechanarFlames::ROOM_X_MAX - MechanarFlames::KITE_TURN_WALL_INSET
                                        : MechanarFlames::ROOM_X_MIN + MechanarFlames::KITE_TURN_WALL_INSET;
                float laneEndY = flame->GetPositionY() - endSign * MechanarFlames::KITE_TURN_PASS_BEHIND;
                laneEndY = std::min(std::max(laneEndY, MechanarFlames::ROOM_Y_MIN + 3.0f),
                                    MechanarFlames::ROOM_Y_MAX - 3.0f);
                bool const laneSafe = SegSafe(avoidFlames, patches, bx, by, sideX, by) &&
                                      SegSafe(avoidFlames, patches, sideX, by, sideX, laneEndY);
                float const key = (laneSafe ? 1000.0f : 0.0f) +
                                  std::fabs(sideX - flame->GetPositionX());
                if (key > bestKey)
                {
                    bestKey = key;
                    _turnSide = side;
                }
            }
            _turnEnd = endSign;
            _turnFlame = flame->GetGUID();
            _turnLateral = true;
            _turnUntilMs = nowMs + MechanarFlames::KITE_TURN_FAILSAFE_MS;
            turning = true;
        }
    }
    if (turning)
    {
        float const sideX = _turnSide > 0.0f
                                ? MechanarFlames::ROOM_X_MAX - MechanarFlames::KITE_TURN_WALL_INSET
                                : MechanarFlames::ROOM_X_MIN + MechanarFlames::KITE_TURN_WALL_INSET;
        if (_turnLateral && std::fabs(bx - sideX) <= 2.0f)
            _turnLateral = false;  // at the wall: swing-out done, run the lane

        float destY;
        if (_turnLateral)
            destY = by;  // phase 1: straight out to the side wall
        else
        {
            // Phase 2: down the wall to a lane point past the flame. The point is
            // recomputed off the flame's live position each tick, so it retreats as the
            // flame advances and the pass cannot stall short.
            destY = flame->GetPositionY() - _turnEnd * MechanarFlames::KITE_TURN_PASS_BEHIND;
            destY = std::min(std::max(destY, MechanarFlames::ROOM_Y_MIN + 3.0f),
                             MechanarFlames::ROOM_Y_MAX - 3.0f);
        }
        return MoveTo(bot->GetMapId(), sideX, destY, bot->GetPositionZ(), false, false, false, false,
                      MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    // Hold position (keep casting/healing this tick) only when all are true: the slow
    // flame is not yet closing in, the bot has already dragged it clear of the party, and
    // (for a healer) the tank is still inside the leash. Otherwise move. The group-clearance
    // term is the important one: when Inferno re-fixates a bot standing in the stack, the
    // flame is ~30yd away (at the previous kite spot), so a `dist >= threshold` test alone
    // would leave the bot casting in place while the elemental walks its 10yd AoE all the
    // way into the group to reach it. Leading it out immediately is the fix. The leash term
    // keeps a healer that has drifted out (or whose tank moved) walking back into heal range
    // instead of parking out there while the tank dies unhealed.
    if (dist >= MechanarFlames::KITE_THRESHOLD && groupDist >= MechanarFlames::KITE_GROUP_CLEARANCE &&
        (!leashed || tankDist <= MechanarFlames::HEALER_KITE_LEASH))
        return false;

    // Away-from-party bias on the flee bearing: strongest while the bot is still inside the
    // stack (groupDist small) and fading to nothing once it is clear, so the bot first
    // walks the flame out of the group, then simply keeps opening distance from it.
    if (haveParty)
    {
        float ax = bot->GetPositionX() - px;
        float ay = bot->GetPositionY() - py;
        if (Normalize(ax, ay))
        {
            float const clr = MechanarFlames::KITE_GROUP_CLEARANCE;
            float bias = (clr - groupDist) / clr;  // 1 inside the stack, 0 once clear
            if (bias < 0.0f)
                bias = 0.0f;
            bias *= 1.5f;  // dominate the flee vector while extracting from the group
            fx += bias * ax;
            fy += bias * ay;
            Normalize(fx, fy);
        }
    }

    float const step = MechanarFlames::KITE_STEP;

    // If the bot has drifted out of the room (noisy positions, a shove, a bad earlier
    // hop), get back inside first: ignore the flame and walk toward the nearest interior
    // point. Being outside is the one state that risks the NW opening, so recovering
    // containment beats opening flame distance.
    if (!MechanarFlames::InRoom(bx, by))
    {
        float inX = bx, inY = by;
        MechanarFlames::ClampIntoRoom(inX, inY);
        float dx = inX - bx, dy = inY - by;
        if (Normalize(dx, dy))
        {
            float rX = bx + dx * step, rY = by + dy * step;
            MechanarFlames::ClampIntoRoom(rX, rY);
            return MoveTo(bot->GetMapId(), rX, rY, bot->GetPositionZ(), false, false, false, false,
                          MovementPriority::MOVEMENT_COMBAT, true, false);
        }
    }

    // Hazards to steer around while kiting: the other elemental's Inferno (ignore the bot's
    // own fixating flame, which it flees directly) and every ground-fire trail patch.
    // Collected once, then checked against each candidate hop so the bot never kites through
    // the second elemental's fire.
    std::vector<std::pair<float, float>> avoidFlames;
    MechanarFlames::CollectAvoidFlames(bot, flame, avoidFlames);
    std::vector<MechanarFlames::TrailPatch> patches;
    MechanarFlames::CollectTrailPatches(bot, MechanarFlames::TRAIL_SCAN + step, patches);

    // Fan candidate headings around the ideal flee bearing and pick the highest-scoring one
    // whose whole hop stays inside the room polygon and clears the flame hazards. The score
    // is the distance opened from the bot's flame plus a reward for running along the room's
    // long axis (world Y). The hall is long and narrow, so a raw away-from-flame hop often
    // points across the short width and jams the bot into a side wall; the long-axis reward
    // makes it instead run the flame up and down the length. Every in-polygon point is
    // verified walkable, so an accepted hop never targets a wall or the raised platform, and
    // it can never point at the NW corridor. Each heading is also tried at a few hop lengths:
    // when a full-length hop overshoots into the other Inferno or a trail, a shorter hop in
    // the same direction often stays clear.
    float const stepLens[] = { step, 10.0f, 7.0f };
    float const baseAng = std::atan2(fy, fx);
    float bestX = 0.0f, bestY = 0.0f, bestScore = -1e18f;
    bool found = false;
    // Fallback if every candidate is compromised by fire: take the least-bad in-room hop
    // (max hazard clearance) so the bot still moves out of the danger rather than freezing.
    float safeX = 0.0f, safeY = 0.0f, bestClear = -1e18f;
    bool haveFallback = false;
    for (float off : KITE_FAN_OFFSETS)
    {
        float const ang = baseAng + off;
        for (float len : stepLens)
        {
            float const destX = bx + std::cos(ang) * len;
            float const destY = by + std::sin(ang) * len;
            if (!MechanarFlames::SegmentInRoom(bx, by, destX, destY))
                continue;

            float const clear = HazardClearance(avoidFlames, patches, destX, destY);
            if (clear > bestClear)
            {
                bestClear = clear;
                safeX = destX;
                safeY = destY;
                haveFallback = true;
            }

            if (!SegSafe(avoidFlames, patches, bx, by, destX, destY))
                continue;  // hop crosses the other Inferno or a trail; reject

            // Healer leash: never hop to a spot past the hard leash unless it closes on the
            // tank. When already outside (e.g. the tank moved), only hops back in are
            // allowed; when inside, no hop may take the bot out past it.
            float const destTankDist = leashed ? Dist2d(destX, destY, tankX, tankY) : 0.0f;
            if (leashed && destTankDist > MechanarFlames::HEALER_KITE_LEASH_MAX &&
                destTankDist >= tankDist)
                continue;

            float const fd = Dist2d(destX, destY, flame->GetPositionX(), flame->GetPositionY());
            // |sin(ang)| is the heading's alignment with world Y (the long axis); a small
            // reward for a longer hop breaks ties toward covering more ground per kite. A
            // leashed healer skips the long-axis reward: its kite orbits the tank, and the
            // reward would fight exactly the cross-hall turn hops the leash needs
            // (containment is already enforced by the room polygon).
            float const longAxis = leashed ? 0.0f : MechanarFlames::KITE_LONG_AXIS_WEIGHT;
            float score = fd + longAxis * std::fabs(std::sin(ang)) + len * 0.1f;
            if (leashed)
            {
                // Past the soft leash, opening tank distance costs more than any flame
                // distance a hop can gain, so the best hop turns the kite back toward the
                // tank rather than running on down the hall.
                if (destTankDist > MechanarFlames::HEALER_KITE_LEASH)
                    score -= MechanarFlames::HEALER_KITE_LEASH_WEIGHT *
                             (destTankDist - MechanarFlames::HEALER_KITE_LEASH);
                // The turn-back must pass the pursuing flame: charge for how close the hop's
                // path cuts to it (the destination-only fd cannot see a straight-through
                // pass) so reversals arc wide of the Inferno when the hall allows, yet can
                // still force through when cornered.
                float const passDist = MinSegDist(bx, by, destX, destY,
                                                  flame->GetPositionX(), flame->GetPositionY());
                if (passDist < MechanarFlames::INFERNO_RADIUS)
                    score -= MechanarFlames::HEALER_KITE_FLAME_PATH_WEIGHT *
                             (MechanarFlames::INFERNO_RADIUS - passDist);
            }
            if (score > bestScore)
            {
                bestScore = score;
                bestX = destX;
                bestY = destY;
                found = true;
            }
        }
    }

    if (!found)
    {
        // No fully-clear heading: hop to the least-dangerous in-room spot if one exists,
        // else hold rather than walk into a wall.
        if (!haveFallback)
            return false;
        bestX = safeX;
        bestY = safeY;
    }

    return MoveTo(bot->GetMapId(), bestX, bestY, bot->GetPositionZ(), false, false, false, false,
                  MovementPriority::MOVEMENT_COMBAT, true, false);
}

// Bystander (including a melee that would otherwise charge the boss straight through a
// freshly-spawned elemental): reposition to a spot clear of both elementals' Infernos and
// every trail patch. Stepping directly away from the nearest elemental is not enough with
// two of them (they spawn together on the boss and fixate two players): it can shove the
// bot out of one Inferno and into the other. Instead this searches a ring of candidate
// spots and moves to the nearest one that clears them all.
bool SepethreaAvoidFlameAction::Execute(Event /*event*/)
{
    // Every elemental is a hazard for a bystander (none is fixated on this bot; the trigger
    // hands a fixated bot to the kite action), plus every trail patch.
    std::vector<std::pair<float, float>> avoidFlames;
    MechanarFlames::CollectAvoidFlames(bot, nullptr, avoidFlames);
    std::vector<MechanarFlames::TrailPatch> patches;
    MechanarFlames::CollectTrailPatches(bot, MechanarFlames::TRAIL_SCAN + MechanarFlames::INFERNO_AVOID_CLEAR, patches);

    float const bx = bot->GetPositionX();
    float const by = bot->GetPositionY();
    if (SpotSafe(avoidFlames, patches, bx, by))
        return false;  // already clear of all fire; nothing to dodge

    // Search rings of increasing radius (the nearest safe spot is the least disruption to
    // combat position) for a spot that clears every Inferno and trail and stays in the
    // room. 16 headings so a gap between two Infernos can always be threaded.
    constexpr int DIRS = 16;
    float bestX = bx, bestY = by, bestClear = HazardClearance(avoidFlames, patches, bx, by);
    bool found = false;
    for (float radius = 4.0f; radius <= MechanarFlames::INFERNO_AVOID_CLEAR + 8.0f && !found; radius += 3.0f)
    {
        for (int i = 0; i < DIRS; ++i)
        {
            float const ang = (i * 2.0f * static_cast<float>(M_PI)) / DIRS;
            float const x = bx + std::cos(ang) * radius;
            float const y = by + std::sin(ang) * radius;
            if (!MechanarFlames::InRoom(x, y))
                continue;
            if (SpotSafe(avoidFlames, patches, x, y))
            {
                bestX = x;
                bestY = y;
                found = true;
                break;  // first (nearest ring) safe spot wins
            }
            // track the least-bad in-room spot as a fallback
            float const clear = HazardClearance(avoidFlames, patches, x, y);
            if (clear > bestClear)
            {
                bestClear = clear;
                bestX = x;
                bestY = y;
            }
        }
    }

    if (bestX == bx && bestY == by)
        return false;  // nowhere better reachable; hold rather than oscillate

    // Heal-capable bots use a gentler priority so a needed heal can still fire once out;
    // everyone else moves FORCED so combat formation can't immediately drag them back in.
    MovementPriority const priority =
        botAI->IsHeal(bot) ? MovementPriority::MOVEMENT_COMBAT : MovementPriority::MOVEMENT_FORCED;
    return MoveTo(bot->GetMapId(), bestX, bestY, bot->GetPositionZ(), false, false, false, false,
                  priority, true, false);
}

// Caught in the trailing ribbon of fire patches: repel out of all of them at once.
bool SepethreaAvoidTrailAction::Execute(Event /*event*/)
{
    std::vector<MechanarFlames::TrailPatch> patches;
    MechanarFlames::CollectTrailPatches(bot, MechanarFlames::TRAIL_SCAN, patches);
    if (patches.empty())
        return false;

    // Distance-weighted repulsion away from every nearby patch (closer patches push
    // harder), summed so the resultant points straight off the ribbon's short axis. A
    // single hop then clears the whole overlapping trail rather than landing in a
    // neighbour, which is the stock avoid-aoe's weakness here: it only knows the one patch
    // underfoot.
    float ax = 0.0f, ay = 0.0f;
    bool anyDanger = false;
    for (MechanarFlames::TrailPatch const& p : patches)
    {
        float const dx = bot->GetPositionX() - p.x;
        float const dy = bot->GetPositionY() - p.y;
        float const d = std::sqrt(dx * dx + dy * dy);
        float const danger = p.radius + MechanarFlames::TRAIL_DANGER_MARGIN;
        if (d >= danger)
            continue;
        anyDanger = true;
        if (d < 0.1f)
            continue;  // dead-centre on this patch: no usable bearing from it, lean on the others
        float const w = (danger - d) / danger;
        ax += dx / d * w;
        ay += dy / d * w;
    }

    if (!anyDanger)
        return false;

    if (!Normalize(ax, ay))
    {
        // Net vector cancelled (symmetric overlap, or dead-centre on a lone patch): head
        // away from the flame, or failing that outward from the room centre.
        if (Unit* flame = MechanarFlames::GetNearestFlame(bot, 100.0f))
        {
            ax = bot->GetPositionX() - flame->GetPositionX();
            ay = bot->GetPositionY() - flame->GetPositionY();
        }
        if (!Normalize(ax, ay))
        {
            ax = bot->GetPositionX() - MechanarFlames::ROOM_ANCHOR_X;
            ay = bot->GetPositionY() - MechanarFlames::ROOM_ANCHOR_Y;
            if (!Normalize(ax, ay))
            {
                ax = 1.0f;
                ay = 0.0f;
            }
        }
    }

    float destX = bot->GetPositionX() + ax * MechanarFlames::TRAIL_CLEAR_STEP;
    float destY = bot->GetPositionY() + ay * MechanarFlames::TRAIL_CLEAR_STEP;
    // Keep the dodge inside the room (never toward the NW opening / into a wall). If the
    // straight hop would clip the L-polygon's notch, pull the destination back inside.
    if (!MechanarFlames::SegmentInRoom(bot->GetPositionX(), bot->GetPositionY(), destX, destY))
        MechanarFlames::ClampIntoRoom(destX, destY);

    // Heal-capable bots use a gentler priority so a needed heal can still fire once out;
    // everyone else moves FORCED so combat formation can't immediately drag them back in.
    MovementPriority const priority =
        botAI->IsHeal(bot) ? MovementPriority::MOVEMENT_COMBAT : MovementPriority::MOVEMENT_FORCED;

    return MoveTo(bot->GetMapId(), destX, destY, bot->GetPositionZ(), false, false, false, false,
                  priority, true, false);
}

// Force every bot's DPS onto Sepethrea (never the flame).
bool SepethreaFocusBossAction::Execute(Event /*event*/)
{
    Unit* boss = MechanarFlames::GetSepethrea(bot);
    if (!boss)
        return false;

    // "prioritized targets" makes IsHighPriority true for the boss, which every find-target
    // strategy honours first, so the flame (whose 1,000,000 fixate threat gives it the
    // largest threat gap, the metric a DPS bot would otherwise pick by) is never selected.
    // Set each tick while she is engaged; a dead or absent boss GUID resolves to nothing,
    // so it self-clears once she dies.
    GuidVector const forced = { boss->GetGUID() };
    botAI->GetAiObjectContext()->GetValue<GuidVector>("prioritized targets")->Set(forced);

    // "prioritized targets" only steers the DPS find-target path (IsHighPriority). The tank
    // picks its target by threat: FindTankTargetSmartStrategy ranks a mob it has no aggro
    // on highest ("needs pickup"), and the flame's 1,000,000 fixate threat is an unbreakable
    // taunt the tank can never hold, so the tank forever treats it as the loose add and
    // chases it off the boss and through the fire. So for the tank (and for anyone who
    // somehow ends up on a flame) redirect onto the boss. The stock "tank assist" action
    // that would re-grab the flame is zeroed for the fight by SepethreaTankFocusMultiplier,
    // so this does not fight it tick-to-tick. A fixated bot is kiting, so it is never yanked
    // to melee, with one exception: a fixated tank whose boss is loose (TankMustGrabBoss)
    // has its kite suspended and must be put back on her, since reclaiming the boss outranks
    // the kite.
    if (!MechanarFlames::GetFixatingFlame(bot) || MechanarFlames::TankMustGrabBoss(bot))
    {
        Unit* const current = botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Get();
        bool const onFlame = MechanarFlames::IsFlame(current);
        bool const tankOffBoss = PlayerbotAI::IsTank(bot) && current != boss;
        if (onFlame || tankOffBoss)
            return Attack(boss);  // returns true only when it actually switches
    }

    // Don't consume the tick; let the normal dps/heal actions act on the forced target.
    return false;
}
