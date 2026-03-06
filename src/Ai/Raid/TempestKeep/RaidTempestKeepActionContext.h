#ifndef _PLAYERBOT_RAIDTEMPESTKEEPACTIONCONTEXT_H
#define _PLAYERBOT_RAIDTEMPESTKEEPACTIONCONTEXT_H

#include "RaidTempestKeepActions.h"
#include "NamedObjectContext.h"

class RaidTempestKeepActionContext : public NamedObjectContext<Action>
{
public:
    RaidTempestKeepActionContext()
    {
        // Trash
        creators["crimson hand centurion cast polymorph"] =
            &RaidTempestKeepActionContext::crimson_hand_centurion_cast_polymorph;

        // Al'ar <Phoenix God>
        creators["al'ar misdirect boss to main tank"] =
            &RaidTempestKeepActionContext::alar_misdirect_boss_to_main_tank;

        creators["al'ar boss tanks move between platforms"] =
            &RaidTempestKeepActionContext::alar_boss_tanks_move_between_platforms;

        creators["al'ar melee dps move between platforms"] =
            &RaidTempestKeepActionContext::alar_melee_dps_move_between_platforms;

        creators["al'ar ranged and ember tank move under platforms"] =
            &RaidTempestKeepActionContext::alar_ranged_and_ember_tank_move_under_platforms;

        creators["al'ar assist tanks pick up embers"] =
            &RaidTempestKeepActionContext::alar_assist_tanks_pick_up_embers;

        creators["al'ar ranged dps prioritize embers"] =
            &RaidTempestKeepActionContext::alar_ranged_dps_prioritize_embers;

        creators["al'ar jump from platform"] =
            &RaidTempestKeepActionContext::alar_jump_from_platform;

        creators["al'ar move away from rebirth"] =
            &RaidTempestKeepActionContext::alar_move_away_from_rebirth;

        creators["al'ar swap tanks on boss"] =
            &RaidTempestKeepActionContext::alar_swap_tanks_on_boss;

        creators["al'ar avoid flame patches and dive bombs"] =
            &RaidTempestKeepActionContext::alar_avoid_flame_patches_and_dive_bombs;

        creators["al'ar return to room center"] =
            &RaidTempestKeepActionContext::alar_return_to_room_center;

        creators["al'ar manage phase tracker"] =
            &RaidTempestKeepActionContext::alar_manage_phase_tracker;

        // Void Reaver
        creators["void reaver tanks position boss"] =
            &RaidTempestKeepActionContext::void_reaver_tanks_position_boss;

        creators["void reaver use aggro dump ability"] =
            &RaidTempestKeepActionContext::void_reaver_use_aggro_dump_ability;

        creators["void reaver spread ranged"] =
            &RaidTempestKeepActionContext::void_reaver_spread_ranged;

        creators["void reaver avoid arcane orb"] =
            &RaidTempestKeepActionContext::void_reaver_avoid_arcane_orb;

        creators["void reaver erase trackers"] =
            &RaidTempestKeepActionContext::void_reaver_erase_trackers;

        // High Astromancer Solarian
        creators["high astromancer solarian ranged leave space for melee"] =
            &RaidTempestKeepActionContext::high_astromancer_solarian_ranged_leave_space_for_melee;

        creators["high astromancer solarian move away from group"] =
            &RaidTempestKeepActionContext::high_astromancer_solarian_move_away_from_group;

        creators["high astromancer solarian stack for aoe"] =
            &RaidTempestKeepActionContext::high_astromancer_solarian_stack_for_aoe;

        creators["high astromancer solarian target solarium priests"] =
            &RaidTempestKeepActionContext::high_astromancer_solarian_target_solarium_priests;

        creators["high astromancer solarian cast fear ward on main tank"] =
            &RaidTempestKeepActionContext::high_astromancer_solarian_cast_fear_ward_on_main_tank;

        // Kael'thas Sunstrider <Lord of the Blood Elves>
        creators["kael'thas sunstrider kite thaladred"] =
            &RaidTempestKeepActionContext::kaelthas_sunstrider_kite_thaladred;

        creators["kael'thas sunstrider misdirect advisors to tanks"] =
            &RaidTempestKeepActionContext::kaelthas_sunstrider_misdirect_advisors_to_tanks;

        creators["kael'thas sunstrider main tank position sanguinar"] =
            &RaidTempestKeepActionContext::kaelthas_sunstrider_main_tank_position_sanguinar;

        creators["kael'thas sunstrider cast fear ward on sanguinar tank"] =
            &RaidTempestKeepActionContext::kaelthas_sunstrider_cast_fear_ward_on_sanguinar_tank;

        creators["kael'thas sunstrider warlock tank position capernian"] =
            &RaidTempestKeepActionContext::kaelthas_sunstrider_warlock_tank_position_capernian;

        creators["kael'thas sunstrider spread and move away from capernian"] =
            &RaidTempestKeepActionContext::kaelthas_sunstrider_spread_and_move_away_from_capernian;

        creators["kael'thas sunstrider first assist tank position telonicus"] =
            &RaidTempestKeepActionContext::kaelthas_sunstrider_first_assist_tank_position_telonicus;

        creators["kael'thas sunstrider handle advisor roles in phase 3"] =
            &RaidTempestKeepActionContext::kaelthas_sunstrider_handle_advisor_roles_in_phase_3;

        creators["kael'thas sunstrider assign advisor dps priority"] =
            &RaidTempestKeepActionContext::kaelthas_sunstrider_assign_advisor_dps_priority;

        creators["kael'thas sunstrider manage advisor dps timer"] =
            &RaidTempestKeepActionContext::kaelthas_sunstrider_manage_advisor_dps_timer;

        creators["kael'thas sunstrider assign legendary weapon dps priority"] =
            &RaidTempestKeepActionContext::kaelthas_sunstrider_assign_legendary_weapon_dps_priority;

        creators["kael'thas sunstrider main tank move devastation away"] =
            &RaidTempestKeepActionContext::kaelthas_sunstrider_main_tank_move_devastation_away;

        creators["kael'thas sunstrider loot legendary weapons"] =
            &RaidTempestKeepActionContext::kaelthas_sunstrider_loot_legendary_weapons;

        creators["kael'thas sunstrider use legendary weapons"] =
            &RaidTempestKeepActionContext::kaelthas_sunstrider_use_legendary_weapons;

        creators["kael'thas sunstrider reequip gear"] =
            &RaidTempestKeepActionContext::kaelthas_sunstrider_reequip_gear;

        creators["kael'thas sunstrider main tank position boss"] =
            &RaidTempestKeepActionContext::kaelthas_sunstrider_main_tank_position_boss;

        creators["kael'thas sunstrider avoid flame strike"] =
            &RaidTempestKeepActionContext::kaelthas_sunstrider_avoid_flame_strike;

        creators["kael'thas sunstrider handle phoenixes and eggs"] =
            &RaidTempestKeepActionContext::kaelthas_sunstrider_handle_phoenixes_and_eggs;

        creators["kael'thas sunstrider break mind control"] =
            &RaidTempestKeepActionContext::kaelthas_sunstrider_break_mind_control;

        creators["kael'thas sunstrider break through shock barrier"] =
            &RaidTempestKeepActionContext::kaelthas_sunstrider_break_through_shock_barrier;

        creators["kael'thas sunstrider spread out in midair"] =
            &RaidTempestKeepActionContext::kaelthas_sunstrider_spread_out_in_midair;
    }

private:
    // Trash
    static Action* crimson_hand_centurion_cast_polymorph(
        PlayerbotAI* botAI) { return new CrimsonHandCenturionCastPolymorphAction(botAI); }

