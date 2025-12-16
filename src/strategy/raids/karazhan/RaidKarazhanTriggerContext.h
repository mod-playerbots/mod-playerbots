#ifndef _PLAYERBOT_RAIDKARAZHANTRIGGERCONTEXT_H
#define _PLAYERBOT_RAIDKARAZHANTRIGGERCONTEXT_H

#include "RaidKarazhanTriggers.h"
#include "AiObjectContext.h"

class RaidKarazhanTriggerContext : public NamedObjectContext<Trigger>
{
public:
    RaidKarazhanTriggerContext()
    {
        // Trash
        creators["mana warp is about to explode"] =
            &RaidKarazhanTriggerContext::mana_warp_is_about_to_explode;

        // Attumen the Huntsman
        creators["attumen the huntsman need target priority"] =
            &RaidKarazhanTriggerContext::attumen_the_huntsman_need_target_priority;

        creators["attumen the huntsman attumen spawned"] =
            &RaidKarazhanTriggerContext::attumen_the_huntsman_attumen_spawned;

        creators["attumen the huntsman attumen is mounted"] =
            &RaidKarazhanTriggerContext::attumen_the_huntsman_attumen_is_mounted;

        creators["attumen the huntsman boss wipes aggro when mounting"] =
            &RaidKarazhanTriggerContext::attumen_the_huntsman_boss_wipes_aggro_when_mounting;

        // Moroes
        creators["moroes boss engaged by main tank"] =
            &RaidKarazhanTriggerContext::moroes_boss_engaged_by_main_tank;

        creators["moroes need target priority"] =
            &RaidKarazhanTriggerContext::moroes_need_target_priority;

        // Maiden of Virtue
        creators["maiden of virtue healers are stunned by repentance"] =
            &RaidKarazhanTriggerContext::maiden_of_virtue_healers_are_stunned_by_repentance;

        creators["maiden of virtue holy wrath deals chain damage"] =
            &RaidKarazhanTriggerContext::maiden_of_virtue_holy_wrath_deals_chain_damage;

        // The Big Bad Wolf
        creators["big bad wolf boss engaged by tank"] =
            &RaidKarazhanTriggerContext::big_bad_wolf_boss_engaged_by_tank;

        creators["big bad wolf boss is chasing little red riding hood"] =
            &RaidKarazhanTriggerContext::big_bad_wolf_boss_is_chasing_little_red_riding_hood;

        // Romulo and Julianne
        creators["romulo and julianne both bosses revived"] =
            &RaidKarazhanTriggerContext::romulo_and_julianne_both_bosses_revived;

        // The Wizard of Oz
        creators["wizard of oz need target priority"] =
            &RaidKarazhanTriggerContext::wizard_of_oz_need_target_priority;

        creators["wizard of oz strawman is vulnerable to fire"] =
            &RaidKarazhanTriggerContext::wizard_of_oz_strawman_is_vulnerable_to_fire;

        // The Curator
        creators["the curator astral flare spawned"] =
            &RaidKarazhanTriggerContext::the_curator_astral_flare_spawned;

        creators["the curator boss engaged by tanks"] =
            &RaidKarazhanTriggerContext::the_curator_boss_engaged_by_tanks;

        creators["the curator astral flares cast arcing sear"] =
            &RaidKarazhanTriggerContext::the_curator_astral_flares_cast_arcing_sear;

        // Terestian Illhoof
        creators["terestian illhoof need target priority"] =
            &RaidKarazhanTriggerContext::terestian_illhoof_need_target_priority;

        // Shade of Aran
        creators["shade of aran arcane explosion is casting"] =
            &RaidKarazhanTriggerContext::shade_of_aran_arcane_explosion_is_casting;

        creators["shade of aran flame wreath is active"] =
            &RaidKarazhanTriggerContext::shade_of_aran_flame_wreath_is_active;

        creators["shade of aran conjured elementals summoned"] =
            &RaidKarazhanTriggerContext::shade_of_aran_conjured_elementals_summoned;

        creators["shade of aran boss uses counterspell and blizzard"] =
            &RaidKarazhanTriggerContext::shade_of_aran_boss_uses_counterspell_and_blizzard;

        // Netherspite
        creators["netherspite red beam is active"] =
            &RaidKarazhanTriggerContext::netherspite_red_beam_is_active;

        creators["netherspite blue beam is active"] =
            &RaidKarazhanTriggerContext::netherspite_blue_beam_is_active;

        creators["netherspite green beam is active"] =
            &RaidKarazhanTriggerContext::netherspite_green_beam_is_active;

        creators["netherspite bot is not beam blocker"] =
            &RaidKarazhanTriggerContext::netherspite_bot_is_not_beam_blocker;

        creators["netherspite boss is banished"] =
            &RaidKarazhanTriggerContext::netherspite_boss_is_banished;

        creators["netherspite need to manage timers and trackers"] =
            &RaidKarazhanTriggerContext::netherspite_need_to_manage_timers_and_trackers;

        // Prince Malchezaar
        creators["prince malchezaar bot is enfeebled"] =
            &RaidKarazhanTriggerContext::prince_malchezaar_bot_is_enfeebled;

        creators["prince malchezaar infernals are spawned"] =
            &RaidKarazhanTriggerContext::prince_malchezaar_infernals_are_spawned;

        creators["prince malchezaar boss engaged by main tank"] =
            &RaidKarazhanTriggerContext::prince_malchezaar_boss_engaged_by_main_tank;

        // Nightbane
        creators["nightbane boss engaged by main tank"] =
            &RaidKarazhanTriggerContext::nightbane_boss_engaged_by_main_tank;

        creators["nightbane ranged bots are in charred earth"] =
            &RaidKarazhanTriggerContext::nightbane_ranged_bots_are_in_charred_earth;

        creators["nightbane main tank is susceptible to fear"] =
            &RaidKarazhanTriggerContext::nightbane_main_tank_is_susceptible_to_fear;

        creators["nightbane pets ignore collision to chase flying boss"] =
            &RaidKarazhanTriggerContext::nightbane_pets_ignore_collision_to_chase_flying_boss;

        creators["nightbane boss is flying"] =
            &RaidKarazhanTriggerContext::nightbane_boss_is_flying;

        creators["nightbane need to manage timers and trackers"] =
            &RaidKarazhanTriggerContext::nightbane_need_to_manage_timers_and_trackers;
    }

private:
    // Trash
    static Trigger* mana_warp_is_about_to_explode(
        PlayerbotAI* botAI) { return new ManaWarpIsAboutToExplodeTrigger(botAI); }

