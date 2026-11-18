/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "ToCHelpers_Jaraxxus.h"

#include <algorithm>
#include <cmath>
#include <unordered_map>

#include "Creature.h"
#include "EncounterHelpers.h"
#include "Group.h"
#include "Map.h"
#include "ObjectAccessor.h"
#include "Player.h"
#include "Playerbots.h"
#include "RaidCombatHelpers.h"
#include "RaidDirectTargeting.h"
#include "RaidInstanceState.h"
#include "Spell.h"
#include "SpellAuraEffects.h"
#include "SpellAuras.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include "TemporarySummon.h"
#include "Timer.h"
#include "ToCEncounterGate.h"
#include "ToCHelpers_Shared.h"
#include "Unit.h"

using namespace EncounterHelpers;

namespace TrialOfTheCrusaderHelpers
{

namespace
{

// The arena floor reaches ~80 yd from ARENA_CENTER. A bot further out than scan radius minus that
// would miss part of the floor, so it reads the last scan instead of refreshing it.
constexpr float JARAXXUS_ARENA_REACH = 80.0f;
constexpr float JARAXXUS_FLAME_CARRIER_RAID_SCAN = 40.0f;
constexpr float JARAXXUS_MELEE_INTERRUPT_RANGE = 5.0f;
// spell damage per Nether Power stack
constexpr float JARAXXUS_NETHER_POWER_PER_STACK = 0.2f;
// FindNearestPositionClearOfHazards's own step defaults, spelled out where a later argument is passed
constexpr float SWEEP_RING_STEP = 2.0f;
constexpr float SWEEP_ANGLE_STEP = static_cast<float>(M_PI) / 8.0f;

std::vector<uint32> const JARAXXUS_SCAN_ENTRIES = {
    static_cast<uint32>(ToCNpcs::NPC_JARAXXUS),        static_cast<uint32>(ToCNpcs::NPC_NETHER_PORTAL),
    static_cast<uint32>(ToCNpcs::NPC_INFERNAL_VOLCANO), static_cast<uint32>(ToCNpcs::NPC_MISTRESS_OF_PAIN),
    static_cast<uint32>(ToCNpcs::NPC_FEL_INFERNAL),     static_cast<uint32>(ToCNpcs::NPC_LEGION_FLAME),
};

// Guids, never pointers: a unit can despawn between two refreshes.
struct JaraxxusScan
{
    uint32 stampMs = 0;
    bool valid = false;  // 0 is a real getMSTime value
    std::vector<ObjectGuid> bosses;
    std::vector<ObjectGuid> portals;
    std::vector<ObjectGuid> volcanoes;
    std::vector<ObjectGuid> mistresses;
    std::vector<ObjectGuid> infernals;
    std::vector<Position> flames;
};

struct ThreatCapLatch
{
    bool capped = false;
    bool memoValid = false;
    uint32 memoMs = 0;
    bool value = false;
};

enum class InfernoPhase : uint8
{
    None,
    Coming,
    Running
};

// One read per instance per ms: every bot's cover and personal nodes ask each tick
struct InfernoMemo
{
    bool valid = false;
    uint32 ms = 0;
    InfernoPhase phase = InfernoPhase::None;
    RaidPredictedHit hit;
};

struct JaraxxusState
{
    JaraxxusScan scan;
    InfernoMemo inferno;
    // Group walks behind triggers every bot asks each tick
    ToCGroupMemo<std::vector<ObjectGuid>> pinnedCasters;
    ToCGroupMemo<ObjectGuid> incinerateTarget;
    ToCGroupMemo<bool> nonHealingRemover;
    ToCGroupMemo<ObjectGuid> fireballInterrupter;
    ToCGroupMemo<bool> assistTankOne;
    std::unordered_map<ObjectGuid, ThreatCapLatch> threatCaps;
};

RaidInstanceState<JaraxxusState> jaraxxusStates;

JaraxxusState* StateFor(Player* bot)
{
    if (!bot || !bot->IsInWorld() || bot->GetMapId() != TRIAL_OF_THE_CRUSADER_MAP_ID || !bot->GetInstanceId())
        return nullptr;

    return &jaraxxusStates.For(bot->GetInstanceId());
}

JaraxxusScan const& ScanOf(Player* bot, JaraxxusState& state)
{
    JaraxxusScan& scan = state.scan;
    uint32 const now = getMSTime();
    if (scan.valid && getMSTimeDiff(scan.stampMs, now) < JARAXXUS_SCAN_MS)
        return scan;

    if (bot->GetExactDist(ARENA_CENTER) > JARAXXUS_SCAN_RADIUS - JARAXXUS_ARENA_REACH)
        return scan;

    scan.stampMs = now;
    scan.valid = true;
    scan.bosses.clear();
    scan.portals.clear();
    scan.volcanoes.clear();
    scan.mistresses.clear();
    scan.infernals.clear();
    scan.flames.clear();

    static ToCEntrySet const scanSet(JARAXXUS_SCAN_ENTRIES);
    std::vector<Creature*> creatures;
    GetCreatureListWithEntriesInGrid(bot, creatures, scanSet, JARAXXUS_SCAN_RADIUS);
    for (Creature* creature : creatures)
    {
        if (!creature || !creature->IsAlive())
            continue;

        switch (static_cast<ToCNpcs>(creature->GetEntry()))
        {
            case ToCNpcs::NPC_JARAXXUS:
                scan.bosses.push_back(creature->GetGUID());
                break;
            case ToCNpcs::NPC_NETHER_PORTAL:
                scan.portals.push_back(creature->GetGUID());
                break;
            case ToCNpcs::NPC_INFERNAL_VOLCANO:
                scan.volcanoes.push_back(creature->GetGUID());
                break;
            case ToCNpcs::NPC_MISTRESS_OF_PAIN:
                scan.mistresses.push_back(creature->GetGUID());
                break;
            case ToCNpcs::NPC_FEL_INFERNAL:
                scan.infernals.push_back(creature->GetGUID());
                break;
            case ToCNpcs::NPC_LEGION_FLAME:
                scan.flames.push_back(creature->GetPosition());
                break;
            default:
                break;
        }
    }

    for (std::vector<ObjectGuid>* guids :
         {&scan.bosses, &scan.portals, &scan.volcanoes, &scan.mistresses, &scan.infernals})
        std::sort(guids->begin(), guids->end());

    return scan;
}

JaraxxusScan const* ScanFor(Player* bot)
{
    JaraxxusState* state = StateFor(bot);
    return state ? &ScanOf(bot, *state) : nullptr;
}

Creature* ResolveAlive(Player* bot, ObjectGuid guid)
{
    Map* map = bot->FindMap();
    Creature* creature = map ? map->GetCreature(guid) : nullptr;
    return creature && creature->IsAlive() ? creature : nullptr;
}

Creature* FirstAlive(Player* bot, std::vector<ObjectGuid> const& guids)
{
    for (ObjectGuid guid : guids)
        if (Creature* creature = ResolveAlive(bot, guid))
            return creature;

    return nullptr;
}

// Normal portals and volcanoes are NOT_SELECTABLE and gone after 15 s, heroic ones have to be killed.
Creature* FirstSelectable(Player* bot, std::vector<ObjectGuid> const& guids)
{
    for (ObjectGuid guid : guids)
    {
        Creature* creature = ResolveAlive(bot, guid);
        if (creature && !creature->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE))
            return creature;
    }

