/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "Aq40Strategy.h"
#include "Aq40Helpers.h"
#include "Aq40Multipliers.h"
#include "EncounterHelpers.h"
#include "Playerbots.h"
#include "Timer.h"

using namespace Aq40Helpers;

namespace
{
// Out of combat, just above follow (1.0) and below everything else, so summoning the Voidwalker, buffing and
// drinking still happen at the start spot.
constexpr float ACTION_PREPULL = ACTION_BG + 1.0f;
// In combat, below heals and the class rotation: a spot the bot can't reach must not cost it a cast.
constexpr float ACTION_POSITION = ACTION_LIGHT_HEAL - 1.0f;
}  // namespace

void RaidAq40Strategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // General
    triggers.push_back(new TriggerNode("aq40 no encounter in progress",
                                       {NextAction("aq40 reset encounter states", ACTION_EMERGENCY + 10)}));
    triggers.push_back(
        new TriggerNode("aq40 should track encounter", {NextAction("aq40 track encounter", ACTION_EMERGENCY + 11)}));
    triggers.push_back(
        new TriggerNode("aq40 should assign add tanks", {NextAction("aq40 assign add tanks", ACTION_EMERGENCY + 10)}));
    triggers.push_back(new TriggerNode("aq40 stuck midair after knockback",
                                       {NextAction("aq40 drop to ground", ACTION_EMERGENCY + 9)}));
    triggers.push_back(new TriggerNode("aq40 tank should taunt assigned target",
                                       {NextAction("aq40 tank taunt assigned target", ACTION_RAID + 4)}));
    triggers.push_back(new TriggerNode("aq40 no assigned target",
                                       {NextAction("aq40 stop attacking without assigned target", ACTION_RAID + 1)}));
    triggers.push_back(new TriggerNode("aq40 ranged should hold boss range",
                                       {NextAction("aq40 ranged move into boss range", ACTION_POSITION)}));

    // The Prophet Skeram
    triggers.push_back(new TriggerNode("the prophet skeram should assign platforms",
                                       {NextAction("the prophet skeram assign platforms", ACTION_EMERGENCY + 10)}));
    triggers.push_back(
        new TriggerNode("the prophet skeram raid member mind controlled",
                        {NextAction("the prophet skeram crowd control mind controlled", ACTION_RAID + 2)}));
    triggers.push_back(new TriggerNode("the prophet skeram should move to platform",
                                       {NextAction("the prophet skeram move to assigned platform", ACTION_RAID + 3)}));
    triggers.push_back(new TriggerNode("the prophet skeram should assign dps priority",
                                       {NextAction("the prophet skeram assign dps priority", ACTION_RAID + 1)}));

    // Bug Trio
    triggers.push_back(new TriggerNode("bug trio poison cloud nearby",
                                       {NextAction("bug trio move away from poison cloud", ACTION_EMERGENCY + 5)}));
    triggers.push_back(
        new TriggerNode("bug trio bosses too close", {NextAction("bug trio tanks separate bosses", ACTION_POSITION)}));
    triggers.push_back(new TriggerNode("bug trio should assign dps priority",
                                       {NextAction("bug trio assign dps priority", ACTION_RAID + 1)}));

    // Battleguard Sartura
    triggers.push_back(
        new TriggerNode("battleguard sartura whirlwind nearby",
                        {NextAction("battleguard sartura move away from whirlwind", ACTION_EMERGENCY + 5)}));
    triggers.push_back(new TriggerNode("battleguard sartura should assign dps priority",
                                       {NextAction("battleguard sartura assign dps priority", ACTION_RAID + 1)}));

    // Fankriss the Unyielding
    triggers.push_back(new TriggerNode("fankriss the unyielding should swap tanks",
                                       {NextAction("fankriss the unyielding swap tanks", ACTION_EMERGENCY + 10)}));
    triggers.push_back(new TriggerNode("fankriss the unyielding should assign dps priority",
                                       {NextAction("fankriss the unyielding assign dps priority", ACTION_RAID + 1)}));

    // Viscidus
    triggers.push_back(new TriggerNode("viscidus toxin cloud nearby",
                                       {NextAction("viscidus escape toxin clouds", ACTION_EMERGENCY + 5)}));
    triggers.push_back(
        new TriggerNode("viscidus should be frozen", {NextAction("viscidus cast frost spells", ACTION_RAID + 2)}));
    triggers.push_back(new TriggerNode("viscidus is frozen", {NextAction("viscidus melee shatter", ACTION_RAID + 2)}));
    triggers.push_back(new TriggerNode("viscidus frozen out of melee range",
                                       {NextAction("viscidus melee move in to shatter", ACTION_RAID + 3)}));
    triggers.push_back(new TriggerNode("viscidus ranged should hold ring",
                                       {NextAction("viscidus position ranged on ring", ACTION_POSITION)}));
    triggers.push_back(new TriggerNode("viscidus should assign dps priority",
                                       {NextAction("viscidus assign dps priority", ACTION_RAID + 1)}));

    // Princess Huhuran
    triggers.push_back(
        new TriggerNode("princess huhuran should assign soakers",
                        {NextAction("princess huhuran assign poison bolt soakers", ACTION_EMERGENCY + 10)}));
    triggers.push_back(new TriggerNode("princess huhuran should swap tanks",
                                       {NextAction("princess huhuran swap tanks", ACTION_EMERGENCY + 10)}));
    triggers.push_back(new TriggerNode("princess huhuran soaker too far",
                                       {NextAction("princess huhuran soakers stand near boss", ACTION_RAID + 3)}));
    triggers.push_back(new TriggerNode("princess huhuran ranged should spread",
                                       {NextAction("princess huhuran spread ranged and healers", ACTION_RAID + 3)}));
    triggers.push_back(new TriggerNode("princess huhuran frenzied",
                                       {NextAction("princess huhuran tranquilizing shot", ACTION_RAID + 2)}));
    triggers.push_back(new TriggerNode("princess huhuran wyvern sting on tank",
                                       {NextAction("princess huhuran cure wyvern sting", ACTION_RAID + 2)}));
    triggers.push_back(new TriggerNode("princess huhuran should assign dps priority",
                                       {NextAction("princess huhuran assign dps priority", ACTION_RAID + 1)}));

    // Twin Emperors
    triggers.push_back(new TriggerNode("twin emperors warlock should take start spot",
                                       {NextAction("twin emperors warlock move to start spot", ACTION_PREPULL)}));
    triggers.push_back(new TriggerNode("twin emperors should assign roles",
                                       {NextAction("twin emperors assign tanks and sides", ACTION_EMERGENCY + 10)}));
    triggers.push_back(
        new TriggerNode("twin emperors exploding bug nearby",
                        {NextAction("twin emperors move away from exploding bug", ACTION_EMERGENCY + 5)}));
    triggers.push_back(new TriggerNode("twin emperors in blizzard",
                                       {NextAction("twin emperors escape blizzard", ACTION_EMERGENCY + 5)}));
    triggers.push_back(new TriggerNode("emperor vek'lor arcane burst range",
                                       {NextAction("twin emperors move away from vek'lor", ACTION_EMERGENCY + 6)}));
    triggers.push_back(
        new TriggerNode("emperor vek'nilash choosing target after teleport",
                        {NextAction("twin emperors clear vek'nilash landing spot", ACTION_EMERGENCY + 7)}));
    triggers.push_back(new TriggerNode("twin emperors emperor cannot reach target",
                                       {NextAction("twin emperors lead emperor to platform", ACTION_EMERGENCY + 3)}));
    triggers.push_back(new TriggerNode("twin emperors non-tank has emperor aggro",
                                       {NextAction("twin emperors drop emperor threat", ACTION_EMERGENCY + 4)}));
    triggers.push_back(
        new TriggerNode("emperor vek'nilash hitting tank",
                        {NextAction("emperor vek'nilash tank use defensive cooldown", ACTION_EMERGENCY + 3)}));
    triggers.push_back(
        new TriggerNode("emperor vek'lor warlock tank low health",
                        {NextAction("emperor vek'lor warlock voidwalker sacrifice", ACTION_EMERGENCY + 3)}));
    triggers.push_back(new TriggerNode("emperor vek'lor held by wrong player",
                                       {NextAction("emperor vek'lor waiting tank taunt boss", ACTION_RAID + 4)}));
    triggers.push_back(new TriggerNode("twin emperors tanks should hold platforms",
                                       {NextAction("twin emperors tanks position emperors", ACTION_RAID + 3)}));
    triggers.push_back(new TriggerNode("emperor vek'lor warlock tank out of range",
                                       {NextAction("emperor vek'lor warlock tank keep range", ACTION_RAID + 3)}));
    triggers.push_back(new TriggerNode("twin emperors teleport imminent",
                                       {NextAction("twin emperors melee leave landing spots", ACTION_RAID + 3)}));
    triggers.push_back(new TriggerNode("twin emperors dps should position",
                                       {NextAction("twin emperors dps move to assigned emperor", ACTION_RAID + 3)}));
    triggers.push_back(
        new TriggerNode("twin emperors healers should position",
                        {NextAction("twin emperors healers position between tank and warlock", ACTION_RAID + 3)}));
    triggers.push_back(new TriggerNode("twin emperors healer should focus emperor targets",
                                       {NextAction("twin emperors heal emperor targets", ACTION_RAID + 2)}));
    triggers.push_back(new TriggerNode("twin emperors warlock pet active",
                                       {NextAction("twin emperors warlock set pet passive", ACTION_RAID + 2)}));
    triggers.push_back(new TriggerNode("twin emperors warlock waiting",
                                       {NextAction("twin emperors warlock prepare for vek'lor", ACTION_RAID + 2)}));
    triggers.push_back(
        new TriggerNode("emperor vek'lor should be tanked by warlock",
                        {NextAction("emperor vek'lor warlock tank cast searing pain", ACTION_RAID + 2)}));
    triggers.push_back(new TriggerNode("emperor vek'lor tank in shout range",
                                       {NextAction("emperor vek'lor tank demoralizing shout", ACTION_RAID + 2)}));
    triggers.push_back(new TriggerNode("twin emperors should assign dps priority",
                                       {NextAction("twin emperors assign dps priority", ACTION_RAID + 1)}));

    // Ouro
    triggers.push_back(
        new TriggerNode("ouro frontal cone", {NextAction("ouro move out of frontal cone", ACTION_EMERGENCY + 5)}));
    triggers.push_back(
        new TriggerNode("ouro dirt mound nearby", {NextAction("ouro avoid dirt mound", ACTION_EMERGENCY + 5)}));
    triggers.push_back(new TriggerNode("ouro should accept new main tank",
                                       {NextAction("ouro accept new main tank", ACTION_EMERGENCY + 10)}));
    triggers.push_back(
        new TriggerNode("ouro should assign dps priority", {NextAction("ouro assign dps priority", ACTION_RAID + 1)}));

    // C'Thun
    triggers.push_back(new TriggerNode("c'thun puller should walk in",
                                       {NextAction("c'thun puller move to pull spot", ACTION_PREPULL)}));
    triggers.push_back(
        new TriggerNode("c'thun raid should trail puller", {NextAction("c'thun trail puller", ACTION_PREPULL)}));
    triggers.push_back(new TriggerNode("eye of c'thun should track dark glare",
                                       {NextAction("eye of c'thun track dark glare", ACTION_EMERGENCY + 10)}));
    triggers.push_back(
        new TriggerNode("c'thun should assign spots", {NextAction("c'thun assign spots", ACTION_EMERGENCY + 10)}));
    triggers.push_back(new TriggerNode("c'thun should assign tentacle killers",
                                       {NextAction("c'thun assign tentacle killers", ACTION_EMERGENCY + 10)}));
    triggers.push_back(new TriggerNode("c'thun should assign giant claw tanks",
                                       {NextAction("c'thun assign giant claw tanks", ACTION_EMERGENCY + 10)}));
    triggers.push_back(new TriggerNode("c'thun on spit-out circle",
                                       {NextAction("c'thun send spit-out trigger", ACTION_EMERGENCY + 9)}));
    triggers.push_back(
        new TriggerNode("c'thun should leave stomach", {NextAction("c'thun exit stomach", ACTION_EMERGENCY + 8)}));
    triggers.push_back(new TriggerNode("eye of c'thun dark glare incoming",
                                       {NextAction("eye of c'thun dodge dark glare", ACTION_EMERGENCY + 6)}));
    triggers.push_back(new TriggerNode("eye of c'thun puller holding",
                                       {NextAction("eye of c'thun puller hold pull spot", ACTION_RAID + 3)}));
    triggers.push_back(new TriggerNode("eye of c'thun out of assigned spot",
                                       {NextAction("eye of c'thun move to assigned spot", ACTION_RAID + 3)}));
    triggers.push_back(new TriggerNode("c'thun out of stack", {NextAction("c'thun move to stack", ACTION_RAID + 3)}));
    triggers.push_back(new TriggerNode("c'thun melee should close in for burn",
                                       {NextAction("c'thun melee move into burn range", ACTION_RAID + 3)}));
    triggers.push_back(new TriggerNode("c'thun melee out of burn range",
                                       {NextAction("c'thun melee hold until in burn range", ACTION_RAID + 1)}));
    triggers.push_back(new TriggerNode("c'thun healers should damage c'thun",
                                       {NextAction("c'thun healers damage c'thun", ACTION_RAID + 2)}));
    triggers.push_back(new TriggerNode("c'thun healers should damage flesh tentacles",
                                       {NextAction("c'thun healers damage flesh tentacles", ACTION_RAID + 2)}));
    triggers.push_back(new TriggerNode("c'thun should assign dps priority",
                                       {NextAction("c'thun assign dps priority", ACTION_RAID + 1)}));
}