    // Al'ar <Phoenix God>
    static Action* alar_misdirect_boss_to_main_tank(
        PlayerbotAI* botAI) { return new AlarMisdirectBossToMainTankAction(botAI); }

    static Action* alar_boss_tanks_move_between_platforms(
        PlayerbotAI* botAI) { return new AlarBossTanksMoveBetweenPlatformsAction(botAI); }

    static Action* alar_melee_dps_move_between_platforms(
        PlayerbotAI* botAI) { return new AlarMeleeDpsMoveBetweenPlatformsAction(botAI); }

    static Action* alar_ranged_and_ember_tank_move_under_platforms(
        PlayerbotAI* botAI) { return new AlarRangedAndEmberTankMoveUnderPlatformsAction(botAI); }

    static Action* alar_assist_tanks_pick_up_embers(
        PlayerbotAI* botAI) { return new AlarAssistTanksPickUpEmbersAction(botAI); }

    static Action* alar_ranged_dps_prioritize_embers(
        PlayerbotAI* botAI) { return new AlarRangedDpsPrioritizeEmbersAction(botAI); }

    static Action* alar_jump_from_platform(
        PlayerbotAI* botAI) { return new AlarJumpFromPlatformAction(botAI); }

    static Action* alar_move_away_from_rebirth(
        PlayerbotAI* botAI) { return new AlarMoveAwayFromRebirthAction(botAI); }

