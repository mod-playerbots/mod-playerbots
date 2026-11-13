/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_AQ40ACTIONCONTEXT_H
#define PLAYERBOTS_AQ40ACTIONCONTEXT_H

#include "Aq40Actions.h"
#include "Aq40Helpers.h"
#include "NamedObjectContext.h"

class RaidAq40ActionContext : public NamedObjectContext<Action>
{
public:
    RaidAq40ActionContext()
    {
        // General
        creators["aq40 reset encounter states"] = &RaidAq40ActionContext::aq40_reset_encounter_states;
        creators["aq40 track encounter"] = &RaidAq40ActionContext::aq40_track_encounter;
        creators["aq40 assign add tanks"] = &RaidAq40ActionContext::aq40_assign_add_tanks;
        creators["aq40 drop to ground"] = &RaidAq40ActionContext::aq40_drop_to_ground;
        creators["aq40 tank taunt assigned target"] = &RaidAq40ActionContext::aq40_tank_taunt_assigned_target;
        creators["aq40 stop attacking without assigned target"] =
            &RaidAq40ActionContext::aq40_stop_attacking_without_assigned_target;
        creators["aq40 ranged move into boss range"] = &RaidAq40ActionContext::aq40_ranged_move_into_boss_range;

        // The Prophet Skeram
        creators["the prophet skeram assign platforms"] = &RaidAq40ActionContext::the_prophet_skeram_assign_platforms;
        creators["the prophet skeram crowd control mind controlled"] =
            &RaidAq40ActionContext::the_prophet_skeram_crowd_control_mind_controlled;
        creators["the prophet skeram move to assigned platform"] =
            &RaidAq40ActionContext::the_prophet_skeram_move_to_assigned_platform;
        creators["the prophet skeram assign dps priority"] =
            &RaidAq40ActionContext::the_prophet_skeram_assign_dps_priority;

        // Bug Trio
        creators["bug trio move away from poison cloud"] = &RaidAq40ActionContext::bug_trio_move_away_from_poison_cloud;
        creators["bug trio tanks separate bosses"] = &RaidAq40ActionContext::bug_trio_tanks_separate_bosses;
        creators["bug trio assign dps priority"] = &RaidAq40ActionContext::bug_trio_assign_dps_priority;

        // Battleguard Sartura
        creators["battleguard sartura move away from whirlwind"] =
            &RaidAq40ActionContext::battleguard_sartura_move_away_from_whirlwind;
        creators["battleguard sartura assign dps priority"] =
            &RaidAq40ActionContext::battleguard_sartura_assign_dps_priority;

        // Fankriss the Unyielding
        creators["fankriss the unyielding swap tanks"] = &RaidAq40ActionContext::fankriss_the_unyielding_swap_tanks;
        creators["fankriss the unyielding mark kill target"] =
            &RaidAq40ActionContext::fankriss_the_unyielding_mark_kill_target;
        creators["fankriss the unyielding main tank attack boss"] =
            &RaidAq40ActionContext::fankriss_the_unyielding_main_tank_attack_boss;
        creators["fankriss the unyielding tanks pick up spawns"] =
            &RaidAq40ActionContext::fankriss_the_unyielding_tanks_pick_up_spawns;

        // Viscidus
        creators["viscidus escape toxin clouds"] = &RaidAq40ActionContext::viscidus_escape_toxin_clouds;
        creators["viscidus cast frost spells"] = &RaidAq40ActionContext::viscidus_cast_frost_spells;
        creators["viscidus melee shatter"] = &RaidAq40ActionContext::viscidus_melee_shatter;
        creators["viscidus melee move in to shatter"] = &RaidAq40ActionContext::viscidus_melee_move_in_to_shatter;
        creators["viscidus position ranged on ring"] = &RaidAq40ActionContext::viscidus_position_ranged_on_ring;
        creators["viscidus assign dps priority"] = &RaidAq40ActionContext::viscidus_assign_dps_priority;

        // Princess Huhuran
        creators["princess huhuran assign poison bolt soakers"] =
            &RaidAq40ActionContext::princess_huhuran_assign_poison_bolt_soakers;
        creators["princess huhuran swap tanks"] = &RaidAq40ActionContext::princess_huhuran_swap_tanks;
        creators["princess huhuran soakers stand near boss"] =
            &RaidAq40ActionContext::princess_huhuran_soakers_stand_near_boss;
        creators["princess huhuran spread ranged and healers"] =
            &RaidAq40ActionContext::princess_huhuran_spread_ranged_and_healers;
        creators["princess huhuran tranquilizing shot"] = &RaidAq40ActionContext::princess_huhuran_tranquilizing_shot;
        creators["princess huhuran cure wyvern sting"] = &RaidAq40ActionContext::princess_huhuran_cure_wyvern_sting;
        creators["princess huhuran assign dps priority"] = &RaidAq40ActionContext::princess_huhuran_assign_dps_priority;

        // Twin Emperors
        creators["twin emperors warlock move to start spot"] =
            &RaidAq40ActionContext::twin_emperors_warlock_move_to_start_spot;
        creators["twin emperors assign tanks and sides"] = &RaidAq40ActionContext::twin_emperors_assign_tanks_and_sides;
        creators["twin emperors move away from exploding bug"] =
            &RaidAq40ActionContext::twin_emperors_move_away_from_exploding_bug;
        creators["twin emperors escape blizzard"] = &RaidAq40ActionContext::twin_emperors_escape_blizzard;
        creators["twin emperors move away from vek'lor"] = &RaidAq40ActionContext::twin_emperors_move_away_from_vek_lor;
        creators["twin emperors clear vek'nilash landing spot"] =
            &RaidAq40ActionContext::twin_emperors_clear_vek_nilash_landing_spot;
        creators["twin emperors lead emperor to platform"] =
            &RaidAq40ActionContext::twin_emperors_lead_emperor_to_platform;
        creators["twin emperors drop emperor threat"] = &RaidAq40ActionContext::twin_emperors_drop_emperor_threat;
        creators["emperor vek'nilash tank use defensive cooldown"] =
            &RaidAq40ActionContext::emperor_vek_nilash_tank_use_defensive_cooldown;
        creators["emperor vek'lor warlock voidwalker sacrifice"] =
            &RaidAq40ActionContext::emperor_vek_lor_warlock_voidwalker_sacrifice;
        creators["emperor vek'lor waiting tank taunt boss"] =
            &RaidAq40ActionContext::emperor_vek_lor_waiting_tank_taunt_boss;
        creators["twin emperors tanks position emperors"] =
            &RaidAq40ActionContext::twin_emperors_tanks_position_emperors;
        creators["emperor vek'lor warlock tank keep range"] =
            &RaidAq40ActionContext::emperor_vek_lor_warlock_tank_keep_range;
        creators["twin emperors melee leave landing spots"] =
            &RaidAq40ActionContext::twin_emperors_melee_leave_landing_spots;
        creators["twin emperors dps move to assigned emperor"] =
            &RaidAq40ActionContext::twin_emperors_dps_move_to_assigned_emperor;
        creators["twin emperors healers position between tank and warlock"] =
            &RaidAq40ActionContext::twin_emperors_healers_position_between_tank_and_warlock;
        creators["twin emperors heal emperor targets"] = &RaidAq40ActionContext::twin_emperors_heal_emperor_targets;
        creators["twin emperors warlock set pet passive"] =
            &RaidAq40ActionContext::twin_emperors_warlock_set_pet_passive;
        creators["twin emperors warlock prepare for vek'lor"] =
            &RaidAq40ActionContext::twin_emperors_warlock_prepare_for_vek_lor;
        creators["emperor vek'lor warlock tank cast searing pain"] =
            &RaidAq40ActionContext::emperor_vek_lor_warlock_tank_cast_searing_pain;
        creators["emperor vek'lor tank demoralizing shout"] =
            &RaidAq40ActionContext::emperor_vek_lor_tank_demoralizing_shout;
        creators["twin emperors assign dps priority"] = &RaidAq40ActionContext::twin_emperors_assign_dps_priority;

        // Ouro
        creators["ouro move out of frontal cone"] = &RaidAq40ActionContext::ouro_move_out_of_frontal_cone;
        creators["ouro avoid dirt mound"] = &RaidAq40ActionContext::ouro_avoid_dirt_mound;
        creators["ouro accept new main tank"] = &RaidAq40ActionContext::ouro_accept_new_main_tank;
        creators["ouro assign dps priority"] = &RaidAq40ActionContext::ouro_assign_dps_priority;

        // C'Thun
        creators["c'thun puller move to pull spot"] = &RaidAq40ActionContext::c_thun_puller_move_to_pull_spot;
        creators["c'thun trail puller"] = &RaidAq40ActionContext::c_thun_trail_puller;
        creators["eye of c'thun track dark glare"] = &RaidAq40ActionContext::eye_of_c_thun_track_dark_glare;
        creators["c'thun assign spots"] = &RaidAq40ActionContext::c_thun_assign_spots;
        creators["c'thun assign tentacle killers"] = &RaidAq40ActionContext::c_thun_assign_tentacle_killers;
        creators["c'thun assign giant claw tanks"] = &RaidAq40ActionContext::c_thun_assign_giant_claw_tanks;
        creators["c'thun send spit-out trigger"] = &RaidAq40ActionContext::c_thun_send_spit_out_trigger;
        creators["c'thun exit stomach"] = &RaidAq40ActionContext::c_thun_exit_stomach;
        creators["eye of c'thun dodge dark glare"] = &RaidAq40ActionContext::eye_of_c_thun_dodge_dark_glare;
        creators["eye of c'thun puller hold pull spot"] = &RaidAq40ActionContext::eye_of_c_thun_puller_hold_pull_spot;
        creators["eye of c'thun move to assigned spot"] = &RaidAq40ActionContext::eye_of_c_thun_move_to_assigned_spot;
        creators["c'thun move to stack"] = &RaidAq40ActionContext::c_thun_move_to_stack;
        creators["c'thun melee move into burn range"] = &RaidAq40ActionContext::c_thun_melee_move_into_burn_range;
        creators["c'thun melee hold until in burn range"] =
            &RaidAq40ActionContext::c_thun_melee_hold_until_in_burn_range;
        creators["c'thun healers damage c'thun"] = &RaidAq40ActionContext::c_thun_healers_damage_c_thun;
        creators["c'thun healers damage flesh tentacles"] =
            &RaidAq40ActionContext::c_thun_healers_damage_flesh_tentacles;
        creators["c'thun assign dps priority"] = &RaidAq40ActionContext::c_thun_assign_dps_priority;
    }

private:
    static Action* aq40_reset_encounter_states(PlayerbotAI* botAI) { return new Aq40ResetEncounterStatesAction(botAI); }
    static Action* aq40_track_encounter(PlayerbotAI* botAI) { return new Aq40TrackEncounterAction(botAI); }
    static Action* aq40_assign_add_tanks(PlayerbotAI* botAI) { return new Aq40AssignAddTanksAction(botAI); }
    static Action* aq40_drop_to_ground(PlayerbotAI* botAI) { return new Aq40DropToGroundAction(botAI); }
    static Action* aq40_tank_taunt_assigned_target(PlayerbotAI* botAI)
    {
        return new Aq40TankTauntAssignedTargetAction(botAI);
    }
    static Action* aq40_stop_attacking_without_assigned_target(PlayerbotAI* botAI)
    {
        return new Aq40StopAttackingAction(botAI, "aq40 stop attacking without assigned target");
    }
    static Action* aq40_ranged_move_into_boss_range(PlayerbotAI* botAI)
    {
        return new Aq40RangedMoveIntoBossRangeAction(botAI);
    }
    static Action* the_prophet_skeram_assign_platforms(PlayerbotAI* botAI)
    {
        return new TheProphetSkeramAssignPlatformsAction(botAI);
    }
    static Action* the_prophet_skeram_crowd_control_mind_controlled(PlayerbotAI* botAI)
    {
        return new TheProphetSkeramCrowdControlMindControlledAction(botAI);
    }
    static Action* the_prophet_skeram_move_to_assigned_platform(PlayerbotAI* botAI)
    {
        return new TheProphetSkeramMoveToAssignedPlatformAction(botAI);
    }
    static Action* the_prophet_skeram_assign_dps_priority(PlayerbotAI* botAI)
    {
        return new Aq40AttackAssignedTargetAction(botAI, "the prophet skeram assign dps priority");
    }
    static Action* bug_trio_move_away_from_poison_cloud(PlayerbotAI* botAI)
    {
        return new BugTrioMoveAwayFromPoisonCloudAction(botAI);
    }
    static Action* bug_trio_tanks_separate_bosses(PlayerbotAI* botAI)
    {
        return new BugTrioTanksSeparateBossesAction(botAI);
    }
    static Action* bug_trio_assign_dps_priority(PlayerbotAI* botAI)
    {
        return new Aq40AttackAssignedTargetAction(botAI, "bug trio assign dps priority");
    }
    static Action* battleguard_sartura_move_away_from_whirlwind(PlayerbotAI* botAI)
    {
        return new BattleguardSarturaMoveAwayFromWhirlwindAction(botAI);
    }
    static Action* battleguard_sartura_assign_dps_priority(PlayerbotAI* botAI)
    {
        return new Aq40AttackAssignedTargetAction(botAI, "battleguard sartura assign dps priority");
    }
    static Action* fankriss_the_unyielding_swap_tanks(PlayerbotAI* botAI)
    {
        return new FankrissTheUnyieldingSwapTanksAction(botAI);
    }
    static Action* fankriss_the_unyielding_mark_kill_target(PlayerbotAI* botAI)
    {
        return new FankrissTheUnyieldingMarkKillTargetAction(botAI);
    }
    static Action* fankriss_the_unyielding_main_tank_attack_boss(PlayerbotAI* botAI)
    {
        return new FankrissTheUnyieldingMainTankAttackBossAction(botAI);
    }
    static Action* fankriss_the_unyielding_tanks_pick_up_spawns(PlayerbotAI* botAI)
    {
        return new FankrissTheUnyieldingTanksPickUpSpawnsAction(botAI);
    }
    static Action* viscidus_escape_toxin_clouds(PlayerbotAI* botAI)
    {
        return new ViscidusEscapeToxinCloudsAction(botAI);
    }
    static Action* viscidus_cast_frost_spells(PlayerbotAI* botAI) { return new ViscidusCastFrostSpellsAction(botAI); }
    static Action* viscidus_melee_shatter(PlayerbotAI* botAI) { return new ViscidusMeleeShatterAction(botAI); }
    static Action* viscidus_melee_move_in_to_shatter(PlayerbotAI* botAI)
    {
        return new ViscidusMeleeMoveInToShatterAction(botAI);
    }
    static Action* viscidus_position_ranged_on_ring(PlayerbotAI* botAI)
    {
        return new ViscidusPositionRangedOnRingAction(botAI);
    }
    static Action* viscidus_assign_dps_priority(PlayerbotAI* botAI)
    {
        return new Aq40AttackAssignedTargetAction(botAI, "viscidus assign dps priority");
    }
    static Action* princess_huhuran_assign_poison_bolt_soakers(PlayerbotAI* botAI)
    {
        return new PrincessHuhuranAssignPoisonBoltSoakersAction(botAI);
    }
    static Action* princess_huhuran_swap_tanks(PlayerbotAI* botAI) { return new PrincessHuhuranSwapTanksAction(botAI); }
    static Action* princess_huhuran_soakers_stand_near_boss(PlayerbotAI* botAI)
    {
        return new PrincessHuhuranSoakersStandNearBossAction(botAI);
    }
    static Action* princess_huhuran_spread_ranged_and_healers(PlayerbotAI* botAI)
    {
        return new PrincessHuhuranSpreadRangedAndHealersAction(botAI);
    }
    static Action* princess_huhuran_tranquilizing_shot(PlayerbotAI* botAI)
    {
        return new PrincessHuhuranTranquilizingShotAction(botAI);
    }
    static Action* princess_huhuran_cure_wyvern_sting(PlayerbotAI* botAI)
    {
        return new PrincessHuhuranCureWyvernStingAction(botAI);
    }
    static Action* princess_huhuran_assign_dps_priority(PlayerbotAI* botAI)
    {
        return new Aq40AttackAssignedTargetAction(botAI, "princess huhuran assign dps priority");
    }
    static Action* twin_emperors_warlock_move_to_start_spot(PlayerbotAI* botAI)
    {
        return new TwinEmperorsWarlockMoveToStartSpotAction(botAI);
    }
    static Action* twin_emperors_assign_tanks_and_sides(PlayerbotAI* botAI)
    {
        return new TwinEmperorsAssignTanksAndSidesAction(botAI);
    }
    static Action* twin_emperors_move_away_from_exploding_bug(PlayerbotAI* botAI)
    {
        return new TwinEmperorsMoveAwayFromExplodingBugAction(botAI);
    }
    static Action* twin_emperors_escape_blizzard(PlayerbotAI* botAI)
    {
        return new TwinEmperorsEscapeBlizzardAction(botAI);
    }
    static Action* twin_emperors_move_away_from_vek_lor(PlayerbotAI* botAI)
    {
        return new TwinEmperorsMoveAwayFromVeklorAction(botAI);
    }
    static Action* twin_emperors_clear_vek_nilash_landing_spot(PlayerbotAI* botAI)
    {
        return new TwinEmperorsClearVeknilashLandingSpotAction(botAI);
    }
    static Action* twin_emperors_lead_emperor_to_platform(PlayerbotAI* botAI)
    {
        return new TwinEmperorsLeadEmperorToPlatformAction(botAI);
    }
    static Action* twin_emperors_drop_emperor_threat(PlayerbotAI* botAI)
    {
        return new TwinEmperorsDropEmperorThreatAction(botAI);
    }
    static Action* emperor_vek_nilash_tank_use_defensive_cooldown(PlayerbotAI* botAI)
    {
        return new EmperorVeknilashTankUseDefensiveCooldownAction(botAI);
    }
    static Action* emperor_vek_lor_warlock_voidwalker_sacrifice(PlayerbotAI* botAI)
    {
        return new EmperorVeklorWarlockVoidwalkerSacrificeAction(botAI);
    }
    static Action* emperor_vek_lor_waiting_tank_taunt_boss(PlayerbotAI* botAI)
    {
        return new EmperorVeklorWaitingTankTauntBossAction(botAI);
    }
    static Action* twin_emperors_tanks_position_emperors(PlayerbotAI* botAI)
    {
        return new TwinEmperorsTanksPositionEmperorsAction(botAI);
    }
    static Action* emperor_vek_lor_warlock_tank_keep_range(PlayerbotAI* botAI)
    {
        return new EmperorVeklorWarlockTankKeepRangeAction(botAI);
    }
    static Action* twin_emperors_melee_leave_landing_spots(PlayerbotAI* botAI)
    {
        return new TwinEmperorsMeleeLeaveLandingSpotsAction(botAI);
    }
    static Action* twin_emperors_dps_move_to_assigned_emperor(PlayerbotAI* botAI)
    {
        return new TwinEmperorsDpsMoveToAssignedEmperorAction(botAI);
    }
    static Action* twin_emperors_healers_position_between_tank_and_warlock(PlayerbotAI* botAI)
    {
        return new TwinEmperorsHealersPositionBetweenTankAndWarlockAction(botAI);
    }
    static Action* twin_emperors_heal_emperor_targets(PlayerbotAI* botAI)
    {
        return new TwinEmperorsHealEmperorTargetsAction(botAI);
    }
    static Action* twin_emperors_warlock_set_pet_passive(PlayerbotAI* botAI)
    {
        return new TwinEmperorsWarlockSetPetPassiveAction(botAI);
    }
    static Action* twin_emperors_warlock_prepare_for_vek_lor(PlayerbotAI* botAI)
    {
        return new TwinEmperorsWarlockPrepareForVeklorAction(botAI);
    }
    static Action* emperor_vek_lor_warlock_tank_cast_searing_pain(PlayerbotAI* botAI)
    {
        return new EmperorVeklorWarlockTankCastSearingPainAction(botAI);
    }
    static Action* emperor_vek_lor_tank_demoralizing_shout(PlayerbotAI* botAI)
    {
        return new EmperorVeklorTankDemoralizingShoutAction(botAI);
    }
    static Action* twin_emperors_assign_dps_priority(PlayerbotAI* botAI)
    {
        return new Aq40AttackAssignedTargetAction(botAI, "twin emperors assign dps priority");
    }
    static Action* ouro_move_out_of_frontal_cone(PlayerbotAI* botAI)
    {
        return new OuroMoveOutOfFrontalConeAction(botAI);
    }
    static Action* ouro_avoid_dirt_mound(PlayerbotAI* botAI) { return new OuroAvoidDirtMoundAction(botAI); }
    static Action* ouro_accept_new_main_tank(PlayerbotAI* botAI) { return new OuroAcceptNewMainTankAction(botAI); }
    static Action* ouro_assign_dps_priority(PlayerbotAI* botAI)
    {
        return new Aq40AttackAssignedTargetAction(botAI, "ouro assign dps priority");
    }
    static Action* c_thun_puller_move_to_pull_spot(PlayerbotAI* botAI)
    {
        return new CthunPullerMoveToPullSpotAction(botAI);
    }
    static Action* c_thun_trail_puller(PlayerbotAI* botAI) { return new CthunTrailPullerAction(botAI); }
    static Action* eye_of_c_thun_track_dark_glare(PlayerbotAI* botAI)
    {
        return new EyeOfCthunTrackDarkGlareAction(botAI);
    }
    static Action* c_thun_assign_spots(PlayerbotAI* botAI) { return new CthunAssignSpotsAction(botAI); }
    static Action* c_thun_assign_tentacle_killers(PlayerbotAI* botAI)
    {
        return new CthunAssignTentacleKillersAction(botAI);
    }
    static Action* c_thun_assign_giant_claw_tanks(PlayerbotAI* botAI)
    {
        return new CthunAssignGiantClawTanksAction(botAI);
    }
    static Action* c_thun_send_spit_out_trigger(PlayerbotAI* botAI) { return new CthunSendSpitOutTriggerAction(botAI); }
    static Action* c_thun_exit_stomach(PlayerbotAI* botAI) { return new CthunExitStomachAction(botAI); }
    static Action* eye_of_c_thun_dodge_dark_glare(PlayerbotAI* botAI)
    {
        return new EyeOfCthunDodgeDarkGlareAction(botAI);
    }
    static Action* eye_of_c_thun_puller_hold_pull_spot(PlayerbotAI* botAI)
    {
        return new EyeOfCthunPullerHoldPullSpotAction(botAI);
    }
    static Action* eye_of_c_thun_move_to_assigned_spot(PlayerbotAI* botAI)
    {
        return new EyeOfCthunMoveToAssignedSpotAction(botAI);
    }
    static Action* c_thun_move_to_stack(PlayerbotAI* botAI) { return new CthunMoveToStackAction(botAI); }
    static Action* c_thun_melee_move_into_burn_range(PlayerbotAI* botAI)
    {
        return new CthunMeleeMoveIntoBurnRangeAction(botAI);
    }
    static Action* c_thun_melee_hold_until_in_burn_range(PlayerbotAI* botAI)
    {
        return new Aq40StopAttackingAction(botAI, "c'thun melee hold until in burn range");
    }
    static Action* c_thun_healers_damage_c_thun(PlayerbotAI* botAI) { return new CthunHealersDamageCthunAction(botAI); }
    static Action* c_thun_healers_damage_flesh_tentacles(PlayerbotAI* botAI)
    {
        return new CthunHealersDamageFleshTentaclesAction(botAI);
    }
    static Action* c_thun_assign_dps_priority(PlayerbotAI* botAI)
    {
        return new Aq40AttackAssignedTargetAction(botAI, "c'thun assign dps priority");
    }
};

#endif
