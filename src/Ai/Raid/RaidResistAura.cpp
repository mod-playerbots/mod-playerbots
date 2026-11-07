/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "RaidResistAura.h"

#include <array>
#include <tuple>
#include <vector>

#include "BossAuraTriggers.h"
#include "GenericBuffUtils.h"
#include "Group.h"
#include "Map.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "RaidInstanceState.h"
#include "Timer.h"

namespace
{
constexpr size_t PALADIN_SCHOOL_COUNT = 3;

// Election order: with one free paladin, fire wins
constexpr std::array<RaidResistSchool, PALADIN_SCHOOL_COUNT> PALADIN_SCHOOLS = {
    RaidResistSchool::Fire, RaidResistSchool::Frost, RaidResistSchool::Shadow};

constexpr uint32 ELECT_INTERVAL_MS = 1000;

constexpr uint8 PALADIN_MASK = RAID_RESIST_FIRE | RAID_RESIST_FROST | RAID_RESIST_SHADOW;

constexpr size_t SchoolIndex(RaidResistSchool school) { return static_cast<size_t>(school) - 1; }

// GetAuraOfRankedSpell takes any rank of the chain
uint32 RankOneAura(RaidResistSchool school)
{
    switch (school)
    {
        case RaidResistSchool::Fire:
            return SPELL_FIRE_RESISTANCE_AURA_RANK_1;
        case RaidResistSchool::Frost:
            return SPELL_FROST_RESISTANCE_AURA_RANK_1;
        case RaidResistSchool::Shadow:
            return SPELL_SHADOW_RESISTANCE_AURA_RANK_1;
        default:
            return 0;
    }
}

// The paladin's own aura strategy. They're siblings, so a bot runs one at most.
enum class AuraStrategy : uint8
{
    None,
    Fire,
    Frost,
    Shadow,
    Speed,
    Aoe,
    Cast,
    Armor
};

struct AuraStrategyName
{
    char const* name;
    AuraStrategy strategy;
};

constexpr AuraStrategyName AURA_STRATEGIES[] = {
    {"rfire", AuraStrategy::Fire}, {"rfrost", AuraStrategy::Frost}, {"rshadow", AuraStrategy::Shadow},
    {"bspeed", AuraStrategy::Speed}, {"baoe", AuraStrategy::Aoe},    {"bcast", AuraStrategy::Cast},
    {"barmor", AuraStrategy::Armor},
};

AuraStrategy AuraStrategyOf(PlayerbotAI* memberAI)
{
    for (AuraStrategyName const& entry : AURA_STRATEGIES)
        if (memberAI->HasStrategy(entry.name, BOT_STATE_COMBAT))
            return entry.strategy;

    return AuraStrategy::None;
}

AuraStrategy ResistStrategyOf(RaidResistSchool school)
{
    switch (school)
    {
        case RaidResistSchool::Fire:
            return AuraStrategy::Fire;
        case RaidResistSchool::Frost:
            return AuraStrategy::Frost;
        case RaidResistSchool::Shadow:
            return AuraStrategy::Shadow;
        default:
            return AuraStrategy::None;
    }
}

// What the raid loses when this paladin swaps auras. Sanctified and Swift Retribution work under any
// of its auras, Improved Devotion only under Devotion.
uint8 AuraCost(AuraStrategy strategy)
{
    switch (strategy)
    {
        case AuraStrategy::Fire:
        case AuraStrategy::Frost:
        case AuraStrategy::Shadow:
        case AuraStrategy::Speed:
            return 0;
        case AuraStrategy::Cast:
            return 2;
        case AuraStrategy::Armor:
            return 3;
        default:
            return 1;
    }
}

struct Paladin
{
    Player* player = nullptr;
    AuraStrategy strategy = AuraStrategy::None;
    std::array<bool, PALADIN_SCHOOL_COUNT> knows{};
    bool mastery = false;
};

struct ResistAuraState
{
    uint32 memoMs = 0;
    bool memoValid = false;  // 0 is a real getMSTime value
    uint8 wanted = 0;
    bool fireWithMastery = false;

