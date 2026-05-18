/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "RaidHyjalSummitMultipliers.h"
#include "RaidHyjalSummitActions.h"
#include "RaidHyjalSummitHelpers.h"
#include "AiFactory.h"
#include "ChooseTargetActions.h"
#include "DKActions.h"
#include "DruidBearActions.h"
#include "HunterActions.h"
#include "PaladinActions.h"
#include "RaidBossHelpers.h"
#include "ReachTargetActions.h"
#include "ShamanActions.h"
#include "WarriorActions.h"

using namespace HyjalSummitHelpers;

// Without this multiplier, Bloodlust/Heroism will not be available for
// bosses because it will be used on cooldown during trash waves
float HyjalSummitTimeBloodlustAndHeroismMultiplier::GetValue(Action* action)
{
    if (bot->getClass() != CLASS_SHAMAN)
        return 1.0f;

    if (dynamic_cast<CastBloodlustAction*>(action) ||
        dynamic_cast<CastHeroismAction*>(action))
    {
        Unit* archimonde = AI_VALUE2(Unit*, "find target", "archimonde");
        if (archimonde && archimonde->GetHealthPct() < 90.0f)
            return 1.0f;

        Unit* azgalor = AI_VALUE2(Unit*, "find target", "azgalor");
        if (azgalor && azgalor->GetHealthPct() < 90.0f)
            return 1.0f;

        Unit* kazrogal = AI_VALUE2(Unit*, "find target", "kaz'rogal");
        if (kazrogal && kazrogal->GetHealthPct() < 90.0f)
            return 1.0f;

        Unit* anetheron = AI_VALUE2(Unit*, "find target", "anetheron");
        if (anetheron && anetheron->GetHealthPct() < 85.0f)
            return 1.0f;

        Unit* winterchill = AI_VALUE2(Unit*, "find target", "rage winterchill");
        if (winterchill && winterchill->GetHealthPct() < 90.0f)
            return 1.0f;

        return 0.0f;
    }

    return 1.0f;
}

// Rage Winterchill

float RageWinterchillDisableCombatFormationMoveMultiplier::GetValue(Action* action)
{
    if (!AI_VALUE2(Unit*, "find target", "rage winterchill"))
        return 1.0f;

    if (dynamic_cast<CombatFormationMoveAction*>(action) &&
        !dynamic_cast<SetBehindTargetAction*>(action))
        return 0.0f;

    return 1.0f;
}

float RageWinterchillMeleeControlAvoidanceMultiplier::GetValue(Action* action)
{
    if (botAI->IsRanged(bot))
        return 1.0f;

    Unit* winterchill = AI_VALUE2(Unit*, "find target", "rage winterchill");
    if (!winterchill)
        return 1.0f;

    if (IsInDeathAndDecay(bot, DEATH_AND_DECAY_SAFE_RADIUS + 2.0f))
    {
        if (dynamic_cast<AvoidAoeAction*>(action))
            return 0.0f;

        if (botAI->IsMainTank(bot) || winterchill->GetVictim() == bot)
            return 1.0f;

        if (dynamic_cast<MovementAction*>(action) &&
            !dynamic_cast<RageWinterchillMeleeGetOutOfDeathAndDecayAction*>(action))
            return 0.0f;

        if (dynamic_cast<CastReachTargetSpellAction*>(action))
            return 0.0f;
    }

    return 1.0f;
}

// Anetheron

float AnetheronDisableTankActionsMultiplier::GetValue(Action* action)
{
    if (!botAI->IsTank(bot) || !AI_VALUE2(Unit*, "find target", "anetheron"))
        return 1.0f;

    if (dynamic_cast<AvoidAoeAction*>(action))
        return 0.0f;

    if (bot->GetVictim() != nullptr &&
        dynamic_cast<TankAssistAction*>(action))
        return 0.0f;

    return 1.0f;
}

float AnetheronDisableCombatFormationMoveMultiplier::GetValue(Action* action)
{
    if (!AI_VALUE2(Unit*, "find target", "anetheron"))
        return 1.0f;

    if (dynamic_cast<CombatFormationMoveAction*>(action) &&
        !dynamic_cast<SetBehindTargetAction*>(action))
        return 0.0f;

    return 1.0f;
}

float AnetheronControlMisdirectionMultiplier::GetValue(Action* action)
{
    if (bot->getClass() != CLASS_HUNTER ||
        !AI_VALUE2(Unit*, "find target", "anetheron"))
        return 1.0f;

    if (dynamic_cast<CastMisdirectionOnMainTankAction*>(action))
        return 0.0f;

    return 1.0f;
}

// Kaz'rogal

float KazrogalLowManaBotStayAwayFromGroupMultiplier::GetValue(Action* action)
{
    if (bot->getClass() == CLASS_WARRIOR || bot->getClass() == CLASS_ROGUE ||
        bot->getClass() == CLASS_DEATH_KNIGHT || bot->getClass() == CLASS_HUNTER)
        return 1.0f;

    uint8 tab = AiFactory::GetPlayerSpecTab(bot);
    if (bot->getClass() == CLASS_DRUID && tab == DRUID_TAB_FERAL)
        return 1.0f;

    if (!AI_VALUE2(Unit*, "find target", "kaz'rogal"))
        return 1.0f;

    if (!isBelowManaThreshold.count(bot->GetGUID()))
        return 1.0f;

    if (dynamic_cast<CastReachTargetSpellAction*>(action) ||
        (dynamic_cast<MovementAction*>(action) &&
         !dynamic_cast<AttackAction*>(action) &&
         !dynamic_cast<KazrogalLowManaBotTakeDefensiveMeasuresAction*>(action)))
        return 0.0f;

    return 1.0f;
}

