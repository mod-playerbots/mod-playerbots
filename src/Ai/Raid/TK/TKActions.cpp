/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "TKActions.h"
#include "AiFactory.h"
#include "EncounterHelpers.h"
#include "EquipAction.h"
#include "ItemPackets.h"
#include "LootAction.h"
#include "LootObjectStack.h"
#include "MotionMaster.h"
#include "MoveSpline.h"
#include "ObjectAccessor.h"
#include "Playerbots.h"
#include "StatsWeightCalculator.h"
#include "TKHelpers.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <iterator>
#include <limits>
#include <list>

using namespace TkHelpers;
using namespace EncounterHelpers;

// General

bool TempestKeepResetEncounterStatesAction::Execute(Event /*event*/)
{
    uint32 const instanceId = bot->GetInstanceId();
    bool reset = false;

    if (!AI_VALUE2(Unit*, "find target", "alar"))
    {
        reset |= isAlarInPhase2.erase(instanceId) > 0;
        reset |= lastRebirthState.erase(instanceId) > 0;
    }

    if (!AI_VALUE2(Unit*, "find target", "void reaver"))
        reset |= voidReaverArcaneOrbs.erase(instanceId) > 0;

    if (!AI_VALUE2(Unit*, "find target", "kael'thas sunstrider"))
    {
        reset |= advisorDpsWaitTimer.erase(instanceId) > 0;

        // Clear stale falling movement flag that may linger if a bot dies while falling during
        // Kael's Gravity Lapse and then is not resurrected until after the encounter
        if (!bot->HasUnitMovementFlag(MOVEMENTFLAG_FALLING) || !bot->movespline->Finalized())
            return reset;

        float const floorZ = bot->GetMapHeight(
            bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(), true, MAX_FALL_DISTANCE);
        if (floorZ <= INVALID_HEIGHT || bot->GetPositionZ() - floorZ > 1.0f)
            return reset;

        bot->RemoveUnitMovementFlag(MOVEMENTFLAG_FALLING | MOVEMENTFLAG_FALLING_FAR);
        if (!bot->IsRooted())
            bot->SendMovementFlagUpdate();
    }

    return reset;
}

bool TempestKeepCastFearWardOnMainTankAction::Execute(Event /*event*/)
{
    constexpr uint32 fearWard = Id(TkSpells::SPELL_FEAR_WARD);
    Player* mainTank = GetGroupMainTank(bot);
    if (!mainTank || mainTank->HasAura(fearWard))
        return false;

    return botAI->CanCastSpell(fearWard, mainTank) && botAI->CastSpell(fearWard, mainTank);
}

// Trash

bool CrimsonHandCenturionCastPolymorphAction::Execute(Event /*event*/)
{
    Unit* target = nullptr;
    constexpr float searchRadius = 40.0f;
    std::list<Creature*> centurions;
    bot->GetCreatureListWithEntryInGrid(
        centurions, Id(TkNpcs::NPC_CRIMSON_HAND_CENTURION), searchRadius);

    for (Creature* centurion : centurions)
    {
        if (!centurion || !centurion->HasAura(Id(TkSpells::SPELL_ARCANE_FLURRY)) ||
            botAI->HasAura("polymorph", centurion))
        {
            continue;
        }

        if (!target || centurion->GetGUID() < target->GetGUID())
            target = centurion;
    }

    if (!target)
        return false;

    if (!botAI->CanCastSpell("polymorph", target))
        return false;

    return botAI->CastSpell("polymorph", target);
}

// Al'ar <Phoenix God>
// CombatReach is 15 yards

bool AlarMisdirectBossToMainTankAction::Execute(Event /*event*/)
{
    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar)
        return false;

    Player* mainTank = GetGroupMainTank(bot);
    if (!mainTank)
        return false;

    if (botAI->CanCastSpell("misdirection", mainTank))
        return botAI->CastSpell("misdirection", mainTank);

    if (!bot->HasAura(Id(TkSpells::SPELL_MISDIRECTION)))
        return false;

    return botAI->CanCastSpell("steady shot", alar) && botAI->CastSpell("steady shot", alar);
}

bool AlarBossTanksMoveBetweenPlatformsAction::Execute(Event event)
{
    bool const isFirstAlarTank = IsFirstAlarTank(bot);
    if (!isFirstAlarTank && !IsSecondAlarTank(bot))
        return false;

    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar)
        return false;

    if (AI_VALUE(Unit*, "current target") != alar)
        return Attack(alar);

    int8 alarPlatformIndex = GetAlarPlatformIndex(alar);
    int8 tankPlatformIndex; // Determine which platform the tank goes to based on Al'ar's platform
    if (isFirstAlarTank)
    {
        tankPlatformIndex =
            (alarPlatformIndex == PLATFORM_0_IDX || alarPlatformIndex == PLATFORM_3_IDX) ?
            PLATFORM_0_IDX : PLATFORM_2_IDX;
    }
    else // isSecondAlarTank
    {
        tankPlatformIndex =
            (alarPlatformIndex == PLATFORM_0_IDX || alarPlatformIndex == PLATFORM_1_IDX) ?
            PLATFORM_1_IDX : PLATFORM_3_IDX;
    }

    Position const& target = ALAR_TANK_PLATFORM_POSITIONS[tankPlatformIndex];
    if (bot->GetExactDist2d(target) <= 2.0f)
    {
        if (alar->GetVictim() != bot)
            return botAI->DoSpecificAction("taunt spell", event, true);

        return false;
    }

    return MoveTo(
        TK_MAP_ID, target.GetPositionX(), target.GetPositionY(), target.GetPositionZ(),
        false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
}

bool AlarMeleeDpsMoveBetweenPlatformsAction::Execute(Event /*event*/)
{
    if (!PlayerbotAI::IsMelee(bot) || IsFirstAlarTank(bot) || IsSecondAlarTank(bot) ||
        IsPrimaryEmberTank(bot))
    {
        return false;
    }

    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar || alar->GetHealthPct() <= 5.0f)
        return false;

    if (AI_VALUE(Unit*, "current target") != alar)
        return Attack(alar);

    int8 platformIndex = GetAlarPlatformIndex(alar);
    if (platformIndex == LOCATION_NONE)
        return false;

    Position const& target = ALAR_MELEE_DPS_PLATFORM_POSITIONS[platformIndex];

    if (bot->GetExactDist2d(target) <= 2.0f)
        return false;

    return MoveTo(
        TK_MAP_ID, target.GetPositionX(), target.GetPositionY(), target.GetPositionZ(),
        false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
}

bool AlarRangedAndEmberTankMoveUnderPlatformsAction::Execute(Event /*event*/)
{
    bool const isRanged = PlayerbotAI::IsRanged(bot);
    bool const isEmberTank = IsPrimaryEmberTank(bot);
    if (!isRanged && !isEmberTank)
        return false;

    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar)
        return false;

    int8 platformIndex = GetAlarPlatformIndex(alar);
    if (platformIndex == LOCATION_NONE)
        return false;

    Position const& position = ALAR_GROUND_POSITIONS[platformIndex];

    float distFromTarget = 0.0f;
    if (isRanged)
        distFromTarget = 8.0f;
    else if (isEmberTank && !AI_VALUE2(Unit*, "find target", "ember of al'ar"))
        distFromTarget = 20.0f;
    else
        return false;

    if (bot->GetExactDist2d(position) <= distFromTarget)
        return false;

    return MoveInside(
        TK_MAP_ID, position.GetPositionX(), position.GetPositionY(),
        position.GetPositionZ(), distFromTarget, MovementPriority::MOVEMENT_COMBAT);
}

bool AlarAssistTanksPickUpEmbersAction::Execute(Event event)
{
    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar)
        return false;

    if (!IsAlarInPhase2(alar->GetInstanceId()))
        return HandlePhase1Embers(alar);

    return HandlePhase2Embers(event);
}