    bool elected = false;
    uint32 electMs = 0;
    uint8 electedFor = 0;
    bool electedWithMastery = false;
    std::array<ObjectGuid, PALADIN_SCHOOL_COUNT> holder;
};

RaidInstanceState<ResistAuraState> resistStates;

// Bot paladins only: a human can't be handed the aura. A human already running one on himself covers
// the raid for that school, so it lands in humanHeld instead.
std::vector<Paladin> GatherPaladins(Player* bot, uint32 mapId, uint8& humanHeld)
{
    std::vector<Paladin> paladins;
    uint32 const instanceId = bot->GetInstanceId();

    auto const consider = [&](Player* member)
    {
        if (!member || !member->IsInWorld() || !member->IsAlive() || member->getClass() != CLASS_PALADIN ||
            member->GetMapId() != mapId || member->GetInstanceId() != instanceId)
            return;

        PlayerbotAI* memberAI = GET_PLAYERBOT_AI(member);
        if (!memberAI)
        {
            for (RaidResistSchool school : PALADIN_SCHOOLS)
                if (member->GetAuraOfRankedSpell(RankOneAura(school), member->GetGUID()))
                    humanHeld |= RaidResistBit(school);

            return;
        }

        Paladin paladin;
        paladin.player = member;
        paladin.strategy = AuraStrategyOf(memberAI);
        for (RaidResistSchool school : PALADIN_SCHOOLS)
            paladin.knows[SchoolIndex(school)] =
                memberAI->GetAiObjectContext()->GetValue<uint32>("spell id", RaidResistAuraName(school))->Get() != 0;
        paladin.mastery = memberAI->GetAiObjectContext()->GetValue<uint32>("spell id", "aura mastery")->Get() != 0;

        paladins.push_back(paladin);
    };

    Group* group = bot->GetGroup();
    if (!group)
    {
        consider(bot);
        return paladins;
    }

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        consider(ref->GetSource());

    return paladins;
}

// Another paladin left on the same aura keeps it up for the raid. Only those not already holding a
// school this round, since a holder gives its own aura up.
bool AuraDuplicated(std::vector<Paladin> const& paladins, std::vector<bool> const& used, size_t self)
{
    if (paladins[self].strategy == AuraStrategy::None)
        return false;

    for (size_t i = 0; i < paladins.size(); ++i)
        if (i != self && !used[i] && paladins[i].strategy == paladins[self].strategy)
            return true;

    return false;
}

void Elect(Player* bot, uint32 mapId, ResistAuraState& state, uint32 now)
{
    state.elected = true;
    state.electMs = now;
    state.electedFor = state.wanted;
    state.electedWithMastery = state.fireWithMastery;

    std::array<ObjectGuid, PALADIN_SCHOOL_COUNT> next{};
    if (!(state.wanted & PALADIN_MASK))
    {
        state.holder = next;
        return;
    }

    uint8 humanHeld = 0;
    std::vector<Paladin> const paladins = GatherPaladins(bot, mapId, humanHeld);
    std::vector<bool> used(paladins.size(), false);

    auto const open = [&](RaidResistSchool school)
    {
        return (state.wanted & RaidResistBit(school)) && !(humanHeld & RaidResistBit(school));
    };

    auto const wantsMastery = [&](RaidResistSchool school)
    { return state.fireWithMastery && school == RaidResistSchool::Fire; };
    // a kept fire holder without Aura Mastery gives way to a free paladin with it
    auto const masteryFree = [&](size_t keeper)
    {
        for (size_t i = 0; i < paladins.size(); ++i)
            if (i != keeper && paladins[i].mastery && paladins[i].knows[SchoolIndex(RaidResistSchool::Fire)])
                return true;

        return false;
    };

    // Kept holders first, so a dead fire holder can't pull the frost holder over to fire
    for (RaidResistSchool school : PALADIN_SCHOOLS)
    {
        size_t const index = SchoolIndex(school);
        if (!open(school) || state.holder[index].IsEmpty())
            continue;

        for (size_t i = 0; i < paladins.size(); ++i)
        {
            if (!used[i] && paladins[i].knows[index] && paladins[i].player->GetGUID() == state.holder[index] &&
                (!wantsMastery(school) || paladins[i].mastery || !masteryFree(i)))
            {
                next[index] = state.holder[index];
                used[i] = true;
                break;
            }
        }
    }

    for (RaidResistSchool school : PALADIN_SCHOOLS)
    {
        size_t const index = SchoolIndex(school);
        if (!open(school) || !next[index].IsEmpty())
            continue;

        auto const rank = [&](size_t i)
        {
            Paladin const& paladin = paladins[i];
            return std::make_tuple(wantsMastery(school) && !paladin.mastery ? 1 : 0,
                                   paladin.strategy == ResistStrategyOf(school) ? 0 : 1,
                                   AuraDuplicated(paladins, used, i) ? 0 : 1, AuraCost(paladin.strategy),
                                   paladin.player->GetGUID());
        };

        size_t best = paladins.size();
        for (size_t i = 0; i < paladins.size(); ++i)
        {
            if (used[i] || !paladins[i].knows[index])
                continue;

            if (best == paladins.size() || rank(i) < rank(best))
                best = i;
        }

        if (best == paladins.size())
            continue;

        next[index] = paladins[best].player->GetGUID();
        used[best] = true;
    }

    state.holder = next;
}

// One read per instance per ms: every paladin and hunter asks each tick, and the rules ask per action.
ResistAuraState const* ReadResist(PlayerbotAI* botAI, RaidResistAuraConfig const& config)
{
    Player* bot = botAI ? botAI->GetBot() : nullptr;
    Map* map = bot ? bot->FindMap() : nullptr;
    if (!map || map->GetId() != config.mapId || !bot->GetInstanceId())
        return nullptr;

    ResistAuraState& state = resistStates.For(bot->GetInstanceId());
    uint32 const now = getMSTime();
    if (state.memoValid && state.memoMs == now)
        return &state;

    state.memoMs = now;
    state.memoValid = true;
    state.wanted = config.wantedSchools(botAI, state.wanted);
    state.fireWithMastery =
        (state.wanted & RAID_RESIST_FIRE) && config.fireWantsMastery && config.fireWantsMastery(botAI);

    if (!state.elected || state.wanted != state.electedFor || state.fireWithMastery != state.electedWithMastery ||
        getMSTimeDiff(state.electMs, now) >= ELECT_INTERVAL_MS)
        Elect(bot, config.mapId, state, now);

    return &state;
}

}  // namespace

