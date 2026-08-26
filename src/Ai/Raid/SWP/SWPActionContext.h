/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_SWPACTIONCONTEXT_H
#define PLAYERBOTS_SWPACTIONCONTEXT_H

#include "NamedObjectContext.h"
#include "SWPActions.h"

class RaidSunwellActionContext : public NamedObjectContext<Action>
{
public:
    RaidSunwellActionContext()
    {
        // General
        creators["sunwell plateau reset encounter states"] =
            &RaidSunwellActionContext::sunwell_plateau_reset_encounter_states;

        creators["sunwell plateau remove aura"] =
            &RaidSunwellActionContext::sunwell_plateau_remove_aura;

        // Trash
        creators["volatile fiend keep enemy away from group"] =
            &RaidSunwellActionContext::volatile_fiend_keep_enemy_away_from_group;

        creators["apocalypse guard attack with holy magic"] =
            &RaidSunwellActionContext::apocalypse_guard_attack_with_holy_magic;

        // Kalecgos
        creators["kalecgos announce boss health"] =
            &RaidSunwellActionContext::kalecgos_announce_boss_health;

        creators["kalecgos misdirect boss to main tank"] =
            &RaidSunwellActionContext::kalecgos_misdirect_boss_to_main_tank;

        creators["kalecgos surface tank position dragon"] =
            &RaidSunwellActionContext::kalecgos_surface_tank_position_dragon;

        creators["kalecgos enter spectral rift"] =
            &RaidSunwellActionContext::kalecgos_enter_spectral_rift;

        creators["kalecgos disperse ranged"] =
            &RaidSunwellActionContext::kalecgos_disperse_ranged;

        creators["kalecgos remove arcane buffet"] =
            &RaidSunwellActionContext::kalecgos_remove_arcane_buffet;

        creators["kalecgos sathrovarr tank stand with kalec"] =
            &RaidSunwellActionContext::kalecgos_sathrovarr_tank_stand_with_kalec;

        creators["kalecgos return to spectral realm ground"] =
            &RaidSunwellActionContext::kalecgos_return_to_spectral_realm_ground;

        // Brutallus
        creators["brutallus misdirect boss to main tank"] =
            &RaidSunwellActionContext::brutallus_misdirect_boss_to_main_tank;

        creators["brutallus tanks position and swap"] =
            &RaidSunwellActionContext::brutallus_tanks_position_and_swap;

        creators["brutallus position melee at rear center"] =
            &RaidSunwellActionContext::brutallus_position_melee_at_rear_center;

        creators["brutallus position ranged in two groups"] =
            &RaidSunwellActionContext::brutallus_position_ranged_in_two_groups;

        creators["brutallus isolate burn"] =
            &RaidSunwellActionContext::brutallus_isolate_burn;

        // Felmyst
        creators["felmyst misdirect boss to main tank"] =
            &RaidSunwellActionContext::felmyst_misdirect_boss_to_main_tank;

        creators["felmyst main tank position boss on ground"] =
            &RaidSunwellActionContext::felmyst_main_tank_position_boss_on_ground;

        creators["felmyst ranged stack in three groups"] =
            &RaidSunwellActionContext::felmyst_ranged_stack_in_three_groups;

        creators["felmyst melee stack behind boss"] =
            &RaidSunwellActionContext::felmyst_melee_stack_behind_boss;

        creators["felmyst remove encapsulate"] =
            &RaidSunwellActionContext::felmyst_remove_encapsulate;

        creators["felmyst run away from encapsulated player"] =
            &RaidSunwellActionContext::felmyst_run_away_from_encapsulated_player;

        creators["felmyst mass dispel gas nova"] =
            &RaidSunwellActionContext::felmyst_mass_dispel_gas_nova;

        creators["felmyst avoid demonic vapor"] =
            &RaidSunwellActionContext::felmyst_avoid_demonic_vapor;

        creators["felmyst kite demonic vapor"] =
            &RaidSunwellActionContext::felmyst_kite_demonic_vapor;

        creators["felmyst move to safe fog lane"] =
            &RaidSunwellActionContext::felmyst_move_to_safe_fog_lane;

        creators["felmyst melee clear target"] =
            &RaidSunwellActionContext::felmyst_melee_clear_target;

        creators["felmyst kill charmed player"] =
            &RaidSunwellActionContext::felmyst_kill_charmed_player;

        creators["felmyst manage landing dps timer"] =
            &RaidSunwellActionContext::felmyst_manage_landing_dps_timer;

        // Eredar Twins
        creators["eredar twins melee jump from balcony"] =
            &RaidSunwellActionContext::eredar_twins_melee_jump_from_balcony;

        creators["eredar twins misdirect bosses to tanks"] =
            &RaidSunwellActionContext::eredar_twins_misdirect_bosses_to_tanks;

        creators["eredar twins position sacrolash tanks"] =
            &RaidSunwellActionContext::eredar_twins_position_sacrolash_tanks;

        creators["eredar twins alythess tank move out of blaze"] =
            &RaidSunwellActionContext::eredar_twins_alythess_tank_move_out_of_blaze;

        creators["eredar twins ranged stack at balcony edge"] =
            &RaidSunwellActionContext::eredar_twins_ranged_stack_at_balcony_edge;

        creators["eredar twins stack in room center"] =
            &RaidSunwellActionContext::eredar_twins_stack_in_room_center;

        creators["eredar twins remove flame sear"] =
            &RaidSunwellActionContext::eredar_twins_remove_flame_sear;

        creators["eredar twins dps prioritize sacrolash"] =
            &RaidSunwellActionContext::eredar_twins_dps_prioritize_sacrolash;

        creators["eredar twins conflagration target move from group"] =
            &RaidSunwellActionContext::eredar_twins_conflagration_target_move_from_group;

        creators["eredar twins move away from sacrolash victim"] =
            &RaidSunwellActionContext::eredar_twins_move_away_from_sacrolash_victim;

        // M'uru
        creators["m'uru misdirect enemies to tanks"] =
            &RaidSunwellActionContext::muru_misdirect_enemies_to_tanks;

        creators["m'uru main tank pick up entropius"] =
            &RaidSunwellActionContext::muru_main_tank_pick_up_entropius;

        creators["m'uru position ranged by phase"] =
            &RaidSunwellActionContext::muru_position_ranged_by_phase;

        creators["m'uru assign dps priority"] =
            &RaidSunwellActionContext::muru_assign_dps_priority;

        creators["m'uru kill dark fiends with dispel"] =
            &RaidSunwellActionContext::muru_kill_dark_fiends_with_dispel;

        creators["m'uru tanks move sentinel to safe position"] =
            &RaidSunwellActionContext::muru_tanks_move_sentinel_to_safe_position;

        creators["m'uru second assist tank guard ranged"] =
            &RaidSunwellActionContext::muru_second_assist_tank_guard_ranged;

        creators["m'uru melee flee the darkness"] =
            &RaidSunwellActionContext::muru_melee_flee_the_darkness;

        creators["m'uru cast stun on berserker"] =
            &RaidSunwellActionContext::muru_cast_stun_on_berserker;

        creators["m'uru interrupt fel fireball"] =
            &RaidSunwellActionContext::muru_interrupt_fel_fireball;

        creators["m'uru cast spellsteal on spell fury"] =
            &RaidSunwellActionContext::muru_cast_spellsteal_on_spell_fury;

        creators["m'uru warlock enslave void spawn"] =
            &RaidSunwellActionContext::muru_warlock_enslave_void_spawn;

        creators["m'uru void spawn cast shadow bolt volley"] =
            &RaidSunwellActionContext::muru_void_spawn_cast_shadow_bolt_volley;

        creators["m'uru keep distance from dark fiends"] =
            &RaidSunwellActionContext::muru_keep_distance_from_dark_fiends;

        creators["m'uru escape the singularity"] =
            &RaidSunwellActionContext::muru_escape_the_singularity;

        // Kil'jaeden <The Deceiver>
        creators["kil'jaeden announce dragon orb user"] =
            &RaidSunwellActionContext::kiljaeden_announce_dragon_orb_user;

        creators["kil'jaeden assign hands of the deceiver"] =
            &RaidSunwellActionContext::kiljaeden_assign_hands_of_the_deceiver;

        creators["kil'jaeden stun hands of the deceiver"] =
            &RaidSunwellActionContext::kiljaeden_stun_hands_of_the_deceiver;

        creators["kil'jaeden position and move tanks"] =
            &RaidSunwellActionContext::kiljaeden_position_and_move_tanks;

        creators["kil'jaeden position melee"] =
            &RaidSunwellActionContext::kiljaeden_position_melee;

        creators["kil'jaeden position ranged and avoid armageddons"] =
            &RaidSunwellActionContext::kiljaeden_position_ranged_and_avoid_armageddons;

        creators["kil'jaeden remove fire bloom"] =
            &RaidSunwellActionContext::kiljaeden_remove_fire_bloom;

        creators["kil'jaeden stack for shield of the blue"] =
            &RaidSunwellActionContext::kiljaeden_stack_for_shield_of_the_blue;

        creators["kil'jaeden use dragon orb"] =
            &RaidSunwellActionContext::kiljaeden_use_dragon_orb;

        creators["kil'jaeden release stale root"] =
            &RaidSunwellActionContext::kiljaeden_release_stale_root;

        creators["kil'jaeden dragon buff and protect raid"] =
            &RaidSunwellActionContext::kiljaeden_dragon_buff_and_protect_raid;
    }

private:
    // General
    static Action* sunwell_plateau_reset_encounter_states(PlayerbotAI* botAI) {
        return new SunwellPlateauResetEncounterStatesAction(botAI);
    }
    static Action* sunwell_plateau_remove_aura(PlayerbotAI* botAI) {
        return new SunwellPlateauRemoveAuraAction(botAI);
    }

