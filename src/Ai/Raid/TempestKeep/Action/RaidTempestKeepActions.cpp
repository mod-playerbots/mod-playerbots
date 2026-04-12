#include "RaidTempestKeepActions.h"
#include "RaidTempestKeepHelpers.h"
#include "RaidTempestKeepKaelthasBossAI.h"
#include "AiFactory.h"
#include "EquipAction.h"
#include "LootAction.h"
#include "LootObjectStack.h"
#include "ObjectAccessor.h"
#include "Playerbots.h"
#include "RaidBossHelpers.h"

using namespace TempestKeepHelpers;

// Trash

bool CrimsonHandCenturionCastPolymorphAction::Execute(Event /*event*/)
{
    Unit* centurion = AI_VALUE2(Unit*, "find target", "crimson hand centurion");
    if (!centurion)
        return false;

    if (centurion->GetHealth() == centurion->GetMaxHealth() &&
        !botAI->HasAura("polymorph", centurion) &&
        botAI->CanCastSpell("polymorph", centurion))
    {
        return botAI->CastSpell("polymorph", centurion);
    }
    else if (centurion->HasAura(SPELL_ARCANE_FLURRY))
    {
        botAI->Reset();
        return botAI->CastSpell("polymorph", centurion);
    }

    return false;
}

// Al'ar <Phoenix God>

bool AlarMisdirectBossToMainTankAction::Execute(Event /*event*/)
{
    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar)
        return false;

    Player* mainTank = GetGroupMainTank(botAI, bot);
    if (mainTank && botAI->CanCastSpell("misdirection", mainTank))
        return botAI->CastSpell("misdirection", mainTank);

    if (bot->HasAura(SPELL_MISDIRECTION) && botAI->CanCastSpell("steady shot", alar))
        return botAI->CastSpell("steady shot", alar);

    return false;
}

bool AlarBossTanksMoveBetweenPlatformsAction::Execute(Event /*event*/)
{
    if (!botAI->IsMainTank(bot) && !botAI->IsAssistTankOfIndex(bot, 0, true))
        return false;

    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar)
        return false;

    MarkTargetWithStar(bot, alar);
    SetRtiTarget(botAI, "star", alar);

    int8 locationIndex = GetAlarCurrentLocationIndex(alar);
    if (locationIndex == LOCATION_NONE)
    {
        Position dest;
        locationIndex = GetAlarDestinationLocationIndex(alar, dest);
    }

    if (botAI->IsMainTank(bot))
        return PositionMainTank(alar, locationIndex);
    else
        return PositionAssistTank(alar, locationIndex);
}

bool AlarBossTanksMoveBetweenPlatformsAction::PositionMainTank(
    Unit* alar, int8 locationIndex)
{
    if (locationIndex >= PLATFORM_0_IDX && locationIndex <= PLATFORM_3_IDX)
    {
        const Position& target =
            (locationIndex == PLATFORM_0_IDX || locationIndex == PLATFORM_3_IDX)
                ? PLATFORM_POSITIONS[0] : PLATFORM_POSITIONS[2];

        if (bot->GetExactDist2d(target.GetPositionX(), target.GetPositionY()) > 5.0f)
        {
            return MoveTo(TEMPEST_KEEP_MAP_ID, target.GetPositionX(), target.GetPositionY(),
                          target.GetPositionZ(), false, false, false, false,
                          MovementPriority::MOVEMENT_COMBAT, true, false);
        }
        else if ((locationIndex == PLATFORM_0_IDX || locationIndex == PLATFORM_2_IDX) &&
                 bot->GetTarget() != alar->GetGUID())
                 return Attack(alar);
    }

    return false;
}

bool AlarBossTanksMoveBetweenPlatformsAction::PositionAssistTank(
    Unit* alar, int8 locationIndex)
{
    if (locationIndex >= PLATFORM_0_IDX && locationIndex <= PLATFORM_3_IDX)
    {
        const Position& target =
            (locationIndex == PLATFORM_0_IDX || locationIndex == PLATFORM_1_IDX)
                ? PLATFORM_POSITIONS[1] : PLATFORM_POSITIONS[3];

        if (bot->GetExactDist2d(target.GetPositionX(), target.GetPositionY()) > 5.0f)
        {
            return MoveTo(TEMPEST_KEEP_MAP_ID, target.GetPositionX(), target.GetPositionY(),
                          target.GetPositionZ(), false, false, false, false,
                          MovementPriority::MOVEMENT_COMBAT, true, false);
        }
        else if ((locationIndex == PLATFORM_1_IDX || locationIndex == PLATFORM_3_IDX) &&
                 bot->GetTarget() != alar->GetGUID())
                 return Attack(alar);
    }

    return false;
}

bool AlarMeleeDpsMoveBetweenPlatformsAction::Execute(Event /*event*/)
{
    if (!botAI->IsMelee(bot) || !botAI->IsDps(bot))
        return false;

    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar)
        return false;

    SetRtiTarget(botAI, "star", alar);

    int8 locationIndex = GetAlarCurrentLocationIndex(alar);
    if (locationIndex == LOCATION_NONE)
    {
        Position dest;
        locationIndex = GetAlarDestinationLocationIndex(alar, dest);
    }

    if (locationIndex >= PLATFORM_0_IDX && locationIndex <= PLATFORM_3_IDX)
    {
        const Position& target = PLATFORM_POSITIONS[locationIndex];

        if (bot->GetExactDist2d(target.GetPositionX(), target.GetPositionY()) > 5.0f)
        {
            return MoveTo(TEMPEST_KEEP_MAP_ID, target.GetPositionX(), target.GetPositionY(),
                          target.GetPositionZ(), false, false, false, false,
                          MovementPriority::MOVEMENT_COMBAT, true, false);
        }

        if (bot->GetTarget() != alar->GetGUID())
            return Attack(alar);
    }

    return false;
}

bool AlarRangedAndEmberTankMoveUnderPlatformsAction::Execute(Event /*event*/)
{
    if (!botAI->IsRanged(bot) && !botAI->IsAssistTankOfIndex(bot, 1, true))
        return false;

    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar)
        return false;

    int8 locationIndex = GetAlarCurrentLocationIndex(alar);
    if (locationIndex == LOCATION_NONE)
    {
        Position dest;
        locationIndex = GetAlarDestinationLocationIndex(alar, dest);
    }

    if (locationIndex >= PLATFORM_0_IDX && locationIndex <= PLATFORM_3_IDX)
    {
        const Position& groundTarget = GROUND_POSITIONS[locationIndex];

        constexpr float distRangedFromTarget = 8.0f;
        constexpr float distTankFromTarget = 20.0f;
        if (botAI->IsRanged(bot) && bot->GetExactDist2d(
            groundTarget.GetPositionX(), groundTarget.GetPositionY()) > distRangedFromTarget)
        {
            return MoveInside(TEMPEST_KEEP_MAP_ID, groundTarget.GetPositionX(),
                              groundTarget.GetPositionY(), groundTarget.GetPositionZ(),
                              distRangedFromTarget, MovementPriority::MOVEMENT_COMBAT);
        }
        else if (botAI->IsAssistTankOfIndex(bot, 1, true) &&
                 !AI_VALUE2(Unit*, "find target", "ember of al'ar") && bot->GetExactDist2d(
                 groundTarget.GetPositionX(), groundTarget.GetPositionY()) > distTankFromTarget)
        {
            return MoveInside(TEMPEST_KEEP_MAP_ID, groundTarget.GetPositionX(),
                              groundTarget.GetPositionY(), groundTarget.GetPositionZ(),
                              distTankFromTarget, MovementPriority::MOVEMENT_COMBAT);
        }
    }

    return false;
}

bool AlarAssistTanksPickUpEmbersAction::Execute(Event /*event*/)
{
    if (!botAI->IsTank(bot))
        return false;

    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar)
        return false;

    if (!isAlarInPhase2[alar->GetMap()->GetInstanceId()])
        return HandlePhase1Embers(alar);
    else
        return HandlePhase2Embers(alar);
}