    return nullptr;
}

Creature* SecondAlive(Player* bot, std::vector<ObjectGuid> const& guids)
{
    bool skipped = false;
    for (ObjectGuid guid : guids)
    {
        Creature* creature = ResolveAlive(bot, guid);
        if (!creature)
            continue;

        if (skipped)
            return creature;

        skipped = true;
    }

    return nullptr;
}

Unit* FindJaraxxus(Player* bot)
{
    JaraxxusScan const* scan = ScanFor(bot);
    if (!scan)
        return nullptr;

    for (ObjectGuid guid : scan->bosses)
    {
        Creature* boss = ResolveAlive(bot, guid);
        if (boss && !boss->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE))
            return boss;
    }

    return nullptr;
}

std::vector<Player*> AliveMembersOnMap(Player* bot)
{
    std::vector<Player*> members;
    Group* group = bot->GetGroup();
    if (!group)
    {
        if (bot->IsAlive())
            members.push_back(bot);

        return members;
    }

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive() && member->IsInMap(bot))
            members.push_back(member);
    }

    return members;
}

// Effect 0 of Incinerate Flesh is the heal absorb, worn down by every heal it eats
uint32 IncinerateAbsorbLeft(Unit* unit)
{
    Aura* aura = unit ? unit->GetAura(sSpellMgr->GetSpellIdForDifficulty(SPELL_INCINERATE_FLESH, unit)) : nullptr;
    AuraEffect* absorb = aura ? aura->GetEffect(EFFECT_0) : nullptr;
    if (!absorb || absorb->GetAuraType() != SPELL_AURA_SCHOOL_HEAL_ABSORB)
        return 0;

    return static_cast<uint32>(std::max(absorb->GetAmount(), 0));
}

Unit* IncinerateFleshTargetFor(Player* bot)
{
    JaraxxusState* state = StateFor(bot);
    if (!state)
        return nullptr;

    ObjectGuid const guid = state->incinerateTarget.Get(bot, [bot](ObjectGuid& lowest)
    {
        for (Player* member : AliveMembersOnMap(bot))
            if (HasIncinerateFlesh(member) && (lowest.IsEmpty() || member->GetGUID() < lowest))
                lowest = member->GetGUID();
    });

    return guid.IsEmpty() ? nullptr : ObjectAccessor::GetPlayer(*bot, guid);
}

// What its periodic damage effect still has to tick, at the amount it landed with
uint32 PeriodicDamageLeft(Aura const* aura)
{
    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        AuraEffect const* effect = aura->GetEffect(i);
        if (!effect || effect->GetAuraType() != SPELL_AURA_PERIODIC_DAMAGE)
            continue;

        int32 const ticks = std::max(effect->GetTotalTicks() - static_cast<int32>(effect->GetTickNumber()), 0);
        return static_cast<uint32>(std::max(effect->GetAmount(), 0) * ticks);
    }

    return 0;
}

// The DBC's average roll for one tick of a periodic damage effect
uint32 PeriodicTickOf(uint32 spellId)
{
    SpellInfo const* info = sSpellMgr->GetSpellInfo(spellId);
    if (!info)
        return 0;

    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        SpellEffectInfo const& effect = info->Effects[i];
        if (effect.ApplyAuraName == SPELL_AURA_PERIODIC_DAMAGE)
            return static_cast<uint32>(std::max(effect.BasePoints + (effect.DieSides + 1) / 2, 0));
    }

    return 0;
}

// the kiss punishes a channel, and a bot walking off a flame breaks one
bool CannotChannel(Player* player)
{
    return HasMistressKiss(player) || GetJaraxxusFlameRole(player) != JaraxxusFlameRole::None;
}

