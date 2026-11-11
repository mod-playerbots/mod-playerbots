/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_AQ40TRIGGERCONTEXT_H
#define PLAYERBOTS_AQ40TRIGGERCONTEXT_H

#include "Aq40Helpers.h"
#include "Aq40Triggers.h"
#include "NamedObjectContext.h"

class RaidAq40TriggerContext : public NamedObjectContext<Trigger>
{
public:
    RaidAq40TriggerContext()
    {
        // General
        creators["aq40 no encounter in progress"] = &RaidAq40TriggerContext::aq40_no_encounter_in_progress;
        creators["aq40 should track encounter"] = &RaidAq40TriggerContext::aq40_should_track_encounter;
        creators["aq40 should assign add tanks"] = &RaidAq40TriggerContext::aq40_should_assign_add_tanks;
        creators["aq40 stuck midair after knockback"] = &RaidAq40TriggerContext::aq40_stuck_midair_after_knockback;
        creators["aq40 tank should taunt assigned target"] =
            &RaidAq40TriggerContext::aq40_tank_should_taunt_assigned_target;
        creators["aq40 no assigned target"] = &RaidAq40TriggerContext::aq40_no_assigned_target;
        creators["aq40 ranged should hold boss range"] = &RaidAq40TriggerContext::aq40_ranged_should_hold_boss_range;

        // The Prophet Skeram
        creators["the prophet skeram should assign platforms"] =
            &RaidAq40TriggerContext::the_prophet_skeram_should_assign_platforms;
        creators["the prophet skeram raid member mind controlled"] =
            &RaidAq40TriggerContext::the_prophet_skeram_raid_member_mind_controlled;
        creators["the prophet skeram should move to platform"] =
            &RaidAq40TriggerContext::the_prophet_skeram_should_move_to_platform;
        creators["the prophet skeram should assign dps priority"] =
            &RaidAq40TriggerContext::the_prophet_skeram_should_assign_dps_priority;

        // Bug Trio
        creators["bug trio poison cloud nearby"] = &RaidAq40TriggerContext::bug_trio_poison_cloud_nearby;
        creators["bug trio bosses too close"] = &RaidAq40TriggerContext::bug_trio_bosses_too_close;
        creators["bug trio should assign dps priority"] = &RaidAq40TriggerContext::bug_trio_should_assign_dps_priority;

        // Battleguard Sartura
        creators["battleguard sartura whirlwind nearby"] =
            &RaidAq40TriggerContext::battleguard_sartura_whirlwind_nearby;
        creators["battleguard sartura should assign dps priority"] =
            &RaidAq40TriggerContext::battleguard_sartura_should_assign_dps_priority;

        // Fankriss the Unyielding
        creators["fankriss the unyielding should swap tanks"] =
            &RaidAq40TriggerContext::fankriss_the_unyielding_should_swap_tanks;
        creators["fankriss the unyielding should assign dps priority"] =
            &RaidAq40TriggerContext::fankriss_the_unyielding_should_assign_dps_priority;

        // Viscidus
        creators["viscidus toxin cloud nearby"] = &RaidAq40TriggerContext::viscidus_toxin_cloud_nearby;
        creators["viscidus should be frozen"] = &RaidAq40TriggerContext::viscidus_should_be_frozen;
        creators["viscidus is frozen"] = &RaidAq40TriggerContext::viscidus_is_frozen;
        creators["viscidus frozen out of melee range"] = &RaidAq40TriggerContext::viscidus_frozen_out_of_melee_range;
        creators["viscidus ranged should hold ring"] = &RaidAq40TriggerContext::viscidus_ranged_should_hold_ring;
        creators["viscidus should assign dps priority"] = &RaidAq40TriggerContext::viscidus_should_assign_dps_priority;

        // Princess Huhuran
        creators["princess huhuran should assign soakers"] =
            &RaidAq40TriggerContext::princess_huhuran_should_assign_soakers;
        creators["princess huhuran should swap tanks"] = &RaidAq40TriggerContext::princess_huhuran_should_swap_tanks;
        creators["princess huhuran soaker too far"] = &RaidAq40TriggerContext::princess_huhuran_soaker_too_far;
        creators["princess huhuran ranged should spread"] =
            &RaidAq40TriggerContext::princess_huhuran_ranged_should_spread;
        creators["princess huhuran frenzied"] = &RaidAq40TriggerContext::princess_huhuran_frenzied;
        creators["princess huhuran wyvern sting on tank"] =
            &RaidAq40TriggerContext::princess_huhuran_wyvern_sting_on_tank;
        creators["princess huhuran should assign dps priority"] =
            &RaidAq40TriggerContext::princess_huhuran_should_assign_dps_priority;

        // Twin Emperors
        creators["twin emperors warlock should take start spot"] =
            &RaidAq40TriggerContext::twin_emperors_warlock_should_take_start_spot;
        creators["twin emperors should assign roles"] = &RaidAq40TriggerContext::twin_emperors_should_assign_roles;
        creators["twin emperors exploding bug nearby"] = &RaidAq40TriggerContext::twin_emperors_exploding_bug_nearby;
        creators["twin emperors in blizzard"] = &RaidAq40TriggerContext::twin_emperors_in_blizzard;
        creators["emperor vek'lor arcane burst range"] = &RaidAq40TriggerContext::emperor_vek_lor_arcane_burst_range;
        creators["emperor vek'nilash choosing target after teleport"] =
            &RaidAq40TriggerContext::emperor_vek_nilash_choosing_target_after_teleport;
        creators["twin emperors emperor cannot reach target"] =
            &RaidAq40TriggerContext::twin_emperors_emperor_cannot_reach_target;
        creators["twin emperors non-tank has emperor aggro"] =
            &RaidAq40TriggerContext::twin_emperors_non_tank_has_emperor_aggro;
        creators["emperor vek'nilash hitting tank"] = &RaidAq40TriggerContext::emperor_vek_nilash_hitting_tank;
        creators["emperor vek'lor warlock tank low health"] =
            &RaidAq40TriggerContext::emperor_vek_lor_warlock_tank_low_health;
        creators["emperor vek'lor held by wrong player"] =
            &RaidAq40TriggerContext::emperor_vek_lor_held_by_wrong_player;
        creators["twin emperors tanks should hold platforms"] =
            &RaidAq40TriggerContext::twin_emperors_tanks_should_hold_platforms;
        creators["emperor vek'lor warlock tank out of range"] =
            &RaidAq40TriggerContext::emperor_vek_lor_warlock_tank_out_of_range;
        creators["twin emperors teleport imminent"] = &RaidAq40TriggerContext::twin_emperors_teleport_imminent;
        creators["twin emperors dps should position"] = &RaidAq40TriggerContext::twin_emperors_dps_should_position;
        creators["twin emperors healers should position"] =
            &RaidAq40TriggerContext::twin_emperors_healers_should_position;
        creators["twin emperors healer should focus emperor targets"] =
            &RaidAq40TriggerContext::twin_emperors_healer_should_focus_emperor_targets;
        creators["twin emperors warlock pet active"] = &RaidAq40TriggerContext::twin_emperors_warlock_pet_active;
        creators["twin emperors warlock waiting"] = &RaidAq40TriggerContext::twin_emperors_warlock_waiting;
        creators["emperor vek'lor should be tanked by warlock"] =
            &RaidAq40TriggerContext::emperor_vek_lor_should_be_tanked_by_warlock;
        creators["emperor vek'lor tank in shout range"] = &RaidAq40TriggerContext::emperor_vek_lor_tank_in_shout_range;
        creators["twin emperors should assign dps priority"] =
            &RaidAq40TriggerContext::twin_emperors_should_assign_dps_priority;

        // Ouro
        creators["ouro frontal cone"] = &RaidAq40TriggerContext::ouro_frontal_cone;
        creators["ouro dirt mound nearby"] = &RaidAq40TriggerContext::ouro_dirt_mound_nearby;
        creators["ouro should accept new main tank"] = &RaidAq40TriggerContext::ouro_should_accept_new_main_tank;
        creators["ouro should assign dps priority"] = &RaidAq40TriggerContext::ouro_should_assign_dps_priority;

        // C'Thun
        creators["c'thun puller should walk in"] = &RaidAq40TriggerContext::c_thun_puller_should_walk_in;
        creators["c'thun raid should trail puller"] = &RaidAq40TriggerContext::c_thun_raid_should_trail_puller;
        creators["eye of c'thun should track dark glare"] =
            &RaidAq40TriggerContext::eye_of_c_thun_should_track_dark_glare;
        creators["c'thun should assign spots"] = &RaidAq40TriggerContext::c_thun_should_assign_spots;
        creators["c'thun should assign tentacle killers"] =
            &RaidAq40TriggerContext::c_thun_should_assign_tentacle_killers;
        creators["c'thun should assign giant claw tanks"] =
            &RaidAq40TriggerContext::c_thun_should_assign_giant_claw_tanks;
        creators["c'thun on spit-out circle"] = &RaidAq40TriggerContext::c_thun_on_spit_out_circle;
        creators["c'thun should leave stomach"] = &RaidAq40TriggerContext::c_thun_should_leave_stomach;
        creators["eye of c'thun dark glare incoming"] = &RaidAq40TriggerContext::eye_of_c_thun_dark_glare_incoming;
        creators["eye of c'thun puller holding"] = &RaidAq40TriggerContext::eye_of_c_thun_puller_holding;
        creators["eye of c'thun out of assigned spot"] = &RaidAq40TriggerContext::eye_of_c_thun_out_of_assigned_spot;
        creators["c'thun out of stack"] = &RaidAq40TriggerContext::c_thun_out_of_stack;
        creators["c'thun melee should close in for burn"] =
            &RaidAq40TriggerContext::c_thun_melee_should_close_in_for_burn;
        creators["c'thun melee out of burn range"] = &RaidAq40TriggerContext::c_thun_melee_out_of_burn_range;
        creators["c'thun healers should damage c'thun"] = &RaidAq40TriggerContext::c_thun_healers_should_damage_c_thun;
        creators["c'thun healers should damage flesh tentacles"] =
            &RaidAq40TriggerContext::c_thun_healers_should_damage_flesh_tentacles;
        creators["c'thun should assign dps priority"] = &RaidAq40TriggerContext::c_thun_should_assign_dps_priority;
    }

private:
    static Trigger* aq40_no_encounter_in_progress(PlayerbotAI* botAI)
    {
        return new Aq40NoEncounterInProgressTrigger(botAI);
    }
    static Trigger* aq40_should_track_encounter(PlayerbotAI* botAI)
    {
        return new Aq40TrackerTrigger(botAI, "aq40 should track encounter", Aq40Helpers::Aq40Encounter::None);
    }
    static Trigger* aq40_should_assign_add_tanks(PlayerbotAI* botAI)
    {
        return new Aq40TrackerTrigger(botAI, "aq40 should assign add tanks", Aq40Helpers::Aq40Encounter::None);
    }
    static Trigger* aq40_stuck_midair_after_knockback(PlayerbotAI* botAI) { return new Aq40StuckMidairTrigger(botAI); }
    static Trigger* aq40_tank_should_taunt_assigned_target(PlayerbotAI* botAI)
    {
        return new Aq40TankShouldTauntTrigger(botAI);
    }
    static Trigger* aq40_no_assigned_target(PlayerbotAI* botAI) { return new Aq40NoAssignedTargetTrigger(botAI); }
    static Trigger* aq40_ranged_should_hold_boss_range(PlayerbotAI* botAI)
    {
        return new Aq40FormationTrigger(botAI, "aq40 ranged should hold boss range", &Aq40Helpers::GetBossRangeGoal,
                                        Aq40Helpers::POSITION_INTERVAL_MS);
    }
    static Trigger* the_prophet_skeram_should_assign_platforms(PlayerbotAI* botAI)
    {
        return new Aq40TrackerTrigger(botAI, "the prophet skeram should assign platforms",
                                      Aq40Helpers::Aq40Encounter::Skeram);
    }
    static Trigger* the_prophet_skeram_raid_member_mind_controlled(PlayerbotAI* botAI)
    {
        return new TheProphetSkeramRaidMemberMindControlledTrigger(botAI);
    }
    static Trigger* the_prophet_skeram_should_move_to_platform(PlayerbotAI* botAI)
    {
        return new Aq40FormationTrigger(botAI, "the prophet skeram should move to platform",
                                        &Aq40Helpers::GetSkeramPlatformGoal);
    }
    static Trigger* the_prophet_skeram_should_assign_dps_priority(PlayerbotAI* botAI)
    {
        return new Aq40AssignedTargetTrigger(botAI, "the prophet skeram should assign dps priority",
                                             Aq40Helpers::Aq40Encounter::Skeram);
    }
    static Trigger* bug_trio_poison_cloud_nearby(PlayerbotAI* botAI)
    {
        return new Aq40HazardTrigger(botAI, "bug trio poison cloud nearby", &Aq40Helpers::GetPoisonCloudEscapeGoal);
    }
    static Trigger* bug_trio_bosses_too_close(PlayerbotAI* botAI)
    {
        return new Aq40FormationTrigger(botAI, "bug trio bosses too close", &Aq40Helpers::GetTrioTankGoal,
                                        Aq40Helpers::POSITION_INTERVAL_MS);
    }
    static Trigger* bug_trio_should_assign_dps_priority(PlayerbotAI* botAI)
    {
        return new Aq40AssignedTargetTrigger(botAI, "bug trio should assign dps priority",
                                             Aq40Helpers::Aq40Encounter::Trio);
    }
    static Trigger* battleguard_sartura_whirlwind_nearby(PlayerbotAI* botAI)
    {
        return new BattleguardSarturaWhirlwindNearbyTrigger(botAI);
    }
    static Trigger* battleguard_sartura_should_assign_dps_priority(PlayerbotAI* botAI)
    {
        return new Aq40AssignedTargetTrigger(botAI, "battleguard sartura should assign dps priority",
                                             Aq40Helpers::Aq40Encounter::Sartura);
    }
    static Trigger* fankriss_the_unyielding_should_swap_tanks(PlayerbotAI* botAI)
    {
        return new Aq40TrackerTrigger(botAI, "fankriss the unyielding should swap tanks",
                                      Aq40Helpers::Aq40Encounter::Fankriss);
    }
    static Trigger* fankriss_the_unyielding_should_assign_dps_priority(PlayerbotAI* botAI)
    {
        return new Aq40AssignedTargetTrigger(botAI, "fankriss the unyielding should assign dps priority",
                                             Aq40Helpers::Aq40Encounter::Fankriss);
    }
    static Trigger* viscidus_toxin_cloud_nearby(PlayerbotAI* botAI)
    {
        return new Aq40HazardTrigger(botAI, "viscidus toxin cloud nearby", &Aq40Helpers::GetToxinEscapeGoal);
    }
    static Trigger* viscidus_should_be_frozen(PlayerbotAI* botAI) { return new ViscidusShouldBeFrozenTrigger(botAI); }
    static Trigger* viscidus_is_frozen(PlayerbotAI* botAI) { return new ViscidusIsFrozenTrigger(botAI); }
    static Trigger* viscidus_frozen_out_of_melee_range(PlayerbotAI* botAI)
    {
        return new Aq40FormationTrigger(botAI, "viscidus frozen out of melee range",
                                        &Aq40Helpers::GetViscidusShatterGoal);
    }
    static Trigger* viscidus_ranged_should_hold_ring(PlayerbotAI* botAI)
    {
        return new Aq40FormationTrigger(botAI, "viscidus ranged should hold ring", &Aq40Helpers::GetViscidusRingGoal,
                                        Aq40Helpers::POSITION_INTERVAL_MS);
    }
    static Trigger* viscidus_should_assign_dps_priority(PlayerbotAI* botAI)
    {
        return new Aq40AssignedTargetTrigger(botAI, "viscidus should assign dps priority",
                                             Aq40Helpers::Aq40Encounter::Viscidus);
    }
    static Trigger* princess_huhuran_should_assign_soakers(PlayerbotAI* botAI)
    {
        return new Aq40TrackerTrigger(botAI, "princess huhuran should assign soakers",
                                      Aq40Helpers::Aq40Encounter::Huhuran);
    }
    static Trigger* princess_huhuran_should_swap_tanks(PlayerbotAI* botAI)
    {
        return new Aq40TrackerTrigger(botAI, "princess huhuran should swap tanks", Aq40Helpers::Aq40Encounter::Huhuran);
    }
    static Trigger* princess_huhuran_soaker_too_far(PlayerbotAI* botAI)
    {
        return new Aq40FormationTrigger(botAI, "princess huhuran soaker too far", &Aq40Helpers::GetHuhuranSoakGoal,
                                        Aq40Helpers::POSITION_INTERVAL_MS);
    }
    static Trigger* princess_huhuran_ranged_should_spread(PlayerbotAI* botAI)
    {
        return new Aq40FormationTrigger(botAI, "princess huhuran ranged should spread",
                                        &Aq40Helpers::GetHuhuranSpreadGoal, Aq40Helpers::POSITION_INTERVAL_MS);
    }
    static Trigger* princess_huhuran_frenzied(PlayerbotAI* botAI) { return new PrincessHuhuranFrenziedTrigger(botAI); }
    static Trigger* princess_huhuran_wyvern_sting_on_tank(PlayerbotAI* botAI)
    {
        return new PrincessHuhuranWyvernStingOnTankTrigger(botAI);
    }
    static Trigger* princess_huhuran_should_assign_dps_priority(PlayerbotAI* botAI)
    {
        return new Aq40AssignedTargetTrigger(botAI, "princess huhuran should assign dps priority",
                                             Aq40Helpers::Aq40Encounter::Huhuran);
    }
    static Trigger* twin_emperors_warlock_should_take_start_spot(PlayerbotAI* botAI)
    {
        return new Aq40PrepullTrigger(botAI, "twin emperors warlock should take start spot",
                                      &Aq40Helpers::GetTwinPrepullSpot);
    }
    static Trigger* twin_emperors_should_assign_roles(PlayerbotAI* botAI)
    {
        return new Aq40TrackerTrigger(botAI, "twin emperors should assign roles", Aq40Helpers::Aq40Encounter::Twins);
    }
    static Trigger* twin_emperors_exploding_bug_nearby(PlayerbotAI* botAI)
    {
        return new Aq40HazardTrigger(botAI, "twin emperors exploding bug nearby",
                                     &Aq40Helpers::GetExplodingBugEscapeGoal);
    }
    static Trigger* twin_emperors_in_blizzard(PlayerbotAI* botAI)
    {
        return new Aq40HazardTrigger(botAI, "twin emperors in blizzard", &Aq40Helpers::GetBlizzardEscapeGoal);
    }
    static Trigger* emperor_vek_lor_arcane_burst_range(PlayerbotAI* botAI)
    {
        return new Aq40HazardTrigger(botAI, "emperor vek'lor arcane burst range", &Aq40Helpers::GetVeklorEscapeGoal);
    }
    static Trigger* emperor_vek_nilash_choosing_target_after_teleport(PlayerbotAI* botAI)
    {
        return new Aq40HazardTrigger(botAI, "emperor vek'nilash choosing target after teleport",
                                     &Aq40Helpers::GetVeknilashLandingEscapeGoal);
    }
    static Trigger* twin_emperors_emperor_cannot_reach_target(PlayerbotAI* botAI)
    {
        return new TwinEmperorsEmperorCannotReachTargetTrigger(botAI);
    }
    static Trigger* twin_emperors_non_tank_has_emperor_aggro(PlayerbotAI* botAI)
    {
        return new TwinEmperorsNonTankHasEmperorAggroTrigger(botAI);
    }
    static Trigger* emperor_vek_nilash_hitting_tank(PlayerbotAI* botAI)
    {
        return new EmperorVeknilashHittingTankTrigger(botAI);
    }
    static Trigger* emperor_vek_lor_warlock_tank_low_health(PlayerbotAI* botAI)
    {
        return new EmperorVeklorWarlockTankLowHealthTrigger(botAI);
    }
    static Trigger* emperor_vek_lor_held_by_wrong_player(PlayerbotAI* botAI)
    {
        return new EmperorVeklorHeldByWrongPlayerTrigger(botAI);
    }
    static Trigger* twin_emperors_tanks_should_hold_platforms(PlayerbotAI* botAI)
    {
        return new Aq40FormationTrigger(botAI, "twin emperors tanks should hold platforms",
                                        &Aq40Helpers::GetTwinTankGoal);
    }
    static Trigger* emperor_vek_lor_warlock_tank_out_of_range(PlayerbotAI* botAI)
    {
        return new Aq40FormationTrigger(botAI, "emperor vek'lor warlock tank out of range",
                                        &Aq40Helpers::GetTwinWarlockGoal);
    }
    static Trigger* twin_emperors_teleport_imminent(PlayerbotAI* botAI)
    {
        return new Aq40FormationTrigger(botAI, "twin emperors teleport imminent", &Aq40Helpers::GetTwinMeleeSwapGoal);
    }
    static Trigger* twin_emperors_dps_should_position(PlayerbotAI* botAI)
    {
        return new Aq40FormationTrigger(botAI, "twin emperors dps should position", &Aq40Helpers::GetTwinDpsGoal);
    }
    static Trigger* twin_emperors_healers_should_position(PlayerbotAI* botAI)
    {
        return new Aq40FormationTrigger(botAI, "twin emperors healers should position",
                                        &Aq40Helpers::GetTwinHealerGoal);
    }
    static Trigger* twin_emperors_healer_should_focus_emperor_targets(PlayerbotAI* botAI)
    {
        return new TwinEmperorsHealerShouldFocusEmperorTargetsTrigger(botAI);
    }
    static Trigger* twin_emperors_warlock_pet_active(PlayerbotAI* botAI)
    {
        return new TwinEmperorsWarlockPetActiveTrigger(botAI);
    }
    static Trigger* twin_emperors_warlock_waiting(PlayerbotAI* botAI)
    {
        return new TwinEmperorsWarlockWaitingTrigger(botAI);
    }
    static Trigger* emperor_vek_lor_should_be_tanked_by_warlock(PlayerbotAI* botAI)
    {
        return new EmperorVeklorShouldBeTankedByWarlockTrigger(botAI);
    }
    static Trigger* emperor_vek_lor_tank_in_shout_range(PlayerbotAI* botAI)
    {
        return new EmperorVeklorTankInShoutRangeTrigger(botAI);
    }
    static Trigger* twin_emperors_should_assign_dps_priority(PlayerbotAI* botAI)
    {
        return new Aq40AssignedTargetTrigger(botAI, "twin emperors should assign dps priority",
                                             Aq40Helpers::Aq40Encounter::Twins);
    }
    static Trigger* ouro_frontal_cone(PlayerbotAI* botAI)
    {
        return new Aq40HazardTrigger(botAI, "ouro frontal cone", &Aq40Helpers::GetOuroConeEscapeGoal);
    }
    static Trigger* ouro_dirt_mound_nearby(PlayerbotAI* botAI)
    {
        return new Aq40HazardTrigger(botAI, "ouro dirt mound nearby", &Aq40Helpers::GetDirtMoundEscapeGoal);
    }
    static Trigger* ouro_should_accept_new_main_tank(PlayerbotAI* botAI)
    {
        return new Aq40TrackerTrigger(botAI, "ouro should accept new main tank", Aq40Helpers::Aq40Encounter::Ouro);
    }
    static Trigger* ouro_should_assign_dps_priority(PlayerbotAI* botAI)
    {
        return new Aq40AssignedTargetTrigger(botAI, "ouro should assign dps priority",
                                             Aq40Helpers::Aq40Encounter::Ouro);
    }
    static Trigger* c_thun_puller_should_walk_in(PlayerbotAI* botAI)
    {
        return new Aq40PrepullTrigger(botAI, "c'thun puller should walk in", &Aq40Helpers::GetCthunPullerPrepullSpot);
    }
    static Trigger* c_thun_raid_should_trail_puller(PlayerbotAI* botAI)
    {
        return new Aq40PrepullTrigger(botAI, "c'thun raid should trail puller", &Aq40Helpers::GetCthunTrailSpot);
    }
    static Trigger* eye_of_c_thun_should_track_dark_glare(PlayerbotAI* botAI)
    {
        return new Aq40TrackerTrigger(botAI, "eye of c'thun should track dark glare",
                                      Aq40Helpers::Aq40Encounter::Cthun);
    }
    static Trigger* c_thun_should_assign_spots(PlayerbotAI* botAI)
    {
        return new Aq40TrackerTrigger(botAI, "c'thun should assign spots", Aq40Helpers::Aq40Encounter::Cthun);
    }
    static Trigger* c_thun_should_assign_tentacle_killers(PlayerbotAI* botAI)
    {
        return new Aq40TrackerTrigger(botAI, "c'thun should assign tentacle killers",
                                      Aq40Helpers::Aq40Encounter::Cthun);
    }
    static Trigger* c_thun_should_assign_giant_claw_tanks(PlayerbotAI* botAI)
    {
        return new Aq40TrackerTrigger(botAI, "c'thun should assign giant claw tanks",
                                      Aq40Helpers::Aq40Encounter::Cthun);
    }
    static Trigger* c_thun_on_spit_out_circle(PlayerbotAI* botAI) { return new CthunOnSpitOutCircleTrigger(botAI); }
    static Trigger* c_thun_should_leave_stomach(PlayerbotAI* botAI)
    {
        return new Aq40HazardTrigger(botAI, "c'thun should leave stomach", &Aq40Helpers::GetStomachExitGoal);
    }
    static Trigger* eye_of_c_thun_dark_glare_incoming(PlayerbotAI* botAI)
    {
        return new Aq40HazardTrigger(botAI, "eye of c'thun dark glare incoming", &Aq40Helpers::GetDarkGlareEscapeGoal);
    }
    static Trigger* eye_of_c_thun_puller_holding(PlayerbotAI* botAI)
    {
        return new Aq40FormationTrigger(botAI, "eye of c'thun puller holding", &Aq40Helpers::GetCthunPullerHoldGoal);
    }
    static Trigger* eye_of_c_thun_out_of_assigned_spot(PlayerbotAI* botAI)
    {
        return new Aq40FormationTrigger(botAI, "eye of c'thun out of assigned spot", &Aq40Helpers::GetCthunSpotGoal);
    }
    static Trigger* c_thun_out_of_stack(PlayerbotAI* botAI)
    {
        return new Aq40FormationTrigger(botAI, "c'thun out of stack", &Aq40Helpers::GetCthunStackGoal);
    }
    static Trigger* c_thun_melee_should_close_in_for_burn(PlayerbotAI* botAI)
    {
        return new Aq40FormationTrigger(botAI, "c'thun melee should close in for burn",
                                        &Aq40Helpers::GetCthunMeleeBurnGoal);
    }
    static Trigger* c_thun_melee_out_of_burn_range(PlayerbotAI* botAI)
    {
        return new CthunMeleeOutOfBurnRangeTrigger(botAI);
    }
    static Trigger* c_thun_healers_should_damage_c_thun(PlayerbotAI* botAI)
    {
        return new CthunHealersShouldDamageCthunTrigger(botAI);
    }
    static Trigger* c_thun_healers_should_damage_flesh_tentacles(PlayerbotAI* botAI)
    {
        return new CthunHealersShouldDamageFleshTentaclesTrigger(botAI);
    }
    static Trigger* c_thun_should_assign_dps_priority(PlayerbotAI* botAI)
    {
        return new Aq40AssignedTargetTrigger(botAI, "c'thun should assign dps priority",
                                             Aq40Helpers::Aq40Encounter::Cthun);
    }
};

#endif
