/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "ToCHelpers_NorthrendBeasts.h"

#include <algorithm>
#include <array>
#include <list>
#include <vector>

#include "Creature.h"
#include "Group.h"
#include "InstanceScript.h"
#include "Map.h"
#include "ObjectAccessor.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "RaidCombatHelpers.h"
#include "RaidDirectTargeting.h"
#include "RaidInstanceState.h"
#include "Timer.h"
#include "ToCData.h"
#include "ToCEncounterGate.h"
#include "ToCHelpers_Gormok.h"
#include "ToCHelpers_Jormungars.h"

namespace TrialOfTheCrusaderHelpers
{
namespace
{
constexpr size_t BEAST_COUNT = 4;
constexpr size_t DUTY_COUNT = 6;

// Gormok has to come before GormokSwap: pass 1 gives his victim the hold, never the swap.
constexpr std::array<BeastsTankDuty, 5> DEAL_ORDER = {
    BeastsTankDuty::Gormok,     BeastsTankDuty::Icehowl,        BeastsTankDuty::WormMobile,
    BeastsTankDuty::GormokSwap, BeastsTankDuty::WormStationary,
};

// Same reach as the encounter gate's: the arena spans about 80 yd from ARENA_CENTER.
constexpr float ICEHOWL_SEARCH_RADIUS = 200.0f;
// A miss walks the whole grid, and he's absent for the first two stages. He walks in for 10 s
// before he attacks, so a second's delay costs nothing.
constexpr uint32 ICEHOWL_SEARCH_INTERVAL_MS = 1000;

// Health points Dreadscale has to drop under Acidmaw before melee move to Acidmaw, and climb back over
// before they return. With melee on one and ranged on the other the two stay within about 1.5.
constexpr float MELEE_ACIDMAW_MARGIN = 5.0f;

struct BeastsState
{
    uint32 stage{};

    uint32 memoMs = 0;
    bool memoValid = false;  // 0 is a real getMSTime value
    uint32 icehowlSearchMs = 0;
    bool icehowlSearched = false;

    std::array<ObjectGuid, BEAST_COUNT> beasts;
    // Engaged beast per duty this ms, empty while it isn't up
    std::array<ObjectGuid, DUTY_COUNT> dutyBeast;
    std::array<ObjectGuid, DUTY_COUNT> holder;
    // One switch for the raid, so melee move together
    bool meleeOnAcidmaw = false;
};

RaidInstanceState<BeastsState> beastsStates;

size_t Index(NorthrendBeast beast) { return static_cast<size_t>(beast); }

size_t Index(BeastsTankDuty duty) { return static_cast<size_t>(duty); }

bool IsEngaged(Creature const* creature) { return creature && creature->IsAlive() && creature->IsInCombat(); }

Creature* Resolve(Map* map, ObjectGuid const& guid) { return guid.IsEmpty() ? nullptr : map->GetCreature(guid); }

// A guid resolves map-wide, hence the combat test.
Creature* ResolveEngaged(Map* map, ObjectGuid const& guid)
{
    Creature* creature = Resolve(map, guid);
    return IsEngaged(creature) ? creature : nullptr;
}

ObjectGuid GuidOf(Creature const* creature) { return creature ? creature->GetGUID() : ObjectGuid::Empty; }

// Icehowl has no guid slot in the script. A cached guid is kept while it resolves, corpse included;
// a wipe despawns him and the next attempt summons a new one.
void RefreshIcehowl(Player* bot, Map* map, BeastsState& state, uint32 now)
{
    ObjectGuid& guid = state.beasts[Index(NorthrendBeast::Icehowl)];
    if (Resolve(map, guid))
        return;

    if (state.icehowlSearched && getMSTimeDiff(state.icehowlSearchMs, now) < ICEHOWL_SEARCH_INTERVAL_MS)
        return;

    state.icehowlSearched = true;
    state.icehowlSearchMs = now;
    guid = ObjectGuid::Empty;

    std::list<Creature*> found;
    bot->GetCreatureListWithEntryInGrid(found, static_cast<uint32>(ToCNpcs::NPC_ICEHOWL), ICEHOWL_SEARCH_RADIUS);
    for (Creature* creature : found)
    {
        if (creature->IsAlive())
        {
            guid = creature->GetGUID();
            return;
        }
    }
}

// Neither in mobile form leaves Acidmaw on WormMobile. A lone survivor is WormMobile whatever its
// form, so one holder keeps it through every form change.
void SplitWorms(Creature* acidmaw, Creature* dreadscale, Creature*& mobile, Creature*& stationary)
{
    if (acidmaw && dreadscale)
    {
        bool const dreadscaleMobile = IsWormMobile(dreadscale) && !IsWormMobile(acidmaw);
        mobile = dreadscaleMobile ? dreadscale : acidmaw;
        stationary = dreadscaleMobile ? acidmaw : dreadscale;
        return;
    }

    mobile = acidmaw ? acidmaw : dreadscale;
    stationary = nullptr;
}

bool CountsForRoster(Player* member, uint32 instanceId)
{
    return member && member->IsInWorld() && member->IsAlive() &&
           member->GetMapId() == TRIAL_OF_THE_CRUSADER_MAP_ID && member->GetInstanceId() == instanceId &&
           GET_PLAYERBOT_AI(member) && IsBeastsTank(member);
}

// Bots only: a human can't be handed a duty, so he only gets one in pass 1 for what he's tanking.
// Flagged main tank first, then by guid, which every bot sorts the same way.
std::vector<Player*> GatherRoster(Player* bot)
{
    std::vector<Player*> roster;
    uint32 const instanceId = bot->GetInstanceId();

    Group* group = bot->GetGroup();
    if (!group)
    {
        if (CountsForRoster(bot, instanceId))
            roster.push_back(bot);

        return roster;
    }

    ObjectGuid flagged;
    for (Group::MemberSlot const& slot : group->GetMemberSlots())
    {
        if (slot.flags & MEMBER_FLAG_MAINTANK)
        {
            flagged = slot.guid;
            break;
        }
    }

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (CountsForRoster(member, instanceId))
            roster.push_back(member);
    }

