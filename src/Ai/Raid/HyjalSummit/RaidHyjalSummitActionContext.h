/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_RAIDHYJALSUMMITACTIONCONTEXT_H
#define _PLAYERBOT_RAIDHYJALSUMMITACTIONCONTEXT_H

#include "RaidHyjalSummitActions.h"
#include "NamedObjectContext.h"

class RaidHyjalSummitActionContext : public NamedObjectContext<Action>
{
public:
    RaidHyjalSummitActionContext()
    {
        // General
        creators["hyjal summit erase trackers"] =
            &RaidHyjalSummitActionContext::hyjal_summit_erase_trackers;

        // Rage Winterchill
        creators["rage winterchill misdirect boss to main tank"] =
            &RaidHyjalSummitActionContext::rage_winterchill_misdirect_boss_to_main_tank;

        creators["rage winterchill main tank position boss"] =
            &RaidHyjalSummitActionContext::rage_winterchill_main_tank_position_boss;

        creators["rage winterchill spread ranged in circle"] =
            &RaidHyjalSummitActionContext::rage_winterchill_spread_ranged_in_circle;

        creators["rage winterchill melee get out of death and decay"] =
            &RaidHyjalSummitActionContext::rage_winterchill_melee_get_out_of_death_and_decay;

        // Anetheron
        creators["anetheron misdirect boss and infernals to tanks"] =
            &RaidHyjalSummitActionContext::anetheron_misdirect_boss_and_infernals_to_tanks;

        creators["anetheron main tank position boss"] =
            &RaidHyjalSummitActionContext::anetheron_main_tank_position_boss;

        creators["anetheron spread ranged in circle"] =
            &RaidHyjalSummitActionContext::anetheron_spread_ranged_in_circle;

        creators["anetheron bring infernal to infernal tank"] =
            &RaidHyjalSummitActionContext::anetheron_bring_infernal_to_infernal_tank;

        creators["anetheron first assist tank pick up infernals"] =
            &RaidHyjalSummitActionContext::anetheron_first_assist_tank_pick_up_infernals;

        creators["anetheron assign dps priority"] =
            &RaidHyjalSummitActionContext::anetheron_assign_dps_priority;

        // Kaz'rogal
        creators["kaz'rogal misdirect boss to main tank"] =
            &RaidHyjalSummitActionContext::kazrogal_misdirect_boss_to_main_tank;

        creators["kaz'rogal main tank position boss"] =
            &RaidHyjalSummitActionContext::kazrogal_main_tank_position_boss;

        creators["kaz'rogal assist tanks move in front of boss"] =
            &RaidHyjalSummitActionContext::kazrogal_assist_tanks_move_in_front_of_boss;

        creators["kaz'rogal spread ranged in arc"] =
            &RaidHyjalSummitActionContext::kazrogal_spread_ranged_in_arc;

        creators["kaz'rogal low mana bot take defensive measures"] =
            &RaidHyjalSummitActionContext::kazrogal_low_mana_bot_take_defensive_measures;

        creators["kaz'rogal cast shadow protection spell"] =
            &RaidHyjalSummitActionContext::kazrogal_cast_shadow_protection_spell;

        // Azgalor
        creators["azgalor misdirect boss to main tank"] =
            &RaidHyjalSummitActionContext::azgalor_misdirect_boss_to_main_tank;

        creators["azgalor main tank position boss"] =
            &RaidHyjalSummitActionContext::azgalor_main_tank_position_boss;

        creators["azgalor wait at safe position"] =
            &RaidHyjalSummitActionContext::azgalor_wait_at_safe_position;

        creators["azgalor disperse ranged"] =
            &RaidHyjalSummitActionContext::azgalor_disperse_ranged;

        creators["azgalor melee get out of fire and swap targets"] =
            &RaidHyjalSummitActionContext::azgalor_melee_get_out_of_fire_and_swap_targets;

        creators["azgalor move to doomguard tank"] =
            &RaidHyjalSummitActionContext::azgalor_move_to_doomguard_tank;

        creators["azgalor first assist tank position doomguard"] =
            &RaidHyjalSummitActionContext::azgalor_first_assist_tank_position_doomguard;

        creators["azgalor ranged dps prioritize doomguards"] =
            &RaidHyjalSummitActionContext::azgalor_ranged_dps_prioritize_doomguards;

        // Archimonde
        creators["archimonde misdirect boss to main tank"] =
            &RaidHyjalSummitActionContext::archimonde_misdirect_boss_to_main_tank;

        creators["archimonde move boss to initial position"] =
            &RaidHyjalSummitActionContext::archimonde_move_boss_to_initial_position;

        creators["archimonde cast fear immunity spell"] =
            &RaidHyjalSummitActionContext::archimonde_cast_fear_immunity_spell;

        creators["archimonde spread to avoid air burst"] =
            &RaidHyjalSummitActionContext::archimonde_spread_to_avoid_air_burst;

        creators["archimonde avoid doomfire"] =
            &RaidHyjalSummitActionContext::archimonde_avoid_doomfire;

        creators["archimonde remove doomfire dot"] =
            &RaidHyjalSummitActionContext::archimonde_remove_doomfire_dot;
    }

private:
    // General
    static Action* hyjal_summit_erase_trackers(
        PlayerbotAI* botAI) { return new HyjalSummitEraseTrackersAction(botAI); }

