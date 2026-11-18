/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "ToCTriggers_Anubarak.h"

#include "Creature.h"
#include "Map.h"
#include "Playerbots.h"
#include "RaidRedirectThreat.h"
#include "RaidTankDefensive.h"
#include "Strategy.h"
#include "ToCData.h"
#include "ToCHelpers_Anubarak.h"
#include "ToCHelpers_Shared.h"

using namespace TrialOfTheCrusaderHelpers;

namespace
{

bool IsAnubarakSurfaced(AnubarakPhase phase)
{
    return phase == AnubarakPhase::Surface || phase == AnubarakPhase::Swarm;
}

}

bool AnubarakKiterNeedsImmunityTrigger::IsActive()
{
    return AnubarakEngaged(botAI) && AnubarakKiterNeedsImmunity(bot);
}

// A held dodge still counts as gathered for the gather's movement block, it just isn't walked back yet
bool AnubarakRaidGathersTrigger::IsActive()
{
    return AnubarakEngaged(botAI) && AnubarakGathers(botAI) && !IsAnubarakDodgeHeld(botAI);
}

bool AnubarakHealerStandTrigger::IsActive()
{
    if (!AnubarakEngaged(botAI) || !PlayerbotAI::IsHeal(bot))
        return false;

    Position slot;
    if (!GetAnubarakHealerStand(bot, slot) || IsAnubarakDodgeHeld(botAI) || UnitOnPermafrost(bot))
        return false;

    // a burrower on it stays by its side tank until the taunt
    AnubarakBurrowerLead lead;
    if (GetAnubarakBurrowerLead(botAI, lead))
        return false;

    bool const off = bot->GetExactDist2d(slot.GetPositionX(), slot.GetPositionY()) > ANUBARAK_HEALER_STAND_LEAVE;
    // on while it still moves, so the action can stop the walk
    if (off && AnubarakHealerStandWaits(bot))
        return bot->isMoving();

    return off;
}

bool AnubarakRangedStandTrigger::IsActive()
{
    if (!AnubarakEngaged(botAI) || !PlayerbotAI::IsRangedDps(bot))
        return false;

    Position stand;
    if (!GetAnubarakRangedStand(bot, stand) || IsAnubarakDodgeHeld(botAI) || UnitOnPermafrost(bot))
        return false;

    AnubarakBurrowerLead lead;
    if (GetAnubarakBurrowerLead(botAI, lead))
        return false;

    return bot->GetExactDist2d(stand.GetPositionX(), stand.GetPositionY()) > ANUBARAK_HEALER_STAND_LEAVE;
}

bool AnubarakDemonicCircleDueTrigger::IsActive() { return AnubarakDemonicCircleDue(botAI); }

// A held dodge spot inside a patch, when none was clear of one, stays: the lane is worse than the slow
bool AnubarakOnPermafrostTrigger::IsActive()
{
    return AnubarakEngaged(botAI) && !IsAnubarakDodgeHeld(botAI) && AnubarakNonTankOnPermafrost(bot);
}

bool AnubarakMeleeBurrowerStandTrigger::IsActive() { return AnubarakMeleeNeedsBurrowerStand(botAI); }

bool AnubarakBurrowerLeadTrigger::IsActive()
{
    AnubarakBurrowerLead lead;
    return GetAnubarakBurrowerLead(botAI, lead) && lead.walk && !IsAnubarakDodgeHeld(botAI);
}

bool AnubarakKiteCatDropTrigger::IsActive() { return AnubarakKiteCatDropDue(botAI); }

bool AnubarakTankNeedsRescueTrigger::IsActive()
{
    AnubarakTankRescue rescue;
    return GetAnubarakTankRescue(botAI, rescue);
}

bool AnubarakLustDueTrigger::IsActive() { return GetAnubarakLust(botAI) != nullptr; }

bool AnubarakPursuedBySpikeTrigger::IsActive()
{
    return AnubarakEngaged(botAI) && bot->HasAura(SPELL_MARK);
}

