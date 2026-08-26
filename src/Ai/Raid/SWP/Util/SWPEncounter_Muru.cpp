/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "SWPEncounter_Muru.h"
#include "AiObjectContext.h"
#include "Playerbots.h"
#include <algorithm>
#include <list>

namespace SwpHelpers
{

namespace
{

void StampMuruDarknessWindow(
    MuruDarknessState& state, uint32 now, uint32 elapsedMs, uint32 remainingMs)
{
    uint32 const startMs = now > elapsedMs ? now - elapsedMs : 0;

    if (!state.startMs || state.expireMs <= now || startMs < state.startMs)
        state.startMs = startMs;

    state.expireMs = std::max(state.expireMs, now + remainingMs);
}

MuruEncounterGuids const& GetCachedMuruEncounterGuids(PlayerbotAI* botAI)
{
    return botAI->GetAiObjectContext()
        ->GetValue<MuruEncounterGuids>("muru encounter targets")->RefGet();
}

Unit* ResolveLivingUnit(PlayerbotAI* botAI, ObjectGuid const& guid)
{
    Unit* unit = botAI->GetUnit(guid);
    return unit && unit->IsAlive() ? unit : nullptr;
}

void ResolveLivingUnits(PlayerbotAI* botAI, GuidVector const& guids, std::vector<Unit*>& units)
{
    units.reserve(guids.size());
    for (ObjectGuid const& guid : guids)
    {
        if (Unit* unit = ResolveLivingUnit(botAI, guid))
            units.push_back(unit);
    }
}

float GetBerserkerStunReach(Player* bot)
{
    switch (bot->getClass())
    {
        case CLASS_DRUID:
        case CLASS_ROGUE:
        case CLASS_WARRIOR:
            return MURU_MELEE_ABILITY_REACH;

        case CLASS_PALADIN:
            return MURU_HAMMER_OF_JUSTICE_REACH;

        case CLASS_WARLOCK:
            return MURU_RANGED_ABILITY_REACH;

        default:
            return bot->getRace() == RACE_TAUREN ? MURU_WAR_STOMP_REACH : 0.0f;
    }
}

float GetFuryMageInterruptReach(Player* bot)
{
    switch (bot->getClass())
    {
        case CLASS_ROGUE:
        case CLASS_WARRIOR:
            return MURU_MELEE_ABILITY_REACH;

        case CLASS_SHAMAN:
            return MURU_WIND_SHEAR_REACH;

        case CLASS_DEATH_KNIGHT:
        case CLASS_MAGE:
        case CLASS_PALADIN:
        case CLASS_PRIEST:
        case CLASS_WARLOCK:
            return MURU_RANGED_ABILITY_REACH;

        case CLASS_HUNTER:
            return MURU_SILENCING_SHOT_REACH;

        default:
            return 0.0f;
    }
}

bool IsFlurriedBerserker(Unit* berserker)
{
    return berserker->HasAura(Id(SwpSpells::SPELL_FLURRY)) &&
        !berserker->HasUnitState(UNIT_STATE_STUNNED);
}

bool IsCastingFelFireball(Unit* furyMage)
{
    return furyMage->HasUnitState(UNIT_STATE_CASTING) &&
        furyMage->FindCurrentSpellBySpellId(Id(SwpSpells::SPELL_FEL_FIREBALL));
}

bool IsSpellFuryBuffedFuryMage(Unit* furyMage)
{
    return furyMage->HasAura(Id(SwpSpells::SPELL_SPELL_FURY));
}

Unit* SelectNearestQualifying(
    PlayerbotAI* botAI, GuidVector const& candidates, float reach, bool (*qualifies)(Unit*))
{
    Player* bot = botAI->GetBot();
    Unit* currentTarget = botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Get();

    Unit* best = nullptr;
    float bestDistance = 0.0f;

    for (ObjectGuid const& guid : candidates)
    {
        Unit* candidate = botAI->GetUnit(guid);
        if (!candidate || !candidate->IsAlive() || !qualifies(candidate))
            continue;

        float const distance = bot->GetExactDist(candidate);
        if (distance > reach)
            continue;

        if (candidate == currentTarget)
            return candidate;

        if (!best || distance < bestDistance)
        {
            best = candidate;
            bestDistance = distance;
        }
    }

    return best;
}

} // end anonymous namespace

std::unordered_map<uint32, MuruDarknessState> muruDarknessStates;
std::unordered_map<uint32, std::unordered_map<ObjectGuid, uint8>> muruVoidSentinelTankAssignments;

bool IsMuruPhaseActive(Unit* muru)
{
    // DamageTaken caps M'uru at exactly 1 health when phase 2 activates, and from then on M'uru
    // is invisible but remains on players' threat lists.
    return muru && muru->GetHealth() > 1;
}

bool TryGetMuruDarknessActiveState(Player* bot, Unit* muru)
{
    if (!muru)
        return false;

    uint32 const instanceId = bot->GetInstanceId();
    uint32 const now = getMSTime();

    if (Aura* darkness = muru->GetAura(Id(SwpSpells::SPELL_DARKNESS)))
    {
        int32 const remainingMs = std::max(darkness->GetDuration(), 0);
        int32 const elapsedZoneMs = std::max(darkness->GetMaxDuration() - remainingMs, 0);

        StampMuruDarknessWindow(
            muruDarknessStates[instanceId], now,
            MURU_DARKNESS_PRE_EFFECT_MS + static_cast<uint32>(elapsedZoneMs),
            static_cast<uint32>(remainingMs));
    }
    else if (Aura* preEffect = muru->GetAura(Id(SwpSpells::SPELL_DARKNESS_PRE_EFFECT)))
    {
        int32 const duration = preEffect->GetDuration();
        uint32 const remainingPreEffectMs = duration < 0 ?
            MURU_DARKNESS_PRE_EFFECT_MS :
            std::min(static_cast<uint32>(duration), MURU_DARKNESS_PRE_EFFECT_MS);

        StampMuruDarknessWindow(
            muruDarknessStates[instanceId], now,
            MURU_DARKNESS_PRE_EFFECT_MS - remainingPreEffectMs,
            remainingPreEffectMs + MURU_DARKNESS_AURA_MS);
    }

    auto const stateItr = muruDarknessStates.find(instanceId);
    if (stateItr == muruDarknessStates.end())
        return false;

    uint32 const expireMs = stateItr->second.expireMs;
    if (expireMs > now + MURU_DARKNESS_RUN_BACK_ALLOWANCE_MS)
        return true;

    if (expireMs <= now)
        muruDarknessStates.erase(stateItr);

    return false;
}

bool TryGetMuruDarknessEarlyState(Player* bot, Unit* muru, uint32 earlyWindowMs)
{
    if (!TryGetMuruDarknessActiveState(bot, muru))
        return false;

    auto const stateItr = muruDarknessStates.find(bot->GetInstanceId());
    if (stateItr == muruDarknessStates.end())
        return false;

    uint32 const now = getMSTime();
    return stateItr->second.startMs < now && now - stateItr->second.startMs < earlyWindowMs;
}

MuruEncounterGuids FindMuruEncounterGuids(PlayerbotAI* botAI)
{
    AiObjectContext* context = botAI->GetAiObjectContext();
    auto const& units = AI_VALUE(GuidVector, "possible targets no los");

    MuruEncounterGuids guids;
    for (ObjectGuid const& guid : units)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit)
            continue;

