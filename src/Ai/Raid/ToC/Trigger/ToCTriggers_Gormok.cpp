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
#include "Strategy.h"

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

void AddToCGormokTriggerNodes(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("gormok tank duty", {
        NextAction("gormok tank hold boss", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("gormok swap tank duty", {
        NextAction("gormok swap tank hold", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("gormok dps target", {
        NextAction("gormok dps target", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode("gormok snobolled", {
        NextAction("gormok bring snobold to melee", ACTION_RAID + 3) }));

    triggers.push_back(new TriggerNode("gormok stomp range", {
        NextAction("gormok leave stomp range", ACTION_RAID + 3) }));

    triggers.push_back(new TriggerNode("gormok tank swap needed", {
        NextAction("gormok tank swap taunt", ACTION_RAID + 5) }));

    triggers.push_back(new TriggerNode("gormok tank defensive", {
        NextAction("gormok tank defensive", ACTION_RAID + 6) }));

    triggers.push_back(new TriggerNode("gormok impale reset", {
        NextAction("gormok hand of protection", ACTION_RAID + 5) }));

    // Off the global cooldown, and the hand pacifies the tank till it goes
    triggers.push_back(new TriggerNode("gormok cancel hand of protection", {
        NextAction("gormok cancel hand of protection", ACTION_EMERGENCY + 1) }));

    triggers.push_back(new TriggerNode("gormok fire bomb incoming", {
        NextAction("gormok dodge fire bomb", ACTION_EMERGENCY + 2) }));
}
