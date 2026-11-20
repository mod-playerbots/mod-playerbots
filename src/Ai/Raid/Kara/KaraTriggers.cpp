/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "KaraTriggers.h"
#include "EncounterHelpers.h"
#include "KaraActions.h"
#include "KaraHelpers.h"
#include "Playerbots.h"
#include "RtiTargetValue.h"
#include "ThreatHelpers.h"
#include <algorithm>

using namespace KaraHelpers;
using namespace EncounterHelpers;

// Shared

bool KarazhanNoEncounterInProgressTrigger::IsActive()
{
    return !IsEncounterInProgress(bot, KARA_MAP_ID);
}

bool KarazhanEnemiesCastFearTrigger::IsActive()
{
    if (bot->getClass() != CLASS_SHAMAN)
        return false;

    return IsKarazhanFearCasterActive(bot) && !AI_VALUE2(bool, "has totem", "tremor totem");
}

// Trash

bool ManaWarpIncomingWarpBreachTrigger::IsActive()
{
    return GetManaWarpToStun(botAI);
}

bool ManaWarpTargetAtLowHealthTrigger::IsActive()
{
    return GetManaWarpToStunNext(botAI);
}

// Attumen the Huntsman

bool AttumenTheHuntsmanUnmountedShouldBeTankedTrigger::IsActiveInEncounter()
{
    Unit* midnight = FindThreatTarget(bot, "midnight");
    if (!midnight || IsAttumenMounted(midnight))
        return false;

    if (!FindThreatTarget(bot, "attumen the huntsman"))
        return false;

    return PlayerbotAI::IsAssistTankOfIndex(bot, 0, true);
}

bool AttumenTheHuntsmanMidnightShouldBeFocusedTrigger::IsActiveInEncounter()
{
    Unit* midnight = FindThreatTarget(bot, "midnight");
    if (!midnight || IsAttumenMounted(midnight))
        return false;

    if (AI_VALUE(Unit*, "current target") == midnight)
        return false;

    return !PlayerbotAI::IsAssistTankOfIndex(bot, 0, true);
}

bool AttumenTheHuntsmanMountedShouldBeTankedTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    Unit* attumen = GetMountedAttumen(bot);
    return attumen && attumen->GetVictim() == bot;
}

bool AttumenTheHuntsmanShouldStackBehindTrigger::IsActiveInEncounter()
{
    Unit* attumen = GetMountedAttumen(bot);
    if (!attumen || attumen->GetVictim() == bot)
        return false;

    if (PlayerbotAI::IsTank(bot) && PlayerbotAI::IsMainTank(bot))
        return false;

    return GetAttumenStackPosition(bot, attumen).has_value();
}

bool AttumenTheHuntsmanShouldHoldDpsTrigger::IsActiveInEncounter()
{
    return IsAttumenDpsHeld(bot);
}

bool AttumenTheHuntsmanShouldManageDpsTimerTrigger::IsActiveInEncounter()
{
    if (!IsMechanicTrackerBot(bot, KARA_MAP_ID))
        return false;

    Unit* midnight = FindThreatTarget(bot, "midnight");
    return midnight && IsAttumenMounted(midnight);
}

// Moroes

bool MoroesShouldPrioritizeGuestsTrigger::IsActiveInEncounter()
{
    return IsMechanicTrackerBot(bot, KARA_MAP_ID);
}

// Maiden of Virtue

bool MaidenOfVirtueShouldBeTankedTrigger::IsActiveInEncounter()
{
    return PlayerbotAI::IsTank(bot);
}

bool MaidenOfVirtueShouldBreakRepentanceTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    Unit* maiden = GetEncounterBoss();
    return maiden && maiden->GetVictim() == bot;
}

bool MaidenOfVirtueRangedShouldSpreadTrigger::IsActiveInEncounter()
{
    return PlayerbotAI::IsRanged(bot);
}

bool MaidenOfVirtueShamanShouldGroundHolyFireTrigger::IsActiveInEncounter()
{
    return bot->getClass() == CLASS_SHAMAN && !AI_VALUE2(bool, "has totem", "grounding totem");
}

// The Big Bad Wolf

bool BigBadWolfShouldBeTankedTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    if (!GetOperaActor(bot, "the big bad wolf"))
        return false;

    return !IsLittleRedRidingHood(bot);
}

bool BigBadWolfChasesRedHoodTrigger::IsActiveInEncounter()
{
    return IsLittleRedRidingHood(bot);
}