    static Action* alar_swap_tanks_on_boss(
        PlayerbotAI* botAI) { return new AlarSwapTanksOnBossAction(botAI); }

    static Action* alar_avoid_flame_patches_and_dive_bombs(
        PlayerbotAI* botAI) { return new AlarAvoidFlamePatchesAndDiveBombsAction(botAI); }

    static Action* alar_return_to_room_center(
        PlayerbotAI* botAI) { return new AlarReturnToRoomCenterAction(botAI); }

    static Action* alar_manage_phase_tracker(
        PlayerbotAI* botAI) { return new AlarManagePhaseTrackerAction(botAI); }

    // Void Reaver
    static Action* void_reaver_tanks_position_boss(
        PlayerbotAI* botAI) { return new VoidReaverTanksPositionBossAction(botAI); }

    static Action* void_reaver_use_aggro_dump_ability(
        PlayerbotAI* botAI) { return new VoidReaverUseAggroDumpAbilityAction(botAI); }

    static Action* void_reaver_spread_ranged(
        PlayerbotAI* botAI) { return new VoidReaverSpreadRangedAction(botAI); }

    static Action* void_reaver_avoid_arcane_orb(
        PlayerbotAI* botAI) { return new VoidReaverAvoidArcaneOrbAction(botAI); }

    static Action* void_reaver_erase_trackers(
        PlayerbotAI* botAI) { return new VoidReaverEraseTrackersAction(botAI); }

    // High Astromancer Solarian
    static Action* high_astromancer_solarian_ranged_leave_space_for_melee(
        PlayerbotAI* botAI) { return new HighAstromancerSolarianRangedLeaveSpaceForMeleeAction(botAI); }

    static Action* high_astromancer_solarian_move_away_from_group(
        PlayerbotAI* botAI) { return new HighAstromancerSolarianMoveAwayFromGroupAction(botAI); }

    static Action* high_astromancer_solarian_stack_for_aoe(
        PlayerbotAI* botAI) { return new HighAstromancerSolarianStackForAoeAction(botAI); }

    static Action* high_astromancer_solarian_target_solarium_priests(
        PlayerbotAI* botAI) { return new HighAstromancerSolarianTargetSolariumPriestsAction(botAI); }

    static Action* high_astromancer_solarian_cast_fear_ward_on_main_tank(
        PlayerbotAI* botAI) { return new HighAstromancerSolarianCastFearWardOnMainTankAction(botAI); }

    // Kael'thas Sunstrider <Lord of the Blood Elves>
    static Action* kaelthas_sunstrider_kite_thaladred(
        PlayerbotAI* botAI) { return new KaelthasSunstriderKiteThaladredAction(botAI); }

