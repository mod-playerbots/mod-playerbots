/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "SWPEncounter_Muru.h"
#include "CharmInfo.h"
#include "Playerbots.h"
#include "RaidBossHelpers.h"
#include <algorithm>
#include <list>

// Note: M'uru goes invisible during the Entropius phase but remains on player threat lists

namespace SwpHelpers
{

Position const MURU_STACK_POSITION =                { 1836.532f, 608.957f, 71.222f };
Position const MURU_VOID_SENTINEL_N_TANK_POSITION = { 1840.448f, 630.605f, 70.567f };
Position const MURU_VOID_SENTINEL_E_TANK_POSITION = { 1814.960f, 601.646f, 70.547f };
Position const MURU_CENTER_POSITION =               { 1816.250f, 625.484f, 69.604f };
Position const MURU_ENTRANCE_POSITION =             { 1840.567f, 605.769f, 71.250f };

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
    uint32 const instanceId = bot->GetInstanceId();
    uint32 const now = getMSTime();
    MuruDarknessState& state = muruDarknessStates[instanceId];

    if (Aura* darknessPreEffect = muru->GetAura(
            static_cast<uint32>(SwpSpells::SPELL_DARKNESS_PRE_EFFECT)))
    {
        int32 remainingPreEffectMs = darknessPreEffect->GetDuration();
        if (remainingPreEffectMs < 0)
            remainingPreEffectMs = darknessPreEffectMs;

        uint32 const remainingPreEffect = static_cast<uint32>(remainingPreEffectMs);
        uint32 const elapsedPreEffectMs = remainingPreEffect < darknessPreEffectMs ?
            darknessPreEffectMs - remainingPreEffect : 0;
        uint32 const startMs = now > elapsedPreEffectMs ? now - elapsedPreEffectMs : 0;

        if (!state.startMs || state.expireMs <= now || startMs < state.startMs)
            state.startMs = startMs;

        state.expireMs = std::max(state.expireMs, startMs + darknessTotalMs);
        return true;
    }

    if (muru->HasUnitState(UNIT_STATE_CASTING) &&
        muru->FindCurrentSpellBySpellId(static_cast<uint32>(SwpSpells::SPELL_DARKNESS)))
    {
        uint32 const startMs = now > darknessPreEffectMs ? now - darknessPreEffectMs : 0;
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

    uint32 const now = getMSTime();
    return stateItr->second.startMs < now && now - stateItr->second.startMs < earlyWindowMs;
}

void GatherMuruEncounterTargets(PlayerbotAI* botAI, MuruEncounterTargets& targets)
{
    auto const& units =
        botAI->GetAiObjectContext()->GetValue<GuidVector>("possible targets no los")->Get();

    auto const considerTarget = [&](Unit* unit)
    {
        if (!unit)
            return;

        switch (unit->GetEntry())
        {
            case static_cast<uint32>(SwpNpcs::NPC_MURU):
                targets.muru = unit;
                break;

            case static_cast<uint32>(SwpNpcs::NPC_ENTROPIUS):
                targets.entropius = unit;
                break;

            case static_cast<uint32>(SwpNpcs::NPC_VOID_SENTINEL):
                targets.voidSentinels.push_back(unit);
                break;

            case static_cast<uint32>(SwpNpcs::NPC_VOID_SPAWN):
                targets.voidSpawns.push_back(unit);
                break;

            case static_cast<uint32>(SwpNpcs::NPC_SHADOWSWORD_FURY_MAGE):
                targets.furyMages.push_back(unit);
                break;

            case static_cast<uint32>(SwpNpcs::NPC_SHADOWSWORD_BERSERKER):
                targets.berserkers.push_back(unit);
                break;

            default:
                break;
        }
    };

    for (ObjectGuid const& guid : units)
    {
        Unit* unit = botAI->GetUnit(guid);
        considerTarget(unit);
    }
}

Creature* FindAvailableVoidSpawnForEnslave(Player* bot)
{
    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
    Creature* bestSpawn = nullptr;
    float closestDistance = std::numeric_limits<float>::max();
    auto const& units =
        botAI->GetAiObjectContext()->GetValue<GuidVector>("possible targets no los")->Get();

    for (ObjectGuid const& guid : units)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || unit->GetEntry() != static_cast<uint32>(SwpNpcs::NPC_VOID_SPAWN) ||
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
        singularities, static_cast<uint32>(SwpNpcs::NPC_SINGULARITY), searchRadius);

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
