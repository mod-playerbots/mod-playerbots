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
#include "Strategy.h"

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

void AddToCJaraxxusTriggerNodes(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("jaraxxus fel fireball interruptible", {
        NextAction("jaraxxus interrupt fel fireball", ACTION_EMERGENCY + 9) }));

    // above the flame walk: an instant goes out on the move, and a carrier's walk holds every tick
    triggers.push_back(new TriggerNode("jaraxxus raid cover", {
        NextAction("jaraxxus cast raid cover", ACTION_EMERGENCY + 8.6f) }));

    triggers.push_back(new TriggerNode("jaraxxus personal defensive", {
        NextAction("jaraxxus cast personal defensive", ACTION_EMERGENCY + 8.4f) }));

    triggers.push_back(new TriggerNode("jaraxxus legion flame nearby", {
        NextAction("jaraxxus avoid legion flame", ACTION_EMERGENCY + 8) }));

    triggers.push_back(new TriggerNode("jaraxxus pinned cast", {
        NextAction("jaraxxus break pinned cast", ACTION_EMERGENCY + 7) }));

    triggers.push_back(new TriggerNode("jaraxxus portal spawn", {
        NextAction("jaraxxus leave portal", ACTION_EMERGENCY + 6) }));

    triggers.push_back(new TriggerNode("jaraxxus engaged by main tank", {
        NextAction("jaraxxus main tank hold boss", ACTION_RAID + 5) }));

    triggers.push_back(new TriggerNode("jaraxxus add needs assist tank", {
        NextAction("jaraxxus assist tank hold add", ACTION_RAID + 4) }));

    triggers.push_back(new TriggerNode("jaraxxus second add needs assist tank", {
        NextAction("jaraxxus assist tank hold second add", ACTION_RAID + 3) }));

    triggers.push_back(new TriggerNode("jaraxxus nether power active", {
        NextAction("jaraxxus remove nether power", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode("jaraxxus dps target", {
        NextAction("jaraxxus dps target", ACTION_RAID + 1) }));

    // health reads full under the absorb, so no generic heal picks the target. stays under critical heals
    triggers.push_back(new TriggerNode("jaraxxus incinerate flesh on raid", {
        NextAction("jaraxxus heal incinerate target", ACTION_MEDIUM_HEAL + 5) }));
}
