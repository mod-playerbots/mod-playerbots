/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "ToCTriggers_Jormungars.h"
#include "ToCHelpers_Jormungars.h"
#include "ToCHelpers_NorthrendBeasts.h"
#include "Playerbots.h"

using namespace TrialOfTheCrusaderHelpers;

// Not IsMainTank: on heroic the main tank can be holding Gormok or Icehowl while the worms are up.
bool NorthrendWormsMobileTankDutyTrigger::IsActive()
{
    return GetBeastsTankDuty(botAI) == BeastsTankDuty::WormMobile;
}

bool NorthrendWormsStationaryTankDutyTrigger::IsActive()
{
    return GetBeastsTankDuty(botAI) == BeastsTankDuty::WormStationary;
}

bool NorthrendWormsRedirectThreatTrigger::IsActive()
{
    Player* holder = nullptr;
    return GetWormRedirectTarget(botAI, holder) != nullptr;
}

bool NorthrendWormsMisplacedTrigger::IsActive()
{
    WormMovePlan plan;
    return GetWormMovePlan(botAI, plan) && plan.reason != WormMoveReason::None;
}

bool NorthrendWormsPoolAfterWormsTrigger::IsActive() { return InSlimePoolWithoutWorms(botAI); }

bool NorthrendWormsToxinCarrierHurtTrigger::IsActive()
{
    if (!PlayerbotAI::IsHeal(bot))
        return false;

    Player* carrier = GetWormToxinHealTarget(botAI);
    return carrier && carrier->GetHealthPct() < WORM_TOXIN_HEAL_PCT &&
           bot->GetDistance(carrier) <= sPlayerbotAIConfig.HealDistance && bot->IsWithinLOSInMap(carrier);
}

bool NorthrendWormsToxinStuckTrigger::IsActive()
{
    char const* purge = GetWormToxinSelfPurge(botAI);
    return purge && botAI->CanCastSpell(purge, bot);
}
