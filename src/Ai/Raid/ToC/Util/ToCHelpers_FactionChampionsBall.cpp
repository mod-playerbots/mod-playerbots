/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "ToCHelpers_FactionChampionsBall.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <unordered_map>
#include <vector>

#include "EncounterHelpers.h"
#include "GameObject.h"
#include "Group.h"
#include "Map.h"
#include "ObjectAccessor.h"
#include "Player.h"
#include "Playerbots.h"
#include "RaidInstanceState.h"
#include "Spell.h"
#include "SpellAuras.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include "Timer.h"
#include "ToCData.h"
#include "ToCEncounterGate.h"
#include "ToCHelpers_FactionChampions.h"
#include "ToCHelpers_FactionChampionsDefence.h"
#include "ToCHelpers_Shared.h"
#include "ToCHelpers_Staging.h"

namespace TrialOfTheCrusaderHelpers
{

namespace
{
// Same floor the grip stands and the Beasts spots keep to
constexpr float ARENA_FLOOR_RADIUS = 44.0f;

constexpr float SIDE_STEP_DEG = 15.0f;
// a fresh Cyclone stand prefers one still in reach after the pack healer drifts this far
constexpr float SIDE_REANCHOR = 6.0f;
// Slack inside Cyclone's reach for the healer's drift
constexpr float CYCLONE_STAND_MARGIN = 2.0f;
// The Cyclone stand's rings round the kill target. 30 yd still sits inside Wrath's 33 with Nature's Reach.
constexpr std::array<float, 3> CYCLONE_RINGS = {20.0f, 26.0f, 30.0f};

// a healer steps off melee champions this close, this far, then stands again for a while so one that
// follows doesn't turn it into a kite
constexpr float STEP_OFF_TRIGGER = 8.0f;
constexpr float STEP_OFF_DIST = 10.0f;
constexpr uint32 STEP_OFF_MS = 2500;
constexpr uint32 STEP_OFF_REST_MS = 3000;

// Nitro Boosts are for running from a focus: this many champions on the bot, or about to be
constexpr uint32 NITRO_FOCUS = 3;
// lower, the boots no longer outrun the hits
constexpr float NITRO_MIN_HEALTH_PCT = 30.0f;
// a free melee champion this close keeps hitting through a kite, only a speed boost opens a gap on it
constexpr float BOOST_CONTACT = 8.0f;
// each bearing's run steps back this much from the boost's reach till it ends on clear floor
constexpr float NITRO_STEP_BACK = 5.0f;
constexpr float NITRO_MIN_RUN = 15.0f;
// off straight away from the chasers, for the Nitro run and each kite hop
constexpr std::array<float, 9> RUN_TURNS = {0.0f, 30.0f, -30.0f, 60.0f, -60.0f, 90.0f, -90.0f, 120.0f, -120.0f};
constexpr float RUN_ARRIVE = 2.0f;
// the focus on a bot fearing to be picked next, read off the champions around it
constexpr float NEXT_FOCUS_RADIUS = 30.0f;

constexpr uint32 OPENER_MIN_CHAMPIONS = 3;
// inside the AoE by this much, so a step by the champions doesn't leave them out
constexpr float OPENER_MARGIN = 1.0f;

struct OpenerSpell
{
    std::array<uint32, 6> ranks;  // highest first, 0 pads
};

constexpr OpenerSpell INTIMIDATING_SHOUT{{5246, 0, 0, 0, 0, 0}};
constexpr OpenerSpell PSYCHIC_SCREAM{{10890, 10888, 8124, 8122, 0, 0}};
constexpr OpenerSpell HUNGERING_COLD{{49203, 0, 0, 0, 0, 0}};
// only once talented instant
constexpr OpenerSpell HOWL_OF_TERROR{{17928, 5484, 0, 0, 0, 0}};
constexpr OpenerSpell FROST_NOVA{{42917, 27088, 10230, 6131, 865, 122}};
constexpr OpenerSpell DRAGONS_BREATH{{42950, 42949, 33043, 33042, 33041, 31661}};
constexpr OpenerSpell SHADOWFURY{{47847, 47846, 30414, 30413, 30283, 0}};

// Feared and frozen both pass the champions' CC test, which pops the trinket
constexpr std::array<OpenerSpell, 5> OPENER_SPELLS = {INTIMIDATING_SHOUT, PSYCHIC_SCREAM, HUNGERING_COLD,
                                                      HOWL_OF_TERROR, FROST_NOVA};

constexpr OpenerSpell SPRINT{{11305, 8696, 2983, 0, 0, 0}};
constexpr OpenerSpell DASH{{33357, 9821, 1850, 0, 0, 0}};

// A healer or ranged bot runs or CCs its way out once a melee champion on it is this close
constexpr float ESCAPE_MELEE_REACH = 8.0f;
// the circle has to put this much more ground between the bot and the melee on it
constexpr float ESCAPE_MIN_GAIN = 12.0f;
constexpr float BLINK_DIST = 20.0f;
// an AoE's search counts both ends' size on top of its radius
constexpr float ESCAPE_AOE_SLACK = 2.0f;
constexpr uint32 SPELL_BLINK = 1953;
constexpr uint32 SPELL_DEMONIC_CIRCLE_TELEPORT = 48020;
// on the warlock while its circle is within the teleport's 40 yd
constexpr uint32 SPELL_DEMONIC_CIRCLE_IN_RANGE = 62388;
// the circle lasts 6 min, a long wait between pulls gets a fresh one before it runs out
constexpr int32 CIRCLE_REFRESH_MS = 60000;
// two melee champions on a cloth caster kill it in seconds, well before a third arrives
constexpr uint32 NITRO_MELEE_FOCUS = 2;
// Blink flees every melee champion this close, and a landing nearer one than this isn't worth the cooldown
constexpr float BLINK_FLEE_RADIUS = 15.0f;
constexpr float BLINK_MIN_CLEAR = 12.0f;
constexpr uint32 SPELL_DISENGAGE = 781;
// Shares its 30 s cooldown with Freezing Arrow, the peel's. Its trap arms 1 s after it's set in combat and
// goes off at 2.5 yd, laying a 10 yd slick that slows 50% for 30 s; Entrapment adds a root there.
constexpr uint32 SPELL_FROST_TRAP = 13809;
constexpr float FROST_TRAP_SLICK = 10.0f;
constexpr float DISENGAGE_REACH = 5.0f;
constexpr float DISENGAGE_DIST = 15.0f;

// cloth beats a plate DPS in melee for the re-pick up to ~27 yd, plus the 110% switch margin
constexpr float KITE_CLEAR = 30.0f;
constexpr float KITE_STEP = 15.0f;
constexpr uint32 KITE_STEP_MS = 2500;
// a chaser dragged onto a healer would just re-pick the healer
constexpr float KITE_HEALER_GAP = 10.0f;
constexpr float KITE_HOLD = 45.0f;
// a chaser this slow only catches a bot standing next to it, so its victim keeps casting till it's near
constexpr float KITE_CRAWL_RATE = 0.6f;
constexpr float KITE_CRAWL_REACH = 10.0f;
// the Ret's Hammer of Justice goes on its target within 15 yd, so its victim leaves before that
constexpr float KITE_CRAWL_REACH_RET = 17.0f;
// the Hands' reach, heals go to 40 so the healers can drift a bit
constexpr float KITE_HEAL_REACH = 30.0f;
constexpr uint32 CAST_BREAK_HOLD_MS = 1000;

struct Vec
{
    float x;
    float y;
};

struct Run
{
    uint32 sinceMs = 0;
    Position spot;
};

// Kept while the healer and the kill target stay in reach and the spot stays clear
struct CycloneStand
{
    ObjectGuid kill;
    ObjectGuid healer;
    Position spot;
};

struct BallState
{
    std::unordered_map<ObjectGuid, uint32> opener;

