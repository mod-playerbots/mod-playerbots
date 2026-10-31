/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_BTACTIONCONTEXT_H
#define PLAYERBOTS_BTACTIONCONTEXT_H

#include "BTActions.h"
#include "NamedObjectContext.h"

class RaidBlackTempleActionContext : public NamedObjectContext<Action>
{
public:
    RaidBlackTempleActionContext()
    {
        // Shared
        creators["black temple reset encounter states"] =
            &RaidBlackTempleActionContext::black_temple_reset_encounter_states;

        creators["black temple assign class roles"] =
            &RaidBlackTempleActionContext::black_temple_assign_class_roles;

        // Trash
        creators["sister of pleasure focus dps"] =
            &RaidBlackTempleActionContext::sister_of_pleasure_focus_dps;

        creators["shadowmoon reaver stop wand"] =
            &RaidBlackTempleActionContext::shadowmoon_reaver_stop_wand;

        creators["shadowmoon reaver control caster pet"] =
            &RaidBlackTempleActionContext::shadowmoon_reaver_control_caster_pet;

        // High Warlord Naj'entus
        creators["high warlord naj'entus misdirect to main tank"] =
            &RaidBlackTempleActionContext::high_warlord_najentus_misdirect_to_main_tank;

        creators["high warlord naj'entus tanks position boss"] =
            &RaidBlackTempleActionContext::high_warlord_najentus_tanks_position_boss;

        creators["high warlord naj'entus spread ranged"] =
            &RaidBlackTempleActionContext::high_warlord_najentus_spread_ranged;

        creators["high warlord naj'entus assign spine remover"] =
            &RaidBlackTempleActionContext::high_warlord_najentus_assign_spine_remover;

        creators["high warlord naj'entus remove impaling spine"] =
            &RaidBlackTempleActionContext::high_warlord_najentus_remove_impaling_spine;

        creators["high warlord naj'entus assign spine thrower"] =
            &RaidBlackTempleActionContext::high_warlord_najentus_assign_spine_thrower;

        creators["high warlord naj'entus throw impaling spine"] =
            &RaidBlackTempleActionContext::high_warlord_najentus_throw_impaling_spine;

        // Supremus
        creators["supremus misdirect to main tank"] =
            &RaidBlackTempleActionContext::supremus_misdirect_to_main_tank;

        creators["supremus tanks position boss"] =
            &RaidBlackTempleActionContext::supremus_tanks_position_boss;

        creators["supremus spread ranged"] = &RaidBlackTempleActionContext::supremus_spread_ranged;

        creators["supremus kite boss"] = &RaidBlackTempleActionContext::supremus_kite_boss;

        creators["supremus move away from fire"] =
            &RaidBlackTempleActionContext::supremus_move_away_from_fire;

        creators["supremus reach around fire"] =
            &RaidBlackTempleActionContext::supremus_reach_around_fire;

        // Shade of Akama
        creators["shade of akama attack channelers and sorcerers"] =
            &RaidBlackTempleActionContext::shade_of_akama_attack_channelers_and_sorcerers;

        // Teron Gorefiend
        creators["teron gorefiend misdirect to main tank"] =
            &RaidBlackTempleActionContext::teron_gorefiend_misdirect_to_main_tank;

        creators["teron gorefiend tanks position boss"] =
            &RaidBlackTempleActionContext::teron_gorefiend_tanks_position_boss;

        creators["teron gorefiend position ranged on balcony"] =
            &RaidBlackTempleActionContext::teron_gorefiend_position_ranged_on_balcony;

        creators["teron gorefiend avoid shadow of death"] =
            &RaidBlackTempleActionContext::teron_gorefiend_avoid_shadow_of_death;

        creators["teron gorefiend move to corner to die"] =
            &RaidBlackTempleActionContext::teron_gorefiend_move_to_corner_to_die;

        creators["teron gorefiend destroy shadowy constructs"] =
            &RaidBlackTempleActionContext::teron_gorefiend_destroy_shadowy_constructs;

        // Gurtogg Bloodboil
        creators["gurtogg bloodboil misdirect to main tank"] =
            &RaidBlackTempleActionContext::gurtogg_bloodboil_misdirect_to_main_tank;

        creators["gurtogg bloodboil tanks position boss"] =
            &RaidBlackTempleActionContext::gurtogg_bloodboil_tanks_position_boss;

        creators["gurtogg bloodboil rotate ranged groups"] =
            &RaidBlackTempleActionContext::gurtogg_bloodboil_rotate_ranged_groups;

        creators["gurtogg bloodboil melee move behind"] =
            &RaidBlackTempleActionContext::gurtogg_bloodboil_melee_move_behind;

        creators["gurtogg bloodboil ranged move out of frontal cone"] =
            &RaidBlackTempleActionContext::gurtogg_bloodboil_ranged_move_out_of_frontal_cone;

        creators["gurtogg bloodboil lead boss to tank position"] =
            &RaidBlackTempleActionContext::gurtogg_bloodboil_lead_boss_to_tank_position;

        // Reliquary of Souls
        creators["reliquary of souls misdirect to main tank"] =
            &RaidBlackTempleActionContext::reliquary_of_souls_misdirect_to_main_tank;

        creators["reliquary of souls adjust distance from suffering"] =
            &RaidBlackTempleActionContext::reliquary_of_souls_adjust_distance_from_suffering;

        creators["reliquary of souls healers dps suffering"] =
            &RaidBlackTempleActionContext::reliquary_of_souls_healers_dps_suffering;

        creators["reliquary of souls spellsteal rune shield"] =
            &RaidBlackTempleActionContext::reliquary_of_souls_spellsteal_rune_shield;

        creators["reliquary of souls spell reflect deaden"] =
            &RaidBlackTempleActionContext::reliquary_of_souls_spell_reflect_deaden;

        // Mother Shahraz
        creators["mother shahraz misdirect to main tank"] =
            &RaidBlackTempleActionContext::mother_shahraz_misdirect_to_main_tank;

        creators["mother shahraz tanks position boss under pillar"] =
            &RaidBlackTempleActionContext::mother_shahraz_tanks_position_boss_under_pillar;

        creators["mother shahraz melee dps wait at safe position"] =
            &RaidBlackTempleActionContext::mother_shahraz_melee_dps_wait_at_safe_position;

        creators["mother shahraz position ranged under statue"] =
            &RaidBlackTempleActionContext::mother_shahraz_position_ranged_under_statue;

        creators["mother shahraz break fatal attraction"] =
            &RaidBlackTempleActionContext::mother_shahraz_break_fatal_attraction;

        // Illidari Council
        creators["illidari council misdirect to tanks"] =
            &RaidBlackTempleActionContext::illidari_council_misdirect_to_tanks;

        creators["illidari council main tank position gathios"] =
            &RaidBlackTempleActionContext::illidari_council_main_tank_position_gathios;

        creators["illidari council main tank reflect judgement of command"] =
            &RaidBlackTempleActionContext::illidari_council_main_tank_reflect_judgement_of_command;

        creators["illidari council interrupt circle of healing"] =
            &RaidBlackTempleActionContext::illidari_council_interrupt_circle_of_healing;

        creators["illidari council first assist tank focus malande"] =
            &RaidBlackTempleActionContext::illidari_council_first_assist_tank_focus_malande;

        creators["illidari council second assist tank position veras"] =
            &RaidBlackTempleActionContext::illidari_council_second_assist_tank_position_veras;

        creators["illidari council mage tank position zerevor"] =
            &RaidBlackTempleActionContext::illidari_council_mage_tank_position_zerevor;

        creators["illidari council position mage tank healer"] =
            &RaidBlackTempleActionContext::illidari_council_position_mage_tank_healer;

        creators["illidari council spread ranged"] =
            &RaidBlackTempleActionContext::illidari_council_spread_ranged;

        creators["illidari council command pet target"] =
            &RaidBlackTempleActionContext::illidari_council_command_pet_target;

        creators["illidari council assign dps targets"] =
            &RaidBlackTempleActionContext::illidari_council_assign_dps_targets;

        creators["illidari council manage dps timer"] =
            &RaidBlackTempleActionContext::illidari_council_manage_dps_timer;

        creators["illidari council announce class roles"] =
            &RaidBlackTempleActionContext::illidari_council_announce_class_roles;

        // Illidan Stormrage <The Betrayer>
        creators["illidan stormrage position on grate during speech"] =
            &RaidBlackTempleActionContext::illidan_stormrage_position_on_grate_during_speech;

        creators["illidan stormrage misdirect to tanks"] =
            &RaidBlackTempleActionContext::illidan_stormrage_misdirect_to_tanks;

        creators["illidan stormrage main tank reposition boss"] =
            &RaidBlackTempleActionContext::illidan_stormrage_main_tank_reposition_boss;

        creators["illidan stormrage isolate bot with infection"] =
            &RaidBlackTempleActionContext::illidan_stormrage_isolate_bot_with_infection;

        creators["illidan stormrage track infections"] =
            &RaidBlackTempleActionContext::illidan_stormrage_track_infections;

        creators["illidan stormrage assist tanks handle flames"] =
            &RaidBlackTempleActionContext::illidan_stormrage_assist_tanks_handle_flames;

        creators["illidan stormrage control pet aggression"] =
            &RaidBlackTempleActionContext::illidan_stormrage_control_pet_aggression;

        creators["illidan stormrage command pet attack add"] =
            &RaidBlackTempleActionContext::illidan_stormrage_command_pet_attack_add;

        creators["illidan stormrage position above grate"] =
            &RaidBlackTempleActionContext::illidan_stormrage_position_above_grate;

        creators["illidan stormrage clear debuff with immunity"] =
            &RaidBlackTempleActionContext::illidan_stormrage_clear_debuff_with_immunity;

        creators["illidan stormrage cancel immunity"] =
            &RaidBlackTempleActionContext::illidan_stormrage_cancel_immunity;

        creators["illidan stormrage recall totems"] =
            &RaidBlackTempleActionContext::illidan_stormrage_recall_totems;

        creators["illidan stormrage position for landing"] =
            &RaidBlackTempleActionContext::illidan_stormrage_position_for_landing;

        creators["illidan stormrage melee stop attacking"] =
            &RaidBlackTempleActionContext::illidan_stormrage_melee_stop_attacking;

        creators["illidan stormrage stop shooting"] =
            &RaidBlackTempleActionContext::illidan_stormrage_stop_shooting;

        creators["illidan stormrage spread ranged"] =
            &RaidBlackTempleActionContext::illidan_stormrage_spread_ranged;

        creators["illidan stormrage reach shadow demon"] =
            &RaidBlackTempleActionContext::illidan_stormrage_reach_shadow_demon;

        creators["illidan stormrage melee stay away from demon form"] =
            &RaidBlackTempleActionContext::illidan_stormrage_melee_stay_away_from_demon_form;

        creators["illidan stormrage warlock tank handle demon form"] =
            &RaidBlackTempleActionContext::illidan_stormrage_warlock_tank_handle_demon_form;

        creators["illidan stormrage seed shadow demons"] =
            &RaidBlackTempleActionContext::illidan_stormrage_seed_shadow_demons;

        creators["illidan stormrage assign target priority"] =
            &RaidBlackTempleActionContext::illidan_stormrage_assign_target_priority;

        creators["illidan stormrage manage dps timers"] =
            &RaidBlackTempleActionContext::illidan_stormrage_manage_dps_timers;

        creators["illidan stormrage clear hazards"] =
            &RaidBlackTempleActionContext::illidan_stormrage_clear_hazards;

        creators["illidan stormrage announce class roles"] =
            &RaidBlackTempleActionContext::illidan_stormrage_announce_class_roles;
    }

private:
    // Shared
    static Action* black_temple_reset_encounter_states(PlayerbotAI* botAI)
    {
        return new BlackTempleResetEncounterStatesAction(botAI);
    }
    static Action* black_temple_assign_class_roles(PlayerbotAI* botAI)
    {
        return new BlackTempleAssignClassRolesAction(botAI);
    }