        switch (unit->GetEntry())
        {
            case Id(SwpNpcs::NPC_MURU):
                guids.muru = guid;
                break;

            case Id(SwpNpcs::NPC_ENTROPIUS):
                guids.entropius = guid;
                break;

            case Id(SwpNpcs::NPC_VOID_SENTINEL):
                guids.voidSentinels.push_back(guid);
                break;

            case Id(SwpNpcs::NPC_VOID_SPAWN):
                guids.voidSpawns.push_back(guid);
                break;

            case Id(SwpNpcs::NPC_SHADOWSWORD_FURY_MAGE):
                guids.furyMages.push_back(guid);
                break;

            case Id(SwpNpcs::NPC_SHADOWSWORD_BERSERKER):
                guids.berserkers.push_back(guid);
                break;

            default:
                break;
        }
    }

    return guids;
}

void GatherMuruEncounterTargets(PlayerbotAI* botAI, MuruEncounterTargets& targets)
{
    MuruEncounterGuids const& guids = GetCachedMuruEncounterGuids(botAI);

    targets.muru = ResolveLivingUnit(botAI, guids.muru);
    targets.entropius = ResolveLivingUnit(botAI, guids.entropius);
    ResolveLivingUnits(botAI, guids.voidSentinels, targets.voidSentinels);
    ResolveLivingUnits(botAI, guids.voidSpawns, targets.voidSpawns);
    ResolveLivingUnits(botAI, guids.furyMages, targets.furyMages);
    ResolveLivingUnits(botAI, guids.berserkers, targets.berserkers);
}