    uint32 memoMs = 0;
    bool memoValid = false;
    std::unordered_map<ObjectGuid, Run> runs;   // chased bot -> its boosted run
    std::unordered_map<ObjectGuid, Run> steps;  // healer -> its step off the melee
    std::unordered_map<ObjectGuid, Run> kites;  // kiting bot -> its current hop
    std::unordered_map<ObjectGuid, uint32> stepEndMs;
    std::unordered_map<ObjectGuid, CycloneStand> cycloneStands;
    std::unordered_map<ObjectGuid, uint32> castBreaks;  // chased caster -> when its cast break was asked
    // Living healers on the map, all a Nitro run or kite hop measures its spots against
    ToCGroupMemo<std::vector<ObjectGuid>> healers;
};

RaidInstanceState<BallState> ballStates;

Vec Rotate(Vec v, float degrees)
{
    float const r = degrees * static_cast<float>(M_PI) / 180.0f;
    return {v.x * std::cos(r) - v.y * std::sin(r), v.x * std::sin(r) + v.y * std::cos(r)};
}

Vec Unit2(float x, float y)
{
    float const length = std::hypot(x, y);
    return length > 0.0f ? Vec{x / length, y / length} : Vec{1.0f, 0.0f};
}

bool OnFloor(Position const& spot) { return ARENA_CENTER.GetExactDist2d(spot) <= ARENA_FLOOR_RADIUS; }

Position Along(Position const& from, Vec dir, float distance)
{
    return Position(from.GetPositionX() + dir.x * distance, from.GetPositionY() + dir.y * distance,
                    from.GetPositionZ());
}

std::vector<Player*> GroupOnMap(Player* bot)
{
    std::vector<Player*> members;
    Map* map = bot->FindMap();
    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (member && member->IsInWorld() && member->IsAlive() && member->FindMap() == map)
                members.push_back(member);
        }
    }
    else
        members.push_back(bot);

    std::sort(members.begin(), members.end(),
              [](Player* a, Player* b) { return a->GetGUID() < b->GetGUID(); });
    return members;
}

std::vector<Player*> HealersOnMap(Player* bot, BallState const& state)
{
    std::vector<ObjectGuid> const& guids = state.healers.Get(bot, [bot](std::vector<ObjectGuid>& healers)
    {
        for (Player* member : GroupOnMap(bot))
            if (PlayerbotAI::IsHeal(member))
                healers.push_back(member->GetGUID());
    });

    std::vector<Player*> healers;
    for (ObjectGuid const& guid : guids)
        if (Player* healer = ObjectAccessor::GetPlayer(*bot, guid))
            healers.push_back(healer);

    return healers;
}

uint32 KnownOpenerRank(Player* player, OpenerSpell const& spell)
{
    for (uint32 rank : spell.ranks)
        if (rank && player->HasSpell(rank))
            return rank;

    return 0;
}

// An instant the bot knows, off cooldown and affordable
SpellInfo const* ReadyInstant(Player* player, uint32 rank)
{
    SpellInfo const* info = rank ? sSpellMgr->GetSpellInfo(rank) : nullptr;
    if (!info || player->HasSpellCooldown(rank) || info->CalcCastTime(player) ||
        info->PowerType >= static_cast<uint32>(MAX_POWERS))
        return nullptr;

    return info->CalcPowerCost(player, info->GetSchoolMask()) <=
                   static_cast<int32>(player->GetPower(Powers(info->PowerType)))
               ? info
               : nullptr;
}

float SpellRadius(SpellInfo const* info, Unit* caster)
{
    float radius = 0.0f;
    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
        if (info->Effects[i].IsEffect())
            radius = std::max(radius, info->Effects[i].CalcRadius(caster));

    return radius;
}

bool IsGripDk(Player* player)
{
    PlayerbotAI* memberAI = GET_PLAYERBOT_AI(player);
    Position stand;
    return memberAI && FactionChampionsGripTarget(memberAI, stand) != nullptr;
}