    static Action* kaelthas_sunstrider_misdirect_advisors_to_tanks(
        PlayerbotAI* botAI) { return new KaelthasSunstriderMisdirectAdvisorsToTanksAction(botAI); }

    static Action* kaelthas_sunstrider_main_tank_position_sanguinar(
        PlayerbotAI* botAI) { return new KaelthasSunstriderMainTankPositionSanguinarAction(botAI); }

    static Action* kaelthas_sunstrider_cast_fear_ward_on_sanguinar_tank(
        PlayerbotAI* botAI) { return new KaelthasSunstriderCastFearWardOnSanguinarTankAction(botAI); }

    static Action* kaelthas_sunstrider_warlock_tank_position_capernian(
        PlayerbotAI* botAI) { return new KaelthasSunstriderWarlockTankPositionCapernianAction(botAI); }

    static Action* kaelthas_sunstrider_spread_and_move_away_from_capernian(
        PlayerbotAI* botAI) { return new KaelthasSunstriderSpreadAndMoveAwayFromCapernianAction(botAI); }

    static Action* kaelthas_sunstrider_first_assist_tank_position_telonicus(
        PlayerbotAI* botAI) { return new KaelthasSunstriderFirstAssistTankPositionTelonicusAction(botAI); }

    static Action* kaelthas_sunstrider_handle_advisor_roles_in_phase_3(
        PlayerbotAI* botAI) { return new KaelthasSunstriderHandleAdvisorRolesInPhase3Action(botAI); }

    static Action* kaelthas_sunstrider_assign_advisor_dps_priority(
        PlayerbotAI* botAI) { return new KaelthasSunstriderAssignAdvisorDpsPriorityAction(botAI); }

    static Action* kaelthas_sunstrider_manage_advisor_dps_timer(
        PlayerbotAI* botAI) { return new KaelthasSunstriderManageAdvisorDpsTimerAction(botAI); }

    static Action* kaelthas_sunstrider_assign_legendary_weapon_dps_priority(
        PlayerbotAI* botAI) { return new KaelthasSunstriderAssignLegendaryWeaponDpsPriorityAction(botAI); }

    static Action* kaelthas_sunstrider_main_tank_move_devastation_away(
        PlayerbotAI* botAI) { return new KaelthasSunstriderMoveDevastationAwayAction(botAI); }

    static Action* kaelthas_sunstrider_loot_legendary_weapons(
        PlayerbotAI* botAI) { return new KaelthasSunstriderLootLegendaryWeaponsAction(botAI); }

    static Action* kaelthas_sunstrider_use_legendary_weapons(
        PlayerbotAI* botAI) { return new KaelthasSunstriderUseLegendaryWeaponsAction(botAI); }

    static Action* kaelthas_sunstrider_reequip_gear(
        PlayerbotAI* botAI) { return new KaelthasSunstriderReequipGearAction(botAI); }

    static Action* kaelthas_sunstrider_main_tank_position_boss(
        PlayerbotAI* botAI) { return new KaelthasSunstriderMainTankPositionBossAction(botAI); }

    static Action* kaelthas_sunstrider_avoid_flame_strike(
        PlayerbotAI* botAI) { return new KaelthasSunstriderAvoidFlameStrikeAction(botAI); }

    static Action* kaelthas_sunstrider_handle_phoenixes_and_eggs(
        PlayerbotAI* botAI) { return new KaelthasSunstriderHandlePhoenixesAndEggsAction(botAI); }

    static Action* kaelthas_sunstrider_break_mind_control(
        PlayerbotAI* botAI) { return new KaelthasSunstriderBreakMindControlAction(botAI); }

    static Action* kaelthas_sunstrider_break_through_shock_barrier(
        PlayerbotAI* botAI) { return new KaelthasSunstriderBreakThroughShockBarrierAction(botAI); }

    static Action* kaelthas_sunstrider_spread_out_in_midair(
        PlayerbotAI* botAI) { return new KaelthasSunstriderSpreadOutInMidairAction(botAI); }
};

#endif
