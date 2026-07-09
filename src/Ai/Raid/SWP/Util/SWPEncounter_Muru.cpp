/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include <algorithm>
#include <list>

#include "SWPEncounter_Muru.h"
#include "CharmInfo.h"
#include "Playerbots.h"
#include "RaidBossHelpers.h"

// M'uru goes invisible during the Entropius phase but remains on player threat lists

namespace SunwellHelpers
{

const Position MURU_STACK_POSITION =                { 1836.532f, 608.957f, 71.222f };
const Position MURU_VOID_SENTINEL_N_TANK_POSITION = { 1840.448f, 630.605f, 70.567f };
const Position MURU_VOID_SENTINEL_E_TANK_POSITION = { 1814.960f, 601.646f, 70.547f };
const Position MURU_CENTER_POSITION =               { 1816.250f, 625.484f, 69.604f };
const Position MURU_ENTRANCE_POSITION =             { 1840.567f, 605.769f, 71.250f };

std::unordered_map<uint32, MuruDarknessState> muruDarknessStates;
std::unordered_map<uint32, std::unordered_map<ObjectGuid, uint8>> muruVoidSentinelTankAssignments;

bool TryGetMuruDarknessActiveState(Player* bot, Unit* muru)
{
    if (!muru)
        return false;

    constexpr uint32 darknessPreEffectMs = 3000;
    constexpr uint32 darknessCastMs = 2000;
    constexpr uint32 darknessPostCastDangerMs = 18000;
    constexpr uint32 darknessTotalMs =
        darknessPreEffectMs + darknessCastMs + darknessPostCastDangerMs;
    const uint32 instanceId = bot->GetInstanceId();
    const uint32 now = getMSTime();
    MuruDarknessState& state = muruDarknessStates[instanceId];

    if (Aura* darknessPreEffect = muru->GetAura(
            static_cast<uint32>(SunwellSpells::SPELL_DARKNESS_PRE_EFFECT)))
    {
        int32 remainingPreEffectMs = darknessPreEffect->GetDuration();
        if (remainingPreEffectMs < 0)
            remainingPreEffectMs = darknessPreEffectMs;

        const uint32 remainingPreEffect = static_cast<uint32>(remainingPreEffectMs);
        const uint32 elapsedPreEffectMs = remainingPreEffect < darknessPreEffectMs ?
            darknessPreEffectMs - remainingPreEffect : 0;
        const uint32 startMs = now > elapsedPreEffectMs ? now - elapsedPreEffectMs : 0;

        if (!state.startMs || state.expireMs <= now || startMs < state.startMs)
            state.startMs = startMs;

        state.expireMs = std::max(state.expireMs, startMs + darknessTotalMs);
        return true;
    }

    if (muru->HasUnitState(UNIT_STATE_CASTING) &&
        muru->FindCurrentSpellBySpellId(static_cast<uint32>(SunwellSpells::SPELL_DARKNESS)))
    {
        const uint32 startMs = now > darknessPreEffectMs ? now - darknessPreEffectMs : 0;
        if (!state.startMs || state.expireMs <= now || startMs < state.startMs)
            state.startMs = startMs;

        state.expireMs = std::max(state.expireMs, now + darknessCastMs + darknessPostCastDangerMs);
        return true;
    }

    if (state.expireMs > now)
        return true;

    muruDarknessStates.erase(instanceId);
    return false;
}

bool TryGetMuruDarknessEarlyState(Player* bot, Unit* muru, uint32 earlyWindowMs)
{
    if (!TryGetMuruDarknessActiveState(bot, muru))
        return false;

    auto const stateItr = muruDarknessStates.find(bot->GetInstanceId());
    if (stateItr == muruDarknessStates.end())
        return false;

    const uint32 now = getMSTime();
    return stateItr->second.startMs < now && now - stateItr->second.startMs < earlyWindowMs;
}

void GatherMuruEncounterTargets(PlayerbotAI* botAI, MuruEncounterTargets& targets)
{
    auto const& units =
        botAI->GetAiObjectContext()->GetValue<GuidVector>("possible targets no los")->Get();

    auto const considerTarget = [&](Unit* unit)
    {
        if (!unit || !unit->IsAlive())
            return;

        switch (unit->GetEntry())
        {
            case static_cast<uint32>(SunwellNpcs::NPC_MURU):
                targets.muru = unit;
                break;

            case static_cast<uint32>(SunwellNpcs::NPC_ENTROPIUS):
                targets.entropius = unit;
                break;

            case static_cast<uint32>(SunwellNpcs::NPC_VOID_SENTINEL):
                targets.voidSentinels.push_back(unit);
                break;

            case static_cast<uint32>(SunwellNpcs::NPC_VOID_SPAWN):
                targets.voidSpawns.push_back(unit);
                break;

            case static_cast<uint32>(SunwellNpcs::NPC_SHADOWSWORD_FURY_MAGE):
                targets.furyMages.push_back(unit);
                break;

            case static_cast<uint32>(SunwellNpcs::NPC_SHADOWSWORD_BERSERKER):
                targets.berserkers.push_back(unit);
                break;

            default:
                break;
        }
    };

    for (const ObjectGuid& guid : units)
    {
        Unit* unit = botAI->GetUnit(guid);
        considerTarget(unit);
    }
}

Creature* FindAvailableVoidSpawnForEnslave(PlayerbotAI* botAI, Player* bot)
{
    Creature* bestSpawn = nullptr;
    float closestDistance = std::numeric_limits<float>::max();
    auto const& units =
        botAI->GetAiObjectContext()->GetValue<GuidVector>("possible targets no los")->Get();

    for (const ObjectGuid& guid : units)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive() ||
            unit->GetEntry() != static_cast<uint32>(SunwellNpcs::NPC_VOID_SPAWN) ||
            unit->IsCharmed() || unit->GetCharmer())
        {
            continue;
        }

        float distance = bot->GetExactDist2d(unit);
        if (distance >= closestDistance)
            continue;

        Creature* creature = unit->ToCreature();
        if (!creature)
            continue;

        bestSpawn = creature;
        closestDistance = distance;
    }

    return bestSpawn;
}

Creature* GetNearestMuruSingularity(Player* bot, float searchRadius)
{
    Creature* nearestSingularity = nullptr;
    float nearestDistance = std::numeric_limits<float>::max();
    std::list<Creature*> singularities;
    bot->GetCreatureListWithEntryInGrid(
        singularities, static_cast<uint32>(SunwellNpcs::NPC_SINGULARITY), searchRadius);

    for (Creature* singularity : singularities)
    {
        if (!singularity || !singularity->IsAlive())
            continue;

        float distance = bot->GetExactDist2d(singularity);
        if (distance < nearestDistance)
        {
            nearestDistance = distance;
            nearestSingularity = singularity;
        }
    }

    return nearestSingularity;
}

}