bool BigBadWolfVictimHasRedHoodTrigger::IsActiveInEncounter()
{
    uint8 const botClass = bot->getClass();
    if (botClass != CLASS_PALADIN && botClass != CLASS_PRIEST && botClass != CLASS_WARRIOR)
        return false;

    Unit* wolf = GetOperaActor(bot, "the big bad wolf");
    if (!wolf)
        return false;

    Player* target = GetBigBadWolfRedHood(wolf);
    return target && target != bot;
}

// Romulo and Julianne

bool RomuloAndJulianneShouldDieTogetherTrigger::IsActiveInEncounter()
{
    if (!IsMechanicTrackerBot(bot, KARA_MAP_ID))
        return false;

    return GetOperaActor(bot, "romulo") && GetOperaActor(bot, "julianne");
}

// The Wizard of Oz

bool WizardOfOzShouldPrioritizeTargetsTrigger::IsActiveInEncounter()
{
    if (!IsMechanicTrackerBot(bot, KARA_MAP_ID))
        return false;

    for (char const* actor : OZ_TARGETS)
    {
        if (GetOperaActor(bot, actor))
            return true;
    }

    return false;
}

bool WizardOfOzMageShouldBurnStrawmanTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_MAGE)
        return false;

    Unit* strawman = GetOperaActor(bot, "strawman");
    return strawman && !strawman->HasAura(Id(KaraSpells::SPELL_BURNING_STRAW));
}

// The Curator

bool TheCuratorShouldPrioritizeAstralFlaresTrigger::IsActiveInEncounter()
{
    return IsMechanicTrackerBot(bot, KARA_MAP_ID) && FindThreatTarget(bot, "astral flare");
}

bool TheCuratorShouldBeTankedTrigger::IsActiveInEncounter()
{
    return PlayerbotAI::IsTank(bot);
}

bool TheCuratorRangedShouldSpreadTrigger::IsActiveInEncounter()
{
    return PlayerbotAI::IsRanged(bot);
}

// Terestian Illhoof

bool TerestianIllhoofShouldPrioritizeTargetsTrigger::IsActiveInEncounter()
{
    return IsMechanicTrackerBot(bot, KARA_MAP_ID);
}

// Shade of Aran

bool ShadeOfAranCastingArcaneExplosionTrigger::IsActiveInEncounter()
{
    // Return false for rooted bots (Chains of Ice) so they can at least keep casting.
    Unit* aran = GetEncounterBoss();
    return aran && IsAranCastingArcaneExplosion(aran) && botAI->CanMove();
}

// Ranged only. Blizzard moves around Aran in a band, and melee range is inside the minimum radius.
bool ShadeOfAranRangedInBlizzardTrigger::IsActiveInEncounter()
{
    if (!bot->HasAura(Id(KaraSpells::SPELL_BLIZZARD)))
        return false;

    // Return false for rooted bots (Chains of Ice) so they can at least keep casting.
    return PlayerbotAI::IsRanged(bot) && botAI->CanMove();
}

bool ShadeOfAranFlameWreathActiveTrigger::IsActiveInEncounter()
{
    // Do not check for Aran here. A bot that does not find Aran (most likely due to being battle
    // rezzed between the 5s zone combat pulses) still needs to stop moving for Flame Wreath.
    Unit* aran = GetEncounterBoss();
    if (!IsFlameWreathInProgress(botAI, aran))
        return false;

    // The Flame Wreath stop cannot be permitted to stop sheeped bots from wandering.
    return botAI->CanMove() && !CanReachTargetDuringFlameWreath(botAI, aran);
}

// The elementals never refresh zone combat, so a resurrected tracker cannot find them by threat.
bool ShadeOfAranShouldPrioritizeElementalsTrigger::IsActiveInEncounter()
{
    if (!IsMechanicTrackerBot(bot, KARA_MAP_ID))
        return false;

    GuidVector const elementals =
        context->GetValue<GuidVector>("shade of aran unbanished elementals")->Get();
    if (elementals.empty())
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    ObjectGuid const skull = group->GetTargetIcon(RtiTargetValue::skullIndex);
    return std::find(elementals.begin(), elementals.end(), skull) == elementals.end();
}