float KazrogalKeepAspectOfTheViperActiveMultiplier::GetValue(Action* action)
{
    if (bot->getClass() != CLASS_HUNTER || bot->GetPower(POWER_MANA) > 4000 ||
        !AI_VALUE2(Unit*, "find target", "kaz'rogal"))
        return 1.0f;

    if (dynamic_cast<CastAspectOfTheHawkAction*>(action) ||
        dynamic_cast<CastAspectOfTheWildAction*>(action) ||
        dynamic_cast<CastAspectOfTheDragonhawkAction*>(action) ||
        dynamic_cast<CastAspectOfTheCheetahAction*>(action) ||
        dynamic_cast<CastAspectOfThePackAction*>(action) ||
        dynamic_cast<CastAspectOfTheMonkeyAction*>(action))
        return 0.0f;

    return 1.0f;
}

float KazrogalControlMovementMultiplier::GetValue(Action* action)
{
    if (!AI_VALUE2(Unit*, "find target", "kaz'rogal"))
        return 1.0f;

    if (dynamic_cast<CombatFormationMoveAction*>(action) &&
        !dynamic_cast<SetBehindTargetAction*>(action))
        return 0.0f;

    if (dynamic_cast<FleeAction*>(action))
        return 0.0f;

    if (botAI->IsRanged(bot) && dynamic_cast<ReachTargetAction*>(action))
        return 0.0f;

    return 1.0f;
}

// Azgalor

float AzgalorDisableTankActionsMultiplier::GetValue(Action* action)
{
    if (bot->GetVictim() == nullptr)
        return 1.0f;

    if (!botAI->IsTank(bot) || !AI_VALUE2(Unit*, "find target", "azgalor"))
        return 1.0f;

    if (dynamic_cast<TankFaceAction*>(action))
        return 0.0f;

    if (dynamic_cast<TankAssistAction*>(action) || dynamic_cast<AvoidAoeAction*>(action))
    {
        if (botAI->IsMainTank(bot))
        {
            return 0.0f;
        }
        else if (botAI->IsAssistTank(bot) && (AnyGroupMemberHasDoom(bot) ||
                 AI_VALUE2(Unit*, "find target", "lesser doomguard")))
        {
            return 0.0f;
        }
    }

    return 1.0f;
}

float AzgalorDoomedBotPrioritizePositioningMultiplier::GetValue(Action* action)
{
    if (!bot->HasAura(static_cast<uint32>(HyjalSummitSpells::SPELL_DOOM)))
        return 1.0f;

    if (dynamic_cast<MovementAction*>(action) &&
        !dynamic_cast<AttackAction*>(action) &&
        !dynamic_cast<AvoidAoeAction*>(action) &&
        !dynamic_cast<AzgalorMoveToDoomguardTankAction*>(action))
        return 0.0f;

    return 1.0f;
}

float AzgalorMeleeDpsControlAvoidanceMultiplier::GetValue(Action* action)
{
    if (botAI->IsRanged(bot) || botAI->IsTank(bot))
        return 1.0f;

    Unit* azgalor = AI_VALUE2(Unit*, "find target", "azgalor");
    if (!azgalor)
        return 1.0f;

    constexpr float singleTickMoveAwayDist = 6.0f;
    if (IsInRainOfFire(bot, RAIN_OF_FIRE_RADIUS + singleTickMoveAwayDist))
    {
        if (dynamic_cast<AvoidAoeAction*>(action) ||
            dynamic_cast<CastReachTargetSpellAction*>(action))
            return 0.0f;

        if (dynamic_cast<MovementAction*>(action) &&
            !dynamic_cast<AzgalorMeleeGetOutOfFireAndSwapTargetsAction*>(action))
            return 0.0f;
    }

    Player* mainTank = GetGroupMainTank(botAI, bot);
    if (!mainTank || !GET_PLAYERBOT_AI(mainTank))
        return 1.0f;

    TankPositionState tankState = GetAzgalorTankPositionState(botAI, bot);
    if ((tankState == TankPositionState::Unknown ||
         tankState == TankPositionState::MovingToTransition) &&
         dynamic_cast<MovementAction*>(action) &&
         !dynamic_cast<AzgalorWaitAtSafePositionAction*>(action))
    {
        return 0.0f;
    }

    return 1.0f;
}

// Archimonde

float ArchimondeDisableCombatFormationMoveMultiplier::GetValue(Action* action)
{
    if (!AI_VALUE2(Unit*, "find target", "archimonde"))
        return 1.0f;

    if (dynamic_cast<CombatFormationMoveAction*>(action) &&
        !dynamic_cast<SetBehindTargetAction*>(action))
        return 0.0f;

    return 1.0f;
}
