/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "SethTriggers.h"
#include "EncounterHelpers.h"
#include "Playerbots.h"
#include "SethShared.h"
#include "ThreatHelpers.h"

using namespace SethShared;
using namespace EncounterHelpers;

bool TimeLostControllerDropsCharmingTotemTrigger::IsActive()
{
    return IsMechanicTrackerBot(bot, SETH_MAP_ID) && FindThreatTarget(bot, "time-lost controller");
}

bool SethekkProphetCastsFearTrigger::IsActive()
{
    if (bot->getClass() != CLASS_SHAMAN)
        return false;

    if (!FindThreatTarget(bot, "sethekk prophet"))
        return false;

    return !AI_VALUE2(bool, "has totem", "tremor totem");
}

bool DarkweaverSythBossSummonsElementalsTrigger::IsActive()
{
    if (!IsMechanicTrackerBot(bot, SETH_MAP_ID))
        return false;

    Unit* syth = FindThreatTarget(bot, "darkweaver syth");
    return syth && syth->GetHealthPct() > BOSS_BURN_HEALTH_PCT;
}

bool AnzuEncounterHasTwoPhasesTrigger::IsActive()
{
    return FindThreatTarget(bot, "anzu");
}

bool AnzuBirdSpiritsProvideBuffsTrigger::IsActive()
{
    return bot->getClass() == CLASS_DRUID && PlayerbotAI::IsHeal(bot) &&
        FindThreatTarget(bot, "anzu");
}

bool TalonKingIkissShouldBeTankedTrigger::IsActive()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    Unit* ikiss = FindThreatTarget(bot, "talon king ikiss");
    return ikiss && ikiss->GetVictim() == bot && bot->IsWithinMeleeRange(ikiss);
}

bool TalonKingIkissRangedShouldStayNearTankTrigger::IsActive()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    Unit* ikiss = FindThreatTarget(bot, "talon king ikiss");
    return ikiss && !ikiss->HasAura(Id(SethSpells::SPELL_ARCANE_BUBBLE)) &&
        bot->IsWithinLOSInMap(ikiss);
}

bool TalonKingIkissBossCastingArcaneExplosionTrigger::IsActive()
{
    // Arcane Bubble is put up 1s before casting Arcane Explosion
    Unit* ikiss = FindThreatTarget(bot, "talon king ikiss");
    return ikiss && ikiss->HasAura(Id(SethSpells::SPELL_ARCANE_BUBBLE));
}

bool TalonKingIkissOutOfLosTrigger::IsActive()
{
    Unit* ikiss = FindThreatTarget(bot, "talon king ikiss");
    return ikiss && !ikiss->HasAura(Id(SethSpells::SPELL_ARCANE_BUBBLE)) &&
        !bot->IsWithinLOSInMap(ikiss);
}