    // Trash
    static Action* volatile_fiend_keep_enemy_away_from_group(PlayerbotAI* botAI) {
        return new VolatileFiendKeepEnemyAwayFromGroupAction(botAI);
    }
    static Action* apocalypse_guard_attack_with_holy_magic(PlayerbotAI* botAI) {
        return new ApocalypseGuardAttackWithHolyMagicAction(botAI);
    }

    // Kalecgos
    static Action* kalecgos_misdirect_boss_to_main_tank(PlayerbotAI* botAI) {
        return new SunwellPlateauMisdirectBossToMainTankAction(
            botAI, "kalecgos misdirect boss to main tank", "kalecgos");
    }
    static Action* kalecgos_announce_boss_health(PlayerbotAI* botAI) {
        return new KalecgosAnnounceBossHealthAction(botAI);
    }
    static Action* kalecgos_surface_tank_position_dragon(PlayerbotAI* botAI) {
        return new KalecgosSurfaceTankPositionDragonAction(botAI);
    }
    static Action* kalecgos_enter_spectral_rift(PlayerbotAI* botAI) {
        return new KalecgosEnterSpectralRiftAction(botAI);
    }
    static Action* kalecgos_disperse_ranged(PlayerbotAI* botAI) {
        return new KalecgosDisperseRangedAction(botAI);
    }
    static Action* kalecgos_remove_arcane_buffet(PlayerbotAI* botAI) {
        return new KalecgosRemoveArcaneBuffetAction(botAI);
    }
    static Action* kalecgos_sathrovarr_tank_stand_with_kalec(PlayerbotAI* botAI) {
        return new KalecgosSathrovarrTankStandWithKalecAction(botAI);
    }
    static Action* kalecgos_return_to_spectral_realm_ground(PlayerbotAI* botAI) {
        return new KalecgosReturnToSpectralRealmGroundAction(botAI);
    }

