/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include <list>

#include "SWPActions.h"
#include "SWPData.h"
#include "SWPEncounter_Brut.h"
#include "SWPEncounter_Felmyst.h"
#include "SWPEncounter_Kalec.h"
#include "SWPEncounter_KJ.h"
#include "SWPEncounter_Muru.h"
#include "SWPEncounter_Twins.h"
#include "CreatureAI.h"
#include "Playerbots.h"
#include "RaidBossHelpers.h"

using namespace SunwellHelpers;

bool SunwellPlateauEraseEncounterStatesAction::Execute(Event /*event*/)
{
    const ObjectGuid guid = bot->GetGUID();
    const uint32 instanceId = bot->GetInstanceId();
    const bool isMechanicTracker = IsMechanicTrackerBot(botAI, bot, SUNWELL_MAP_ID);

    bool erased = false;

    if (!AI_VALUE2(Unit*, "find target", "kalecgos") &&
        !AI_VALUE2(Unit*, "find target", "sathrovarr the corruptor"))
    {
        if (isMechanicTracker && kalecgosEncounterStates.erase(instanceId) > 0)
            erased = true;

        if (kalecgosRealmStates.erase(guid) > 0)
            erased = true;
    }

    if (!AI_VALUE2(Unit*, "find target", "brutallus"))
    {
        if (bot->HasAura(static_cast<uint32>(SunwellSpells::SPELL_BURN)))
        {
            bot->RemoveAura(static_cast<uint32>(SunwellSpells::SPELL_BURN));
            erased = true;
        }

        if (botAI->IsRanged(bot) && brutallusRangedBurnStates.erase(guid) > 0)
            erased = true;

        if (botAI->IsRanged(bot) && ReleaseBrutallusBurnPad(bot))
            erased = true;

        if (isMechanicTracker && brutallusRangedAssignments.erase(instanceId) > 0)
            erased = true;

        if (isMechanicTracker && brutallusRangedBurnPadAssignments.erase(instanceId) > 0)
            erased = true;
    }

    if (isMechanicTracker && !AI_VALUE2(Unit*, "find target", "felmyst") &&
        felmystEncounterStates.erase(instanceId) > 0)
    {
        erased = true;
    }

    if (isMechanicTracker && !AI_VALUE2(Unit*, "find target", "grand warlock alythess"))
    {
        if (eredarTwinsIncomingConflagrationStates.erase(instanceId) > 0)
            erased = true;

        if (eredarTwinsDpsHoldTimer.erase(instanceId) > 0)
            erased = true;
    }

    if (isMechanicTracker && !AI_VALUE2(Unit*, "find target", "m'uru") &&
        !AI_VALUE2(Unit*, "find target", "entropius"))
    {
        if (muruDarknessStates.erase(instanceId) > 0)
            erased = true;

        if (muruVoidSentinelTankAssignments.erase(instanceId) > 0)
            erased = true;
    }

    if (isMechanicTracker && !AI_VALUE2(Unit*, "find target", "kil'jaeden") &&
        kiljaedenEncounterStates.erase(instanceId) > 0)
    {
        erased = true;
    }

    if (isMechanicTracker && !AI_VALUE2(Unit*, "find target", "hand of the deceiver") &&
        ResetKiljaedenDragonOrbUserAnnouncement(instanceId))
    {
        erased = true;
    }

    return erased;
}

bool SunwellPlateauRemoveProtectiveAuraAction::Execute(Event /*event*/)
{
    if (bot->getClass() == CLASS_MAGE)
    {
        bot->RemoveAura(static_cast<uint32>(SunwellSpells::SPELL_ICE_BLOCK));
        return true;
    }
    else if (bot->getClass() == CLASS_PALADIN)
    {
        bot->RemoveAura(static_cast<uint32>(SunwellSpells::SPELL_DIVINE_SHIELD));
        return true;
    }

    return false;
}

bool VolatileFiendKeepEnemyAwayFromGroupAction::Execute(Event /*event*/)
{
    constexpr float searchRadius = 25.0f;
    Unit* volatileFiend = bot->FindNearestCreature(
        static_cast<uint32>(SunwellNpcs::NPC_VOLATILE_FIEND), searchRadius, true);
    if (!volatileFiend)
        return false;

    if (botAI->IsTank(bot))
    {
        if (AI_VALUE(Unit*, "current target") != volatileFiend)
            return Attack(volatileFiend);
    }
    else
    {
        constexpr float safeDistance = 20.0f;
        const float currentDistance = bot->GetDistance(volatileFiend);
        if (currentDistance < safeDistance)
        {
            botAI->InterruptSpell();
            return MoveAway(volatileFiend, safeDistance - currentDistance);
        }
    }

    return false;
}

bool ApocalypseGuardAttackWithHolyMagicAction::Execute(Event /*event*/)
{
    Unit* target = nullptr;
    constexpr float searchRadius = 40.0f;
    std::list<Creature*> apocalypseGuards;
    bot->GetCreatureListWithEntryInGrid(
        apocalypseGuards, static_cast<uint32>(SunwellNpcs::NPC_APOCALYPSE_GUARD), searchRadius);

    for (Creature* apocalypseGuard : apocalypseGuards)
    {
        if (!apocalypseGuard || !apocalypseGuard->IsAlive() ||
            !apocalypseGuard->HasAura(static_cast<uint32>(SunwellSpells::SPELL_INFERNAL_DEFENSE)))
        {
            continue;
        }

        if (!target || apocalypseGuard->GetGUID() < target->GetGUID())
            target = apocalypseGuard;
    }

    if (bot->HasAura(static_cast<uint32>(SunwellSpells::SPELL_SHADOWFORM)))
        bot->RemoveAura(static_cast<uint32>(SunwellSpells::SPELL_SHADOWFORM));

    if (botAI->CanCastSpell("smite", target))
        return botAI->CastSpell("smite", target);

    return false;
}