// Embers will be tanked by only the second assist tank in Phase 1
bool AlarAssistTanksPickUpEmbersAction::HandlePhase1Embers(Unit* alar)
{
    if (!botAI->IsAssistTankOfIndex(bot, 1, true))
        return false;

    if (Unit* ember = AI_VALUE2(Unit*, "find target", "ember of al'ar"))
    {
        MarkTargetWithSquare(bot, ember);
        SetRtiTarget(botAI, "square", ember);

        if (bot->GetTarget() != ember->GetGUID())
            return Attack(ember);

        if (ember->GetVictim() == bot)
        {
            int8 locationIndex = GetAlarCurrentLocationIndex(alar);
            if (locationIndex == LOCATION_NONE)
            {
                Position dest;
                locationIndex = GetAlarDestinationLocationIndex(alar, dest);
            }

            if (locationIndex >= PLATFORM_0_IDX && locationIndex <= PLATFORM_3_IDX)
            {
                const Position& groundTarget = GROUND_POSITIONS[locationIndex];
                const Position& center = ALAR_POINT_MIDDLE;
                float dx = center.GetPositionX() - groundTarget.GetPositionX();
                float dy = center.GetPositionY() - groundTarget.GetPositionY();
                float distToCenter =
                    groundTarget.GetExactDist2d(center.GetPositionX(), center.GetPositionY());

                constexpr float moveDist = 25.0f;
                float targetX = groundTarget.GetPositionX() + (dx / distToCenter) * moveDist;
                float targetY = groundTarget.GetPositionY() + (dy / distToCenter) * moveDist;

                return MoveTo(TEMPEST_KEEP_MAP_ID, targetX, targetY, groundTarget.GetPositionZ(), false,
                              false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
            }
            else
            {
                constexpr float safeDistance = 16.0f;
                if (GetNearestPlayerInRadius(bot, safeDistance))
                    return MoveFromGroup(safeDistance);
            }
        }
        else if (!bot->IsWithinMeleeRange(ember))
        {
            return MoveTo(TEMPEST_KEEP_MAP_ID, ember->GetPositionX(), ember->GetPositionY(),
                          ember->GetPositionZ(), false, false, false, false,
                          MovementPriority::MOVEMENT_COMBAT, true, false);
        }
    }

    return false;
}

// One Ember will be tanked by the second assist tank in Phase 2, and the other by
// the main tank or first assist tank (whichever is not tanking Al'ar)
bool AlarAssistTanksPickUpEmbersAction::HandlePhase2Embers(Unit* alar)
{
    auto [firstEmber, secondEmber] = GetFirstTwoEmbersOfAlar(botAI);

    if (botAI->IsAssistTankOfIndex(bot, 1, true) && firstEmber)
    {
        MarkTargetWithSquare(bot, firstEmber);
        SetRtiTarget(botAI, "square", firstEmber);

        if (firstEmber->GetVictim() != bot)
        {
            if (bot->GetTarget() != firstEmber->GetGUID())
                return Attack(firstEmber);

            return botAI->DoSpecificAction("taunt spell", Event(), true);
        }
        else if (bot->IsWithinMeleeRange(firstEmber))
        {
            constexpr float safeDistance = 16.0f;
            if (GetNearestNonTankPlayerInRadius(botAI, bot, safeDistance))
                return MoveFromGroup(safeDistance);
        }
    }
    else if (GetSecondEmberTank(botAI) == bot && secondEmber)
    {
        MarkTargetWithCircle(bot, secondEmber);
        SetRtiTarget(botAI, "circle", secondEmber);

        if (secondEmber->GetVictim() != bot)
        {
            if (bot->GetTarget() != secondEmber->GetGUID())
                return Attack(secondEmber);

            return botAI->DoSpecificAction("taunt spell", Event(), true);
        }
        else if (bot->IsWithinMeleeRange(secondEmber))
        {
            constexpr float safeDistance = 16.0f;
            if (GetNearestNonTankPlayerInRadius(botAI, bot, safeDistance))
                return MoveFromGroup(safeDistance);
        }
    }

    return false;
}

bool AlarRangedDpsPrioritizeEmbersAction::Execute(Event /*event*/)
{
    auto [firstEmber, secondEmber] = GetFirstTwoEmbersOfAlar(botAI);

    constexpr float safeDistance = 16.0f;
    if (firstEmber)
    {
        if (bot->GetDistance2d(firstEmber) < safeDistance)
        {
            bot->AttackStop();
            bot->InterruptNonMeleeSpells(true);
            return MoveAway(firstEmber, safeDistance - bot->GetDistance2d(firstEmber));
        }

        SetRtiTarget(botAI, "square", firstEmber);
        if (bot->GetTarget() != firstEmber->GetGUID())
            return Attack(firstEmber);
    }
    else if (secondEmber)
    {
        if (bot->GetDistance2d(secondEmber) < safeDistance)
        {
            bot->AttackStop();
            bot->InterruptNonMeleeSpells(true);
            return MoveAway(secondEmber, safeDistance - bot->GetDistance2d(secondEmber));
        }

        SetRtiTarget(botAI, "circle", secondEmber);
        if (bot->GetTarget() != secondEmber->GetGUID())
            return Attack(secondEmber);
    }
    else if (Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar"))
    {
        SetRtiTarget(botAI, "star", alar);
        if (bot->GetTarget() != alar->GetGUID())
            return Attack(alar);
    }

    return false;
}

// Jump from platform during Flame Quills and wait at assigned position after landing
bool AlarJumpFromPlatformAction::Execute(Event /*event*/)
{
    if (bot->GetPositionZ() > ALAR_BALCONY_Z)
    {
        int8 closestPlatform;
        Position ground;
        GetClosestPlatformAndGround(bot->GetPosition(), closestPlatform, ground);

        bot->AttackStop();
        bot->InterruptNonMeleeSpells(true);
        return JumpTo(TEMPEST_KEEP_MAP_ID, ground.GetPositionX(), ground.GetPositionY(),
                      ground.GetPositionZ(), MovementPriority::MOVEMENT_FORCED);
    }
    else
    {
        constexpr float distAlarTankFromPosition = 5.0f;
        constexpr float distEmberTankFromPos = 25.0f;
        constexpr float distMeleeDpsFromPos = 5.0f;
        constexpr float distRangedFromPos = 10.0f;

        if (botAI->IsMainTank(bot) &&
            bot->GetExactDist2d(ALAR_SW_RAMP_BASE.GetPositionX(),
                                ALAR_SW_RAMP_BASE.GetPositionY()) > distAlarTankFromPosition)
        {
            return MoveTo(TEMPEST_KEEP_MAP_ID, ALAR_SW_RAMP_BASE.GetPositionX(),
                          ALAR_SW_RAMP_BASE.GetPositionY(), ALAR_SW_RAMP_BASE.GetPositionZ(),
                          false, false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
        }
        else if (botAI->IsAssistTankOfIndex(bot, 0, true) &&
                 bot->GetExactDist2d(ALAR_SE_RAMP_BASE.GetPositionX(),
                                     ALAR_SE_RAMP_BASE.GetPositionY()) > distAlarTankFromPosition)
        {
            return MoveTo(TEMPEST_KEEP_MAP_ID, ALAR_SE_RAMP_BASE.GetPositionX(),
                          ALAR_SE_RAMP_BASE.GetPositionY(), ALAR_SE_RAMP_BASE.GetPositionZ(),
                          false, false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
        }
        else if (botAI->IsAssistTankOfIndex(bot, 1, true) &&
                 bot->GetExactDist2d(ALAR_POINT_MIDDLE.GetPositionX(),
                                     ALAR_POINT_MIDDLE.GetPositionY()) > distEmberTankFromPos)
        {
            return MoveInside(TEMPEST_KEEP_MAP_ID, ALAR_POINT_MIDDLE.GetPositionX(),
                              ALAR_POINT_MIDDLE.GetPositionY(), ALAR_POINT_MIDDLE.GetPositionZ(),
                              distEmberTankFromPos, MovementPriority::MOVEMENT_FORCED);
        }
        else if (botAI->IsMelee(bot) &&
                 bot->GetExactDist2d(ALAR_ROOM_S_CENTER.GetPositionX(),
                                     ALAR_ROOM_S_CENTER.GetPositionY()) > distMeleeDpsFromPos)
        {
            return MoveInside(TEMPEST_KEEP_MAP_ID, ALAR_ROOM_S_CENTER.GetPositionX(),
                              ALAR_ROOM_S_CENTER.GetPositionY(), ALAR_ROOM_S_CENTER.GetPositionZ(),
                              distMeleeDpsFromPos, MovementPriority::MOVEMENT_FORCED);
        }
        else if (botAI->IsRanged(bot) &&
                 bot->GetExactDist2d(ALAR_POINT_MIDDLE.GetPositionX(),
                                     ALAR_POINT_MIDDLE.GetPositionY()) > distRangedFromPos)
        {
            return MoveInside(TEMPEST_KEEP_MAP_ID, ALAR_POINT_MIDDLE.GetPositionX(),
                              ALAR_POINT_MIDDLE.GetPositionY(), ALAR_POINT_MIDDLE.GetPositionZ(),
                              distRangedFromPos, MovementPriority::MOVEMENT_FORCED);
        }
    }

    return false;
}

bool AlarMoveAwayFromRebirthAction::Execute(Event /*event*/)
{
    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar)
        return false;

    if (bot->GetPositionZ() > ALAR_BALCONY_Z)
    {
        int8 closestPlatform;
        Position ground;
        GetClosestPlatformAndGround(bot->GetPosition(), closestPlatform, ground);

        bot->AttackStop();
        bot->InterruptNonMeleeSpells(true);
        return JumpTo(TEMPEST_KEEP_MAP_ID, ground.GetPositionX(), ground.GetPositionY(),
                      ground.GetPositionZ(), MovementPriority::MOVEMENT_FORCED);
    }
    else
    {
        float currentDistance = bot->GetDistance2d(alar);
        constexpr float safeDistance = 20.0f;
        if (currentDistance < safeDistance)
            return MoveAway(alar, safeDistance - currentDistance);
    }

    return false;
}

// Main tank and first assist tank will swap tanking Al'ar when Melt Armor is applied
bool AlarSwapTanksOnBossAction::Execute(Event /*event*/)
{
    if (!botAI->IsMainTank(bot) && !botAI->IsAssistTankOfIndex(bot, 0, true))
        return false;

    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar)
        return false;

    if (alar->GetHealth() == alar->GetMaxHealth())
    {
        SetRtiTarget(botAI, "star", alar);
        if (bot->GetTarget() != alar->GetGUID())
            return Attack(alar);
    }

    Player* secondEmberTank = GetSecondEmberTank(botAI);
    if (secondEmberTank && secondEmberTank != bot)
    {
        SetRtiTarget(botAI, "star", alar);
        if (bot->GetTarget() != alar->GetGUID())
            return Attack(alar);
        else if (alar->GetVictim() != bot)
            return botAI->DoSpecificAction("taunt spell", Event(), true);
    }

    return false;
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
    constexpr float hazardRadius = 8.0f;

    std::vector<Unit*> flamePatches =
        GetAllHazardTriggers(bot, NPC_FLAME_PATCH, searchRadius);

    for (Unit* flamePatch : flamePatches)
    {
        if (bot->GetExactDist2d(flamePatch) < hazardRadius)
        {
            Position safestPos = FindSafestNearbyPosition(bot, flamePatches, hazardRadius);
            bot->AttackStop();
            bot->InterruptNonMeleeSpells(true);
            return MoveTo(TEMPEST_KEEP_MAP_ID, safestPos.GetPositionX(), safestPos.GetPositionY(),
                          safestPos.GetPositionZ(), false, false, false, true,
                          MovementPriority::MOVEMENT_FORCED, true, false);
        }
    }

    return false;
}

bool AlarAvoidFlamePatchesAndDiveBombsAction::HandleDiveBomb(Unit* alar)
{
    if ((alar->HasUnitState(UNIT_STATE_CASTING) &&
        alar->FindCurrentSpellBySpellId(SPELL_REBIRTH_DIVE)) ||
        !alar->IsVisible())
    {
        float currentDistance = bot->GetDistance2d(alar);
        constexpr float safeDistance = 20.0f;
        if (currentDistance < safeDistance)
        {
            bot->AttackStop();
            bot->InterruptNonMeleeSpells(true);
            return MoveAway(alar, safeDistance - currentDistance);
        }
    }
    else
    {
        Position dest;
        if (GetAlarCurrentLocationIndex(alar) == POINT_QUILL_OR_DIVE_IDX ||
            GetAlarDestinationLocationIndex(alar, dest) == POINT_QUILL_OR_DIVE_IDX)
        {
            constexpr float safeDistance = 10.0f;
            constexpr uint32 minInterval = 0;
            if (Unit* nearestPlayer = GetNearestPlayerInRadius(bot, safeDistance))
                return FleePosition(nearestPlayer->GetPosition(), safeDistance, minInterval);
        }
    }

    return false;
}

// For Phase 2, ensure that bots don't get too far away and become inactive
bool AlarReturnToRoomCenterAction::Execute(Event /*event*/)
{
    constexpr float distFromCenter = 45.0f;
    const Position& center = ALAR_ROOM_CENTER;
    if (bot->GetVictim() == nullptr &&
        bot->GetExactDist2d(center.GetPositionX(), center.GetPositionY()) > distFromCenter)
    {
        return MoveInside(TEMPEST_KEEP_MAP_ID, center.GetPositionX(), center.GetPositionY(),
                          center.GetPositionZ(), distFromCenter - 5.0f,
                          MovementPriority::MOVEMENT_COMBAT);
    }

    return false;
}

bool AlarManagePhaseTrackerAction::Execute(Event /*event*/)
{
    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar)
        return false;

    const uint32 instanceId = alar->GetMap()->GetInstanceId();

    if (alar->GetHealthPct() > 99.5f && alar->GetPositionZ() >= ALAR_BALCONY_Z)
    {
        isAlarInPhase2.erase(instanceId);
        lastRebirthState.erase(instanceId);
    }

    bool rebirthActive = alar->HasUnitState(UNIT_STATE_CASTING) &&
                         alar->FindCurrentSpellBySpellId(SPELL_REBIRTH_PHASE2);
    bool lastRebirth = lastRebirthState[instanceId];

    if (lastRebirth && !rebirthActive)
        isAlarInPhase2[instanceId] = true;

    lastRebirthState[instanceId] = rebirthActive;

    return false;
}

// Void Reaver

bool VoidReaverTanksPositionBossAction::Execute(Event /*event*/)
{
    Unit* voidReaver = AI_VALUE2(Unit*, "find target", "void reaver");
    if (!voidReaver)
        return false;

    const Position& position = VOID_REAVER_TANK_POSITION;

    float dX = position.GetPositionX() - bot->GetPositionX();
    float dY = position.GetPositionY() - bot->GetPositionY();
    float distToPosition = bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());

    if (bot->IsWithinMeleeRange(voidReaver) && distToPosition > 2.0f)
    {
        float moveDist = std::min(5.0f, distToPosition);
        float moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
        float moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

        return MoveTo(TEMPEST_KEEP_MAP_ID, moveX, moveY, position.GetPositionZ(), false,
                      false, false, false, MovementPriority::MOVEMENT_COMBAT, true, true);
    }

    return false;
}