// Embers will be tanked by only the second assist tank in Phase 1
bool AlarAssistTanksPickUpEmbersAction::HandlePhase1Embers(Unit* alar)
{
    if (!IsPrimaryEmberTank(bot))
        return false;

    Unit* ember = AI_VALUE2(Unit*, "find target", "ember of al'ar");
    if (!ember)
        return false;

    if (AI_VALUE(Unit*, "current target") != ember)
        return Attack(ember);

    if (!bot->IsWithinMeleeRange(ember))
    {
        return MoveTo(
            TK_MAP_ID, ember->GetPositionX(), ember->GetPositionY(), ember->GetPositionZ(),
            false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    if (ember->GetVictim() != bot)
        return false;

    int8 platformIndex = GetAlarPlatformIndex(alar);
    if (platformIndex == LOCATION_NONE)
        return false;

    Position const& position = ALAR_GROUND_POSITIONS[platformIndex];
    Position const& center = ALAR_POINT_MIDDLE;

    float dx = center.GetPositionX() - position.GetPositionX();
    float dy = center.GetPositionY() - position.GetPositionY();
    float distToCenter = position.GetExactDist2d(center);

    constexpr float moveDist = 26.0f;
    float targetX = position.GetPositionX() + (dx / distToCenter) * moveDist;
    float targetY = position.GetPositionY() + (dy / distToCenter) * moveDist;

    if (bot->GetExactDist2d(targetX, targetY) <= 1.0f)
        return false;

    return MoveTo(
        TK_MAP_ID, targetX, targetY, position.GetPositionZ(), false, false,
        false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
}

// One Ember will be tanked by the second assist tank in Phase 2, and the other by
// the main tank or first assist tank (whichever is not tanking Al'ar).
bool AlarAssistTanksPickUpEmbersAction::HandlePhase2Embers(Event const& event)
{
    auto const& [firstEmber, secondEmber] = GetTargetUnitPair(botAI, Id(TkNpcs::NPC_EMBER_OF_ALAR));

    Unit* ember = nullptr;
    if (IsPrimaryEmberTank(bot))
        ember = firstEmber;
    else if (GetSecondaryEmberTank(bot) == bot)
        ember = secondEmber;

    if (!ember)
        return false;

    if (AI_VALUE(Unit*, "current target") != ember)
        return Attack(ember);

    if (ember->GetVictim() != bot)
        return botAI->DoSpecificAction("taunt spell", event, true);

    // The Embers' Rebirth has a range of 15 yards (16.5y damage radius), though more space is
    // needed since this movement is tank-to-player.
    // Embers have a CombatReach of 3 yards, though the 20-yard distance is more of an arbitrarily
    // tested in-game distance.
    constexpr float safeDistance = 20.0f;
    if (!GetNearestNonTankPlayerInRadius(bot, safeDistance))
        return false;

    return MoveFromGroup(safeDistance);
}

bool AlarRangedDpsPrioritizeEmbersAction::Execute(Event /*event*/)
{
    auto const& [firstEmber, secondEmber] = GetTargetUnitPair(botAI, Id(TkNpcs::NPC_EMBER_OF_ALAR));
    Unit* ember = firstEmber;
    if (!ember && secondEmber)
        ember = secondEmber;

    Unit* target = nullptr;
    if (ember)
    {
        target = ember;
        constexpr float safeDistance = 20.0f;
        float const currentDistance = bot->GetExactDist2d(ember);
        if (currentDistance < safeDistance)
        {
            bot->CastStop();
            return MoveAway(ember, safeDistance - currentDistance);
        }
    }

    if (!target)
    {
        if (Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar"))
            target = alar;
    }

    if (AI_VALUE(Unit*, "current target") == target)
        return false;

    return Attack(target);
}

bool AlarJumpFromPlatformAction::Execute(Event /*event*/)
{
    if (bot->GetPositionZ() > ALAR_BALCONY_Z)
    {
        int8 closestPlatform;
        Position ground;
        GetClosestPlatformAndGround(bot->GetPosition(), closestPlatform, ground);

        bot->CastStop();
        return JumpTo(
            TK_MAP_ID, ground.GetPositionX(), ground.GetPositionY(), ground.GetPositionZ(),
            MovementPriority::MOVEMENT_FORCED);
    }

    constexpr float distFromPos = 10.0f;
    bool const isFirstAlarTank = IsFirstAlarTank(bot);
    bool const isSecondAlarTank = IsSecondAlarTank(bot);
    bool const isTank = PlayerbotAI::IsTank(bot);

    if (isFirstAlarTank) // Having no distance check on Al'ar tanks is intentional here.
    {
        return MoveTo(
            TK_MAP_ID, ALAR_SW_RAMP_BASE.GetPositionX(), ALAR_SW_RAMP_BASE.GetPositionY(),
            ALAR_SW_RAMP_BASE.GetPositionZ(), false, false, false, false,
            MovementPriority::MOVEMENT_FORCED, true, false);
    }

    if (isSecondAlarTank)
    {
        return MoveTo(
            TK_MAP_ID, ALAR_SE_RAMP_BASE.GetPositionX(), ALAR_SE_RAMP_BASE.GetPositionY(),
            ALAR_SE_RAMP_BASE.GetPositionZ(), false, false, false, false,
            MovementPriority::MOVEMENT_FORCED, true, false);
    }

    if (isTank && !isFirstAlarTank && !isSecondAlarTank && bot->GetExactDist2d(
        ALAR_POINT_MIDDLE.GetPositionX(), ALAR_POINT_MIDDLE.GetPositionY()) > distFromPos)
    {
        return MoveTo(
            TK_MAP_ID, ALAR_POINT_MIDDLE.GetPositionX(), ALAR_POINT_MIDDLE.GetPositionY(),
            ALAR_POINT_MIDDLE.GetPositionZ(), false, false, false, false,
            MovementPriority::MOVEMENT_FORCED, true, false);
    }

    if (!isTank && PlayerbotAI::IsMelee(bot) && bot->GetExactDist2d(
        ALAR_ROOM_S_CENTER.GetPositionX(), ALAR_ROOM_S_CENTER.GetPositionY()) > distFromPos)
    {
        return MoveInside(
            TK_MAP_ID, ALAR_ROOM_S_CENTER.GetPositionX(), ALAR_ROOM_S_CENTER.GetPositionY(),
            ALAR_ROOM_S_CENTER.GetPositionZ(), distFromPos, MovementPriority::MOVEMENT_FORCED);
    }

    if (PlayerbotAI::IsRanged(bot) && bot->GetExactDist2d( // Ranged
        ALAR_POINT_MIDDLE.GetPositionX(), ALAR_POINT_MIDDLE.GetPositionY()) > distFromPos)
    {
        return MoveInside(
            TK_MAP_ID, ALAR_POINT_MIDDLE.GetPositionX(), ALAR_POINT_MIDDLE.GetPositionY(),
            ALAR_POINT_MIDDLE.GetPositionZ(), distFromPos, MovementPriority::MOVEMENT_FORCED);
    }

    return false;
}

bool AlarMoveAwayFromRebirthAction::Execute(Event /*event*/)
{
    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar)
        return false;

    // Ranged/tanks wait until Al'ar actually "dies" to activate this P1->P2 transition action
    if (PlayerbotAI::IsRanged(bot) || PlayerbotAI::IsTank(bot))
    {
        Creature* alarCreature = alar->ToCreature();
        if (!alarCreature || alarCreature->GetReactState() != REACT_PASSIVE)
            return false;
    }

    // On the other hand, melee dps jumps off at 5% HP because TBC hates melee dps
    if (bot->GetPositionZ() > ALAR_BALCONY_Z)
    {
        int8 closestPlatform;
        Position position;
        GetClosestPlatformAndGround(bot->GetPosition(), closestPlatform, position);

        bot->CastStop();
        return JumpTo(
            TK_MAP_ID, position.GetPositionX(), position.GetPositionY(),
            position.GetPositionZ(), MovementPriority::MOVEMENT_FORCED);
    }

    constexpr float safeDistance = 35.0f;
    float const currentDistance = bot->GetExactDist2d(ALAR_ROOM_CENTER);
    if (currentDistance >= safeDistance)
        return false;

    return MoveAway(alar, safeDistance - currentDistance);
}

bool AlarSwapTanksOnBossAction::Execute(Event event)
{
    if (!IsFirstAlarTank(bot) && !IsSecondAlarTank(bot))
        return false;

    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar)
        return false;

    if (GetSecondaryEmberTank(bot) == bot)
        return false;

    if (AI_VALUE(Unit*, "current target") != alar)
        return Attack(alar);

    if (alar->GetVictim() == bot)
        return false;

    return botAI->DoSpecificAction("taunt spell", event, true);
}

bool AlarAvoidFlamePatchesAndDiveBombsAction::Execute(Event /*event*/)
{
    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar)
        return false;

    return AvoidFlamePatch() || HandleDiveBomb(alar);
}

bool AlarAvoidFlamePatchesAndDiveBombsAction::AvoidFlamePatch()
{
    constexpr float searchRadius = 40.0f;
    std::vector<Unit*> flamePatches = GetAllHazardTriggers(
        bot, Id(TkNpcs::NPC_FLAME_PATCH), searchRadius);

    constexpr float hazardRadius = 8.0f;

    for (Unit* flamePatch : flamePatches)
    {
        if (bot->GetExactDist2d(flamePatch) < hazardRadius)
        {
            Position safestPos = FindSafestNearbyPosition(bot, flamePatches, hazardRadius);
            bot->CastStop();
            return MoveTo(
                TK_MAP_ID, safestPos.GetPositionX(), safestPos.GetPositionY(),
                safestPos.GetPositionZ(), false, false, false, false,
                MovementPriority::MOVEMENT_FORCED, true, false);
        }
    }

    return false;
}

bool AlarAvoidFlamePatchesAndDiveBombsAction::HandleDiveBomb(Unit* alar)
{
    // Avoidance after Dive Bomb, before reapperance
    if (alar->HasAura(Id(TkSpells::SPELL_MODEL_INVISIBILITY)) ||
        alar->FindCurrentSpellBySpellId(Id(TkSpells::SPELL_REBIRTH_DIVE)))
    {
        constexpr float safeDistance = 25.0f;
        float const currentDistance = bot->GetExactDist2d(alar);
        if (currentDistance >= safeDistance)
            return false;

        bot->CastStop();
        return MoveAway(alar, safeDistance - currentDistance);
    }

    if (GetAlarCurrentLocationIndex(alar) != POINT_QUILL_OR_DIVE_IDX &&
        GetAlarDestinationLocationIndex(alar) != POINT_QUILL_OR_DIVE_IDX)
    {
        return false;
    }

    // Avoidance during Dive Bomb sequence
    constexpr float safeDistance = 10.0f;
    Player* nearestPlayer = GetNearestPlayerInRadius(bot, safeDistance);
    if (!nearestPlayer)
        return false;

    float const currentDistance = bot->GetExactDist2d(nearestPlayer);
    return MoveAway(nearestPlayer, safeDistance - currentDistance);
}

bool AlarManagePhaseTrackerAction::Execute(Event /*event*/)
{
    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar)
        return false;

    uint32 const instanceId = alar->GetInstanceId();
    bool const rebirthActive = alar->FindCurrentSpellBySpellId(Id(TkSpells::SPELL_REBIRTH_PHASE2));

    if (!IsAlarInPhase2(instanceId) && lastRebirthState[instanceId] && !rebirthActive)
    {
        isAlarInPhase2[instanceId] = true;
        return true;
    }

    lastRebirthState[instanceId] = rebirthActive;

    return false;
}

// Void Reaver
// CombatReach is 15 yards