    // Attumen the Huntsman
    static Trigger* attumen_the_huntsman_need_target_priority(
        PlayerbotAI* botAI) { return new AttumenTheHuntsmanNeedTargetPriorityTrigger(botAI); }

    static Trigger* attumen_the_huntsman_attumen_spawned(
        PlayerbotAI* botAI) { return new AttumenTheHuntsmanAttumenSpawnedTrigger(botAI); }

    static Trigger* attumen_the_huntsman_attumen_is_mounted(
        PlayerbotAI* botAI) { return new AttumenTheHuntsmanAttumenIsMountedTrigger(botAI); }

    static Trigger* attumen_the_huntsman_boss_wipes_aggro_when_mounting(
        PlayerbotAI* botAI) { return new AttumenTheHuntsmanBossWipesAggroWhenMountingTrigger(botAI); }

    // Moroes
    static Trigger* moroes_boss_engaged_by_main_tank(
        PlayerbotAI* botAI) { return new MoroesBossEngagedByMainTankTrigger(botAI); }

    static Trigger* moroes_need_target_priority(
        PlayerbotAI* botAI) { return new MoroesNeedTargetPriorityTrigger(botAI); }

    // Maiden of Virtue
    static Trigger* maiden_of_virtue_healers_are_stunned_by_repentance(
        PlayerbotAI* botAI) { return new MaidenOfVirtueHealersAreStunnedByRepentanceTrigger(botAI); }

    static Trigger* maiden_of_virtue_holy_wrath_deals_chain_damage(
        PlayerbotAI* botAI) { return new MaidenOfVirtueHolyWrathDealsChainDamageTrigger(botAI); }

    // The Big Bad Wolf
    static Trigger* big_bad_wolf_boss_engaged_by_tank(
        PlayerbotAI* botAI) { return new BigBadWolfBossEngagedByTankTrigger(botAI); }

    static Trigger* big_bad_wolf_boss_is_chasing_little_red_riding_hood(
        PlayerbotAI* botAI) { return new BigBadWolfBossIsChasingLittleRedRidingHoodTrigger(botAI); }

    // Romulo and Julianne
    static Trigger* romulo_and_julianne_both_bosses_revived(
        PlayerbotAI* botAI) { return new RomuloAndJulianneBothBossesRevivedTrigger(botAI); }

    // The Wizard of Oz
    static Trigger* wizard_of_oz_need_target_priority(
        PlayerbotAI* botAI) { return new WizardOfOzNeedTargetPriorityTrigger(botAI); }

