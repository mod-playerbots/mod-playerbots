/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "ToCTriggers_FactionChampions.h"
#include "ToCHelpers_FactionChampions.h"
#include "ToCHelpers_FactionChampionsDefence.h"
#include "Playerbots.h"
#include "RaidDirectTargeting.h"
#include "Strategy.h"

using namespace TrialOfTheCrusaderHelpers;

bool FactionChampionsAntiFearTrigger::FearWindowActive() { return FactionChampionsFearWindowActive(botAI); }

bool FactionChampionsShouldFocusTrigger::IsActive()
{
    if (!FactionChampionsFocusBot(botAI))
        return false;

    Unit* killTarget = FactionChampionsKillTarget(botAI);
    return killTarget &&
           (AI_VALUE(Unit*, "current target") != killTarget || RaidPetOffPick(botAI, killTarget));
}

bool FactionChampionsKillTargetHealingTrigger::IsActive() { return FactionChampionsCounterspellDuty(botAI); }

bool FactionChampionsPetAutocastTrigger::IsActive()
{
    bool enable = false;
    return FactionChampionsPetAutocastToggle(botAI, enable) != 0;
}

void AddToCFactionChampionsTriggerNodes(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("faction champions anti fear", {
        NextAction("faction champions anti fear", ACTION_RAID + 3) }));

    triggers.push_back(new TriggerNode("faction champions should focus", {
        NextAction("faction champions focus priority", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode("faction champions kill target healing", {
        NextAction("faction champions counterspell kill target", ACTION_INTERRUPT + 3) }));

    triggers.push_back(new TriggerNode("faction champions pet autocast", {
        NextAction("faction champions pet autocast", ACTION_RAID + 6) }));
}