RaidResistSchool GetRaidResistDuty(PlayerbotAI* botAI, RaidResistAuraConfig const& config)
{
    Player* bot = botAI ? botAI->GetBot() : nullptr;
    if (!bot || (bot->getClass() != CLASS_PALADIN && bot->getClass() != CLASS_HUNTER))
        return RaidResistSchool::None;

    ResistAuraState const* state = ReadResist(botAI, config);
    if (!state)
        return RaidResistSchool::None;

    if (bot->getClass() == CLASS_PALADIN)
    {
        for (RaidResistSchool school : PALADIN_SCHOOLS)
            if (state->holder[SchoolIndex(school)] == bot->GetGUID())
                return school;

        return RaidResistSchool::None;
    }

    // The generic pick hands the role on by mana, through Viper
    if ((state->wanted & RAID_RESIST_NATURE) && ai::buff::GetNatureResistanceHunter(botAI, bot) == bot)
        return RaidResistSchool::Nature;

    return RaidResistSchool::None;
}

char const* RaidResistAuraName(RaidResistSchool school)
{
    switch (school)
    {
        case RaidResistSchool::Fire:
            return "fire resistance aura";
        case RaidResistSchool::Frost:
            return "frost resistance aura";
        case RaidResistSchool::Shadow:
            return "shadow resistance aura";
        case RaidResistSchool::Nature:
            return "aspect of the wild";
        default:
            return nullptr;
    }
}

bool RaidResistAuraMissing(PlayerbotAI* botAI, RaidResistAuraConfig const& config)
{
    RaidResistSchool const duty = GetRaidResistDuty(botAI, config);
    if (duty == RaidResistSchool::None)
        return false;

    Player* bot = botAI->GetBot();
    if (!bot->IsAlive())
        return false;

    if (duty == RaidResistSchool::Nature)
        return !botAI->HasAura("aspect of the wild", bot);

    return !bot->GetAuraOfRankedSpell(RankOneAura(duty), bot->GetGUID());
}

bool IsRaidResistHunterHolding(PlayerbotAI* botAI, RaidResistAuraConfig const& config)
{
    return GetRaidResistDuty(botAI, config) == RaidResistSchool::Nature;
}

bool RaidResistAuraTrigger::IsActive() { return RaidResistAuraMissing(botAI, _config); }

bool RaidRaiseResistAuraAction::Execute(Event /*event*/)
{
    char const* aura = RaidResistAuraName(GetRaidResistDuty(botAI, _config));
    return aura && botAI->DoSpecificAction(aura, Event(), true);
}

// A queued basket can pop after the duty moved on
bool RaidRaiseResistAuraAction::isUseful() { return RaidResistAuraMissing(botAI, _config); }
