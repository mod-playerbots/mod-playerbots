/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "ToCTriggers_Jaraxxus.h"
#include "ToCData.h"
#include "ToCHelpers_Jaraxxus.h"
#include "Playerbots.h"
#include "PlayerbotAIConfig.h"
#include "RaidDefensive.h"
#include "RaidDirectTargeting.h"

using namespace TrialOfTheCrusaderHelpers;

bool JaraxxusFelFireballInterruptibleTrigger::IsActive()
{
    Unit* jaraxxus = GetJaraxxus(botAI);
    return jaraxxus && JaraxxusIsFelFireballInterrupter(bot, jaraxxus);
}

bool JaraxxusLegionFlameNearbyTrigger::IsActive()
{
    return GetJaraxxusFlameRole(bot) != JaraxxusFlameRole::None;
}

bool JaraxxusPinnedCastTrigger::IsActive()
{
    return !GetJaraxxusPinnedCasters(bot).empty();
}

bool JaraxxusPortalSpawnTrigger::IsActive()
{
    Unit* portal = GetJaraxxusPortalToClear(botAI);
    return portal && bot->GetExactDist2d(portal) < JARAXXUS_PORTAL_TRIGGER;
}

bool JaraxxusRaidCoverTrigger::IsActive()
{
    RaidPredictedHit hit;
    return GetRaidCover(botAI, TOC_DEFENSIVE_SCOPE, GetJaraxxusBurningInferno(botAI, hit) ? &hit : nullptr) !=
           RaidCover::None;
}

bool JaraxxusPersonalDefensiveTrigger::IsActive()
{
    RaidPredictedHit hit;
    return static_cast<bool>(
        GetRaidPersonalDefensive(botAI, TOC_DEFENSIVE_SCOPE, GetJaraxxusPersonalHit(botAI, hit) ? &hit : nullptr));
}

bool JaraxxusEngagedByMainTankTrigger::IsActive() { return JaraxxusMainTankHolds(botAI); }

bool JaraxxusAddNeedsAssistTankTrigger::IsActive() { return JaraxxusAssistTankHolds(botAI, 0); }

bool JaraxxusSecondAddNeedsAssistTankTrigger::IsActive() { return JaraxxusAssistTankHolds(botAI, 1); }

bool JaraxxusNetherPowerActiveTrigger::IsActive()
{
    // the aura test first: a healing remover walks the group
    return JaraxxusNetherPowerStacks(botAI) > 0 && JaraxxusIsNetherPowerRemover(bot);
}

bool JaraxxusDpsTargetTrigger::IsActive()
{
    Unit* target = GetJaraxxusDpsTarget(botAI);
    return target && (AI_VALUE(Unit*, "current target") != target || RaidPetOffPick(botAI, target));
}

bool JaraxxusIncinerateFleshOnRaidTrigger::IsActive()
{
    if (!PlayerbotAI::IsHeal(bot))
        return false;

    Unit* target = GetIncinerateFleshTarget(botAI);
    return target && bot->GetDistance(target) <= sPlayerbotAIConfig.HealDistance && bot->IsWithinLOSInMap(target);
}
