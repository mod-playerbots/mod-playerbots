/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_SWPTRIGGERCONTEXT_H
#define PLAYERBOTS_SWPTRIGGERCONTEXT_H

#include "NamedObjectContext.h"
#include "SWPTriggers.h"

class RaidSunwellTriggerContext : public NamedObjectContext<Trigger>
{
public:
    RaidSunwellTriggerContext()
    {
        // General
        creators["sunwell plateau bot is not in combat"] =
            &RaidSunwellTriggerContext::sunwell_plateau_bot_is_not_in_combat;

        creators["sunwell plateau bot has protective aura"] =
            &RaidSunwellTriggerContext::sunwell_plateau_bot_has_protective_aura;

        // Trash
        creators["volatile fiend self destructs when near"] =
            &RaidSunwellTriggerContext::volatile_fiend_self_destructs_when_near;

        creators["apocalypse guard protected by infernal defense"] =
            &RaidSunwellTriggerContext::apocalypse_guard_protected_by_infernal_defense;

        // Kalecgos
        creators["kalecgos boss engaged by tank"] =
            &RaidSunwellTriggerContext::kalecgos_boss_engaged_by_tank;

        creators["kalecgos spectral rift is open"] =
            &RaidSunwellTriggerContext::kalecgos_spectral_rift_is_open;

        creators["kalecgos bots take splash damage"] =
            &RaidSunwellTriggerContext::kalecgos_bots_take_splash_damage;

        creators["kalecgos bot has too many arcane buffet stacks"] =
            &RaidSunwellTriggerContext::kalecgos_bot_has_too_many_arcane_buffet_stacks;

        creators["kalecgos humanoid kalec tanks sathrovarr"] =
            &RaidSunwellTriggerContext::kalecgos_humanoid_kalec_tanks_sathrovarr;

        creators["kalecgos bots don't observe gravity"] =
            &RaidSunwellTriggerContext::kalecgos_bots_dont_observe_gravity;

        // Brutallus
        creators["brutallus pulling boss"] =
            &RaidSunwellTriggerContext::brutallus_pulling_boss;

        creators["brutallus boss engaged by tanks"] =
            &RaidSunwellTriggerContext::brutallus_boss_engaged_by_tanks;

        creators["brutallus boss engaged by melee"] =
            &RaidSunwellTriggerContext::brutallus_boss_engaged_by_melee;

        creators["brutallus boss engaged by ranged"] =
            &RaidSunwellTriggerContext::brutallus_boss_engaged_by_ranged;

        creators["brutallus bot is burning"] =
            &RaidSunwellTriggerContext::brutallus_bot_is_burning;

        // Felmyst
        creators["felmyst pulling boss"] =
            &RaidSunwellTriggerContext::felmyst_pulling_boss;

        creators["felmyst boss engaged by main tank on ground"] =
            &RaidSunwellTriggerContext::felmyst_boss_engaged_by_main_tank_on_ground;

        creators["felmyst boss engaged by ranged on ground"] =
            &RaidSunwellTriggerContext::felmyst_boss_engaged_by_ranged_on_ground;

        creators["felmyst boss engaged by melee on ground"] =
            &RaidSunwellTriggerContext::felmyst_boss_engaged_by_melee_on_ground;

        creators["felmyst bot is encapsulated"] =
            &RaidSunwellTriggerContext::felmyst_bot_is_encapsulated;

        creators["felmyst bot near encapsulated player"] =
            &RaidSunwellTriggerContext::felmyst_bot_near_encapsulated_player;

        creators["felmyst player has gas nova"] =
            &RaidSunwellTriggerContext::felmyst_player_has_gas_nova;

        creators["felmyst demonic vapor trails are active"] =
            &RaidSunwellTriggerContext::felmyst_demonic_vapor_trails_are_active;

        creators["felmyst bot is demonic vapor target"] =
            &RaidSunwellTriggerContext::felmyst_bot_is_demonic_vapor_target;

        creators["felmyst fog of corruption is active"] =
            &RaidSunwellTriggerContext::felmyst_fog_of_corruption_is_active;

        creators["felmyst melee cannot reach boss"] =
            &RaidSunwellTriggerContext::felmyst_melee_cannot_reach_boss;

        creators["felmyst player is charmed by fog"] =
            &RaidSunwellTriggerContext::felmyst_player_is_charmed_by_fog;

        // Eredar Twins
        creators["eredar twins melee is at balcony"] =
            &RaidSunwellTriggerContext::eredar_twins_melee_is_at_balcony;

        creators["eredar twins pulling bosses"] =
            &RaidSunwellTriggerContext::eredar_twins_pulling_bosses;

        creators["eredar twins sacrolash engaged by two tanks"] =
            &RaidSunwellTriggerContext::eredar_twins_sacrolash_engaged_by_two_tanks;

        creators["eredar twins alythess engaged by first assist tank"] =
            &RaidSunwellTriggerContext::eredar_twins_alythess_engaged_by_first_assist_tank;

        creators["eredar twins bosses engaged by ranged"] =
            &RaidSunwellTriggerContext::eredar_twins_bosses_engaged_by_ranged;

        creators["eredar twins only one boss remains"] =
            &RaidSunwellTriggerContext::eredar_twins_only_one_boss_remains;

        creators["eredar twins bot has too many flame touched stacks"] =
            &RaidSunwellTriggerContext::eredar_twins_bot_has_too_many_flame_touched_stacks;

        creators["eredar twins determining dps priority"] =
            &RaidSunwellTriggerContext::eredar_twins_determining_dps_priority;

        creators["eredar twins bot has conflagration"] =
            &RaidSunwellTriggerContext::eredar_twins_bot_has_conflagration;

        creators["eredar twins sacrolash victim has conflagration"] =
            &RaidSunwellTriggerContext::eredar_twins_sacrolash_victim_has_conflagration;

        // M'uru
        creators["m'uru void sentinel or entropius has appeared"] =
            &RaidSunwellTriggerContext::muru_void_sentinel_or_entropius_has_appeared;

        creators["m'uru boss transformed into entropius"] =
            &RaidSunwellTriggerContext::muru_boss_transformed_into_entropius;

        creators["m'uru bosses engaged by ranged"] =
            &RaidSunwellTriggerContext::muru_bosses_engaged_by_ranged;

        creators["m'uru determining dps priority"] =
            &RaidSunwellTriggerContext::muru_determining_dps_priority;

        creators["m'uru void sentinel pulses shadow"] =
            &RaidSunwellTriggerContext::muru_void_sentinel_pulses_shadow;

        creators["m'uru adds spawn at entrance"] =
            &RaidSunwellTriggerContext::muru_adds_spawn_at_entrance;

        creators["m'uru dark fiends spawned"] =
            &RaidSunwellTriggerContext::muru_dark_fiends_spawned;

        creators["m'uru entropius spawns darkness pools"] =
            &RaidSunwellTriggerContext::muru_entropius_spawns_darkness_pools;

        creators["m'uru darkness is coming"] =
            &RaidSunwellTriggerContext::muru_darkness_is_coming;

        creators["m'uru the singularity is near"] =
            &RaidSunwellTriggerContext::muru_the_singularity_is_near;

        creators["m'uru berserker is buffed with flurry"] =
            &RaidSunwellTriggerContext::muru_berserker_is_buffed_with_flurry;

        creators["m'uru fury mage casting fel fireball"] =
            &RaidSunwellTriggerContext::muru_fury_mage_casting_fel_fireball;

        creators["m'uru fury mage is buffed with spell fury"] =
            &RaidSunwellTriggerContext::muru_fury_mage_is_buffed_with_spell_fury;

        creators["m'uru void spawn available for enslave"] =
            &RaidSunwellTriggerContext::muru_void_spawn_available_for_enslave;

        creators["m'uru warlock has enslaved void spawn"] =
            &RaidSunwellTriggerContext::muru_warlock_has_enslaved_void_spawn;

        // Kil'jaeden <The Deceiver>
        creators["kil'jaeden encounter has begun"] =
            &RaidSunwellTriggerContext::kiljaeden_encounter_has_begun;

        creators["kil'jaeden hands of the deceiver are active"] =
            &RaidSunwellTriggerContext::kiljaeden_hands_of_the_deceiver_are_active;

        creators["kil'jaeden boss engaged by tanks"] =
            &RaidSunwellTriggerContext::kiljaeden_boss_engaged_by_tanks;

        creators["kil'jaeden boss engaged by melee"] =
            &RaidSunwellTriggerContext::kiljaeden_boss_engaged_by_melee;

        creators["kil'jaeden boss engaged by ranged"] =
            &RaidSunwellTriggerContext::kiljaeden_boss_engaged_by_ranged;

        creators["kil'jaeden bot has fire bloom"] =
            &RaidSunwellTriggerContext::kiljaeden_bot_has_fire_bloom;

        creators["kil'jaeden says: Chaos! Destruction! Oblivion!"] =
            &RaidSunwellTriggerContext::kiljaeden_says_chaos_destruction_oblivion;

        creators["kil'jaeden dragon orb is active"] =
            &RaidSunwellTriggerContext::kiljaeden_dragon_orb_is_active;

        creators["kil'jaeden bot has stale root after dragon"] =
            &RaidSunwellTriggerContext::kiljaeden_bot_has_stale_root_after_dragon;

        creators["kil'jaeden bot controls dragon"] =
            &RaidSunwellTriggerContext::kiljaeden_bot_controls_dragon;
    }

private:
    // General
    static Trigger* sunwell_plateau_bot_is_not_in_combat(PlayerbotAI* botAI) {
        return new SunwellPlateauBotIsNotInCombatTrigger(botAI);
    }
    static Trigger* sunwell_plateau_bot_has_protective_aura(PlayerbotAI* botAI) {
        return new SunwellPlateauBotHasProtectiveAuraTrigger(botAI);
    }