    // Trash
    static Action* sister_of_pleasure_focus_dps(PlayerbotAI* botAI)
    {
        return new SisterOfPleasureFocusDpsAction(botAI);
    }
    static Action* shadowmoon_reaver_stop_wand(PlayerbotAI* botAI)
    {
        return new ShadowmoonReaverStopWandAction(botAI);
    }
    static Action* shadowmoon_reaver_control_caster_pet(PlayerbotAI* botAI)
    {
        return new ShadowmoonReaverControlCasterPetAction(botAI);
    }

    // High Warlord Naj'entus
    static Action* high_warlord_najentus_misdirect_to_main_tank(PlayerbotAI* botAI)
    {
        return new BlackTempleMisdirectToMainTankAction(
            botAI, "high warlord naj'entus misdirect to main tank",
            BtHelpers::BtBosses::DATA_HIGH_WARLORD_NAJENTUS);
    }
    static Action* high_warlord_najentus_tanks_position_boss(PlayerbotAI* botAI)
    {
        return new BlackTemplePositionBossAction(
            botAI, "high warlord naj'entus tanks position boss",
            BtHelpers::BtBosses::DATA_HIGH_WARLORD_NAJENTUS, BtHelpers::NAJENTUS_TANK_POSITION);
    }
    static Action* high_warlord_najentus_spread_ranged(PlayerbotAI* botAI)
    {
        return new BlackTempleSpreadRangedAction(
            botAI, "high warlord naj'entus spread ranged",
            BtHelpers::NAJENTUS_RANGED_SPREAD_DISTANCE);
    }
    static Action* high_warlord_najentus_assign_spine_remover(PlayerbotAI* botAI)
    {
        return new HighWarlordNajentusAssignSpineRemoverAction(botAI);
    }
    static Action* high_warlord_najentus_remove_impaling_spine(PlayerbotAI* botAI)
    {
        return new HighWarlordNajentusRemoveImpalingSpineAction(botAI);
    }
    static Action* high_warlord_najentus_assign_spine_thrower(PlayerbotAI* botAI)
    {
        return new HighWarlordNajentusAssignSpineThrowerAction(botAI);
    }
    static Action* high_warlord_najentus_throw_impaling_spine(PlayerbotAI* botAI)
    {
        return new HighWarlordNajentusThrowImpalingSpineAction(botAI);
    }

