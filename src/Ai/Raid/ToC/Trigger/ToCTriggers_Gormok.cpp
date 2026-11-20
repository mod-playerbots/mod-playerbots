/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "ToCTriggers_Gormok.h"
#include "ToCHelpers_Gormok.h"
#include "ToCHelpers_NorthrendBeasts.h"
#include "Playerbots.h"
#include "RaidDirectTargeting.h"

using namespace TrialOfTheCrusaderHelpers;

bool GormokTankDutyTrigger::IsActive()
{
    return GetBeastsTankDuty(botAI) == BeastsTankDuty::Gormok &&
           GetEngagedBeast(botAI, NorthrendBeast::Gormok);
}

bool GormokSwapTankDutyTrigger::IsActive()
{
    if (GetBeastsTankDuty(botAI) != BeastsTankDuty::GormokSwap)
        return false;

    Unit* gormok = GetEngagedBeast(botAI, NorthrendBeast::Gormok);
    return gormok && (AI_VALUE(Unit*, "current target") != gormok || RaidPetOffPick(botAI, gormok));
}

bool GormokTankSwapNeededTrigger::IsActive()
{
    return GetGormokSwapTauntTarget(botAI) != nullptr;
}

bool GormokTankDefensiveTrigger::IsActive()
{
    return GormokTankNeedsDefensive(botAI);
}

bool GormokImpaleResetTrigger::IsActive()
{
    return GetGormokImpaleResetTarget(botAI) != nullptr;
}

bool GormokCancelHandOfProtectionTrigger::IsActive()
{
    return GormokHandOfProtectionToCancel(botAI);
}

bool GormokSnobolledTrigger::IsActive()
{
    return GetGormokForSnoboldCarrier(botAI) != nullptr;
}

bool GormokStompRangeTrigger::IsActive()
{
    return GetGormokStompThreat(botAI) != nullptr;
}

bool GormokFireBombIncomingTrigger::IsActive()
{
    return IsInFireBombImpact(botAI);
}