    std::sort(roster.begin(), roster.end(),
              [&flagged](Player* lhs, Player* rhs)
              {
                  bool const lhsFlagged = lhs->GetGUID() == flagged;
                  bool const rhsFlagged = rhs->GetGUID() == flagged;
                  if (lhsFlagged != rhsFlagged)
                      return lhsFlagged;

                  return lhs->GetGUID() < rhs->GetGUID();
              });

    return roster;
}

bool HoldsDuty(BeastsState const& state, ObjectGuid const& guid)
{
    return std::find(state.holder.begin(), state.holder.end(), guid) != state.holder.end();
}

void Deal(Player* bot, BeastsState& state, std::array<Creature*, DUTY_COUNT> const& dutyCreature)
{
    state.holder.fill(ObjectGuid::Empty);
    if (std::none_of(dutyCreature.begin(), dutyCreature.end(), [](Creature* beast) { return beast != nullptr; }))
        return;

    // Pass 1: a duty stays with the tank its beast is on, so a swap or a worm changing form only
    // moves the tanks that have to move.
    for (BeastsTankDuty duty : DEAL_ORDER)
    {
        Creature* beast = dutyCreature[Index(duty)];
        Unit* victim = beast ? beast->GetVictim() : nullptr;
        Player* tank = victim ? victim->ToPlayer() : nullptr;
        if (tank && IsBeastsTank(tank) && !HoldsDuty(state, tank->GetGUID()))
            state.holder[Index(duty)] = tank->GetGUID();
    }

    std::vector<Player*> roster;
    bool rosterBuilt = false;
    auto const inRoster = [&roster](ObjectGuid const& guid)
    { return std::any_of(roster.begin(), roster.end(), [&guid](Player* tank) { return tank->GetGUID() == guid; }); };

    // Pass 2: the rest in priority order. Out of free tanks, a duty takes the one on the
    // lowest-priority duty below it, but never a human's.
    for (size_t i = 0; i < DEAL_ORDER.size(); ++i)
    {
        size_t const duty = Index(DEAL_ORDER[i]);
        if (!dutyCreature[duty] || !state.holder[duty].IsEmpty())
            continue;

        if (!rosterBuilt)
        {
            roster = GatherRoster(bot);
            rosterBuilt = true;
        }

        auto const free = std::find_if(roster.begin(), roster.end(),
                                       [&state](Player* tank) { return !HoldsDuty(state, tank->GetGUID()); });
        if (free != roster.end())
        {
            state.holder[duty] = (*free)->GetGUID();
            continue;
        }

        for (size_t j = DEAL_ORDER.size() - 1; j > i; --j)
        {
            size_t const lower = Index(DEAL_ORDER[j]);
            if (!state.holder[lower].IsEmpty() && inRoster(state.holder[lower]))
            {
                state.holder[duty] = state.holder[lower];
                state.holder[lower] = ObjectGuid::Empty;
                break;
            }
        }
    }
}

// Null off a ToC instance. Every trigger and multiplier on every bot asks, so one read per instance
// per ms.
BeastsState* ReadBeasts(PlayerbotAI* botAI, Map*& map)
{
    Player* bot = botAI ? botAI->GetBot() : nullptr;
    map = bot ? bot->FindMap() : nullptr;
    if (!map || map->GetId() != TRIAL_OF_THE_CRUSADER_MAP_ID || !bot->GetInstanceId())
        return nullptr;

    InstanceScript* instance = bot->GetInstanceScript();
    if (!instance)
        return nullptr;

    BeastsState& state = beastsStates.For(bot->GetInstanceId());
    uint32 const now = getMSTime();
    if (state.memoValid && state.memoMs == now)
        return &state;

    state.memoMs = now;
    state.memoValid = true;

    state.beasts[Index(NorthrendBeast::Gormok)] = instance->GetGuidData(TOC_DATA_GORMOK);
    state.beasts[Index(NorthrendBeast::Acidmaw)] = instance->GetGuidData(TOC_DATA_ACIDMAW);
    state.beasts[Index(NorthrendBeast::Dreadscale)] = instance->GetGuidData(TOC_DATA_DREADSCALE);
    RefreshIcehowl(bot, map, state, now);

    Creature* gormok = ResolveEngaged(map, state.beasts[Index(NorthrendBeast::Gormok)]);
    Creature* acidmaw = ResolveEngaged(map, state.beasts[Index(NorthrendBeast::Acidmaw)]);
    Creature* dreadscale = ResolveEngaged(map, state.beasts[Index(NorthrendBeast::Dreadscale)]);
    Creature* icehowl = ResolveEngaged(map, state.beasts[Index(NorthrendBeast::Icehowl)]);

    uint32 mask = 0;
    if (gormok)
        mask |= BEASTS_STAGE_GORMOK;
    if (acidmaw || dreadscale)
        mask |= BEASTS_STAGE_WORMS;
    if (icehowl)
        mask |= BEASTS_STAGE_ICEHOWL;
    state.stage = mask;

    if (!acidmaw || !dreadscale)
        state.meleeOnAcidmaw = false;
    else
    {
        float const gap = dreadscale->GetHealthPct() - acidmaw->GetHealthPct();
        if (gap <= -MELEE_ACIDMAW_MARGIN)
            state.meleeOnAcidmaw = true;
        else if (gap >= MELEE_ACIDMAW_MARGIN)
            state.meleeOnAcidmaw = false;
    }

    Creature* mobile = nullptr;
    Creature* stationary = nullptr;
    SplitWorms(acidmaw, dreadscale, mobile, stationary);

    std::array<Creature*, DUTY_COUNT> dutyCreature{};
    dutyCreature[Index(BeastsTankDuty::Gormok)] = gormok;
    dutyCreature[Index(BeastsTankDuty::GormokSwap)] = gormok;
    dutyCreature[Index(BeastsTankDuty::Icehowl)] = icehowl;
    dutyCreature[Index(BeastsTankDuty::WormMobile)] = mobile;
    dutyCreature[Index(BeastsTankDuty::WormStationary)] = stationary;

    for (size_t duty = 0; duty < DUTY_COUNT; ++duty)
        state.dutyBeast[duty] = GuidOf(dutyCreature[duty]);

    Deal(bot, state, dutyCreature);
    return &state;
}

bool IsBeastsSkullTarget(Unit* unit)
{
    switch (static_cast<ToCNpcs>(unit->GetEntry()))
    {
        case ToCNpcs::NPC_GORMOK:
        case ToCNpcs::NPC_ACIDMAW:
        case ToCNpcs::NPC_DREADSCALE:
        case ToCNpcs::NPC_ICEHOWL:
        case ToCNpcs::NPC_SNOBOLD_VASSAL:
            return true;
        default:
            return false;
    }
}

Unit* ResolveDpsPick(PlayerbotAI* botAI)
{
    if (Unit* skull = GetRaidHumanSkull(botAI, &IsBeastsSkullTarget))
        return skull;

    if (Unit* snobold = GetGormokSnoboldPick(botAI))
        return snobold;

    if (Unit* gormok = GetEngagedBeast(botAI, NorthrendBeast::Gormok))
        return gormok;

    // Acidmaw first: alone, its Toxin has no Bile left to cure it. Melee stay on the mobile worm, out of
    // the stationary one's Sweep, unless Dreadscale falls well under Acidmaw.
    Unit* worm = GetBeastOfDuty(botAI, BeastsTankDuty::WormMobile);
    Unit* acidmaw = GetEngagedBeast(botAI, NorthrendBeast::Acidmaw);
    if (acidmaw && !IsWormSubmerged(acidmaw))
    {
        Map* map = nullptr;
        BeastsState const* state = ReadBeasts(botAI, map);
        bool const dreadscaleLower =
            !GetEngagedBeast(botAI, NorthrendBeast::Dreadscale) || (state && state->meleeOnAcidmaw);
        if (PlayerbotAI::IsRanged(botAI->GetBot()) || acidmaw == worm || dreadscaleLower)
            return acidmaw;
    }

    if (worm && !IsWormSubmerged(worm))
        return worm;

    return GetEngagedBeast(botAI, NorthrendBeast::Icehowl);
}
}