bool VoidReaverUseAggroDumpAbilityAction::Execute(Event /*event*/)
{
    botAI->Reset();
    static const std::array<const char*, 6> spells =
    {
        "divine protection",
        "fade",
        "feign death",
        "ice block",
        "soulshatter",
        "vanish",
    };
    for (const char* spell : spells)
    {
        if (botAI->CanCastSpell(spell, bot) &&
            botAI->CastSpell(spell, bot))
            return true;
    }

    return false;
}

bool VoidReaverSpreadRangedAction::Execute(Event /*event*/)
{
    Unit* voidReaver = AI_VALUE2(Unit*, "find target", "void reaver");
    if (!voidReaver)
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    ObjectGuid guid = bot->GetGUID();

    if (!hasReachedVoidReaverPosition[guid])
    {
        int healerCount = 0, rangedDpsCount = 0;
        int healerIndex = GetHealerIndex(group, healerCount);
        int rangedDpsIndex = GetRangedDpsIndex(group, rangedDpsCount);

        // Void Reaver's hitbox is 15 yards (GetDistance2d() of 16.5 yards for non-Tauren)
        constexpr float radius = 45.0f;
        float targetX = 0.0f;
        float targetY = 0.0f;

        if (healerIndex != -1 && healerCount > 0)
        {
            float angle = 2 * M_PI * healerIndex / healerCount;
            targetX = voidReaver->GetPositionX() + radius * std::cos(angle);
            targetY = voidReaver->GetPositionY() + radius * std::sin(angle);
        }
        else if (rangedDpsIndex != -1 && rangedDpsCount > 0)
        {
            float angle = 2 * M_PI * rangedDpsIndex / rangedDpsCount;
            if (healerCount > 0)
                angle += M_PI / rangedDpsCount;

            targetX = voidReaver->GetPositionX() + radius * std::cos(angle);
            targetY = voidReaver->GetPositionY() + radius * std::sin(angle);
        }

        if (bot->GetExactDist2d(targetX, targetY) > 2.0f)
        {
            return MoveTo(TEMPEST_KEEP_MAP_ID, targetX, targetY, bot->GetPositionZ(), false,
                          false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
        }
        else
        {
            hasReachedVoidReaverPosition[guid] = true;
        }
    }
    else
    {
        constexpr float safeDistance = 20.0f;
        constexpr uint32 minInterval = 1000;
        if (bot->GetDistance2d(voidReaver) < safeDistance)
            return FleePosition(voidReaver->GetPosition(), safeDistance, minInterval);
    }

    return false;
}

int VoidReaverSpreadRangedAction::GetHealerIndex(Group* group, int& healerCount)
{
    std::vector<Player*> healers;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !botAI->IsHeal(member))
            continue;

        healers.push_back(member);
    }

    healerCount = healers.size();
    auto it = std::find(healers.begin(), healers.end(), bot);
    return (it != healers.end()) ? std::distance(healers.begin(), it) : -1;
}