    // Trash
    static Trigger* volatile_fiend_self_destructs_when_near(PlayerbotAI* botAI) {
        return new VolatileFiendSelfDestructsWhenNearTrigger(botAI);
    }
    static Trigger* apocalypse_guard_protected_by_infernal_defense(PlayerbotAI* botAI) {
        return new ApocalypseGuardProtectedByInfernalDefenseTrigger(botAI);
    }

    // Kalecgos
    static Trigger* kalecgos_boss_engaged_by_tank(PlayerbotAI* botAI) {
        return new KalecgosBossEngagedByTankTrigger(botAI);
    }
    static Trigger* kalecgos_spectral_rift_is_open(PlayerbotAI* botAI) {
        return new KalecgosSpectralRiftIsOpenTrigger(botAI);
    }
    static Trigger* kalecgos_bots_take_splash_damage(PlayerbotAI* botAI) {
        return new KalecgosBotsTakeSplashDamageTrigger(botAI);
    }
    static Trigger* kalecgos_humanoid_kalec_tanks_sathrovarr(PlayerbotAI* botAI) {
        return new KalecgosHumanoidKalecTanksSathrovarrTrigger(botAI);
    }
    static Trigger* kalecgos_bot_has_too_many_arcane_buffet_stacks(PlayerbotAI* botAI) {
        return new KalecgosBotHasTooManyArcaneBuffetStacksTrigger(botAI);
    }
    static Trigger* kalecgos_bots_dont_observe_gravity(PlayerbotAI* botAI) {
        return new KalecgosBotsDontObserveGravityTrigger(botAI);
    }