bool AnubarakKiterNeedsProtectionTrigger::IsActive()
{
    return bot->getClass() == CLASS_PALADIN && AnubarakEngaged(botAI) && GetAnubarakKiterToProtect(bot) != nullptr;
}

bool AnubarakKiterNeedsFreedomTrigger::IsActive()
{
    return bot->getClass() == CLASS_PALADIN && AnubarakEngaged(botAI) && GetAnubarakKiterToFree(bot) != nullptr;
}

bool AnubarakHopEscortTrigger::IsActive()
{
    Position spot;
    if (bot->getClass() != CLASS_PALADIN || !AnubarakEngaged(botAI))
        return false;

    if (!GetAnubarakEscortSpot(bot, spot))
        return false;

    return bot->GetExactDist2d(spot.GetPositionX(), spot.GetPositionY()) > ANUBARAK_ESCORT_HOLD;
}

bool AnubarakRedirectThreatTrigger::IsActive()
{
    AnubarakRedirect redirect;
    if (!AnubarakEngaged(botAI) || !GetAnubarakRedirect(bot, redirect) || !redirect.tank)
        return false;

    if (bot->getClass() == CLASS_ROGUE)
        return botAI->CanCastSpell("tricks of the trade", redirect.tank);

    return botAI->CanCastSpell("misdirection", redirect.tank) ||
           (redirect.dump && bot->HasAura(SPELL_MISDIRECTION_PROC));
}

// Stays on through the walk to the spot, so one step out of danger doesn't hand the bot to another walk
bool AnubarakSpikeNearbyTrigger::IsActive()
{
    return AnubarakEngaged(botAI) && (AnubarakInSpikeDanger(bot) || AnubarakDodgeWalking(bot));
}

bool AnubarakBurrowerCastingShadowStrikeTrigger::IsActive()
{
    return AnubarakEngaged(botAI) && GetAnubarakShadowStrikeDuty(bot);
}

bool AnubarakLeechingSwarmOnTankTrigger::IsActive()
{
    if (!AnubarakEngaged(botAI) || GetAnubarakPhase(botAI) != AnubarakPhase::Swarm)
        return false;

    Creature* boss = GetAnubarak(bot);
    return boss && boss->GetVictim() == bot && NextTankDefensive(botAI, bot);
}

bool AnubarakRangedShouldSeedPermafrostTrigger::IsActive()
{
    return AnubarakEngaged(botAI) && PlayerbotAI::IsRangedDps(bot) && GetAnubarakSphereToShoot(bot);
}

bool AnubarakDpsTargetTrigger::IsActive()
{
    Unit* target = GetAnubarakDpsTarget(botAI);
    return target && (AI_VALUE(Unit*, "current target") != target || ToCPetOffPick(botAI, target));
}

bool AnubarakBurrowerNeedsAssistTankTrigger::IsActive()
{
    if (!AnubarakEngaged(botAI))
        return false;

    if (IsAnubarakPickupTank(bot) && IsAnubarakSurfaced(GetAnubarakPhase(botAI)))
        return false;

    return static_cast<bool>(GetAnubarakBurrowerPick(botAI));
}

bool AnubarakSideTankReadyTrigger::IsActive()
{
    if (!AnubarakEngaged(botAI) || GetAnubarakBurrowerTankSide(bot) < 0 || bot->HasAura(SPELL_MARK))
        return false;

    if (IsAnubarakPickupTank(bot) && IsAnubarakSurfaced(GetAnubarakPhase(botAI)))
        return false;

    return !GetAnubarakBurrowerPick(botAI) && !AnubarakGathers(botAI) && !IsAnubarakEscort(botAI);
}

bool AnubarakSideTankAreaThreatTrigger::IsActive()
{
    AnubarakSideTankAoe aoe;
    return AnubarakEngaged(botAI) && GetAnubarakSideTankAoe(botAI, aoe);
}