InfernoPhase InfernoFor(Player* bot, JaraxxusState& state, RaidPredictedHit& hit)
{
    InfernoMemo& memo = state.inferno;
    uint32 const now = getMSTime();
    if (memo.valid && memo.ms == now)
    {
        hit = memo.hit;
        return memo.phase;
    }

    memo.valid = true;
    memo.ms = now;
    memo.phase = InfernoPhase::None;
    memo.hit = RaidPredictedHit();

    RaidPredictedHit next;
    next.durationMs = (JARAXXUS_INFERNO_TICKS - 1) * IN_MILLISECONDS;
    next.school = SPELL_SCHOOL_MASK_FIRE;
    next.noChannel = &CannotChannel;

    uint32 const infernoId = sSpellMgr->GetSpellIdForDifficulty(SPELL_BURNING_INFERNO, bot);
    for (Player* member : AliveMembersOnMap(bot))
    {
        Aura const* running = member->GetAura(infernoId);
        if (!running)
            continue;

        uint32 const elapsed = static_cast<uint32>(std::max(running->GetMaxDuration() - running->GetDuration(), 0));
        next.startMs = now - elapsed + JARAXXUS_INFERNO_FIRST_TICK_MS;
        next.damage = PeriodicDamageLeft(running);
        memo.phase = InfernoPhase::Running;
        memo.hit = next;
        hit = next;
        return memo.phase;
    }

    Unit* target = IncinerateFleshTargetFor(bot);
    Aura const* incinerate =
        target ? target->GetAura(sSpellMgr->GetSpellIdForDifficulty(SPELL_INCINERATE_FLESH, target)) : nullptr;
    if (!incinerate || incinerate->GetDuration() > static_cast<int32>(JARAXXUS_INFERNO_LEAD_MS) ||
        !IncinerateAbsorbLeft(target))
        return memo.phase;

    // his Nether Power counts: Burning Inferno is cast on his behalf
    float const power =
        1.0f + JARAXXUS_NETHER_POWER_PER_STACK * static_cast<float>(JaraxxusNetherPowerStacks(GET_PLAYERBOT_AI(bot)));
    next.startMs = now + static_cast<uint32>(std::max(incinerate->GetDuration(), 0)) + JARAXXUS_INFERNO_FIRST_TICK_MS;
    next.damage = static_cast<uint32>(PeriodicTickOf(infernoId) * JARAXXUS_INFERNO_TICKS * power);
    memo.phase = InfernoPhase::Coming;
    memo.hit = next;
    hit = next;
    return memo.phase;
}

// Melee DPS only. The trigger reads the distance, the hold also covers one walking in to hit it.
bool ClearsPortal(PlayerbotAI* botAI, Creature* portal, JaraxxusScan const& scan)
{
    Player* bot = botAI->GetBot();
    if (botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Get() != portal &&
        bot->GetExactDist2d(portal) >= JARAXXUS_PORTAL_TRIGGER)
        return false;

    AuraEffect const* spawn =
        portal->GetAuraEffect(sSpellMgr->GetSpellIdForDifficulty(SPELL_NETHER_PORTAL_SPAWN, portal), EFFECT_0);
    if (spawn && spawn->GetPeriodicTimer() <= static_cast<int32>(JARAXXUS_PORTAL_SPAWN_LEAD_MS))
        return true;

    // she opens on whoever stands nearest as she spawns, then goes by threat, so standing off her
    // portal only matters until she has picked
    for (ObjectGuid guid : scan.mistresses)
    {
        Creature* mistress = ResolveAlive(bot, guid);
        TempSummon const* summon = mistress ? mistress->ToTempSummon() : nullptr;
        if (summon && summon->GetSummonerGUID() == portal->GetGUID() && !mistress->GetVictim())
            return true;
    }

    return false;
}

bool IsNonHealingRemover(Player* player)
{
    switch (player->getClass())
    {
        case CLASS_MAGE:
            return true;
        case CLASS_PRIEST:
        case CLASS_SHAMAN:
            return !PlayerbotAI::IsHeal(player);
        default:
            return false;
    }
}

// One on another player first, tanks included: Fel Streak can land on any of them and its 50,000
// threat keeps it there. Then the one already on this bot, so a tank doesn't drop its own for another.
Unit* PickInfernal(Player* bot, JaraxxusScan const& scan)
{
    Unit* lowest = nullptr;
    Unit* onBot = nullptr;
    for (ObjectGuid guid : scan.infernals)
    {
        Creature* infernal = ResolveAlive(bot, guid);
        if (!infernal)
            continue;

        if (!lowest)
            lowest = infernal;

        Unit* victim = infernal->GetVictim();
        if (victim && victim != bot && victim->ToPlayer())
            return infernal;

        if (!onBot && victim == bot)
            onBot = infernal;
    }

    return onBot ? onBot : lowest;
}

bool AnyLivingAssistTankOne(Player* bot, JaraxxusState const& state)
{
    return state.assistTankOne.Get(bot, [bot](bool& any)
    {
        std::vector<Player*> const members = AliveMembersOnMap(bot);
        any = std::any_of(members.begin(), members.end(),
                          [](Player* member) { return PlayerbotAI::IsAssistTankOfIndex(member, 1, true); });
    });
}

bool InInterruptRange(Player* bot, Unit* boss, SpellInfo const* spellInfo)
{
    float const maxRange = bot->GetSpellMaxRangeForTarget(boss, spellInfo);
    if (maxRange <= JARAXXUS_MELEE_INTERRUPT_RANGE)
        return bot->IsWithinMeleeRange(boss);

    return bot->IsWithinCombatRange(boss, maxRange);
}

// CanCastSpell probes with TRIGGERED_IGNORE_POWER_AND_REAGENT_COST, so energy, rage and runic power
// get checked here. POWER_HEALTH is -2 and Powers is signed, hence the cast.
bool CanPayFor(Player* bot, SpellInfo const* spellInfo)
{
    if (spellInfo->PowerType >= static_cast<uint32>(MAX_POWERS))
        return true;

    return static_cast<int32>(bot->GetPower(Powers(spellInfo->PowerType))) >=
           spellInfo->CalcPowerCost(bot, spellInfo->GetSchoolMask());
}

// the sweep's answer when nothing inside its radius is clear
bool IsEmptyPosition(Position const& position) { return !position.GetPositionX() && !position.GetPositionY(); }

