/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "ToCHelpers_Icehowl.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <limits>
#include <tuple>
#include <unordered_map>
#include <vector>

#include "Creature.h"
#include "EncounterHelpers.h"
#include "Group.h"
#include "Map.h"
#include "ObjectAccessor.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "RaidInstanceState.h"
#include "SpellAuras.h"
#include "SpellMgr.h"
#include "Timer.h"
#include "ToCHelpers_Jormungars.h"
#include "ToCHelpers_NorthrendBeasts.h"

namespace TrialOfTheCrusaderHelpers
{
namespace
{
constexpr float GATE_ANGLE_MIN = 1.0f;
constexpr float GATE_ANGLE_MAX = 2.0f;

// A cycle runs about 9 s from his jump to the centre to the end of the charge, so a line older than
// this is stale whatever he's doing.
constexpr uint32 CHARGE_LATCH_MAX_MS = 12000;

// Shorter than the 30 s he always spends out of a cycle, so a silence this long means the end of the
// last one went unseen.
constexpr uint32 CHARGE_REFRESH_GAP_MS = 20000;

struct IcehowlChargeState
{
    uint32 memoMs = 0;
    bool memoValid = false;  // 0 is a real getMSTime value

    bool inCycle = false;
    uint32 lastPassiveMs = 0;
    ObjectGuid gazeTarget;
    bool frozen = false;
    bool latched = false;
    bool expired = false;
    uint32 latchMs = 0;
    Position start;
    Position end;
};

RaidInstanceState<IcehowlChargeState> chargeStates;

void ResetCycle(IcehowlChargeState& state)
{
    state.inCycle = false;
    state.gazeTarget = ObjectGuid::Empty;
    state.frozen = false;
    state.latched = false;
    state.expired = false;
    state.latchMs = 0;
}

// Same maths as EVENT_JUMP_BACK, angle measured from the centre toward the charge's end.
void LatchLane(IcehowlChargeState& state, float angle, uint32 now)
{
    float const cx = ARENA_CENTER.GetPositionX();
    float const cy = ARENA_CENTER.GetPositionY();
    float const cz = ARENA_CENTER.GetPositionZ();
    float const dx = std::cos(angle);
    float const dy = std::sin(angle);
    float const reach =
        angle > GATE_ANGLE_MIN && angle < GATE_ANGLE_MAX ? ICEHOWL_CHARGE_REACH_GATE : ICEHOWL_CHARGE_REACH;

    state.start = Position(cx - dx * ICEHOWL_CHARGE_BACK, cy - dy * ICEHOWL_CHARGE_BACK, cz);
    state.end = Position(cx + dx * reach, cy + dy * reach, cz);

    if (!state.latched)
    {
        state.latched = true;
        state.latchMs = now;
    }
}

// He is REACT_PASSIVE in combat only from the jump to the centre until the charge ends, so the cycle reads
// off that. The gaze stuns the raid, so the line matters only from the jump back, where he stands on it.
IcehowlChargeState* RefreshCharge(PlayerbotAI* botAI)
{
    Player* bot = botAI ? botAI->GetBot() : nullptr;
    Map* map = bot ? bot->FindMap() : nullptr;
    if (!map || map->GetId() != TRIAL_OF_THE_CRUSADER_MAP_ID || !bot->GetInstanceId())
        return nullptr;

    IcehowlChargeState& state = chargeStates.For(bot->GetInstanceId());
    uint32 const now = getMSTime();
    if (state.memoValid && state.memoMs == now)
        return &state;

    state.memoMs = now;
    state.memoValid = true;

    Unit* icehowl = GetEngagedBeast(botAI, NorthrendBeast::Icehowl);
    Creature* creature = icehowl ? icehowl->ToCreature() : nullptr;

    if (!creature || creature->GetReactState() != REACT_PASSIVE)
    {
        ResetCycle(state);
        return &state;
    }

    if (!state.inCycle || getMSTimeDiff(state.lastPassiveMs, now) > CHARGE_REFRESH_GAP_MS)
    {
        ResetCycle(state);
        state.inCycle = true;
    }

    state.lastPassiveMs = now;

    // Sticky until he leaves passive, or a charge that never arrives would keep every bot's movers off
    // for the rest of the pull.
    if (state.latched && getMSTimeDiff(state.latchMs, now) > CHARGE_LATCH_MAX_MS)
    {
        state.latched = false;
        state.expired = true;
    }

    // GetPlayer finds a dead gaze target too, which the script still charges at.
    Player* target = ObjectAccessor::GetPlayer(*creature, creature->GetTarget());
    bool const atCentre = creature->GetExactDist2d(ARENA_CENTER.GetPositionX(), ARENA_CENTER.GetPositionY()) <=
                          ICEHOWL_CENTRE_TOLERANCE;

    if (state.frozen)
        return &state;

    if (atCentre)
    {
        // A gaze target dying mid gaze leaves the line where it was.
        if (target && target->IsAlive())
        {
            state.gazeTarget = target->GetGUID();
            if (!state.expired)
                LatchLane(state, creature->GetAngle(target), now);
        }
    }
    else if (target || !state.gazeTarget.IsEmpty())
    {
        state.frozen = true;
        if (target && state.gazeTarget.IsEmpty())
            state.gazeTarget = target->GetGUID();

        if (!state.expired)
            LatchLane(state, creature->GetAngle(ARENA_CENTER.GetPositionX(), ARENA_CENTER.GetPositionY()), now);
    }

    return &state;
}

constexpr uint32 BREATH_ROSTER_INTERVAL_MS = 1000;
constexpr float BREATH_POOL_STEP = 1.0f;
// Bearing counts land on whole multiples of the step, this only absorbs float noise
constexpr float BEARING_EPSILON_DEG = 0.001f;

// Also the deal order: melee before ranged, or the few bearings near his back fill with ranged first
enum class BreathRole : uint8
{
    Healer,
    Melee,
    Ranged
};

struct BreathMember
{
    ObjectGuid guid;
    BreathRole role;
};

struct BreathBearing
{
    int32 step;      // steps round from straight behind him, negative one way
    float room;      // clear floor out along it
    bool meleeOnly;  // too short for the healer and ranged bands
};

enum class BreathSpread : uint8
{
    Off = 0,
    Up = 1,
    Grace = 2,   // his victim stepped out of his reach, the layout holds as latched
    Moving = 3,  // he walks or just stopped, the layout holds as latched with no stands
    Daze = 4,
};

struct IcehowlBreathState
{
    uint32 memoMs = 0;
    bool memoValid = false;  // 0 is a real getMSTime value