bool AnubarakCleaveBuffNearBurrowerTrigger::IsActive() { return GetAnubarakCleaveBuffToDrop(botAI) != nullptr; }

bool AnubarakSideTankNeedsDefensiveTrigger::IsActive()
{
    if (!AnubarakEngaged(botAI) || !AnubarakSideTankNeedsDefensive(bot))
        return false;

    uint32 const defensive = NextTankDefensive(botAI, bot, GetAnubarakPhase(botAI) != AnubarakPhase::Swarm);
    return defensive && !AnubarakSideTankDefensiveKept(botAI, defensive);
}

// Also while a bubble is up, so the action can cancel the one it cast
bool AnubarakColdSelfSaveTrigger::IsActive()
{
    return GetAnubarakColdSelfSave(botAI) || (AnubarakLeechingSwarmActive(botAI) && AnubarakColdBubbleUp(bot));
}

bool AnubarakSideTankDeadTrigger::IsActive()
{
    return AnubarakEngaged(botAI) && GetAnubarakBattleResTarget(botAI);
}

bool AnubarakGripFrostSphereTrigger::IsActive()
{
    AnubarakGripDuty duty;
    return AnubarakEngaged(botAI) && GetAnubarakGripDuty(bot, duty);
}

bool AnubarakScarabOnRaidTrigger::IsActive()
{
    if (!AnubarakEngaged(botAI) || !IsAnubarakTankPlayer(bot))
        return false;

    AnubarakPhase const phase = GetAnubarakPhase(botAI);
    if (IsAnubarakPickupTank(bot) && phase != AnubarakPhase::Submerged)
        return false;

    // Normal phase 2's scarabs are the main tank's and side tanks hold their patches. Heroic has a free
    // side tank peel them off the healers. Either way a side tank keeps its taunts for a wave that's due.
    bool const heroic = bot->GetMap()->IsHeroic();
    if (GetAnubarakBurrowerTankSide(bot) >= 0 &&
        ((!IsAnubarakSurfaced(phase) && !heroic) || AnubarakWaveImminent(botAI) || bot->HasAura(SPELL_MARK) ||
         IsAnubarakEscort(botAI)))
        return false;

    // Leaves burrower duty to the hold node, which outranks this one and would swap targets back
    if (GetAnubarakBurrowerPick(botAI))
        return false;

    return GetAnubarakScarabToPickUp(bot) != nullptr;
}

bool AnubarakEngagedByMainTankTrigger::IsActive()
{
    return AnubarakEngaged(botAI) && IsAnubarakPickupTank(bot) && IsAnubarakSurfaced(GetAnubarakPhase(botAI));
}

bool AnubarakPenetratingColdOnRaidTrigger::IsActive()
{
    return AnubarakEngaged(botAI) && PlayerbotAI::IsHeal(bot) && GetAnubarakPenetratingColdHealTarget(botAI);
}

bool AnubarakSwarmHealUrgentTrigger::IsActive()
{
    AnubarakSwarmHealPick const pick = GetAnubarakSwarmHealPick(botAI);
    return pick.target && pick.urgent;
}

bool AnubarakSwarmHealTrigger::IsActive()
{
    AnubarakSwarmHealPick const pick = GetAnubarakSwarmHealPick(botAI);
    return pick.target && !pick.urgent;
}

bool AnubarakAuraMasteryDueTrigger::IsActive() { return AnubarakAuraMasteryDue(botAI); }

bool AnubarakDivinePleaTrigger::IsActive() { return AnubarakDivinePleaDue(botAI); }

bool AnubarakInnervateHealerTrigger::IsActive() { return AnubarakInnervateHealerDue(botAI); }

bool AnubarakScarabKiteTrigger::IsActive()
{
    Position step;
    return GetAnubarakScarabKite(botAI, step);
}

bool AnubarakManaTopUpTrigger::IsActive() { return AnubarakManaTopUpDue(botAI); }

bool AnubarakLightJudgeDueTrigger::IsActive() { return AnubarakLightJudgeDue(botAI); }