int VoidReaverSpreadRangedAction::GetRangedDpsIndex(Group* group, int& rangedDpsCount)
{
    std::vector<Player*> rangedDps;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !botAI->IsRanged(member) || botAI->IsHeal(member))
            continue;

        rangedDps.push_back(member);
    }

    rangedDpsCount = rangedDps.size();
    auto it = std::find(rangedDps.begin(), rangedDps.end(), bot);
    return (it != rangedDps.end()) ? std::distance(rangedDps.begin(), it) : -1;
}

bool VoidReaverAvoidArcaneOrbAction::Execute(Event /*event*/)
{
    Unit* voidReaver = AI_VALUE2(Unit*, "find target", "void reaver");
    if (!voidReaver)
        return false;

    auto it = voidReaverArcaneOrbs.find(bot->GetMap()->GetInstanceId());
    if (it == voidReaverArcaneOrbs.end() || it->second.empty())
        return false;

    uint32 currentTime = getMSTime();
    constexpr uint32 orbDuration = 7000;
    constexpr float safeDistance = 22.0f;
    bool shouldFlee = false;
    Position fleeDest;

    for (auto const& orb : it->second)
    {
        if (getMSTimeDiff(orb.castTime, currentTime) <= orbDuration)
        {
            if (bot->GetExactDist2d(orb.destination.GetPositionX(),
                                    orb.destination.GetPositionY()) < safeDistance)
            {
                shouldFlee = true;
                fleeDest = orb.destination;
                break;
            }
        }
    }

    it->second.erase(std::remove_if(it->second.begin(), it->second.end(),
        [currentTime](const ArcaneOrbData& orb) {
            return getMSTimeDiff(orb.castTime, currentTime) > orbDuration;
        }), it->second.end());

    if (shouldFlee)
    {
        constexpr uint32 minInterval = 0;
        bot->AttackStop();
        bot->InterruptNonMeleeSpells(true);
        return FleePosition(fleeDest, safeDistance, minInterval);
    }

    return false;
}

bool VoidReaverEraseTrackersAction::Execute(Event /*event*/)
{
    Unit* voidReaver = AI_VALUE2(Unit*, "find target", "void reaver");
    if (voidReaver)
        return false;

    bool erased = false;

    if (voidReaverArcaneOrbs.erase(bot->GetMap()->GetInstanceId()))
        erased = true;

    if (hasReachedVoidReaverPosition.erase(bot->GetGUID()))
        erased = true;

    return erased;
}

// High Astromancer Solarian

bool HighAstromancerSolarianRangedLeaveSpaceForMeleeAction::Execute(Event /*event*/)
{
    Unit* astromancer = AI_VALUE2(Unit*, "find target", "high astromancer solarian");
    if (!astromancer)
        return false;

    float currentDistance = bot->GetExactDist2d(astromancer);
    constexpr float minDistance = 20.0f;
    if (currentDistance < minDistance)
        return MoveAway(astromancer, minDistance - currentDistance);

    return false;
}

bool HighAstromancerSolarianMoveAwayFromGroupAction::Execute(Event /*event*/)
{
    constexpr float safeDistance = 15.0f;
    if (GetNearestPlayerInRadius(bot, safeDistance))
    {
        botAI->Reset();
        return MoveFromGroup(safeDistance);
    }

    return false;
}

bool HighAstromancerSolarianStackForAoeAction::Execute(Event /*event*/)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    Player* stackTarget = nullptr;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive() && botAI->IsRanged(member))
        {
            stackTarget = member;
            break;
        }
    }

    if (stackTarget && bot != stackTarget && bot->GetExactDist2d(stackTarget) > 5.0f)
    {
        return MoveTo(TEMPEST_KEEP_MAP_ID, stackTarget->GetPositionX(), stackTarget->GetPositionY(),
                      stackTarget->GetPositionZ(), false, false, false, false,
                      MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    return false;
}

// Split melee into two groups, one on each Solarium Priest
bool HighAstromancerSolarianTargetSolariumPriestsAction::Execute(Event /*event*/)
{
    auto priestsPair = GetSolariumPriests(botAI);
    if (!priestsPair.first || !priestsPair.second)
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    auto meleeMembers = GetMeleeBots(group);
    if (meleeMembers.empty())
        return false;

    Unit* targetPriest = AssignSolariumPriestsToBots(priestsPair, meleeMembers);
    if (!targetPriest)
        return false;

    auto it = std::find(meleeMembers.begin(), meleeMembers.end(), bot);
    if (it == meleeMembers.end())
        return false;

    if (targetPriest == priestsPair.first)
    {
        MarkTargetWithSquare(bot, targetPriest);
        SetRtiTarget(botAI, "square", targetPriest);
    }
    else
    {
        MarkTargetWithStar(bot, targetPriest);
        SetRtiTarget(botAI, "star", targetPriest);
    }

    if (bot->GetTarget() != targetPriest->GetGUID())
        return Attack(targetPriest);

    return false;
}

std::pair<Unit*, Unit*> HighAstromancerSolarianTargetSolariumPriestsAction::GetSolariumPriests(PlayerbotAI* botAI)
{
    Unit* lowest = nullptr;
    Unit* highest = nullptr;

    for (auto const& guid :
         botAI->GetAiObjectContext()->GetValue<GuidVector>("possible targets no los")->Get())
    {
        Unit* unit = botAI->GetUnit(guid);
        if (unit && unit->GetEntry() == NPC_SOLARIUM_PRIEST)
        {
            if (!lowest || unit->GetGUID().GetRawValue() < lowest->GetGUID().GetRawValue())
                lowest = unit;

            if (!highest || unit->GetGUID().GetRawValue() > highest->GetGUID().GetRawValue())
                highest = unit;
        }
    }

    return {lowest, highest};
}

std::vector<Player*> HighAstromancerSolarianTargetSolariumPriestsAction::GetMeleeBots(Group* group)
{
    std::vector<Player*> meleeMembers;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive() && botAI->IsMelee(member) && GET_PLAYERBOT_AI(member))
            meleeMembers.push_back(member);
    }

    return meleeMembers;
}

Unit* HighAstromancerSolarianTargetSolariumPriestsAction::AssignSolariumPriestsToBots(
    const std::pair<Unit*, Unit*>& priestsPair, const std::vector<Player*>& meleeMembers)
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
    else
        return priestsPair.second;
}

bool HighAstromancerSolarianCastFearWardOnMainTankAction::Execute(Event /*event*/)
{
    Player* mainTank = GetGroupMainTank(botAI, bot);
    if (mainTank && botAI->CanCastSpell("fear ward", mainTank))
        return botAI->CastSpell("fear ward", mainTank);

    return false;
}

// Kael'thas Sunstrider <Lord of the Blood Elves>