    BreathSpread spread = BreathSpread::Off;
    bool active = false;
    bool latched = false;
    bool victimOut = false;
    uint32 victimOutMs = 0;
    bool movedSeen = false;  // 0 is a real getMSTime value
    uint32 lastMovingMs = 0;
    Position spot;
    float front = 0.0f;  // bearing to his victim
    float anchor = 0.0f;
    float stepDeg = 0.0f;
    std::vector<BreathBearing> bearings;
    // Slime Pools the worms left behind, at their size 5 s on
    std::vector<EncounterHelpers::HazardCircle> pools;

    uint32 rosterMs = 0;
    bool rosterValid = false;
    std::vector<BreathMember> roster;  // deal order

    std::unordered_map<ObjectGuid, int32> slots;
    bool dealDirty = false;
};

RaidInstanceState<IcehowlBreathState> breathStates;

float DegToRad(float degrees) { return degrees * static_cast<float>(M_PI) / 180.0f; }

float AngleBetween(float lhs, float rhs)
{
    return std::fabs(std::remainder(lhs - rhs, 2.0f * static_cast<float>(M_PI)));
}

// 60° on 10N from its spell_cone row, the other three ids have none
float BreathStepDeg(Unit* icehowl)
{
    float cone = ICEHOWL_BREATH_DEFAULT_CONE_DEG;
    uint32 const breath = sSpellMgr->GetSpellIdForDifficulty(SPELL_ARCTIC_BREATH, icehowl);
    if (SpellCone const* row = sSpellMgr->GetSpellCone(breath))
        if (row->cone_degrees > 0)
            cone = static_cast<float>(row->cone_degrees);

    return cone / 2.0f + ICEHOWL_SPREAD_MARGIN_DEG;
}

BreathBearing const* FindBearing(IcehowlBreathState const& state, int32 step)
{
    for (BreathBearing const& bearing : state.bearings)
        if (bearing.step == step)
            return &bearing;

    return nullptr;
}

BreathMember const* FindMember(IcehowlBreathState const& state, ObjectGuid const& guid)
{
    for (BreathMember const& member : state.roster)
        if (member.guid == guid)
            return &member;

    return nullptr;
}

float BearingAngle(IcehowlBreathState const& state, int32 step)
{
    return state.anchor + DegToRad(static_cast<float>(step) * state.stepDeg);
}

// How far out along the bearing from (x, y) the first pool ahead starts. A pool round the start itself
// is left to the stand check, which looks for a distance past it.
float PoolCut(std::vector<EncounterHelpers::HazardCircle> const& pools, float x, float y, float angle)
{
    float const dx = std::cos(angle);
    float const dy = std::sin(angle);
    float cut = std::numeric_limits<float>::max();
    for (EncounterHelpers::HazardCircle const& pool : pools)
    {
        float const ox = pool.first.GetPositionX() - x;
        float const oy = pool.first.GetPositionY() - y;
        float const along = ox * dx + oy * dy;
        float const across2 = ox * ox + oy * oy - along * along;
        float const radius2 = pool.second * pool.second;
        if (ox * ox + oy * oy <= radius2 || along <= 0.0f || across2 >= radius2)
            continue;

        cut = std::min(cut, along - std::sqrt(radius2 - across2));
    }

    return cut;
}

bool InPool(std::vector<EncounterHelpers::HazardCircle> const& pools, float x, float y)
{
    return std::any_of(pools.begin(), pools.end(), [x, y](EncounterHelpers::HazardCircle const& pool)
                       { return pool.first.GetExactDist2d(x, y) < pool.second; });
}

// The probe measured from the latched spot, and he may have drifted along the bearing since. Pools grow,
// so they cut it afresh from where he stands.
float RoomLeft(IcehowlBreathState const& state, BreathBearing const& bearing, Unit* icehowl)
{
    float const angle = BearingAngle(state, bearing.step);
    float const drift = (icehowl->GetPositionX() - state.spot.GetPositionX()) * std::cos(angle) +
                        (icehowl->GetPositionY() - state.spot.GetPositionY()) * std::sin(angle);
    return std::min(bearing.room - drift, PoolCut(state.pools, icehowl->GetPositionX(), icehowl->GetPositionY(), angle));
}

bool RoomRanShort(IcehowlBreathState const& state, Unit* icehowl)
{
    return std::any_of(state.bearings.begin(), state.bearings.end(),
                       [&state, icehowl](BreathBearing const& bearing)
                       {
                           float const minRoom =
                               bearing.meleeOnly ? ICEHOWL_SPREAD_MELEE_ROOM : ICEHOWL_SPREAD_MIN_ROOM;
                           return RoomLeft(state, bearing, icehowl) < minRoom;
                       });
}

// Past 90° off his back he parries melee and hastes his next swing
bool InMeleeArc(IcehowlBreathState const& state, int32 step)
{
    return static_cast<float>(std::abs(step)) * state.stepDeg <= ICEHOWL_SPREAD_MELEE_ARC_DEG + BEARING_EPSILON_DEG;
}

bool CanSeat(IcehowlBreathState const& state, BreathRole role, BreathBearing const& bearing)
{
    return role == BreathRole::Melee ? InMeleeArc(state, bearing.step) : !bearing.meleeOnly;
}

// Outward from his back (k = 0, 1, -1, 2, ...), short of his front by a step plus the re-latch turn, so a
// breath on his victim misses them all. Walled bearings are skipped, inside the melee arc kept for melee.
void ProbeBearings(Player* bot, Map* map, IcehowlBreathState& state)
{
    state.bearings.clear();

    float const step = state.stepDeg;
    uint32 const wanted =
        2 * static_cast<uint32>(std::floor(ICEHOWL_SPREAD_HALF_ARC_DEG / step + BEARING_EPSILON_DEG)) + 1;
    int32 const reach = static_cast<int32>(
        std::floor((180.0f - step - ICEHOWL_SPREAD_RELATCH_TURN_DEG) / step + BEARING_EPSILON_DEG));

    float const sx = state.spot.GetPositionX();
    float const sy = state.spot.GetPositionY();
    float const sz = state.spot.GetPositionZ();
    uint32 full = 0;
    for (int32 i = 0; i <= 2 * reach && full < wanted; ++i)
    {
        int32 const k = (i % 2) ? (i + 1) / 2 : -(i / 2);
        float const angle = BearingAngle(state, k);
        float x = sx + ICEHOWL_SPREAD_PROBE * std::cos(angle);
        float y = sy + ICEHOWL_SPREAD_PROBE * std::sin(angle);
        float z = sz;

        // From the bot: the floor is a gameobject, and the walk out takes a player's path over it
        float const wall =
            map->CheckCollisionAndGetValidCoords(bot, sx, sy, sz, x, y, z) ? std::hypot(x - sx, y - sy) : 0.0f;
        float const room = std::min(wall, PoolCut(state.pools, sx, sy, angle));
        if (room >= ICEHOWL_SPREAD_MIN_ROOM)
        {
            state.bearings.push_back({k, room, false});
            ++full;
        }
        else if (room >= ICEHOWL_SPREAD_MELEE_ROOM && InMeleeArc(state, k))
            state.bearings.push_back({k, room, true});
    }
}

void LatchBreath(Player* bot, Map* map, Unit* icehowl, float front, IcehowlBreathState& state)
{
    state.latched = true;
    state.spot = icehowl->GetPosition();
    state.front = front;
    state.anchor = Position::NormalizeOrientation(front + static_cast<float>(M_PI));
    state.stepDeg = BreathStepDeg(icehowl);
    ProbeBearings(bot, map, state);
    state.dealDirty = true;
}

BreathRole RoleOf(Player* member)
{
    if (PlayerbotAI::IsHeal(member))
        return BreathRole::Healer;

    return PlayerbotAI::IsRanged(member) ? BreathRole::Ranged : BreathRole::Melee;
}

// Humans can't be moved and tanks hold him
bool CanHoldBearing(Player* member, uint32 instanceId)
{
    return member && member->IsInWorld() && member->IsAlive() &&
           member->GetMapId() == TRIAL_OF_THE_CRUSADER_MAP_ID && member->GetInstanceId() == instanceId &&
           GET_PLAYERBOT_AI(member) && !IsBeastsTank(member);
}

void RebuildRoster(Player* bot, IcehowlBreathState& state, uint32 now)
{
    state.roster.clear();
    uint32 const instanceId = bot->GetInstanceId();
    auto const add = [&state, instanceId](Player* member)
    {
        if (CanHoldBearing(member, instanceId))
            state.roster.push_back({member->GetGUID(), RoleOf(member)});
    };

    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
            add(ref->GetSource());
    }
    else
        add(bot);