float DistanceToSegment2d(Position const& point, Position const& from, Position const& to)
{
    float const dx = to.GetPositionX() - from.GetPositionX();
    float const dy = to.GetPositionY() - from.GetPositionY();
    float const lengthSq = dx * dx + dy * dy;

    float t = 0.0f;
    if (lengthSq > 0.0f)
    {
        t = ((point.GetPositionX() - from.GetPositionX()) * dx + (point.GetPositionY() - from.GetPositionY()) * dy) /
            lengthSq;
        t = std::clamp(t, 0.0f, 1.0f);
    }

    return point.GetExactDist2d(from.GetPositionX() + t * dx, from.GetPositionY() + t * dy);
}

// Only flames that can reach a spot inside the sweep radius
void AddFlameHazards(Player* bot, JaraxxusScan const& scan, std::vector<HazardCircle>& hazards)
{
    for (Position const& flame : scan.flames)
        if (bot->GetExactDist2d(flame) <= JARAXXUS_LEGION_FLAME_SEARCH + JARAXXUS_LEGION_FLAME_CLEAR)
            hazards.emplace_back(flame, JARAXXUS_LEGION_FLAME_CLEAR);
}

// A bot already inside radius of a centre may walk on as long as it gets no nearer to it
bool CirclesCrossPath(std::vector<Position> const& centres, float radius, Position const& from, Position const& to)
{
    for (Position const& centre : centres)
    {
        float const limit = std::min(radius, centre.GetExactDist2d(from) - 0.1f);
        if (DistanceToSegment2d(centre, from, to) < limit)
            return true;
    }

    return false;
}

bool FlameCrossesPath(JaraxxusScan const& scan, Position const& from, Position const& to)
{
    return CirclesCrossPath(scan.flames, JARAXXUS_LEGION_FLAME_CLEAR, from, to);
}

// A heroic portal drops a Mistress every 6 s, inside any 8 s carry
std::vector<Position> SelectablePortals(Player* bot, JaraxxusScan const& scan)
{
    std::vector<Position> portals;
    for (ObjectGuid guid : scan.portals)
    {
        Creature* portal = ResolveAlive(bot, guid);
        if (portal && !portal->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE))
            portals.push_back(portal->GetPosition());
    }

    return portals;
}

bool DeriveCarrierSpot(Player* bot, JaraxxusScan const& scan, float heading, Position& spot)
{
    std::vector<HazardCircle> flames;
    AddFlameHazards(bot, scan, flames);
    // next flame lands where the carrier stands. Also keeps the list non-empty before the first
    // flame, since the sweep answers nothing for an empty one
    flames.emplace_back(bot->GetPosition(), JARAXXUS_LEGION_FLAME_CLEAR);

    Position const from = bot->GetPosition();
    std::vector<Position> const portals = SelectablePortals(bot, scan);
    std::vector<HazardCircle> offPortals = flames;
    for (Position const& portal : portals)
        offPortals.emplace_back(portal, JARAXXUS_PORTAL_CLEAR);

    Position const preferNear(bot->GetPositionX() + std::cos(heading) * 2.0f * JARAXXUS_FLAME_CARRIER_MIN_LEG,
                              bot->GetPositionY() + std::sin(heading) * 2.0f * JARAXXUS_FLAME_CARRIER_MIN_LEG,
                              bot->GetPositionZ());

    std::vector<Position> healers;
    std::vector<Player*> const members = AliveMembersOnMap(bot);
    for (Player* member : members)
        if (member != bot && PlayerbotAI::IsHeal(member))
            healers.push_back(member->GetPosition());

    // the ring's first clear spot nearest the heading, so a leg bends round a healer's reach
    auto const healable = [&healers](float x, float y)
    {
        return healers.empty() || std::any_of(healers.begin(), healers.end(), [x, y](Position const& healer)
                                              { return healer.GetExactDist2d(x, y) <= JARAXXUS_CARRIER_HEALER_REACH; });
    };

    HazardSweepCache cache;
    auto const sweep = [&](std::vector<HazardCircle> const& hazards, bool bound, bool offPortal)
    {
        auto const accept = [&](float x, float y)
        {
            if (bot->GetExactDist2d(x, y) < JARAXXUS_FLAME_CARRIER_MIN_LEG || (bound && !healable(x, y)))
                return false;

            return !offPortal ||
                   !CirclesCrossPath(portals, JARAXXUS_PORTAL_CLEAR, from, Position(x, y, from.GetPositionZ()));
        };
        return FindNearestPositionClearOfHazards(bot, hazards, JARAXXUS_LEGION_FLAME_SEARCH, SWEEP_RING_STEP,
                                                 SWEEP_ANGLE_STEP, &preferNear, accept, &cache);
    };

    Unit* boss = FindJaraxxus(bot);

    // a tank carrier drags him along, so it can't keep clear of him or of the melee on him
    if (boss && boss->GetVictim() == bot)
    {
        spot = sweep(offPortals, true, true);
    }
    else
    {
        std::vector<HazardCircle> hazards = offPortals;
        for (Player* member : members)
            if (member != bot && bot->GetExactDist2d(member) <= JARAXXUS_FLAME_CARRIER_RAID_SCAN)
                hazards.emplace_back(member->GetPosition(), JARAXXUS_FLAME_CARRIER_RAID_CLEAR);

        if (boss)
            hazards.emplace_back(boss->GetPosition(), JARAXXUS_FLAME_CARRIER_BOSS_CLEAR);

        spot = sweep(hazards, true, true);
        if (IsEmptyPosition(spot))
            spot = sweep(offPortals, true, true);
    }

    if (IsEmptyPosition(spot) && !healers.empty())
        spot = sweep(offPortals, false, true);

    // walking past a portal still beats standing in the trail
    if (IsEmptyPosition(spot) && !portals.empty())
        spot = sweep(flames, false, false);

    return !IsEmptyPosition(spot);
}

