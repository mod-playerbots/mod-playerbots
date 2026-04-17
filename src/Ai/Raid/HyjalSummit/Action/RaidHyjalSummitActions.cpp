/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "RaidHyjalSummitActions.h"
#include "RaidHyjalSummitHelpers.h"
#include "Playerbots.h"
#include "RaidBossHelpers.h"
#include "Timer.h"

using namespace HyjalSummitHelpers;

// General

bool HyjalSummitEraseTrackersAction::Execute(Event /*event*/)
{
    const ObjectGuid guid = bot->GetGUID();

    bool erased = false;
    if (botAI->IsTank(bot))
    {
        if (!AI_VALUE2(Unit*, "find target", "kaz'rogal") &&
            kazrogalTankStep.erase(guid) > 0)
            erased = true;

        if (!AI_VALUE2(Unit*, "find target", "azgalor") &&
            azgalorTankStep.erase(guid) > 0)
            erased = true;

        return erased;
    }
    else
    {
        if (!AI_VALUE2(Unit*, "find target", "rage winterchill") &&
            hasReachedWinterchillPosition.erase(guid) > 0)
            erased = true;

        if (!AI_VALUE2(Unit*, "find target", "anetheron") &&
            hasReachedAnetheronPosition.erase(guid) > 0)
            erased = true;

        if (!AI_VALUE2(Unit*, "find target", "kaz'rogal") &&
            isBelowManaThreshold.erase(guid) > 0)
            erased = true;

        if (!AI_VALUE2(Unit*, "find target", "archimonde") &&
            doomfireTrails.erase(bot->GetMap()->GetInstanceId()) > 0)
            erased = true;

        return erased;
    }
}

// Rage Winterchill

bool RageWinterchillMisdirectBossToMainTankAction::Execute(Event /*event*/)
{
    Unit* winterchill = AI_VALUE2(Unit*, "find target", "rage winterchill");
    if (!winterchill)
        return false;

    Player* mainTank = GetGroupMainTank(botAI, bot);
    if (!mainTank)
        return false;

    if (botAI->CanCastSpell("misdirection", mainTank))
        return botAI->CastSpell("misdirection", mainTank);

    if (bot->HasAura(static_cast<uint32>(HyjalSummitSpells::SPELL_MISDIRECTION)) &&
        botAI->CanCastSpell("steady shot", winterchill))
        return botAI->CastSpell("steady shot", winterchill);

    return false;
}

// Position is back towards the center of the base to give some more room to manuever
bool RageWinterchillMainTankPositionBossAction::Execute(Event /*event*/)
{
    Unit* winterchill = AI_VALUE2(Unit*, "find target", "rage winterchill");
    if (!winterchill)
        return false;

    if (bot->GetVictim() != winterchill)
        return Attack(winterchill);

    if (winterchill->GetVictim() == bot)
    {
        const Position& position = WINTERCHILL_TANK_POSITION;
        float distToPosition =
            bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());

        if (distToPosition > 4.0f)
        {
            float dX = position.GetPositionX() - bot->GetPositionX();
            float dY = position.GetPositionY() - bot->GetPositionY();
            float moveDist = std::min(10.0f, distToPosition);
            float moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
            float moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

            return MoveTo(HYJAL_SUMMIT_MAP_ID, moveX, moveY, position.GetPositionZ(), false,
                          false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
        }
    }

    return false;
}

// Spread ranged DPS in a circle initially--after the initial spread, movement is free
bool RageWinterchillSpreadRangedInCircleAction::Execute(Event /*event*/)
{
    RangedGroups groups = GetRangedGroups(botAI, bot);

    if (groups.healers.empty() && groups.rangedDps.empty())
        return false;

    const ObjectGuid guid = bot->GetGUID();

    if (!hasReachedWinterchillPosition[guid])
    {
        auto [botIndex, count] = GetBotCircleIndexAndCount(botAI, bot, groups);
        const float radius = botAI->IsHeal(bot) ? 25.0f : 35.0f;
        float angle = 0.0f;

        constexpr float arcSpan = 2.0f * M_PI;
        constexpr float arcCenter = 0.0f;
        constexpr float arcStart = arcCenter - arcSpan / 2.0f;

        angle = (count == 1) ? arcCenter :
            (arcStart + arcSpan * static_cast<float>(botIndex) / static_cast<float>(count - 1));

        const Position& position = WINTERCHILL_TANK_POSITION;
        float targetX = position.GetPositionX() + radius * std::cos(angle);
        float targetY = position.GetPositionY() + radius * std::sin(angle);

        float targetZ = bot->GetMapWaterOrGroundLevel(targetX, targetY, position.GetPositionZ());

        if (targetZ <= INVALID_HEIGHT)
            targetZ = position.GetPositionZ();

        bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bot->GetPositionX(), bot->GetPositionY(),
                                                       bot->GetPositionZ(), targetX, targetY,
                                                       targetZ, false);

        if (bot->GetExactDist(targetX, targetY, targetZ) > 2.0f)
        {
            return MoveTo(HYJAL_SUMMIT_MAP_ID, targetX, targetY, targetZ, false, false,
                          false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
        }
        else
        {
            hasReachedWinterchillPosition[guid] = true;
        }
    }

    return false;
}

