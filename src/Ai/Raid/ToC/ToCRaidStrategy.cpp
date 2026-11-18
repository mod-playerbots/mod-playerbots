/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "ToCRaidStrategy.h"
#include "RaidDirectTargeting.h"
#include "ToCData.h"
#include "ToCDefinitions.h"
#include "ToCHelpers_FactionChampionsDefence.h"
#include "ToCHelpers_Staging.h"
#include "ToCMultipliers_Shared.h"

void RaidTrialOfTheCrusaderStrategy::OnTick()
{
    // before triggers, so the generic pull's multiplier never sees it armed
    TrialOfTheCrusaderHelpers::FactionChampionsTakePullRequest(botAI);
    TrialOfTheCrusaderHelpers::UpdateToCStagingPark(botAI);

    // once per strategy object, so a player's later rti order sticks until the strategy is rebuilt
    if (_legacyRtiChecked)
        return;

    _legacyRtiChecked = true;
    RaidRestoreLegacyRti(botAI, TrialOfTheCrusaderHelpers::TRIAL_OF_THE_CRUSADER_MAP_ID);
}

void RaidTrialOfTheCrusaderStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    for (EncounterDefinition const* encounter : ToCEncounterDefinitions())
        encounter->AddTriggerNodes(triggers);

    // the chat command's own trigger, which no row can register: a tank bot takes the pull too, and the
    // chain opens once either way
    triggers.push_back(new TriggerNode("pull", {
        NextAction("faction champions chain request", ACTION_RAID + 4.7f) }));
}

void RaidTrialOfTheCrusaderStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    for (EncounterDefinition const* encounter : ToCEncounterDefinitions())
        encounter->AddMultipliers(botAI, multipliers);

    multipliers.push_back(new RaidBurstWindowMultiplier(botAI, "toc burst window", &ToCLiveBurstWindow));
}
