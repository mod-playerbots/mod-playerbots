/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "RaidHyjalSummitHelpers.h"

#include <algorithm>

#include "Playerbots.h"
#include "RaidBossHelpers.h"
#include "Timer.h"

namespace HyjalSummitHelpers
{
    // General

    bool GetGroundedStepPosition(
        Player* bot, float destinationX, float destinationY, float moveDist,
        float& stepX, float& stepY, float& stepZ)
    {
        const float distance = bot->GetExactDist2d(destinationX, destinationY);
        if (distance <= 0.0f)
            return false;

        const float stepDistance = std::min(moveDist, distance);
        const float deltaX = destinationX - bot->GetPositionX();
        const float deltaY = destinationY - bot->GetPositionY();
        stepX = bot->GetPositionX() + (deltaX / distance) * stepDistance;
        stepY = bot->GetPositionY() + (deltaY / distance) * stepDistance;
        stepZ = bot->GetMapWaterOrGroundLevel(stepX, stepY, bot->GetPositionZ());
        if (stepZ <= INVALID_HEIGHT)
            stepZ = bot->GetPositionZ();

        bot->GetMap()->CheckCollisionAndGetValidCoords(
            bot, bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(),
            stepX, stepY, stepZ, false);

        return true;
    }

    RangedGroups GetRangedGroups(PlayerbotAI* botAI, Player* bot)
    {
        RangedGroups result;
        Group* group = bot->GetGroup();
        if (!group)
            return result;

        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !botAI->IsRanged(member))
                continue;