bool VoidReaverTanksPositionBossAction::Execute(Event /*event*/)
{
    Unit* voidReaver = AI_VALUE2(Unit*, "find target", "void reaver");
    if (!voidReaver)
        return false;

    if (AI_VALUE(Unit*, "current target") != voidReaver)
        return Attack(voidReaver);

    if (voidReaver->GetVictim() != bot || !bot->IsWithinMeleeRange(voidReaver))
        return false;

    Position const& position = VOID_REAVER_TANK_POSITION;
    float const distToPosition = bot->GetExactDist2d(position);
    if (distToPosition <= 2.0f)
        return false;

    float const posX = position.GetPositionX();
    float const posY = position.GetPositionY();
    float const botX = bot->GetPositionX();
    float const botY = bot->GetPositionY();

    float const toPosX = posX - botX;
    float const toPosY = posY - botY;
    float const toBossX = voidReaver->GetPositionX() - botX;
    float const toBossY = voidReaver->GetPositionY() - botY;
    bool const backwards = (toPosX * toBossX + toPosY * toBossY) < 0.0f;

    float const maxMoveDist = backwards ? 2.25f : 3.5f;
    float const moveDist = std::min(maxMoveDist, distToPosition);
    float const moveX = botX + (toPosX / distToPosition) * moveDist;
    float const moveY = botY + (toPosY / distToPosition) * moveDist;

    return MoveTo(
        TK_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false,
        false, false, MovementPriority::MOVEMENT_COMBAT, true, backwards);
}

bool VoidReaverUseAggroDumpAbilityAction::Execute(Event /*event*/)
{
    static constexpr std::array spells = {
        "divine shield",
        "divine protection",
        "fade",
        "feign death",
        "ice block",
        "soulshatter",
        "vanish",
    };

    for (char const* spell : spells)
    {
        if (botAI->CanCastSpell(spell, bot) && botAI->CastSpell(spell, bot))
            return true;
    }

    return false;
}

bool VoidReaverKeepRangedInGoldilocksZoneAction::Execute(Event /*event*/)
{
    Unit* voidReaver = AI_VALUE2(Unit*, "find target", "void reaver");
    if (!voidReaver)
        return false;

    constexpr float minDistFromBoss = 38.5f; // 22.0f GetDistance2d()
    constexpr uint32 minInterval = 0;
    if (bot->GetExactDist2d(voidReaver) < minDistFromBoss)
        return FleePosition(voidReaver->GetPosition(), minDistFromBoss, minInterval);

    if (voidReaver->GetHealthPct() > 90.0f)
        return false;

    // Maintain small spread after pull to discourage clumping from avoiding orbs
    constexpr float minDistFromPlayer = 3.0f;
    Player* nearestPlayer = GetNearestPlayerInRadius(bot, minDistFromPlayer);
    return nearestPlayer && FleePosition(nearestPlayer->GetPosition(), minDistFromPlayer);
}

bool VoidReaverAvoidArcaneOrbAction::Execute(Event /*event*/)
{
    Unit* voidReaver = AI_VALUE2(Unit*, "find target", "void reaver");
    if (!voidReaver)
        return false;

    auto const it = voidReaverArcaneOrbs.find(bot->GetInstanceId());
    if (it == voidReaverArcaneOrbs.end() || it->second.empty())
        return false;

    uint32 const now = getMSTime();

    std::vector<Position> activeOrbs;
    bool inDanger = false;
    for (auto const& orb : it->second)
    {
        if (getMSTimeDiff(orb.castTime, now) > ARCANE_ORB_DURATION_MS)
            continue;

        activeOrbs.push_back(orb.destination);
        if (!inDanger && bot->GetExactDist2d(
                orb.destination.GetPositionX(),
                orb.destination.GetPositionY()) < ARCANE_ORB_SAFE_DISTANCE)
        {
            inDanger = true;
        }
    }

    if (!inDanger)
        return false;

    constexpr float searchStep = M_PI / 12.0f;
    constexpr float minSearchDist = 1.0f;
    constexpr float searchDistStep = 1.0f;
    constexpr float minDistFromBoss = 20.5f;
    constexpr float maxDistFromBoss = 28.5f;
    constexpr uint8 numAngles = 24;
    constexpr uint8 numDistSteps = 39;

    std::vector<Position> bestCandidates;
    float bestMoveDist = std::numeric_limits<float>::max();

    float const botX = bot->GetPositionX();
    float const botY = bot->GetPositionY();

    for (uint8 i = 0; i <= numDistSteps; ++i)
    {
        float const dist = minSearchDist + i * searchDistStep;
        if (dist > bestMoveDist)
            break;

        for (uint8 j = 0; j < numAngles; ++j)
        {
            float const angle = j * searchStep;
            float const x = botX + dist * std::cos(angle);
            float const y = botY + dist * std::sin(angle);

            float const distFromBoss = voidReaver->GetDistance2d(x, y);
            if (distFromBoss < minDistFromBoss || distFromBoss > maxDistFromBoss)
                continue;

            bool safeFromOrbs = true;
            for (auto const& orbPos : activeOrbs)
            {
                float const dx = x - orbPos.GetPositionX();
                float const dy = y - orbPos.GetPositionY();
                if (std::sqrt(dx * dx + dy * dy) < ARCANE_ORB_SAFE_DISTANCE)
                {
                    safeFromOrbs = false;
                    break;
                }
            }

            if (safeFromOrbs && dist <= bestMoveDist)
            {
                if (dist < bestMoveDist)
                {
                    bestCandidates.clear();
                    bestMoveDist = dist;
                }
                bestCandidates.push_back(Position(x, y, bot->GetPositionZ()));
            }
        }
    }

    bot->CastStop();

    if (!bestCandidates.empty())
    {
        Position const& chosen = bestCandidates[urand(0, bestCandidates.size() - 1)];
        return MoveTo(
            TK_MAP_ID, chosen.GetPositionX(), chosen.GetPositionY(), chosen.GetPositionZ(),
            false, false, false, true, MovementPriority::MOVEMENT_FORCED, true, false);
    }

    constexpr uint32 minInterval = 0;
    return FleePosition(activeOrbs[0], ARCANE_ORB_SAFE_DISTANCE, minInterval);
}

// High Astromancer Solarian

bool HighAstromancerSolarianMainTankPickUpBossAction::Execute(Event /*event*/)
{
    Unit* astromancer = AI_VALUE2(Unit*, "find target", "high astromancer solarian");
    if (!astromancer)
        return false;

    return AI_VALUE(Unit*, "current target") != astromancer && Attack(astromancer);
}

bool HighAstromancerSolarianMoveAwayFromGroupAction::Execute(Event /*event*/)
{
    constexpr float safeDistance = 15.0f;
    if (!GetNearestPlayerInRadius(bot, safeDistance))
        return false;

    bot->CastStop();
    return MoveFromGroup(safeDistance);
}

bool HighAstromancerSolarianTargetSolariumPriestsAction::Execute(Event /*event*/)
{
    auto const& priestsPair = GetTargetUnitPair(botAI, Id(TkNpcs::NPC_SOLARIUM_PRIEST));
    if (!priestsPair.first || !priestsPair.second)
        return false;

    if (PlayerbotAI::IsRanged(bot) && !AI_VALUE2(Unit*, "find target", "solarium agent"))
    {
        return AI_VALUE(Unit*, "current target") != priestsPair.first &&
            Attack(priestsPair.first);
    }

    // Split melee into two groups, one on each Solarium Priest
    Unit* targetPriest = AssignSolariumPriestsToMeleeBots(priestsPair, GetMeleeBots());
    if (!targetPriest)
        return false;

    return AI_VALUE(Unit*, "current target") != targetPriest && Attack(targetPriest);
}

std::vector<Player*> HighAstromancerSolarianTargetSolariumPriestsAction::GetMeleeBots()
{
    Group* group = bot->GetGroup();
    if (!group)
        return {};

    std::vector<Player*> meleeMembers;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || !GET_PLAYERBOT_AI(member))
            continue;

        if (PlayerbotAI::IsMelee(member) && !PlayerbotAI::IsMainTank(member))
            meleeMembers.push_back(member);
    }

    return meleeMembers;
}

Unit* HighAstromancerSolarianTargetSolariumPriestsAction::AssignSolariumPriestsToMeleeBots(
    std::pair<Unit*, Unit*> const& priestsPair, std::vector<Player*> const& meleeMembers)
{
    if (!priestsPair.first || !priestsPair.second || meleeMembers.empty())
        return nullptr;

    auto it = std::find(meleeMembers.begin(), meleeMembers.end(), bot);
    if (it == meleeMembers.end())
        return nullptr;

    size_t botIndex = std::distance(meleeMembers.begin(), it);
    size_t totalMelee = meleeMembers.size();

    if (totalMelee == 1)
        return priestsPair.first;

    size_t split = totalMelee / 2;

    if (botIndex < split)
        return priestsPair.first;

    return priestsPair.second;
}

// Kael'thas Sunstrider <Lord of the Blood Elves>

bool KaelthasSunstriderKiteThaladredAction::Execute(Event /*event*/)
{
    Unit* thaladred = AI_VALUE2(Unit*, "find target", "thaladred the darkener");
    if (!thaladred)
        return false;

    constexpr float safeDistance = 15.0f;
    float const currentDistance = bot->GetDistance2d(thaladred);
    if (currentDistance >= safeDistance)
        return false;

    bot->CastStop();
    return MoveAway(thaladred, safeDistance - currentDistance);
}

