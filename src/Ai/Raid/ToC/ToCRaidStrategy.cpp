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
#include "ToCRaidMultipliers.h"
#include "ToCRaidTriggers.h"

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

// Don't reorder: equal relevances pop in insertion order.
void RaidTrialOfTheCrusaderStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    AddToCGormokTriggerNodes(triggers);
    AddToCJormungarsTriggerNodes(triggers);
    AddToCIcehowlTriggerNodes(triggers);
    AddToCNorthrendBeastsTriggerNodes(triggers);
    AddToCJaraxxusTriggerNodes(triggers);
    AddToCAnubarakTriggerNodes(triggers);
    AddToCFactionChampionsTriggerNodes(triggers);
    AddToCFactionChampionsDefenceTriggerNodes(triggers);
    AddToCFactionChampionsBallTriggerNodes(triggers);
    AddToCTwinValkyrTriggerNodes(triggers);
    AddToCStagingTriggerNodes(triggers);
    AddToCResistAuraTriggerNodes(triggers);
    AddToCOpenerTriggerNodes(triggers);
}

// The engine stops at the first rule or multiplier that zeroes an action and credits only that one,
// so each encounter's rules follow its own stem multipliers.
void RaidTrialOfTheCrusaderStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    AddToCGormokMultipliers(botAI, multipliers);
    AddToCJormungarsMultipliers(botAI, multipliers);
    AddToCIcehowlMultipliers(botAI, multipliers);
    AddToCNorthrendBeastsMultipliers(botAI, multipliers);
    ToCNorthrendBeastsDefinition().AddMultipliers(botAI, multipliers);
    AddToCJaraxxusMultipliers(botAI, multipliers);
    ToCJaraxxusDefinition().AddMultipliers(botAI, multipliers);
    AddToCAnubarakMultipliers(botAI, multipliers);
    ToCAnubarakDefinition().AddMultipliers(botAI, multipliers);
    AddToCFactionChampionsMultipliers(botAI, multipliers);
    ToCFactionChampionsDefinition().AddMultipliers(botAI, multipliers);
    AddToCFactionChampionsDefenceMultipliers(botAI, multipliers);
    AddToCTwinValkyrMultipliers(botAI, multipliers);
    ToCTwinValkyrDefinition().AddMultipliers(botAI, multipliers);
    ToCStagingDefinition().AddMultipliers(botAI, multipliers);
    ToCResistAuraDefinition().AddMultipliers(botAI, multipliers);
    ToCOpenerDefinition().AddMultipliers(botAI, multipliers);
    multipliers.push_back(new RaidBurstWindowMultiplier(botAI, "toc burst window", &ToCLiveBurstWindow));
}
