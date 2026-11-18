/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "ToCTriggers_Icehowl.h"
#include "ToCHelpers_Icehowl.h"
#include "ToCHelpers_NorthrendBeasts.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"

using namespace TrialOfTheCrusaderHelpers;

bool IcehowlTankDutyTrigger::IsActive() { return GetBeastsTankDuty(botAI) == BeastsTankDuty::Icehowl; }

bool IcehowlChargeIncomingTrigger::IsActive()
{
    return DistanceToIcehowlCharge(botAI, bot->GetPositionX(), bot->GetPositionY()) < ICEHOWL_CHARGE_TRIGGER;
}

bool IcehowlFrothingRageTrigger::IsActive()
{
    Unit* icehowl = GetEngagedBeast(botAI, NorthrendBeast::Icehowl);
    return icehowl && icehowl->GetVictim() == bot && HasIcehowlFrothingRage(icehowl);
}

bool IcehowlBreathSpreadTrigger::IsActive()
{
    IcehowlBreathStand stand;
    if (!GetIcehowlBreathStand(botAI, stand) || !botAI->CanMove())
        return false;

    if (IsOnIcehowlBreathStand(stand, bot->GetPositionX(), bot->GetPositionY(), ICEHOWL_SPREAD_TRIGGER_DEG,
                               stand.trigger))
        return false;

    // A healer yields to its heal target or the two walks fight. "party member to heal out of spell range"
    // measured from the stand, since from the bot the reach walk's arrival would end the yield.
    if (PlayerbotAI::IsHeal(bot))
    {
        if (Unit* target = AI_VALUE(Unit*, "party member to heal"))
        {
            float const x = stand.spot.GetPositionX();
            float const y = stand.spot.GetPositionY();
            float const reach = botAI->GetRange("heal") + 1.0f + sPlayerbotAIConfig.ContactDistance;
            if (target->GetDistance2d(x, y) - bot->GetCombatReach() > reach ||
                !target->IsWithinLOS(x, y, stand.spot.GetPositionZ()))
                return false;
        }
    }

    return true;
}

bool IcehowlDazeNitroTrigger::IsActive() { return IcehowlDazeNitroDue(botAI); }
