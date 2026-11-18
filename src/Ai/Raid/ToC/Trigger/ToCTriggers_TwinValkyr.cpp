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