    // Brutallus
    static Action* brutallus_misdirect_boss_to_main_tank(PlayerbotAI* botAI) {
        return new SunwellPlateauMisdirectBossToMainTankAction(
            botAI, "brutallus misdirect boss to main tank", "brutallus");
    }
    static Action* brutallus_tanks_position_and_swap(PlayerbotAI* botAI) {
        return new BrutallusTanksPositionAndSwapAction(botAI);
    }
    static Action* brutallus_position_melee_at_rear_center(PlayerbotAI* botAI) {
        return new BrutallusPositionMeleeAtRearCenterAction(botAI);
    }
    static Action* brutallus_position_ranged_in_two_groups(PlayerbotAI* botAI) {
        return new BrutallusPositionRangedInTwoGroupsAction(botAI);
    }
    static Action* brutallus_isolate_burn(PlayerbotAI* botAI) {
        return new BrutallusIsolateBurnAction(botAI);
    }

    // Felmyst
    static Action* felmyst_misdirect_boss_to_main_tank(PlayerbotAI* botAI) {
        return new SunwellPlateauMisdirectBossToMainTankAction(
            botAI, "felmyst misdirect boss to main tank", "felmyst");
    }
    static Action* felmyst_main_tank_position_boss_on_ground(PlayerbotAI* botAI) {
        return new FelmystMainTankPositionBossOnGroundAction(botAI);
    }
    static Action* felmyst_ranged_stack_in_three_groups(PlayerbotAI* botAI) {
        return new FelmystRangedStackInThreeGroupsAction(botAI);
    }
    static Action* felmyst_melee_stack_behind_boss(PlayerbotAI* botAI) {
        return new FelmystMeleeStackBehindBossAction(botAI);
    }
    static Action* felmyst_remove_encapsulate(PlayerbotAI* botAI) {
        return new FelmystRemoveEncapsulateAction(botAI);
    }
    static Action* felmyst_run_away_from_encapsulated_player(PlayerbotAI* botAI) {
        return new FelmystRunAwayFromEncapsulatedPlayerAction(botAI);
    }
    static Action* felmyst_mass_dispel_gas_nova(PlayerbotAI* botAI) {
        return new FelmystMassDispelGasNovaAction(botAI);
    }
    static Action* felmyst_avoid_demonic_vapor(PlayerbotAI* botAI) {
        return new FelmystAvoidDemonicVaporAction(botAI);
    }
    static Action* felmyst_kite_demonic_vapor(PlayerbotAI* botAI) {
        return new FelmystKiteDemonicVaporAction(botAI);
    }
    static Action* felmyst_move_to_safe_fog_lane(PlayerbotAI* botAI) {
        return new FelmystMoveToSafeFogLaneAction(botAI);
    }
    static Action* felmyst_melee_clear_target(PlayerbotAI* botAI) {
        return new FelmystMeleeClearTargetAction(botAI);
    }
    static Action* felmyst_kill_charmed_player(PlayerbotAI* botAI) {
        return new FelmystKillCharmedPlayerAction(botAI);
    }
    static Action* felmyst_manage_landing_dps_timer(PlayerbotAI* botAI) {
        return new FelmystManageLandingDpsTimerAction(botAI);
    }