// Anetheron

bool AnetheronMisdirectBossAndInfernalsToTanksAction::Execute(Event /*event*/)
{
    Unit* anetheron = AI_VALUE2(Unit*, "find target", "anetheron");
    if (!anetheron)
        return false;

    if (anetheron->GetHealthPct() > 95.0f)
    {
        Player* mainTank = GetGroupMainTank(botAI, bot);
        if (!mainTank)
            return false;

        if (botAI->CanCastSpell("misdirection", mainTank))
            return botAI->CastSpell("misdirection", mainTank);

        if (bot->HasAura(static_cast<uint32>(HyjalSummitSpells::SPELL_MISDIRECTION)) &&
            botAI->CanCastSpell("steady shot", anetheron))
            return botAI->CastSpell("steady shot", anetheron);
    }

    if (Unit* infernal = AI_VALUE2(Unit*, "find target", "towering infernal");
        infernal && infernal->GetHealthPct() > 50.0f)
    {
        Player* firstAssistTank = GetGroupAssistTank(botAI, bot, 0);
        if (!firstAssistTank)
            return false;

        if (botAI->CanCastSpell("misdirection", firstAssistTank))
            return botAI->CastSpell("misdirection", firstAssistTank);

        if (bot->HasAura(static_cast<uint32>(HyjalSummitSpells::SPELL_MISDIRECTION)) &&
            botAI->CanCastSpell("steady shot", infernal))
            return botAI->CastSpell("steady shot", infernal);
    }

    return false;
}

// Position is back towards the center of the base, near the crossroads
bool AnetheronMainTankPositionBossAction::Execute(Event /*event*/)
{
    Unit* anetheron = AI_VALUE2(Unit*, "find target", "anetheron");
    if (!anetheron)
        return false;

    MarkTargetWithSquare(bot, anetheron);
    SetRtiTarget(botAI, "square", anetheron);

    if (bot->GetVictim() != anetheron)
        return Attack(anetheron);

    if (anetheron->GetVictim() == bot)
    {
        const Position& position = ANETHERON_TANK_POSITION;
        float distToPosition =
            bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());

        if (distToPosition > 4.0f)
        {
            float dX = position.GetPositionX() - bot->GetPositionX();
            float dY = position.GetPositionY() - bot->GetPositionY();
            float moveDist = std::min(10.0f, distToPosition);
            float moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
            float moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

            return MoveTo(HYJAL_SUMMIT_MAP_ID, moveX, moveY, position.GetPositionZ(), false,
                          false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
        }
    }

    return false;
}

bool AnetheronSpreadRangedInCircleAction::Execute(Event /*event*/)
{
    RangedGroups groups = GetRangedGroups(botAI, bot);

    if (groups.healers.empty() && groups.rangedDps.empty())
        return false;

    const ObjectGuid guid = bot->GetGUID();

    if (!hasReachedAnetheronPosition[guid])
    {
        auto [botIndex, count] = GetBotCircleIndexAndCount(botAI, bot, groups);
        const float radius = botAI->IsHeal(bot) ? 27.0f : 34.0f;
        float angle = 0.0f;

        constexpr float arcSpan = M_PI * 2.0f;
        constexpr float arcCenter = 0.0f;
        constexpr float arcStart = arcCenter - arcSpan / 2.0f;

        angle = (count == 1) ? arcCenter :
            (arcStart + arcSpan * static_cast<float>(botIndex) / static_cast<float>(count - 1));

        const Position& position = ANETHERON_TANK_POSITION;

        float targetX = position.GetPositionX() + radius * std::sin(angle);
        float targetY = position.GetPositionY() + radius * std::cos(angle);

        float targetZ = bot->GetMapWaterOrGroundLevel(targetX, targetY, position.GetPositionZ());
        if (targetZ <= INVALID_HEIGHT)
            targetZ = position.GetPositionZ();

        bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bot->GetPositionX(), bot->GetPositionY(),
                                                       bot->GetPositionZ(), targetX, targetY,
                                                       targetZ, false);

        if (bot->GetExactDist(targetX, targetY, targetZ) > 2.0f)
        {
            return MoveTo(HYJAL_SUMMIT_MAP_ID, targetX, targetY, targetZ, false, false,
                          false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
        }
        else
        {
            hasReachedAnetheronPosition[guid] = true;
        }
    }
    else
    {
        constexpr float safeDistFromPlayer = 6.0f;
        constexpr uint32 minInterval = 2000;
        if (Unit* nearestPlayer = GetNearestPlayerInRadius(bot, safeDistFromPlayer))
            return FleePosition(nearestPlayer->GetPosition(), safeDistFromPlayer, minInterval);
    }

    return false;
}