// Misdirect order: (1) Capernian, (2) Telonicus, (3) Capernian (again for good measure)
bool KaelthasSunstriderMisdirectAdvisorsToTanksAction::Execute(Event /*event*/)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    std::vector<Player*> hunters;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive() && member->getClass() == CLASS_HUNTER &&
            GET_PLAYERBOT_AI(member))
        {
            hunters.push_back(member);
        }

        if (hunters.size() >= 3)
            break;
    }

    int8 hunterIndex = -1;
    for (size_t i = 0; i < hunters.size(); ++i)
    {
        if (hunters[i] == bot)
        {
            hunterIndex = static_cast<int8>(i);
            break;
        }
    }
    if (hunterIndex == -1)
        return false;

    Unit* advisor = nullptr;
    Player* tank = nullptr;
    if (hunterIndex == 0 || hunterIndex == 2)
    {
        advisor = AI_VALUE2(Unit*, "find target", "grand astromancer capernian");
        tank = GetCapernianTank(bot);
    }
    else if (hunterIndex == 1)
    {
        advisor = AI_VALUE2(Unit*, "find target", "master engineer telonicus");
        tank = GetGroupAssistTank(bot, 0);
    }

    if (!advisor || advisor->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE) ||
        advisor->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE) || IsFeigningDeath(advisor))
    {
        return false;
    }

    if (!tank || !tank->IsAlive())
        return false;

    if (botAI->CanCastSpell("misdirection", tank))
        return botAI->CastSpell("misdirection", tank);

    if (!bot->HasAura(Id(TkSpells::SPELL_MISDIRECTION)))
        return false;

    return botAI->CanCastSpell("steady shot", advisor) && botAI->CastSpell("steady shot", advisor);
}

bool KaelthasSunstriderMeleeTanksPositionAdvisorsAction::Execute(Event /*event*/)
{
    Unit* advisor = nullptr;
    Position position;
    if (PlayerbotAI::IsMainTank(bot))
    {
        advisor = AI_VALUE2(Unit*, "find target", "lord sanguinar");
        position = SANGUINAR_TANK_POSITION;
    }
    else // PlayerbotAI::IsAssistTankOfIndex(bot, 0, false)
    {
        advisor = AI_VALUE2(Unit*, "find target", "master engineer telonicus");
        position = TELONICUS_TANK_POSITION;
    }

    if (!advisor)
        return false;

    if (AI_VALUE(Unit*, "current target") != advisor)
        return Attack(advisor);

    if (advisor->GetVictim() != bot || !bot->IsWithinMeleeRange(advisor))
        return false;

    float const distToPosition = bot->GetExactDist2d(position);
    if (distToPosition <= 2.0f)
        return false;

    float const posX = position.GetPositionX();
    float const posY = position.GetPositionY();
    float const botX = bot->GetPositionX();
    float const botY = bot->GetPositionY();

    float const toPosX = posX - botX;
    float const toPosY = posY - botY;
    float const toBossX = advisor->GetPositionX() - botX;
    float const toBossY = advisor->GetPositionY() - botY;
    bool const backwards = (toPosX * toBossX + toPosY * toBossY) < 0.0f;

    float const maxMoveDist = backwards ? 2.25f : 3.5f;
    float const moveDist = std::min(maxMoveDist, distToPosition);
    float const moveX = botX + (toPosX / distToPosition) * moveDist;
    float const moveY = botY + (toPosY / distToPosition) * moveDist;

    return MoveTo(
        TK_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_COMBAT, true, backwards);
}

bool KaelthasSunstriderWarlockTankPositionCapernianAction::Execute(Event /*event*/)
{
    Unit* capernian = AI_VALUE2(Unit*, "find target", "grand astromancer capernian");
    if (!capernian)
        return false;

    char const* const searingPain = "searing pain";

    if (AI_VALUE(Unit*, "current target") != capernian &&
        botAI->CanCastSpell(searingPain, capernian) && botAI->CastSpell(searingPain, capernian))
    {
        return true;
    }

    if (capernian->GetVictim() == bot)
    {
        constexpr float minDistance = 28.0f;
        float const currentDist = bot->GetDistance2d(capernian);
        if ((currentDist < minDistance) && MoveAway(capernian, minDistance - currentDist))
            return true;
    }

    return botAI->CanCastSpell(searingPain, capernian) &&
        botAI->CastSpell(searingPain, capernian);
}

bool KaelthasSunstriderSpreadAndMoveAwayFromCapernianAction::Execute(Event /*event*/)
{
    Unit* capernian = AI_VALUE2(Unit*, "find target", "grand astromancer capernian");
    if (!capernian)
        return false;

    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return false;

    uint32 const phase = GetKaelthasPhase(kaelthas);
    if (phase == PHASE_NONE)
        return false;

    if (PlayerbotAI::IsRanged(bot) && capernian->GetVictim() != bot)
        return RangedBotsDisperse(phase, capernian);

    if (PlayerbotAI::IsMelee(bot) && phase == PHASE_SINGLE_ADVISOR)
        return MeleeStayBackFromCapernian(capernian);

    return false;
}

bool KaelthasSunstriderSpreadAndMoveAwayFromCapernianAction::RangedBotsDisperse(
    uint32 phase, Unit* capernian)
{
    if (phase == PHASE_ALL_ADVISORS)
    {
        if (AI_VALUE2(Unit*, "find target", "thaladred the darkener"))
            return false;

        constexpr float safeDistance = 6.0f;
        Player* nearestPlayer = GetNearestPlayerInRadius(bot, safeDistance);
        if (!nearestPlayer)
            return false;

        return FleePosition(nearestPlayer->GetPosition(), safeDistance);
    }

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    std::vector<Player*> healers;
    std::vector<Player*> rangedDps;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !PlayerbotAI::IsRanged(member))
            continue;

        if (PlayerbotAI::IsHeal(member))
            healers.push_back(member);
        else
            rangedDps.push_back(member);
    }

    if (healers.empty() && rangedDps.empty())
        return false;

    size_t count = healers.size() + rangedDps.size();
    size_t botIndex = 0;
    float radius = 0.0f;
    float angle = 0.0f;

    // Spread is 90-degree arc for healers and 120-degree arc for ranged DPS
    float arcSpan = PlayerbotAI::IsHeal(bot) ? M_PI / 2.0f : 2.0f * M_PI / 3.0f;
    constexpr float arcCenter = 2.9f;
    float arcStart = arcCenter - arcSpan / 2.0f;

    // Capernian's CombatReach is 4.5y
    if (PlayerbotAI::IsHeal(bot))
    {
        auto findIt = std::find(healers.begin(), healers.end(), bot);
        botIndex = (findIt != healers.end()) ? std::distance(healers.begin(), findIt) : 0;
        radius = 42.0f; // 36 yards of actual distance
        count = healers.size();
    }
    else
    {
        auto findIt = std::find(rangedDps.begin(), rangedDps.end(), bot);
        botIndex = (findIt != rangedDps.end()) ? std::distance(rangedDps.begin(), findIt) : 0;
        radius = 34.0f; // 28 yards of actual distance
        count = rangedDps.size();
    }

    angle = (count == 1) ? arcCenter :
        (arcStart + arcSpan * static_cast<float>(botIndex) / static_cast<float>(count - 1));

    float targetX = capernian->GetPositionX() + radius * std::cos(angle);
    float targetY = capernian->GetPositionY() + radius * std::sin(angle);
    float targetZ = bot->GetPositionZ();

    if (bot->GetExactDist2d(targetX, targetY) <= 1.0f)
        return false;

    if (!bot->GetMap()->CheckCollisionAndGetValidCoords(
            bot, bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(),
            targetX, targetY, targetZ))
    {
        return false;
    }

    bot->CastStop();
    return MoveTo(
        TK_MAP_ID, targetX, targetY, targetZ, false, false, false, false,
        MovementPriority::MOVEMENT_FORCED, true, false);
}

bool KaelthasSunstriderSpreadAndMoveAwayFromCapernianAction::MeleeStayBackFromCapernian(
    Unit* capernian)
{
    // Main tank purposely stays in range to bait Conflagration in Phase 1
    /* if (PlayerbotAI::IsMainTank(bot))
    {
        constexpr float targetDist = 20.0f;
        float const angle = capernian->GetAngle(bot);
        float const targetX = capernian->GetPositionX() + std::cos(angle) * targetDist;
        float const targetY = capernian->GetPositionY() + std::sin(angle) * targetDist;

        return MoveTo(
            TK_MAP_ID, targetX, targetY, bot->GetPositionZ(), false, false,
            false, false, MovementPriority::MOVEMENT_FORCED, true, false);
    }
    else
    { */
        constexpr float safeDistance = 50.0f; // There's no need to be anywhere closeby
        float const currentDistance = bot->GetExactDist2d(capernian);
        if (currentDistance >= safeDistance)
            return true;

        bot->CastStop();
        return MoveAway(capernian, safeDistance - currentDistance);
    // }
}

bool KaelthasSunstriderHandleAdvisorRolesInPhase3Action::Execute(Event /*event*/)
{
    Position position;
    if (PlayerbotAI::IsAssistHealOfIndex(bot, 0, true))
        position = ADVISOR_HEAL_POSITION;
    else if (PlayerbotAI::IsMainTank(bot))
        position = SANGUINAR_WAITING_POSITION;
    else if (PlayerbotAI::IsAssistTankOfIndex(bot, 0, true))
        position = TELONICUS_WAITING_POSITION;
    else // Capernian Tank
        position = CAPERNIAN_WAITING_POSITION;

    if (bot->GetExactDist2d(position) <= 2.0f)
        return false;

    return MoveTo(
        TK_MAP_ID, position.GetPositionX(), position.GetPositionY(), position.GetPositionZ(),
        false, false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
}

bool KaelthasSunstriderReequipGearAction::Execute(Event event)
{
    return botAI->DoSpecificAction("equip upgrade", event, true);
}

bool KaelthasSunstriderAssignAdvisorDpsPriorityAction::Execute(Event /*event*/)
{
    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return false;

    uint32 const phase = GetKaelthasPhase(kaelthas);
    if (phase == PHASE_NONE)
        return false;

    bool const isPhase3 = phase == PHASE_ALL_ADVISORS;
    bool const isActiveCapernianTank = isPhase3 && bot->getClass() == CLASS_WARLOCK &&
        GetCapernianTank(bot) == bot;

    Unit* target = nullptr;

    // Target priority 1: Thaladred, except Capernian tank during all advisors phase
    Unit* thaladred = AI_VALUE2(Unit*, "find target", "thaladred the darkener");
    if (!isActiveCapernianTank && thaladred && !thaladred->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE) &&
        !IsFeigningDeath(thaladred))
    {
        target = thaladred;
        if (isPhase3 && MarkTargetWithSkull(bot, thaladred))
            return true;
    }

    // Target priority 2: Capernian for ranged only (excluding debuff hunter)
    Unit* capernian = AI_VALUE2(Unit*, "find target", "grand astromancer capernian");
    if (!target && capernian && !capernian->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE) &&
        !IsFeigningDeath(capernian) && PlayerbotAI::IsRangedDps(bot) &&
        !IsSanguinarDebuffHunter(bot))
    {
        target = capernian;
        if (isPhase3 && MarkTargetWithCross(bot, capernian))
            return true;
    }

    // Target priority 3: Sanguinar (debuff hunter and melee move here after Thaladred)
    Unit* sanguinar = AI_VALUE2(Unit*, "find target", "lord sanguinar");
    if (!target && sanguinar && !sanguinar->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE) &&
        !IsFeigningDeath(sanguinar))
    {
        target = sanguinar;
        if (isPhase3 && MarkTargetWithSkull(bot, sanguinar))
            return true;
    }

    // Target priority 4: Telonicus
    Unit* telonicus = AI_VALUE2(Unit*, "find target", "master engineer telonicus");
    if (!target && telonicus && !telonicus->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE) &&
        !IsFeigningDeath(telonicus))
    {
        target = telonicus;
        if (isPhase3 && MarkTargetWithSkull(bot, telonicus))
            return true;
    }

    if (!target)
        return false;

    if (AI_VALUE(Unit*, "current target") != target)
        return Attack(target);

    if (target != telonicus || telonicus->GetVictim() == bot)
        return false;

    // Melee DPS need to stay at max-ish melee range behind Telonicus to avoid bombs
    return MeleeDpsPositionOutsideBombRange(telonicus);
}

