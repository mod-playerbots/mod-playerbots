/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_KARATRIGGERCONTEXT_H
#define PLAYERBOTS_KARATRIGGERCONTEXT_H

#include "KaraTriggers.h"
#include "NamedObjectContext.h"

class RaidKarazhanTriggerContext : public NamedObjectContext<Trigger>
{
public:
    RaidKarazhanTriggerContext()
    {
        // Shared
        creators["karazhan no encounter in progress"] =
            &RaidKarazhanTriggerContext::karazhan_no_encounter_in_progress;

        creators["karazhan enemies cast fear"] =
            &RaidKarazhanTriggerContext::karazhan_enemies_cast_fear;

        // Trash
        creators["mana warp incoming warp breach"] =
            &RaidKarazhanTriggerContext::mana_warp_incoming_warp_breach;

        creators["mana warp target at low health"] =
            &RaidKarazhanTriggerContext::mana_warp_target_at_low_health;

        // Attumen the Huntsman
        creators["attumen the huntsman unmounted should be tanked"] =
            &RaidKarazhanTriggerContext::attumen_the_huntsman_unmounted_should_be_tanked;

        creators["attumen the huntsman midnight should be focused"] =
            &RaidKarazhanTriggerContext::attumen_the_huntsman_midnight_should_be_focused;

        creators["attumen the huntsman mounted should be tanked"] =
            &RaidKarazhanTriggerContext::attumen_the_huntsman_mounted_should_be_tanked;

        creators["attumen the huntsman should stack behind"] =
            &RaidKarazhanTriggerContext::attumen_the_huntsman_should_stack_behind;

        creators["attumen the huntsman should hold dps"] =
            &RaidKarazhanTriggerContext::attumen_the_huntsman_should_hold_dps;

        creators["attumen the huntsman should manage dps timer"] =
            &RaidKarazhanTriggerContext::attumen_the_huntsman_should_manage_dps_timer;

        // Moroes
        creators["moroes should prioritize guests"] =
            &RaidKarazhanTriggerContext::moroes_should_prioritize_guests;

        // Maiden of Virtue
        creators["maiden of virtue should be tanked"] =
            &RaidKarazhanTriggerContext::maiden_of_virtue_should_be_tanked;

        creators["maiden of virtue should break repentance"] =
            &RaidKarazhanTriggerContext::maiden_of_virtue_should_break_repentance;

        creators["maiden of virtue ranged should spread"] =
            &RaidKarazhanTriggerContext::maiden_of_virtue_ranged_should_spread;

        creators["maiden of virtue shaman should ground holy fire"] =
            &RaidKarazhanTriggerContext::maiden_of_virtue_shaman_should_ground_holy_fire;

        // The Big Bad Wolf
        creators["big bad wolf should be tanked"] =
            &RaidKarazhanTriggerContext::big_bad_wolf_should_be_tanked;

        creators["big bad wolf chases red hood"] =
            &RaidKarazhanTriggerContext::big_bad_wolf_chases_red_hood;

        creators["big bad wolf victim has red hood"] =
            &RaidKarazhanTriggerContext::big_bad_wolf_victim_has_red_hood;

        // Romulo and Julianne
        creators["romulo and julianne should die together"] =
            &RaidKarazhanTriggerContext::romulo_and_julianne_should_die_together;

        // The Wizard of Oz
        creators["wizard of oz should prioritize targets"] =
            &RaidKarazhanTriggerContext::wizard_of_oz_should_prioritize_targets;

        creators["wizard of oz mage should burn strawman"] =
            &RaidKarazhanTriggerContext::wizard_of_oz_mage_should_burn_strawman;

        // The Curator
        creators["the curator should prioritize astral flares"] =
            &RaidKarazhanTriggerContext::the_curator_should_prioritize_astral_flares;

        creators["the curator should be tanked"] =
            &RaidKarazhanTriggerContext::the_curator_should_be_tanked;

        creators["the curator ranged should spread"] =
            &RaidKarazhanTriggerContext::the_curator_ranged_should_spread;

        // Terestian Illhoof
        creators["terestian illhoof should prioritize targets"] =
            &RaidKarazhanTriggerContext::terestian_illhoof_should_prioritize_targets;

        // Shade of Aran
        creators["shade of aran casting arcane explosion"] =
            &RaidKarazhanTriggerContext::shade_of_aran_casting_arcane_explosion;

        creators["shade of aran ranged in blizzard"] =
            &RaidKarazhanTriggerContext::shade_of_aran_ranged_in_blizzard;

        creators["shade of aran flame wreath active"] =
            &RaidKarazhanTriggerContext::shade_of_aran_flame_wreath_active;

        creators["shade of aran should prioritize elementals"] =
            &RaidKarazhanTriggerContext::shade_of_aran_should_prioritize_elementals;

        creators["shade of aran ranged should maintain distance"] =
            &RaidKarazhanTriggerContext::shade_of_aran_ranged_should_maintain_distance;

        // Netherspite
        creators["netherspite should block red beam"] =
            &RaidKarazhanTriggerContext::netherspite_should_block_red_beam;

        creators["netherspite should block blue or green beam"] =
            &RaidKarazhanTriggerContext::netherspite_should_block_blue_or_green_beam;

        creators["netherspite next beam blocker should prepare"] =
            &RaidKarazhanTriggerContext::netherspite_next_beam_blocker_should_prepare;

        creators["netherspite should stand between beams"] =
            &RaidKarazhanTriggerContext::netherspite_should_stand_between_beams;

        creators["netherspite should spread for netherbreath"] =
            &RaidKarazhanTriggerContext::netherspite_should_spread_for_netherbreath;

        creators["netherspite should hold dps"] =
            &RaidKarazhanTriggerContext::netherspite_should_hold_dps;

        creators["netherspite should manage pet aggression"] =
            &RaidKarazhanTriggerContext::netherspite_should_manage_pet_aggression;

        creators["netherspite should manage timers and trackers"] =
            &RaidKarazhanTriggerContext::netherspite_should_manage_timers_and_trackers;

        // Prince Malchezaar
        creators["prince malchezaar bot is enfeebled"] =
            &RaidKarazhanTriggerContext::prince_malchezaar_bot_is_enfeebled;

        creators["prince malchezaar near hazard"] =
            &RaidKarazhanTriggerContext::prince_malchezaar_near_hazard;

        creators["prince malchezaar should be tanked"] =
            &RaidKarazhanTriggerContext::prince_malchezaar_should_be_tanked;

        // Nightbane
        creators["nightbane should be tanked"] =
            &RaidKarazhanTriggerContext::nightbane_should_be_tanked;

        creators["nightbane ranged should avoid charred earth"] =
            &RaidKarazhanTriggerContext::nightbane_ranged_should_avoid_charred_earth;

        creators["nightbane melee should avoid charred earth"] =
            &RaidKarazhanTriggerContext::nightbane_melee_should_avoid_charred_earth;

        creators["nightbane should manage pet aggression"] =
            &RaidKarazhanTriggerContext::nightbane_should_manage_pet_aggression;

        creators["nightbane should hold dps"] =
            &RaidKarazhanTriggerContext::nightbane_should_hold_dps;

        creators["nightbane in flight phase"] =
            &RaidKarazhanTriggerContext::nightbane_in_flight_phase;

        creators["nightbane in landing phase"] =
            &RaidKarazhanTriggerContext::nightbane_in_landing_phase;

        creators["nightbane should manage timers and trackers"] =
            &RaidKarazhanTriggerContext::nightbane_should_manage_timers_and_trackers;
    }

private:
    // Shared
    static Trigger* karazhan_no_encounter_in_progress(PlayerbotAI* botAI)
    {
        return new KarazhanNoEncounterInProgressTrigger(botAI);
    }
    static Trigger* karazhan_enemies_cast_fear(PlayerbotAI* botAI)
    {
        return new KarazhanEnemiesCastFearTrigger(botAI);
    }