bool DeriveDodgeSpot(Player* bot, JaraxxusScan const& scan, Position& spot)
{
    std::vector<HazardCircle> flames;
    AddFlameHazards(bot, scan, flames);

    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
    Unit* target = botAI ? botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Get() : nullptr;
    if (target && !target->IsAlive())
        target = nullptr;

    // a dodge inside the melee hold's circle only sends the bot back out through the flames
    Unit* held = botAI ? GetJaraxxusPortalToClear(botAI) : nullptr;
    std::vector<HazardCircle> hazards = flames;
    std::vector<Position> heldPortal;
    if (held)
    {
        hazards.emplace_back(held->GetPosition(), JARAXXUS_PORTAL_CLEAR);
        heldPortal.push_back(held->GetPosition());
    }

    bool const ranged = PlayerbotAI::IsRanged(bot);
    Position anchor;
    Position const* preferNear = nullptr;
    if (!ranged)
    {
        if (Unit* toward = target && target != held ? target : FindJaraxxus(bot))
        {
            anchor = toward->GetPosition();
            preferNear = &anchor;
        }
    }

    // the same range reach spell and reach melee walk to, so neither fires from the spot
    float reach = 0.0f;
    if (target && target != held)
    {
        float const range = ranged ? botAI->GetRange("spell") : sPlayerbotAIConfig.MeleeDistance;
        reach = range + bot->GetCombatReach() + target->GetCombatReach() - JARAXXUS_DODGE_REACH_MARGIN;
    }

    Position const from = bot->GetPosition();
    HazardSweepCache cache;
    auto const sweep = [&](std::vector<HazardCircle> const& circles, bool keepReach, bool checkWalk)
    {
        auto const accept = [&](float x, float y)
        {
            if (keepReach && target->GetExactDist2d(x, y) > reach)
                return false;

            if (!checkWalk)
                return true;

            Position const to(x, y, from.GetPositionZ());
            return !FlameCrossesPath(scan, from, to) && !CirclesCrossPath(heldPortal, JARAXXUS_PORTAL_CLEAR, from, to);
        };
        return FindNearestPositionClearOfHazards(bot, circles, JARAXXUS_LEGION_FLAME_SEARCH, SWEEP_RING_STEP,
                                                 SWEEP_ANGLE_STEP, preferNear, accept, &cache);
    };

    spot = Position();
    if (reach > 0.0f)
        spot = sweep(hazards, true, true);

    if (IsEmptyPosition(spot))
        spot = sweep(hazards, false, true);

    if (IsEmptyPosition(spot))
        spot = sweep(flames, false, false);

    return !IsEmptyPosition(spot);
}

}  // namespace

Unit* GetJaraxxus(PlayerbotAI* botAI) { return botAI ? FindJaraxxus(botAI->GetBot()) : nullptr; }

bool JaraxxusHasNetherPower(Unit* jaraxxus)
{
    if (!jaraxxus)
        return false;

    return jaraxxus->HasAura(SPELL_NETHER_POWER_10N) ||
           jaraxxus->HasAura(SPELL_NETHER_POWER_10H) ||
           jaraxxus->HasAura(SPELL_NETHER_POWER_25N) ||
           jaraxxus->HasAura(SPELL_NETHER_POWER_25H);
}

uint32 JaraxxusNetherPowerStacks(PlayerbotAI* botAI)
{
    Player* bot = botAI ? botAI->GetBot() : nullptr;
    if (!StateFor(bot))
        return 0;

    uint32 stacks = 0;
    if (Unit* boss = FindJaraxxus(bot))
    {
        for (uint32 spellId :
             {SPELL_NETHER_POWER_10N, SPELL_NETHER_POWER_25N, SPELL_NETHER_POWER_10H, SPELL_NETHER_POWER_25H})
        {
            if (Aura* aura = boss->GetAura(spellId))
            {
                stacks = aura->GetStackAmount();
                break;
            }
        }
    }

    return stacks;
}

bool JaraxxusIsNetherPowerRemover(Player* bot)
{
    JaraxxusState* state = StateFor(bot);
    if (!state)
        return false;

    if (IsNonHealingRemover(bot))
        return true;

    uint8 const botClass = bot->getClass();
    if (botClass != CLASS_PRIEST && botClass != CLASS_SHAMAN)
        return false;

    // a healer only fills in when nobody else can, and never while Incinerate Flesh needs its heals.
    // a healing priest or shaman isn't one itself, so the group's answer only counts the others
    bool const covered = state->nonHealingRemover.Get(bot, [bot](bool& any)
    {
        std::vector<Player*> const members = AliveMembersOnMap(bot);
        any = std::any_of(members.begin(), members.end(), IsNonHealingRemover);
    });
    if (covered)
        return false;

    return !IncinerateFleshTargetFor(bot);
}

bool HasIncinerateFlesh(Unit* unit)
{
    return unit && unit->HasAura(sSpellMgr->GetSpellIdForDifficulty(SPELL_INCINERATE_FLESH, unit));
}

Unit* GetIncinerateFleshTarget(PlayerbotAI* botAI)
{
    return botAI ? IncinerateFleshTargetFor(botAI->GetBot()) : nullptr;
}

bool IsLegionFlameCarrier(Unit* unit)
{
    return unit && (unit->HasAura(sSpellMgr->GetSpellIdForDifficulty(SPELL_LEGION_FLAME, unit)) ||
                    unit->HasAura(sSpellMgr->GetSpellIdForDifficulty(SPELL_LEGION_FLAME_TRAIL, unit)));
}

bool HasMistressKiss(Unit* unit)
{
    return unit && unit->HasAura(sSpellMgr->GetSpellIdForDifficulty(SPELL_MISTRESS_KISS, unit));
}