    // Eredar Twins
    static Action* eredar_twins_melee_jump_from_balcony(PlayerbotAI* botAI) {
        return new EredarTwinsMeleeJumpFromBalconyAction(botAI);
    }
    static Action* eredar_twins_misdirect_bosses_to_tanks(PlayerbotAI* botAI) {
        return new EredarTwinsMisdirectBossesToTanksAction(botAI);
    }
    static Action* eredar_twins_position_sacrolash_tanks(PlayerbotAI* botAI) {
        return new EredarTwinsPositionSacrolashTanksAction(botAI);
    }
    static Action* eredar_twins_alythess_tank_move_out_of_blaze(PlayerbotAI* botAI) {
        return new EredarTwinsAlythessTankMoveOutOfBlazeAction(botAI);
    }
    static Action* eredar_twins_ranged_stack_at_balcony_edge(PlayerbotAI* botAI) {
        return new EredarTwinsRangedStackAtBalconyEdgeAction(botAI);
    }
    static Action* eredar_twins_stack_in_room_center(PlayerbotAI* botAI) {
        return new EredarTwinsStackInRoomCenterAction(botAI);
    }
    static Action* eredar_twins_remove_flame_sear(PlayerbotAI* botAI) {
        return new EredarTwinsRemoveFlameSearAction(botAI);
    }
    static Action* eredar_twins_dps_prioritize_sacrolash(PlayerbotAI* botAI) {
        return new EredarTwinsDpsPrioritizeSacrolashAction(botAI);
    }
    static Action* eredar_twins_conflagration_target_move_from_group(PlayerbotAI* botAI) {
        return new EredarTwinsConflagrationTargetMoveFromGroupAction(botAI);
    }
    static Action* eredar_twins_move_away_from_sacrolash_victim(PlayerbotAI* botAI) {
        return new EredarTwinsMoveAwayFromSacrolashVictimAction(botAI);
    }