// Run to the nearest of two Infernal tanking spots, East and West of Anetheron
bool AnetheronBringInfernalToInfernalTankAction::Execute(Event /*event*/)
{
    const Position& position = GetClosestInfernalTankPosition(bot);
    if (bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY()) > 2.0f)
    {
        return MoveTo(HYJAL_SUMMIT_MAP_ID, position.GetPositionX(), position.GetPositionY(),
                      position.GetPositionZ(), false, false, false, false,
                      MovementPriority::MOVEMENT_FORCED, true, false);
    }

    return false;
}

// Pick up the Infernal and bring it to the closest Infernal tanking position
bool AnetheronFirstAssistTankPickUpInfernalsAction::Execute(Event /*event*/)
{
    Unit* anetheron = AI_VALUE2(Unit*, "find target", "anetheron");
    if (!anetheron)
        return false;

    Player* infernoTarget = GetInfernoTarget(anetheron);
    if (infernoTarget && infernoTarget != bot)
    {
        float distToInfernoTarget = bot->GetExactDist2d(infernoTarget);
        if (distToInfernoTarget > 5.0f)
        {
            return MoveTo(HYJAL_SUMMIT_MAP_ID, infernoTarget->GetPositionX(),
                          infernoTarget->GetPositionY(), infernoTarget->GetPositionZ(),
                          false, false, false, false, MovementPriority::MOVEMENT_FORCED,
                          true, false);
        }
    }

    Unit* infernal = AI_VALUE2(Unit*, "find target", "towering infernal");
    if (!infernal)
        return false;

    MarkTargetWithDiamond(bot, infernal);
    SetRtiTarget(botAI, "diamond", infernal);

    if (bot->GetVictim() != infernal)
        return Attack(infernal);

    if ((infernoTarget && infernoTarget == bot) ||
        (infernal->GetVictim() == bot && bot->IsWithinMeleeRange(infernal)))
    {
        const Position& position = GetClosestInfernalTankPosition(bot);
        float distToPosition =
            bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());

        if (distToPosition > 3.0f)
        {
            float dX = position.GetPositionX() - bot->GetPositionX();
            float dY = position.GetPositionY() - bot->GetPositionY();
            float moveDist = std::min(5.0f, distToPosition);
            float moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
            float moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

            return MoveTo(HYJAL_SUMMIT_MAP_ID, moveX, moveY, position.GetPositionZ(), false,
                          false, false, true, MovementPriority::MOVEMENT_COMBAT, true, true);
        }
    }

    return false;
}

// Only nearbyish ranged DPS should attack Infernals
bool AnetheronAssignDpsPriorityAction::Execute(Event /*event*/)
{
    Unit* anetheron = AI_VALUE2(Unit*, "find target", "anetheron");
    if (!anetheron)
        return false;

    if (botAI->IsMelee(bot))
    {
        SetRtiTarget(botAI, "square", anetheron);

        if (bot->GetVictim() != anetheron)
            return Attack(anetheron);

        return false;
    }
    if (Unit* infernal = AI_VALUE2(Unit*, "find target", "towering infernal"))
    {
        constexpr float safeDistFromInfernal = 10.0f;
        constexpr uint32 minInterval = 0;
        if (infernal->GetVictim() != bot &&
            bot->GetDistance2d(infernal) < safeDistFromInfernal)
        {
            return FleePosition(infernal->GetPosition(), safeDistFromInfernal, minInterval);
        }

        if (anetheron->GetHealthPct() > 10.0f && botAI->IsRangedDps(bot) &&
            bot->GetDistance2d(infernal) < 50.0f)
        {
            if (Player* firstAssistTank = GetGroupAssistTank(botAI, bot, 0);
                !firstAssistTank || infernal->GetVictim() == firstAssistTank)
            {
                SetRtiTarget(botAI, "diamond", infernal);

                if (bot->GetTarget() != infernal->GetGUID())
                    return Attack(infernal);
            }
        }
    }
    else if (botAI->IsRangedDps(bot))
    {
        SetRtiTarget(botAI, "square", anetheron);

        if (bot->GetTarget() != anetheron->GetGUID())
            return Attack(anetheron);
    }

    return false;
}

// Kaz'rogal