// Champions with their trinket up inside the spell's reach of the caster, also listed into inReach if given.
// None while one the grip guard holds is anywhere near it: a fear or freeze ends a pull where it catches the champion.
size_t CollectTrinketsInReach(PlayerbotAI* botAI, Player* caster, SpellInfo const* info,
                              std::vector<Unit*> const& champions, std::vector<Unit*>* inReach)
{
    size_t count = 0;
    float const radius = SpellRadius(info, caster);
    for (Unit* champion : champions)
    {
        float const distance = caster->GetExactDist2d(champion);
        if (distance <= radius + OPENER_MARGIN && FactionChampionsGripGuarded(botAI, champion))
        {
            if (inReach)
                inReach->clear();

            return 0;
        }

        if (distance <= radius - OPENER_MARGIN && FactionChampionsTrinketReady(champion) &&
            !champion->IsImmunedToSpell(info))
        {
            ++count;
            if (inReach)
                inReach->push_back(champion);
        }
    }

    return count;
}

std::vector<Unit*> TrinketsInReach(PlayerbotAI* botAI, Player* caster, SpellInfo const* info,
                                   std::vector<Unit*> const& champions)
{
    std::vector<Unit*> inReach;
    CollectTrinketsInReach(botAI, caster, info, champions, &inReach);
    return inReach;
}

// Heroic: the bot that reaches the most champions with their trinket up, three or more, with a ready AoE
// fear or Frost Nova, casts it where it stands. A trinket used comes back in 2 min, and so does this.
void UpdateOpener(BallState& state, PlayerbotAI* botAI, std::vector<Player*> const& members)
{
    Map* map = botAI->GetBot()->FindMap();
    std::vector<Unit*> champions;
    if (map && map->IsHeroic())
        champions = FactionChampionsInCombat(botAI);

    ObjectGuid const holding = state.opener.empty() ? ObjectGuid::Empty : state.opener.begin()->first;
    Player* best = nullptr;
    uint32 bestSpell = 0;
    size_t bestCount = 0;
    for (Player* member : members)
    {
        if (champions.size() < OPENER_MIN_CHAMPIONS || !GET_PLAYERBOT_AI(member) || PlayerbotAI::IsHeal(member) ||
            member->HasUnitState(UNIT_STATE_LOST_CONTROL) || member->HasUnitFlag(UNIT_FLAG_SILENCED) ||
            IsGripDk(member))
            continue;

        for (OpenerSpell const& spell : OPENER_SPELLS)
        {
            uint32 const rank = KnownOpenerRank(member, spell);
            SpellInfo const* info = ReadyInstant(member, rank);
            if (!info)
                continue;

            size_t const count = CollectTrinketsInReach(botAI, member, info, champions, nullptr);
            // the holder keeps a tie, so the duty doesn't hop between two bots
            bool const better = count > bestCount || (count == bestCount && member->GetGUID() == holding);
            if (count >= OPENER_MIN_CHAMPIONS && better)
            {
                best = member;
                bestSpell = rank;
                bestCount = count;
            }
        }
    }

    if (!holding.IsEmpty() && (!best || best->GetGUID() != holding))
        state.opener.erase(holding);

    if (best)
        state.opener[best->GetGUID()] = bestSpell;
}

bool Clear(Position const& spot, std::vector<EncounterHelpers::HazardCircle> const& hazards)
{
    return OnFloor(spot) && std::none_of(hazards.begin(), hazards.end(),
                                         [&spot](EncounterHelpers::HazardCircle const& hazard)
                                         { return spot.GetExactDist2d(hazard.first) < hazard.second; });
}

// Distance from the spot to the nearest melee champion, max with none
float MeleeClearance(PlayerbotAI* botAI, Position const& spot)
{
    float nearest = std::numeric_limits<float>::max();
    for (Unit* champion : FactionChampionsInCombat(botAI))
        if (IsFactionChampionMelee(champion->GetEntry()))
            nearest = std::min(nearest, champion->GetExactDist2d(&spot));

    return nearest;
}

std::vector<Unit*> MeleeOn(PlayerbotAI* botAI, Player* bot, float radius)
{
    std::vector<Unit*> on;
    for (Unit* champion : FactionChampionsInCombat(botAI))
        if (FactionChampionsKitesFrom(bot, champion) && bot->GetExactDist2d(champion) <= radius)
            on.push_back(champion);

    return on;
}

// rooted, slowed or stunned, it drops behind a walking kite anyway
bool Unhindered(Unit* champion)
{
    return !champion->HasUnitState(UNIT_STATE_LOST_CONTROL) && !champion->HasAuraType(SPELL_AURA_MOD_ROOT) &&
           !champion->HasAuraType(SPELL_AURA_MOD_DECREASE_SPEED);
}

bool Crawling(Unit* champion)
{
    return champion->HasUnitState(UNIT_STATE_LOST_CONTROL) || champion->HasAuraType(SPELL_AURA_MOD_ROOT) ||
           champion->GetSpeedRate(MOVE_RUN) <= KITE_CRAWL_RATE;
}

float CrawlReach(Unit* champion)
{
    uint32 const entry = champion->GetEntry();
    bool const ret = entry == static_cast<uint32>(ToCFactionChampions::NPC_ALLIANCE_PALADIN_RETRIBUTION) ||
                     entry == static_cast<uint32>(ToCFactionChampions::NPC_HORDE_PALADIN_RETRIBUTION);
    return ret ? KITE_CRAWL_REACH_RET : KITE_CRAWL_REACH;
}

// The ones its kite runs from, a crawling one only once it's near
std::vector<Unit*> KiteChasers(PlayerbotAI* botAI, Player* bot)
{
    std::vector<Unit*> chasers = MeleeOn(botAI, bot, KITE_CLEAR);
    chasers.erase(std::remove_if(chasers.begin(), chasers.end(), [bot](Unit* chaser)
                                 { return Crawling(chaser) && bot->GetExactDist2d(chaser) > CrawlReach(chaser); }),
                  chasers.end());
    return chasers;
}

bool KiteFree(BallState const& state, PlayerbotAI* botAI, Player* bot)
{
    return !bot->HasUnitState(UNIT_STATE_LOST_CONTROL) && !bot->HasAuraType(SPELL_AURA_MOD_ROOT) &&
           !state.runs.count(bot->GetGUID()) && !FactionChampionsHasCcDuty(botAI, bot->GetGUID());
}