    // Trash
    static Trigger* mana_warp_incoming_warp_breach(PlayerbotAI* botAI)
    {
        return new ManaWarpIncomingWarpBreachTrigger(botAI);
    }
    static Trigger* mana_warp_target_at_low_health(PlayerbotAI* botAI)
    {
        return new ManaWarpTargetAtLowHealthTrigger(botAI);
    }

    // Attumen the Huntsman
    static Trigger* attumen_the_huntsman_unmounted_should_be_tanked(PlayerbotAI* botAI)
    {
        return new AttumenTheHuntsmanUnmountedShouldBeTankedTrigger(botAI);
    }
    static Trigger* attumen_the_huntsman_midnight_should_be_focused(PlayerbotAI* botAI)
    {
        return new AttumenTheHuntsmanMidnightShouldBeFocusedTrigger(botAI);
    }
    static Trigger* attumen_the_huntsman_mounted_should_be_tanked(PlayerbotAI* botAI)
    {
        return new AttumenTheHuntsmanMountedShouldBeTankedTrigger(botAI);
    }
    static Trigger* attumen_the_huntsman_should_stack_behind(PlayerbotAI* botAI)
    {
        return new AttumenTheHuntsmanShouldStackBehindTrigger(botAI);
    }
    static Trigger* attumen_the_huntsman_should_hold_dps(PlayerbotAI* botAI)
    {
        return new AttumenTheHuntsmanShouldHoldDpsTrigger(botAI);
    }
    static Trigger* attumen_the_huntsman_should_manage_dps_timer(PlayerbotAI* botAI)
    {
        return new AttumenTheHuntsmanShouldManageDpsTimerTrigger(botAI);
    }