bool KaelthasSunstriderKiteThaladredAction::Execute(Event /*event*/)
{
    Unit* thaladred = AI_VALUE2(Unit*, "find target", "thaladred the darkener");
    if (!thaladred)
        return false;

    float currentDistance = bot->GetExactDist2d(thaladred);
    constexpr float safeDistance = 25.0f;
    if (currentDistance < safeDistance)
        return MoveAway(thaladred, safeDistance - currentDistance);

    return false;
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
        if (member && member->IsAlive() && member->getClass() == CLASS_HUNTER && GET_PLAYERBOT_AI(member))
            hunters.push_back(member);

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

    Unit* advisorTarget = nullptr;
    Player* tankTarget = nullptr;
    if (hunterIndex == 0 || hunterIndex == 2)
    {
        advisorTarget = AI_VALUE2(Unit*, "find target", "grand astromancer capernian");
        tankTarget = GetCapernianTank(bot);
    }
    else if (hunterIndex == 1)
    {
        advisorTarget = AI_VALUE2(Unit*, "find target", "master engineer telonicus");
        tankTarget = GetGroupAssistTank(botAI, bot, 0);
    }

    if (!advisorTarget ||
        advisorTarget->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE) ||
        advisorTarget->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE) ||
        advisorTarget->HasAura(SPELL_PERMANENT_FEIGN_DEATH))
        return false;

    if (!tankTarget || !tankTarget->IsAlive())
        return false;

    if (botAI->CanCastSpell("misdirection", tankTarget))
        return botAI->CastSpell("misdirection", tankTarget);

    if (bot->HasAura(SPELL_MISDIRECTION) && botAI->CanCastSpell("steady shot", advisorTarget))
        return botAI->CastSpell("steady shot", advisorTarget);

    return false;
}

bool KaelthasSunstriderMainTankPositionSanguinarAction::Execute(Event /*event*/)
{
    Unit* sanguinar = AI_VALUE2(Unit*, "find target", "lord sanguinar");
    if (!sanguinar)
        return false;

    MarkTargetWithStar(bot, sanguinar);
    SetRtiTarget(botAI, "star", sanguinar);

    if (bot->GetTarget() != sanguinar->GetGUID())
        return Attack(sanguinar);

    if (sanguinar->GetVictim() == bot && bot->IsWithinMeleeRange(sanguinar))
    {
        const Position& position = SANGUINAR_TANK_POSITION;
        float distToPosition =
            bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());

        if (distToPosition > 2.0f)
        {
            float dX = position.GetPositionX() - bot->GetPositionX();
            float dY = position.GetPositionY() - bot->GetPositionY();
            float moveDist = std::min(5.0f, distToPosition);
            float moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
            float moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

            return MoveTo(TEMPEST_KEEP_MAP_ID, moveX, moveY, position.GetPositionZ(), false,
                          false, false, false, MovementPriority::MOVEMENT_COMBAT, true, true);
        }
    }

    return false;
}

bool KaelthasSunstriderCastFearWardOnSanguinarTankAction::Execute(Event /*event*/)
{
    Player* mainTank = GetGroupMainTank(botAI, bot);
    if (mainTank && botAI->CanCastSpell("fear ward", mainTank))
        return botAI->CastSpell("fear ward", mainTank);

    return false;
}

bool KaelthasSunstriderWarlockTankPositionCapernianAction::Execute(Event /*event*/)
{
    Unit* capernian = AI_VALUE2(Unit*, "find target", "grand astromancer capernian");
    if (!capernian)
        return false;

    MarkTargetWithCircle(bot, capernian);
    SetRtiTarget(botAI, "circle", capernian);

    if (bot->GetTarget() != capernian->GetGUID() &&
        botAI->CanCastSpell("searing pain", capernian) &&
        botAI->CastSpell("searing pain", capernian))
        return true;

    if (capernian->GetVictim() == bot)
    {
        float currentDist = bot->GetDistance2d(capernian);
        constexpr float minDistance = 28.0f;
        if (currentDist < minDistance)
            return MoveAway(capernian, minDistance - currentDist);
    }

    if (botAI->CanCastSpell("searing pain", capernian))
        return botAI->CastSpell("searing pain", capernian);

    return false;
}

bool KaelthasSunstriderSpreadAndMoveAwayFromCapernianAction::Execute(Event /*event*/)
{
    Unit* capernian = AI_VALUE2(Unit*, "find target", "grand astromancer capernian");
    if (!capernian)
        return false;

    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return false;

    boss_kaelthas* kaelAI = dynamic_cast<boss_kaelthas*>(kaelthas->GetAI());
    if (!kaelAI)
        return false;

    if (botAI->IsRanged(bot) && capernian->GetVictim() != bot &&
        RangedBotsDisperse(kaelAI, capernian))
    {
        return true;
    }
    else if (botAI->IsMelee(bot) && kaelAI->GetPhase() == PHASE_SINGLE_ADVISOR &&
             MeleeStayBackFromCapernian(capernian))
    {
        return true;
    }

    return false;
}

bool KaelthasSunstriderSpreadAndMoveAwayFromCapernianAction::RangedBotsDisperse(boss_kaelthas* kaelAI, Unit* capernian)
{
    if (kaelAI->GetPhase() == PHASE_SINGLE_ADVISOR)
    {
        Group* group = bot->GetGroup();
        if (!group)
            return false;

        std::vector<Player*> healers;
        std::vector<Player*> rangedDps;
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !botAI->IsRanged(member))
                continue;

            if (botAI->IsHeal(member))
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
        float arcSpan = botAI->IsHeal(bot) ? M_PI / 2.0f : 2.0f * M_PI / 3.0f;
        constexpr float arcCenter = 2.9f;
        float arcStart = arcCenter - arcSpan / 2.0f;

        // Capernian's hitbox is 4.5 yards (GetDistance2d of 6.0f for non-Tauren)
        if (botAI->IsHeal(bot))
        {
            auto findIt = std::find(healers.begin(), healers.end(), bot);
            botIndex = (findIt != healers.end()) ? std::distance(healers.begin(), findIt) : 0;
            radius = 42.0f;
            count = healers.size();
        }
        else
        {
            auto findIt = std::find(rangedDps.begin(), rangedDps.end(), bot);
            botIndex = (findIt != rangedDps.end()) ? std::distance(rangedDps.begin(), findIt) : 0;
            radius = 34.0f;
            count = rangedDps.size();
        }

        angle = (count == 1) ? arcCenter :
            (arcStart + arcSpan * static_cast<float>(botIndex) / static_cast<float>(count - 1));

        float targetX = capernian->GetPositionX() + radius * std::cos(angle);
        float targetY = capernian->GetPositionY() + radius * std::sin(angle);

        if (bot->GetExactDist2d(targetX, targetY) > 1.0f)
        {
            bot->AttackStop();
            bot->InterruptNonMeleeSpells(true);
            return MoveTo(TEMPEST_KEEP_MAP_ID, targetX, targetY, bot->GetPositionZ(), false, false,
                          false, true, MovementPriority::MOVEMENT_FORCED, true, false);
        }
    }
    else
    {
        if (AI_VALUE2(Unit*, "find target", "thaladred the darkener"))
            return false;

        const float safeDistance = 6.0f;
        constexpr uint32 minInterval = 1000;
        if (Unit* nearestPlayer = GetNearestPlayerInRadius(bot, safeDistance))
            return FleePosition(nearestPlayer->GetPosition(), safeDistance, minInterval);
    }

    return false;
}

bool KaelthasSunstriderSpreadAndMoveAwayFromCapernianAction::MeleeStayBackFromCapernian(Unit* capernian)
{
    // Main tank purposely stays in range to bait Conflagration in Phase 1
    if (botAI->IsMainTank(bot))
    {
        // MoveTo called for a WorldObj is a GetDistance() check so both hitboxes are account for
        constexpr float desiredDist = 15.0f;
        botAI->Reset();
        return MoveTo(capernian, desiredDist, MovementPriority::MOVEMENT_FORCED);
    }
    else
    {
        constexpr float safeDistance = 42.0f;
        float currentDistance = bot->GetDistance2d(capernian);
        if (currentDistance < safeDistance)
        {
            botAI->Reset();
            return MoveAway(capernian, safeDistance - currentDistance);
        }
        else
        {
            return true;
        }
    }
}