    // Brutallus
    static Trigger* brutallus_pulling_boss(PlayerbotAI* botAI) {
        return new BrutallusPullingBossTrigger(botAI);
    }
    static Trigger* brutallus_boss_engaged_by_tanks(PlayerbotAI* botAI) {
        return new BrutallusBossEngagedByTanksTrigger(botAI);
    }
    static Trigger* brutallus_boss_engaged_by_melee(PlayerbotAI* botAI) {
        return new BrutallusBossEngagedByMeleeTrigger(botAI);
    }
    static Trigger* brutallus_boss_engaged_by_ranged(PlayerbotAI* botAI) {
        return new BrutallusBossEngagedByRangedTrigger(botAI);
    }
    static Trigger* brutallus_bot_is_burning(PlayerbotAI* botAI) {
        return new BrutallusBotIsBurningTrigger(botAI);
    }

    // Felmyst
    static Trigger* felmyst_pulling_boss(PlayerbotAI* botAI) {
        return new FelmystPullingBossTrigger(botAI);
    }
    static Trigger* felmyst_boss_engaged_by_main_tank_on_ground(PlayerbotAI* botAI) {
        return new FelmystBossEngagedByMainTankOnGroundTrigger(botAI);
    }
    static Trigger* felmyst_boss_engaged_by_ranged_on_ground(PlayerbotAI* botAI) {
        return new FelmystBossEngagedByRangedOnGroundTrigger(botAI);
    }
    static Trigger* felmyst_boss_engaged_by_melee_on_ground(PlayerbotAI* botAI) {
        return new FelmystBossEngagedByMeleeOnGroundTrigger(botAI);
    }
    static Trigger* felmyst_bot_is_encapsulated(PlayerbotAI* botAI) {
        return new FelmystBotIsEncapsulatedTrigger(botAI);
    }
    static Trigger* felmyst_bot_near_encapsulated_player(PlayerbotAI* botAI) {
        return new FelmystBotNearEncapsulatedPlayerTrigger(botAI);
    }
    static Trigger* felmyst_player_has_gas_nova(PlayerbotAI* botAI) {
        return new FelmystPlayerHasGasNovaTrigger(botAI);
    }
    static Trigger* felmyst_demonic_vapor_trails_are_active(PlayerbotAI* botAI) {
        return new FelmystDemonicVaporTrailsAreActiveTrigger(botAI);
    }
    static Trigger* felmyst_bot_is_demonic_vapor_target(PlayerbotAI* botAI) {
        return new FelmystBotIsDemonicVaporTargetTrigger(botAI);
    }
    static Trigger* felmyst_fog_of_corruption_is_active(PlayerbotAI* botAI) {
        return new FelmystFogOfCorruptionIsActiveTrigger(botAI);
    }
    static Trigger* felmyst_melee_cannot_reach_boss(PlayerbotAI* botAI) {
        return new FelmystMeleeCannotReachBossTrigger(botAI);
    }
    static Trigger* felmyst_player_is_charmed_by_fog(PlayerbotAI* botAI) {
        return new FelmystPlayerIsCharmedByFogTrigger(botAI);
    }

