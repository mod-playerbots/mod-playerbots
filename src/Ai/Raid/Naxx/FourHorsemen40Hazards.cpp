/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 * Ported from reddevilnlp/mod-playerbots, commit 13492578.
 */

/*
 * MIT License
 *
 * Copyright (c) 2026 buildthehomelab
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "FourHorsemen40Hazards.h"

#include <algorithm>
#include <cmath>
#include <iterator>
#include <limits>
#include <list>
#include <map>
#include <optional>
#include <string>
#include <utility>

#include "Creature.h"
#include "Group.h"
#include "LastMovementValue.h"
#include "Log.h"
#include "Map.h"
#include "MotionMaster.h"
#include "MoveSpline.h"
#include "MoveSplineInit.h"
#include "ObjectAccessor.h"
#include "PathGenerator.h"
#include "Player.h"
#include "Playerbots.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include "Timer.h"

namespace
{
constexpr uint32 NPC_VOID_ZONE = 16697;
constexpr uint32 SPELL_CONSUMPTION = 28865;
constexpr uint32 ZoneUpdateMs = 250;
constexpr float ZoneScanRadius = 115.0f;
constexpr float EscapeRadius = 15.0f;
constexpr float NearVoidEscapeRadius = 3.5f;
constexpr float MovementClearance = 1.5f;
constexpr uint32 RearExchangeVoidExposureMs = 2000;
constexpr uint32 RearExchangeMaxConsumptionTick = 4840;
constexpr float VoidBoundaryTolerance = 0.01f;
constexpr float DirectionTolerance = 0.10f;
constexpr float CornerLateralAllowance = 5.0f;
constexpr float CornerEscapeRingStep = 1.5f;
constexpr float CenterwardProbeDistances[] = {3.0f, 6.0f};
constexpr int CenterwardProbeAngles[] = {0, -30, 30, -60, 60, -85, 85, -89, 89};
constexpr float EntranceOuterY = -3040.0f;
constexpr float EntranceInnerY = -3016.0f;
constexpr float MarkMargin = 3.0f;
constexpr float RearSeparationSearch = 21.0f;
constexpr float RearSeparationMaxPath = 24.0f;
constexpr float MarkEscapeSearch = 39.0f;
constexpr float MarkEscapeMaxPath = 45.0f;
constexpr float CompleteMarkEscapeSearch = 96.0f;
constexpr float CompleteMarkEscapeMaxPath = 110.0f;
constexpr float MarkRetreatSpacing = 12.0f;
constexpr float MarkRetreatStep = 6.0f;
constexpr float DegreesToRadians = static_cast<float>(M_PI) / 180.0f;
constexpr int MarkRetreatAngles[] = {0,   -5, 5,   -10, 10,  -15, 15,  -20, 20,  -25, 25,
                                     -30, 30, -45, 45,  -60, 60,  -75, 75,  -90, 90};

std::string const VoidExposureKey = "naxx40 horsemen void exposure";

struct VoidContact
{
    std::optional<uint32> lastOutsideAt;
    std::optional<uint32> enteredAt;
};

struct VoidExposure : DataMap::Base
{
    uint32 instanceId = 0;
    std::optional<uint32> scannedAt;
    std::map<ObjectGuid, VoidContact> contacts;
};

// The path-only part of RaidClear's idle-creature leash and escape search
// is adapted from reddevilnlp/mod-playerbots (13492578), under the MIT License.
constexpr float SCAN_RANGE = 75.0f;
constexpr float AGGRO_HEIGHT = 6.0f;
constexpr float PATH_STEP = 2.0f;
constexpr uint32 IDLE_SCAN_MS = 250;
constexpr float AHEAD_STEP = 5.0f;
constexpr float BOSS_FLOOR = 3.0f;
constexpr float LeashBuffer = 6.0f;
constexpr float IdleBossBuffer = 8.0f;
constexpr float PatrolLookAhead = 15.0f;
constexpr float WayOutFloorUp = 2.5f;
constexpr float WayOutFloorDown = 5.0f;
constexpr float WayOutEndSlack = 1.5f;
constexpr uint32 WayOutRetryMs = 400;

std::string const IdleCreatureCacheKey = "playerbots.naxx40.horsemen.idle-creatures";

struct IdleCreatureCache : DataMap::Base
{
    bool scanned = false;
    uint32 scanMs = 0;
    GuidVector idle;
};

bool WouldAggro(Creature const* creature)
{
    return creature->IsInWorld() && creature->IsAlive() && !creature->IsInCombat() && !creature->IsInEvadeMode() &&
           creature->HasReactState(REACT_AGGRESSIVE) && creature->IsVisible() && !creature->IsCivilian() &&
           !creature->IsTrigger() && !creature->IsNeutralToAll() && !creature->IsPet() && !creature->IsTotem() &&
           !creature->GetCharmerOrOwnerGUID().IsPlayer() && creature->GetCreatureType() != CREATURE_TYPE_CRITTER &&
           !creature->IsImmuneToPC() && !creature->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE) &&
           !creature->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE);
}

// The instance's spawns, walked a few times a second for the whole raid. (Playerbots'
// "possible targets" would do a grid search per bot on every call.)
void RefreshIdle(Map* map, IdleCreatureCache& state)
{
    uint32 const now = getMSTime();
    if (state.scanned && getMSTimeDiff(state.scanMs, now) < IDLE_SCAN_MS)
        return;
    state.scanned = true;
    state.scanMs = now;
    state.idle.clear();
    for (auto const& [spawnId, creature] : map->GetCreatureBySpawnIdStore())
        if (creature && WouldAggro(creature))
            state.idle.push_back(creature->GetGUID());
}

struct IdleMob
{
    Creature* creature;
    float x;
    float y;
    float z;
    float aggro;  // its aggro radius for this bot, centre to centre
    // Its aggro radius plus the buffer already covers one of the bot's targets: there is no
    // getting at that target without coming this close.
    bool tied;
    // Not where the mob is but where it is walking to (a patrol, a mob wandering round its
    // spot): it will be standing there by the time a bot's walk is over.
    bool ahead = false;
    // A boss: yards of room on top of its aggro radius and the buffer. Not for a walk to the
    // bot's target, not for a boss the target stands next to, not for the step back out.
    float room = 0.0f;
    // A boss: BOSS_FLOOR holds for it even when it is tied.
    bool boss = false;
};

// What the bot is attacking and what it is about to attack. They differ during a pull:
// playerbots sets "current target" first and the swing follows.
struct BotTargets
{
    Unit* victim = nullptr;
    Unit* current = nullptr;

    bool Has(Unit const* unit) const { return unit && (unit == victim || unit == current); }
    bool Any() const { return victim || current; }
};

BotTargets TargetsOf(PlayerbotAI* botAI, Player* bot)
{
    BotTargets targets;
    targets.victim = bot->GetVictim();
    targets.current = botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Get();
    if (targets.victim && !targets.victim->IsAlive())
        targets.victim = nullptr;
    if (targets.current && !targets.current->IsAlive())
        targets.current = nullptr;
    return targets;
}

bool Tied(BotTargets const& targets, float x, float y, float reach)
{
    return (targets.victim && targets.victim->GetExactDist2d(x, y) < reach) ||
           (targets.current && targets.current->GetExactDist2d(x, y) < reach);
}

// The next `ahead` yards of a walking mob's way, as spots of their own: one every few yards
// and one where the look ends. The core only knows the leg the mob is on (to its next
// waypoint), so that is as far as this sees.
void AddWayAhead(std::vector<IdleMob>& mobs, IdleMob const& mob, BotTargets const& targets, float tieReach, float ahead)
{
    Movement::MoveSpline const* walk = mob.creature->movespline;
    if (ahead <= 0.0f || !mob.creature->isMoving() || !walk->Initialized() || walk->Finalized())
        return;

    auto const add = [&](G3D::Vector3 const& at)
    {
        mobs.push_back(
            {mob.creature, at.x, at.y, at.z, mob.aggro, Tied(targets, at.x, at.y, tieReach), true, mob.room, mob.boss});
    };

    Movement::MoveSpline::MySpline const& spline = walk->_Spline();
    G3D::Vector3 from(mob.x, mob.y, mob.z);
    G3D::Vector3 end = from;
    float gone = 0.0f;      // yards of its way looked at so far
    float lastSpot = 0.0f;  // how far along the last spot was put
    int nextStep = 1;
    for (int32 i = walk->_currentSplineIdx() + 1; i <= spline.last() && gone < ahead; ++i)
    {
        G3D::Vector3 const to = spline.getPoint(i);
        float const leg = std::hypot(to.x - from.x, to.y - from.y);
        if (leg > 0.01f)
        {
            float const upTo = std::min(gone + leg, ahead);
            while (true)
            {
                float const at = float(nextStep) * AHEAD_STEP;
                if (!(at <= upTo))
                    break;

                add(from + (to - from) * ((at - gone) / leg));
                lastSpot = at;
                ++nextStep;
            }
            end = from + (to - from) * ((upTo - gone) / leg);
            gone = upTo;
        }
        from = to;
    }
    // Where the look ends, or the walk if that ends first.
    if (gone - lastSpot > 1.0f)
        add(end);
}

// The idle mobs around this bot, and where the walking ones among them are headed. Its own
// targets are left out: attacking a mob that isn't in combat yet is a pull.
std::vector<IdleMob> IdleMobsNear(Player& bot, BotTargets const& targets)
{
    Map* const map = bot.GetMap();
    auto& state = *map->CustomData.GetDefault<IdleCreatureCache>(IdleCreatureCacheKey);
    RefreshIdle(map, state);

    std::vector<IdleMob> mobs;
    for (ObjectGuid const& guid : state.idle)
    {
        Creature* creature = map->GetCreature(guid);
        if (!creature || targets.Has(creature) || !WouldAggro(creature))
            continue;
        if (bot.GetExactDist2d(creature) > SCAN_RANGE || !creature->IsHostileTo(&bot))
            continue;

        // A second pack is a harder fight; a second boss is a wipe, so a boss is given more room.
        float const aggro = creature->GetAggroRange(&bot);
        bool const boss = creature->IsDungeonBoss() || creature->isWorldBoss();
        IdleMob const mob = {creature,
                             creature->GetPositionX(),
                             creature->GetPositionY(),
                             creature->GetPositionZ(),
                             aggro,
                             Tied(targets, creature->GetPositionX(), creature->GetPositionY(), aggro + LeashBuffer),
                             false,
                             boss ? IdleBossBuffer : 0.0f,
                             boss};
        mobs.push_back(mob);
        AddWayAhead(mobs, mob, targets, aggro + LeashBuffer, PatrolLookAhead);
    }
    return mobs;
}

// Whether the mob sees a spot from where it stands, or would from the spot it is walking to.
bool Sees(IdleMob const& mob, float x, float y, float z)
{
    if (!mob.ahead)
        return mob.creature->IsWithinLOS(x, y, z + 2.0f);
    return mob.creature->GetMap()->isInLineOfSight(mob.x, mob.y, mob.z + mob.creature->GetCollisionHeight(), x, y,
                                                   z + 2.0f, mob.creature->GetPhaseMask(), LINEOFSIGHT_ALL_CHECKS,
                                                   VMAP::ModelIgnoreFlags::Nothing);
}

bool Approaches(Player const& bot, BotTargets const& targets, float x, float y)
{
    for (Unit* target : {targets.victim, targets.current})
        if (target && target->GetExactDist2d(x, y) < target->GetExactDist2d(&bot) - 1.0f)
            return true;
    return false;
}

// Whether a stretch of a walk passes too close to one of the mobs.
//   - A walk toward a target (going to the fight, or pulling): mobs tied to the target don't
//     count, and the others only have to be passed by half the buffer. Round the corner of a
//     pack is fine, through it is not.
//   - Any other walk (backing off, spreading out, dodging): every mob counts. A boss tanked
//     next to an idle pack doesn't make that pack safe to back into. The full buffer for
//     most; half of it for a mob tied to the target, so a melee bot shifting a few yards
//     around its target isn't stopped for nothing.
//   - An idle boss counts for every walk, tied or not, by at least BOSS_FLOOR.
bool StretchPulls(Player const& bot, std::vector<IdleMob> const& mobs, bool approach, G3D::Vector3 const& from,
                  G3D::Vector3 const& to)
{
    float const botX = bot.GetPositionX();
    float const botY = bot.GetPositionY();
    float const length = std::hypot(to.x - from.x, to.y - from.y);
    int const steps = std::max(1, int(std::ceil(length / PATH_STEP)));
    for (int step = 1; step <= steps; ++step)
    {
        float const t = float(step) / float(steps);
        float const x = from.x + (to.x - from.x) * t;
        float const y = from.y + (to.y - from.y) * t;
        float const z = from.z + (to.z - from.z) * t;
        for (IdleMob const& mob : mobs)
        {
            if ((approach && mob.tied && !mob.boss) || std::fabs(mob.z - z) > AGGRO_HEIGHT)
                continue;
            // A boss's extra room is not for a walk to the target, nor for a boss the target
            // stands next to: the fight is where it is. Its own radius still is.
            float margin = approach || mob.tied ? LeashBuffer * 0.5f : LeashBuffer + mob.room;
            if (mob.boss)
                margin = std::max(margin, BOSS_FLOOR);
            float const there = std::hypot(x - mob.x, y - mob.y);
            // Only a walk that takes the bot closer to the mob than it already is.
            if (there < mob.aggro + margin && there < std::hypot(botX - mob.x, botY - mob.y) - 0.5f &&
                Sees(mob, x, y, z))
                return true;
        }
    }
    return false;
}

class WalkCheck
{
public:
    WalkCheck(PlayerbotAI* botAI, Player& bot)
        : _bot(bot), _targets(TargetsOf(botAI, &bot)), _mobs(IdleMobsNear(bot, _targets))
    {
    }

    bool Pulls(Movement::PointsArray const& path) const
    {
        if (_mobs.empty() || path.empty())
            return false;

        bool const approach = Approaches(_bot, _targets, path.back().x, path.back().y);
        G3D::Vector3 from(_bot.GetPositionX(), _bot.GetPositionY(), _bot.GetPositionZ());
        for (G3D::Vector3 const& to : path)
        {
            if (StretchPulls(_bot, _mobs, approach, from, to))
                return true;
            from = to;
        }
        return false;
    }

private:
    Player const& _bot;
    BotTargets const _targets;
    std::vector<IdleMob> const _mobs;
};

template <typename GapAt, typename PathAllowed>
bool FindWayOut(PlayerbotAI* botAI, Player& bot, GapAt const& gapAt, float want, float worthIt, Position& spot,
                float search, float nearest, PathAllowed const& pathAllowed, Movement::PointsArray& checkedPath,
                bool dryOnly = false, float ringStep = 3.0f, float angleOffset = 0.0f, bool shortest = false)
{
    Map* map = bot.GetMap();
    float const botX = bot.GetPositionX();
    float const botY = bot.GetPositionY();
    float const botZ = bot.GetPositionZ();
    WalkCheck const leash(botAI, bot);

    constexpr int STEPS = 16;
    bool found = false;
    // What a spot has to beat: good enough, or that much better than here.
    float bestGap = (shortest ? want : std::min(gapAt(botX, botY, botZ) + worthIt, want)) - 0.01f;
    float bestLength = std::numeric_limits<float>::max();
    int ringIndex = 0;
    while (true)
    {
        float const ring = nearest + float(ringIndex) * ringStep;
        if (!(ring <= search && (shortest ? ring <= bestLength + WayOutEndSlack : !found || bestGap < want)))
            break;

        for (int i = 0; i < STEPS; ++i)
        {
            float const angle = angleOffset + float(i) * 2.0f * float(M_PI) / float(STEPS);
            float const x = botX + std::cos(angle) * ring;
            float const y = botY + std::sin(angle) * ring;
            // The cheap test first: most of the circle is no better than where the bot is.
            if (gapAt(x, y, botZ) <= bestGap)
                continue;

            // The floor at the bot's own level: asked from higher up, a ledge over it answers.
            float const z = map->GetHeight(bot.GetPhaseMask(), x, y, botZ + WayOutFloorUp);
            if (z <= INVALID_HEIGHT || z < botZ - WayOutFloorDown)
                continue;

            PathGenerator path(&bot);
            if (dryOnly)
                path.SetExcludeFlags(NAV_WATER | NAV_MAGMA | NAV_SLIME | NAV_GROUND_STEEP);
            // A whole path over the floor: not one that stops short, and not the straight line
            // the core hands back when it finds no floor under either end.
            constexpr uint32 NOT_A_WALK = PATHFIND_INCOMPLETE | PATHFIND_NOPATH | PATHFIND_SHORTCUT |
                                          PATHFIND_NOT_USING_PATH | PATHFIND_FARFROMPOLY;
            if (!path.CalculatePath(x, y, z) || !(path.GetPathType() & PATHFIND_NORMAL) ||
                (path.GetPathType() & NOT_A_WALK))
                continue;
            G3D::Vector3 const& end = path.GetActualEndPosition();
            float const length = path.getPathLength();
            if (std::hypot(end.x - x, end.y - y) > WayOutEndSlack || length > ring * 1.5f + 2.0f ||
                (shortest && length >= bestLength))
                continue;
            float const gap = gapAt(end.x, end.y, end.z);
            if (gap <= bestGap || !pathAllowed(path.GetPath()) || leash.Pulls(path.GetPath()))
                continue;

            found = true;
            if (shortest)
                bestLength = length;
            else
                bestGap = gap;
            spot.Relocate(end.x, end.y, end.z);
            checkedPath = path.GetPath();
        }
        ++ringIndex;
    }
    return found;
}

bool InsideHorsemenRoom(float x, float y, float z)
{
    // The entrance gate is at Y -3017.17. This boundary preserves the
    // dry Korth'azz pads while excluding the corridor from retreat paths.
    return x >= 2450.0f && x <= 2630.0f && y >= EntranceInnerY && y <= -2880.0f && z >= 239.0f && z <= 248.0f;
}

bool InsideHorsemenEntrance(G3D::Vector3 const& point)
{
    return point.y >= EntranceOuterY && point.y < EntranceInnerY &&
           InsideHorsemenRoom(point.x, EntranceInnerY, point.z);
}

float ConsumptionRadius()
{
    SpellInfo const* spell = sSpellMgr->GetSpellInfo(SPELL_CONSUMPTION);
    float const radius = spell ? spell->Effects[EFFECT_0].CalcRadius() : 0.0f;
    return radius > 0.0f ? radius : 2.0f;
}

G3D::Vector3 Point(Position const& position)
{
    return {position.GetPositionX(), position.GetPositionY(), position.GetPositionZ()};
}

G3D::Vector3 CurrentPoint(Player const& bot)
{
    if (bot.movespline->Finalized() || !bot.movespline->HasStarted())
        return Point(bot);
    auto const current = bot.movespline->ComputePosition();
    return {current.x, current.y, current.z};
}

float PathLength(std::span<G3D::Vector3 const> path)
{
    float length = 0.0f;
    for (std::size_t index = 1; index < path.size(); ++index)
        length += (path[index] - path[index - 1]).length();
    return length;
}

Movement::PointsArray RemainingSpline(Player const& bot, uint32 splineId)
{
    Movement::PointsArray remaining;
    if (splineId && bot.isMoving() && !bot.movespline->Finalized() && bot.movespline->GetId() == splineId)
    {
        auto const& spline = bot.movespline->_Spline();
        for (int32 index = bot.movespline->_currentSplineIdx() + 1; index <= spline.last(); ++index)
            remaining.push_back(spline.getPoint(index));
    }
    return remaining;
}

bool StartsToward(std::span<G3D::Vector3 const> path, G3D::Vector3 const& from, G3D::Vector3 const& destination)
{
    G3D::Vector3 const direction(destination.x - from.x, destination.y - from.y, 0.0f);
    for (G3D::Vector3 const& point : path)
    {
        G3D::Vector3 const step(point.x - from.x, point.y - from.y, 0.0f);
        if (step.squaredLength() >= DirectionTolerance * DirectionTolerance)
            return direction.dot(step) > 0.0f;
    }
    return false;
}

bool InsideHorsemenRoom(G3D::Vector3 const& point) { return InsideHorsemenRoom(point.x, point.y, point.z); }

Unit const* FindWrathSource(Player const& bot, ObjectGuid guid)
{
    Unit const* const source = ObjectAccessor::GetUnit(bot, guid);
    return source && source->IsAlive() && source->IsInMap(&bot) && source->InSamePhase(&bot) ? source : nullptr;
}

bool HasWrathLineOfSight(Unit const& source, Player const& bot, G3D::Vector3 const& point)
{
    return source.IsWithinLOS(point.x, point.y, point.z + bot.GetCollisionHeight() - source.GetCollisionHeight(),
                              VMAP::ModelIgnoreFlags::M2);
}

bool CrossesWrathRange(Player const& bot, FourHorsemenMarkSource const& source, G3D::Vector3 const& from,
                        G3D::Vector3 const& leg, float lengthSquared, float tolerance)
{
    Unit const* const origin = FindWrathSource(bot, source.wrathSource);
    if (!origin)
        return false;

    G3D::Vector3 const offset = from - Point(*origin);
    float const radiusSquared = source.radius * source.radius;
    float const projection = -offset.dot(leg) / lengthSquared;
    float const closest = std::clamp(projection, 0.0f, 1.0f);
    float const closestSquared = (offset + leg * closest).squaredLength();
    if (closestSquared >= radiusSquared - tolerance)
        return false;

    float const startSquared = offset.squaredLength();
    bool const startsExposed = startSquared < radiusSquared && HasWrathLineOfSight(*origin, bot, from);
    // A bot already exposed can take a route that immediately moves away.
    float const threshold = (startsExposed ? startSquared : radiusSquared) - tolerance;
    if (closestSquared >= threshold)
        return false;

    float const depth = std::max(0.0f, threshold - (offset + leg * projection).squaredLength());
    float const halfWidth = std::sqrt(depth / lengthSquared);
    float const enter = std::max(0.0f, projection - halfWidth);
    float const exit = std::min(1.0f, projection + halfWidth);
    constexpr float sampleStep = 1.0f;
    int const steps = std::max(1, static_cast<int>(std::ceil((exit - enter) * std::sqrt(lengthSquared) / sampleStep)));
    for (int step = 0; step <= steps; ++step)
    {
        float const t = enter + (exit - enter) * static_cast<float>(step) / static_cast<float>(steps);
        if (HasWrathLineOfSight(*origin, bot, from + leg * t))
            return true;
    }
    return false;
}

bool CrossesRestrictedMark(Player const& bot, std::span<G3D::Vector3 const> path,
                           std::span<FourHorsemenMarkSource const> sources, bool allowDeeperInside = false)
{
    G3D::Vector3 from = CurrentPoint(bot);
    float const minimumLeg = allowDeeperInside ? std::numeric_limits<float>::min() : 0.0001f;
    float const tolerance = allowDeeperInside ? 0.0f : 0.01f;
    for (G3D::Vector3 const& to : path)
    {
        G3D::Vector3 const spatial = to - from;
        G3D::Vector3 const planar(spatial.x, spatial.y, 0.0f);
        float const spatialLength = spatial.squaredLength();
        float const planarLength = planar.squaredLength();
        if (spatialLength < minimumLeg)
            continue;
        for (FourHorsemenMarkSource const& source : sources)
        {
            if (!source.wrathSource.IsEmpty())
            {
                if (CrossesWrathRange(bot, source, from, spatial, spatialLength, tolerance))
                    return true;
                continue;
            }
            if (planarLength < minimumLeg)
                continue;
            G3D::Vector3 const offset(from.x - source.location.GetPositionX(), from.y - source.location.GetPositionY(),
                                      0.0f);
            float const along = std::clamp(-offset.dot(planar) / planarLength, 0.0f, 1.0f);
            float const radiusSquared = source.radius * source.radius;
            if (allowDeeperInside && source.isMark && offset.squaredLength() < radiusSquared)
                continue;
            // Let a player already in the radius walk out without taking a path deeper into it.
            if ((offset + planar * along).squaredLength() < std::min(offset.squaredLength(), radiusSquared) - tolerance)
                return true;
        }
        from = to;
    }
    return false;
}

bool InRestrictedPosition(Player const& bot, G3D::Vector3 const& end, std::span<FourHorsemenMarkSource const> avoid)
{
    return std::ranges::any_of(avoid, [&end](FourHorsemenMarkSource const& source)
                               { return source.wrathSource.IsEmpty() &&
                                        source.location.GetExactDist2d(end.x, end.y) < source.radius; }) ||
           FourHorsemen40Hazards::CanReceiveZeliekWrath(bot, Position(end.x, end.y, end.z), avoid);
}

bool HasPathLineOfSight(Player& bot, std::span<G3D::Vector3 const> vertices)
{
    for (std::size_t i = 1; i < vertices.size(); ++i)
    {
        G3D::Vector3 const& first = vertices[i - 1];
        G3D::Vector3 const& last = vertices[i];
        if (!bot.GetMap()->isInLineOfSight(first.x, first.y, first.z + 0.85f, last.x, last.y, last.z + 0.85f,
                                           bot.GetPhaseMask(), LINEOFSIGHT_ALL_CHECKS, VMAP::ModelIgnoreFlags::Nothing))
            return false;
    }
    return true;
}

bool GroundLeg(Player& bot, G3D::Vector3 const& from, G3D::Vector3 const& to, Movement::PointsArray& vertices,
               bool entry = false)
{
    PathGenerator path(&bot);
    path.SetExcludeFlags(NAV_WATER | NAV_MAGMA | NAV_SLIME | NAV_GROUND_STEEP);
    if (!path.CalculatePath(from.x, from.y, from.z, to.x, to.y, to.z, false) || path.GetPathType() != PATHFIND_NORMAL ||
        path.GetPath().size() < 2 || (path.GetActualEndPosition() - to).length() > WayOutEndSlack)
        return false;

    vertices = path.GetPath();
    vertices.front() = from;
    // A joining bot can walk in through the gate before the room boundary applies.
    bool entered = !entry;
    for (G3D::Vector3 const& point : vertices)
    {
        if (InsideHorsemenRoom(point))
            entered = true;
        else if (entered || !InsideHorsemenEntrance(point))
            return false;
    }
    return entered && HasPathLineOfSight(bot, vertices);
}

template <typename ClearanceAt, typename PathAllowed>
bool FindCenterwardMarkExit(Player& bot, std::span<FourHorsemenMarkSource const> sources, Position const& toward,
                            ClearanceAt const& clearanceAt, PathAllowed const& pathAllowed, Movement::PointsArray& path)
{
    auto const source =
        std::ranges::find_if(sources, [&bot](FourHorsemenMarkSource const& mark)
                             { return mark.isMark && bot.GetExactDist2d(&mark.location) < mark.radius + MarkMargin; });
    if (source == sources.end())
        return false;

    // Use the same direction and candidate order for everyone clearing this caster.
    G3D::Vector3 const origin = Point(source->location);
    G3D::Vector3 const from = CurrentPoint(bot);
    float const bearing = std::atan2(toward.GetPositionY() - origin.y, toward.GetPositionX() - origin.x);
    float const boundary = source->radius + MarkMargin + 1.0f;
    for (float offset : {0.0f, MarkRetreatStep, 2.0f * MarkRetreatStep})
        for (int degrees : MarkRetreatAngles)
        {
            float const angle = bearing + static_cast<float>(degrees) * DegreesToRadians;
            float const x = origin.x + std::cos(angle) * (boundary + offset);
            float const y = origin.y + std::sin(angle) * (boundary + offset);
            if (clearanceAt(x, y, from.z) < 0.0f)
                continue;

            float const z = bot.GetMap()->GetHeight(bot.GetPhaseMask(), x, y, from.z + WayOutFloorUp);
            if (z <= INVALID_HEIGHT || z < from.z - WayOutFloorDown || !InsideHorsemenRoom(x, y, z) ||
                !GroundLeg(bot, from, {x, y, z}, path))
                continue;

            G3D::Vector3 const& end = path.back();
            if (clearanceAt(end.x, end.y, end.z) >= 0.0f && pathAllowed(path))
                return true;
        }
    return false;
}
}  // namespace

void FourHorsemen40Hazards::ObserveVoidExposure(Player& bot, bool active)
{
    if (!active)
    {
        if (bot.CustomData.Get<VoidExposure>(VoidExposureKey))
            bot.CustomData.Erase(VoidExposureKey);
        return;
    }

    VoidExposure& exposure = *bot.CustomData.GetDefault<VoidExposure>(VoidExposureKey);
    uint32 const now = getMSTime();
    if (exposure.instanceId != bot.GetInstanceId())
    {
        exposure = VoidExposure{};
        exposure.instanceId = bot.GetInstanceId();
    }

    if (!exposure.scannedAt || getMSTimeDiff(*exposure.scannedAt, now) >= ZoneUpdateMs)
    {
        exposure.scannedAt = now;
        std::list<Creature*> zones;
        bot.GetCreatureListWithEntryInGrid(zones, NPC_VOID_ZONE, ZoneScanRadius);
        for (Creature const* zone : zones)
            if (zone->IsAlive())
                exposure.contacts.try_emplace(zone->GetGUID());
    }

    static float const damageRadiusSquared = std::pow(ConsumptionRadius(), 2.0f);
    static float const scanRadiusSquared = ZoneScanRadius * ZoneScanRadius;
    G3D::Vector3 const current = CurrentPoint(bot);
    Map* const map = bot.GetMap();
    for (auto contact = exposure.contacts.begin(); contact != exposure.contacts.end();)
    {
        Creature const* const zone = map->GetCreature(contact->first);
        if (!zone || !zone->IsAlive() || !bot.InSamePhase(zone))
        {
            contact = exposure.contacts.erase(contact);
            continue;
        }

        float const distanceSquared = (current - Point(zone->GetPosition())).squaredLength();
        if (distanceSquared > scanRadiusSquared)
        {
            contact = exposure.contacts.erase(contact);
            continue;
        }

        VoidContact& state = contact->second;
        if (distanceSquared > damageRadiusSquared)
        {
            state.lastOutsideAt = now;
            state.enteredAt.reset();
        }
        else if (state.lastOutsideAt)
        {
            // Entry occurred between observations. Count from the earlier sample.
            state.enteredAt = std::exchange(state.lastOutsideAt, std::nullopt);
        }
        ++contact;
    }
}

void FourHorsemen40Hazards::RefreshVoidZones(Player& bot)
{
    uint32 const now = getMSTime();
    if (_instanceId == bot.GetInstanceId() && _checkedAt && getMSTimeDiff(_checkedAt, now) < ZoneUpdateMs)
        return;

    _instanceId = bot.GetInstanceId();
    _checkedAt = now;
    _voidZones.clear();

    std::list<Creature*> zones;
    bot.GetCreatureListWithEntryInGrid(zones, NPC_VOID_ZONE, ZoneScanRadius);
    for (Creature const* zone : zones)
        if (zone->IsAlive())
            _voidZones.push_back({zone->GetGUID(), zone->GetPosition()});
}

float FourHorsemen40Hazards::Clearance(Position const& point) const
{
    static float const radius = ConsumptionRadius();
    float gap = std::numeric_limits<float>::max();
    for (VoidZone const& zone : _voidZones)
        gap = std::min(gap, zone.position.GetExactDist(point) - radius);
    return gap;
}

Position const* FourHorsemen40Hazards::FirstBlockedZone(Player const& bot, std::span<G3D::Vector3 const> path,
                                                        VoidPath mode) const
{
    static float const damageRadiusSquared = std::pow(ConsumptionRadius(), 2.0f);
    static float const paddedRadiusSquared = std::pow(ConsumptionRadius() + MovementClearance, 2.0f);
    float const radiusSquared = mode == VoidPath::Padded ? paddedRadiusSquared : damageRadiusSquared;
    G3D::Vector3 const start = CurrentPoint(bot);
    G3D::Vector3 from = start;
    bool firstLeg = true;
    for (G3D::Vector3 const& to : path)
    {
        G3D::Vector3 const leg = to - from;
        float const lengthSquared = leg.squaredLength();
        if (lengthSquared < 0.0001f)
            continue;
        bool const floorProjection =
            firstLeg && std::abs(leg.x) < 0.01f && std::abs(leg.y) < 0.01f && std::abs(leg.z) <= 0.25f;
        firstLeg = false;
        for (VoidZone const& zone : _voidZones)
        {
            G3D::Vector3 const center = Point(zone.position);
            G3D::Vector3 const offset = from - center;
            float const startSquared = offset.squaredLength();
            // MMAP can project the starting point a fraction of a yard down onto the same floor.
            if (floorProjection && startSquared < radiusSquared)
                continue;
            // A forced exit can go deeper into a damaging pool already underfoot.
            if (mode == VoidPath::ForcedExit && startSquared <= damageRadiusSquared &&
                (start - center).squaredLength() <= damageRadiusSquared)
                continue;
            float const along = std::clamp(-offset.dot(leg) / lengthSquared, 0.0f, 1.0f);
            float const closestSquared = (offset + leg * along).squaredLength();
            if (mode != VoidPath::Padded && startSquared > damageRadiusSquared && closestSquared <= damageRadiusSquared)
                return &zone.position;
            // An underfoot zone can only be left by walking out. Permit progress
            // away from its center, but never a shortcut back through it.
            if (closestSquared < std::min(startSquared, radiusSquared) - 0.01f)
                return &zone.position;
        }
        from = to;
    }
    return nullptr;
}

std::string_view FourHorsemen40Hazards::RearExchangeCrossingBlock(Player const& bot, std::span<G3D::Vector3 const> path,
                                                                  FourHorsemenRearCrossing crossing,
                                                                  bool& crossesDamage) const
{
    static float const damageRadiusSquared = std::pow(ConsumptionRadius(), 2.0f);
    float const speed = bot.GetSpeed(MOVE_RUN);
    uint32 const now = getMSTime();
    uint32 damageBudget = 0;
    G3D::Vector3 const start = CurrentPoint(bot);
    VoidExposure const* const exposure = bot.CustomData.Get<VoidExposure>(VoidExposureKey);
    for (VoidZone const& zone : _voidZones)
    {
        Creature const* const creature = bot.GetMap()->GetCreature(zone.guid);
        if (!creature || !creature->IsAlive())
            continue;

        G3D::Vector3 const center = Point(zone.position);
        bool const inside = (start - center).squaredLength() <= damageRadiusSquared;
        uint32 elapsed = 0;
        if (inside)
        {
            if (!exposure || exposure->instanceId != bot.GetInstanceId())
                return "entry time into a void zone is unknown";
            auto const contact = exposure->contacts.find(zone.guid);
            if (contact == exposure->contacts.end() || !contact->second.enteredAt)
                return "entry time into a void zone is unknown";
            elapsed = getMSTimeDiff(*contact->second.enteredAt, now);
        }
        float distance = 0.0f;
        bool canTakeDamage = inside;
        bool leftZone = false;
        G3D::Vector3 from = start;
        for (G3D::Vector3 const& to : path)
        {
            G3D::Vector3 const leg = to - from;
            G3D::Vector3 const offset = from - center;
            from = to;
            float const lengthSquared = leg.squaredLength();
            if (lengthSquared < 0.0001f)
                continue;

            float const closest = -offset.dot(leg) / lengthSquared;
            float const projectionSquared = (offset + leg * closest).squaredLength();
            float const insideSquared = damageRadiusSquared - projectionSquared;
            float const halfWidth = std::sqrt(std::max(0.0f, insideSquared) / lengthSquared);
            float const enter = std::max(0.0f, closest - halfWidth);
            float const exit = std::min(1.0f, closest + halfWidth);
            if (insideSquared < 0.0f || enter > exit)
            {
                leftZone = canTakeDamage;
                continue;
            }

            float const length = std::sqrt(lengthSquared);
            // A later visit can overlap a second tick even when the total exposure is brief.
            if (canTakeDamage && (leftZone || enter * length > VoidBoundaryTolerance))
                return "ground route re-enters a void zone";
            canTakeDamage = true;
            leftZone = (1.0f - exit) * length > VoidBoundaryTolerance;
            distance += (exit - enter) * length;
            if (crossing.healer)
            {
                // Verify coverage along the damaging part of the path as the pair moves.
                for (float fraction : {enter, std::clamp(closest, enter, exit), exit})
                {
                    G3D::Vector3 const point = to + leg * (fraction - 1.0f);
                    if (crossing.healer->GetExactDist(point.x, point.y, point.z) > crossing.healRange ||
                        !crossing.healer->IsWithinLOS(point.x, point.y, point.z))
                        return "damaging part of the crossing is outside healer coverage";
                }
            }
        }
        if (!canTakeDamage)
            continue;
        if (!crossing.allowed || (crossing.healer && (!crossing.healer->IsAlive() || !crossing.healer->IsInWorld())))
            return "void crossing without healer coverage";
        if (speed <= 0.0f || elapsed + std::round(distance / speed * IN_MILLISECONDS) > RearExchangeVoidExposureMs)
            return "exposure to one void zone exceeds 2 seconds";
        // Overlapping pools can each deal a tick.
        damageBudget += RearExchangeMaxConsumptionTick;
        if (bot.GetHealth() <= damageBudget)
            return "insufficient health for void damage";
        crossesDamage = true;
    }
    return {};
}

bool FourHorsemen40Hazards::SafeSpline(Player const& bot, uint32 splineId,
                                       std::span<FourHorsemenMarkSource const> avoid, bool allowDeeperInside,
                                       float maxPathLength) const
{
    Movement::PointsArray const remaining = RemainingSpline(bot, splineId);
    if (remaining.empty())
        return false;
    if (maxPathLength > 0.0f)
    {
        G3D::Vector3 from = CurrentPoint(bot);
        float length = 0.0f;
        for (G3D::Vector3 const& to : remaining)
        {
            length += (to - from).length();
            from = to;
        }
        if (length > maxPathLength || Clearance(Position(from.x, from.y, from.z)) < MovementClearance)
            return false;
    }
    return !FirstBlockedZone(bot, remaining) &&
           (avoid.empty() || !CrossesRestrictedMark(bot, remaining, avoid, allowDeeperInside)) &&
           std::ranges::all_of(remaining, [](G3D::Vector3 const& point) { return InsideHorsemenRoom(point); });
}

bool FourHorsemen40Hazards::Launch(Player& bot, Movement::PointsArray const& path, uint32& splineId)
{
    if (bot.IsSitState())
        bot.SetStandState(UNIT_STAND_STATE_STAND);
    bot.CastStop();
    LastMovement& movement = _ai->GetAiObjectContext()->GetValue<LastMovement&>("last movement")->Get();
    movement.clear();
    bot.GetMotionMaster()->Clear();
    Movement::MoveSplineInit init(&bot);
    init.MovebyPath(path);
    init.SetWalk(false);
    int32 const duration = init.Launch();
    if (duration <= 0)
        return false;
    splineId = bot.movespline->GetId();
    G3D::Vector3 const end = bot.movespline->FinalDestination();
    movement.Set(bot.GetMapId(), end.x, end.y, end.z, bot.GetOrientation(),
                 std::min(static_cast<float>(duration), static_cast<float>(sPlayerbotAIConfig.MaxWaitForMove)),
                 MovementPriority::MOVEMENT_FORCED);
    return true;
}

bool FourHorsemen40Hazards::IsSafeFromVoidZones(Player& bot, Position const& destination)
{
    RefreshVoidZones(bot);
    return Clearance(destination) >= MovementClearance;
}

bool FourHorsemen40Hazards::IsSafeFromVoidDamage(Player& bot, Position const& position)
{
    RefreshVoidZones(bot);
    return Clearance(position) > 0.0f;
}

bool FourHorsemen40Hazards::HasVoidZones(Player& bot)
{
    RefreshVoidZones(bot);
    return !_voidZones.empty();
}

bool FourHorsemen40Hazards::EscapeVoidZones(Player& bot, Position const& corner, float cornerRadius)
{
    if (IsSafeFromVoidZones(bot, bot.GetPosition()))
        return false;
    if (!_ai->CanMove() || !InsideHorsemenRoom(Point(bot)))
        return false;
    if (SafeSpline(bot, _rearSpline))
    {
        G3D::Vector3 const end = bot.movespline->FinalDestination();
        if (Clearance(Position(end.x, end.y, end.z)) >= MovementClearance)
            return true;
    }
    if (bot.isMoving())
        bot.StopMoving();

    uint32 const now = getMSTime();
    if (_voidRetryAt && getMSTimeDiff(_voidRetryAt, now) < WayOutRetryMs)
        return true;
    _voidRetryAt = now;

    // Keep the assigned corner once there. In transit, an escape can also move towards it.
    float const limit = std::max(cornerRadius, bot.GetExactDist2d(&corner));
    auto const clearanceAt = [this, &corner, limit](float x, float y, float z)
    {
        if (!InsideHorsemenRoom(x, y, z) || corner.GetExactDist2d(x, y) > limit)
            return std::numeric_limits<float>::lowest();
        return Clearance(Position(x, y, z));
    };
    auto const safePath = [this, &bot](Movement::PointsArray const& path)
    {
        return !FirstBlockedZone(bot, path) &&
               std::ranges::all_of(path, [](G3D::Vector3 const& point) { return InsideHorsemenRoom(point); });
    };
    Position spot;
    Movement::PointsArray path;
    float const want = MovementClearance + 0.5f;
    if (!FindWayOut(_ai, bot, clearanceAt, want, 1.0f, spot, EscapeRadius, 3.0f, safePath, path) ||
        Clearance(spot) < MovementClearance)
    {
        // A pool against the outer edge of a corner can leave no safe spot
        // inward. Keep the forced movement inside this encounter room.
        auto const roomClearanceAt = [this](float x, float y, float z)
        { return InsideHorsemenRoom(x, y, z) ? Clearance(Position(x, y, z)) : std::numeric_limits<float>::lowest(); };
        bool const foundNearby = FindWayOut(_ai, bot, roomClearanceAt, want, 1.0f, spot, NearVoidEscapeRadius,
                                            NearVoidEscapeRadius, safePath, path, true) &&
                                 Clearance(spot) >= MovementClearance;
        if (!foundNearby &&
            !FindWayOut(_ai, bot, roomClearanceAt, want, 1.0f, spot, EscapeRadius, 3.0f, safePath, path))
            return true;
    }
    Launch(bot, path, _rearSpline);
    return true;
}

bool FourHorsemen40Hazards::EscapeCornerVoidZones(Player& bot, Position const& corner, Position const& center,
                                                 float cornerRadius)
{
    RefreshVoidZones(bot);
    G3D::Vector3 const from = CurrentPoint(bot);
    Position const current(from.x, from.y, from.z);
    float const currentClearance = Clearance(current);
    bool const insidePool = currentClearance < MovementClearance;
    bool const escaping = _cornerVoidSpline && bot.isMoving() && !bot.movespline->Finalized() &&
                          bot.movespline->GetId() == _cornerVoidSpline;
    if (!insidePool && !escaping)
    {
        _cornerVoidRetryAt = 0;
        return false;
    }
    if (!_ai->CanMove() || !InsideHorsemenRoom(from))
        return insidePool;

    float const distance = corner.GetExactDist2d(from.x, from.y);
    // Ranged players may already be stationed beyond the tank's corner radius.
    float const limit = std::hypot(std::max(cornerRadius, distance), CornerLateralAllowance);
    G3D::Vector3 const away(Point(corner) - Point(center));
    float const horizontalLength = std::hypot(away.x, away.y);
    auto const staysLocal = [&](float x, float y, float z)
    {
        return InsideHorsemenRoom(x, y, z) && corner.GetExactDist2d(x, y) <= limit &&
               (x - from.x) * away.x + (y - from.y) * away.y >= -DirectionTolerance * horizontalLength;
    };
    auto const safePath = [&](Movement::PointsArray const& path)
    {
        return !path.empty() && !FirstBlockedZone(bot, path) &&
               std::ranges::all_of(path,
                                   [&](G3D::Vector3 const& point) { return staysLocal(point.x, point.y, point.z); });
    };
    if (escaping)
    {
        Movement::PointsArray const remaining = RemainingSpline(bot, _cornerVoidSpline);
        if (safePath(remaining))
        {
            G3D::Vector3 const& end = remaining.back();
            if (Clearance(Position(end.x, end.y, end.z)) >= MovementClearance)
                return true;
        }
    }
    if (!insidePool)
    {
        bot.StopMoving();
        return false;
    }

    uint32 const now = getMSTime();
    if (!_cornerVoidRetryAt || getMSTimeDiff(_cornerVoidRetryAt, now) >= WayOutRetryMs)
    {
        auto const clearanceAt = [&](float x, float y, float z)
        {
            return staysLocal(x, y, z) ? Clearance(Position(x, y, z)) : std::numeric_limits<float>::lowest();
        };
        Position spot;
        Movement::PointsArray path;
        float const lateral = std::atan2(away.y, away.x) + float(M_PI) / 2.0f;
        if (FindWayOut(_ai, bot, clearanceAt, MovementClearance + 0.5f, 1.0f, spot, EscapeRadius, 3.0f,
                       safePath, path, true, CornerEscapeRingStep, lateral) &&
            Clearance(spot) >= MovementClearance && Launch(bot, path, _cornerVoidSpline))
        {
            _cornerVoidRetryAt = 0;
            LOG_INFO("playerbots",
                     "[Naxx40] Horsemen instance={} {} void detour "
                     "({:.2f}, {:.2f}, {:.2f}) -> ({:.2f}, {:.2f}, {:.2f})",
                     bot.GetInstanceId(), bot.GetName(), from.x, from.y, from.z,
                     spot.GetPositionX(), spot.GetPositionY(), spot.GetPositionZ());
            return true;
        }
        if (!_cornerVoidRetryAt)
            LOG_INFO("playerbots",
                     "[Naxx40] Horsemen instance={} {} has no local void detour at ({:.2f}, {:.2f}, {:.2f})",
                     bot.GetInstanceId(), bot.GetName(), from.x, from.y, from.z);
        _cornerVoidRetryAt = now;
    }
    if (currentClearance <= 0.0f)
        return EscapeRearExchangeVoidZone(bot, &center);
    if (bot.isMoving())
        bot.StopMoving();
    return true;
}

bool FourHorsemen40Hazards::HasActiveRearExchangeExit(Player const& bot) const
{
    return _rearExchangeEscapeSpline && bot.isMoving() && !bot.movespline->Finalized() &&
           bot.movespline->GetId() == _rearExchangeEscapeSpline;
}

bool FourHorsemen40Hazards::IsEscapingRearExchangeVoidZone(Player& bot)
{
    Movement::PointsArray const remaining = RemainingSpline(bot, _rearExchangeEscapeSpline);
    if (remaining.empty())
        return false;
    RefreshVoidZones(bot);
    return IsDryRearExchangeExit(bot, remaining, VoidPath::ForcedExit);
}

bool FourHorsemen40Hazards::IsDryRearExchangeExit(Player const& bot, std::span<G3D::Vector3 const> path,
                                                  VoidPath mode) const
{
    if (path.empty() || FirstBlockedZone(bot, path, mode))
        return false;
    G3D::Vector3 const& end = path.back();
    return Clearance(Position(end.x, end.y, end.z)) >= MovementClearance &&
           std::ranges::all_of(path, [](G3D::Vector3 const& point) { return InsideHorsemenRoom(point); });
}

bool FourHorsemen40Hazards::EscapeRearExchangeVoidZone(Player& bot, Position const* toward)
{
    if (IsEscapingRearExchangeVoidZone(bot))
        return true;

    if (!toward && HasActiveRearExchangeExit(bot) && _rearExchangeEscapeToward)
        toward = &*_rearExchangeEscapeToward;

    RefreshVoidZones(bot);
    G3D::Vector3 const current = CurrentPoint(bot);
    if (Clearance(Position(current.x, current.y, current.z)) >= MovementClearance)
    {
        if (HasActiveRearExchangeExit(bot))
        {
            bot.StopMoving();
            _ai->GetAiObjectContext()->GetValue<LastMovement&>("last movement")->Get().clear();
        }
        _rearExchangeEscapeRetryAt = 0;
        return false;
    }
    if (!_ai->CanMove() || !InsideHorsemenRoom(current))
        return true;

    uint32 const now = getMSTime();
    if (!_rearExchangeEscapeRetryAt || getMSTimeDiff(_rearExchangeEscapeRetryAt, now) >= WayOutRetryMs)
    {
        bool const firstAttempt = !_rearExchangeEscapeRetryAt;
        _rearExchangeEscapeRetryAt = now;
        Position spot;
        Movement::PointsArray path;
        auto const findExit = [&](VoidPath mode, Position const* preferred)
        {
            G3D::Vector3 const destination = preferred ? Point(*preferred) : current;
            float const direction = preferred ? std::atan2(destination.y - current.y, destination.x - current.x) : 0.0f;
            float const distance = preferred ? preferred->GetExactDist2d(current.x, current.y) : 0.0f;
            auto const clearanceAt = [&](float x, float y, float z)
            {
                if (!InsideHorsemenRoom(x, y, z) || (preferred && preferred->GetExactDist2d(x, y) >= distance))
                    return std::numeric_limits<float>::lowest();
                return Clearance(Position(x, y, z));
            };
            auto const safeExit = [&](Movement::PointsArray const& route)
            {
                return IsDryRearExchangeExit(bot, route, mode) &&
                       (!preferred || StartsToward(route, current, destination));
            };
            return FindWayOut(_ai, bot, clearanceAt, MovementClearance, 1.0f, spot, EscapeRadius, CornerEscapeRingStep,
                              safeExit, path, true, CornerEscapeRingStep, direction, true);
        };
        bool const centerward =
            toward && (findExit(VoidPath::DryExit, toward) || findExit(VoidPath::ForcedExit, toward));
        if ((centerward || findExit(VoidPath::DryExit, nullptr) || findExit(VoidPath::ForcedExit, nullptr)) &&
            Launch(bot, path, _rearExchangeEscapeSpline))
        {
            _rearExchangeEscapeRetryAt = 0;
            _rearExchangeEscapeToward = toward ? std::optional<Position>(*toward) : std::nullopt;
            LOG_INFO("playerbots", "[Naxx40] Horsemen instance={} {} takes dry {} exit ({:.2f}, {:.2f}, {:.2f})",
                     bot.GetInstanceId(), bot.GetName(), centerward ? "centerward void" : "rear exchange",
                     spot.GetPositionX(), spot.GetPositionY(), spot.GetPositionZ());
            return true;
        }
        if (firstAttempt)
            LOG_INFO("playerbots", "[Naxx40] Horsemen instance={} {} has no reachable dry rear exchange exit",
                     bot.GetInstanceId(), bot.GetName());
    }
    if (bot.isMoving())
        bot.StopMoving();
    return true;
}

bool FourHorsemen40Hazards::ContinueCenterRetreat(Player& bot, bool clearingMarks, bool& crossesVoid)
{
    Movement::PointsArray const remaining = RemainingSpline(bot, _centerRetreatSpline);
    if (remaining.empty())
        return false;

    RefreshVoidZones(bot);
    G3D::Vector3 const& end = remaining.back();
    if (clearingMarks && _ai->CanMove() && Clearance(Position(end.x, end.y, end.z)) >= MovementClearance)
    {
        std::string_view const reason = RearExchangeCrossingBlock(bot, remaining, {true}, crossesVoid);
        if (reason.empty())
            return true;
        LOG_INFO("playerbots", "[Naxx40] Horsemen instance={} {} interrupts mark retreat: {}",
                 bot.GetInstanceId(), bot.GetName(), reason);
    }

    bot.StopMoving();
    _ai->GetAiObjectContext()->GetValue<LastMovement&>("last movement")->Get().clear();
    _centerRetreatSpline = 0;
    return false;
}

bool FourHorsemen40Hazards::FindCenterRetreatStep(Player& bot, Position const& destination,
                                                Movement::PointsArray& path)
{
    G3D::Vector3 const from = CurrentPoint(bot);
    G3D::Vector3 const to = Point(destination);
    float const distance = destination.GetExactDist2d(from.x, from.y);
    auto const dryProgress = [&](float x, float y, float z)
    {
        if (!InsideHorsemenRoom(x, y, z) || distance - destination.GetExactDist2d(x, y) < CornerEscapeRingStep)
            return std::numeric_limits<float>::lowest();
        return Clearance(Position(x, y, z));
    };
    auto const allowed = [&](Movement::PointsArray const& route)
    {
        if (route.empty() || !StartsToward(route, from, to))
            return false;
        G3D::Vector3 const& end = route.back();
        return dryProgress(end.x, end.y, end.z) >= MovementClearance &&
               std::ranges::all_of(route, [](G3D::Vector3 const& point) { return InsideHorsemenRoom(point); }) &&
               HasPathLineOfSight(bot, route);
    };
    auto const dryRoute = [&](Movement::PointsArray const& route)
    { return allowed(route) && !FirstBlockedZone(bot, route, VoidPath::DryExit); };
    auto const crossingRoute = [&](Movement::PointsArray const& route)
    {
        bool crossesVoid = false;
        return allowed(route) && RearExchangeCrossingBlock(bot, route, {true}, crossesVoid).empty();
    };

    Position spot;
    float const bearing = std::atan2(to.y - from.y, to.x - from.x);
    bool const startsInDamage = Clearance(Position(from.x, from.y, from.z)) <= 0.0f;
    return (!startsInDamage &&
            FindWayOut(_ai, bot, dryProgress, MovementClearance, 0.0f, spot, EscapeRadius, CornerEscapeRingStep,
                       dryRoute, path, true, CornerEscapeRingStep, bearing, true)) ||
           FindWayOut(_ai, bot, dryProgress, MovementClearance, 0.0f, spot, EscapeRadius, CornerEscapeRingStep,
                      crossingRoute, path, true, CornerEscapeRingStep, bearing, true);
}

bool FourHorsemen40Hazards::WalkToFixedPoint(Player& bot, Position const& destination, float arrival, bool centerward,
                                             std::span<Position const> alternatives)
{
    G3D::Vector3 const from = CurrentPoint(bot);
    bool const entry = InsideHorsemenEntrance(from);
    if (!_ai->CanMove() || (!InsideHorsemenRoom(from) && !entry))
        return false;

    RefreshVoidZones(bot);
    G3D::Vector3 const to = Point(destination);
    uint32 const now = getMSTime();
    bool const sameBlockedGoal =
        _blockedFixedGoal.GetExactDist(destination) <= arrival && centerward == _blockedFixedCenterward;
    bool const canRetry = !_fixedPointRetryAt || !sameBlockedGoal ||
                          getMSTimeDiff(_fixedPointRetryAt, now) >= WayOutRetryMs;
    bool const validGoal = InsideHorsemenRoom(to) && (to - from).length() > arrival;
    bool const dryGoal = validGoal && Clearance(destination) >= MovementClearance;
    bool const startsInDamage = centerward && Clearance(Position(from.x, from.y, from.z)) <= 0.0f;
    Movement::PointsArray const remaining =
        dryGoal || !alternatives.empty() ? RemainingSpline(bot, _fixedPointSpline) : Movement::PointsArray{};
    auto const canContinueTo = [&](G3D::Vector3 const& goal)
    {
        return !startsInDamage && !remaining.empty() && (remaining.back() - goal).length() <= arrival &&
               !FirstBlockedZone(bot, remaining) &&
               (!centerward || _fixedPointCenterward || StartsToward(remaining, from, goal));
    };
    Movement::PointsArray crossingPath;
    if (dryGoal)
    {
        if (canContinueTo(to))
        {
            _fixedPointCenterward = centerward;
            return true;
        }

        if (canRetry)
        {
            Movement::PointsArray path;
            WalkCheck const leash(_ai, bot);
            bool safe = false;
            if (GroundLeg(bot, from, to, path, entry))
            {
                Position const* const blocked = FirstBlockedZone(bot, path);
                if (centerward && !entry && (blocked || startsInDamage) && StartsToward(path, from, to) &&
                    (path.back() - to).length() <= arrival &&
                    Clearance(Position(path.back().x, path.back().y, path.back().z)) >= MovementClearance)
                    crossingPath = path;
                if (!startsInDamage)
                {
                    safe = blocked ? FindVoidDetour(bot, from, to, *blocked, {}, path, centerward)
                                   : (!centerward || StartsToward(path, from, to)) && !leash.Pulls(path);
                    safe = safe && (path.back() - to).length() <= arrival;
                }
            }
            if (!safe && centerward && !startsInDamage)
                safe = FindCenterwardDetour(bot, from, to, arrival, path);
            if (safe && Launch(bot, path, _fixedPointSpline))
            {
                _fixedPointRetryAt = 0;
                _fixedPointCenterward = centerward;
                auto const first = std::ranges::find_if(path, [&](G3D::Vector3 const& point)
                { return std::hypot(point.x - from.x, point.y - from.y) >= DirectionTolerance; });
                if (first != path.end())
                    LOG_INFO("playerbots",
                             "[Naxx40] Horsemen instance={} {} center path marks={} "
                             "({:.2f}, {:.2f}, {:.2f}) first=({:.2f}, {:.2f}, {:.2f})",
                             bot.GetInstanceId(), bot.GetName(), centerward, from.x, from.y, from.z,
                             first->x, first->y, first->z);
                return true;
            }
        }
    }
    if (canRetry && validGoal && centerward && !entry)
    {
        bool crossesVoid = false;
        bool const reachesCenter = !crossingPath.empty() && !WalkCheck(_ai, bot).Pulls(crossingPath) &&
                                   RearExchangeCrossingBlock(bot, crossingPath, {true}, crossesVoid).empty();
        if ((reachesCenter || FindCenterRetreatStep(bot, destination, crossingPath)) &&
            Launch(bot, crossingPath, _centerRetreatSpline))
        {
            _fixedPointRetryAt = 0;
            G3D::Vector3 const& end = crossingPath.back();
            LOG_INFO("playerbots",
                     "[Naxx40] Horsemen instance={} {} resumes mark retreat to ({:.2f}, {:.2f}, {:.2f})",
                     bot.GetInstanceId(), bot.GetName(), end.x, end.y, end.z);
            return true;
        }
    }
    if (canRetry && (dryGoal || (validGoal && centerward && !entry)))
    {
        _blockedFixedGoal = destination;
        _blockedFixedCenterward = centerward;
        _fixedPointRetryAt = now;
    }
    // Let the caller try the next waypoint without interrupting its active route.
    if (!remaining.empty() && (remaining.back() - to).length() > arrival)
        for (Position const& alternative : alternatives)
        {
            G3D::Vector3 const goal = Point(alternative);
            if (InsideHorsemenRoom(goal) && (goal - from).length() > arrival &&
                Clearance(alternative) >= MovementClearance && canContinueTo(goal))
                return false;
        }
    if (bot.isMoving())
        bot.StopMoving();
    return false;
}

bool FourHorsemen40Hazards::PrepareRearExchangePath(Player& bot, Position const& destination, float arrival,
                                                    Movement::PointsArray& path, FourHorsemenRearCrossing crossing,
                                                    bool* crossesVoid, std::string_view* blockReason,
                                                    std::span<FourHorsemenMarkSource const> avoid,
                                                    std::span<Position const> lane)
{
    if (crossesVoid)
        *crossesVoid = false;
    if (blockReason)
        *blockReason = {};
    auto const blocked = [blockReason](std::string_view reason)
    {
        if (blockReason)
            *blockReason = reason;
        return false;
    };

    RefreshVoidZones(bot);
    G3D::Vector3 const from = CurrentPoint(bot);
    G3D::Vector3 const to = Point(destination);
    if (!_ai->CanMove())
        return blocked("cannot move");
    if (!InsideHorsemenRoom(from) || !InsideHorsemenRoom(to))
        return blocked("outside the Horsemen room");
    if (InRestrictedPosition(bot, to, avoid))
        return blocked("meeting point is inside a restricted area");
    if (Clearance(destination) < MovementClearance)
        return blocked("meeting point is covered by a void zone");
    float const distanceToArrive = crossing.secondsToArrive * bot.GetSpeed(MOVE_RUN);
    auto const validate = [&](Movement::PointsArray const& route, bool nearVoid,
                              bool& crossesDamage) -> std::string_view
    {
        G3D::Vector3 const& end = route.back();
        if ((end - to).length() > arrival)
            return "ground route stops short of meeting point";
        if (Clearance(Position(end.x, end.y, end.z)) < MovementClearance)
            return "ground route ends in a void zone";
        if (!avoid.empty() && (InRestrictedPosition(bot, end, avoid) || CrossesRestrictedMark(bot, route, avoid)))
            return "ground route enters a restricted area";
        bool const late = crossing.secondsToArrive > 0.0f && PathLength(route) > distanceToArrive;
        if (late && !crossing.allowDryApproachAcrossMark)
            return "route is too long to arrive before the next mark";
        if (nearVoid)
            if (std::string_view const reason = RearExchangeCrossingBlock(bot, route, crossing, crossesDamage);
                !reason.empty())
                return reason;
        if (late && crossesDamage)
            return "void crossing is too long to arrive before the next mark";
        return {};
    };
    bool const startsInVoid = Clearance(Position(from.x, from.y, from.z)) < MovementClearance;
    if (!lane.empty())
    {
        for (Position const& waypoint : lane)
        {
            bool crossesDamage = false;
            if (FindLanePath(bot, to, std::span(&waypoint, 1), arrival, avoid, path, true) &&
                validate(path, startsInVoid, crossesDamage).empty())
            {
                if (crossesVoid)
                    *crossesVoid = crossesDamage;
                return true;
            }
        }
        WalkCheck const leash(_ai, bot);
        for (Position const& waypoint : lane)
        {
            Movement::PointsArray onward;
            if (Clearance(waypoint) < MovementClearance || !GroundLeg(bot, from, Point(waypoint), path) ||
                !GroundLeg(bot, path.back(), to, onward))
                continue;
            path.insert(path.end(), std::next(onward.begin()), onward.end());
            bool crossesDamage = false;
            if (validate(path, true, crossesDamage).empty() && !leash.Pulls(path))
            {
                if (crossesVoid)
                    *crossesVoid = crossesDamage;
                return true;
            }
        }
        return blocked("no safe route through the assigned flank before the next mark");
    }

    if (!GroundLeg(bot, from, to, path))
        return blocked("no checked ground route to meeting point");
    Position const* const zone = FirstBlockedZone(bot, path);
    bool crossesDamage = false;
    if (zone)
    {
        Movement::PointsArray detour;
        float const maxDetourLength = crossing.allowDryApproachAcrossMark ? 0.0f : distanceToArrive;
        if (FindVoidDetour(bot, from, to, *zone, avoid, detour, false, maxDetourLength) &&
            validate(detour, startsInVoid, crossesDamage).empty())
        {
            path = std::move(detour);
            if (crossesVoid)
                *crossesVoid = crossesDamage;
            return true;
        }
    }

    crossesDamage = false;
    if (std::string_view const reason = validate(path, startsInVoid || zone, crossesDamage); !reason.empty())
        return blocked(reason);
    if (WalkCheck(_ai, bot).Pulls(path))
        return blocked("ground route may pull idle enemies");
    if (crossesVoid)
        *crossesVoid = crossesDamage;
    return true;
}

bool FourHorsemen40Hazards::StartRearExchangePath(Player& bot, Movement::PointsArray const& path, uint32& splineId)
{
    return _ai->CanMove() && Launch(bot, path, splineId);
}

bool FourHorsemen40Hazards::IsFollowingRearExchangePath(Player& bot, Position const& destination, float arrival,
                                                        uint32 splineId, FourHorsemenRearCrossing crossing,
                                                        bool* crossesVoid, std::string_view* blockReason,
                                                        std::span<FourHorsemenMarkSource const> avoid)
{
    if (crossesVoid)
        *crossesVoid = false;
    if (blockReason)
        *blockReason = {};
    auto const blocked = [blockReason](std::string_view reason)
    {
        if (blockReason)
            *blockReason = reason;
        return false;
    };

    RefreshVoidZones(bot);
    if (!_ai->CanMove())
        return blocked("cannot move");
    Movement::PointsArray const remaining = RemainingSpline(bot, splineId);
    if (remaining.empty())
        return blocked("no active exchange movement");
    if (!std::ranges::all_of(remaining, [](G3D::Vector3 const& point) { return InsideHorsemenRoom(point); }))
        return blocked("remaining ground route leaves the Horsemen room");

    G3D::Vector3 const& end = remaining.back();
    if ((end - Point(destination)).length() > arrival)
        return blocked("active movement does not reach the selected point");
    if (Clearance(destination) < MovementClearance || Clearance(Position(end.x, end.y, end.z)) < MovementClearance)
        return blocked("meeting point is covered by a void zone");
    if (!avoid.empty() && (InRestrictedPosition(bot, end, avoid) || CrossesRestrictedMark(bot, remaining, avoid)))
        return blocked("active movement enters a restricted area");

    G3D::Vector3 const start = CurrentPoint(bot);
    bool const late = crossing.secondsToArrive > 0.0f && (remaining.front() - start).length() + PathLength(remaining) >
                                                           crossing.secondsToArrive * bot.GetSpeed(MOVE_RUN);
    if (late && !crossing.allowDryApproachAcrossMark)
        return blocked("remaining route is too long to arrive before the next mark");
    bool crossesDamage = false;
    bool const nearVoid =
        Clearance(Position(start.x, start.y, start.z)) < MovementClearance || FirstBlockedZone(bot, remaining);
    if (nearVoid)
    {
        std::string_view const reason = RearExchangeCrossingBlock(bot, remaining, crossing, crossesDamage);
        if (!reason.empty())
            return blocked(reason);
    }
    if (late && crossesDamage)
        return blocked("remaining void crossing is too long to arrive before the next mark");
    if (crossesVoid)
        *crossesVoid = crossesDamage;
    return true;
}

bool FourHorsemen40Hazards::HasActiveRearRenditionPath(Player const& bot) const
{
    return _rearRenditionSpline && bot.isMoving() && !bot.movespline->Finalized() &&
           bot.movespline->GetId() == _rearRenditionSpline;
}

void FourHorsemen40Hazards::StopRearRenditionPath(Player& bot)
{
    if (HasActiveRearRenditionPath(bot))
    {
        bot.StopMoving();
        _ai->GetAiObjectContext()->GetValue<LastMovement&>("last movement")->Get().clear();
    }
    _rearRenditionSpline = 0;
}

bool FourHorsemen40Hazards::AvoidVoidOnRearRenditionPath(Player& bot, FourHorsemenRearCrossing crossing)
{
    bool const inVoid = !IsSafeFromVoidDamage(bot, bot.GetPosition());
    if (IsFollowingRearExchangePath(bot, _rearRenditionDestination, _rearRenditionArrival, _rearRenditionSpline,
                                    crossing))
        return inVoid;
    if (inVoid)
        return EscapeRearExchangeVoidZone(bot);
    StopRearRenditionPath(bot);
    return false;
}

bool FourHorsemen40Hazards::WalkRearExchangeToPoint(Player& bot, Position const& destination, float arrival,
                                                    FourHorsemenRearCrossing crossing, std::span<Position const> lane)
{
    if (IsEscapingRearExchangeVoidZone(bot) ||
        ((lane.empty() || _rearRenditionViaLane) &&
         IsFollowingRearExchangePath(bot, destination, arrival, _rearRenditionSpline, crossing)))
        return true;
    if (!IsSafeFromVoidDamage(bot, bot.GetPosition()))
        return EscapeRearExchangeVoidZone(bot);

    StopRearRenditionPath(bot);
    if (bot.GetExactDist(destination) <= arrival)
        return false;
    uint32 const now = getMSTime();
    if (_rearRenditionRetryAt && _rearRenditionDestination.GetExactDist(destination) <= arrival &&
        getMSTimeDiff(_rearRenditionRetryAt, now) < WayOutRetryMs)
        return false;

    _rearRenditionDestination = destination;
    _rearRenditionArrival = arrival;
    Movement::PointsArray path;
    bool crossesVoid = false;
    if (!PrepareRearExchangePath(bot, destination, arrival, path, crossing, &crossesVoid, nullptr, {}, lane) ||
        !StartRearExchangePath(bot, path, _rearRenditionSpline))
    {
        _rearRenditionRetryAt = now;
        return false;
    }
    _rearRenditionViaLane = !lane.empty();
    _rearRenditionRetryAt = 0;
    if (crossesVoid)
        LOG_INFO("playerbots", "[Naxx40] Horsemen instance={} {} crosses a pool during rear tank handoff",
                 bot.GetInstanceId(), bot.GetName());
    return true;
}

bool FourHorsemen40Hazards::FindLanePath(Player& bot, G3D::Vector3 const& destination,
                                       std::span<Position const> lane, float arrival,
                                       std::span<FourHorsemenMarkSource const> avoid, Movement::PointsArray& path,
                                       bool detour) const
{
    G3D::Vector3 const from = CurrentPoint(bot);
    WalkCheck const leash(_ai, bot);
    for (Position const& waypoint : lane)
    {
        if (Clearance(waypoint) < MovementClearance)
            continue;

        G3D::Vector3 const via = Point(waypoint);
        if (!GroundLeg(bot, from, via, path))
            continue;
        if (Position const* pool = FirstBlockedZone(bot, path);
            pool && (!detour || !FindVoidDetour(bot, from, via, *pool, avoid, path)))
            continue;

        Movement::PointsArray onward;
        if (!GroundLeg(bot, path.back(), destination, onward) || (onward.back() - destination).length() > arrival)
            continue;

        std::size_t const waypointIndex = path.size() - 1;
        path.insert(path.end(), std::next(onward.begin()), onward.end());
        if (Position const* pool = FirstBlockedZone(bot, path))
        {
            path.resize(waypointIndex + 1);
            if (!detour || !FindVoidDetour(bot, path.back(), destination, *pool, avoid, onward, false, 0.0f, path))
                continue;
            path = std::move(onward);
        }
        if ((path.back() - destination).length() <= arrival &&
            (avoid.empty() || !CrossesRestrictedMark(bot, path, avoid)) && !leash.Pulls(path))
            return true;
    }
    return false;
}

bool FourHorsemen40Hazards::WalkRear(Player& bot, Position const& destination, float arrival, bool detour,
                                     float maxPathLength, std::span<FourHorsemenMarkSource const> avoid,
                                     Position const* via, bool requireArrival, std::span<Position const> lane)
{
    if (!_ai->CanMove() || !InsideHorsemenRoom(Point(bot)) || !InsideHorsemenRoom(Point(destination)))
        return false;

    RefreshVoidZones(bot);
    bool const movingSafely = SafeSpline(bot, _rearSpline, avoid, false, maxPathLength);
    if (movingSafely)
    {
        G3D::Vector3 const end = bot.movespline->FinalDestination();
        if (destination.GetExactDist(end.x, end.y, end.z) <= arrival &&
            (lane.empty() || _rearSpline == _rearLaneSpline))
            return true;
    }
    else if (bot.isMoving() && maxPathLength <= 0.0f && (lane.empty() || _rearSpline == bot.movespline->GetId()))
        bot.StopMoving();

    if (bot.GetExactDist(destination) <= arrival || Clearance(destination) < MovementClearance)
        return false;
    uint32 const now = getMSTime();
    if (_rearRetryAt && getMSTimeDiff(_rearRetryAt, now) < WayOutRetryMs &&
        _blockedGoal.GetExactDist(destination) <= arrival)
        return false;

    G3D::Vector3 const from = CurrentPoint(bot);
    G3D::Vector3 const to = Point(destination);
    auto const reachesDestination = [&](Movement::PointsArray const& candidate)
    { return !requireArrival || (candidate.back() - to).length() <= arrival; };
    Movement::PointsArray path;
    if (!lane.empty())
    {
        if (!FindLanePath(bot, to, lane, arrival, avoid, path, detour) || !Launch(bot, path, _rearSpline))
            return false;
        _rearLaneSpline = _rearSpline;
        return true;
    }
    if (via && maxPathLength <= 0.0f && bot.GetExactDist(*via) > 10.0f)
    {
        Movement::PointsArray onward;
        if (GroundLeg(bot, from, Point(*via), path) && GroundLeg(bot, path.back(), to, onward))
        {
            path.insert(path.end(), std::next(onward.begin()), onward.end());
            WalkCheck const leash(_ai, bot);
            if (reachesDestination(path) && !FirstBlockedZone(bot, path) &&
                (avoid.empty() || !CrossesRestrictedMark(bot, path, avoid)) &&
                !leash.Pulls(path) && Launch(bot, path, _rearSpline))
                return true;
        }
    }

    if (!GroundLeg(bot, from, to, path) || !reachesDestination(path))
        return false;
    if (maxPathLength > 0.0f)
    {
        float length = 0.0f;
        for (std::size_t i = 1; i < path.size(); ++i)
            length += (path[i] - path[i - 1]).length();
        G3D::Vector3 const& end = path.back();
        if (length > maxPathLength || Clearance(Position(end.x, end.y, end.z)) < MovementClearance)
            return false;
    }
    WalkCheck const leash(_ai, bot);
    Position const* const zone = FirstBlockedZone(bot, path);
    if (!zone)
        return (avoid.empty() || !CrossesRestrictedMark(bot, path, avoid)) && !leash.Pulls(path) &&
               Launch(bot, path, _rearSpline);
    if (!detour || maxPathLength > 0.0f)
        return false;

    if (FindVoidDetour(bot, from, to, *zone, avoid, path) && reachesDestination(path))
        return Launch(bot, path, _rearSpline);

    _blockedGoal = destination;
    _rearRetryAt = now;
    return false;
}

bool FourHorsemen40Hazards::FindCenterwardDetour(Player& bot, G3D::Vector3 const& from, G3D::Vector3 const& to,
                                                 float arrival, Movement::PointsArray& path) const
{
    float const bearing = std::atan2(to.y - from.y, to.x - from.x);
    WalkCheck const leash(_ai, bot);
    for (float distance : CenterwardProbeDistances)
        for (int degrees : CenterwardProbeAngles)
        {
            float const angle = bearing + static_cast<float>(degrees) * DegreesToRadians;
            float const x = from.x + std::cos(angle) * distance;
            float const y = from.y + std::sin(angle) * distance;
            float const z = bot.GetMap()->GetHeight(bot.GetPhaseMask(), x, y, from.z + WayOutFloorUp);
            if (z <= INVALID_HEIGHT || z < from.z - WayOutFloorDown || !InsideHorsemenRoom(x, y, z))
                continue;

            Movement::PointsArray first;
            Movement::PointsArray onward;
            if (!GroundLeg(bot, from, {x, y, z}, first, InsideHorsemenEntrance(from)) ||
                !StartsToward(first, from, to) ||
                FirstBlockedZone(bot, first) || !GroundLeg(bot, first.back(), to, onward) ||
                (onward.back() - to).length() > arrival)
                continue;

            first.insert(first.end(), std::next(onward.begin()), onward.end());
            if (!FirstBlockedZone(bot, first) && !leash.Pulls(first))
            {
                path = std::move(first);
                return true;
            }
        }
    return false;
}

bool FourHorsemen40Hazards::FindVoidDetour(Player& bot, G3D::Vector3 const& from, G3D::Vector3 const& to,
                                           Position const& zone, std::span<FourHorsemenMarkSource const> avoid,
                                           Movement::PointsArray& path, bool centerward, float maxPathLength,
                                           std::span<G3D::Vector3 const> prefix) const
{
    // The navmesh does not contain spell hazards. Try a dry dogleg around
    // the first obstruction, with both legs checked against every active pool.
    std::vector<G3D::Vector3> detours;
    for (float radius : {6.0f, 9.0f})
        for (int step = 0; step < 8; ++step)
        {
            float const angle = static_cast<float>(step) * static_cast<float>(M_PI) / 4.0f;
            float const x = zone.GetPositionX() + std::cos(angle) * radius;
            float const y = zone.GetPositionY() + std::sin(angle) * radius;
            float const z = bot.GetMapHeight(x, y, zone.GetPositionZ() + 2.0f, true, 5.0f);
            if (InsideHorsemenRoom(x, y, z) && Clearance(Position(x, y, z)) >= MovementClearance)
                detours.emplace_back(x, y, z);
        }
    std::ranges::sort(detours, {}, [&from, &to](G3D::Vector3 const& point)
                      { return (point - from).length() + (point - to).length(); });
    WalkCheck const leash(_ai, bot);
    for (G3D::Vector3 const& waypoint : detours)
    {
        Movement::PointsArray first;
        Movement::PointsArray second;
        if (!GroundLeg(bot, from, waypoint, first, InsideHorsemenEntrance(from)) ||
            (prefix.empty() && FirstBlockedZone(bot, first)) ||
            !GroundLeg(bot, first.back(), to, second))
            continue;
        first.insert(first.end(), std::next(second.begin()), second.end());
        if (!prefix.empty())
            first.insert(first.begin(), prefix.begin(), std::prev(prefix.end()));
        if ((maxPathLength <= 0.0f || PathLength(first) <= maxPathLength) &&
            (!centerward || StartsToward(first, from, to)) && !FirstBlockedZone(bot, first) &&
            (avoid.empty() || !CrossesRestrictedMark(bot, first, avoid)) && !leash.Pulls(first))
        {
            path = std::move(first);
            return true;
        }
    }

    return false;
}

bool FourHorsemen40Hazards::ApproachRear(Player& bot, Unit const& target, float range, float arrival,
                                         float minimumRange, float maxPathLength,
                                         std::span<FourHorsemenMarkSource const> avoid, Position* exchangeDestination,
                                         uint32* exchangeSpline, bool allowExchangeVoidCrossing)
{
    RefreshVoidZones(bot);
    auto const canReachTargetFrom = [&](float x, float y, float z)
    {
        return InsideHorsemenRoom(x, y, z) && target.GetExactDist(x, y, z) < range &&
               target.GetExactDist2d(x, y) >= minimumRange &&
               std::ranges::none_of(
                   avoid, [x, y](FourHorsemenMarkSource const& source)
                   { return source.location.GetExactDist2d(x, y) < source.radius; }) &&
               bot.GetMap()->isInLineOfSight(x, y, z + 1.5f, target.GetPositionX(), target.GetPositionY(),
                                             target.GetPositionZ() + 1.5f, bot.GetPhaseMask(), LINEOFSIGHT_ALL_CHECKS,
                                             VMAP::ModelIgnoreFlags::Nothing);
    };
    bool const exchange = exchangeDestination && exchangeSpline;
    if (!exchange && SafeSpline(bot, _rearSpline, avoid, false, maxPathLength))
    {
        G3D::Vector3 const end = bot.movespline->FinalDestination();
        if (canReachTargetFrom(end.x, end.y, end.z))
            return true;
    }

    float const angleFromTarget = target.GetAbsoluteAngle(&bot);
    std::vector<Position> detourGoals;
    Movement::PointsArray crossingPath;
    std::optional<Position> crossingDestination;
    auto const startExchangePath = [&](Position const& position, Movement::PointsArray const& path)
    {
        if (!StartRearExchangePath(bot, path, *exchangeSpline))
            return false;
        *exchangeDestination = position;
        return true;
    };
    for (float distance : {range - 3.0f, range - 8.0f})
        for (int step : {0, 1, -1, 2, -2, 3, -3, 4})
        {
            float const angle = angleFromTarget + static_cast<float>(step) * static_cast<float>(M_PI) / 4.0f;
            float const x = target.GetPositionX() + std::cos(angle) * distance;
            float const y = target.GetPositionY() + std::sin(angle) * distance;
            float const z = bot.GetMapHeight(x, y, bot.GetPositionZ() + 2.0f, true, 5.0f);
            Position const position(x, y, z);
            if (!canReachTargetFrom(x, y, z) || Clearance(position) < MovementClearance)
                continue;
            if (exchange)
            {
                Movement::PointsArray path;
                bool crossesVoid = false;
                if (bot.GetExactDist(position) <= arrival ||
                    !PrepareRearExchangePath(bot, position, arrival, path,
                                             {allowExchangeVoidCrossing && !crossingDestination}, &crossesVoid, nullptr,
                                             avoid))
                    continue;
                if (maxPathLength > 0.0f)
                    if (PathLength(path) > maxPathLength)
                        continue;
                if (crossesVoid)
                {
                    crossingPath = std::move(path);
                    crossingDestination = position;
                }
                else if (startExchangePath(position, path))
                    return true;
                continue;
            }
            if (WalkRear(bot, position, arrival, false, maxPathLength, avoid))
                return true;
            if (maxPathLength <= 0.0f)
                detourGoals.push_back(position);
        }
    if (exchange)
        return crossingDestination && startExchangePath(*crossingDestination, crossingPath);
    return std::ranges::any_of(detourGoals,
                               [&](Position const& goal) { return WalkRear(bot, goal, arrival, true, 0.0f, avoid); });
}

bool FourHorsemen40Hazards::KeepRearMarkSeparation(Player& bot, Unit const& source, float minimumDistance)
{
    if (!_rearSeparationSpline || bot.movespline->GetId() != _rearSeparationSpline ||
        _rearSeparationSource != source.GetGUID())
        return false;
    RefreshVoidZones(bot);
    if (!SafeSpline(bot, _rearSeparationSpline, {}, false, RearSeparationMaxPath))
        return false;
    G3D::Vector3 const end = bot.movespline->FinalDestination();
    return source.GetExactDist2d(end.x, end.y) >= minimumDistance;
}

bool FourHorsemen40Hazards::WalkRearOutsideMark(Player& bot, Unit const& source, float minimumDistance)
{
    if (!_ai->CanMove() || !InsideHorsemenRoom(Point(bot)))
        return false;
    RefreshVoidZones(bot);
    auto const separation = [&](float x, float y, float z)
    {
        if (!InsideHorsemenRoom(x, y, z) || Clearance(Position(x, y, z)) < MovementClearance)
            return std::numeric_limits<float>::lowest();
        return source.GetExactDist2d(x, y) - minimumDistance;
    };
    uint32 const now = getMSTime();
    if (_rearSeparationRetryAt && getMSTimeDiff(_rearSeparationRetryAt, now) < WayOutRetryMs)
        return false;
    _rearSeparationRetryAt = now;

    auto const clearPath = [&](Movement::PointsArray const& path)
    {
        if (path.empty() || FirstBlockedZone(bot, path))
            return false;
        G3D::Vector3 from = CurrentPoint(bot);
        float length = 0.0f;
        for (G3D::Vector3 const& to : path)
        {
            if (!InsideHorsemenRoom(to))
                return false;
            length += (to - from).length();
            from = to;
        }
        return length <= RearSeparationMaxPath;
    };
    Position spot;
    Movement::PointsArray path;
    if (!FindWayOut(_ai, bot, separation, 0.0f, 0.0f, spot, RearSeparationSearch, 3.0f, clearPath, path, true) ||
        separation(spot.GetPositionX(), spot.GetPositionY(), spot.GetPositionZ()) < 0.0f ||
        !Launch(bot, path, _rearSeparationSpline))
        return false;

    _rearSeparationSource = source.GetGUID();
    LOG_INFO("playerbots", "[Naxx40] Horsemen rear instance={} {} steps away from {} to ({:.2f}, {:.2f}, {:.2f})",
             bot.GetInstanceId(), bot.GetName(), source.GetName(), spot.GetPositionX(), spot.GetPositionY(),
             spot.GetPositionZ());
    return true;
}

bool FourHorsemen40Hazards::CanReceiveZeliekWrath(Player const& bot, Position const& point,
                                               std::span<FourHorsemenMarkSource const> sources, float margin)
{
    for (FourHorsemenMarkSource const& source : sources)
        if (!source.wrathSource.IsEmpty())
            if (Unit const* origin = FindWrathSource(bot, source.wrathSource);
                origin && point.GetExactDist(origin) < source.radius + margin &&
                HasWrathLineOfSight(*origin, bot, Point(point)))
                return true;
    return false;
}

bool FourHorsemen40Hazards::CanAttackFrom(Player& bot, Unit const& target, Position const& position, float range,
                                          std::span<FourHorsemenMarkSource const> avoid,
                                          std::span<Position const> spread, float minimumRange, float spreadClearance)
{
    float const distance = position.GetExactDist(&target);
    if (!InsideHorsemenRoom(Point(position)) || distance > range || distance < minimumRange ||
        InRestrictedPosition(bot, Point(position), avoid) ||
        std::ranges::any_of(
            spread, [&](Position const& neighbor) { return position.GetExactDist2d(&neighbor) < spreadClearance; }) ||
        !IsSafeFromVoidZones(bot, position))
        return false;

    return bot.GetMap()->isInLineOfSight(position.GetPositionX(), position.GetPositionY(),
                                         position.GetPositionZ() + 1.5f, target.GetPositionX(), target.GetPositionY(),
                                         target.GetPositionZ() + 1.5f, bot.GetPhaseMask(), LINEOFSIGHT_ALL_CHECKS,
                                         VMAP::ModelIgnoreFlags::Nothing);
}

std::optional<Position> FourHorsemen40Hazards::FindAttackPosition(Player& bot, Unit const& target, float range,
                                                                  bool ranged,
                                                                  std::span<FourHorsemenMarkSource const> avoid,
                                                                  std::span<Position const> spread, float minimumRange,
                                                                  float spreadClearance,
                                                                  std::function<bool(Position const&)> const& usable,
                                                                  std::span<Position const> preferred, float arrival,
                                                                  std::span<Position const> lane)
{
    if (!_ai->CanMove() || !InsideHorsemenRoom(Point(bot)) || minimumRange >= range)
        return std::nullopt;

    float const angleFromTarget = target.GetAbsoluteAngle(&bot);
    std::vector<std::pair<Position, Position>> blockedGoals;
    G3D::Vector3 const from = CurrentPoint(bot);
    WalkCheck const leash(_ai, bot);
    auto const reachable = [&](Position const& point, bool detour = false)
    {
        if ((usable && !usable(point)) ||
            !CanAttackFrom(bot, target, point, range, avoid, spread, minimumRange, spreadClearance))
            return false;
        Movement::PointsArray path;
        if (!lane.empty())
            return FindLanePath(bot, Point(point), lane, arrival, avoid, path, detour);
        if (!GroundLeg(bot, from, Point(point), path) ||
            (arrival > 0.0f && (path.back() - Point(point)).length() > arrival))
            return false;
        if (Position const* zone = FirstBlockedZone(bot, path))
        {
            blockedGoals.emplace_back(point, *zone);
            return false;
        }
        return !leash.Pulls(path) && (avoid.empty() || !CrossesRestrictedMark(bot, path, avoid));
    };
    for (Position const& point : preferred)
        if (reachable(point, true))
            return point;

    float const middle = minimumRange + (range - minimumRange) * 0.5f;
    int const halfCircleSteps = preferred.empty() ? 4 : 8;
    for (float distance : {std::max(range - (ranged ? 3.0f : 0.75f), middle), std::max(range * 0.7f, middle)})
        for (int attempt = 0; attempt < 2 * halfCircleSteps; ++attempt)
        {
            int const step = attempt % 2 ? (attempt + 1) / 2 : -attempt / 2;
            float const angle = angleFromTarget +
                                static_cast<float>(step) * static_cast<float>(M_PI) / static_cast<float>(halfCircleSteps);
            float const x = target.GetPositionX() + std::cos(angle) * distance;
            float const y = target.GetPositionY() + std::sin(angle) * distance;
            float const z = bot.GetMapHeight(x, y, target.GetPositionZ() + 2.0f, true, 5.0f);
            Position const point(x, y, z);
            if (reachable(point))
                return point;
        }
    Movement::PointsArray path;
    for (auto const& [goal, pool] : blockedGoals)
        if (FindVoidDetour(bot, from, Point(goal), pool, avoid, path) &&
            (arrival <= 0.0f || (path.back() - Point(goal)).length() <= arrival))
            return goal;
    return std::nullopt;
}

void FourHorsemen40Hazards::StopWalkTo(Player& bot, Position const& destination, float arrival)
{
    if (bot.movespline->Finalized() || bot.movespline->GetId() != _rearSpline)
        return;
    G3D::Vector3 const end = bot.movespline->FinalDestination();
    if (destination.GetExactDist(end.x, end.y, end.z) <= arrival)
        bot.StopMoving();
}

void FourHorsemen40Hazards::StopMarkRetreat(Player& bot)
{
    if (!bot.movespline->Finalized() && bot.movespline->GetId() == _markSpline)
        bot.StopMoving();
}

bool FourHorsemen40Hazards::EscapeMarks(Player& bot, std::span<FourHorsemenMarkSource const> sources,
                                        bool seekCompleteEscape, Position const* toward,
                                        std::span<FourHorsemenMarkSource const> otherMarks)
{
    if (sources.empty() || !_ai->CanMove())
        return false;

    RefreshVoidZones(bot);
    auto const markClearanceAt = [this, sources](float x, float y, float z)
    {
        if (!InsideHorsemenRoom(x, y, z))
            return std::numeric_limits<float>::lowest();
        float gap = Clearance(Position(x, y, z)) - MovementClearance;
        for (FourHorsemenMarkSource const& source : sources)
            gap = std::min(gap, source.location.GetExactDist2d(x, y) - source.radius - MarkMargin);
        return gap;
    };
    auto const clearOfAllMarks = [&](float x, float y, float z)
    {
        float gap = markClearanceAt(x, y, z);
        for (FourHorsemenMarkSource const& source : otherMarks)
            gap = std::min(gap, source.location.GetExactDist2d(x, y) - source.radius - MarkMargin);
        return gap;
    };
    auto const roomPath = [this, &bot](auto const& path, float maxLength)
    {
        if (path.empty() || FirstBlockedZone(bot, path))
            return false;
        float length = 0.0f;
        for (std::size_t i = 0; i < path.size(); ++i)
        {
            auto const& point = path[i];
            if (!InsideHorsemenRoom(point.x, point.y, point.z))
                return false;
            if (i)
                length += (point - path[i - 1]).length();
        }
        return length <= maxLength;
    };

    float const current = markClearanceAt(bot.GetPositionX(), bot.GetPositionY(), bot.GetPositionZ());
    bool const overlaps =
        seekCompleteEscape &&
        std::ranges::count_if(
            sources, [&bot](FourHorsemenMarkSource const& source)
            { return source.isMark && bot.GetExactDist2d(&source.location) < source.radius + MarkMargin; }) > 1;
    bool keepPartial = false;
    if (bot.isMoving() && !bot.movespline->Finalized())
    {
        G3D::Vector3 const end = bot.movespline->FinalDestination();
        float const endClearance = markClearanceAt(end.x, end.y, end.z);
        auto const avoid = seekCompleteEscape ? sources : std::span<FourHorsemenMarkSource const>{};
        if (SafeSpline(bot, _markSpline, avoid, seekCompleteEscape) &&
            (endClearance >= 0.0f || (current < 0.0f && endClearance > current)))
        {
            if (toward && current >= 0.0f)
            {
                if (!otherMarks.empty() &&
                    clearOfAllMarks(bot.GetPositionX(), bot.GetPositionY(), bot.GetPositionZ()) < 0.0f &&
                    clearOfAllMarks(end.x, end.y, end.z) >= 0.0f)
                    return true;
                bot.StopMoving();
                return false;
            }
            if (!overlaps || endClearance >= 0.0f)
                return current < 0.0f;
            keepPartial = true;
        }

        // A normal chase may have planned a detour through the corridor.
        auto const& spline = bot.movespline->_Spline();
        bool outsideRoom = false;
        for (int32 i = bot.movespline->_currentSplineIdx() + 1; i <= spline.last(); ++i)
        {
            G3D::Vector3 const& point = spline.getPoint(i);
            if (!InsideHorsemenRoom(point.x, point.y, point.z))
            {
                outsideRoom = true;
                break;
            }
        }
        bool const replaceRearWalk =
            toward && bot.movespline->GetId() == _rearSpline && Clearance(bot.GetPosition()) >= MovementClearance;
        if (!keepPartial &&
            (outsideRoom || current >= 0.0f || replaceRearWalk || bot.movespline->GetId() == _markSpline))
            bot.StopMoving();
    }
    if (current >= 0.0f || !InsideHorsemenRoom(bot.GetPositionX(), bot.GetPositionY(), bot.GetPositionZ()))
        return false;
    uint32 const now = getMSTime();
    if (_markRetryAt && getMSTimeDiff(_markRetryAt, now) < WayOutRetryMs)
        return keepPartial;
    _markRetryAt = now;

    auto const launchRetreat = [&](Movement::PointsArray const& path, bool complete)
    {
        if (!Launch(bot, path, _markSpline))
            return false;
        G3D::Vector3 const end = bot.movespline->FinalDestination();
        LOG_INFO("playerbots",
                 "[Naxx40] Horsemen mark retreat instance={} {} -> ({:.1f}, {:.1f}, {:.2f}) "
                 "distance={:.1f}yd clearance={:.1f}yd completeEscape={}",
                 bot.GetInstanceId(), bot.GetName(), end.x, end.y, end.z, bot.GetExactDist2d(end.x, end.y),
                 markClearanceAt(end.x, end.y, end.z), complete);
        return true;
    };
    Movement::PointsArray checkedPath;
    auto const completePath = [&bot, &roomPath, sources](auto const& path)
    { return roomPath(path, CompleteMarkEscapeMaxPath) && !CrossesRestrictedMark(bot, path, sources, true); };
    if (toward)
    {
        WalkCheck const leash(_ai, bot);
        auto const pathAllowed = [&](auto const& path) { return completePath(path) && !leash.Pulls(path); };
        auto const pathClearOfOtherMarks = [&](auto const& path)
        { return pathAllowed(path) && !CrossesRestrictedMark(bot, path, otherMarks, true); };
        if (!otherMarks.empty() &&
            FindCenterwardMarkExit(bot, sources, *toward, clearOfAllMarks, pathClearOfOtherMarks, checkedPath))
            return launchRetreat(checkedPath, true);
        if (FindCenterwardMarkExit(bot, sources, *toward, markClearanceAt, pathAllowed, checkedPath))
            return launchRetreat(checkedPath, true);
    }

    std::vector<Position> occupied;
    if (Group* group = toward ? nullptr : bot.GetGroup())
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
            if (Player* member = ref->GetSource();
                member && member != &bot && member->IsInWorld() && member->IsAlive() && member->IsInMap(&bot))
            {
                Position position = member->GetPosition();
                if (member->isMoving() && !member->movespline->Finalized())
                {
                    G3D::Vector3 const end = member->movespline->FinalDestination();
                    position.Relocate(end.x, end.y, end.z);
                }
                if (bot.GetExactDist2d(&position) <= MarkEscapeSearch + MarkRetreatSpacing)
                    occupied.push_back(position);
            }

    auto const spacingAt = [&occupied](float x, float y)
    {
        float spacing = 0.0f;
        for (Position const& position : occupied)
            spacing = std::min(spacing, position.GetExactDist2d(x, y) - MarkRetreatSpacing);
        return spacing;
    };
    auto const clearanceAt = [&bot, &markClearanceAt, &spacingAt, current](float x, float y, float z)
    {
        float const gap = markClearanceAt(x, y, z);
        if ((x != bot.GetPositionX() || y != bot.GetPositionY()) && gap < current + 1.0f)
            return std::numeric_limits<float>::lowest();
        return gap + spacingAt(x, y);
    };

    // Break long retreats into reachable steps. Reject the full walk if it
    // crosses the encounter boundary even when the endpoint is inside.
    Position spot;
    auto const shortPath = [&bot, &roomPath, sources, seekCompleteEscape](auto const& path)
    {
        return roomPath(path, MarkEscapeMaxPath) &&
               (!seekCompleteEscape || !CrossesRestrictedMark(bot, path, sources, true));
    };
    bool found = !keepPartial &&
                 FindWayOut(_ai, bot, clearanceAt, 1.0f, 3.0f, spot, MarkEscapeSearch, 3.0f, shortPath, checkedPath);
    bool completeEscape = false;
    if ((overlaps || toward) &&
        (!found || markClearanceAt(spot.GetPositionX(), spot.GetPositionY(), spot.GetPositionZ()) < 0.0f))
    {
        // A partial retreat between overlapping marks can end in a dead end by the wall.
        auto const completeGapAt = [&markClearanceAt, &spacingAt](float x, float y, float z)
        {
            float const gap = markClearanceAt(x, y, z);
            return gap >= 0.0f ? std::max(0.0f, gap + spacingAt(x, y)) : std::numeric_limits<float>::lowest();
        };
        Movement::PointsArray escapePath;
        completeEscape = FindWayOut(_ai, bot, completeGapAt, 0.0f, 0.0f, spot, CompleteMarkEscapeSearch, 3.0f,
                                    completePath, escapePath, true);
        if (completeEscape)
        {
            checkedPath = std::move(escapePath);
            found = true;
        }
    }
    // Launch the checked vertices directly, including on paths with just two points.
    return found ? launchRetreat(checkedPath, completeEscape) : keepPartial;
}