    // Supremus
    static Action* supremus_misdirect_to_main_tank(PlayerbotAI* botAI)
    {
        return new BlackTempleMisdirectToMainTankAction(
            botAI, "supremus misdirect to main tank", BtHelpers::BtBosses::DATA_SUPREMUS);
    }
    static Action* supremus_tanks_position_boss(PlayerbotAI* botAI)
    {
        return new BlackTemplePositionBossAction(
            botAI, "supremus tanks position boss",
            BtHelpers::BtBosses::DATA_SUPREMUS, BtHelpers::SUPREMUS_INITIAL_TANK_POSITION);
    }
    static Action* supremus_spread_ranged(PlayerbotAI* botAI)
    {
        return new BlackTempleSpreadRangedAction(
            botAI, "supremus spread ranged", BtHelpers::SUPREMUS_RANGED_SPREAD_DISTANCE);
    }
    static Action* supremus_kite_boss(PlayerbotAI* botAI)
    {
        return new SupremusKiteBossAction(botAI);
    }
    static Action* supremus_move_away_from_fire(PlayerbotAI* botAI)
    {
        return new SupremusMoveAwayFromFireAction(botAI);
    }
    static Action* supremus_reach_around_fire(PlayerbotAI* botAI)
    {
        return new SupremusReachAroundFireAction(botAI);
    }