    // Eredar Twins
    static Trigger* eredar_twins_melee_is_at_balcony(PlayerbotAI* botAI) {
        return new EredarTwinsMeleeIsAtBalconyTrigger(botAI);
    }
    static Trigger* eredar_twins_pulling_bosses(PlayerbotAI* botAI) {
        return new EredarTwinsPullingBossesTrigger(botAI);
    }
    static Trigger* eredar_twins_sacrolash_engaged_by_two_tanks(PlayerbotAI* botAI) {
        return new EredarTwinsSacrolashEngagedByTwoTanksTrigger(botAI);
    }
    static Trigger* eredar_twins_alythess_engaged_by_first_assist_tank(PlayerbotAI* botAI) {
        return new EredarTwinsAlythessEngagedByFirstAssistTankTrigger(botAI);
    }
    static Trigger* eredar_twins_bosses_engaged_by_ranged(PlayerbotAI* botAI) {
        return new EredarTwinsBossesEngagedByRangedTrigger(botAI);
    }
    static Trigger* eredar_twins_only_one_boss_remains(PlayerbotAI* botAI) {
        return new EredarTwinsOnlyOneBossRemainsTrigger(botAI);
    }
    static Trigger* eredar_twins_bot_has_too_many_flame_touched_stacks(PlayerbotAI* botAI) {
        return new EredarTwinsBotHasTooManyFlameTouchedStacksTrigger(botAI);
    }
    static Trigger* eredar_twins_determining_dps_priority(PlayerbotAI* botAI) {
        return new EredarTwinsDeterminingDpsPriorityTrigger(botAI);
    }
    static Trigger* eredar_twins_bot_has_conflagration(PlayerbotAI* botAI) {
        return new EredarTwinsBotHasConflagrationTrigger(botAI);
    }
    static Trigger* eredar_twins_sacrolash_victim_has_conflagration(PlayerbotAI* botAI) {
        return new EredarTwinsSacrolashVictimHasConflagrationTrigger(botAI);
    }