    std::sort(state.roster.begin(), state.roster.end(),
              [](BreathMember const& lhs, BreathMember const& rhs)
              {
                  if (lhs.role != rhs.role)
                      return lhs.role < rhs.role;

                  return lhs.guid < rhs.guid;
              });

    // Dead or gone. A bot coming back is dealt a bearing afresh.
    for (auto it = state.slots.begin(); it != state.slots.end();)
    {
        if (FindMember(state, it->first))
            ++it;
        else
            it = state.slots.erase(it);
    }

    state.rosterMs = now;
    state.rosterValid = true;
    state.dealDirty = true;
}

// Sticky while a bearing holds at most one over the emptiest its role may take. Healers pick first, one to a
// bearing near 60° off his back; the rest fill the emptiest nearest his back.
void DealBreath(IcehowlBreathState& state)
{
    size_t const count = state.bearings.size();
    if (!count)
        return;

    std::vector<uint32> occupants(count, 0);
    std::vector<uint32> healers(count, 0);
    auto const seat = [&occupants, &healers](size_t index, BreathRole role)
    {
        ++occupants[index];
        if (role == BreathRole::Healer)
            ++healers[index];
    };

    // Per roster member, its bearing's index, or count while unseated
    size_t const members = state.roster.size();
    std::vector<size_t> seats(members, count);
    for (size_t i = 0; i < members; ++i)
    {
        BreathMember const& member = state.roster[i];
        auto const slot = state.slots.find(member.guid);
        if (slot == state.slots.end())
            continue;

        auto const kept = std::find_if(state.bearings.begin(), state.bearings.end(),
                                       [&slot](BreathBearing const& bearing) { return bearing.step == slot->second; });
        if (kept != state.bearings.end() && CanSeat(state, member.role, *kept))
        {
            seats[i] = static_cast<size_t>(kept - state.bearings.begin());
            seat(seats[i], member.role);
        }
        else
            state.slots.erase(slot);
    }

    // Moving a bot needs a gap of 2, or it would only swap which bearing holds the extra one
    for (size_t i = members; i-- > 0;)
    {
        if (seats[i] == count)
            continue;

        BreathRole const role = state.roster[i].role;
        uint32 emptiest = std::numeric_limits<uint32>::max();
        for (size_t index = 0; index < count; ++index)
            if (CanSeat(state, role, state.bearings[index]))
                emptiest = std::min(emptiest, occupants[index]);

        if (occupants[seats[i]] < emptiest + 2)
            continue;

        --occupants[seats[i]];
        if (role == BreathRole::Healer)
            --healers[seats[i]];

        state.slots.erase(state.roster[i].guid);
        seats[i] = count;
    }

    std::vector<BreathMember const*> unseated;
    for (size_t i = 0; i < members; ++i)
        if (seats[i] == count)
            unseated.push_back(&state.roster[i]);

    float const stepDeg = state.stepDeg;
    auto const healerKey = [&](size_t index)
    {
        int32 const k = state.bearings[index].step;
        float const offBearing =
            std::fabs(static_cast<float>(std::abs(k)) * stepDeg - ICEHOWL_SPREAD_HEALER_BEARING_DEG);
        return std::make_tuple(healers[index], offBearing, occupants[index], k);
    };
    auto const otherKey = [&](size_t index)
    {
        int32 const k = state.bearings[index].step;
        return std::make_tuple(occupants[index], std::abs(k), k);
    };

    for (BreathMember const* member : unseated)
    {
        bool const healer = member->role == BreathRole::Healer;
        size_t best = count;
        for (size_t index = 0; index < count; ++index)
        {
            if (!CanSeat(state, member->role, state.bearings[index]))
                continue;

            if (best == count || (healer ? healerKey(index) < healerKey(best) : otherKey(index) < otherKey(best)))
                best = index;
        }

        if (best == count)
            continue;

        state.slots[member->guid] = state.bearings[best].step;
        seat(best, member->role);
    }
}

// Off under a charge and in a heroic overlap. A latched layout holds while he walks, and for the grace
// while his victim is out of reach, since a tank at the edge of it flickers in and out.
BreathSpread ReadBreathSpread(PlayerbotAI* botAI, Unit* icehowl, IcehowlBreathState& state, uint32 now,
                              Unit*& victim)
{
    // Stamped on every read, so the first latch after a charge also waits for him to settle
    if (icehowl->isMoving())
    {
        state.lastMovingMs = now;
        state.movedSeen = true;
    }

    Creature* creature = icehowl->ToCreature();
    if (!creature || creature->GetReactState() == REACT_PASSIVE)
        return BreathSpread::Off;

    if (GetBeastsStageMask(botAI) != BEASTS_STAGE_ICEHOWL)
        return BreathSpread::Off;

    Position start;
    Position end;
    if (IcehowlChargeLatched(botAI, start, end))
        return BreathSpread::Off;

    // The stun holds his events 15 s, so the next breath lands 5-8 s after it ends, usually once a Whirl
    // has moved him anyway. A stand walk here only eats the burn window.
    if (IsIcehowlStaggered(icehowl))
        return BreathSpread::Daze;

    victim = icehowl->GetVictim();
    if (!victim || !victim->ToPlayer())
        return BreathSpread::Off;

    if (state.movedSeen && getMSTimeDiff(state.lastMovingMs, now) < ICEHOWL_SPREAD_SETTLE_MS)
    {
        // The grace counts only while he stands, or a chase would use it up
        state.victimOut = false;
        return state.latched ? BreathSpread::Moving : BreathSpread::Off;
    }

    if (icehowl->IsWithinMeleeRange(victim))
    {
        state.victimOut = false;
        return BreathSpread::Up;
    }

    if (!state.latched)
        return BreathSpread::Off;

    if (!state.victimOut)
    {
        state.victimOut = true;
        state.victimOutMs = now;
    }

    return getMSTimeDiff(state.victimOutMs, now) < ICEHOWL_SPREAD_VICTIM_GRACE_MS ? BreathSpread::Grace
                                                                                  : BreathSpread::Off;
}

void TurnBreathOff(IcehowlBreathState& state)
{
    state.active = false;
    state.latched = false;
    state.victimOut = false;
}

void SetSpread(IcehowlBreathState& state, BreathSpread spread)
{
    state.spread = spread;
}

// Every bot's trigger, action and multiplier ask, so one read per instance per ms.
IcehowlBreathState* RefreshBreath(PlayerbotAI* botAI)
{
    Player* bot = botAI ? botAI->GetBot() : nullptr;
    Map* map = bot ? bot->FindMap() : nullptr;
    if (!map || map->GetId() != TRIAL_OF_THE_CRUSADER_MAP_ID || !bot->GetInstanceId() || !bot->IsAlive())
        return nullptr;

    IcehowlBreathState& state = breathStates.For(bot->GetInstanceId());
    uint32 const now = getMSTime();
    if (state.memoValid && state.memoMs == now)
        return &state;

    state.memoMs = now;
    state.memoValid = true;

    Unit* icehowl = GetEngagedBeast(botAI, NorthrendBeast::Icehowl);
    if (!icehowl)
    {
        state.slots.clear();
        state.rosterValid = false;
        state.movedSeen = false;
        TurnBreathOff(state);
        SetSpread(state, BreathSpread::Off);
        return &state;
    }

    state.pools = GetSlimePoolCircles(botAI, WORM_POOL_SOON_TICKS, WORM_POOL_CLEARANCE_PAD);
    Unit* victim = nullptr;
    BreathSpread const spread = ReadBreathSpread(botAI, icehowl, state, now, victim);
    SetSpread(state, spread);
    if (spread == BreathSpread::Off || spread == BreathSpread::Daze)
    {
        TurnBreathOff(state);
        return &state;
    }

    // A drift toward a wall gets re-probed before it eats into a kept bearing's bands. Not in the grace,
    // where the bearing to a victim out of his reach is no front to latch, nor while he moves.
    if (spread == BreathSpread::Up)
    {
        float const front = icehowl->GetAngle(victim);
        if (!state.latched || icehowl->GetExactDist2d(state.spot) > ICEHOWL_SPREAD_RELATCH_MOVE ||
            AngleBetween(front, state.front) > DegToRad(ICEHOWL_SPREAD_RELATCH_TURN_DEG) ||
            RoomRanShort(state, icehowl))
            LatchBreath(bot, map, icehowl, front, state);
    }

    // Walled in on every bearing. Stays latched, or the walls get probed again every ms until he moves.
    if (state.bearings.empty())
    {
        state.active = false;
        return &state;
    }

    if (!state.rosterValid || getMSTimeDiff(state.rosterMs, now) >= BREATH_ROSTER_INTERVAL_MS)
        RebuildRoster(bot, state, now);

    if (state.dealDirty)
    {
        DealBreath(state);
        state.dealDirty = false;
    }

    state.active = true;
    return &state;
}

BreathBearing const* SlotOf(IcehowlBreathState const& state, ObjectGuid const& guid)
{
    if (!state.active)
        return nullptr;

    auto const slot = state.slots.find(guid);
    return slot != state.slots.end() ? FindBearing(state, slot->second) : nullptr;
}

// Clamp, don't chase: the bot keeps its own distance to him inside its band
bool ResolveBreathStand(PlayerbotAI* botAI, Player* bot, IcehowlBreathState const& state,
                        IcehowlBreathStand& stand)
{
    BreathBearing const* bearing = SlotOf(state, bot->GetGUID());
    BreathMember const* member = FindMember(state, bot->GetGUID());
    Unit* icehowl = GetEngagedBeast(botAI, NorthrendBeast::Icehowl);
    if (!bearing || !member || !icehowl)
        return false;

    float bandMin = ICEHOWL_SPREAD_MELEE_MIN;
    float bandMax = ICEHOWL_SPREAD_MELEE_MAX;
    stand.trigger = ICEHOWL_SPREAD_MELEE_TRIGGER;
    stand.arrive = ICEHOWL_SPREAD_MELEE_ARRIVE;
    if (member->role == BreathRole::Healer)
    {
        bandMin = ICEHOWL_SPREAD_HEALER_MIN;
        bandMax = ICEHOWL_SPREAD_HEALER_MAX;
        stand.trigger = ICEHOWL_SPREAD_TRIGGER;
        stand.arrive = ICEHOWL_SPREAD_ARRIVE;
    }
    else if (member->role == BreathRole::Ranged)
    {
        bandMin = ICEHOWL_SPREAD_RANGED_MIN;
        bandMax = ICEHOWL_SPREAD_RANGED_MAX;
        stand.trigger = ICEHOWL_SPREAD_TRIGGER;
        stand.arrive = ICEHOWL_SPREAD_ARRIVE;
    }

    float const top = std::min(bandMax, RoomLeft(state, *bearing, icehowl) - 1.0f);
    if (top < bandMin)
        return false;

    float distance = std::clamp(bot->GetExactDist2d(icehowl), bandMin, top);
    float const angle = BearingAngle(state, bearing->step);
    float const hx = icehowl->GetPositionX();
    float const hy = icehowl->GetPositionY();

    // A pool round him: the band distance nearest the bot's own that clears it, else no stand
    if (InPool(state.pools, hx + distance * std::cos(angle), hy + distance * std::sin(angle)))
    {
        bool found = false;
        float best = distance;
        for (float candidate = bandMin; candidate <= top; candidate += BREATH_POOL_STEP)
        {
            if (InPool(state.pools, hx + candidate * std::cos(angle), hy + candidate * std::sin(angle)))
                continue;

            if (!found || std::fabs(candidate - distance) < std::fabs(best - distance))
                best = candidate;

            found = true;
        }

        if (!found)
            return false;

        distance = best;
    }

    stand.spot = Position(hx + distance * std::cos(angle), hy + distance * std::sin(angle), icehowl->GetPositionZ());
    stand.icehowl = icehowl->GetPosition();
    stand.bearing = angle;
    stand.bandMin = bandMin;
    stand.bandMax = top;
    return true;
}
}  // namespace