bool ShadeOfAranRangedShouldMaintainDistanceTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    Unit* aran = GetEncounterBoss();
    if (!aran)
        return false;

    if (bot->HasAura(Id(KaraSpells::SPELL_BLIZZARD)))
        return false;

    float const distance = bot->GetExactDist2d(aran);
    if (distance >= ARAN_RANGED_MIN_DISTANCE - ARAN_RANGED_BAND_TOLERANCE &&
        distance <= ARAN_RANGED_MAX_DISTANCE + ARAN_RANGED_BAND_TOLERANCE)
    {
        return false;
    }

    if (IsAranCastingArcaneExplosion(aran) || IsFlameWreathInProgress(botAI, aran))
        return false;

    // If elementals are up, let ranged have freedom to reach them even if outside the band.
    return context->GetValue<GuidVector>("shade of aran unbanished elementals")->Get().empty();
}

// Netherspite

bool NetherspiteShouldBlockRedBeamTrigger::IsActiveInEncounter()
{
    KaraInstanceState const* state = FindKaraState(bot->GetInstanceId());
    return state &&
        state->netherspiteBeams[Id(NetherspiteBeam::Red)].blocker == bot->GetGUID();
}

bool NetherspiteShouldBlockBlueOrGreenBeamTrigger::IsActiveInEncounter()
{
    KaraInstanceState const* state = FindKaraState(bot->GetInstanceId());
    if (!state || state->netherspiteBeams[Id(NetherspiteBeam::Red)].blocker == bot->GetGUID())
        return false;

    Position const* position = FindNetherspiteBlockerPosition(*state, bot->GetGUID());
    return position && bot->GetExactDist2d(*position) > NETHERSPITE_BEAM_POSITION_TOLERANCE;
}

bool NetherspiteNextBeamBlockerShouldPrepareTrigger::IsActiveInEncounter()
{
    KaraInstanceState const* state = FindKaraState(bot->GetInstanceId());
    Position const* position =
        state ? FindNetherspiteNextPosition(*state, bot->GetGUID()) : nullptr;
    return position && bot->GetExactDist2d(*position) > NETHERSPITE_BEAM_POSITION_TOLERANCE;
}

bool NetherspiteShouldStandBetweenBeamsTrigger::IsActiveInEncounter()
{
    KaraInstanceState const* state = FindKaraState(bot->GetInstanceId());
    if (!state || !AreNetherspitePortalsKnown(*state) ||
        IsNetherspiteBlockerOrNext(*state, bot->GetGUID()))
    {
        return false;
    }

    Unit* netherspite = GetEncounterBoss();
    return netherspite && !IsBanishPhase(netherspite) && !IsBetweenNetherspiteBeams(
        bot, *netherspite, *state, bot->GetPositionX(), bot->GetPositionY());
}

bool NetherspiteShouldSpreadForNetherbreathTrigger::IsActiveInEncounter()
{
    KaraInstanceState const* state = FindKaraState(bot->GetInstanceId());
    NetherspiteBanishSlot const* slot =
        state ? FindNetherspiteBanishSlot(*state, bot->GetGUID()) : nullptr;
    if (!slot)
        return false;

    if (!IsClearOfVoidZones(
            *state, bot->GetPositionX(), bot->GetPositionY(), NETHERSPITE_VZ_CLEARANCE))
    {
        return true;
    }

    Unit* netherspite = GetEncounterBoss();
    if (!netherspite)
        return false;

    if (netherspite->GetExactDist2d(bot) > netherspite->GetExactDist2d(slot->position) +
            NETHERSPITE_BANISH_SLOT_RADIUS_TOLERANCE)
    {
        return true;
    }

    float const gap = Position::NormalizeOrientation(
        netherspite->GetAngle(bot->GetPositionX(), bot->GetPositionY()) -
        netherspite->GetAngle(slot->position.GetPositionX(), slot->position.GetPositionY()));
    return std::min(gap, 2.0f * static_cast<float>(M_PI) - gap) >
        NETHERSPITE_BANISH_SLOT_TOLERANCE;
}

bool NetherspiteShouldHoldDpsTrigger::IsActiveInEncounter()
{
    Unit* netherspite = GetEncounterBoss();
    return netherspite && IsNetherspiteDpsHeld(bot, netherspite);
}

bool NetherspiteShouldManagePetAggressionTrigger::IsActiveInEncounter()
{
    if (!HasControllableMinion(bot))
        return false;

    Unit* netherspite = GetEncounterBoss();
    return netherspite && IsMinionStanceWrongForHold(bot, IsNetherspiteDpsHeld(bot, netherspite));
}

bool NetherspiteShouldManageTimersAndTrackersTrigger::IsActiveInEncounter()
{
    return IsMechanicTrackerBot(bot, KARA_MAP_ID);
}