// Other living healers: a spot this close to one drags the chaser onto it
bool NearHealer(Player* bot, std::vector<Player*> const& members, Position const& spot)
{
    return std::any_of(members.begin(), members.end(),
                       [bot, &spot](Player* member)
                       {
                           return member != bot && PlayerbotAI::IsHeal(member) &&
                                  member->GetExactDist2d(&spot) < KITE_HEALER_GAP;
                       });
}

// Nearest living healer other than the bot, max with none
float HealerGap(Player* bot, std::vector<Player*> const& members, Position const& spot)
{
    float nearest = std::numeric_limits<float>::max();
    for (Player* member : members)
        if (member != bot && member->IsAlive() && PlayerbotAI::IsHeal(member))
            nearest = std::min(nearest, member->GetExactDist2d(&spot));

    return nearest;
}

// In heal reach, or nearer a healer than the bot is now so one out of reach walks back in
bool Healed(float gap, float nowGap)
{
    return gap <= KITE_HEAL_REACH || gap < nowGap || nowGap == std::numeric_limits<float>::max();
}

// A ring spot round the kill target still in Cyclone reach of the pack healer after a SIDE_REANCHOR drift,
// else in reach now, else nearest it. A held spot stays while both are in reach, so no walk per drift.
bool CycloneSpot(BallState& state, Player* druid, Unit* kill, Unit* healer, Position const& centre,
                 std::vector<EncounterHelpers::HazardCircle> const& hazards, Position& spot)
{
    SpellInfo const* info = sSpellMgr->GetSpellInfo(SPELL_CYCLONE);
    float const reach = (info ? info->GetMaxRange(false, druid) : 0.0f) + druid->GetCombatReach() +
                        healer->GetCombatReach() - CYCLONE_STAND_MARGIN;
    auto const held = state.cycloneStands.find(druid->GetGUID());
    if (held != state.cycloneStands.end() && held->second.kill == kill->GetGUID() &&
        held->second.healer == healer->GetGUID() && healer->GetExactDist2d(&held->second.spot) <= reach &&
        kill->GetExactDist2d(&held->second.spot) <= CYCLONE_RINGS.back() + SIDE_REANCHOR &&
        Clear(held->second.spot, hazards))
    {
        spot = held->second.spot;
        return true;
    }

    Vec const out = Unit2(kill->GetPositionX() - centre.GetPositionX(), kill->GetPositionY() - centre.GetPositionY());
    uint32 const steps = static_cast<uint32>(360.0f / SIDE_STEP_DEG);

    // 0 holds through the healer's drift, 1 reaches it now, 2 doesn't
    auto const tier = [&](Position const& candidate)
    {
        float const toHealer = healer->GetExactDist2d(&candidate);
        return toHealer <= reach - SIDE_REANCHOR ? 0 : (toHealer <= reach ? 1 : 2);
    };

    bool found = false;
    int bestTier = 2;
    Position best;
    for (float ring : CYCLONE_RINGS)
    {
        for (uint32 step = 0; step < steps; ++step)
        {
            Position const candidate = Along(kill->GetPosition(), Rotate(out, SIDE_STEP_DEG * step), ring);
            if (!Clear(candidate, hazards))
                continue;

            int const candidateTier = tier(candidate);
            bool better = !found || candidateTier < bestTier;
            if (found && candidateTier == bestTier)
            {
                if (candidateTier == 2)
                    better = healer->GetExactDist2d(&candidate) < healer->GetExactDist2d(&best);
                else if (centre.GetExactDist2d(&candidate) != centre.GetExactDist2d(&best))
                    better = centre.GetExactDist2d(&candidate) > centre.GetExactDist2d(&best);
                else
                    better = druid->GetExactDist2d(&candidate) < druid->GetExactDist2d(&best);
            }

            if (better)
            {
                found = true;
                bestTier = candidateTier;
                best = candidate;
            }
        }
    }

    if (!found)
        return false;

    state.cycloneStands[druid->GetGUID()] = {kill->GetGUID(), healer->GetGUID(), best};
    spot = best;
    return true;
}

// Healers and ranged stand where they are. Only the Cyclone druid gets a stand, once the kill target has been
// out and a champion healer is left in the pack.
void UpdateCycloneStands(BallState& state, PlayerbotAI* botAI, std::vector<Player*> const& members)
{
    Position anchor;
    Position centre;
    Unit* killTarget = FactionChampionsKillTarget(botAI);
    // latched rather than IsPulled, which flips each time a champion steps near the kill target
    Unit* packHealer = killTarget && FactionChampionsPack(botAI, anchor, centre) && FactionChampionsKillTargetOut(botAI)
                           ? FactionChampionsBlobHealer(botAI)
                           : nullptr;
    std::vector<ObjectGuid> covering;
    if (packHealer)
    {
        std::vector<EncounterHelpers::HazardCircle> const hazards = FactionChampionsAoeClearances(botAI);
        for (Player* member : members)
        {
            Position spot;
            if (GET_PLAYERBOT_AI(member) && !PlayerbotAI::IsTank(member) && !PlayerbotAI::IsHeal(member) &&
                PlayerbotAI::IsRanged(member) && FactionChampionsCycloneBot(member) &&
                CycloneSpot(state, member, killTarget, packHealer, centre, hazards, spot))
                covering.push_back(member->GetGUID());
        }
    }

    auto const covers = [&covering](ObjectGuid guid)
    { return std::find(covering.begin(), covering.end(), guid) != covering.end(); };
    std::vector<ObjectGuid> released;
    for (auto const& held : state.cycloneStands)
        if (!covers(held.first))
            released.push_back(held.first);

    for (ObjectGuid const& guid : released)
        state.cycloneStands.erase(guid);
}