bool IcehowlChargeLatched(PlayerbotAI* botAI, Position& start, Position& end)
{
    IcehowlChargeState const* state = RefreshCharge(botAI);
    if (!state || !state->latched)
        return false;

    start = state->start;
    end = state->end;
    return true;
}

float DistanceToIcehowlCharge(PlayerbotAI* botAI, float x, float y)
{
    Position start;
    Position end;
    if (!IcehowlChargeLatched(botAI, start, end))
        return std::numeric_limits<float>::max();

    return DistanceToIcehowlLane(start, end, x, y);
}

float DistanceToIcehowlLane(Position const& start, Position const& end, float x, float y)
{
    float const sx = start.GetPositionX();
    float const sy = start.GetPositionY();
    float const dx = end.GetPositionX() - sx;
    float const dy = end.GetPositionY() - sy;
    float const lengthSq = dx * dx + dy * dy;

    float t = lengthSq > 0.0f ? ((x - sx) * dx + (y - sy) * dy) / lengthSq : 0.0f;
    t = std::clamp(t, 0.0f, 1.0f);

    return std::hypot(x - (sx + t * dx), y - (sy + t * dy));
}

float IcehowlBreathStepDeg(Unit* anyUnitOnMap) { return BreathStepDeg(anyUnitOnMap); }