bool KaelthasSunstriderAssignAdvisorDpsPriorityAction::MeleeDpsPositionOutsideBombRange(
    Unit* telonicus)
{
    if (!PlayerbotAI::IsMelee(bot) || !PlayerbotAI::IsDps(bot))
        return false;

    float const desiredDist = bot->GetMeleeRange(telonicus) - 0.5f;
    float const behindAngle = Position::NormalizeOrientation(telonicus->GetOrientation() + M_PI);
    float const targetX = telonicus->GetPositionX() + desiredDist * std::cos(behindAngle);
    float const targetY = telonicus->GetPositionY() + desiredDist * std::sin(behindAngle);

    if (bot->GetExactDist2d(targetX, targetY) <= 0.25f)
        return false;

    return MoveTo(
        TK_MAP_ID, targetX, targetY, telonicus->GetPositionZ(), false, false,
        false, false, MovementPriority::MOVEMENT_FORCED, true, false);
}

bool KaelthasSunstriderManageAdvisorDpsTimerAction::Execute(Event /*event*/)
{
    static constexpr std::array advisorNames = {
        "grand astromancer capernian", "master engineer telonicus", "lord sanguinar", };

    bool advisorAtFullHp = false;
    for (char const* name : advisorNames)
    {
        Unit* advisor = AI_VALUE2(Unit*, "find target", name);
        if (!advisor)
            continue;

        if (advisor->GetHealth() == advisor->GetMaxHealth() &&
            !advisor->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE))
        {
            advisorAtFullHp = true;
            break;
        }
    }

    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return false;

    uint32 const instanceId = kaelthas->GetInstanceId();

    if (advisorAtFullHp)
    {
        advisorDpsWaitTimer[instanceId] = ADVISOR_DPS_WAIT_NOT_STARTED;
        return false;
    }

    auto it = advisorDpsWaitTimer.find(instanceId);
    if (it == advisorDpsWaitTimer.end() || it->second != ADVISOR_DPS_WAIT_NOT_STARTED)
        return false;

    it->second = getMSTime();
    return true;
}

bool KaelthasSunstriderAssignLegendaryWeaponDpsPriorityAction::Execute(Event /*event*/)
{
    Unit* axe = AI_VALUE2(Unit*, "find target", "devastation");
    Unit* mace = AI_VALUE2(Unit*, "find target", "cosmic infuser");
    Unit* dagger = AI_VALUE2(Unit*, "find target", "infinity blades");
    Unit* sword = AI_VALUE2(Unit*, "find target", "warp slicer");

    bool const isTank = PlayerbotAI::IsTank(bot);
    bool const isRangedDps = PlayerbotAI::IsRangedDps(bot);
    bool const isMeleeDps = PlayerbotAI::IsMelee(bot) && PlayerbotAI::IsDps(bot);

    // Priority 0: Everybody other than the main tank needs to stay away from the axe
    // But for assist tanks, move away only after getting aggro on the mace, dagger, or sword
    // Variable return allows failure to MoveAway not to exit the function
    bool didAvoidDevastation = false;
    if (axe && HandleDevastationAvoidance(axe, mace, dagger, sword, isTank, isMeleeDps))
        didAvoidDevastation = true;

    if (isTank)
        return didAvoidDevastation;

    constexpr float safeDistance = 12.0f;
    Unit* target = nullptr;
    bool targetTooCloseToAxe = isMeleeDps && axe && target &&
        target->GetDistance2d(axe) <= safeDistance;

    // Priority 1: Staff of Disintegration
    if (Unit* staff = AI_VALUE2(Unit*, "find target", "staff of disintegration"))
    {
        target = staff;
        if (MarkTargetWithSkull(bot, staff))
            return true;

        if (targetTooCloseToAxe)
            target = nullptr;
    }
    // Priority 2: Cosmic Infuser
    if (!target && mace)
    {
        target = mace;
        if (MarkTargetWithSkull(bot, mace))
            return true;

        if (targetTooCloseToAxe)
            target = nullptr;
    }
    // Priority 3: Netherstrand Longbow
    if (Unit* longbow = AI_VALUE2(Unit*, "find target", "netherstrand longbow");
        longbow && !target)
    {
        target = longbow;
        if (MarkTargetWithSkull(bot, longbow))
            return true;

        if (targetTooCloseToAxe)
            target = nullptr;
    }
    // Priority 4: Devastation - ranged only
    if (isRangedDps && !target && axe)
    {
        target = axe;
        if (MarkTargetWithCross(bot, axe))
            return true;
    }
    // Priority 5: Infinity Blades
    if (!target && dagger)
    {
        target = dagger;
        if (MarkTargetWithSkull(bot, dagger))
            return true;

        if (targetTooCloseToAxe)
            target = nullptr;
    }
    // Priority 6: Warp Slicer
    if (!target && sword)
    {
        target = sword;
        if (MarkTargetWithSkull(bot, sword))
            return true;

        if (targetTooCloseToAxe)
            target = nullptr;
    }
    // Priority 7: Phaseshift Bulwark
    if (Unit* shield = AI_VALUE2(Unit*, "find target", "phaseshift bulwark");
        shield && !target)
    {
        target = shield;
        if (MarkTargetWithSkull(bot, shield))
            return true;

        if (targetTooCloseToAxe)
            target = nullptr;
    }

    if (!target)
        return didAvoidDevastation;

    return didAvoidDevastation ||
        (AI_VALUE(Unit*, "current target") != target && Attack(target));
}

bool KaelthasSunstriderAssignLegendaryWeaponDpsPriorityAction::HandleDevastationAvoidance(
    Unit* axe, Unit* mace, Unit* dagger, Unit* sword, bool const isTank, bool const isMeleeDps)
{
    bool const hasAggroFromWeapon =
        (mace && mace->GetVictim() == bot) ||
        (dagger && dagger->GetVictim() == bot) ||
        (sword && sword->GetVictim() == bot);

    bool result = false;

    if (!isTank || hasAggroFromWeapon)
    {
        float const safeDistance = isTank ? 15.0f : 10.0f;
        float const currentDistance = bot->GetDistance2d(axe);
        if (currentDistance < safeDistance)
            result = MoveAway(axe, safeDistance - currentDistance);
    }

    if (isMeleeDps && AI_VALUE(Unit*, "current target") == axe)
    {
        // Just in case melee ends up on the axe despite the target exclusion...
        bot->AttackStop();
        bot->InterruptSpell(CURRENT_MELEE_SPELL);
        bot->CastStop();
        context->GetValue<Unit*>("current target")->Set(nullptr);
        bot->SetSelection(ObjectGuid());
    }

    return result;
}

bool KaelthasSunstriderMoveDevastationAwayAction::Execute(Event /*event*/)
{
    Unit* axe = AI_VALUE2(Unit*, "find target", "devastation");
    if (!axe)
        return false;

    if (AI_VALUE(Unit*, "current target") != axe)
        return Attack(axe);

    if (MarkTargetWithCross(bot, axe))
        return true;

    if (axe->GetVictim() != bot || !bot->IsWithinMeleeRange(axe))
        return false;

    constexpr float safeDistance = 13.0f;
    if (!GetNearestNonTankPlayerInRadius(bot, safeDistance))
        return false;

    return MoveFromGroup(safeDistance);
}

bool KaelthasSunstriderLootLegendaryWeaponsAction::Execute(Event /*event*/)
{
    static constexpr std::array weapons = {
        WeaponInfo{ TkNpcs::NPC_NETHERSTRAND_LONGBOW, TkItems::ITEM_NETHERSTRAND_LONGBOW },
        WeaponInfo{ TkNpcs::NPC_COSMIC_INFUSER, TkItems::ITEM_COSMIC_INFUSER },
        WeaponInfo{ TkNpcs::NPC_DEVASTATION, TkItems::ITEM_DEVASTATION },
        WeaponInfo{ TkNpcs::NPC_INFINITY_BLADES, TkItems::ITEM_INFINITY_BLADE },
        WeaponInfo{ TkNpcs::NPC_WARP_SLICER, TkItems::ITEM_WARP_SLICER },
        WeaponInfo{ TkNpcs::NPC_STAFF_OF_DISINTEGRATION, TkItems::ITEM_STAFF_OF_DISINTEGRATION },
        WeaponInfo{ TkNpcs::NPC_PHASESHIFT_BULWARK, TkItems::ITEM_PHASESHIFT_BULWARK },
    };

    for (auto const& weapon : weapons)
    {
        if (ShouldBotLootWeapon(weapon.npcEntry))
        {
            if (bot->HasItemCount(Id(weapon.itemId), 1, false))
            {
                EquipLegendaryWeapon(Id(weapon.itemId));
                continue;
            }

            return LootWeapon(Id(weapon.npcEntry), Id(weapon.itemId));
        }
    }

    return false;
}

