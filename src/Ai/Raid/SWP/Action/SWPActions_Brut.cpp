/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "SWPActions.h"
#include "SWPEncounter_Brut.h"
#include "Playerbots.h"
#include "RaidBossHelpers.h"
#include "SWPData.h"
#include <array>
#include <cmath>

using namespace SwpHelpers;

bool BrutallusMisdirectBossToMainTankAction::Execute(Event /*event*/)
{
    Unit* brutallus = AI_VALUE2(Unit*, "find target", "brutallus");
    if (!brutallus)
        return false;

    Player* mainTank = GetGroupMainTank(botAI, bot);
    if (!mainTank)
        return false;

    if (botAI->CanCastSpell("misdirection", mainTank))
        return botAI->CastSpell("misdirection", mainTank);

    if (bot->HasAura(Id(SwpSpells::SPELL_MISDIRECTION)) &&
        botAI->CanCastSpell("steady shot", brutallus))
    {
        return botAI->CastSpell("steady shot", brutallus);
    }

    return false;
}

bool BrutallusTanksHandleBossAction::Execute(Event event)
{
    Unit* brutallus = AI_VALUE2(Unit*, "find target", "brutallus");
    if (!brutallus)
        return false;

    if (AI_VALUE(Unit*, "current target") != brutallus)
        return Attack(brutallus);

    Player* mainTank = GetGroupMainTank(botAI, bot);
    Player* assistTank = GetGroupAssistTank(botAI, bot, 0);

    if (!mainTank || !assistTank)
        return false;

    Aura* mainTankAura = mainTank->GetAura(Id(SwpSpells::SPELL_METEOR_SLASH));
    Aura* assistTankAura = assistTank->GetAura(Id(SwpSpells::SPELL_METEOR_SLASH));

    if (mainTank == bot)
    {
        if (brutallus->GetVictim() != bot && !mainTankAura &&
            ((assistTankAura && assistTankAura->GetStackAmount() >= 3) || !assistTankAura))
        {
            return botAI->DoSpecificAction("taunt spell", event, true);
        }

        Position const& position = BRUTALLUS_MAIN_TANK_POSITION;
        float const distToPosition = bot->GetExactDist2d(position);

        if (_mainTankInitialPositionReached == false && distToPosition <= 2.0f)
        {
            _mainTankInitialPositionReached = true;
        }
        else if (_mainTankInitialPositionReached == false)
        {
            if (!bot->IsWithinMeleeRange(brutallus))
                return false;

            float const posX = position.GetPositionX();
            float const posY = position.GetPositionY();
            float const botX = bot->GetPositionX();
            float const botY = bot->GetPositionY();
            float const toPosX = posX - botX;
            float const toPosY = posY - botY;

            float const moveDist = std::min(2.25f, distToPosition);
            float const moveX = botX + (toPosX / distToPosition) * moveDist;
            float const moveY = botY + (toPosY / distToPosition) * moveDist;

            return MoveTo(
                SWP_MAP_ID, moveX, moveY, position.GetPositionZ(), false, false,
                false, false, MovementPriority::MOVEMENT_COMBAT, true, true);
        }
    }
    else if (assistTank == bot)
    {
        if (brutallus->GetVictim() != bot && !assistTankAura &&
            mainTankAura && mainTankAura->GetStackAmount() >= 3)
        {
            return botAI->DoSpecificAction("taunt spell", event, true);
        }

        float const mainTankAngle = Position::NormalizeOrientation(std::atan2(
            mainTank->GetPositionY() - brutallus->GetPositionY(),
            mainTank->GetPositionX() - brutallus->GetPositionX()));

        float const assistTankAngle = Position::NormalizeOrientation(
            mainTankAngle + BRUTALLUS_ASSIST_TANK_ANGLE_OFFSET);

        Position const position = GetBrutallusPositionAtAngle(
            bot, brutallus, assistTankAngle, BRUTALLUS_TANK_POSITION_RADIUS);
        if (bot->GetExactDist2d(position) <= 2.0f)
            return false;

        return MoveTo(
            SWP_MAP_ID, position.GetPositionX(), position.GetPositionY(),
            position.GetPositionZ(), false, false, false, false,
            MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    return false;
}

bool BrutallusPositionMeleeAction::Execute(Event /*event*/)
{
    Unit* brutallus = AI_VALUE2(Unit*, "find target", "brutallus");
    if (!brutallus)
        return false;

    Player* mainTank = GetGroupMainTank(botAI, bot);
    Player* assistTank = GetGroupAssistTank(botAI, bot, 0);

    uint8 meleeIndex = 0;
    if (!TryGetBrutallusAssignedPositionIndex(bot, meleeIndex))
        return false;

    Position position;
    if (!TryGetBrutallusMeleePosition(brutallus, mainTank, assistTank, meleeIndex, position))
        return false;

    if (bot->GetExactDist2d(position) <= 0.5f)
        return false;

    return MoveTo(
        SWP_MAP_ID, position.GetPositionX(), position.GetPositionY(), position.GetPositionZ(),
        false, false, false, true, MovementPriority::MOVEMENT_COMBAT, true, false);
}

bool BrutallusPositionMeleeAction::TryGetBrutallusMeleePosition(
    Unit* brutallus, Player* mainTank, Player* assistTank, uint8 meleeIndex, Position& position)
{
    struct BrutallusMeleeRingLayout
    {
        float radius;
        uint8 slotCount;
    };

    static constexpr std::array meleeRingLayouts = {
        BrutallusMeleeRingLayout{ BRUTALLUS_INNERMOST_MELEE_RADIUS, BRUTALLUS_INNERMOST_MELEE_POSITIONS },
        BrutallusMeleeRingLayout{ BRUTALLUS_INNER_MELEE_RADIUS, BRUTALLUS_INNER_MELEE_POSITIONS },
        BrutallusMeleeRingLayout{ BRUTALLUS_OUTER_MELEE_RADIUS, BRUTALLUS_OUTER_MELEE_POSITIONS },
        BrutallusMeleeRingLayout{ BRUTALLUS_OUTERMOST_MELEE_RADIUS, BRUTALLUS_OUTERMOST_MELEE_POSITIONS },
    };

    uint8 totalMeleeSlots = 0;
    for (auto const& meleeRingLayout : meleeRingLayouts)
        totalMeleeSlots += meleeRingLayout.slotCount;

    if (meleeIndex >= totalMeleeSlots)
        return false;

    float meleeRadius = 0.0f;
    uint8 localMeleeIndex = meleeIndex;
    uint8 maxMeleeSlots = 0;
    for (auto const& meleeRingLayout : meleeRingLayouts)
    {
        if (localMeleeIndex < meleeRingLayout.slotCount)
        {
            meleeRadius = meleeRingLayout.radius;
            maxMeleeSlots = meleeRingLayout.slotCount;
            break;
        }

        localMeleeIndex -= meleeRingLayout.slotCount;
    }

    if (!maxMeleeSlots)
        return false;

    float const mainTankAngle =
        GetBrutallusTankAngle(brutallus, mainTank, GetBrutallusMainTankAngle(brutallus));

    float midpointAngle;
    if (!mainTank || !assistTank)
    {
        midpointAngle = Position::NormalizeOrientation(
            mainTankAngle + BRUTALLUS_ASSIST_TANK_ANGLE_OFFSET / 2.0f);
    }
    else
    {
        float const assistTankAngle = GetBrutallusTankAngle(
            brutallus, assistTank, Position::NormalizeOrientation(
                mainTankAngle + BRUTALLUS_ASSIST_TANK_ANGLE_OFFSET));

        float const midpointX =
            (mainTank->GetPositionX() + assistTank->GetPositionX()) / 2.0f;
        float const midpointY =
            (mainTank->GetPositionY() + assistTank->GetPositionY()) / 2.0f;

        if (brutallus->GetExactDist2d(midpointX, midpointY) <= 0.1f)
        {
            float assistAngleDelta =
                Position::NormalizeOrientation(assistTankAngle - mainTankAngle);
            if (assistAngleDelta > static_cast<float>(M_PI))
                assistAngleDelta -= 2.0f * static_cast<float>(M_PI);

            midpointAngle = Position::NormalizeOrientation(
                mainTankAngle + assistAngleDelta / 2.0f);
        }
        else
        {
            midpointAngle = Position::NormalizeOrientation(std::atan2(
                midpointY - brutallus->GetPositionY(),
                midpointX - brutallus->GetPositionX()));
        }
    }

    float const baseAngle = Position::NormalizeOrientation(midpointAngle + M_PI);
    float const angleOffset = GetBrutallusCenteredArcSlotAngleOffset(
        localMeleeIndex, maxMeleeSlots, BRUTALLUS_SHARED_SAFE_MELEE_ARC_WIDTH);

    float const angle = Position::NormalizeOrientation(baseAngle + angleOffset);
    position = GetBrutallusPositionAtAngle(bot, brutallus, angle, meleeRadius);
    return true;
}

bool BrutallusPositionRangedAction::Execute(Event /*event*/)
{
    Unit* brutallus = AI_VALUE2(Unit*, "find target", "brutallus");
    if (!brutallus)
        return false;

    Player* mainTank = GetGroupMainTank(botAI, bot);
    Player* assistTank = GetGroupAssistTank(botAI, bot, 0);

    ObjectGuid const guid = bot->GetGUID();
    uint8 rangedIndex = 0;
    if (!TryGetBrutallusAssignedPositionIndex(bot, rangedIndex))
        return false;

    auto const burnStateItr = brutallusRangedBurnStates.find(guid);
    BrutallusRangedBurnState burnState = BrutallusRangedBurnState::None;
    if (burnStateItr != brutallusRangedBurnStates.end())
        burnState = burnStateItr->second;

    if (burnState == BrutallusRangedBurnState::MovingToInnerLane)
    {
        ReleaseBrutallusBurnPad(bot);
        brutallusRangedBurnStates.erase(guid);
        burnState = BrutallusRangedBurnState::None;
    }
    else if (burnState == BrutallusRangedBurnState::TraversingInnerLane ||
        burnState == BrutallusRangedBurnState::MovingToBurnPosition ||
        burnState == BrutallusRangedBurnState::AtBurnPosition)
    {
        burnState = BrutallusRangedBurnState::MovingToOuterLane;
        brutallusRangedBurnStates[guid] = burnState;
    }

    if (burnState == BrutallusRangedBurnState::MovingToOuterLane)
    {
        float const currentAngle = Position::NormalizeOrientation(std::atan2(
            bot->GetPositionY() - brutallus->GetPositionY(),
            bot->GetPositionX() - brutallus->GetPositionX()));

        Position const position = GetBrutallusPositionAtAngle(
            bot, brutallus, currentAngle, BRUTALLUS_OUTER_LANE_RADIUS);

        if (bot->GetExactDist2d(position) > 1.0f)
        {
            return MoveTo(
                SWP_MAP_ID, position.GetPositionX(), position.GetPositionY(),
                position.GetPositionZ(), false, false, false, true,
                MovementPriority::MOVEMENT_COMBAT, true, false);
        }

        brutallusRangedBurnStates[guid] = BrutallusRangedBurnState::TraversingOuterLane;
        return false;
    }

    if (burnState == BrutallusRangedBurnState::TraversingOuterLane)
    {
        Position returnTargetPosition;
        if (!TryGetBrutallusRangedPosition(
                bot, brutallus, mainTank, assistTank, rangedIndex,
                BRUTALLUS_OUTER_LANE_RADIUS, returnTargetPosition))
        {
            return false;
        }

        Position position;
        if (!TryGetBrutallusLaneTraversalPosition(
                bot, brutallus, returnTargetPosition.GetPositionX(),
                returnTargetPosition.GetPositionY(), BRUTALLUS_OUTER_LANE_RADIUS,
                bot->GetPositionX(), bot->GetPositionY(), position))
        {
            return false;
        }

        if (bot->GetExactDist2d(position) > 1.0f)
        {
            return MoveTo(
                SWP_MAP_ID, position.GetPositionX(), position.GetPositionY(),
                position.GetPositionZ(), false, false, false, true,
                MovementPriority::MOVEMENT_COMBAT, true, false);
        }

        if (bot->GetExactDist2d(returnTargetPosition) <= 1.0f)
            brutallusRangedBurnStates[guid] = BrutallusRangedBurnState::ReturningToNormalPosition;

        return false;
    }

    if (burnState == BrutallusRangedBurnState::ReturningToNormalPosition)
    {
        Position position;
        if (!TryGetBrutallusRangedPosition(
                bot, brutallus, mainTank, assistTank, rangedIndex,
                BRUTALLUS_NORMAL_RANGED_RADIUS, position))
        {
            return false;
        }

        if (bot->GetExactDist2d(position) > 1.0f)
        {
            return MoveTo(
                SWP_MAP_ID, position.GetPositionX(), position.GetPositionY(),
                position.GetPositionZ(), false, false, false, true,
                MovementPriority::MOVEMENT_COMBAT, true, false);
        }

        ReleaseBrutallusBurnPad(bot);
        brutallusRangedBurnStates.erase(guid);
        return false;
    }

    Position position;
    if (!TryGetBrutallusRangedPosition(
            bot, brutallus, mainTank, assistTank, rangedIndex,
            BRUTALLUS_NORMAL_RANGED_RADIUS, position))
    {
        return false;
    }

    if (bot->GetExactDist2d(position) < 0.5f)
        return false;

    return MoveTo(
        SWP_MAP_ID, position.GetPositionX(), position.GetPositionY(), position.GetPositionZ(),
        false, false, false, true, MovementPriority::MOVEMENT_COMBAT, true, false);
}

bool BrutallusHandleBurnAction::Execute(Event /*event*/)
{
    Unit* brutallus = AI_VALUE2(Unit*, "find target", "brutallus");
    if (!brutallus)
        return false;

    if (RemoveBurnWithCooldown())
        return true;

    if (PlayerbotAI::IsMelee(bot))
        return false;

    ObjectGuid const guid = bot->GetGUID();
    Player* mainTank = GetGroupMainTank(botAI, bot);
    Player* assistTank = GetGroupAssistTank(botAI, bot, 0);
    uint8 rangedIndex = 0;

    if (!TryGetBrutallusAssignedPositionIndex(bot, rangedIndex))
        return false;

    auto const burnStateItr = brutallusRangedBurnStates.find(guid);
    BrutallusRangedBurnState burnState = BrutallusRangedBurnState::None;
    if (burnStateItr != brutallusRangedBurnStates.end())
        burnState = burnStateItr->second;

    if (burnState == BrutallusRangedBurnState::MovingToOuterLane ||
        burnState == BrutallusRangedBurnState::TraversingOuterLane ||
        burnState == BrutallusRangedBurnState::ReturningToNormalPosition)
    {
        burnState = BrutallusRangedBurnState::MovingToInnerLane;
        brutallusRangedBurnStates[guid] = burnState;
    }

    if (burnState == BrutallusRangedBurnState::None)
    {
        burnState = BrutallusRangedBurnState::MovingToInnerLane;
        brutallusRangedBurnStates[guid] = burnState;
    }

    if (burnState == BrutallusRangedBurnState::MovingToInnerLane)
    {
        Position stepPosition;
        if (!TryGetBrutallusRangedPosition(
                bot, brutallus, mainTank, assistTank, rangedIndex,
                BRUTALLUS_INNER_LANE_RADIUS, stepPosition))
        {
            return false;
        }

        if (bot->GetExactDist2d(stepPosition) > 1.0f)
        {
            return MoveTo(
                SWP_MAP_ID, stepPosition.GetPositionX(), stepPosition.GetPositionY(),
                stepPosition.GetPositionZ(), false, false, false, true,
                MovementPriority::MOVEMENT_COMBAT, true, false);
        }

        brutallusRangedBurnStates[guid] = BrutallusRangedBurnState::TraversingInnerLane;
        return false;
    }

    if (burnState == BrutallusRangedBurnState::TraversingInnerLane)
    {
        Position padIngressPosition;
        if (!TryGetBrutallusBurnPadPosition(
                bot, brutallus, mainTank, rangedIndex,
                BRUTALLUS_INNER_LANE_RADIUS, padIngressPosition))
        {
            return false;
        }

        Position position;
        if (!TryGetBrutallusLaneTraversalPosition(
                bot, brutallus, padIngressPosition.GetPositionX(),
                padIngressPosition.GetPositionY(), BRUTALLUS_INNER_LANE_RADIUS,
                bot->GetPositionX(), bot->GetPositionY(), position))
        {
            return false;
        }

        if (bot->GetExactDist2d(position) > 1.0f)
        {
            return MoveTo(
                SWP_MAP_ID, position.GetPositionX(), position.GetPositionY(),
                position.GetPositionZ(), false, false, false, true,
                MovementPriority::MOVEMENT_COMBAT, true, false);
        }

        if (bot->GetExactDist2d(padIngressPosition) <= 1.0f)
            brutallusRangedBurnStates[guid] = BrutallusRangedBurnState::MovingToBurnPosition;

        return false;
    }

    Position position;
    if (!TryGetBrutallusBurnPadPosition(
            bot, brutallus, mainTank, rangedIndex, BRUTALLUS_BURN_PAD_RADIUS, position))
    {
        return false;
    }

    if (bot->GetExactDist2d(position) > 1.0f)
    {
        return MoveTo(
            SWP_MAP_ID, position.GetPositionX(), position.GetPositionY(), position.GetPositionZ(),
            false, false, false, true, MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    brutallusRangedBurnStates[guid] = BrutallusRangedBurnState::AtBurnPosition;

    return false;
}

bool BrutallusHandleBurnAction::RemoveBurnWithCooldown()
{
    switch (bot->getClass())
    {
        case CLASS_MAGE:
            return botAI->CanCastSpell("ice block", bot) &&
                botAI->CastSpell("ice block", bot);

        case CLASS_PALADIN:
            return botAI->CanCastSpell("divine shield", bot) &&
                botAI->CastSpell("divine shield", bot);

        case CLASS_ROGUE:
            return botAI->CanCastSpell("cloak of shadows", bot) &&
                botAI->CastSpell("cloak of shadows", bot);

        default:
            return false;
    }
}