bool IsJaraxxusAdd(Unit* unit)
{
    Creature* creature = unit ? unit->ToCreature() : nullptr;
    if (!creature)
        return false;

    switch (static_cast<ToCNpcs>(creature->GetEntry()))
    {
        case ToCNpcs::NPC_NETHER_PORTAL:
        case ToCNpcs::NPC_INFERNAL_VOLCANO:
        case ToCNpcs::NPC_MISTRESS_OF_PAIN:
        case ToCNpcs::NPC_FEL_INFERNAL:
            return true;
        default:
            return false;
    }
}

Unit* GetJaraxxusFocusAdd(PlayerbotAI* botAI)
{
    Player* bot = botAI ? botAI->GetBot() : nullptr;
    JaraxxusState* state = StateFor(bot);
    if (!state)
        return nullptr;

    JaraxxusScan const& scan = ScanOf(bot, *state);

    Unit* focus = FirstSelectable(bot, scan.portals);
    if (!focus)
        focus = FirstSelectable(bot, scan.volcanoes);
    if (!focus)
        focus = FirstAlive(bot, scan.mistresses);
    if (!focus)
        focus = FirstAlive(bot, scan.infernals);

    return focus;
}

Unit* GetJaraxxusAssistTankAdd(PlayerbotAI* botAI, uint8 index)
{
    Player* bot = botAI ? botAI->GetBot() : nullptr;
    JaraxxusState* state = StateFor(bot);
    if (!state)
        return nullptr;

    JaraxxusScan const& scan = ScanOf(bot, *state);

    Unit* add = nullptr;
    if (index == 0)
    {
        add = FirstAlive(bot, scan.mistresses);
        if (!add && !AnyLivingAssistTankOne(bot, *state))
            add = PickInfernal(bot, scan);
    }
    else if (index == 1)
    {
        add = PickInfernal(bot, scan);
        if (!add)
            add = SecondAlive(bot, scan.mistresses);
    }

    return add;
}

bool JaraxxusMainTankHolds(PlayerbotAI* botAI)
{
    if (!botAI || !PlayerbotAI::IsMainTank(botAI->GetBot()))
        return false;

    // Barrett's gossip frees him straight into combat, so out of combat there's nothing to hold yet
    Unit* jaraxxus = GetJaraxxus(botAI);
    return jaraxxus && jaraxxus->IsInCombat();
}

bool JaraxxusAssistTankHolds(PlayerbotAI* botAI, uint8 index)
{
    Player* bot = botAI ? botAI->GetBot() : nullptr;
    return bot && PlayerbotAI::IsAssistTankOfIndex(bot, index, true) && GetJaraxxusAssistTankAdd(botAI, index);
}

Unit* GetJaraxxusDpsTarget(PlayerbotAI* botAI)
{
    Player* bot = botAI ? botAI->GetBot() : nullptr;
    if (!StateFor(bot) || !IsRaidDpsBot(bot))
        return nullptr;

    Unit* target = nullptr;
    if (ToCEncounterGateOpen(botAI, ToCEncounter::Jaraxxus))
    {
        auto const belongs = [](Unit* unit)
        { return IsJaraxxusAdd(unit) || unit->GetEntry() == static_cast<uint32>(ToCNpcs::NPC_JARAXXUS); };

        if (Unit* skull = GetRaidHumanSkull(botAI, belongs))
            target = skull;
        else if (Unit* focus = GetJaraxxusFocusAdd(botAI))
            target = focus;
        else if (Unit* boss = GetJaraxxus(botAI); boss && boss->IsInCombat())
            target = boss;
    }

    return target;
}

bool JaraxxusIsFelFireballCasting(Unit* boss)
{
    Spell* spell = boss ? boss->GetCurrentSpell(CURRENT_GENERIC_SPELL) : nullptr;
    return spell && spell->GetSpellInfo() &&
           spell->GetSpellInfo()->Id == sSpellMgr->GetSpellIdForDifficulty(SPELL_FEL_FIREBALL, boss);
}

char const* JaraxxusReadyInterrupt(Player* bot, Unit* boss)
{
    if (!bot || !boss)
        return nullptr;

    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
    if (!botAI || bot->HasUnitState(UNIT_STATE_CASTING))
        return nullptr;

    // He is immune to silence, so Silencing Shot and Silence never land. Spell Lock is left out
    // because CanCastSpell answers any pet spell true without looking at its cooldown.
    static char const* const interrupts[] = {"kick",         "pummel",       "shield bash",
                                             "mind freeze",  "counterspell", "wind shear"};

    for (char const* interrupt : interrupts)
    {
        uint32 const spellId = botAI->GetAiObjectContext()->GetValue<uint32>("spell id", interrupt)->Get();
        SpellInfo const* spellInfo = spellId ? sSpellMgr->GetSpellInfo(spellId) : nullptr;
        if (!spellInfo || !CanPayFor(bot, spellInfo) || !InInterruptRange(bot, boss, spellInfo))
            continue;

        if (botAI->CanCastSpell(spellId, boss))
            return interrupt;
    }

    return nullptr;
}

bool JaraxxusIsFelFireballInterrupter(Player* bot, Unit* boss)
{
    JaraxxusState* state = StateFor(bot);
    if (!state || !boss)
        return false;

    bool interrupter = JaraxxusIsFelFireballCasting(boss) && JaraxxusReadyInterrupt(bot, boss);
    if (interrupter)
    {
        // the lowest guid with an interrupt ready takes the cast
        ObjectGuid const first = state->fireballInterrupter.Get(
            bot,
            [bot, boss](ObjectGuid& lowest)
            {
                std::vector<Player*> members = AliveMembersOnMap(bot);
                std::sort(members.begin(), members.end(),
                          [](Player* lhs, Player* rhs) { return lhs->GetGUID() < rhs->GetGUID(); });
                for (Player* member : members)
                {
                    if (JaraxxusReadyInterrupt(member, boss))
                    {
                        lowest = member->GetGUID();
                        break;
                    }
                }
            },
            boss->GetGUID());
        interrupter = first.IsEmpty() || !(first < bot->GetGUID());
    }

    return interrupter;
}