    // Rage Winterchill
    static Action* rage_winterchill_misdirect_boss_to_main_tank(
        PlayerbotAI* botAI) { return new RageWinterchillMisdirectBossToMainTankAction(botAI); }

    static Action* rage_winterchill_main_tank_position_boss(
        PlayerbotAI* botAI) { return new RageWinterchillMainTankPositionBossAction(botAI); }

    static Action* rage_winterchill_spread_ranged_in_circle(
        PlayerbotAI* botAI) { return new RageWinterchillSpreadRangedInCircleAction(botAI); }

    static Action* rage_winterchill_melee_get_out_of_death_and_decay(
        PlayerbotAI* botAI) { return new RageWinterchillMeleeGetOutOfDeathAndDecayAction(botAI); }

    // Anetheron
    static Action* anetheron_misdirect_boss_and_infernals_to_tanks(
        PlayerbotAI* botAI) { return new AnetheronMisdirectBossAndInfernalsToTanksAction(botAI); }

    static Action* anetheron_main_tank_position_boss(
        PlayerbotAI* botAI) { return new AnetheronMainTankPositionBossAction(botAI); }

    static Action* anetheron_spread_ranged_in_circle(
        PlayerbotAI* botAI) { return new AnetheronSpreadRangedInCircleAction(botAI); }

    static Action* anetheron_bring_infernal_to_infernal_tank(
        PlayerbotAI* botAI) { return new AnetheronBringInfernalToInfernalTankAction(botAI); }

    static Action* anetheron_first_assist_tank_pick_up_infernals(
        PlayerbotAI* botAI) { return new AnetheronFirstAssistTankPickUpInfernalsAction(botAI); }

    static Action* anetheron_assign_dps_priority(
        PlayerbotAI* botAI) { return new AnetheronAssignDpsPriorityAction(botAI); }

    // Kaz'rogal
    static Action* kazrogal_misdirect_boss_to_main_tank(
        PlayerbotAI* botAI) { return new KazrogalMisdirectBossToMainTankAction(botAI); }

    static Action* kazrogal_main_tank_position_boss(
        PlayerbotAI* botAI) { return new KazrogalMainTankPositionBossAction(botAI); }

    static Action* kazrogal_assist_tanks_move_in_front_of_boss(
        PlayerbotAI* botAI) { return new KazrogalAssistTanksMoveInFrontOfBossAction(botAI); }

    static Action* kazrogal_spread_ranged_in_arc(
        PlayerbotAI* botAI) { return new KazrogalSpreadRangedInArcAction(botAI); }

    static Action* kazrogal_low_mana_bot_take_defensive_measures(
        PlayerbotAI* botAI) { return new KazrogalLowManaBotTakeDefensiveMeasuresAction(botAI); }

    static Action* kazrogal_cast_shadow_protection_spell(
        PlayerbotAI* botAI) { return new KazrogalCastShadowProtectionSpellAction(botAI); }

    // Azgalor
    static Action* azgalor_misdirect_boss_to_main_tank(
        PlayerbotAI* botAI) { return new AzgalorMisdirectBossToMainTankAction(botAI); }

    static Action* azgalor_main_tank_position_boss(
        PlayerbotAI* botAI) { return new AzgalorMainTankPositionBossAction(botAI); }

    static Action* azgalor_wait_at_safe_position(
        PlayerbotAI* botAI) { return new AzgalorWaitAtSafePositionAction(botAI); }

    static Action* azgalor_disperse_ranged(
        PlayerbotAI* botAI) { return new AzgalorDisperseRangedAction(botAI); }

    static Action* azgalor_melee_get_out_of_fire_and_swap_targets(
        PlayerbotAI* botAI) { return new AzgalorMeleeGetOutOfFireAndSwapTargetsAction(botAI); }

    static Action* azgalor_move_to_doomguard_tank(
        PlayerbotAI* botAI) { return new AzgalorMoveToDoomguardTankAction(botAI); }

    static Action* azgalor_first_assist_tank_position_doomguard(
        PlayerbotAI* botAI) { return new AzgalorFirstAssistTankPositionDoomguardAction(botAI); }

    static Action* azgalor_ranged_dps_prioritize_doomguards(
        PlayerbotAI* botAI) { return new AzgalorRangedDpsPrioritizeDoomguardsAction(botAI); }

    // Archimonde
    static Action* archimonde_misdirect_boss_to_main_tank(
        PlayerbotAI* botAI) { return new ArchimondeMisdirectBossToMainTankAction(botAI); }

    static Action* archimonde_move_boss_to_initial_position(
        PlayerbotAI* botAI) { return new ArchimondeMoveBossToInitialPositionAction(botAI); }

    static Action* archimonde_cast_fear_immunity_spell(
        PlayerbotAI* botAI) { return new ArchimondeCastFearImmunitySpellAction(botAI); }

    static Action* archimonde_spread_to_avoid_air_burst(
        PlayerbotAI* botAI) { return new ArchimondeSpreadToAvoidAirBurstAction(botAI); }

    static Action* archimonde_avoid_doomfire(
        PlayerbotAI* botAI) { return new ArchimondeAvoidDoomfireAction(botAI); }

    static Action* archimonde_remove_doomfire_dot(
        PlayerbotAI* botAI) { return new ArchimondeRemoveDoomfireDotAction(botAI); }
};

#endif