    // M'uru
    static Action* muru_misdirect_enemies_to_tanks(PlayerbotAI* botAI) {
        return new MuruMisdirectEnemiesToTanksAction(botAI);
    }
    static Action* muru_main_tank_pick_up_entropius(PlayerbotAI* botAI) {
        return new MuruMainTankPickUpEntropiusAction(botAI);
    }
    static Action* muru_position_ranged_by_phase(PlayerbotAI* botAI) {
        return new MuruPositionRangedByPhaseAction(botAI);
    }
    static Action* muru_assign_dps_priority(PlayerbotAI* botAI) {
        return new MuruAssignDpsPriorityAction(botAI);
    }
    static Action* muru_kill_dark_fiends_with_dispel(PlayerbotAI* botAI) {
        return new MuruKillDarkFiendsWithDispelAction(botAI);
    }
    static Action* muru_tanks_move_sentinel_to_safe_position(PlayerbotAI* botAI) {
        return new MuruTanksMoveSentinelToSafePositionAction(botAI);
    }
    static Action* muru_second_assist_tank_guard_ranged(PlayerbotAI* botAI) {
        return new MuruSecondAssistTankGuardRangedAction(botAI);
    }
    static Action* muru_melee_flee_the_darkness(PlayerbotAI* botAI) {
        return new MuruMeleeFleeTheDarknessAction(botAI);
    }
    static Action* muru_cast_stun_on_berserker(PlayerbotAI* botAI) {
        return new MuruCastStunOnBerserkerAction(botAI);
    }
    static Action* muru_interrupt_fel_fireball(PlayerbotAI* botAI) {
        return new MuruInterruptFelFireballAction(botAI);
    }
    static Action* muru_cast_spellsteal_on_spell_fury(PlayerbotAI* botAI) {
        return new MuruCastSpellStealOnSpellFuryAction(botAI);
    }
    static Action* muru_warlock_enslave_void_spawn(PlayerbotAI* botAI) {
        return new MuruWarlockEnslaveVoidSpawnAction(botAI);
    }
    static Action* muru_void_spawn_cast_shadow_bolt_volley(PlayerbotAI* botAI) {
        return new MuruVoidSpawnCastShadowBoltVolleyAction(botAI);
    }
    static Action* muru_keep_distance_from_dark_fiends(PlayerbotAI* botAI) {
        return new MuruKeepDistanceFromDarkFiendsAction(botAI);
    }
    static Action* muru_escape_the_singularity(PlayerbotAI* botAI) {
        return new MuruEscapeTheSingularityAction(botAI);
    }

    // Kil'jaeden <The Deceiver>
    static Action* kiljaeden_announce_dragon_orb_user(PlayerbotAI* botAI) {
        return new KiljaedenAnnounceDragonOrbUserAction(botAI);
    }
    static Action* kiljaeden_assign_hands_of_the_deceiver(PlayerbotAI* botAI) {
        return new KiljaedenAssignHandsOfTheDeceiverAction(botAI);
    }
    static Action* kiljaeden_stun_hands_of_the_deceiver(PlayerbotAI* botAI) {
        return new KiljaedenStunHandsOfTheDeceiverAction(botAI);
    }
    static Action* kiljaeden_position_and_move_tanks(PlayerbotAI* botAI) {
        return new KiljaedenPositionAndMoveTanksAction(botAI);
    }
    static Action* kiljaeden_position_melee(PlayerbotAI* botAI) {
        return new KiljaedenPositionMeleeAction(botAI);
    }
    static Action* kiljaeden_position_ranged_and_avoid_armageddons(PlayerbotAI* botAI) {
        return new KiljaedenPositionRangedAndAvoidArmageddonsAction(botAI);
    }
    static Action* kiljaeden_remove_fire_bloom(PlayerbotAI* botAI) {
        return new KiljaedenRemoveFireBloomAction(botAI);
    }
    static Action* kiljaeden_stack_for_shield_of_the_blue(PlayerbotAI* botAI) {
        return new KiljaedenStackForShieldOfTheBlueAction(botAI);
    }
    static Action* kiljaeden_use_dragon_orb(PlayerbotAI* botAI) {
        return new KiljaedenUseDragonOrbAction(botAI);
    }
    static Action* kiljaeden_release_stale_root(PlayerbotAI* botAI) {
        return new KiljaedenReleaseStaleRootAction(botAI);
    }
    static Action* kiljaeden_dragon_buff_and_protect_raid(PlayerbotAI* botAI) {
        return new KiljaedenDragonBuffAndProtectRaidAction(botAI);
    }
};

#endif