    // Moroes
    static Trigger* moroes_should_prioritize_guests(PlayerbotAI* botAI)
    {
        return new MoroesShouldPrioritizeGuestsTrigger(botAI);
    }

    // Maiden of Virtue
    static Trigger* maiden_of_virtue_should_be_tanked(PlayerbotAI* botAI)
    {
        return new MaidenOfVirtueShouldBeTankedTrigger(botAI);
    }
    static Trigger* maiden_of_virtue_should_break_repentance(PlayerbotAI* botAI)
    {
        return new MaidenOfVirtueShouldBreakRepentanceTrigger(botAI);
    }
    static Trigger* maiden_of_virtue_ranged_should_spread(PlayerbotAI* botAI)
    {
        return new MaidenOfVirtueRangedShouldSpreadTrigger(botAI);
    }
    static Trigger* maiden_of_virtue_shaman_should_ground_holy_fire(PlayerbotAI* botAI)
    {
        return new MaidenOfVirtueShamanShouldGroundHolyFireTrigger(botAI);
    }

    // The Big Bad Wolf
    static Trigger* big_bad_wolf_should_be_tanked(PlayerbotAI* botAI)
    {
        return new BigBadWolfShouldBeTankedTrigger(botAI);
    }
    static Trigger* big_bad_wolf_chases_red_hood(PlayerbotAI* botAI)
    {
        return new BigBadWolfChasesRedHoodTrigger(botAI);
    }
    static Trigger* big_bad_wolf_victim_has_red_hood(PlayerbotAI* botAI)
    {
        return new BigBadWolfVictimHasRedHoodTrigger(botAI);
    }

    // Romulo and Julianne
    static Trigger* romulo_and_julianne_should_die_together(PlayerbotAI* botAI)
    {
        return new RomuloAndJulianneShouldDieTogetherTrigger(botAI);
    }

    // The Wizard of Oz
    static Trigger* wizard_of_oz_should_prioritize_targets(PlayerbotAI* botAI)
    {
        return new WizardOfOzShouldPrioritizeTargetsTrigger(botAI);
    }
    static Trigger* wizard_of_oz_mage_should_burn_strawman(PlayerbotAI* botAI)
    {
        return new WizardOfOzMageShouldBurnStrawmanTrigger(botAI);
    }

    // The Curator
    static Trigger* the_curator_should_prioritize_astral_flares(PlayerbotAI* botAI)
    {
        return new TheCuratorShouldPrioritizeAstralFlaresTrigger(botAI);
    }
    static Trigger* the_curator_should_be_tanked(PlayerbotAI* botAI)
    {
        return new TheCuratorShouldBeTankedTrigger(botAI);
    }
    static Trigger* the_curator_ranged_should_spread(PlayerbotAI* botAI)
    {
        return new TheCuratorRangedShouldSpreadTrigger(botAI);
    }

    // Terestian Illhoof
    static Trigger* terestian_illhoof_should_prioritize_targets(PlayerbotAI* botAI)
    {
        return new TerestianIllhoofShouldPrioritizeTargetsTrigger(botAI);
    }