bool KaelthasSunstriderLootLegendaryWeaponsAction::ShouldBotLootWeapon(TkNpcs weaponEntry)
{
    uint8 const tab = AiFactory::GetPlayerSpecTab(bot);

    bool const isDruid = bot->getClass() == CLASS_DRUID;
    bool const isHunter = bot->getClass() == CLASS_HUNTER;
    bool const isPaladin = bot->getClass() == CLASS_PALADIN;
    bool const isRogue = bot->getClass() == CLASS_ROGUE;
    bool const isShaman = bot->getClass() == CLASS_SHAMAN;

    bool const isDk = bot->getClass() == CLASS_DEATH_KNIGHT;
    bool const isFrostDk = (isDk && tab == DEATH_KNIGHT_TAB_FROST);

    bool const isWarrior = bot->getClass() == CLASS_WARRIOR;
    bool const isArmsWarrior = (isWarrior && tab == WARRIOR_TAB_ARMS);

    switch (weaponEntry)
    {
        case TkNpcs::NPC_NETHERSTRAND_LONGBOW: // Hunter
            return isHunter;

        case TkNpcs::NPC_COSMIC_INFUSER: // Healer
            return PlayerbotAI::IsHeal(bot);

        // Fury Warriors could use the axe, but their DPS is terrible at 70 so they're better off
        // looting the dagger to break MC (and DW 1H is better dps than Titan Grip at 70 anyway!?)
        case TkNpcs::NPC_DEVASTATION: // Arms Warrior, Blood/Unholy DK, Ret Paladin
            return isArmsWarrior || (isDk && !isFrostDk) || (isPaladin && PlayerbotAI::IsDps(bot));

        case TkNpcs::NPC_INFINITY_BLADES: // Rogue, Hunter, Fury/Prot Warrior, Frost DK, Enh Shaman
            return isRogue || isHunter || (isWarrior && !isArmsWarrior) || isFrostDk ||
                (isShaman && tab == SHAMAN_TAB_ENHANCEMENT);

        // Sublety will probably want to use the Sword, but the spec is currently unimplemented,
        // and what kind of madman is going to raid with Sub anyway?
        case TkNpcs::NPC_WARP_SLICER: // Prot Paladin, Frost DK, Combat Rogue
            return (isPaladin && PlayerbotAI::IsTank(bot)) || isFrostDk ||
                (isRogue && tab == ROGUE_TAB_COMBAT);

        case TkNpcs::NPC_STAFF_OF_DISINTEGRATION: // Dps caster, Feral Druid
            return (!isHunter && PlayerbotAI::IsRangedDps(bot)) ||
                (isDruid && tab == DRUID_TAB_FERAL);

        case TkNpcs::NPC_PHASESHIFT_BULWARK: // Prot Paladin/Warrior
            return (isWarrior || isPaladin) && PlayerbotAI::IsTank(bot);

        default:
            return false;
    }
}

bool KaelthasSunstriderLootLegendaryWeaponsAction::LootWeapon(uint32 weaponEntry, uint32 itemId)
{
    Creature* weapon = GetDeadLegendaryWeapon(botAI, weaponEntry);
    if (!weapon)
        return false;

    LootObject loot(bot, weapon->GetGUID());
    if (!loot.IsLootPossible(bot))
        return false;

    context->GetValue<LootObject>("loot target")->Set(loot);

    if (bot->GetDistance2d(weapon) > INTERACTION_DISTANCE - 1.0f)
    {
        float const targetDist = INTERACTION_DISTANCE - 2.0f;
        float const angle = weapon->GetAngle(bot);
        float const targetX = weapon->GetPositionX() + std::cos(angle) * targetDist;
        float const targetY = weapon->GetPositionY() + std::sin(angle) * targetDist;

        return MoveTo(
            TK_MAP_ID, targetX, targetY, bot->GetPositionZ(), false, false,
            false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    OpenLootAction open(botAI);
    bool opened = open.Execute(Event());
    if (!opened)
        return opened;

    if (bot->HasItemCount(itemId, 1, false))
        return false;

    bot->SetLootGUID(weapon->GetGUID());

    constexpr uint8 weaponIndex = 0;
    WorldPacket* packet = new WorldPacket(CMSG_AUTOSTORE_LOOT_ITEM, 1);
    *packet << weaponIndex;
    bot->GetSession()->QueuePacket(packet);

    return true;
}

bool KaelthasSunstriderLootLegendaryWeaponsAction::EquipLegendaryWeapon(uint32 itemId)
{
    Item* legendaryItem = nullptr;
    auto const checkSlot = [&](uint8 bag, uint8 slot)
    {
        Item* item = bot->GetItemByPos(bag, slot);
        if (item && item->GetEntry() == itemId)
        {
            legendaryItem = item;
            return true;
        }
        return false;
    };

    for (uint8 slot = EQUIPMENT_SLOT_START; slot < EQUIPMENT_SLOT_END; ++slot)
    {
        if (checkSlot(INVENTORY_SLOT_BAG_0, slot))
            break;
    }

    if (!legendaryItem)
    {
        for (uint8 slot = INVENTORY_SLOT_ITEM_START; slot < INVENTORY_SLOT_ITEM_END; ++slot)
        {
            if (checkSlot(INVENTORY_SLOT_BAG_0, slot))
                break;
        }
    }

    if (!legendaryItem)
    {
        for (uint8 bag = INVENTORY_SLOT_BAG_START; bag < INVENTORY_SLOT_BAG_END; ++bag)
        {
            if (Bag const* pBag = static_cast<Bag*>(bot->GetItemByPos(INVENTORY_SLOT_BAG_0, bag)))
            {
                for (uint32 slot = 0; slot < pBag->GetBagSize(); ++slot)
                {
                    if (checkSlot(bag, slot))
                        break;
                }
            }
            if (legendaryItem)
                break;
        }
    }

    if (!legendaryItem)
        return false;

    ItemTemplate const* proto = legendaryItem->GetTemplate();
    if (!proto)
        return false;

    if (proto->InventoryType == INVTYPE_NON_EQUIP)
        return false;

    uint8 dstSlot = EQUIPMENT_SLOT_MAINHAND;
    if (proto->InventoryType == INVTYPE_RANGED)
    {
        dstSlot = EQUIPMENT_SLOT_RANGED;
    }
    else if (proto->InventoryType == INVTYPE_SHIELD ||
        proto->InventoryType == INVTYPE_WEAPONOFFHAND)
    {
        dstSlot = EQUIPMENT_SLOT_OFFHAND;
    }

    // Infinity Blade prefers OH when MH already holds a legendary (so Combat Rogues and Frost DKs
    // will equip Warp Slicer MH, Infinity Blade OH)
    if (dstSlot == EQUIPMENT_SLOT_MAINHAND && itemId == Id(TkItems::ITEM_INFINITY_BLADE) &&
        bot->CanDualWield())
    {
        if (Item* mhItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND))
        {
            uint32 mhEntry = mhItem->GetEntry();
            if (mhEntry >= ITEM_LEGENDARY_WEAPON_MIN && mhEntry <= ITEM_LEGENDARY_WEAPON_MAX &&
                mhEntry != itemId)
            {
                dstSlot = EQUIPMENT_SLOT_OFFHAND;
            }
        }
    }

    Item* alreadyEquipped = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, dstSlot);
    if (alreadyEquipped && alreadyEquipped->GetEntry() == itemId)
        return false;

    bot->CastStop();

    bool ohCleared = false; // If a 2H is blocking the target OH slot, unequip the 2H first
    if (dstSlot == EQUIPMENT_SLOT_OFFHAND)
    {
        Item* mhItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
        if (mhItem && mhItem->GetTemplate()->InventoryType == INVTYPE_2HWEAPON)
        {
            uint16 const mhPos = (INVENTORY_SLOT_BAG_0 << 8) | EQUIPMENT_SLOT_MAINHAND;
            uint16 const srcPos = (legendaryItem->GetBagSlot() << 8) | legendaryItem->GetSlot();
            bot->SwapItem(mhPos, srcPos);
            ohCleared = true;
            return true;
        }
    }

    uint16 srcPos = (legendaryItem->GetBagSlot() << 8) | legendaryItem->GetSlot();
    uint16 const dstPos = (INVENTORY_SLOT_BAG_0 << 8) | dstSlot;

    if (!alreadyEquipped)
    {
        bot->SwapItem(srcPos, dstPos);
        return true;
    }

    bool const oldIs2H = alreadyEquipped->GetTemplate()->InventoryType == INVTYPE_2HWEAPON;
    bool const newIs2H = proto->InventoryType == INVTYPE_2HWEAPON;

    bot->SwapItem(srcPos, dstPos);

    if (((oldIs2H && !newIs2H && proto->InventoryType != INVTYPE_SHIELD) ||
         (!oldIs2H && newIs2H)) && bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND))
    {
        uint16 const ohPos = (INVENTORY_SLOT_BAG_0 << 8) | EQUIPMENT_SLOT_OFFHAND;
        for (uint8 bpSlot = INVENTORY_SLOT_ITEM_START; bpSlot < INVENTORY_SLOT_ITEM_END; ++bpSlot)
        {
            if (!bot->GetItemByPos(INVENTORY_SLOT_BAG_0, bpSlot))
            {
                bot->SwapItem(ohPos, (INVENTORY_SLOT_BAG_0 << 8) | bpSlot);
                ohCleared = true;
                break;
            }
        }
    }

    // If using a 2H before equipping a 1H legendary, try to equip the best OH from the inventory
    if (!ohCleared || bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND))
        return true;

    Item* mhItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
    if (mhItem && mhItem->GetTemplate()->InventoryType == INVTYPE_2HWEAPON)
        return true;

    StatsWeightCalculator calculator(bot);
    calculator.SetItemSetBonus(false);
    calculator.SetOverflowPenalty(false);

    Item* bestOH = nullptr;
    float bestScore = 0.0f;

    auto const scanSlots = [&](uint8 bag, uint8 start, uint8 end)
    {
        for (uint8 slot = start; slot < end; ++slot)
        {
            Item* item = bot->GetItemByPos(bag, slot);
            if (!item || item == legendaryItem)
                continue;

            ItemTemplate const* itemProto = item->GetTemplate();
            if (!itemProto)
                continue;

            uint8 const invType = itemProto->InventoryType;
            if (invType != INVTYPE_WEAPONOFFHAND && invType != INVTYPE_SHIELD &&
                invType != INVTYPE_HOLDABLE && invType != INVTYPE_WEAPON)
            {
                continue;
            }

            if (invType == INVTYPE_WEAPONMAINHAND)
                continue;

            if (bot->CanUseItem(itemProto) != EQUIP_ERR_OK)
                continue;

            float const score = calculator.CalculateItem(
                itemProto->ItemId, item->GetItemRandomPropertyId());
            if (score > bestScore)
            {
                bestScore = score;
                bestOH = item;
            }
        }
    };

    scanSlots(INVENTORY_SLOT_BAG_0, INVENTORY_SLOT_ITEM_START, INVENTORY_SLOT_ITEM_END);
    for (uint8 bag = INVENTORY_SLOT_BAG_START; bag < INVENTORY_SLOT_BAG_END; ++bag)
    {
        if (Bag const* pBag = static_cast<Bag*>(bot->GetItemByPos(INVENTORY_SLOT_BAG_0, bag)))
            scanSlots(bag, 0, pBag->GetBagSize());
    }

    if (bestOH)
    {
        WorldPacket ohPacket(CMSG_AUTOEQUIP_ITEM_SLOT, 2);
        ohPacket << bestOH->GetGUID() << uint8(EQUIPMENT_SLOT_OFFHAND);

        WorldPackets::Item::AutoEquipItemSlot ohNicePacket(std::move(ohPacket));
        ohNicePacket.Read();
        bot->GetSession()->HandleAutoEquipItemSlotOpcode(ohNicePacket);
    }

    return true;
}