// Prince Malchezaar

bool PrinceMalchezaarBotIsEnfeebledTrigger::IsActiveInEncounter()
{
    Aura const* enfeeble = bot->GetAura(Id(KaraSpells::SPELL_ENFEEBLE));
    if (!enfeeble)
        return false;

    Unit* malchezaar = GetEncounterBoss();
    if (!malchezaar)
        return false;

    bool const canNovaHit = CanShadowNovaHitEnfeebled(malchezaar, *enfeeble);
    if ((canNovaHit && bot->GetExactDist(malchezaar) < MALC_SHADOW_NOVA_CLEARANCE) ||
        !IsClearOfInfernals(*bot, GetLandedInfernals(botAI), MALC_HELLFIRE_CLEARANCE))
    {
        return true;
    }

    Action* action = context->GetAction("prince malchezaar enfeebled bot run to safety");
    return action &&
        static_cast<PrinceMalchezaarEnfeebledBotRunToSafetyAction*>(action)->HasDestinationFor(
            enfeeble->GetApplyTime(), canNovaHit);
}

bool PrinceMalchezaarNearHazardTrigger::IsActiveInEncounter()
{
    if (bot->HasAura(Id(KaraSpells::SPELL_ENFEEBLE)))
        return false;

    Unit* malchezaar = GetEncounterBoss();
    if (malchezaar && malchezaar->GetVictim() == bot && PlayerbotAI::IsTank(bot))
        return false;

    if (!IsClearOfInfernals(*bot, GetLandedInfernals(botAI), MALC_HELLFIRE_CLEARANCE) ||
        (malchezaar && PlayerbotAI::IsRanged(bot) &&
         bot->GetExactDist(malchezaar) < MALC_SHADOW_NOVA_CLEARANCE))
    {
        return true;
    }

    Action* action = context->GetAction("prince malchezaar move clear of hazards");
    return action &&
        static_cast<PrinceMalchezaarMoveClearOfHazardsAction*>(action)->HasDestination();
}

bool PrinceMalchezaarShouldBeTankedTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    Unit* malchezaar = GetEncounterBoss();
    if (!malchezaar)
        return false;

    return malchezaar->GetVictim() == bot || PlayerbotAI::IsMainTank(bot);
}

// Nightbane

bool NightbaneShouldBeTankedTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    Unit* nightbane = GetEncounterBoss();
    if (!nightbane || IsNightbaneFlying(nightbane))
        return false;

    return nightbane->GetVictim() == bot || PlayerbotAI::IsMainTank(bot);
}

bool NightbaneRangedShouldAvoidCharredEarthTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    Unit* nightbane = GetEncounterBoss();
    return nightbane && !IsNightbaneFlying(nightbane);
}

bool NightbaneMeleeShouldAvoidCharredEarthTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsMelee(bot))
        return false;

    Unit* nightbane = GetEncounterBoss();
    return nightbane && !IsNightbaneFlying(nightbane) &&
        HoldsNightbaneGroundPosition(botAI, *nightbane);
}

bool NightbaneShouldManagePetAggressionTrigger::IsActiveInEncounter()
{
    if (!HasControllableMinion(bot))
        return false;

    Unit* nightbane = GetEncounterBoss();
    return nightbane && IsMinionStanceWrongForHold(bot, IsNightbaneDpsHeld(bot, nightbane));
}

bool NightbaneShouldHoldDpsTrigger::IsActiveInEncounter()
{
    Unit* nightbane = GetEncounterBoss();
    return nightbane && IsNightbaneDpsHeld(bot, nightbane);
}

bool NightbaneInFlightPhaseTrigger::IsActiveInEncounter()
{
    Unit* nightbane = GetEncounterBoss();
    if (!nightbane || !IsNightbaneFlying(nightbane))
        return false;

    KaraInstanceState const* state = FindKaraState(nightbane->GetInstanceId());
    return state && state->nightbaneFlightStackPosition;
}

bool NightbaneInLandingPhaseTrigger::IsActiveInEncounter()
{
    Unit* nightbane = GetEncounterBoss();
    if (!nightbane || !IsNightbaneFlying(nightbane) || IsNightbaneFlightPhaseActive(nightbane))
        return false;

    return AI_VALUE(Unit*, "current target") == nightbane;
}

bool NightbaneShouldManageTimersAndTrackersTrigger::IsActiveInEncounter()
{
    return IsMechanicTrackerBot(bot, KARA_MAP_ID);
}
