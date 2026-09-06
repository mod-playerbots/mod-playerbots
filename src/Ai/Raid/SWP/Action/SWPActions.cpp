/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "SWPActions.h"
#include "EncounterHelpers.h"
#include "InstanceScript.h"
#include "Playerbots.h"
#include "RtiTargetValue.h"
#include "SWPEncounter_Brut.h"
#include "SWPEncounter_Felmyst.h"
#include "SWPEncounter_Kalec.h"
#include "SWPEncounter_KJ.h"
#include "SWPEncounter_Muru.h"
#include "SWPEncounter_Twins.h"
#include "SWPShared.h"
#include <list>

using namespace SwpHelpers;
using namespace EncounterHelpers;

bool SunwellPlateauResetEncounterStatesAction::Execute(Event /*event*/)
{
    ObjectGuid const guid = bot->GetGUID();
    uint32 const instanceId = bot->GetInstanceId();

    bool reset = false;

    // Kalecgos
    Action* kalecAction = context->GetAction("kalecgos disperse ranged");
    if (kalecAction && static_cast<KalecgosDisperseRangedAction*>(
            kalecAction)->ResetInitialRangedPositionReached())
    {
        reset = true;
    }

    // Brutallus
    auto const brutallusItr = brutallusEncounterStates.find(instanceId);
    if (brutallusItr != brutallusEncounterStates.end())
        reset |= brutallusItr->second.rangedBurnStates.erase(guid) > 0;

    reset |= ReleaseBrutallusBurnPad(bot);

    Action* brutallusAction = context->GetAction("brutallus tanks position and swap");
    if (brutallusAction && static_cast<BrutallusTanksPositionAndSwapAction*>(
            brutallusAction)->ResetInitialPositionReached())
    {
        reset = true;
    }

    // Eredar Twins
    reset |= alythessTankLastBlazeGuid.erase(guid) > 0;

    Action* twinsAction = context->GetAction("eredar twins alythess tank move out of blaze");
    if (twinsAction && static_cast<EredarTwinsAlythessTankMoveOutOfBlazeAction*>(
            twinsAction)->ResetAlythessTankStep())
    {
        reset = true;
    }

    // M'uru
    Action* muruAction = context->GetAction("m'uru position ranged by phase");
    if (muruAction && static_cast<MuruPositionRangedByPhaseAction*>(
            muruAction)->ResetEntropiusRangedPositionReached())
    {
        reset = true;
    }

    // Kil'jaeden
    reset |= kiljaedenDragonOrbUseTimes.erase(guid.GetCounter()) > 0;

    // Records shared across the raid, so one bot clears them all
    if (!IsMechanicTrackerBot(bot, SWP_MAP_ID))
        return reset;

    if (!AI_VALUE2(bool, "combat", "self target"))
        reset |= ClearTargetIcon(bot, RtiTargetValue::skullIndex);

    reset |= kalecgosEncounterStates.erase(instanceId) > 0;
    reset |= brutallusEncounterStates.erase(instanceId) > 0;
    reset |= felmystEncounterStates.erase(instanceId) > 0;
    reset |= eredarTwinsIncomingConflagrationStates.erase(instanceId) > 0;
    reset |= eredarTwinsBlazeTargetStates.erase(instanceId) > 0;
    reset |= eredarTwinsDpsHoldStartMs.erase(instanceId) > 0;
    reset |= eredarTwinsTankAssignments.erase(instanceId) > 0;
    reset |= muruDarknessStates.erase(instanceId) > 0;
    reset |= muruVoidSentinelTankAssignments.erase(instanceId) > 0;
    reset |= kiljaedenEncounterStates.erase(instanceId) > 0;
    reset |= ResetKiljaedenDragonOrbUserAnnouncement(instanceId);
    reset |= kiljaedenHandControlClaims.erase(instanceId) > 0;

    return reset;
}

// Clear Kalecgos's Arcane Buffet, the Eredar Twins' Flame Sear, and Kil'jaeden's Fire Bloom.
bool SunwellPlateauRemoveDebuffWithImmunityAction::Execute(Event /*event*/)
{
    uint32 const spellId = GetSelfImmunitySpell(bot);
    return spellId && botAI->CanCastSpell(spellId, bot) && botAI->CastSpell(spellId, bot);
}

bool SunwellPlateauRemoveAuraAction::Execute(Event /*event*/)
{
    // Only the immunities that stop the bot from contributing should be cancelled, so Cloak of
    // Shadows and HPal bubbles are excluded.
    uint32 const spellId = GetSelfImmunitySpell(bot);
    if (spellId && bot->getClass() != CLASS_ROGUE && !PlayerbotAI::IsHeal(bot) &&
        bot->HasAura(spellId))
    {
        bot->RemoveAura(spellId);
        return true;
    }

    if (IsEncounterInProgress(bot, SWP_MAP_ID))
        return false;

    // It is Blizzlike for Burn to persist after the kill, but bots will murder the raid without
    // a dedicated non-combat strategy for it. That's a waste of time, so just wipe the aura.
    if (!HasBrutallusBurn(bot))
        return false;

    bot->RemoveAura(Id(SwpSpells::SPELL_BURN));
    return true;
}

bool VolatileFiendKeepEnemyAwayFromGroupAction::Execute(Event /*event*/)
{
    Creature* volatileFiend = botAI->GetCreature(AI_VALUE(ObjectGuid, "swp volatile fiend"));
    if (!volatileFiend || !volatileFiend->IsAlive())
        return false;

    if (PlayerbotAI::IsTank(bot))
        return AI_VALUE(Unit*, "current target") != volatileFiend && Attack(volatileFiend);

    float const currentDistance = bot->GetExactDist2d(volatileFiend);
    if (currentDistance >= VOLATILE_FIEND_SAFE_DISTANCE)
        return false;

    bot->CastStop();
    return MoveAway(volatileFiend, VOLATILE_FIEND_SAFE_DISTANCE - currentDistance);
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

    if (!target)
        return false;

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