bool KazrogalMisdirectBossToMainTankAction::Execute(Event /*event*/)
{
    Unit* kazrogal = AI_VALUE2(Unit*, "find target", "kaz'rogal");
    if (!kazrogal)
        return false;

    Player* mainTank = GetGroupMainTank(botAI, bot);
    if (!mainTank)
        return false;

    if (botAI->CanCastSpell("misdirection", mainTank))
        return botAI->CastSpell("misdirection", mainTank);

    if (bot->HasAura(static_cast<uint32>(HyjalSummitSpells::SPELL_MISDIRECTION)) &&
        botAI->CanCastSpell("steady shot", kazrogal))
        return botAI->CastSpell("steady shot", kazrogal);

    return false;
}

// Position is near the gate so the raid can get start on DPS ASAP
bool KazrogalMainTankPositionBossAction::Execute(Event /*event*/)
{
    Unit* kazrogal = AI_VALUE2(Unit*, "find target", "kaz'rogal");
    if (!kazrogal)
        return false;

    if (bot->GetVictim() != kazrogal)
        return Attack(kazrogal);

    if (kazrogal->GetVictim() == bot && bot->IsWithinMeleeRange(kazrogal))
    {
        const ObjectGuid guid = bot->GetGUID();
        TankPositionState state = kazrogalTankStep.count(guid) ?
            kazrogalTankStep[guid] : TankPositionState::MovingToTransition;

        constexpr float maxDistance = 2.0f;
        const Position& position = state == TankPositionState::MovingToTransition ?
            KAZROGAL_TANK_TRANSITION_POSITION : KAZROGAL_TANK_FINAL_POSITION;
        float distToPosition = bot->GetExactDist2d(position);

        if (distToPosition > maxDistance)
        {
            float dX = position.GetPositionX() - bot->GetPositionX();
            float dY = position.GetPositionY() - bot->GetPositionY();
            float moveDist = std::min(5.0f, distToPosition);
            float moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
            float moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

            return MoveTo(HYJAL_SUMMIT_MAP_ID, moveX, moveY,
                          position.GetPositionZ(), false, false, false, false,
                          MovementPriority::MOVEMENT_COMBAT, true, true);
        }

        if (state == TankPositionState::MovingToTransition && distToPosition <= maxDistance)
        {
            kazrogalTankStep[guid] = TankPositionState::MovingToFinal;
        }
        else if (state != TankPositionState::MovingToTransition &&
                 distToPosition <= maxDistance)
        {
            float orientation = atan2(kazrogal->GetPositionY() - bot->GetPositionY(),
                                      kazrogal->GetPositionX() - bot->GetPositionX());
            bot->SetFacingTo(orientation);
            kazrogalTankStep[guid] = TankPositionState::Positioned;
        }
    }

    return false;
}