bool KaelthasSunstriderFirstAssistTankPositionTelonicusAction::Execute(Event /*event*/)
{
    Unit* telonicus = AI_VALUE2(Unit*, "find target", "master engineer telonicus");
    if (!telonicus)
        return false;

    MarkTargetWithTriangle(bot, telonicus);
    SetRtiTarget(botAI, "triangle", telonicus);

    if (bot->GetTarget() != telonicus->GetGUID())
        return Attack(telonicus);

    if (telonicus->GetVictim() == bot && bot->IsWithinMeleeRange(telonicus))
    {
        const Position& position = TELONICUS_TANK_POSITION;
        float distToPosition =
            bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());

        if (distToPosition > 2.0f)
        {
            float dX = position.GetPositionX() - bot->GetPositionX();
            float dY = position.GetPositionY() - bot->GetPositionY();
            float moveDist = std::min(5.0f, distToPosition);
            float moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
            float moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

            return MoveTo(TEMPEST_KEEP_MAP_ID, moveX, moveY, position.GetPositionZ(), false,
                          false, false, false, MovementPriority::MOVEMENT_COMBAT, true, true);
        }
    }

    return false;
}

bool KaelthasSunstriderHandleAdvisorRolesInPhase3Action::Execute(Event /*event*/)
{
    const Position* movePosition = nullptr;
    if (botAI->IsAssistHealOfIndex(bot, 0, true))
    {
        movePosition = &ADVISOR_HEAL_POSITION;
    }
    else if (botAI->IsMainTank(bot))
    {
        Unit* sanguinar = AI_VALUE2(Unit*, "find target", "lord sanguinar");
        if (sanguinar && sanguinar->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE))
            movePosition = &SANGUINAR_WAITING_POSITION;
    }
    else if (botAI->IsAssistTankOfIndex(bot, 0, true))
    {
        Unit* telonicus = AI_VALUE2(Unit*, "find target", "master engineer telonicus");
        if (telonicus && telonicus->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE))
            movePosition = &TELONICUS_WAITING_POSITION;
    }
    else if (GetCapernianTank(bot) == bot)
    {
        Unit* capernian = AI_VALUE2(Unit*, "find target", "grand astromancer capernian");
        if (capernian && capernian->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE))
            movePosition = &CAPERNIAN_WAITING_POSITION;
    }

    if (movePosition &&
        bot->GetExactDist2d(movePosition->GetPositionX(), movePosition->GetPositionY()) > 2.0f)
    {
        return MoveTo(TEMPEST_KEEP_MAP_ID, movePosition->GetPositionX(), movePosition->GetPositionY(),
                        movePosition->GetPositionZ(), false, false, false, false,
                        MovementPriority::MOVEMENT_FORCED, true, false);
    }

    return false;
}

bool KaelthasSunstriderReequipGearAction::Execute(Event /*event*/)
{
    return botAI->DoSpecificAction("equip upgrade", Event(), true);
}

bool KaelthasSunstriderAssignAdvisorDpsPriorityAction::Execute(Event /*event*/)
{
    // Target priority 1: Thaladred, except Capernian tank
    Player* capernianTank = GetCapernianTank(bot);
    Unit* thaladred = AI_VALUE2(Unit*, "find target", "thaladred the darkener");

    if ((!capernianTank || bot != capernianTank) &&
        thaladred && !thaladred->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE) &&
        !thaladred->HasAura(SPELL_PERMANENT_FEIGN_DEATH))
    {
        MarkTargetWithSquare(bot, thaladred);
        SetRtiTarget(botAI, "square", thaladred);

        if (bot->GetTarget() != thaladred->GetGUID())
            return Attack(thaladred);

        return false;
    }

    // Target priority 2: Capernian for ranged only (excluding longbow tank)
    Unit* capernian = AI_VALUE2(Unit*, "find target", "grand astromancer capernian");

    if (botAI->IsRangedDps(bot) && !IsDebuffHunter(bot) &&
        capernian && !capernian->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE) &&
        !capernian->HasAura(SPELL_PERMANENT_FEIGN_DEATH))
    {
        SetRtiTarget(botAI, "circle", capernian);

        if (bot->GetTarget() != capernian->GetGUID())
            return Attack(capernian);

        return false;
    }

    // Target priority 3: Sanguinar (debuff hunter and melee move here after Thaladred)
    Unit* sanguinar = AI_VALUE2(Unit*, "find target", "lord sanguinar");

    if (sanguinar && !sanguinar->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE) &&
        !sanguinar->HasAura(SPELL_PERMANENT_FEIGN_DEATH))
    {
        SetRtiTarget(botAI, "star", sanguinar);

        if (bot->GetTarget() != sanguinar->GetGUID())
            return Attack(sanguinar);

        return false;
    }

    // Target priority 4: Telonicus
    Unit* telonicus = AI_VALUE2(Unit*, "find target", "master engineer telonicus");

    if (telonicus && !telonicus->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE) &&
        !telonicus->HasAura(SPELL_PERMANENT_FEIGN_DEATH))
    {
        SetRtiTarget(botAI, "triangle", telonicus);
        if (bot->GetTarget() != telonicus->GetGUID())
            return Attack(telonicus);

        // Melee DPS need to stay at max-ish melee range behind Telonicus to avoid bombs
        if (botAI->IsMelee(bot) && botAI->IsDps(bot) && telonicus->GetVictim() != bot)
        {
            float desiredDist = bot->GetMeleeRange(telonicus);
            float behindAngle = Position::NormalizeOrientation(telonicus->GetOrientation() + M_PI);
            float targetX = telonicus->GetPositionX() + desiredDist * std::cos(behindAngle);
            float targetY = telonicus->GetPositionY() + desiredDist * std::sin(behindAngle);

            if (bot->GetExactDist2d(targetX, targetY) > 0.25f)
            {
                return MoveTo(TEMPEST_KEEP_MAP_ID, targetX, targetY, telonicus->GetPositionZ(), false,
                              false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
            }
        }
    }

    return false;
}

bool KaelthasSunstriderManageAdvisorDpsTimerAction::Execute(Event /*event*/)
{
    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return false;

    static const std::array<const char*, 3> advisorNames =
    {
        "grand astromancer capernian",
        "master engineer telonicus",
        "lord sanguinar"
    };

    for (const char* name : advisorNames)
    {
        Unit* advisor = AI_VALUE2(Unit*, "find target", name);
        if (!advisor)
            continue;

        if (advisor->GetHealth() == advisor->GetMaxHealth() &&
            !advisor->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE))
        {
            const time_t now = std::time(nullptr);
            advisorDpsWaitTimer.insert_or_assign(kaelthas->GetMap()->GetInstanceId(), now);
            return true;
        }
    }

    return false;
}

bool KaelthasSunstriderAssignLegendaryWeaponDpsPriorityAction::Execute(Event /*event*/)
{
    if (botAI->IsAssistTank(bot))
        SetRtiTarget(botAI, "moon", nullptr);

    // Priority 0: Everybody other than the main tank needs to stay away from the axe
    // But this applies to assist tanks only after they get aggro on the mace, dagger, or sword
    Unit* axe = AI_VALUE2(Unit*, "find target", "devastation");
    Unit* mace = AI_VALUE2(Unit*, "find target", "cosmic infuser");
    Unit* dagger = AI_VALUE2(Unit*, "find target", "infinity blades");
    Unit* sword = AI_VALUE2(Unit*, "find target", "warp slicer");

    if (axe)
    {
        bool hasAggroFromWeapon = mace && mace->GetVictim() == bot ||
                                  dagger && dagger->GetVictim() == bot ||
                                  sword && sword->GetVictim() == bot;
        if (!botAI->IsTank(bot) ||
            (botAI->IsAssistTank(bot) && hasAggroFromWeapon))
        {
            float currentDistance = bot->GetExactDist2d(axe);
            const float safeDistance = botAI->IsAssistTank(bot) ? 17.0f : 13.0f;
            if (currentDistance < safeDistance)
                return MoveAway(axe, safeDistance - currentDistance);
        }
    }

    if (botAI->IsDps(bot))
    {
        // Priority 1: Staff of Disintegration (Skull)
        if (Unit* staff = AI_VALUE2(Unit*, "find target", "staff of disintegration"))
        {
            MarkTargetWithSkull(bot, staff);
            SetRtiTarget(botAI, "skull", staff);

            if (bot->GetTarget() != staff->GetGUID())
                return Attack(staff);
        }
        // Priority 2: Cosmic Infuser (Skull)
        else if (mace)
        {
            MarkTargetWithSkull(bot, mace);
            SetRtiTarget(botAI, "skull", mace);

            if (bot->GetTarget() != mace->GetGUID())
                return Attack(mace);
        }
        // Priority 3: Warp Slicer (Skull)
        else if (sword)
        {
            MarkTargetWithSkull(bot, sword);
            SetRtiTarget(botAI, "skull", sword);

            if (bot->GetTarget() != sword->GetGUID())
                return Attack(sword);
        }
        // Priority 4: Infinity Blades (Skull)
        else if (dagger)
        {
            MarkTargetWithSkull(bot, dagger);
            SetRtiTarget(botAI, "skull", dagger);

            if (bot->GetTarget() != dagger->GetGUID())
                return Attack(dagger);
        }
        // Priority 5: Devastation - ranged only (Diamond--marked in other method by main tank)
        else if (axe && botAI->IsRangedDps(bot))
        {
            SetRtiTarget(botAI, "diamond", axe);

            if (bot->GetTarget() != axe->GetGUID())
                return Attack(axe);
        }
        // Priority 6: Netherstrand Longbow (Skull)
        else if (Unit* longbow = AI_VALUE2(Unit*, "find target", "netherstrand longbow"))
        {
            MarkTargetWithSkull(bot, longbow);
            SetRtiTarget(botAI, "skull", longbow);

            if (bot->GetTarget() != longbow->GetGUID())
                return Attack(longbow);
        }
        // Priority 7: Phaseshift Bulwark (Skull)
        else if (Unit* shield = AI_VALUE2(Unit*, "find target", "phaseshift bulwark"))
        {
            MarkTargetWithSkull(bot, shield);
            SetRtiTarget(botAI, "skull", shield);

            if (bot->GetTarget() != shield->GetGUID())
                return Attack(shield);
        }
    }

    return false;
}