// A bot mid hard cast runs no triggers, so its kite can't break the cast. This breaks it from any bot's tick
// once a chaser would reach melee before the cast lands.
void BreakChasedCasts(BallState& state, PlayerbotAI* botAI, std::vector<Player*> const& members, uint32 now)
{
    for (Player* member : members)
    {
        PlayerbotAI* memberAI = GET_PLAYERBOT_AI(member);
        Spell* cast = member->GetCurrentSpell(CURRENT_GENERIC_SPELL);
        if (!memberAI || !cast || cast->getState() != SPELL_STATE_PREPARING || !member->IsMovementPreventedByCasting())
            continue;

        // its own tick takes the request, until then every refresh would ask again
        auto const asked = state.castBreaks.find(member->GetGUID());
        if (asked != state.castBreaks.end() && getMSTimeDiff(asked->second, now) < CAST_BREAK_HOLD_MS)
            continue;

        float const leftSec = static_cast<float>(std::max(cast->GetCastTimeRemaining(), 0)) / 1000.0f;
        std::vector<Unit*> const chasers = KiteChasers(botAI, member);
        bool const caught = std::any_of(chasers.begin(), chasers.end(), [member, leftSec](Unit* chaser)
                                        {
                                            return member->GetExactDist2d(chaser) - chaser->GetMeleeRange(member) <=
                                                   chaser->GetSpeed(MOVE_RUN) * leftSec;
                                        });
        if (!caught || !KiteFree(state, botAI, member))
            continue;

        state.castBreaks[member->GetGUID()] = now;
        memberAI->RequestSpellInterrupt();
    }
}

// Null unless live, so no duty outlives the fight
BallState* Refresh(PlayerbotAI* botAI)
{
    if (!botAI || !ToCEncounterIsLive(botAI, ToCEncounter::FactionChampions))
        return nullptr;

    Player* bot = botAI->GetBot();
    if (!bot->GetInstanceId())
        return nullptr;

    BallState& state = ballStates.For(bot->GetInstanceId());
    uint32 const now = getMSTime();
    if (state.memoValid && state.memoMs == now)
        return &state;

    state.memoMs = now;
    state.memoValid = true;

    std::vector<Player*> const members = GroupOnMap(bot);
    UpdateOpener(state, botAI, members);
    UpdateCycloneStands(state, botAI, members);
    BreakChasedCasts(state, botAI, members, now);
    return &state;
}
}  // namespace

bool FactionChampionsCycloneStand(PlayerbotAI* botAI, Position& spot)
{
    BallState* state = Refresh(botAI);
    if (!state)
        return false;

    // a CC duty may walk it into spell range, its stand waits for it
    ObjectGuid const guid = botAI->GetBot()->GetGUID();
    auto const stand = state->cycloneStands.find(guid);
    if (stand == state->cycloneStands.end() || FactionChampionsHasCcDuty(botAI, guid))
        return false;

    spot = stand->second.spot;
    return true;
}

bool FactionChampionsStaysPut(PlayerbotAI* botAI)
{
    Player* bot = botAI ? botAI->GetBot() : nullptr;
    return bot && Refresh(botAI) && !PlayerbotAI::IsTank(bot) &&
           (PlayerbotAI::IsHeal(bot) || PlayerbotAI::IsRanged(bot));
}

// no refresh here, the champion refresh asks it for its CC duties
bool FactionChampionsKitesFrom(Player* member, Unit* champion)
{
    if (!member || !champion || !member->IsAlive() || !IsFactionChampionMelee(champion->GetEntry()) ||
        champion->GetVictim() != member)
        return false;

    return !PlayerbotAI::IsTank(member) || FactionChampionsIceDutyOf(member) == champion->GetGUID();
}

bool FactionChampionsStepOff(PlayerbotAI* botAI, Position& spot)
{
    BallState* state = Refresh(botAI);
    if (!state)
        return false;

    Player* bot = botAI->GetBot();
    ObjectGuid const guid = bot->GetGUID();
    uint32 const now = getMSTime();
    auto const step = state->steps.find(guid);
    if (step != state->steps.end())
    {
        if (getMSTimeDiff(step->second.sinceMs, now) < STEP_OFF_MS &&
            bot->GetExactDist2d(&step->second.spot) > RUN_ARRIVE)
        {
            spot = step->second.spot;
            return true;
        }

        state->steps.erase(step);
        state->stepEndMs[guid] = now;
    }

    auto const rested = state->stepEndMs.find(guid);
    if (!PlayerbotAI::IsHeal(bot) || PlayerbotAI::IsTank(bot) || bot->HasUnitState(UNIT_STATE_LOST_CONTROL) ||
        bot->HasAuraType(SPELL_AURA_MOD_ROOT) || bot->IsNonMeleeSpellCast(false) || state->runs.count(guid) ||
        state->kites.count(guid) ||
        (rested != state->stepEndMs.end() && getMSTimeDiff(rested->second, now) < STEP_OFF_REST_MS))
        return false;

    uint32 nearby = 0;
    float sumX = 0.0f;
    float sumY = 0.0f;
    for (Unit* champion : FactionChampionsInCombat(botAI))
    {
        if (!IsFactionChampionMelee(champion->GetEntry()) || champion->HasUnitState(UNIT_STATE_LOST_CONTROL) ||
            bot->GetExactDist2d(champion) > STEP_OFF_TRIGGER)
            continue;

        ++nearby;
        sumX += champion->GetPositionX();
        sumY += champion->GetPositionY();
    }

    if (!nearby)
        return false;

    std::vector<EncounterHelpers::HazardCircle> const hazards = FactionChampionsAoeClearances(botAI);
    Vec const away = Unit2(bot->GetPositionX() - sumX / nearby, bot->GetPositionY() - sumY / nearby);
    for (float turn : {0.0f, 45.0f, -45.0f, 90.0f, -90.0f})
    {
        Position const target = Along(bot->GetPosition(), Rotate(away, turn), STEP_OFF_DIST);
        if (!Clear(target, hazards))
            continue;

        state->steps[guid] = {now, target};
        spot = target;
        return true;
    }

    return false;
}

Unit* FactionChampionsOpener(PlayerbotAI* botAI, uint32& spellId)
{
    BallState* state = Refresh(botAI);
    if (!state)
        return nullptr;

    Player* bot = botAI->GetBot();
    auto const held = state->opener.find(bot->GetGUID());
    SpellInfo const* info = held != state->opener.end() ? sSpellMgr->GetSpellInfo(held->second) : nullptr;
    if (!info)
        return nullptr;

    spellId = held->second;
    if (!info->NeedsExplicitUnitTarget())
        return bot;

    // Intimidating Shout stuns its target and fears the rest round the warrior
    Unit* nearest = nullptr;
    for (Unit* champion : TrinketsInReach(botAI, bot, info, FactionChampionsInCombat(botAI)))
        if (!nearest || bot->GetExactDist2d(champion) < bot->GetExactDist2d(nearest))
            nearest = champion;

    return nearest;
}

