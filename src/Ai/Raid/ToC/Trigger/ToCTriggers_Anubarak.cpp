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