Unit* FindMuruBerserkerToStun(PlayerbotAI* botAI)
{
    float const reach = GetBerserkerStunReach(botAI->GetBot());
    if (reach <= 0.0f)
        return nullptr;

    return SelectNearestQualifying(
        botAI, GetCachedMuruEncounterGuids(botAI).berserkers, reach, &IsFlurriedBerserker);
}

Unit* FindMuruFuryMageToInterrupt(PlayerbotAI* botAI)
{
    float const reach = GetFuryMageInterruptReach(botAI->GetBot());
    if (reach <= 0.0f)
        return nullptr;

    return SelectNearestQualifying(
        botAI, GetCachedMuruEncounterGuids(botAI).furyMages, reach, &IsCastingFelFireball);
}

Unit* FindMuruFuryMageToSpellsteal(PlayerbotAI* botAI)
{
    if (botAI->GetBot()->getClass() != CLASS_MAGE)
        return nullptr;

    return SelectNearestQualifying(
        botAI, GetCachedMuruEncounterGuids(botAI).furyMages, MURU_RANGED_ABILITY_REACH,
        &IsSpellFuryBuffedFuryMage);
}

bool IsTankingMuruVoidSentinel(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    for (ObjectGuid const& guid : GetCachedMuruEncounterGuids(botAI).voidSentinels)
    {
        Unit* voidSentinel = ResolveLivingUnit(botAI, guid);
        if (voidSentinel && voidSentinel->GetVictim() == bot)
            return true;
    }

    return false;
}

ObjectGuid FindMuruSingularityGuid(Player* bot)
{
    Creature* singularity = bot->FindNearestCreature(
        Id(SwpNpcs::NPC_SINGULARITY), MURU_SINGULARITY_SEARCH_RADIUS);

    return singularity ? singularity->GetGUID() : ObjectGuid::Empty;
}

GuidVector FindMuruVoidZoneGuids(Player* bot)
{
    std::list<Creature*> voidZones;
    bot->GetCreatureListWithEntryInGrid(
        voidZones, Id(SwpNpcs::NPC_DARKNESS), MURU_VOID_ZONE_SEARCH_RADIUS);

    GuidVector guids;
    guids.reserve(voidZones.size());
    for (Creature* voidZone : voidZones)
    {
        if (voidZone && voidZone->IsAlive())
            guids.push_back(voidZone->GetGUID());
    }

    return guids;
}

Creature* FindMuruVoidZoneToAvoid(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    GuidVector const& guids =
        botAI->GetAiObjectContext()->GetValue<GuidVector>("muru void zones")->RefGet();

    Creature* nearest = nullptr;
    float nearestDistance = MURU_VOID_ZONE_SAFE_DISTANCE;

    for (ObjectGuid const& guid : guids)
    {
        Unit* unit = ResolveLivingUnit(botAI, guid);
        if (!unit)
            continue;

        float const distance = bot->GetDistance2d(unit);
        if (distance >= nearestDistance)
            continue;

        Creature* voidZone = unit->ToCreature();
        if (!voidZone)
            continue;

        nearest = voidZone;
        nearestDistance = distance;
    }

    return nearest;
}

Creature* FindAvailableVoidSpawnForEnslave(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();

    Creature* bestSpawn = nullptr;
    float closestDistance = std::numeric_limits<float>::max();

    for (ObjectGuid const& guid : GetCachedMuruEncounterGuids(botAI).voidSpawns)
    {
        Unit* unit = ResolveLivingUnit(botAI, guid);
        if (!unit || unit->IsCharmed() || unit->GetCharmer())
            continue;

        Creature* creature = unit->ToCreature();
        if (!creature)
            continue;

        float const distance = bot->GetExactDist2d(unit);
        if (distance >= closestDistance)
            continue;

        bestSpawn = creature;
        closestDistance = distance;
    }

    return bestSpawn;
}

}