// To spread cleave damage
bool KazrogalAssistTanksMoveInFrontOfBossAction::Execute(Event /*event*/)
{
    Player* mainTank = GetGroupMainTank(botAI, bot);
    if (!mainTank)
        return false;

    if (bot->GetExactDist2d(mainTank) > 4.0f)
    {
        return MoveTo(HYJAL_SUMMIT_MAP_ID, mainTank->GetPositionX(),
                      mainTank->GetPositionY(), bot->GetPositionZ(),
                      false, false, false, false,
                      MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    return false;
}

bool KazrogalSpreadRangedInArcAction::Execute(Event /*event*/)
{
    Unit* kazrogal = AI_VALUE2(Unit*, "find target", "kaz'rogal");
    if (!kazrogal)
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    std::vector<Player*> rangedMembers;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !botAI->IsRanged(member))
            continue;

        rangedMembers.push_back(member);
    }

    if (rangedMembers.empty())
        return false;

    size_t count = rangedMembers.size();
    auto findIt = std::find(rangedMembers.begin(), rangedMembers.end(), bot);
    size_t botIndex = (findIt != rangedMembers.end()) ?
        std::distance(rangedMembers.begin(), findIt) : 0;

    constexpr float arcSpan = M_PI / 3.0f;
    constexpr float arcCenter = 4.225f;
    constexpr float arcStart = arcCenter - arcSpan / 2.0f;

    constexpr float radius = 20.0f;
    float angle = (count == 1) ? arcCenter :
        (arcStart + arcSpan * static_cast<float>(botIndex) / static_cast<float>(count - 1));

    float targetX = kazrogal->GetPositionX() + radius * std::cos(angle);
    float targetY = kazrogal->GetPositionY() + radius * std::sin(angle);

    float targetZ = bot->GetMapWaterOrGroundLevel(targetX, targetY,
                                                  kazrogal->GetPositionZ());
    if (targetZ <= INVALID_HEIGHT)
        targetZ = kazrogal->GetPositionZ();

    bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bot->GetPositionX(), bot->GetPositionY(),
                                                   bot->GetPositionZ(), targetX, targetY,
                                                   targetZ, false);

    if (bot->GetExactDist2d(targetX, targetY) > 0.5f)
    {
        return MoveTo(HYJAL_SUMMIT_MAP_ID, targetX, targetY, targetZ, false, false,
                      false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    return false;
}

bool KazrogalLowManaBotTakeDefensiveMeasuresAction::Execute(Event /*event*/)
{
    if (bot->getClass() == CLASS_HUNTER)
    {
        if (!botAI->HasAura("aspect of the viper", bot) &&
            botAI->CanCastSpell("aspect of the viper", bot))
        {
            return botAI->CastSpell("aspect of the viper", bot);
        }
        return false;
    }
    else
    {
        if (bot->getClass() == CLASS_WARLOCK &&
            botAI->CanCastSpell("life tap", bot) &&
            botAI->CastSpell("life tap", bot))
        {
            return true;
        }

        if (bot->GetPower(POWER_MANA) <= 3200)
            isBelowManaThreshold.try_emplace(bot->GetGUID(), true);

        if (bot->HasAura(static_cast<uint32>(HyjalSummitSpells::SPELL_MARK_OF_KAZROGAL)) &&
            bot->GetPower(POWER_MANA) <= 1200)
        {
            if (bot->getClass() == CLASS_MAGE &&
                botAI->CanCastSpell("ice block", bot) &&
                botAI->CastSpell("ice block", bot))
            {
                return true;
            }
            else if (bot->getClass() == CLASS_PALADIN &&
                     botAI->CanCastSpell("divine shield", bot) &&
                     botAI->CastSpell("divine shield", bot))
            {
                return true;
            }
        }

        constexpr float safeDistance = 16.0f;

        Unit* nearestPlayer = GetNearestPlayerInRadius(bot, safeDistance);
        if (!nearestPlayer)
            return false;

        float currentDistance = bot->GetDistance2d(nearestPlayer);
        if (currentDistance < safeDistance)
        {
            Unit* kazrogal = AI_VALUE2(Unit*, "find target", "kaz'rogal");
            if (!kazrogal)
                return false;

            // MoveFromGroup will make the bot run very far if there is another bot
            // also running next to it; thus, we swap to MoveAway once there is sufficient
            // distance from the group so the parallel bots break away from each other
            if (bot->GetExactDist2d(kazrogal) > 42.0f)
                return MoveAway(nearestPlayer, safeDistance - currentDistance);
            else
                return MoveFromGroup(safeDistance);
        }
    }

    return false;
}

// Warlocks: Use Shadow Ward if Mark is applied and mana is <= 3000
// Paladins: Use Shadow Resistance Aura if Priest Shadow Protection is not up
bool KazrogalCastShadowProtectionSpellAction::Execute(Event /*event*/)
{
    if (bot->getClass() == CLASS_WARLOCK && bot->GetPower(POWER_MANA) <= 3000 &&
        botAI->CanCastSpell("shadow ward", bot))
        return botAI->CastSpell("shadow ward", bot);

    if (bot->getClass() == CLASS_PALADIN &&
        botAI->CanCastSpell("shadow resistance aura", bot))
        return botAI->CastSpell("shadow resistance aura", bot);

    return false;
}

// Azgalor

bool AzgalorMisdirectBossToMainTankAction::Execute(Event /*event*/)
{
    Unit* azgalor = AI_VALUE2(Unit*, "find target", "azgalor");
    if (!azgalor)
        return false;

    Player* mainTank = GetGroupMainTank(botAI, bot);
    if (!mainTank)
        return false;

    if (botAI->CanCastSpell("misdirection", mainTank))
        return botAI->CastSpell("misdirection", mainTank);

    if (bot->HasAura(static_cast<uint32>(HyjalSummitSpells::SPELL_MISDIRECTION)) &&
        botAI->CanCastSpell("steady shot", azgalor))
        return botAI->CastSpell("steady shot", azgalor);

    return false;
}

// Two-step move: back up toward the base, then move back toward the base entrance
// to turn Azgalor away from the raid
bool AzgalorMainTankPositionBossAction::Execute(Event /*event*/)
{
    Unit* azgalor = AI_VALUE2(Unit*, "find target", "azgalor");
    if (!azgalor)
        return false;

    MarkTargetWithStar(bot, azgalor);
    SetRtiTarget(botAI, "star", azgalor);

    if (bot->GetVictim() != azgalor)
        return Attack(azgalor);

    if (azgalor->GetVictim() == bot && bot->IsWithinMeleeRange(azgalor))
    {
        const ObjectGuid guid = bot->GetGUID();
        auto it = azgalorTankStep.try_emplace(
            guid, TankPositionState::MovingToTransition).first;
        TankPositionState state = it->second;

        constexpr float maxDistance = 2.0f;
        const Position& position = state == TankPositionState::MovingToTransition ?
            AZGALOR_TANK_TRANSITION_POSITION : AZGALOR_TANK_FINAL_POSITION;
        float distToPosition = bot->GetExactDist2d(position);

        if (distToPosition > maxDistance)
        {
            float dX = position.GetPositionX() - bot->GetPositionX();
            float dY = position.GetPositionY() - bot->GetPositionY();
            float moveDist = std::min(5.0f, distToPosition);
            float moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
            float moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

            return MoveTo(HYJAL_SUMMIT_MAP_ID, moveX, moveY,
                          position.GetPositionZ(), false, false, false, false,
                          MovementPriority::MOVEMENT_COMBAT, true, true);
        }

        if (state == TankPositionState::MovingToTransition && distToPosition <= maxDistance)
        {
            azgalorTankStep[guid] = TankPositionState::MovingToFinal;
        }
        else if (state != TankPositionState::MovingToTransition &&
                 distToPosition <= maxDistance)
        {
            float orientation = atan2(azgalor->GetPositionY() - bot->GetPositionY(),
                                      azgalor->GetPositionX() - bot->GetPositionX());
            bot->SetFacingTo(orientation);
            azgalorTankStep[guid] = TankPositionState::Positioned;
        }
    }

    return false;
}

bool AzgalorDisperseRangedAction::Execute(Event /*event*/)
{
    Unit* azgalor = AI_VALUE2(Unit*, "find target", "azgalor");
    if (!azgalor)
        return false;

    // Azgalor's hitbox is 8.8 yards
    TankPositionState tankState = GetAzgalorTankPositionState(botAI, bot);
    const float safeDistFromBoss =
        (tankState == TankPositionState::MovingToTransition ? 35.0f : 29.0f);
    constexpr uint32 minInterval = 0;

    if (bot->GetExactDist2d(azgalor) < safeDistFromBoss &&
        FleePosition(azgalor->GetPosition(), safeDistFromBoss, minInterval))
        return true;

    // Lesser Doomguard's hitbox is 3.75 yards
    Unit* doomguard = AI_VALUE2(Unit*, "find target", "lesser doomguard");
    constexpr float safeDistFromDoomguard = 14.0f;
    constexpr float safeDistFromPlayer = 5.0f;

    if (doomguard && bot->GetExactDist2d(doomguard) < safeDistFromDoomguard)
    {
        return FleePosition(doomguard->GetPosition(), safeDistFromDoomguard);
    }
    else if (!doomguard || bot->GetTarget() != doomguard->GetGUID())
    {
        Unit* nearestPlayer = GetNearestPlayerInRadius(bot, safeDistFromPlayer);
        if (nearestPlayer)
            return FleePosition(nearestPlayer->GetPosition(), safeDistFromPlayer);
    }

    return false;
}

bool AzgalorMeleeGetOutOfFireAction::Execute(Event /*event*/)
{
    Unit* azgalor = AI_VALUE2(Unit*, "find target", "azgalor");
    if (!azgalor)
        return false;

    constexpr uint32 RAIN_OF_FIRE_DURATION = 10000;
    uint32 now = getMSTime();

    auto instanceIt = rainOfFirePosition.find(bot->GetMap()->GetInstanceId());
    if (instanceIt == rainOfFirePosition.end())
        return false;

    auto& dynObjMap = instanceIt->second;
    for (auto it = dynObjMap.begin(); it != dynObjMap.end(); )
    {
        if (getMSTimeDiff(it->second.spawnTime, now) >= RAIN_OF_FIRE_DURATION)
            it = dynObjMap.erase(it);
        else
            ++it;
    }

    if (dynObjMap.empty())
        return false;

    bool inAnyRoF = false;
    for (auto const& [guid, data] : dynObjMap)
    {
        if (bot->GetExactDist2d(data.position) < 17.0f)
        {
            inAnyRoF = true;
            break;
        }
    }

    if (inAnyRoF)
        return MoveAway(azgalor, 5.0f);

    return true;
}

// Wait for the tank to get to the transition position (i.e., move in to attack as
// Azgalor turns away from the raid)
bool AzgalorWaitAtSafePositionAction::Execute(Event /*event*/)
{
    const Position& position = AZGALOR_DOOMGUARD_POSITION;
    botAI->Reset();
    return MoveTo(HYJAL_SUMMIT_MAP_ID, position.GetPositionX(), position.GetPositionY(),
                  position.GetPositionZ(), false, false, false, false,
                  MovementPriority::MOVEMENT_FORCED, true, false);
}

// The spot is between the paths leading from Thrall's keep
bool AzgalorMoveToDoomguardTankAction::Execute(Event /*event*/)
{
    const Position& position = AZGALOR_DOOMGUARD_POSITION;
    if (bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY()) > 5.0f)
    {
        return MoveTo(HYJAL_SUMMIT_MAP_ID, position.GetPositionX(), position.GetPositionY(),
                      position.GetPositionZ(), false, false, false, false,
                      MovementPriority::MOVEMENT_FORCED, true, false);
    }

    return false;
}

