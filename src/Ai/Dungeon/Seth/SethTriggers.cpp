/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "SethTriggers.h"
#include "Playerbots.h"
#include "RaidBossHelpers.h"
#include "SethData.h"

using namespace SethData;

bool TimeLostControllerDropsCharmingTotemTrigger::IsActive()
{
    return IsMechanicTrackerBot(bot, SETHEKK_HALLS_MAP_ID) &&
        AI_VALUE2(Unit*, "find target", "time-lost controller");
}

bool SethekkProphetCastsFearTrigger::IsActive()
{
    if (bot->getClass() != CLASS_SHAMAN)
        return false;

    return AI_VALUE2(Unit*, "find target", "sethekk prophet") &&
        !AI_VALUE2(bool, "has totem", "tremor totem");
}

bool DarkweaverSythBossSummonsElementalsTrigger::IsActive()
{
    if (!IsMechanicTrackerBot(bot, SETHEKK_HALLS_MAP_ID))
        return false;

    Unit* syth = AI_VALUE2(Unit*, "find target", "darkweaver syth");
    return syth && syth->GetHealthPct() > 10.0f;
}

bool AnzuEncounterHasTwoPhasesTrigger::IsActive()
{
    return AI_VALUE2(Unit*, "find target", "anzu");
}

bool AnzuBirdSpiritsProvideBuffsTrigger::IsActive()
{
    return bot->getClass() == CLASS_DRUID && PlayerbotAI::IsHeal(bot) &&
        AI_VALUE2(Unit*, "find target", "anzu");
}

bool TalonKingIkissBossEngagedByTankTrigger::IsActive()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    Unit* ikiss = AI_VALUE2(Unit*, "find target", "talon king ikiss");
    return ikiss && ikiss->GetVictim() == bot;
}

bool TalonKingIkissRangedPrepareForArcaneExplosionTrigger::IsActive()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    Unit* ikiss = AI_VALUE2(Unit*, "find target", "talon king ikiss");
    return ikiss && !ikiss->HasAura(static_cast<uint32>(SethSpells::SPELL_ARCANE_BUBBLE)) &&
        bot->IsWithinLOSInMap(ikiss);
}

bool TalonKingIkissBossCastingArcaneExplosionTrigger::IsActive()
{
    // Arcane Bubble is put up 1s before casting Arcane Explosion
    Unit* ikiss = AI_VALUE2(Unit*, "find target", "talon king ikiss");
    return ikiss && ikiss->HasAura(static_cast<uint32>(SethSpells::SPELL_ARCANE_BUBBLE));
}

bool TalonKingIkissBossOutOfLosTrigger::IsActive()
{
    Unit* ikiss = AI_VALUE2(Unit*, "find target", "talon king ikiss");
    return ikiss && !ikiss->HasAura(static_cast<uint32>(SethSpells::SPELL_ARCANE_BUBBLE)) &&
        !bot->IsWithinLOSInMap(ikiss);
}