Unit* GetEngagedBeast(PlayerbotAI* botAI, NorthrendBeast beast)
{
    Map* map = nullptr;
    BeastsState* state = ReadBeasts(botAI, map);
    return state ? ResolveEngaged(map, state->beasts[Index(beast)]) : nullptr;
}

uint32 GetBeastsStageMask(PlayerbotAI* botAI)
{
    Map* map = nullptr;
    BeastsState* state = ReadBeasts(botAI, map);
    return state ? state->stage : 0;
}

bool IsBeastsTank(Player* player)
{
    return player && (PlayerbotAI::IsTank(player) || PlayerbotAI::IsTank(player, true));
}

std::vector<Player*> GetBeastsTankRoster(Player* bot) { return bot ? GatherRoster(bot) : std::vector<Player*>(); }

BeastsTankDuty GetBeastsTankDuty(PlayerbotAI* botAI)
{
    Map* map = nullptr;
    BeastsState* state = ReadBeasts(botAI, map);
    if (!state)
        return BeastsTankDuty::None;

    Player* bot = botAI->GetBot();
    BeastsTankDuty duty = BeastsTankDuty::None;
    for (BeastsTankDuty candidate : DEAL_ORDER)
    {
        if (state->holder[Index(candidate)] == bot->GetGUID())
        {
            duty = candidate;
            break;
        }
    }

    return duty;
}