    // Shade of Akama
    static Action* shade_of_akama_attack_channelers_and_sorcerers(PlayerbotAI* botAI)
    {
        return new ShadeOfAkamaAttackChannelersAndSorcerersAction(botAI);
    }

    // Teron Gorefiend
    static Action* teron_gorefiend_misdirect_to_main_tank(PlayerbotAI* botAI)
    {
        return new BlackTempleMisdirectToMainTankAction(
            botAI, "teron gorefiend misdirect to main tank",
            BtHelpers::BtBosses::DATA_TERON_GOREFIEND);
    }
    static Action* teron_gorefiend_tanks_position_boss(PlayerbotAI* botAI)
    {
        return new BlackTemplePositionBossAction(
            botAI, "teron gorefiend tanks position boss",
            BtHelpers::BtBosses::DATA_TERON_GOREFIEND, BtHelpers::GOREFIEND_TANK_POSITION);
    }
    static Action* teron_gorefiend_position_ranged_on_balcony(PlayerbotAI* botAI)
    {
        return new TeronGorefiendPositionRangedOnBalconyAction(botAI);
    }
    static Action* teron_gorefiend_avoid_shadow_of_death(PlayerbotAI* botAI)
    {
        return new TeronGorefiendAvoidShadowOfDeathAction(botAI);
    }
    static Action* teron_gorefiend_move_to_corner_to_die(PlayerbotAI* botAI)
    {
        return new TeronGorefiendMoveToCornerToDieAction(botAI);
    }
    static Action* teron_gorefiend_destroy_shadowy_constructs(PlayerbotAI* botAI)
    {
        return new TeronGorefiendDestroyShadowyConstructsAction(botAI);
    }