bool KaelthasSunstriderUseLegendaryWeaponsAction::Execute(Event /*event*/)
{
    if (bot->getClass() == CLASS_HUNTER)
        return UseNetherstrandLongbow();

    if (bot->getClass() != CLASS_DRUID && PlayerbotAI::IsTank(bot))
        return UsePhaseshiftBulwark();

    return UseStaffOfDisintegration();
}

bool KaelthasSunstriderUseLegendaryWeaponsAction::UsePhaseshiftBulwark()
{
    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas || !kaelthas->HasAura(Id(TkSpells::SPELL_SHOCK_BARRIER)))
        return false;

    if (bot->HasAura(Id(TkSpells::SPELL_ARCANE_BARRIER)))
        return false;

    Item* offHand = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
    if (!offHand || offHand->GetEntry() != Id(TkItems::ITEM_PHASESHIFT_BULWARK))
        return false;

    if (bot->CanUseItem(offHand) != EQUIP_ERR_OK)
        return false;

    return UseEquippedItemWithPacket(offHand);
}

bool KaelthasSunstriderUseLegendaryWeaponsAction::UseStaffOfDisintegration()
{
    if (bot->HasAura(Id(TkSpells::SPELL_MENTAL_PROTECTION_FIELD)))
        return false;

    Item* mainHand = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
    if (!mainHand || mainHand->GetEntry() != Id(TkItems::ITEM_STAFF_OF_DISINTEGRATION))
        return false;

    if (bot->CanUseItem(mainHand) != EQUIP_ERR_OK)
        return false;

    return UseEquippedItemWithPacket(mainHand);
}

bool KaelthasSunstriderUseLegendaryWeaponsAction::UseNetherstrandLongbow()
{
    if (bot->HasItemCount(Id(TkItems::ITEM_NETHER_SPIKES), 1, false))
        return false;

    Item* ranged = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_RANGED);
    if (!ranged || ranged->GetEntry() != Id(TkItems::ITEM_NETHERSTRAND_LONGBOW))
        return false;

    if (bot->CanUseItem(ranged) != EQUIP_ERR_OK)
        return false;

    return UseEquippedItemWithPacket(ranged);
}

bool KaelthasSunstriderUseLegendaryWeaponsAction::UseEquippedItemWithPacket(Item* item)
{
    if (!item || bot->CanUseItem(item) != EQUIP_ERR_OK || bot->IsNonMeleeSpellCast(true))
        return false;

    uint8 bagIndex = item->GetBagSlot();
    uint8 slot = item->GetSlot();
    uint8 cast_count = 1;
    ObjectGuid item_guid = item->GetGUID();
    uint32 glyphIndex = 0;
    uint8 castFlags = 0;
    uint32 spellId = 0;

    for (uint8 i = 0; i < MAX_ITEM_PROTO_SPELLS; ++i)
    {
        if (item->GetTemplate()->Spells[i].SpellId > 0 &&
            item->GetTemplate()->Spells[i].SpellTrigger == ITEM_SPELLTRIGGER_ON_USE)
        {
            spellId = item->GetTemplate()->Spells[i].SpellId;
            break;
        }
    }

    if (!spellId)
        return false;

    WorldPacket packet(CMSG_USE_ITEM);
    packet << bagIndex << slot << cast_count << spellId << item_guid << glyphIndex << castFlags;

    uint32 targetFlag = TARGET_FLAG_UNIT;
    packet << targetFlag << bot->GetPackGUID();

    bot->GetSession()->HandleUseItemOpcode(packet);
    return true;
}

bool KaelthasSunstriderMainTankPositionBossAction::Execute(Event /*event*/)
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return false;

    if (PlayerbotAI::IsMainTank(bot) && AI_VALUE(Unit*, "current target") != kaelthas)
        return Attack(kaelthas);

    if (kaelthas->GetVictim() != bot || !bot->IsWithinMeleeRange(kaelthas))
        return false;

    Position const& position = KAELTHAS_TANK_POSITION;
    float const distToPosition = bot->GetExactDist2d(position);
    if (distToPosition <= 4.0f)
        return false;

    float const posX = position.GetPositionX();
    float const posY = position.GetPositionY();
    float const botX = bot->GetPositionX();
    float const botY = bot->GetPositionY();

    float const toPosX = posX - botX;
    float const toPosY = posY - botY;
    float const toBossX = kaelthas->GetPositionX() - botX;
    float const toBossY = kaelthas->GetPositionY() - botY;
    bool const backwards = (toPosX * toBossX + toPosY * toBossY) < 0.0f;

    float const maxMoveDist = backwards ? 2.25f : 3.5f;
    float const moveDist = std::min(maxMoveDist, distToPosition);
    float const moveX = botX + (toPosX / distToPosition) * moveDist;
    float const moveY = botY + (toPosY / distToPosition) * moveDist;

    return MoveTo(
        TK_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false,
        false, false, MovementPriority::MOVEMENT_COMBAT, true, backwards);
}