Player* GetBeastsDutyHolder(PlayerbotAI* botAI, BeastsTankDuty duty)
{
    if (duty == BeastsTankDuty::None)
        return nullptr;

    Map* map = nullptr;
    BeastsState* state = ReadBeasts(botAI, map);
    if (!state)
        return nullptr;

    ObjectGuid const& guid = state->holder[Index(duty)];
    return guid.IsEmpty() ? nullptr : ObjectAccessor::GetPlayer(map, guid);
}

Unit* GetBeastOfDuty(PlayerbotAI* botAI, BeastsTankDuty duty)
{
    if (duty == BeastsTankDuty::None)
        return nullptr;

    Map* map = nullptr;
    BeastsState* state = ReadBeasts(botAI, map);
    return state ? ResolveEngaged(map, state->dutyBeast[Index(duty)]) : nullptr;
}

BeastsTankDuty GetDutyOfBeast(PlayerbotAI* botAI, Unit* beast)
{
    if (!beast)
        return BeastsTankDuty::None;

    Map* map = nullptr;
    BeastsState* state = ReadBeasts(botAI, map);
    if (!state)
        return BeastsTankDuty::None;

    ObjectGuid const guid = beast->GetGUID();
    for (BeastsTankDuty duty : DEAL_ORDER)
    {
        if (duty != BeastsTankDuty::GormokSwap && state->dutyBeast[Index(duty)] == guid)
            return duty;
    }

    return BeastsTankDuty::None;
}

Unit* GetBeastsDpsTarget(PlayerbotAI* botAI)
{
    Player* bot = botAI ? botAI->GetBot() : nullptr;
    if (!bot || bot->GetMapId() != TRIAL_OF_THE_CRUSADER_MAP_ID || !IsRaidDpsBot(bot))
        return nullptr;

    return ResolveDpsPick(botAI);
}

}