bool FactionChampionsChased(PlayerbotAI* botAI, Position& spot)
{
    BallState* state = Refresh(botAI);
    if (!state)
        return false;

    Player* bot = botAI->GetBot();
    uint32 const now = getMSTime();
    auto const run = state->runs.find(bot->GetGUID());
    if (run != state->runs.end())
    {
        if (getMSTimeDiff(run->second.sinceMs, now) < EncounterHelpers::NITRO_BOOSTS_DURATION_MS &&
            bot->GetExactDist2d(&run->second.spot) > RUN_ARRIVE)
        {
            spot = run->second.spot;
            return true;
        }

        state->runs.erase(run);
    }

    if (PlayerbotAI::IsTank(bot) || bot->HasUnitState(UNIT_STATE_LOST_CONTROL) ||
        bot->HasAuraType(SPELL_AURA_MOD_ROOT) || FactionChampionsHasCcDuty(botAI, bot->GetGUID()) ||
        bot->GetHealthPct() < NITRO_MIN_HEALTH_PCT || !EncounterHelpers::NitroBoostsReady(bot))
        return false;

    uint32 on = 0;
    uint32 meleeOn = 0;
    uint32 contact = 0;
    std::array<float, 2> onSum = {0.0f, 0.0f};
    std::array<float, 2> meleeSum = {0.0f, 0.0f};
    std::array<float, 2> nearSum = {0.0f, 0.0f};
    uint32 nearby = 0;
    for (Unit* champion : FactionChampionsInCombat(botAI))
    {
        if (champion->HasUnitState(UNIT_STATE_LOST_CONTROL))
            continue;

        if (champion->GetVictim() == bot)
        {
            ++on;
            onSum[0] += champion->GetPositionX();
            onSum[1] += champion->GetPositionY();
            if (IsFactionChampionMelee(champion->GetEntry()))
            {
                ++meleeOn;
                meleeSum[0] += champion->GetPositionX();
                meleeSum[1] += champion->GetPositionY();
                if (bot->GetExactDist2d(champion) <= BOOST_CONTACT && Unhindered(champion))
                    ++contact;
            }
        }

        if (bot->GetExactDist2d(champion) <= NEXT_FOCUS_RADIUS)
        {
            ++nearby;
            nearSum[0] += champion->GetPositionX();
            nearSum[1] += champion->GetPositionY();
        }
    }

    bool const next = FactionChampionsNextFocus(botAI) == bot->GetGUID();
    bool const melee = meleeOn >= NITRO_MELEE_FOCUS;
    if (on < NITRO_FOCUS && !melee && !next && !contact)
        return false;

    // away from the melee on it, else everything on it, else, about to be picked, the champions round it
    uint32 const from = meleeOn ? meleeOn : (on ? on : nearby);
    std::array<float, 2> const& sum = meleeOn ? meleeSum : (on ? onSum : nearSum);
    if (!from)
        return false;

    Vec const away = Unit2(bot->GetPositionX() - sum[0] / from, bot->GetPositionY() - sum[1] / from);
    float const reach = bot->GetSpeed(MOVE_RUN) * EncounterHelpers::NITRO_BOOSTS_SPEED_FACTOR *
                        EncounterHelpers::NITRO_BOOSTS_DURATION_MS / IN_MILLISECONDS;
    std::vector<EncounterHelpers::HazardCircle> const hazards = FactionChampionsAoeClearances(botAI);
    std::vector<Player*> const healers = HealersOnMap(bot, *state);
    std::vector<Player*> const members = PlayerbotAI::IsHeal(bot) ? std::vector<Player*>() : healers;
    float const nowGap = HealerGap(bot, healers, bot->GetPosition());
    bool found = false;
    float bestClear = 0.0f;
    float bestRun = 0.0f;
    Position best;
    for (float turn : RUN_TURNS)
    {
        Vec const dir = Rotate(away, turn);
        for (float run = reach; run >= NITRO_MIN_RUN; run -= NITRO_STEP_BACK)
        {
            Position const target = Along(bot->GetPosition(), dir, run);
            if (!Clear(target, hazards) || NearHealer(bot, members, target) ||
                !Healed(HealerGap(bot, healers, target), nowGap))
                continue;

            float const clear = MeleeClearance(botAI, target);
            if (!found || clear > bestClear || (clear == bestClear && run > bestRun))
            {
                found = true;
                bestClear = clear;
                bestRun = run;
                best = target;
            }

            break;
        }
    }

    if (!found)
        return false;

    state->runs[bot->GetGUID()] = {now, best};
    spot = best;
    return true;
}

namespace
{
SpellInfo const* KnownReady(Player* bot, uint32 spellId)
{
    return spellId && bot->HasSpell(spellId) ? ReadyInstant(bot, spellId) : nullptr;
}

// Nothing a hit of ours would free, nothing a pull needs, and no fear on the kill target, which would run the
// kill squad around the arena
bool AoeEscapeClear(PlayerbotAI* botAI, Position const& centre, float radius, bool fear)
{
    Unit* kill = fear ? FactionChampionsKillTarget(botAI) : nullptr;
    for (Unit* champion : FactionChampionsInCombat(botAI))
    {
        if (champion->GetExactDist2d(&centre) > radius + ESCAPE_AOE_SLACK)
            continue;

        // a duty still casting or a root peel loses nothing to it, a landed Polymorph or Fear would
        if (champion == kill || FactionChampionsHeldByBreakableCc(champion) ||
            FactionChampionsGripGuarded(botAI, champion))
            return false;
    }

    return true;
}

// Entrapment's root would pop an iced champion's trinket, its Chains going with it, and end a grip in the air
bool FrostTrapClear(PlayerbotAI* botAI, Position const& centre)
{
    for (Unit* champion : FactionChampionsInCombat(botAI))
        if (champion->GetExactDist2d(&centre) <= FROST_TRAP_SLICK + ESCAPE_AOE_SLACK &&
            (FactionChampionsIced(botAI, champion) || FactionChampionsGripGuarded(botAI, champion)))
            return false;

    return true;
}

bool AoeEscape(PlayerbotAI* botAI, OpenerSpell const& spell, Unit* at, bool fear, FactionChampionsEscapeCast& cast)
{
    Player* bot = botAI->GetBot();
    uint32 const rank = KnownOpenerRank(bot, spell);
    SpellInfo const* info = KnownReady(bot, rank);
    if (!info || !AoeEscapeClear(botAI, at->GetPosition(), SpellRadius(info, bot), fear))
        return false;

    cast.spellId = rank;
    cast.target = at;
    return true;
}
}  // namespace

