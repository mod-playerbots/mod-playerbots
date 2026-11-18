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
#include "Strategy.h"

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

void AddToCIcehowlTriggerNodes(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("icehowl tank duty", {
        NextAction("icehowl tank hold boss", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("icehowl charge incoming", {
        NextAction("icehowl clear charge path", ACTION_EMERGENCY + 8) }));

    triggers.push_back(new TriggerNode("icehowl frothing rage", {
        NextAction("icehowl tank defensive", ACTION_RAID + 6) }));

    // Ahead of the walk to him, which then runs inside the boost
    triggers.push_back(new TriggerNode("icehowl daze nitro", {
        NextAction("icehowl daze nitro", ACTION_RAID + 3) }));

    // Ahead of the breath stand: its walk holds the tick until it lands, and this switch doesn't stop it
    triggers.push_back(new TriggerNode("icehowl dps target", {
        NextAction("icehowl dps target", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode("icehowl breath spread", {
        NextAction("icehowl move to breath stand", ACTION_RAID + 2) }));
}