    // Gurtogg Bloodboil
    static Action* gurtogg_bloodboil_misdirect_to_main_tank(PlayerbotAI* botAI)
    {
        return new BlackTempleMisdirectToMainTankAction(
            botAI, "gurtogg bloodboil misdirect to main tank",
            BtHelpers::BtBosses::DATA_GURTOGG_BLOODBOIL);
    }
    static Action* gurtogg_bloodboil_tanks_position_boss(PlayerbotAI* botAI)
    {
        constexpr bool allTanks = true;
        return new BlackTemplePositionBossAction(
            botAI, "gurtogg bloodboil tanks position boss",
            BtHelpers::BtBosses::DATA_GURTOGG_BLOODBOIL, BtHelpers::GURTOGG_TANK_POSITION,
            allTanks);
    }
    static Action* gurtogg_bloodboil_rotate_ranged_groups(PlayerbotAI* botAI)
    {
        return new GurtoggBloodboilRotateRangedGroupsAction(botAI);
    }
    static Action* gurtogg_bloodboil_melee_move_behind(PlayerbotAI* botAI)
    {
        return new GurtoggBloodboilMeleeMoveBehindAction(botAI);
    }
    static Action* gurtogg_bloodboil_ranged_move_out_of_frontal_cone(PlayerbotAI* botAI)
    {
        return new GurtoggBloodboilRangedMoveOutOfFrontalConeAction(botAI);
    }
    static Action* gurtogg_bloodboil_lead_boss_to_tank_position(PlayerbotAI* botAI)
    {
        return new BlackTemplePositionBossAction(
            botAI, "gurtogg bloodboil lead boss to tank position",
            BtHelpers::BtBosses::DATA_GURTOGG_BLOODBOIL, BtHelpers::GURTOGG_TANK_POSITION);
    }

    // Reliquary of Souls
    static Action* reliquary_of_souls_misdirect_to_main_tank(PlayerbotAI* botAI)
    {
        return new ReliquaryOfSoulsMisdirectToMainTankAction(botAI);
    }
    static Action* reliquary_of_souls_adjust_distance_from_suffering(PlayerbotAI* botAI)
    {
        return new ReliquaryOfSoulsAdjustDistanceFromSufferingAction(botAI);
    }
    static Action* reliquary_of_souls_healers_dps_suffering(PlayerbotAI* botAI)
    {
        return new ReliquaryOfSoulsHealersDpsSufferingAction(botAI);
    }
    static Action* reliquary_of_souls_spellsteal_rune_shield(PlayerbotAI* botAI)
    {
        return new ReliquaryOfSoulsSpellstealRuneShieldAction(botAI);
    }
    static Action* reliquary_of_souls_spell_reflect_deaden(PlayerbotAI* botAI)
    {
        return new BlackTempleCastSpellReflectionAction(
            botAI, "reliquary of souls spell reflect deaden");
    }