bool KaelthasSunstriderMoveDevastationAwayAction::Execute(Event /*event*/)
{
    Unit* axe = AI_VALUE2(Unit*, "find target", "devastation");
    if (!axe)
        return false;

    MarkTargetWithDiamond(bot, axe);
    SetRtiTarget(botAI, "diamond", axe);

    if (bot->GetTarget() != axe->GetGUID())
        return Attack(axe);

    constexpr float safeDistance = 13.0f;
    if (axe->GetVictim() == bot && GetNearestNonTankPlayerInRadius(botAI, bot, safeDistance))
        return MoveFromGroup(safeDistance);

    return false;
}

bool KaelthasSunstriderLootLegendaryWeaponsAction::Execute(Event /*event*/)
{
    struct WeaponInfo
    {
        uint32 npcEntry;
        uint32 itemId;
    };

    static const std::array<WeaponInfo, 7> weapons =
    {
        WeaponInfo{ NPC_NETHERSTRAND_LONGBOW, ITEM_NETHERSTRAND_LONGBOW },
        WeaponInfo{ NPC_COSMIC_INFUSER, ITEM_COSMIC_INFUSER },
        WeaponInfo{ NPC_DEVASTATION, ITEM_DEVASTATION },
        WeaponInfo{ NPC_INFINITY_BLADES, ITEM_INFINITY_BLADE },
        WeaponInfo{ NPC_WARP_SLICER, ITEM_WARP_SLICER },
        WeaponInfo{ NPC_STAFF_OF_DISINTEGRATION, ITEM_STAFF_OF_DISINTEGRATION },
        WeaponInfo{ NPC_PHASESHIFT_BULWARK, ITEM_PHASESHIFT_BULWARK }
    };

    for (auto const& weapon : weapons)
    {
        if (ShouldBotLootWeapon(weapon.npcEntry))
        {
            if (bot->HasItemCount(weapon.itemId, 1, false))
            {
                EquipAction* equipAction =
                    dynamic_cast<EquipAction*>(botAI->GetAiObjectContext()->GetAction("equip"));
                if (equipAction)
                {
                    ItemIds ids;
                    ids.insert(weapon.itemId);
                    equipAction->EquipItems(ids);
                }
                continue;
            }
            return LootWeapon(weapon.npcEntry, weapon.itemId);
        }
    }

    return false;
}

bool KaelthasSunstriderLootLegendaryWeaponsAction::ShouldBotLootWeapon(uint32 weaponEntry)
{
    uint8 tab = AiFactory::GetPlayerSpecTab(bot);
    switch (weaponEntry)
    {
        case NPC_NETHERSTRAND_LONGBOW:
            return bot->getClass() == CLASS_HUNTER;

        case NPC_COSMIC_INFUSER:
            return botAI->IsHeal(bot);

        // Fury Warriors could use the axe, but their DPS is terrible at appropriate gear levels
        // So IMO they're better off looting only the dagger to MH it and break MCs
        case NPC_DEVASTATION:
            return (bot->getClass() == CLASS_WARRIOR && tab == WARRIOR_TAB_ARMS) ||
                   (bot->getClass() == CLASS_PALADIN && tab == PALADIN_TAB_RETRIBUTION) ||
                   (botAI->IsDps(bot) && bot->getClass() == CLASS_DEATH_KNIGHT);

        case NPC_INFINITY_BLADES:
            return bot->getClass() == CLASS_ROGUE ||
                   bot->getClass() == CLASS_HUNTER ||
                   (bot->getClass() == CLASS_SHAMAN && tab == SHAMAN_TAB_ENHANCEMENT) ||
                   (bot->getClass() == CLASS_WARRIOR && tab != WARRIOR_TAB_ARMS);

        case NPC_WARP_SLICER:
            return bot->getClass() == CLASS_ROGUE && tab != ROGUE_TAB_ASSASSINATION ||
                   (botAI->IsTank(bot) &&
                    (bot->getClass() == CLASS_DEATH_KNIGHT ||
                     bot->getClass() == CLASS_PALADIN));

        case NPC_STAFF_OF_DISINTEGRATION:
            return (botAI->IsRangedDps(bot) && bot->getClass() != CLASS_HUNTER) ||
                   (bot->getClass() == CLASS_DRUID && tab == DRUID_TAB_FERAL);

        case NPC_PHASESHIFT_BULWARK:
            return botAI->IsTank(bot) &&
                   (bot->getClass() == CLASS_PALADIN ||
                    bot->getClass() == CLASS_WARRIOR ||
                    bot->getClass() == CLASS_DEATH_KNIGHT);

        default:
            return false;
    }
}

bool KaelthasSunstriderLootLegendaryWeaponsAction::LootWeapon(
    uint32 weaponEntry, uint32 itemId)
{
    constexpr float searchRadius = 150.0f;
    Creature* weapon = bot->FindNearestCreature(weaponEntry, searchRadius, false);

    if (!weapon || weapon->IsAlive())
        return false;

    LootObject loot(bot, weapon->GetGUID());
    if (!loot.IsLootPossible(bot))
        return false;

    context->GetValue<LootObject>("loot target")->Set(loot);

    const float maxLootRange = sPlayerbotAIConfig.lootDistance;
    constexpr float distFromObject = 2.0f;

    if (bot->GetDistance(weapon) > maxLootRange)
        return MoveTo(weapon, distFromObject, MovementPriority::MOVEMENT_COMBAT);

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

bool KaelthasSunstriderUseLegendaryWeaponsAction::Execute(Event /*event*/)
{
    return UsePhaseshiftBulwark() ||
           UseStaffOfDisintegration() ||
           UseNetherstrandLongbow();
}

bool KaelthasSunstriderUseLegendaryWeaponsAction::UsePhaseshiftBulwark()
{
    Item* offHand = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
    if (!offHand || offHand->GetEntry() != ITEM_PHASESHIFT_BULWARK)
        return false;

    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas || !kaelthas->HasAura(SPELL_SHOCK_BARRIER))
        return false;

    if (bot->HasAura(SPELL_ARCANE_BARRIER) || bot->CanUseItem(offHand) != EQUIP_ERR_OK)
        return false;

    return UseEquippedItemWithPacket(offHand);
}

bool KaelthasSunstriderUseLegendaryWeaponsAction::UseStaffOfDisintegration()
{
    Item* mainHand = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
    if (!mainHand || mainHand->GetEntry() != ITEM_STAFF_OF_DISINTEGRATION)
        return false;

    if (bot->HasAura(SPELL_MENTAL_PROTECTION_FIELD) ||
        bot->CanUseItem(mainHand) != EQUIP_ERR_OK)
        return false;

    return UseEquippedItemWithPacket(mainHand);
}

