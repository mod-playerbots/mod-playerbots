/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "SWPActions.h"
#include "EncounterHelpers.h"
#include "Playerbots.h"
#include "SWPSharedConstants.h"
#include "SWPEncounter_Brut.h"
#include "SWPEncounter_Felmyst.h"
#include "SWPEncounter_Kalec.h"
#include "SWPEncounter_KJ.h"
#include "SWPEncounter_Muru.h"
#include "SWPEncounter_Twins.h"
#include <list>

using namespace SwpHelpers;
using namespace EncounterHelpers;

bool SunwellPlateauResetEncounterStatesAction::Execute(Event /*event*/)
{
    ObjectGuid const guid = bot->GetGUID();
    uint32 const instanceId = bot->GetInstanceId();

    bool didSomething = false;

    // The trigger gates this on InstanceScript reporting no encounter in progress, so nothing here
    // needs a per-boss check of its own - no SWP boss is engaged while this runs.

    // Kalecgos
    Action* kalecAction = context->GetAction("kalecgos disperse ranged");
    if (kalecAction && static_cast<KalecgosDisperseRangedAction*>(
            kalecAction)->ResetInitialRangedPositionReached())
    {
        didSomething = true;
    }

    // Brutallus
    if (bot->HasAura(Id(SwpSpells::SPELL_BURN)))
    {
        bot->RemoveAura(Id(SwpSpells::SPELL_BURN));
        didSomething = true;
    }

    auto const brutallusItr = brutallusEncounterStates.find(instanceId);
    if (brutallusItr != brutallusEncounterStates.end())
        didSomething |= brutallusItr->second.rangedBurnStates.erase(guid) > 0;

    didSomething |= ReleaseBrutallusBurnPad(bot);

    Action* brutallusAction = context->GetAction("brutallus tanks position and swap");
    if (brutallusAction && static_cast<BrutallusTanksPositionAndSwapAction*>(
            brutallusAction)->ResetInitialPositionReached())
    {
        didSomething = true;
    }

    // Eredar Twins
    Action* twinsAction = context->GetAction("eredar twins first assist tank move out of blaze");
    if (twinsAction && static_cast<EredarTwinsFirstAssistTankMoveOutOfBlazeAction*>(
            twinsAction)->ResetAlythessTankStep())
    {
        didSomething = true;
    }

    // Kil'jaeden
    didSomething |= kiljaedenDragonOrbUseTimes.erase(guid.GetCounter()) > 0;

    // Records shared across the raid, so one bot clears them all
    if (!IsMechanicTrackerBot(bot, SWP_MAP_ID))
        return didSomething;

    didSomething |= kalecgosEncounterStates.erase(instanceId) > 0;
    didSomething |= brutallusEncounterStates.erase(instanceId) > 0;
    didSomething |= felmystEncounterStates.erase(instanceId) > 0;
    didSomething |= eredarTwinsIncomingConflagrationStates.erase(instanceId) > 0;
    didSomething |= eredarTwinsBlazeTargetStates.erase(instanceId) > 0;
    didSomething |= eredarTwinsDpsHoldStartMs.erase(instanceId) > 0;
    didSomething |= muruDarknessStates.erase(instanceId) > 0;
    didSomething |= muruVoidSentinelTankAssignments.erase(instanceId) > 0;
    didSomething |= kiljaedenEncounterStates.erase(instanceId) > 0;
    didSomething |= ResetKiljaedenDragonOrbUserAnnouncement(instanceId);
    didSomething |= kiljaedenHandTankAssignments.erase(instanceId) > 0;

    return didSomething;
}

bool SunwellPlateauRemoveProtectiveAuraAction::Execute(Event /*event*/)
{
    if (bot->getClass() == CLASS_MAGE)
    {
        bot->RemoveAura(Id(SwpSpells::SPELL_ICE_BLOCK));
        return true;
    }

    bot->RemoveAura(Id(SwpSpells::SPELL_DIVINE_SHIELD));
    return true;
}

namespace SwpHelpers
{

ObjectGuid FindSwpVolatileFiendGuid(Player* bot)
{
    Creature* fiend = bot->FindNearestCreature(
        Id(SwpNpcs::NPC_VOLATILE_FIEND), SWP_VOLATILE_FIEND_SEARCH_RADIUS, true);

    return fiend ? fiend->GetGUID() : ObjectGuid::Empty;
}

}

bool VolatileFiendKeepEnemyAwayFromGroupAction::Execute(Event /*event*/)
{
    Creature* volatileFiend = botAI->GetCreature(AI_VALUE(ObjectGuid, "swp volatile fiend"));
    if (!volatileFiend || !volatileFiend->IsAlive())
        return false;

    if (PlayerbotAI::IsTank(bot))
        return AI_VALUE(Unit*, "current target") != volatileFiend && Attack(volatileFiend);

    constexpr float safeDistance = 20.0f;
    float const currentDistance = bot->GetDistance(volatileFiend);
    if (currentDistance >= safeDistance)
        return false;

    bot->CastStop();
    return MoveAway(volatileFiend, safeDistance - currentDistance);
}

// At low health, Infernal Defense is cast, granting immunity to all damage but holy
bool ApocalypseGuardAttackWithHolyMagicAction::Execute(Event /*event*/)
{
    Unit* target = nullptr;
    constexpr float searchRadius = 40.0f;
    std::list<Creature*> apocalypseGuards;
    bot->GetCreatureListWithEntryInGrid(
        apocalypseGuards, Id(SwpNpcs::NPC_APOCALYPSE_GUARD), searchRadius);

    for (Creature* apocalypseGuard : apocalypseGuards)
    {
        if (!apocalypseGuard || !apocalypseGuard->IsAlive() ||
            !apocalypseGuard->HasAura(Id(SwpSpells::SPELL_INFERNAL_DEFENSE)))
        {
            continue;
        }

        if (!target || apocalypseGuard->GetGUID() < target->GetGUID())
            target = apocalypseGuard;
    }

    if (bot->HasAura(Id(SwpSpells::SPELL_SHADOWFORM)))
        bot->RemoveAura(Id(SwpSpells::SPELL_SHADOWFORM));

    return botAI->CanCastSpell("smite", target) && botAI->CastSpell("smite", target);
}

bool SunwellPlateauMisdirectBossToMainTankAction::Execute(Event /*event*/)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", _bossName);
    if (!boss)
        return false;

    Player* mainTank = GetGroupMainTank(bot);
    if (!mainTank || !mainTank->IsAlive())
        return false;

    if (botAI->CanCastSpell("misdirection", mainTank))
        return botAI->CastSpell("misdirection", mainTank);

    if (!bot->HasAura(Id(SwpSpells::SPELL_MISDIRECTION)))
        return false;

    return botAI->CanCastSpell("steady shot", boss) && botAI->CastSpell("steady shot", boss);
}