    // Mother Shahraz
    static Action* mother_shahraz_misdirect_to_main_tank(PlayerbotAI* botAI)
    {
        return new BlackTempleMisdirectToMainTankAction(
            botAI, "mother shahraz misdirect to main tank",
            BtHelpers::BtBosses::DATA_MOTHER_SHAHRAZ);
    }
    static Action* mother_shahraz_tanks_position_boss_under_pillar(PlayerbotAI* botAI)
    {
        return new MotherShahrazTanksPositionBossUnderPillarAction(botAI);
    }
    static Action* mother_shahraz_melee_dps_wait_at_safe_position(PlayerbotAI* botAI)
    {
        return new MotherShahrazMeleeDpsWaitAtSafePositionAction(botAI);
    }
    static Action* mother_shahraz_position_ranged_under_statue(PlayerbotAI* botAI)
    {
        return new MotherShahrazPositionRangedUnderStatueAction(botAI);
    }
    static Action* mother_shahraz_break_fatal_attraction(PlayerbotAI* botAI)
    {
        return new MotherShahrazBreakFatalAttractionAction(botAI);
    }

    // Illidari Council
    static Action* illidari_council_misdirect_to_tanks(PlayerbotAI* botAI)
    {
        return new IllidariCouncilMisdirectToTanksAction(botAI);
    }
    static Action* illidari_council_main_tank_position_gathios(PlayerbotAI* botAI)
    {
        return new IllidariCouncilMainTankPositionGathiosAction(botAI);
    }
    static Action* illidari_council_main_tank_reflect_judgement_of_command(PlayerbotAI* botAI)
    {
        return new BlackTempleCastSpellReflectionAction(
            botAI, "illidari council main tank reflect judgement of command");
    }
    static Action* illidari_council_interrupt_circle_of_healing(PlayerbotAI* botAI)
    {
        return new IllidariCouncilInterruptCircleOfHealingAction(botAI);
    }
    static Action* illidari_council_first_assist_tank_focus_malande(PlayerbotAI* botAI)
    {
        return new IllidariCouncilFirstAssistTankFocusMalandeAction(botAI);
    }
    static Action* illidari_council_second_assist_tank_position_veras(PlayerbotAI* botAI)
    {
        return new IllidariCouncilSecondAssistTankPositionVerasAction(botAI);
    }
    static Action* illidari_council_mage_tank_position_zerevor(PlayerbotAI* botAI)
    {
        return new IllidariCouncilMageTankPositionZerevorAction(botAI);
    }
    static Action* illidari_council_position_mage_tank_healer(PlayerbotAI* botAI)
    {
        return new IllidariCouncilPositionMageTankHealerAction(botAI);
    }
    static Action* illidari_council_spread_ranged(PlayerbotAI* botAI)
    {
        return new BlackTempleSpreadRangedAction(
            botAI, "illidari council spread ranged", BtHelpers::COUNCIL_RANGED_SPREAD_DISTANCE);
    }
    static Action* illidari_council_command_pet_target(PlayerbotAI* botAI)
    {
        return new IllidariCouncilCommandPetTargetAction(botAI);
    }
    static Action* illidari_council_assign_dps_targets(PlayerbotAI* botAI)
    {
        return new IllidariCouncilAssignDpsTargetsAction(botAI);
    }
    static Action* illidari_council_manage_dps_timer(PlayerbotAI* botAI)
    {
        return new IllidariCouncilManageDpsTimerAction(botAI);
    }
    static Action* illidari_council_announce_class_roles(PlayerbotAI* botAI)
    {
        return new IllidariCouncilAnnounceClassRolesAction(botAI);
    }