bool FactionChampionsEscape(PlayerbotAI* botAI, FactionChampionsEscapeCast& cast)
{
    if (!FactionChampionsStaysPut(botAI))
        return false;

    Player* bot = botAI->GetBot();
    if (bot->HasUnitState(UNIT_STATE_LOST_CONTROL) || bot->HasUnitFlag(UNIT_FLAG_SILENCED))
        return false;

    std::vector<Unit*> melee;
    float sumX = 0.0f;
    float sumY = 0.0f;
    Unit* nearest = nullptr;
    for (Unit* champion : FactionChampionsInCombat(botAI))
    {
        if (!IsFactionChampionMelee(champion->GetEntry()) || champion->GetVictim() != bot ||
            champion->HasUnitState(UNIT_STATE_LOST_CONTROL) || bot->GetExactDist2d(champion) > ESCAPE_MELEE_REACH)
            continue;

        melee.push_back(champion);
        sumX += champion->GetPositionX();
        sumY += champion->GetPositionY();
        if (!nearest || bot->GetExactDist2d(champion) < bot->GetExactDist2d(nearest))
            nearest = champion;
    }

    if (!nearest)
        return false;

    Position const mean(sumX / melee.size(), sumY / melee.size(), bot->GetPositionZ());
    Vec const away = Unit2(bot->GetPositionX() - mean.GetPositionX(), bot->GetPositionY() - mean.GetPositionY());
    cast = FactionChampionsEscapeCast();
    switch (bot->getClass())
    {
        case CLASS_MAGE:
        {
            // rooted first, so the blink after it leaves them behind
            if (AoeEscape(botAI, FROST_NOVA, bot, false, cast))
                return true;

            if (KnownReady(bot, SPELL_BLINK))
            {
                // off the whole pack, not just the ones on it, or it lands on the far side of the others
                float fleeX = 0.0f;
                float fleeY = 0.0f;
                uint32 flee = 0;
                for (Unit* champion : FactionChampionsInCombat(botAI))
                {
                    if (!IsFactionChampionMelee(champion->GetEntry()) ||
                        bot->GetExactDist2d(champion) > BLINK_FLEE_RADIUS)
                        continue;

                    ++flee;
                    fleeX += champion->GetPositionX();
                    fleeY += champion->GetPositionY();
                }

                Vec const from =
                    flee ? Unit2(bot->GetPositionX() - fleeX / flee, bot->GetPositionY() - fleeY / flee) : away;
                std::vector<EncounterHelpers::HazardCircle> const hazards = FactionChampionsAoeClearances(botAI);
                float bestClear = BLINK_MIN_CLEAR;
                bool found = false;
                Vec best = from;
                for (float turn : {0.0f, 30.0f, -30.0f, 60.0f, -60.0f, 90.0f, -90.0f})
                {
                    Vec const dir = Rotate(from, turn);
                    Position const landing = Along(bot->GetPosition(), dir, BLINK_DIST);
                    if (!Clear(landing, hazards))
                        continue;

                    float const clear = MeleeClearance(botAI, landing);
                    if (clear >= bestClear)
                    {
                        bestClear = clear;
                        best = dir;
                        found = true;
                    }
                }

                if (found)
                {
                    cast.spellId = SPELL_BLINK;
                    cast.target = bot;
                    cast.face = true;
                    cast.facing = Position::NormalizeOrientation(std::atan2(best.y, best.x));
                    return true;
                }
            }

            if (AoeEscape(botAI, DRAGONS_BREATH, bot, true, cast))
            {
                cast.face = true;
                cast.facing = bot->GetAngle(nearest);
                return true;
            }

            return false;
        }
        case CLASS_WARLOCK:
        {
            if (AoeEscape(botAI, SHADOWFURY, nearest, false, cast))
                return true;

            GameObject* circle = bot->GetGameObject(SPELL_DEMONIC_CIRCLE_SUMMON);
            if (circle && bot->HasAura(SPELL_DEMONIC_CIRCLE_IN_RANGE) &&
                KnownReady(bot, SPELL_DEMONIC_CIRCLE_TELEPORT) &&
                circle->GetExactDist2d(&mean) >= bot->GetExactDist2d(&mean) + ESCAPE_MIN_GAIN &&
                Clear(circle->GetPosition(), FactionChampionsAoeClearances(botAI)))
            {
                cast.spellId = SPELL_DEMONIC_CIRCLE_TELEPORT;
                cast.target = bot;
                return true;
            }

            if (AoeEscape(botAI, HOWL_OF_TERROR, bot, true, cast))
                return true;

            auto const known = std::find_if(DEATH_COIL_RANKS.begin(), DEATH_COIL_RANKS.end(),
                                            [bot](uint32 rank) { return bot->HasSpell(rank); });
            uint32 const coil = known != DEATH_COIL_RANKS.end() ? *known : 0;
            if (KnownReady(bot, coil) && nearest != FactionChampionsKillTarget(botAI) &&
                !FactionChampionsGripGuarded(botAI, nearest))
            {
                cast.spellId = coil;
                cast.target = nearest;
                return true;
            }

            return false;
        }
        case CLASS_PRIEST:
            return AoeEscape(botAI, PSYCHIC_SCREAM, bot, true, cast);
        case CLASS_HUNTER:
        {
            // at its feet, so the chaser walks over it on the kite after
            if (KnownReady(bot, SPELL_FROST_TRAP) && FrostTrapClear(botAI, bot->GetPosition()))
            {
                cast.spellId = SPELL_FROST_TRAP;
                cast.target = bot;
                return true;
            }

            if (bot->GetExactDist2d(nearest) > DISENGAGE_REACH || !KnownReady(bot, SPELL_DISENGAGE))
                return false;

            // leaps backwards off its facing, so face the nearest first
            float const facing = bot->GetAngle(nearest);
            Position const landing(bot->GetPositionX() - std::cos(facing) * DISENGAGE_DIST,
                                   bot->GetPositionY() - std::sin(facing) * DISENGAGE_DIST, bot->GetPositionZ());
            if (!Clear(landing, FactionChampionsAoeClearances(botAI)))
                return false;

            cast.spellId = SPELL_DISENGAGE;
            cast.target = bot;
            cast.face = true;
            cast.facing = facing;
            return true;
        }
        default:
            return false;
    }
}

