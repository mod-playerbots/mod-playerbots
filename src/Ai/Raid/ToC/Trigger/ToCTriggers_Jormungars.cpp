/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "ToCTriggers_Jormungars.h"
#include "ToCHelpers_Jormungars.h"
#include "ToCHelpers_NorthrendBeasts.h"
#include "Playerbots.h"
#include "Strategy.h"

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

void AddToCJormungarsTriggerNodes(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("northrend worms mobile tank duty", {
        NextAction("northrend worms tank hold mobile worm", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("northrend worms stationary tank duty", {
        NextAction("northrend worms tank hold stationary worm", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("northrend worms redirect threat", {
        NextAction("northrend worms redirect threat", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("northrend worms dps target", {
        NextAction("northrend worms dps target", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode("northrend worms misplaced", {
        NextAction("northrend worms reposition", ACTION_EMERGENCY + 6) }));

    triggers.push_back(new TriggerNode("northrend worms pool after worms", {
        NextAction("northrend worms leave pool", ACTION_EMERGENCY + 6) }));

    triggers.push_back(new TriggerNode("northrend worms toxin stuck", {
        NextAction("northrend worms purge toxin", ACTION_EMERGENCY + 5) }));

    // Above the staging and breath stand walks, which hold the tick while in flight: on the move only the
    // instants go out
    triggers.push_back(new TriggerNode("northrend worms toxin carrier hurt", {
        NextAction("northrend worms heal toxin carrier", ACTION_RAID + 3) }));
}