bool JaraxxusSpellHasCastTime(PlayerbotAI* botAI, std::string const& spell, bool withChannels)
{
    uint32 const spellId = botAI->GetAiObjectContext()->GetValue<uint32>("spell id", spell)->Get();
    SpellInfo const* info = spellId ? sSpellMgr->GetSpellInfo(spellId) : nullptr;
    if (!info)
        return false;

    if (info->IsChanneled())
        return withChannels;

    return info->CalcCastTime(botAI->GetBot()) > 0;
}

std::vector<Player*> GetJaraxxusPinnedCasters(Player* bot)
{
    std::vector<Player*> pinned;
    JaraxxusState* state = StateFor(bot);
    if (!state)
        return pinned;

    std::vector<ObjectGuid> const& casters = state->pinnedCasters.Get(bot, [bot](std::vector<ObjectGuid>& out)
    {
        for (Player* member : AliveMembersOnMap(bot))
        {
            if (!member->HasUnitState(UNIT_STATE_CASTING) || !GET_PLAYERBOT_AI(member))
                continue;

            if (HasMistressKiss(member) || IsLegionFlameCarrier(member) || JaraxxusRaidCoverPinned(member))
                out.push_back(member->GetGUID());
        }
    });

    for (ObjectGuid const& guid : casters)
        if (Player* caster = ObjectAccessor::GetPlayer(*bot, guid))
            pinned.push_back(caster);

    return pinned;
}

JaraxxusFlameRole GetJaraxxusFlameRole(Player* bot)
{
    JaraxxusScan const* scan = ScanFor(bot);
    if (!scan || !bot->IsAlive())
        return JaraxxusFlameRole::None;

    if (IsLegionFlameCarrier(bot))
        return JaraxxusFlameRole::Carrier;

    for (Position const& flame : scan->flames)
        if (bot->GetExactDist2d(flame) < JARAXXUS_LEGION_FLAME_TRIGGER)
            return JaraxxusFlameRole::Dodge;

    return JaraxxusFlameRole::None;
}

float GetLegionFlameCarrierHeading(Player* bot)
{
    if (!bot)
        return 0.0f;

    Unit* boss = FindJaraxxus(bot);
    Position const from = boss ? boss->GetPosition() : ARENA_CENTER;
    return from.GetAngle(bot);
}

bool DeriveLegionFlameSpot(Player* bot, JaraxxusFlameRole role, float heading, Position& spot)
{
    JaraxxusScan const* scan = ScanFor(bot);
    if (!scan)
        return false;

    switch (role)
    {
        case JaraxxusFlameRole::Carrier:
            return DeriveCarrierSpot(bot, *scan, heading, spot);
        case JaraxxusFlameRole::Dodge:
            return DeriveDodgeSpot(bot, *scan, spot);
        default:
            return false;
    }
}

bool IsLegionFlameSpotClear(PlayerbotAI* botAI, Position const& spot)
{
    JaraxxusScan const* scan = botAI ? ScanFor(botAI->GetBot()) : nullptr;
    if (!scan)
        return true;

    for (Position const& flame : scan->flames)
        if (flame.GetExactDist2d(spot) < JARAXXUS_LEGION_FLAME_CLEAR)
            return false;

    return true;
}

bool LegionFlameCrossesPath(Player* bot, Position const& to)
{
    JaraxxusScan const* scan = ScanFor(bot);
    return scan && FlameCrossesPath(*scan, bot->GetPosition(), to);
}

bool DeriveLegionFlameDetour(Player* bot, Position const& anchor, float maxStep, Position& spot)
{
    JaraxxusScan const* scan = ScanFor(bot);
    if (!scan)
        return false;

    std::vector<HazardCircle> hazards;
    AddFlameHazards(bot, *scan, hazards);
    if (hazards.empty())
        return false;

    Position const from = bot->GetPosition();
    auto const reachable = [scan, &from](float x, float y)
    { return !FlameCrossesPath(*scan, from, Position(x, y, from.GetPositionZ())); };

    spot = FindNearestPositionClearOfHazards(bot, hazards, maxStep, SWEEP_RING_STEP, SWEEP_ANGLE_STEP, &anchor,
                                             reachable);
    return !IsEmptyPosition(spot);
}

bool JaraxxusAnyLegionFlame(PlayerbotAI* botAI)
{
    JaraxxusScan const* scan = botAI ? ScanFor(botAI->GetBot()) : nullptr;
    return scan && !scan->flames.empty();
}

bool JaraxxusFlameNear(PlayerbotAI* botAI)
{
    Player* bot = botAI ? botAI->GetBot() : nullptr;
    JaraxxusScan const* scan = ScanFor(bot);
    if (!scan)
        return false;

    return std::any_of(scan->flames.begin(), scan->flames.end(), [bot](Position const& flame)
                       { return bot->GetExactDist2d(flame) < JARAXXUS_FLAME_STAND_REACH; });
}

bool GetJaraxxusBurningInferno(PlayerbotAI* botAI, RaidPredictedHit& hit)
{
    Player* bot = botAI ? botAI->GetBot() : nullptr;
    JaraxxusState* state = StateFor(bot);
    return state && InfernoFor(bot, *state, hit) != InfernoPhase::None;
}

