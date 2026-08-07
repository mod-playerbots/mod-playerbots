/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "SWPEncounter_Muru.h"
#include "CharmInfo.h"
#include "Playerbots.h"
#include <algorithm>
#include <list>

// Note: M'uru goes invisible during the Entropius phase but remains on player threat lists

namespace SwpHelpers
{

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

    if (Aura* darknessPreEffect = muru->GetAura(Id(SwpSpells::SPELL_DARKNESS_PRE_EFFECT)))
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
        muru->FindCurrentSpellBySpellId(Id(SwpSpells::SPELL_DARKNESS)))
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
    AiObjectContext* context = botAI->GetAiObjectContext();
    auto const& units = AI_VALUE(GuidVector, "possible targets no los");

    auto const considerTarget = [&](Unit* unit)
    {
        if (!unit)
            return;

        switch (unit->GetEntry())
        {
            case Id(SwpNpcs::NPC_MURU):
                targets.muru = unit;
                break;

            case Id(SwpNpcs::NPC_ENTROPIUS):
                targets.entropius = unit;
                break;

            case Id(SwpNpcs::NPC_VOID_SENTINEL):
                targets.voidSentinels.push_back(unit);
                break;

            case Id(SwpNpcs::NPC_VOID_SPAWN):
                targets.voidSpawns.push_back(unit);
                break;

            case Id(SwpNpcs::NPC_SHADOWSWORD_FURY_MAGE):
                targets.furyMages.push_back(unit);
                break;

            case Id(SwpNpcs::NPC_SHADOWSWORD_BERSERKER):
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
    AiObjectContext* context = botAI->GetAiObjectContext();
    auto const& units = AI_VALUE(GuidVector, "possible targets no los");

    Creature* bestSpawn = nullptr;
    float closestDistance = std::numeric_limits<float>::max();

    for (ObjectGuid const& guid : units)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || unit->GetEntry() != Id(SwpNpcs::NPC_VOID_SPAWN) ||
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

}