void RaidAq40Strategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    // Shared
    multipliers.push_back(new Aq40WaitForKnockbackMultiplier(botAI));
    multipliers.push_back(new Aq40DontAttackMindControlledMultiplier(botAI));
    multipliers.push_back(new Aq40ControlTauntingMultiplier(botAI));
    multipliers.push_back(new Aq40TanksDisableTankAssistMultiplier(botAI));
    multipliers.push_back(new Aq40DisableAutoTargetMultiplier(botAI));
    multipliers.push_back(new Aq40AllowedDamageMultiplier(botAI));
    multipliers.push_back(new Aq40DisableAoeMultiplier(botAI));
    multipliers.push_back(new Aq40MaintainFormationMultiplier(botAI));

    // Trash
    multipliers.push_back(new AnubisathDefenderAvoidReflectedSchoolsMultiplier(botAI));

    // The Prophet Skeram
    multipliers.push_back(new SkeramStayOnPlatformMultiplier(botAI));

    // Viscidus
    multipliers.push_back(new ViscidusFrostOnlyMultiplier(botAI));

    // Princess Huhuran
    multipliers.push_back(new PrincessHuhuranControlMovementMultiplier(botAI));
    multipliers.push_back(new PrincessHuhuranControlWyvernStingDispelMultiplier(botAI));
    multipliers.push_back(new PrincessHuhuranDelayDpsCooldownsMultiplier(botAI));

    // Twin Emperors
    multipliers.push_back(new TwinEmperorsOpeningHealHoldMultiplier(botAI));
    multipliers.push_back(new TwinEmperorsPriestShieldOnlyEmperorTargetsMultiplier(botAI));
    multipliers.push_back(new TwinEmperorsHoldDamageInTransitMultiplier(botAI));
    multipliers.push_back(new TwinEmperorsSkipImmuneTargetsMultiplier(botAI));
    multipliers.push_back(new TwinEmperorsVeklorThreatCapMultiplier(botAI));
    multipliers.push_back(new TwinEmperorsSpareTankThreatCapMultiplier(botAI));
    multipliers.push_back(new TwinEmperorsTanksHoldAnchorMultiplier(botAI));

    // C'Thun
    multipliers.push_back(new CthunDisableFollowMultiplier(botAI));
    multipliers.push_back(new CthunStomachExitOnlyMultiplier(botAI));
    multipliers.push_back(new CthunStomachNoTreeOfLifeMultiplier(botAI));
    multipliers.push_back(new CthunStomachNoFloorHealsMultiplier(botAI));
    multipliers.push_back(new CthunStomachHoldMovementMultiplier(botAI));
    multipliers.push_back(new CthunMeleeBurnRangeMultiplier(botAI));
}

void RaidAq40Strategy::AppendTargetExclusions(GuidSet& exclusions, TargetValueExclusionType type)
{
    Player* bot = botAI->GetBot();
    if (type != TargetValueExclusionType::Dps || !EncounterHelpers::IsEncounterInProgress(bot, AQ40_MAP_ID))
        return;
    // Target selection can ask for exclusions several times per tick; reuse them within the tick.
    uint32 const now = getMSTime();
    if (now != _exclusionsTime)
    {
        _exclusionsTime = now;
        _exclusions.clear();
        Unit* target = GetAssignedTarget(botAI);
        for (Creature* unit : GetUnits(bot))
            if (unit != target || !IsAllowedDamage(bot, unit))
                _exclusions.insert(unit->GetGUID());
        for (Player* player : GetMembers(bot))
            if (HasTrueFulfillment(player))
                _exclusions.insert(player->GetGUID());
    }
    exclusions.insert(_exclusions.begin(), _exclusions.end());
}