            if (botAI->IsHeal(member))
                result.healers.push_back(member);
            else
                result.rangedDps.push_back(member);
        }

        return result;
    }

    std::pair<size_t, size_t> GetBotCircleIndexAndCount(PlayerbotAI* botAI, Player* bot,
                                                        const RangedGroups& groups)
    {
        const std::vector<Player*>& vec = botAI->IsHeal(bot) ? groups.healers : groups.rangedDps;
        auto it = std::find(vec.begin(), vec.end(), bot);
        size_t index = (it != vec.end()) ? std::distance(vec.begin(), it) : 0;

        return {index, vec.size()};
    }

    // Rage Winterchill

    const Position WINTERCHILL_TANK_POSITION = { 5031.061f, -1784.521f, 1321.626f };
    std::unordered_map<ObjectGuid, bool> hasReachedWinterchillPosition;
    std::unordered_map<uint32, DeathAndDecayData> deathAndDecayPosition;

    DeathAndDecayData* GetActiveWinterchillDeathAndDecay(uint32 instanceId)
    {
        auto instanceIt = deathAndDecayPosition.find(instanceId);
        if (instanceIt == deathAndDecayPosition.end())
            return nullptr;

        const uint32 now = getMSTime();
        const uint32 elapsed = getMSTimeDiff(instanceIt->second.spawnTime, now);
        if (elapsed >= DEATH_AND_DECAY_REACQUIRE_DELAY)
        {
            deathAndDecayPosition.erase(instanceIt);
            return nullptr;
        }

        if (elapsed >= DEATH_AND_DECAY_DURATION)
            return nullptr;

        return &instanceIt->second;
    }

    bool IsInDeathAndDecay(Player* bot, float radius)
    {
        const uint32 instanceId = bot->GetMap()->GetInstanceId();
        Aura* aura = bot->GetAura(static_cast<uint32>(HyjalSummitSpells::SPELL_DEATH_AND_DECAY));
        if (aura)
        {
            DynamicObject* dynObj = aura->GetDynobjOwner();
            if (dynObj && dynObj->IsInWorld())
            {
                const uint32 now = getMSTime();
                auto instanceIt = deathAndDecayPosition.find(instanceId);
                if (instanceIt == deathAndDecayPosition.end() ||
                    getMSTimeDiff(instanceIt->second.spawnTime, now) >= DEATH_AND_DECAY_REACQUIRE_DELAY)
                {
                    deathAndDecayPosition[instanceId] =
                        DeathAndDecayData{ dynObj->GetPosition(), now };
                }
            }
        }

        DeathAndDecayData* data = GetActiveWinterchillDeathAndDecay(instanceId);
        if (!data)
            return false;

        return bot->GetExactDist2d(data->position) < radius;
    }

    // Anetheron

    const Position ANETHERON_TANK_POSITION =       { 5033.177f, -1765.996f, 1324.195f };
    const Position ANETHERON_E_INFERNAL_POSITION = { 5016.578f, -1800.233f, 1323.070f };
    const Position ANETHERON_W_INFERNAL_POSITION = { 5048.911f, -1722.164f, 1321.408f };
    std::unordered_map<ObjectGuid, bool> hasReachedAnetheronPosition;

    Player* GetInfernoTarget(Unit* anetheron)
    {
        if (!anetheron)
            return nullptr;

        Spell* spell = anetheron->GetCurrentSpell(CURRENT_GENERIC_SPELL);
        if (spell && spell->m_spellInfo->Id ==
            static_cast<uint32>(HyjalSummitSpells::SPELL_INFERNO))
        {
            Unit* spellTarget = spell->m_targets.GetUnitTarget();
            if (spellTarget && spellTarget->IsPlayer())
                return spellTarget->ToPlayer();
        }

        return nullptr;
    }

    const Position& GetClosestInfernalTankPosition(Player* bot)
    {
        const Position& east = ANETHERON_E_INFERNAL_POSITION;
        const Position& west = ANETHERON_W_INFERNAL_POSITION;
        return (bot->GetExactDist2d(east.GetPositionX(), east.GetPositionY()) <=
                bot->GetExactDist2d(west.GetPositionX(), west.GetPositionY())) ? east : west;
    }

    // Kaz'rogal

    const Position KAZROGAL_TANK_TRANSITION_POSITION = { 5528.792f, -2636.486f, 1481.293f };
    const Position KAZROGAL_TANK_FINAL_POSITION =      { 5511.514f, -2662.466f, 1480.288f };
    std::unordered_map<ObjectGuid, TankPositionState> kazrogalTankStep;
    std::unordered_map<ObjectGuid, bool> isBelowManaThreshold;

    TankPositionState GetKazrogalTankPositionState(PlayerbotAI* botAI, Player* bot)
    {
        Player* mainTank = GetGroupMainTank(botAI, bot);
        if (!mainTank)
            return TankPositionState::Unknown;

        auto it = kazrogalTankStep.find(mainTank->GetGUID());
        if (it != kazrogalTankStep.end())
            return it->second;

        return TankPositionState::Unknown;
    }

    // Azgalor

    const Position AZGALOR_TANK_TRANSITION_POSITION = { 5486.787f, -2696.215f, 1482.007f };
    const Position AZGALOR_TANK_FINAL_POSITION =      { 5496.379f, -2675.265f, 1481.053f };
    const Position AZGALOR_DOOMGUARD_POSITION =       { 5485.555f, -2731.659f, 1485.555f };
    std::unordered_map<ObjectGuid, TankPositionState> azgalorTankStep;
    std::unordered_map<uint32, RainOfFireData> rainOfFirePosition;

    RainOfFireData* GetActiveAzgalorRainOfFire(uint32 instanceId)
    {
        auto instanceIt = rainOfFirePosition.find(instanceId);
        if (instanceIt == rainOfFirePosition.end())
            return nullptr;

        const uint32 now = getMSTime();
        const uint32 elapsed = getMSTimeDiff(instanceIt->second.spawnTime, now);
        if (elapsed >= RAIN_OF_FIRE_REACQUIRE_DELAY)
        {
            rainOfFirePosition.erase(instanceIt);
            return nullptr;
        }

        if (elapsed >= RAIN_OF_FIRE_DURATION)
            return nullptr;

        return &instanceIt->second;
    }

    TankPositionState GetAzgalorTankPositionState(PlayerbotAI* botAI, Player* bot)
    {
        Player* mainTank = GetGroupMainTank(botAI, bot);
        if (!mainTank)
            return TankPositionState::Unknown;

        auto it = azgalorTankStep.find(mainTank->GetGUID());
        if (it != azgalorTankStep.end())
            return it->second;

        return TankPositionState::Unknown;
    }

    bool IsInRainOfFire(Player* bot, float radius)
    {
        RainOfFireData* data = GetActiveAzgalorRainOfFire(bot->GetMap()->GetInstanceId());
        if (!data)
            return false;

        return bot->GetExactDist2d(data->position) < radius;
    }

    bool AnyGroupMemberHasDoom(Player* bot)
    {
        if (Group* group = bot->GetGroup())
        {
            for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
            {
                Player* member = ref->GetSource();
                if (member &&
                    member->HasAura(static_cast<uint32>(HyjalSummitSpells::SPELL_DOOM)))
                    return true;
            }
        }

        return false;
    }

    // Archimonde

    const Position ARCHIMONDE_INITIAL_POSITION = { 5640.502f, -3421.238f, 1587.453f };
    std::unordered_map<uint32, AirBurstData> archimondeAirBurstTargets;
    std::unordered_map<uint32, std::vector<DoomfireTrailData>> doomfireTrails;
    std::unordered_map<ObjectGuid, uint32> doomfireLastSampleTime;

    AirBurstData* GetRecentArchimondeAirBurst(uint32 instanceId)
    {
        auto instanceIt = archimondeAirBurstTargets.find(instanceId);
        if (instanceIt == archimondeAirBurstTargets.end())
            return nullptr;

        constexpr uint32 airBurstReactionWindow = 2000;
        const uint32 now = getMSTime();
        if (getMSTimeDiff(instanceIt->second.castTime, now) >= airBurstReactionWindow)
        {
            archimondeAirBurstTargets.erase(instanceIt);
            return nullptr;
        }

        return &instanceIt->second;
    }
}