bool KaelthasSunstriderAvoidFlameStrikeAction::Execute(Event /*event*/)
{
    constexpr float searchRadius = 40.0f;
    std::vector<Unit*> flameStrikes = GetAllHazardTriggers(
        bot, Id(TkNpcs::NPC_FLAME_STRIKE_TRIGGER), searchRadius);

    if (flameStrikes.empty())
        return false;

    constexpr float hazardRadius = 12.0f;
    bool inDanger = false;
    for (Unit* flameStrike : flameStrikes)
    {
        if (bot->GetExactDist2d(flameStrike) < hazardRadius)
        {
            inDanger = true;
            break;
        }
    }

    if (!inDanger)
        return false;

    Position safestPos = FindSafestNearbyPosition(bot, flameStrikes, hazardRadius);

    bot->CastStop();
    return MoveTo(
        TK_MAP_ID, safestPos.GetPositionX(), safestPos.GetPositionY(), safestPos.GetPositionZ(),
        false, false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
}

bool KaelthasSunstriderHandlePhoenixesAndEggsAction::Execute(Event /*event*/)
{
    if (PlayerbotAI::IsAssistTankOfIndex(bot, 0, true) ||
        PlayerbotAI::IsAssistTankOfIndex(bot, 1, true))
    {
        return AssistTanksPickUpPhoenixes();
    }

    return NonTanksDestroyEggsAndAvoidPhoenixes();
}

bool KaelthasSunstriderHandlePhoenixesAndEggsAction::AssistTanksPickUpPhoenixes()
{
    std::vector<Unit*> phoenixes;
    for (auto const& targetGuid : AI_VALUE(GuidVector, "possible targets no los"))
    {
        Unit* target = botAI->GetUnit(targetGuid);
        if (target && target->GetEntry() == Id(TkNpcs::NPC_PHOENIX))
            phoenixes.push_back(target);
    }

    if (phoenixes.empty())
        return false;

    std::sort(phoenixes.begin(), phoenixes.end(),
        [](Unit* first, Unit* second) { return first->GetGUID() < second->GetGUID(); });

    Unit* targetPhoenix = phoenixes[0];
    if (!PlayerbotAI::IsAssistTankOfIndex(bot, 0, true) && phoenixes.size() >= 2)
        targetPhoenix = phoenixes[1];

    if (!targetPhoenix)
        return false;

    if (AI_VALUE(Unit*, "current target") != targetPhoenix)
        return Attack(targetPhoenix);

    if (targetPhoenix->GetVictim() != bot)
        return false;

    constexpr float safeDistance = 12.0f;
    if (!GetNearestNonTankPlayerInRadius(bot, safeDistance))
        return false;

    return MoveFromGroup(safeDistance);
}

bool KaelthasSunstriderHandlePhoenixesAndEggsAction::NonTanksDestroyEggsAndAvoidPhoenixes()
{
    if (Unit* phoenix = AI_VALUE2(Unit*, "find target", "phoenix"))
    {
        constexpr float safeDistance = 15.0f;
        float const currentDistance = bot->GetExactDist2d(phoenix);
        if (currentDistance < safeDistance)
            return MoveAway(phoenix, safeDistance - currentDistance);
    }

    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return false;

    Unit* target = kaelthas;

    if (!kaelthas->HasAura(Id(TkSpells::SPELL_SHOCK_BARRIER)))
    {
        if (Creature* egg = GetPhoenixEgg(bot))
            target = egg;
    }

    return AI_VALUE(Unit*, "current target") != target && Attack(target);
}

bool KaelthasSunstriderBreakMindControlAction::Execute(Event /*event*/)
{
    Player* mcTarget = nullptr;
    float closestDist = std::numeric_limits<float>::max();

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member == bot)
            continue;

        if (member->HasAura(Id(TkSpells::SPELL_KAELTHAS_MIND_CONTROL)))
        {
            float distToMcMember = bot->GetExactDist2d(member);
            if (distToMcMember < closestDist)
            {
                closestDist = distToMcMember;
                mcTarget = member;
            }
        }
    }

    if (!mcTarget)
        return false;

    if (!bot->IsWithinMeleeRange(mcTarget))
    {
        return MoveTo(
            TK_MAP_ID, mcTarget->GetPositionX(), mcTarget->GetPositionY(), mcTarget->GetPositionZ(),
            false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    char const* spell = nullptr;
    switch (bot->getClass())
    {
        case CLASS_DEATH_KNIGHT:
            spell = "blood strike";
            break;

        case CLASS_HUNTER:
            spell = "wing clip";
            break;

        case CLASS_ROGUE:
            spell = (AiFactory::GetPlayerSpecTab(bot) == ROGUE_TAB_COMBAT) ?
                "shiv" : "sinister strike";
            break;

        case CLASS_SHAMAN:
            spell = "stormstrike";
            break;

        case CLASS_WARRIOR:
            spell = "hamstring";
            break;

        default:
            return false;
    }

    if (!spell)
        return false;

    return botAI->CanCastSpell(spell, mcTarget) && botAI->CastSpell(spell, mcTarget);
}

bool KaelthasSunstriderSpreadOutInMidairAction::Execute(Event /*event*/)
{
    if (!bot->HasAura(Id(TkSpells::SPELL_GRAVITY_LAPSE)))
        return DropToGround();

    return HoverAndSpread();
}

bool KaelthasSunstriderSpreadOutInMidairAction::DropToGround()
{
    if (bot->HasUnitMovementFlag(MOVEMENTFLAG_FALLING))
    {
        if (!bot->movespline->Finalized())
            return false;

        bot->RemoveUnitMovementFlag(MOVEMENTFLAG_FALLING | MOVEMENTFLAG_FALLING_FAR);

        if (!bot->IsRooted())
            bot->SendMovementFlagUpdate();

        return true;
    }

    if (!bot->movespline->Finalized())
        return false;

    float const x = bot->GetPositionX();
    float const y = bot->GetPositionY();
    float const floorZ = bot->GetMapHeight(x, y, bot->GetPositionZ(), true, MAX_FALL_DISTANCE);

    if (floorZ <= INVALID_HEIGHT)
        return false;

    float const heightAboveFloor = bot->GetPositionZ() - floorZ;
    if (heightAboveFloor <= 1.0f && !bot->IsFlying() && !bot->CanFly())
        return false;

    if (bot->IsFlying() || bot->CanFly())
    {
        bot->RemoveUnitMovementFlag(
            MOVEMENTFLAG_CAN_FLY | MOVEMENTFLAG_DISABLE_GRAVITY | MOVEMENTFLAG_FLYING);

        if (!bot->IsRooted())
            bot->SendMovementFlagUpdate();
    }

    if (heightAboveFloor <= 0.5f)
        return false;

    bot->GetMotionMaster()->Clear();
    bot->GetMotionMaster()->MoveFall();

    return true;
}

bool KaelthasSunstriderSpreadOutInMidairAction::HoverAndSpread()
{
    if (!bot->IsFlying() || !bot->CanFly())
    {
        bot->AddUnitMovementFlag(
            MOVEMENTFLAG_CAN_FLY | MOVEMENTFLAG_DISABLE_GRAVITY | MOVEMENTFLAG_FLYING);

        if (!bot->IsRooted())
            bot->SendMovementFlagUpdate();
    }

    MotionMaster* mm = bot->GetMotionMaster();
    if (mm->GetMotionSlotType(MOTION_SLOT_CONTROLLED) != NULL_MOTION_TYPE)
        mm->MovementExpiredOnSlot(MOTION_SLOT_CONTROLLED);

    auto const roll = [](uint32 value, uint32 salt)
    {
        uint32 hash = value + salt;
        hash ^= hash >> 16;
        hash *= 0x85EBCA6Bu;
        hash ^= hash >> 13;
        hash *= 0xC2B2AE35u;
        hash ^= hash >> 16;
        return (hash >> 8) / static_cast<float>(1 << 24);
    };

    Aura* aura = bot->GetAura(Id(TkSpells::SPELL_GRAVITY_LAPSE));
    if (!aura)
        return false;

    uint32 const seed = bot->GetGUID().GetCounter() ^ static_cast<uint32>(aura->GetApplyTime());

    constexpr float minHoverHeight = 5.0f;
    constexpr float maxHoverHeight = 35.0f;
    constexpr uint32 heightSalt = 1u;
    float const desiredHeight =
        minHoverHeight + roll(seed, heightSalt) * (maxHoverHeight - minHoverHeight);

    constexpr int32 minReactionMs = 250;
    constexpr int32 maxReactionMs = 1600;
    constexpr uint32 reactionSalt = 2u;
    int32 const reactionDelayMs = minReactionMs + static_cast<int32>(roll(seed, reactionSalt) *
        (maxReactionMs - minReactionMs));

    if (aura->GetMaxDuration() - aura->GetDuration() < reactionDelayMs)
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    constexpr float maxAimBias = 0.20f;
    constexpr uint32 aimSalt = 3u;
    float const aimBias = (roll(seed, aimSalt) * 2.0f - 1.0f) * maxAimBias;

    float const botX = bot->GetPositionX();
    float const botY = bot->GetPositionY();
    float const botZ = bot->GetPositionZ();

    constexpr float safeDistance = 18.0f;
    float pushX = 0.0f;
    float pushY = 0.0f;
    float pushZ = 0.0f;
    float closestDist = std::numeric_limits<float>::max();

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member == bot || !member->IsAlive())
            continue;

        float const distToMember = bot->GetExactDist(member);
        closestDist = std::min(closestDist, distToMember);
        if (distToMember >= safeDistance)
            continue;

        float const violation = safeDistance - distToMember;

        if (distToMember < 0.1f)
        {
            float const bearing = bot->GetAngle(member) + M_PI;
            pushX += std::cos(bearing) * violation;
            pushY += std::sin(bearing) * violation;
            continue;
        }

        pushX += (botX - member->GetPositionX()) / distToMember * violation;
        pushY += (botY - member->GetPositionY()) / distToMember * violation;
        pushZ += (botZ - member->GetPositionZ()) / distToMember * violation;
    }

    float const floorZ = bot->GetMapHeight(botX, botY, botZ, true, MAX_FALL_DISTANCE);
    if (floorZ <= INVALID_HEIGHT)
        return false;

    float const cosBias = std::cos(aimBias);
    float const sinBias = std::sin(aimBias);
    float const biasedPushX = pushX * cosBias - pushY * sinBias;
    pushY = pushX * sinBias + pushY * cosBias;
    pushX = biasedPushX;

    constexpr float heightPull = 0.35f;
    pushZ += (desiredHeight - (botZ - floorZ)) * heightPull;

    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return false;

    constexpr float containmentRadius = 40.0f;
    float const anchorX = kaelthas->GetPositionX();
    float const anchorY = kaelthas->GetPositionY();
    float const anchorDist = bot->GetExactDist2d(anchorX, anchorY);

    if (anchorDist > containmentRadius)
    {
        float const pull = anchorDist - containmentRadius;
        pushX += (anchorX - botX) / anchorDist * pull;
        pushY += (anchorY - botY) / anchorDist * pull;
    }

    constexpr float chainDangerDistance = 15.0f;
    constexpr float minMoveDistance = 4.0f;
    if (closestDist > chainDangerDistance &&
        std::sqrt(pushX * pushX + pushY * pushY + pushZ * pushZ) < minMoveDistance)
    {
        return false;
    }

    constexpr float stepFraction = 0.4f;
    float targetX = botX + pushX * stepFraction;
    float targetY = botY + pushY * stepFraction;
    float targetZ = botZ + pushZ * stepFraction;

    float const targetFloorZ = bot->GetMapHeight(
        targetX, targetY, targetZ, true, MAX_FALL_DISTANCE);
    if (targetFloorZ <= INVALID_HEIGHT)
        return false;

    targetZ = std::clamp(targetZ, targetFloorZ + minHoverHeight, targetFloorZ + maxHoverHeight);

    if (!bot->GetMap()->CheckCollisionAndGetValidCoords(
            bot, botX, botY, botZ, targetX, targetY, targetZ, false))
    {
        return false;
    }

    if (bot->GetExactDist(targetX, targetY, targetZ) <= 1.0f)
        return false;

    return MoveTo(
        TK_MAP_ID, targetX, targetY, targetZ, false, false, false, true,
        MovementPriority::MOVEMENT_FORCED, false, false);
}