bool FactionChampionsKite(PlayerbotAI* botAI, Position& spot)
{
    BallState* state = Refresh(botAI);
    if (!state)
        return false;

    Player* bot = botAI->GetBot();
    ObjectGuid const guid = bot->GetGUID();
    std::vector<Unit*> const chasers = KiteChasers(botAI, bot);
    auto const kite = state->kites.find(guid);
    if (chasers.empty() || !KiteFree(*state, botAI, bot))
    {
        if (kite != state->kites.end())
            state->kites.erase(kite);

        return false;
    }

    uint32 const now = getMSTime();
    if (kite != state->kites.end())
    {
        if (getMSTimeDiff(kite->second.sinceMs, now) < KITE_STEP_MS &&
            bot->GetExactDist2d(&kite->second.spot) > RUN_ARRIVE)
        {
            spot = kite->second.spot;
            return true;
        }

        state->kites.erase(kite);
    }

    // even in contact a chaser does about half its damage to a moving bot, so no chaser is too close to run from
    float sumX = 0.0f;
    float sumY = 0.0f;
    for (Unit* chaser : chasers)
    {
        sumX += chaser->GetPositionX();
        sumY += chaser->GetPositionY();
    }

    std::vector<Player*> const healers = HealersOnMap(bot, *state);
    std::vector<EncounterHelpers::HazardCircle> const hazards = FactionChampionsAoeClearances(botAI);
    Vec const away = Unit2(bot->GetPositionX() - sumX / chasers.size(), bot->GetPositionY() - sumY / chasers.size());
    // kites to the wall left bots out of heals, so a healed hop always beats a clearer one
    float const nowGap = HealerGap(bot, healers, bot->GetPosition());
    bool found = false;
    bool bestHealed = false;
    float bestClear = 0.0f;
    float bestGap = 0.0f;
    Position best;
    for (float turn : RUN_TURNS)
    {
        Position const target = Along(bot->GetPosition(), Rotate(away, turn), KITE_STEP);
        if (!Clear(target, hazards) || NearHealer(bot, healers, target))
            continue;

        float const gap = HealerGap(bot, healers, target);
        bool const healed = Healed(gap, nowGap);
        float const clear = MeleeClearance(botAI, target);
        bool const better = !found || (healed && !bestHealed) ||
                            (healed == bestHealed && (healed ? clear > bestClear : gap < bestGap));
        if (better)
        {
            found = true;
            bestHealed = healed;
            bestClear = clear;
            bestGap = gap;
            best = target;
        }
    }

    if (!found)
        return false;

    state->kites[guid] = {now, best};
    spot = best;
    return true;
}

bool FactionChampionsKiteHold(PlayerbotAI* botAI)
{
    return Refresh(botAI) && !MeleeOn(botAI, botAI->GetBot(), KITE_HOLD).empty();
}

uint32 FactionChampionsSpeedBoost(PlayerbotAI* botAI)
{
    BallState* state = Refresh(botAI);
    if (!state)
        return 0;

    Player* bot = botAI->GetBot();
    uint32 rank = 0;
    if (bot->getClass() == CLASS_ROGUE)
        rank = KnownOpenerRank(bot, SPRINT);
    else if (bot->getClass() == CLASS_DRUID && bot->GetShapeshiftForm() == FORM_CAT)
        rank = KnownOpenerRank(bot, DASH);

    // same speed aura type as Nitro Boosts, only the fastest one counts
    SpellInfo const* info = rank ? ReadyInstant(bot, rank) : nullptr;
    if (!info ||
        bot->GetMaxPositiveAuraModifier(SPELL_AURA_MOD_INCREASE_SPEED) >= info->Effects[EFFECT_0].CalcValue(bot) ||
        bot->HasUnitState(UNIT_STATE_LOST_CONTROL) || bot->HasAuraType(SPELL_AURA_MOD_ROOT) ||
        state->runs.count(bot->GetGUID()) || FactionChampionsHasCcDuty(botAI, bot->GetGUID()))
        return 0;

    std::vector<Unit*> const chasers = MeleeOn(botAI, bot, BOOST_CONTACT);
    return std::any_of(chasers.begin(), chasers.end(), Unhindered) ? rank : 0;
}

bool FactionChampionsCircleDue(PlayerbotAI* botAI)
{
    Player* bot = botAI ? botAI->GetBot() : nullptr;
    if (!bot || bot->getClass() != CLASS_WARLOCK || !bot->IsAlive() || bot->IsInCombat() ||
        !bot->HasSpell(SPELL_DEMONIC_CIRCLE_SUMMON) || GetToCStagingBoss(botAI) != ToCStagingBoss::Champions ||
        !IsToCStagingHeld(botAI) || IsToCStagingRestHeld(botAI))
        return false;

    Aura const* circle = bot->GetAura(SPELL_DEMONIC_CIRCLE_SUMMON, bot->GetGUID());
    return !circle || (circle->GetDuration() >= 0 && circle->GetDuration() < CIRCLE_REFRESH_MS);
}

}