bool AzgalorFirstAssistTankPositionDoomguardAction::Execute(Event /*event*/)
{
    const Position& position = AZGALOR_DOOMGUARD_POSITION;
    float distToPosition =
        bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());

    float moveDist = 0.0f;
    bool shouldMove = false;
    bool moveBackwards = false;

    if (Unit* doomguard = AI_VALUE2(Unit*, "find target", "lesser doomguard"))
    {
        MarkTargetWithCircle(bot, doomguard);
        SetRtiTarget(botAI, "circle", doomguard);

        if (bot->GetVictim() != doomguard)
            return Attack(doomguard);

        if (doomguard->GetVictim() == bot && bot->IsWithinMeleeRange(doomguard) &&
            distToPosition > 3.0f)
        {
            moveDist = std::min(5.0f, distToPosition);
            shouldMove = true;
            moveBackwards = true;
        }
    }
    else if (distToPosition > 3.0f)
    {
        moveDist = std::min(10.0f, distToPosition);
        shouldMove = true;
        moveBackwards = false;
    }
    else
    {
        return true;
    }

    if (shouldMove)
    {
        float dX = position.GetPositionX() - bot->GetPositionX();
        float dY = position.GetPositionY() - bot->GetPositionY();
        float moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
        float moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

        return MoveTo(HYJAL_SUMMIT_MAP_ID, moveX, moveY, position.GetPositionZ(),
                      false, false, false, false, MovementPriority::MOVEMENT_COMBAT,
                      true, moveBackwards);
    }

    return false;
}