bool AnubarakHealingStreamDueTrigger::IsActive() { return AnubarakHealingStreamDue(botAI); }

void AddToCAnubarakTriggerNodes(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("anubarak kiter needs immunity", {
        NextAction("anubarak kiter immunity", ACTION_EMERGENCY + 9) }));

    // Ahead of the kite, so a kiter marked in a patch frees itself before it walks out at 20% speed
    triggers.push_back(new TriggerNode("anubarak kiter needs freedom", {
        NextAction("anubarak free kiter", ACTION_EMERGENCY + 8.5f) }));

    triggers.push_back(new TriggerNode("anubarak pursued by spike", {
        NextAction("anubarak kite spike to permafrost", ACTION_EMERGENCY + 8) }));

    triggers.push_back(new TriggerNode("anubarak kiter needs protection", {
        NextAction("anubarak protect kiter", ACTION_EMERGENCY + 7) }));

    triggers.push_back(new TriggerNode("anubarak spike nearby", {
        NextAction("anubarak avoid spike", ACTION_EMERGENCY + 6) }));

    triggers.push_back(new TriggerNode("anubarak on permafrost", {
        NextAction("anubarak leave permafrost", ACTION_EMERGENCY + 5) }));

    // Over the class Lay on Hands and Pain Suppression, which go to whoever is lowest
    triggers.push_back(new TriggerNode("anubarak tank needs rescue", {
        NextAction("anubarak tank rescue", ACTION_EMERGENCY + 4) }));

    triggers.push_back(new TriggerNode("anubarak burrower casting shadow strike", {
        NextAction("anubarak interrupt shadow strike", ACTION_EMERGENCY + 3) }));

    // The first tick lands 3 s after Penetrating Cold, and the swarm keeps a non-tank under it
    triggers.push_back(new TriggerNode("anubarak penetrating cold self save", {
        NextAction("anubarak penetrating cold self save", ACTION_EMERGENCY + 2) }));

    // Instant and castable on the move, so over the stand walks and the swarm heals
    triggers.push_back(new TriggerNode("anubarak lust due", {
        NextAction("anubarak lust", ACTION_EMERGENCY + 1) }));

    // Off the GCD, ahead of the battle res and every heal that won't cast from Cat Form
    triggers.push_back(new TriggerNode("anubarak kite cat drop", {
        NextAction("anubarak kite cat drop", ACTION_RAID + 7.5f) }));

    triggers.push_back(new TriggerNode("anubarak side tank dead", {
        NextAction("anubarak battle res side tank", ACTION_RAID + 7) }));

    triggers.push_back(new TriggerNode("anubarak side tank needs defensive", {
        NextAction("anubarak side tank defensive", ACTION_RAID + 6.5f) }));

    triggers.push_back(new TriggerNode("anubarak leeching swarm on tank", {
        NextAction("anubarak tank defensive", ACTION_RAID + 6) }));

    triggers.push_back(new TriggerNode("anubarak grip frost sphere", {
        NextAction("anubarak grip frost sphere", ACTION_RAID + 5.5f) }));

    triggers.push_back(new TriggerNode("anubarak ranged should seed permafrost", {
        NextAction("anubarak destroy frost sphere", ACTION_RAID + 5) }));

    // Above every other walk of a DPS, side tank or healer bot that can be the escort
    triggers.push_back(new TriggerNode("anubarak hop escort", {
        NextAction("anubarak hop escort", ACTION_RAID + 4.5f) }));

    // Under the escort, over the target pick: a side tank taunts only within 30 yd
    triggers.push_back(new TriggerNode("anubarak burrower lead", {
        NextAction("anubarak burrower lead", ACTION_RAID + 4.4f) }));

    // Under the lead, over the target pick: every hit adds a minute of Acid-Drenched Mandibles
    triggers.push_back(new TriggerNode("anubarak scarab kite", {
        NextAction("anubarak kite scarab", ACTION_RAID + 4.3f) }));

    // Off the GCD, ahead of the next swing the buff would carry onto the burrower
    triggers.push_back(new TriggerNode("anubarak cleave buff near burrower", {
        NextAction("anubarak drop cleave buff", ACTION_RAID + 4.2f) }));

    triggers.push_back(new TriggerNode("anubarak dps target", {
        NextAction("anubarak dps target", ACTION_RAID + 4) }));

    // Under the target pick, so a new focus is taken before the walk to the old one's stand
    triggers.push_back(new TriggerNode("anubarak melee burrower stand", {
        NextAction("anubarak melee burrower stand", ACTION_RAID + 3.8f) }));

    triggers.push_back(new TriggerNode("anubarak burrower needs assist tank", {
        NextAction("anubarak assist tank hold burrower", ACTION_RAID + 3) }));

    triggers.push_back(new TriggerNode("anubarak scarab on raid", {
        NextAction("anubarak tank pick up scarab", ACTION_RAID + 2) }));

    // Under the hold and pickup, over the class rotation's own area spells
    triggers.push_back(new TriggerNode("anubarak side tank area threat", {
        NextAction("anubarak side tank area threat", ACTION_RAID + 2.5f) }));

    triggers.push_back(new TriggerNode("anubarak side tank ready", {
        NextAction("anubarak side tank ready", ACTION_RAID + 1.5f) }));

    triggers.push_back(new TriggerNode("anubarak raid gathers", {
        NextAction("anubarak gather", ACTION_RAID + 0.5f) }));

    triggers.push_back(new TriggerNode("anubarak healer stand", {
        NextAction("anubarak healer stand", ACTION_RAID + 0.45f) }));

    triggers.push_back(new TriggerNode("anubarak ranged stand", {
        NextAction("anubarak ranged stand", ACTION_RAID + 0.45f) }));

    triggers.push_back(new TriggerNode("anubarak demonic circle due", {
        NextAction("anubarak demonic circle", ACTION_RAID + 0.4f) }));

    // Under every walk, over the class rotation
    triggers.push_back(new TriggerNode("anubarak healing stream due", {
        NextAction("healing stream totem", ACTION_RAID - 0.5f) }));

    triggers.push_back(new TriggerNode("anubarak mana top up", {
        NextAction("anubarak top up mana", ACTION_RAID - 1) }));

    triggers.push_back(new TriggerNode("anubarak light judge due", {
        NextAction("anubarak judge light", ACTION_HIGH + 5) }));

    triggers.push_back(new TriggerNode("anubarak engaged by main tank", {
        NextAction("anubarak main tank hold boss", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("anubarak redirect threat", {
        NextAction("anubarak redirect threat", ACTION_RAID + 1.2f) }));

    triggers.push_back(new TriggerNode("anubarak penetrating cold on raid", {
        NextAction("anubarak heal penetrating cold", ACTION_MEDIUM_HEAL + 5) }));

    // Off the GCD, so ahead of the rescue heals it covers
    triggers.push_back(new TriggerNode("anubarak aura mastery due", {
        NextAction("anubarak aura mastery", ACTION_CRITICAL_HEAL + 13) }));

    // Heroic phase 3: the urgent tiers above every class heal, the top-offs under the mana tools
    triggers.push_back(new TriggerNode("anubarak swarm heal urgent", {
        NextAction("anubarak swarm heal", ACTION_CRITICAL_HEAL + 12) }));

    triggers.push_back(new TriggerNode("anubarak swarm heal", {
        NextAction("anubarak swarm heal", ACTION_MEDIUM_HEAL + 0.75f) }));

    // Above the medium heals. Divine Plea waits until no tank or carrier is low
    triggers.push_back(new TriggerNode("anubarak divine plea", {
        NextAction("divine plea", ACTION_MEDIUM_HEAL + 6) }));

    triggers.push_back(new TriggerNode("anubarak innervate healer", {
        NextAction("anubarak innervate healer", ACTION_MEDIUM_HEAL + 6) }));
}