    // Shade of Aran
    static Trigger* shade_of_aran_casting_arcane_explosion(PlayerbotAI* botAI)
    {
        return new ShadeOfAranCastingArcaneExplosionTrigger(botAI);
    }
    static Trigger* shade_of_aran_ranged_in_blizzard(PlayerbotAI* botAI)
    {
        return new ShadeOfAranRangedInBlizzardTrigger(botAI);
    }
    static Trigger* shade_of_aran_flame_wreath_active(PlayerbotAI* botAI)
    {
        return new ShadeOfAranFlameWreathActiveTrigger(botAI);
    }
    static Trigger* shade_of_aran_should_prioritize_elementals(PlayerbotAI* botAI)
    {
        return new ShadeOfAranShouldPrioritizeElementalsTrigger(botAI);
    }
    static Trigger* shade_of_aran_ranged_should_maintain_distance(PlayerbotAI* botAI)
    {
        return new ShadeOfAranRangedShouldMaintainDistanceTrigger(botAI);
    }

    // Netherspite
    static Trigger* netherspite_should_block_red_beam(PlayerbotAI* botAI)
    {
        return new NetherspiteShouldBlockRedBeamTrigger(botAI);
    }
    static Trigger* netherspite_should_block_blue_or_green_beam(PlayerbotAI* botAI)
    {
        return new NetherspiteShouldBlockBlueOrGreenBeamTrigger(botAI);
    }
    static Trigger* netherspite_next_beam_blocker_should_prepare(PlayerbotAI* botAI)
    {
        return new NetherspiteNextBeamBlockerShouldPrepareTrigger(botAI);
    }
    static Trigger* netherspite_should_stand_between_beams(PlayerbotAI* botAI)
    {
        return new NetherspiteShouldStandBetweenBeamsTrigger(botAI);
    }
    static Trigger* netherspite_should_spread_for_netherbreath(PlayerbotAI* botAI)
    {
        return new NetherspiteShouldSpreadForNetherbreathTrigger(botAI);
    }
    static Trigger* netherspite_should_hold_dps(PlayerbotAI* botAI)
    {
        return new NetherspiteShouldHoldDpsTrigger(botAI);
    }
    static Trigger* netherspite_should_manage_pet_aggression(PlayerbotAI* botAI)
    {
        return new NetherspiteShouldManagePetAggressionTrigger(botAI);
    }
    static Trigger* netherspite_should_manage_timers_and_trackers(PlayerbotAI* botAI)
    {
        return new NetherspiteShouldManageTimersAndTrackersTrigger(botAI);
    }

    // Prince Malchezaar
    static Trigger* prince_malchezaar_bot_is_enfeebled(PlayerbotAI* botAI)
    {
        return new PrinceMalchezaarBotIsEnfeebledTrigger(botAI);
    }
    static Trigger* prince_malchezaar_near_hazard(PlayerbotAI* botAI)
    {
        return new PrinceMalchezaarNearHazardTrigger(botAI);
    }
    static Trigger* prince_malchezaar_should_be_tanked(PlayerbotAI* botAI)
    {
        return new PrinceMalchezaarShouldBeTankedTrigger(botAI);
    }

    // Nightbane
    static Trigger* nightbane_should_be_tanked(PlayerbotAI* botAI)
    {
        return new NightbaneShouldBeTankedTrigger(botAI);
    }
    static Trigger* nightbane_ranged_should_avoid_charred_earth(PlayerbotAI* botAI)
    {
        return new NightbaneRangedShouldAvoidCharredEarthTrigger(botAI);
    }
    static Trigger* nightbane_melee_should_avoid_charred_earth(PlayerbotAI* botAI)
    {
        return new NightbaneMeleeShouldAvoidCharredEarthTrigger(botAI);
    }
    static Trigger* nightbane_should_manage_pet_aggression(PlayerbotAI* botAI)
    {
        return new NightbaneShouldManagePetAggressionTrigger(botAI);
    }
    static Trigger* nightbane_should_hold_dps(PlayerbotAI* botAI)
    {
        return new NightbaneShouldHoldDpsTrigger(botAI);
    }
    static Trigger* nightbane_in_flight_phase(PlayerbotAI* botAI)
    {
        return new NightbaneInFlightPhaseTrigger(botAI);
    }
    static Trigger* nightbane_in_landing_phase(PlayerbotAI* botAI)
    {
        return new NightbaneInLandingPhaseTrigger(botAI);
    }
    static Trigger* nightbane_should_manage_timers_and_trackers(PlayerbotAI* botAI)
    {
        return new NightbaneShouldManageTimersAndTrackersTrigger(botAI);
    }
};

#endif