// Only nearbyish ranged DPS should attack Doomguards; 65 yards should get to the
// side of Azgalor but not bring in any ranged standing in front
bool AzgalorAssignDpsPriorityAction::Execute(Event /*event*/)
{
    Unit* azgalor = AI_VALUE2(Unit*, "find target", "azgalor");
    if (!azgalor)
        return false;

    if (azgalor->GetHealthPct() > 10.0f && botAI->IsRanged(bot))
    {
        if (Unit* doomguard = AI_VALUE2(Unit*, "find target", "lesser doomguard");
            doomguard && bot->GetDistance2d(doomguard) < 65.0f)
        {
            SetRtiTarget(botAI, "circle", doomguard);

            if (bot->GetTarget() != doomguard->GetGUID())
                return Attack(doomguard);
        }
    }
    else
    {
        SetRtiTarget(botAI, "star", azgalor);

        if (bot->GetVictim() != azgalor)
            return Attack(azgalor);
    }

    return false;
}

// Archimonde

bool ArchimondeMisdirectBossToMainTankAction::Execute(Event /*event*/)
{
    Unit* archimonde = AI_VALUE2(Unit*, "find target", "archimonde");
    if (!archimonde)
        return false;

    Player* mainTank = GetGroupMainTank(botAI, bot);
    if (!mainTank)
        return false;

    if (botAI->CanCastSpell("misdirection", mainTank))
        return botAI->CastSpell("misdirection", mainTank);

    if (bot->HasAura(static_cast<uint32>(HyjalSummitSpells::SPELL_MISDIRECTION)) &&
        botAI->CanCastSpell("steady shot", archimonde))
        return botAI->CastSpell("steady shot", archimonde);

    return false;
}

// Initially move Archimonde up the hill a bit to get space from the World Tree
bool ArchimondeMoveBossToInitialPositionAction::Execute(Event /*event*/)
{
    Unit* archimonde = AI_VALUE2(Unit*, "find target", "archimonde");
    if (!archimonde)
        return false;

    if (bot->GetVictim() != archimonde)
        return Attack(archimonde);

    if (archimonde->GetVictim() == bot && bot->IsWithinMeleeRange(archimonde) &&
        bot->GetHealthPct() > 50.0f)
    {
        const Position& position = ARCHIMONDE_INITIAL_POSITION;
        float distToPosition =
            bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());

        if (distToPosition > 3.0f)
        {
            float dX = position.GetPositionX() - bot->GetPositionX();
            float dY = position.GetPositionY() - bot->GetPositionY();
            float moveDist = std::min(5.0f, distToPosition);
            float moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
            float moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

            return MoveTo(HYJAL_SUMMIT_MAP_ID, moveX, moveY, position.GetPositionZ(),
                          false, false, false, false, MovementPriority::MOVEMENT_COMBAT,
                          true, true);
        }
    }

    return false;
}

bool ArchimondeCastFearImmunitySpellAction::Execute(Event /*event*/)
{
    if (bot->getClass() == CLASS_PRIEST)
        return CastFearWardOnMainTank();
    else
        return UseTremorTotemStrategy();
}