    // M'uru
    static Trigger* muru_void_sentinel_or_entropius_has_appeared(PlayerbotAI* botAI) {
        return new MuruVoidSentinelOrEntropiusHasAppearedTrigger(botAI);
    }
    static Trigger* muru_boss_transformed_into_entropius(PlayerbotAI* botAI) {
        return new MuruBossTransformedIntoEntropiusTrigger(botAI);
    }
    static Trigger* muru_bosses_engaged_by_ranged(PlayerbotAI* botAI) {
        return new MuruBossesEngagedByRangedTrigger(botAI);
    }
    static Trigger* muru_determining_dps_priority(PlayerbotAI* botAI) {
        return new MuruDeterminingDpsPriorityTrigger(botAI);
    }
    static Trigger* muru_void_sentinel_pulses_shadow(PlayerbotAI* botAI) {
        return new MuruVoidSentinelPulsesShadowTrigger(botAI);
    }
    static Trigger* muru_adds_spawn_at_entrance(PlayerbotAI* botAI) {
        return new MuruAddsSpawnAtEntranceTrigger(botAI);
    }
    static Trigger* muru_dark_fiends_spawned(PlayerbotAI* botAI) {
        return new MuruDarkFiendsSpawnedTrigger(botAI);
    }
    static Trigger* muru_entropius_spawns_darkness_pools(PlayerbotAI* botAI) {
        return new MuruEntropiusSpawnsDarknessPoolsTrigger(botAI);
    }
    static Trigger* muru_darkness_is_coming(PlayerbotAI* botAI) {
        return new MuruDarknessIsComingTrigger(botAI);
    }
    static Trigger* muru_the_singularity_is_near(PlayerbotAI* botAI) {
        return new MuruTheSingularityIsNearTrigger(botAI);
    }
    static Trigger* muru_berserker_is_buffed_with_flurry(PlayerbotAI* botAI) {
        return new MuruBerserkerIsBuffedWithFlurryTrigger(botAI);
    }
    static Trigger* muru_fury_mage_casting_fel_fireball(PlayerbotAI* botAI) {
        return new MuruFuryMageCastingFelFireballTrigger(botAI);
    }
    static Trigger* muru_fury_mage_is_buffed_with_spell_fury(PlayerbotAI* botAI) {
        return new MuruFuryMageIsBuffedWithSpellFuryTrigger(botAI);
    }
    static Trigger* muru_void_spawn_available_for_enslave(PlayerbotAI* botAI) {
        return new MuruVoidSpawnAvailableForEnslaveTrigger(botAI);
    }
    static Trigger* muru_warlock_has_enslaved_void_spawn(PlayerbotAI* botAI) {
        return new MuruWarlockHasEnslavedVoidSpawnTrigger(botAI);
    }

    // Kil'jaeden <The Deceiver>
    static Trigger* kiljaeden_encounter_has_begun(PlayerbotAI* botAI) {
        return new KiljaedenEncounterHasBegunTrigger(botAI);
    }
    static Trigger* kiljaeden_hands_of_the_deceiver_are_active(PlayerbotAI* botAI) {
        return new KiljaedenHandsOfTheDeceiverAreActiveTrigger(botAI);
    }
    static Trigger* kiljaeden_boss_engaged_by_tanks(PlayerbotAI* botAI) {
        return new KiljaedenBossEngagedByTanksTrigger(botAI);
    }
    static Trigger* kiljaeden_boss_engaged_by_melee(PlayerbotAI* botAI) {
        return new KiljaedenBossEngagedByMeleeTrigger(botAI);
    }
    static Trigger* kiljaeden_boss_engaged_by_ranged(PlayerbotAI* botAI) {
        return new KiljaedenBossEngagedByRangedTrigger(botAI);
    }
    static Trigger* kiljaeden_bot_has_fire_bloom(PlayerbotAI* botAI) {
        return new KiljaedenBotHasFireBloomTrigger(botAI);
    }
    static Trigger* kiljaeden_says_chaos_destruction_oblivion(PlayerbotAI* botAI) {
        return new KiljaedenSaysChaosDestructionOblivionTrigger(botAI);
    }
    static Trigger* kiljaeden_dragon_orb_is_active(PlayerbotAI* botAI) {
        return new KiljaedenDragonOrbIsActiveTrigger(botAI);
    }
    static Trigger* kiljaeden_bot_has_stale_root_after_dragon(PlayerbotAI* botAI) {
        return new KiljaedenBotHasStaleRootAfterDragonTrigger(botAI);
    }
    static Trigger* kiljaeden_bot_controls_dragon(PlayerbotAI* botAI) {
        return new KiljaedenBotControlsDragonTrigger(botAI);
    }
};

#endif