    static Trigger* wizard_of_oz_strawman_is_vulnerable_to_fire(
        PlayerbotAI* botAI) { return new WizardOfOzStrawmanIsVulnerableToFireTrigger(botAI); }

    // The Curator
    static Trigger* the_curator_astral_flare_spawned(
        PlayerbotAI* botAI) { return new TheCuratorAstralFlareSpawnedTrigger(botAI); }

    static Trigger* the_curator_boss_engaged_by_tanks(
        PlayerbotAI* botAI) { return new TheCuratorBossEngagedByTanksTrigger(botAI); }

    static Trigger* the_curator_astral_flares_cast_arcing_sear(
        PlayerbotAI* botAI) { return new TheCuratorBossAstralFlaresCastArcingSearTrigger(botAI); }

    // Terestian Illhoof
    static Trigger* terestian_illhoof_need_target_priority(
        PlayerbotAI* botAI) { return new TerestianIllhoofNeedTargetPriorityTrigger(botAI); }

    // Shade of Aran
    static Trigger* shade_of_aran_arcane_explosion_is_casting(
        PlayerbotAI* botAI) { return new ShadeOfAranArcaneExplosionIsCastingTrigger(botAI); }

    static Trigger* shade_of_aran_flame_wreath_is_active(
        PlayerbotAI* botAI) { return new ShadeOfAranFlameWreathIsActiveTrigger(botAI); }

    static Trigger* shade_of_aran_conjured_elementals_summoned(
        PlayerbotAI* botAI) { return new ShadeOfAranConjuredElementalsSummonedTrigger(botAI); }

    static Trigger* shade_of_aran_boss_uses_counterspell_and_blizzard(
        PlayerbotAI* botAI) { return new ShadeOfAranBossUsesCounterspellAndBlizzardTrigger(botAI); }

    // Netherspite
    static Trigger* netherspite_red_beam_is_active(
        PlayerbotAI* botAI) { return new NetherspiteRedBeamIsActiveTrigger(botAI); }

    static Trigger* netherspite_blue_beam_is_active(
        PlayerbotAI* botAI) { return new NetherspiteBlueBeamIsActiveTrigger(botAI); }

    static Trigger* netherspite_green_beam_is_active(
        PlayerbotAI* botAI) { return new NetherspiteGreenBeamIsActiveTrigger(botAI); }

    static Trigger* netherspite_bot_is_not_beam_blocker(
        PlayerbotAI* botAI) { return new NetherspiteBotIsNotBeamBlockerTrigger(botAI); }

    static Trigger* netherspite_boss_is_banished(
        PlayerbotAI* botAI) { return new NetherspiteBossIsBanishedTrigger(botAI); }

    static Trigger* netherspite_need_to_manage_timers_and_trackers(
        PlayerbotAI* botAI) { return new NetherspiteNeedToManageTimersAndTrackersTrigger(botAI); }

    // Prince Malchezaar
    static Trigger* prince_malchezaar_bot_is_enfeebled(
        PlayerbotAI* botAI) { return new PrinceMalchezaarBotIsEnfeebledTrigger(botAI); }

    static Trigger* prince_malchezaar_infernals_are_spawned(
        PlayerbotAI* botAI) { return new PrinceMalchezaarInfernalsAreSpawnedTrigger(botAI); }

    static Trigger* prince_malchezaar_boss_engaged_by_main_tank(
        PlayerbotAI* botAI) { return new PrinceMalchezaarBossEngagedByMainTankTrigger(botAI); }

    // Nightbane
    static Trigger* nightbane_boss_engaged_by_main_tank(
        PlayerbotAI* botAI) { return new NightbaneBossEngagedByMainTankTrigger(botAI); }

    static Trigger* nightbane_ranged_bots_are_in_charred_earth(
        PlayerbotAI* botAI) { return new NightbaneRangedBotsAreInCharredEarthTrigger(botAI); }

    static Trigger* nightbane_main_tank_is_susceptible_to_fear(
        PlayerbotAI* botAI) { return new NightbaneMainTankIsSusceptibleToFearTrigger(botAI); }

    static Trigger* nightbane_pets_ignore_collision_to_chase_flying_boss(
        PlayerbotAI* botAI) { return new NightbanePetsIgnoreCollisionToChaseFlyingBossTrigger(botAI); }

    static Trigger* nightbane_boss_is_flying(
        PlayerbotAI* botAI) { return new NightbaneBossIsFlyingTrigger(botAI); }

    static Trigger* nightbane_need_to_manage_timers_and_trackers(
        PlayerbotAI* botAI) { return new NightbaneNeedToManageTimersAndTrackersTrigger(botAI); }
};

#endif