bool IsIcehowlStaggered(Unit* icehowl) { return icehowl && icehowl->HasAura(SPELL_STAGGERED_DAZE); }

bool IcehowlDazeNitroDue(PlayerbotAI* botAI)
{
    Player* bot = botAI ? botAI->GetBot() : nullptr;
    if (!bot || !bot->IsAlive() || !PlayerbotAI::IsMelee(bot) || !botAI->CanMove() ||
        bot->IsNonMeleeSpellCast(false, true, true) || !EncounterHelpers::NitroBoostsReady(bot))
        return false;

    Unit* icehowl = GetEngagedBeast(botAI, NorthrendBeast::Icehowl);
    Aura* daze = icehowl ? icehowl->GetAura(SPELL_STAGGERED_DAZE) : nullptr;
    if (!daze || daze->GetDuration() < ICEHOWL_DAZE_NITRO_MIN_LEFT_MS)
        return false;

    return bot->GetExactDist2d(icehowl) - ICEHOWL_MELEE_RANGE >= ICEHOWL_DAZE_NITRO_FROM;
}

bool HasIcehowlFrothingRage(Unit* icehowl)
{
    return icehowl && icehowl->HasAura(sSpellMgr->GetSpellIdForDifficulty(SPELL_FROTHING_RAGE, icehowl));
}

bool GetIcehowlBreathStand(PlayerbotAI* botAI, IcehowlBreathStand& stand)
{
    Player* bot = botAI ? botAI->GetBot() : nullptr;
    if (!bot)
        return false;

    IcehowlBreathState const* state = RefreshBreath(botAI);
    return state && state->spread != BreathSpread::Moving && ResolveBreathStand(botAI, bot, *state, stand);
}

bool IsOnIcehowlBreathStand(IcehowlBreathStand const& stand, float x, float y, float degrees, float yards)
{
    float const dx = x - stand.icehowl.GetPositionX();
    float const dy = y - stand.icehowl.GetPositionY();
    float const distance = std::hypot(dx, dy);
    if (distance < stand.bandMin - yards || distance > stand.bandMax + yards)
        return false;

    return AngleBetween(std::atan2(dy, dx), stand.bearing) <= DegToRad(degrees);
}

bool HasIcehowlBreathSlot(PlayerbotAI* botAI)
{
    Player* bot = botAI ? botAI->GetBot() : nullptr;
    IcehowlBreathState const* state = bot ? RefreshBreath(botAI) : nullptr;
    return state && SlotOf(*state, bot->GetGUID());
}

}