    // Illidan Stormrage <The Betrayer>
    static Action* illidan_stormrage_position_on_grate_during_speech(PlayerbotAI* botAI)
    {
        return new IllidanStormragePositionOnGrateDuringSpeechAction(botAI);
    }
    static Action* illidan_stormrage_misdirect_to_tanks(PlayerbotAI* botAI)
    {
        return new IllidanStormrageMisdirectToTanksAction(botAI);
    }
    static Action* illidan_stormrage_main_tank_reposition_boss(PlayerbotAI* botAI)
    {
        return new IllidanStormrageMainTankRepositionBossAction(botAI);
    }
    static Action* illidan_stormrage_isolate_bot_with_infection(PlayerbotAI* botAI)
    {
        return new IllidanStormrageIsolateBotWithInfectionAction(botAI);
    }
    static Action* illidan_stormrage_track_infections(PlayerbotAI* botAI)
    {
        return new IllidanStormrageTrackInfectionsAction(botAI);
    }
    static Action* illidan_stormrage_assist_tanks_handle_flames(PlayerbotAI* botAI)
    {
        return new IllidanStormrageAssistTanksHandleFlamesAction(botAI);
    }
    static Action* illidan_stormrage_control_pet_aggression(PlayerbotAI* botAI)
    {
        return new IllidanStormrageControlPetAggressionAction(botAI);
    }
    static Action* illidan_stormrage_command_pet_attack_add(PlayerbotAI* botAI)
    {
        return new IllidanStormrageCommandPetAttackAddAction(botAI);
    }
    static Action* illidan_stormrage_position_above_grate(PlayerbotAI* botAI)
    {
        return new IllidanStormragePositionAboveGrateAction(botAI);
    }
    static Action* illidan_stormrage_clear_debuff_with_immunity(PlayerbotAI* botAI)
    {
        return new IllidanStormrageClearDebuffWithImmunityAction(botAI);
    }
    static Action* illidan_stormrage_cancel_immunity(PlayerbotAI* botAI)
    {
        return new IllidanStormrageCancelImmunityAction(botAI);
    }
    static Action* illidan_stormrage_recall_totems(PlayerbotAI* botAI)
    {
        return new IllidanStormrageRecallTotemsAction(botAI);
    }
    static Action* illidan_stormrage_position_for_landing(PlayerbotAI* botAI)
    {
        return new IllidanStormragePositionForLandingAction(botAI);
    }
    static Action* illidan_stormrage_melee_stop_attacking(PlayerbotAI* botAI)
    {
        return new IllidanStormrageMeleeStopAttackingAction(botAI);
    }
    static Action* illidan_stormrage_stop_shooting(PlayerbotAI* botAI)
    {
        return new IllidanStormrageStopShootingAction(botAI);
    }
    static Action* illidan_stormrage_spread_ranged(PlayerbotAI* botAI)
    {
        return new IllidanStormrageSpreadRangedAction(botAI);
    }
    static Action* illidan_stormrage_reach_shadow_demon(PlayerbotAI* botAI)
    {
        return new IllidanStormrageReachShadowDemonAction(botAI);
    }
    static Action* illidan_stormrage_melee_stay_away_from_demon_form(PlayerbotAI* botAI)
    {
        return new IllidanStormrageMeleeStayAwayFromDemonFormAction(botAI);
    }
    static Action* illidan_stormrage_warlock_tank_handle_demon_form(PlayerbotAI* botAI)
    {
        return new IllidanStormrageWarlockTankHandleDemonFormAction(botAI);
    }
    static Action* illidan_stormrage_seed_shadow_demons(PlayerbotAI* botAI)
    {
        return new IllidanStormrageSeedShadowDemonsAction(botAI);
    }
    static Action* illidan_stormrage_assign_target_priority(PlayerbotAI* botAI)
    {
        return new IllidanStormrageAssignTargetPriorityAction(botAI);
    }
    static Action* illidan_stormrage_manage_dps_timers(PlayerbotAI* botAI)
    {
        return new IllidanStormrageManageDpsTimersAction(botAI);
    }
    static Action* illidan_stormrage_clear_hazards(PlayerbotAI* botAI)
    {
        return new IllidanStormrageClearHazardsAction(botAI);
    }
    static Action* illidan_stormrage_announce_class_roles(PlayerbotAI* botAI)
    {
        return new IllidanStormrageAnnounceClassRolesAction(botAI);
    }
};

#endif