bool ArchimondeCastFearImmunitySpellAction::CastFearWardOnMainTank()
{
    Player* mainTank = GetGroupMainTank(botAI, bot);
    if (mainTank && botAI->CanCastSpell("fear ward", mainTank))
        return botAI->CastSpell("fear ward", mainTank);

    return false;
}

bool ArchimondeCastFearImmunitySpellAction::UseTremorTotemStrategy()
{
    if (!botAI->HasStrategy("tremor", BOT_STATE_COMBAT))
    {
        botAI->ChangeStrategy("+tremor", BOT_STATE_COMBAT);
        return botAI->HasStrategy("tremor", BOT_STATE_COMBAT);
    }

    return false;
}

// (1) Try to run away from the Air Burst target
// (2) At the beginning of the fight, spread ranged in anticipation of Air Burst
bool ArchimondeSpreadToAvoidAirBurstAction::Execute(Event /*event*/)
{
    Unit* archimonde = AI_VALUE2(Unit*, "find target", "archimonde");
    if (!archimonde)
        return false;

    if (archimonde->HasUnitState(UNIT_STATE_CASTING))
    {
        Spell* spell = archimonde->GetCurrentSpell(CURRENT_GENERIC_SPELL);
        if (spell && spell->m_spellInfo->Id ==
            static_cast<uint32>(HyjalSummitSpells::SPELL_AIR_BURST))
        {
            Player* mainTank = GetGroupMainTank(botAI, bot);
            if (mainTank && spell->m_targets.GetUnitTarget() == mainTank)
            {
                float currentDistance = bot->GetDistance2d(mainTank);
                constexpr float safeDistance = 14.0f;
                if (currentDistance < safeDistance)
                    return MoveAway(mainTank, safeDistance - currentDistance);
            }
        }
    }

    if (archimonde->GetHealthPct() < 90.0f)
        return false;

    constexpr float safeDistFromPlayer = 8.0f;
    constexpr uint32 minInterval = 2000;

    if (botAI->IsRanged(bot))
    {
        Unit* nearestPlayer = GetNearestPlayerInRadius(bot, safeDistFromPlayer);
        if (nearestPlayer &&
            FleePosition(nearestPlayer->GetPosition(), safeDistFromPlayer, minInterval))
            return true;
    }

    return false;
}

bool ArchimondeAvoidDoomfireAction::Execute(Event /*event*/)
{
    constexpr float dangerDist = 10.0f;
    constexpr uint32 trailDuration = 18000;

    uint32 instanceId = bot->GetMap()->GetInstanceId();
    uint32 now = getMSTime();

    auto it = doomfireTrails.find(instanceId);
    if (it == doomfireTrails.end() || it->second.empty())
        return false;

    it->second.erase(std::remove_if(it->second.begin(), it->second.end(),
        [now](const DoomfireTrailData& d)
        {
            return getMSTimeDiff(d.recordTime, now) > trailDuration;
        }), it->second.end());

    float totalDx = 0.0f, totalDy = 0.0f;
    for (auto const& data : it->second)
    {
        float d = bot->GetExactDist2d(data.position.GetPositionX(),
                                      data.position.GetPositionY());

        if (d < dangerDist && d > 0.0f)
        {
            float weight = (dangerDist - d) / dangerDist;
            totalDx += (bot->GetPositionX() - data.position.GetPositionX()) / d * weight;
            totalDy += (bot->GetPositionY() - data.position.GetPositionY()) / d * weight;
        }
    }

    if (totalDx != 0.0f || totalDy != 0.0f)
    {
        float norm = std::sqrt(totalDx * totalDx + totalDy * totalDy);
        float moveDist = std::min(norm * dangerDist, dangerDist);
        if (moveDist < 0.5f)
            return false;

        float targetX = bot->GetPositionX() + (totalDx / norm) * moveDist;
        float targetY = bot->GetPositionY() + (totalDy / norm) * moveDist;

        MovementPriority priority = botAI->IsHeal(bot) ?
            MovementPriority::MOVEMENT_COMBAT : MovementPriority::MOVEMENT_FORCED;

        bool backwards = botAI->IsMainTank(bot);

        return MoveTo(HYJAL_SUMMIT_MAP_ID, targetX, targetY, bot->GetPositionZ(),
                      false, false, false, false, priority, true, backwards);
    }

    return false;
}

bool ArchimondeRemoveDoomfireDotAction::Execute(Event /*event*/)
{
    if (botAI->CanCastSpell("ice block", bot))
        return botAI->CastSpell("ice block", bot);
    else if (botAI->CanCastSpell("cloak of shadows", bot))
        return botAI->CastSpell("cloak of shadows", bot);
    else if (botAI->CanCastSpell("divine shield", bot))
        return botAI->CastSpell("divine shield", bot);

    return false;
}