bool GetJaraxxusPersonalHit(PlayerbotAI* botAI, RaidPredictedHit& hit)
{
    Player* bot = botAI ? botAI->GetBot() : nullptr;
    JaraxxusState* state = StateFor(bot);
    if (!state || !bot->IsAlive())
        return false;

    uint32 const now = getMSTime();
    if (Aura const* own = bot->GetAura(sSpellMgr->GetSpellIdForDifficulty(SPELL_BURNING_INFERNO, bot)))
    {
        InfernoFor(bot, *state, hit);
        uint32 const elapsed = static_cast<uint32>(std::max(own->GetMaxDuration() - own->GetDuration(), 0));
        hit.startMs = now - elapsed + JARAXXUS_INFERNO_FIRST_TICK_MS;
        hit.damage = PeriodicDamageLeft(own);
        return true;
    }

    // one running on the others and not on this bot passes it by
    if (InfernoFor(bot, *state, hit) == InfernoPhase::Coming)
        return true;

    if (!IsLegionFlameCarrier(bot) || bot->GetHealthPct() >= JARAXXUS_CARRIER_DEFENSIVE_PCT)
        return false;

    uint32 const trailId = sSpellMgr->GetSpellIdForDifficulty(SPELL_LEGION_FLAME_TRAIL, bot);
    hit = RaidPredictedHit();
    hit.school = SPELL_SCHOOL_MASK_FIRE;
    if (Aura const* trail = bot->GetAura(trailId))
    {
        hit.startMs = now;
        hit.durationMs = static_cast<uint32>(std::max(trail->GetDuration(), 0));
        hit.damage = PeriodicDamageLeft(trail);
        return true;
    }

    // the 2 s warning ahead of a 6 s trail ticking once a second
    Aura const* warning = bot->GetAura(sSpellMgr->GetSpellIdForDifficulty(SPELL_LEGION_FLAME, bot));
    if (!warning)
        return false;

    SpellInfo const* info = sSpellMgr->GetSpellInfo(trailId);
    uint32 const ticks = info ? static_cast<uint32>(std::max(info->GetMaxDuration(), 0)) / IN_MILLISECONDS : 0;
    hit.startMs = now + static_cast<uint32>(std::max(warning->GetDuration(), 0)) + IN_MILLISECONDS;
    hit.durationMs = ticks > 1 ? (ticks - 1) * IN_MILLISECONDS : 0;
    hit.damage = PeriodicTickOf(trailId) * ticks;
    return true;
}

bool JaraxxusRaidCoverHeld(PlayerbotAI* botAI)
{
    RaidPredictedHit hit;
    return GetJaraxxusBurningInferno(botAI, hit) && RaidHitLeadIn(hit);
}

bool JaraxxusMagicHitOnBot(PlayerbotAI* botAI)
{
    RaidPredictedHit hit;
    return botAI && (IsLegionFlameCarrier(botAI->GetBot()) || GetJaraxxusBurningInferno(botAI, hit));
}

bool JaraxxusRaidCoverPinned(Player* member)
{
    PlayerbotAI* memberAI = GET_PLAYERBOT_AI(member);
    RaidPredictedHit hit;
    return memberAI && GetJaraxxusBurningInferno(memberAI, hit) &&
           GetRaidCover(memberAI, TOC_DEFENSIVE_SCOPE, &hit) != RaidCover::None;
}

Unit* GetJaraxxusPortalToClear(PlayerbotAI* botAI)
{
    Player* bot = botAI ? botAI->GetBot() : nullptr;
    JaraxxusState* state = StateFor(bot);
    if (!state || !bot->IsAlive() || !IsRaidDpsBot(bot) || PlayerbotAI::IsRanged(bot))
        return nullptr;

    JaraxxusScan const& scan = ScanOf(bot, *state);
    Creature* clear = nullptr;
    for (ObjectGuid guid : scan.portals)
    {
        Creature* portal = ResolveAlive(bot, guid);
        if (portal && !portal->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE) && ClearsPortal(botAI, portal, scan))
        {
            clear = portal;
            break;
        }
    }

    return clear;
}

bool DeriveJaraxxusPortalExit(Player* bot, Unit* portal, Position& spot)
{
    JaraxxusScan const* scan = ScanFor(bot);
    if (!scan || !portal)
        return false;

    std::vector<HazardCircle> hazards;
    AddFlameHazards(bot, *scan, hazards);
    hazards.emplace_back(portal->GetPosition(), JARAXXUS_PORTAL_CLEAR);

    // back toward him, where the melee go once she has picked
    Unit* boss = FindJaraxxus(bot);
    Position const toward = boss ? boss->GetPosition() : bot->GetPosition();
    spot = FindNearestPositionClearOfHazards(bot, hazards, JARAXXUS_PORTAL_CLEAR + JARAXXUS_LEGION_FLAME_SEARCH,
                                             SWEEP_RING_STEP, SWEEP_ANGLE_STEP, &toward);
    return !IsEmptyPosition(spot);
}

bool JaraxxusThreatCapped(Player* bot)
{
    JaraxxusState* state = StateFor(bot);
    if (!state || !IsRaidDpsBot(bot))
        return false;

    ThreatCapLatch& cap = state->threatCaps[bot->GetGUID()];
    uint32 const now = getMSTime();
    if (cap.memoValid && cap.memoMs == now)
        return cap.value;

    cap.memoValid = true;
    cap.memoMs = now;
    cap.value = false;

    Unit* boss = FindJaraxxus(bot);
    Unit* victim = boss ? boss->GetVictim() : nullptr;
    Player* tank = victim ? victim->ToPlayer() : nullptr;
    if (!tank || tank == bot || (!PlayerbotAI::IsTank(tank) && !PlayerbotAI::IsTank(tank, true)))
        return false;

    // the "threat" AI value is a share in a uint8 and wraps past 255%
    float const own = boss->GetThreatMgr().GetThreat(bot);
    float const held = boss->GetThreatMgr().GetThreat(tank);
    if (!cap.capped && own > 0.0f && own >= JARAXXUS_THREAT_CAP * held)
        cap.capped = true;
    else if (cap.capped && own < JARAXXUS_THREAT_RELEASE * held)
        cap.capped = false;

    cap.value = cap.capped;
    return cap.value;
}

}
