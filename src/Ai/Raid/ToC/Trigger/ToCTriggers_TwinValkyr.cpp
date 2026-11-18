/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "ToCTriggers_TwinValkyr.h"
#include "ToCData.h"
#include "ToCEncounterGate.h"
#include "ToCHelpers_TwinValkyr.h"
#include "Playerbots.h"
#include "RaidCombatHelpers.h"
#include "RaidDefensive.h"
#include "RaidDirectTargeting.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include "Strategy.h"
#include "Unit.h"

using namespace TrialOfTheCrusaderHelpers;

namespace
{
constexpr uint32 SPELL_AUTO_SHOT = 75;

bool WantsEssenceFor(PlayerbotAI* botAI, Player* bot, TwinEssenceReason reason)
{
    TwinEssenceWant const want = GetWantedEssence(botAI);
    return want.reason == reason && TwinEssenceGossipDue(bot, want);
}

// the max range check Spell::CheckRange makes, Hawk Eye included
bool InAutoShotRange(Player* bot, Unit* target)
{
    SpellInfo const* info = sSpellMgr->GetSpellInfo(SPELL_AUTO_SHOT);
    if (!info)
        return false;

    float range = bot->GetSpellMaxRangeForTarget(target, info);
    bot->ApplySpellMod(info->Id, SPELLMOD_RANGE, range);
    return bot->IsWithinCombatRange(target, range);
}
}  // namespace

bool TwinValkyrPactInterruptDutyTrigger::IsActive()
{
    Unit* twin = GetTwinCastingPact(botAI);
    return twin && IsTwinPactInterrupter(botAI, twin);
}

bool TwinValkyrTouchedRequiresEssenceTrigger::IsActive()
{
    return WantsEssenceFor(botAI, bot, TwinEssenceReason::Touch);
}

bool TwinValkyrVortexRequiresEssenceTrigger::IsActive()
{
    return WantsEssenceFor(botAI, bot, TwinEssenceReason::Vortex);
}

bool TwinValkyrOrbIncomingTrigger::IsActive()
{
    return !TwinValkyrDodgeYields(botAI) && TwinOrbThreatens(botAI, TWIN_ORB_DODGE_CLEARANCE);
}

bool TwinValkyrVortexRaidCoverTrigger::IsActive()
{
    RaidPredictedHit hit;
    return GetRaidCover(botAI, TOC_DEFENSIVE_SCOPE, GetTwinVortexRaidHit(botAI, hit) ? &hit : nullptr) !=
           RaidCover::None;
}

bool TwinValkyrVortexDefensiveTrigger::IsActive()
{
    RaidPredictedHit hit;
    return static_cast<bool>(
        GetRaidPersonalDefensive(botAI, TOC_DEFENSIVE_SCOPE, GetTwinVortexHit(botAI, hit) ? &hit : nullptr));
}

bool TwinValkyrNextPortalDueTrigger::IsActive()
{
    Position spot;
    Creature* portal = nullptr;
    TwinEssenceReason reason = TwinEssenceReason::None;
    return TwinValkyrNextPortalDue(botAI, spot, portal, reason);
}

bool TwinValkyrNeedsBaseEssenceTrigger::IsActive()
{
    return WantsEssenceFor(botAI, bot, TwinEssenceReason::Base);
}

bool TwinValkyrEngagedByMainTankTrigger::IsActive()
{
    Unit* fjola = GetFjola(botAI);
    return IsTwinAttackable(fjola) && IsTwinTank(bot, fjola);
}

bool TwinValkyrDarkbaneNeedsAssistTankTrigger::IsActive()
{
    Unit* eydis = GetEydis(botAI);
    if (!IsTwinAttackable(eydis) || !IsTwinTank(bot, eydis))
        return false;

    // a lone tank holds both twins from the main tank node
    Unit* fjola = GetFjola(botAI);
    return !fjola || !IsTwinTank(bot, fjola);
}

bool TwinValkyrRedirectThreatTrigger::IsActive()
{
    if (bot->getClass() != CLASS_HUNTER && bot->getClass() != CLASS_ROGUE)
        return false;

    if (!ToCEncounterIsLive(botAI, ToCEncounter::TwinValkyr))
        return false;

    Unit* twin = GetTwinDpsTarget(botAI);
    if (!twin)
        return false;

    Player* tank = GetTwinTank(bot, twin);
    if (!tank || tank == bot || !tank->IsAlive())
        return false;

    // Misdirection's 4 s transfer starts on the hunter's next attack, Kill Command included. Cast
    // while the twins fly in from 90 yd and it runs out before the first shot.
    return bot->getClass() != CLASS_HUNTER || InAutoShotRange(bot, twin);
}

bool TwinValkyrDoorStandTrigger::IsActive() { return TwinValkyrDoorWalkDue(botAI); }

bool TwinValkyrDpsTargetTrigger::IsActive()
{
    if (!IsRaidDpsBot(bot) || !ToCEncounterIsLive(botAI, ToCEncounter::TwinValkyr))
        return false;

    Unit* twin = GetTwinDpsTarget(botAI);
    return twin && (AI_VALUE(Unit*, "current target") != twin || RaidPetOffPick(botAI, twin));
}

void AddToCTwinValkyrTriggerNodes(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("twin valkyr pact interrupt duty", {
        NextAction("twin valkyr interrupt pact", ACTION_EMERGENCY + 8) }));

    triggers.push_back(new TriggerNode("twin valkyr vortex requires essence", {
        NextAction("twin valkyr swap essence for vortex", ACTION_EMERGENCY + 7) }));

    // a Touch ticks only on the touched bot, and an essence of the Touch's colour absorbs it all
    triggers.push_back(new TriggerNode("twin valkyr touched requires essence", {
        NextAction("twin valkyr swap essence for touch", ACTION_EMERGENCY + 6) }));

    triggers.push_back(new TriggerNode("twin valkyr vortex raid cover", {
        NextAction("twin valkyr cast vortex raid cover", ACTION_EMERGENCY + 5) }));

    triggers.push_back(new TriggerNode("twin valkyr vortex defensive", {
        NextAction("twin valkyr cast vortex defensive", ACTION_EMERGENCY + 4) }));

    triggers.push_back(new TriggerNode("twin valkyr orb incoming", {
        NextAction("twin valkyr dodge orb", ACTION_EMERGENCY + 3) }));

    triggers.push_back(new TriggerNode("twin valkyr next portal due", {
        NextAction("twin valkyr walk to next portal", ACTION_EMERGENCY + 2) }));

    triggers.push_back(new TriggerNode("twin valkyr needs base essence", {
        NextAction("twin valkyr take base essence", ACTION_RAID + 5) }));

    triggers.push_back(new TriggerNode("twin valkyr engaged by main tank", {
        NextAction("twin valkyr main tank hold light twin", ACTION_RAID + 4) }));

    triggers.push_back(new TriggerNode("twin valkyr darkbane needs assist tank", {
        NextAction("twin valkyr assist tank hold dark twin", ACTION_RAID + 3) }));

    triggers.push_back(new TriggerNode("twin valkyr redirect threat", {
        NextAction("twin valkyr redirect threat", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode("twin valkyr door stand", {
        NextAction("twin valkyr move to door stand", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("twin valkyr dps target", {
        NextAction("twin valkyr focus twin", ACTION_RAID) }));
}