bool KaelthasSunstriderUseLegendaryWeaponsAction::UseNetherstrandLongbow()
{
    Item* ranged = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_RANGED);
    if (!ranged || ranged->GetEntry() != ITEM_NETHERSTRAND_LONGBOW)
        return false;

    if (bot->HasItemCount(ITEM_NETHER_SPIKES, 1, false) ||
        bot->CanUseItem(ranged) != EQUIP_ERR_OK)
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
    if (!botAI->IsMainTank(bot))
        return false;

    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return false;

    MarkTargetWithStar(bot, kaelthas);
    SetRtiTarget(botAI, "star", kaelthas);

    if (bot->GetTarget() != kaelthas->GetGUID())
        return Attack(kaelthas);

    if (kaelthas->GetVictim() == bot && bot->IsWithinMeleeRange(kaelthas))
    {
        const Position& position = KAELTHAS_TANK_POSITION;
        float distToPosition =
            bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());

        if (distToPosition > 4.0f)
        {
            float dX = position.GetPositionX() - bot->GetPositionX();
            float dY = position.GetPositionY() - bot->GetPositionY();
            float moveDist = std::min(5.0f, distToPosition);
            float moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
            float moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

            return MoveTo(TEMPEST_KEEP_MAP_ID, moveX, moveY, position.GetPositionZ(), false,
                          false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
        }
    }

    return false;
}

bool KaelthasSunstriderAvoidFlameStrikeAction::Execute(Event /*event*/)
{
    constexpr float searchRadius = 40.0f;
    std::vector<Unit*> flameStrikes =
        GetAllHazardTriggers(bot, NPC_FLAME_STRIKE_TRIGGER, searchRadius);

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

    botAI->Reset();
    return MoveTo(TEMPEST_KEEP_MAP_ID, safestPos.GetPositionX(), safestPos.GetPositionY(),
                  safestPos.GetPositionZ(), false, false, false, true,
                  MovementPriority::MOVEMENT_COMBAT, true, false);
}

bool KaelthasSunstriderHandlePhoenixesAndEggsAction::Execute(Event /*event*/)
{
    if (botAI->IsAssistTankOfIndex(bot, 0, true) || botAI->IsAssistTankOfIndex(bot, 1, true))
         return AssistTanksPickUpPhoenixes();
    else
        return NonTanksDestroyEggsAndAvoidPhoenixes();
}

bool KaelthasSunstriderHandlePhoenixesAndEggsAction::AssistTanksPickUpPhoenixes()
{
    std::vector<Unit*> phoenixes;
    auto const& npcs = botAI->GetAiObjectContext()->GetValue<GuidVector>("possible targets no los")->Get();
    for (auto const& npcGuid : npcs)
    {
        Unit* unit = botAI->GetUnit(npcGuid);
        if (unit && unit->GetEntry() == NPC_PHOENIX && unit->IsAlive())
            phoenixes.push_back(unit);
    }

    if (phoenixes.empty())
        return false;

    std::sort(phoenixes.begin(), phoenixes.end(),
              [](Unit* first, Unit* second) { return first->GetGUID() < second->GetGUID(); });

    Unit* targetPhoenix = nullptr;
    if (botAI->IsAssistTankOfIndex(bot, 0, true))
    {
        targetPhoenix = phoenixes[0];
        MarkTargetWithSquare(bot, targetPhoenix);
        SetRtiTarget(botAI, "square", targetPhoenix);
    }
    else if (botAI->IsAssistTankOfIndex(bot, 1, true) && phoenixes.size() >= 2)
    {
        targetPhoenix = phoenixes[1];
        MarkTargetWithCircle(bot, targetPhoenix);
        SetRtiTarget(botAI, "circle", targetPhoenix);
    }

    if (!targetPhoenix)
        return false;

    if (bot->GetTarget() != targetPhoenix->GetGUID())
        return Attack(targetPhoenix);

    constexpr float safeDistance = 12.0f;
    if (targetPhoenix->GetVictim() == bot &&
        GetNearestNonTankPlayerInRadius(botAI, bot, safeDistance))
        return MoveFromGroup(safeDistance);

    return false;
}

bool KaelthasSunstriderHandlePhoenixesAndEggsAction::NonTanksDestroyEggsAndAvoidPhoenixes()
{
    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return false;

    if (botAI->IsDps(bot) && !kaelthas->HasAura(SPELL_SHOCK_BARRIER))
    {
        if (Unit* phoenixEgg = GetFirstAliveUnitByEntry(botAI, NPC_PHOENIX_EGG))
        {
            MarkTargetWithDiamond(bot, phoenixEgg);
            SetRtiTarget(botAI, "diamond", phoenixEgg);

            if (bot->GetTarget() != phoenixEgg->GetGUID())
                return Attack(phoenixEgg);
        }
    }
    else if (botAI->IsDps(bot))
        return false;

    if (Unit* phoenix = AI_VALUE2(Unit*, "find target", "phoenix"))
    {
        float currentDistance = bot->GetExactDist2d(phoenix);
        constexpr float safeDistance = 12.0f;
        if (currentDistance < safeDistance)
            return MoveAway(phoenix, safeDistance - currentDistance);
    }

    return false;
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

        if (member->HasAura(SPELL_KAELTHAS_MIND_CONTROL))
        {
            float dist = bot->GetExactDist2d(member);
            if (dist < closestDist)
            {
                closestDist = dist;
                mcTarget = member;
            }
        }
    }

    if (!mcTarget)
        return false;

    if (!bot->IsWithinMeleeRange(mcTarget))
    {
        return MoveTo(TEMPEST_KEEP_MAP_ID, mcTarget->GetPositionX(), mcTarget->GetPositionY(),
                      mcTarget->GetPositionZ(), false, false, false, false,
                      MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    if (bot->getClass() == CLASS_ROGUE &&
        AiFactory::GetPlayerSpecTab(bot) != ROGUE_TAB_COMBAT &&
        botAI->CanCastSpell("sinister strike", mcTarget))
    {
        return botAI->CastSpell("sinister strike", mcTarget);
    }
    else
    {
        static const std::array<const char*, 4> spells =
        {
            "hamstring",
            "wing clip",
            "shiv",
            "stormstrike"
        };
        for (const char* spell : spells)
        {
            if (botAI->CanCastSpell(spell, mcTarget))
                return botAI->CastSpell(spell, mcTarget);
        }
    }

    return false;
}

// Shock Barrier needs to be #1 focus, even if there is a Phoenix Egg up
bool KaelthasSunstriderBreakThroughShockBarrierAction::Execute(Event /*event*/)
{
    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return false;

    if (!kaelthas->HasAura(SPELL_SHOCK_BARRIER))
    {
        static const std::array<const char*, 8> spells =
        {
            "bash",
            "counterspell",
            "kick",
            "mind freeze",
            "pummel",
            "shield bash",
            "silencing shot",
            "wind shear",
        };
        for (const char* spell : spells)
        {
            if (botAI->CanCastSpell(spell, kaelthas))
                return botAI->CastSpell(spell, kaelthas);
        }
    }
    else if (bot->GetTarget() != kaelthas->GetGUID())
    {
        SetRtiTarget(botAI, "star", kaelthas);
        return Attack(kaelthas);
    }

    return false;
}

bool KaelthasSunstriderSpreadOutInMidairAction::Execute(Event /*event*/)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    constexpr float minSpreadDistance = 16.0f;

    std::vector<Player*> nearbyPlayers;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member == bot || !member->IsAlive())
            continue;

        if (bot->IsWithinDist3d(member, minSpreadDistance * 1.0f))
            nearbyPlayers.push_back(member);
    }

    if (nearbyPlayers.empty())
        return false;

    Player* closestPlayer = nullptr;
    float closestDist = std::numeric_limits<float>::max();
    for (Player* player : nearbyPlayers)
    {
        float distToPlayer = bot->GetExactDist(player);
        if (distToPlayer < closestDist)
        {
            closestDist = distToPlayer;
            closestPlayer = player;
        }
    }

    if (closestPlayer && closestDist < minSpreadDistance)
    {
        float angle = bot->GetAngle(closestPlayer) + M_PI;
        float distance = minSpreadDistance - closestDist;

        float x = bot->GetPositionX() + std::cos(angle) * distance;
        float y = bot->GetPositionY() + std::sin(angle) * distance;

        return MoveTo(TEMPEST_KEEP_MAP_ID, x, y, bot->GetPositionZ(), false, false,
                      false, true, MovementPriority::MOVEMENT_FORCED, true, false);
    }

    return false;
}
