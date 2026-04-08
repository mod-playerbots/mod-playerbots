/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_RAIDZULAMANACTIONCONTEXT_H
#define _PLAYERBOT_RAIDZULAMANACTIONCONTEXT_H

#include "RaidZulAmanActions.h"
#include "NamedObjectContext.h"

class RaidZulAmanActionContext : public NamedObjectContext<Action>
{
public:
    RaidZulAmanActionContext()
    {
        // Trash
        creators["amani'shi medicine man mark ward"] =
            &RaidZulAmanActionContext::amanishi_medicine_man_mark_ward;

        // Akil'zon <Eagle Avatar>
        creators["akil'zon misdirect boss to main tank"] =
            &RaidZulAmanActionContext::akilzon_misdirect_boss_to_main_tank;

        creators["akil'zon tanks position boss"] =
            &RaidZulAmanActionContext::akilzon_tanks_position_boss;

        creators["akil'zon spread ranged"] =
            &RaidZulAmanActionContext::akilzon_spread_ranged;

        creators["akil'zon move to eye of the storm"] =
            &RaidZulAmanActionContext::akilzon_move_to_eye_of_the_storm;

        creators["akil'zon manage electrical storm timer"] =
            &RaidZulAmanActionContext::akilzon_manage_electrical_storm_timer;

        // Nalorakk <Bear Avatar>
        creators["nalorakk misdirect boss to main tank"] =
            &RaidZulAmanActionContext::nalorakk_misdirect_boss_to_main_tank;

        creators["nalorakk tanks position boss"] =
            &RaidZulAmanActionContext::nalorakk_tanks_position_boss;

        creators["nalorakk spread ranged"] =
            &RaidZulAmanActionContext::nalorakk_spread_ranged;

        // Jan'alai <Dragonhawk Avatar>
        creators["jan'alai misdirect boss to main tank"] =
            &RaidZulAmanActionContext::janalai_misdirect_boss_to_main_tank;

        creators["jan'alai tanks position boss"] =
            &RaidZulAmanActionContext::janalai_tanks_position_boss;

        creators["jan'alai spread ranged in circle"] =
            &RaidZulAmanActionContext::janalai_spread_ranged_in_circle;

        creators["jan'alai avoid fire bombs"] =
            &RaidZulAmanActionContext::janalai_avoid_fire_bombs;

        creators["jan'alai mark amani'shi hatchers"] =
            &RaidZulAmanActionContext::janalai_mark_amanishi_hatchers;

        // Halazzi <Lynx Avatar>
        creators["halazzi misdirect boss to main tank"] =
            &RaidZulAmanActionContext::halazzi_misdirect_boss_to_main_tank;

        creators["halazzi main tank position boss"] =
            &RaidZulAmanActionContext::halazzi_main_tank_position_boss;

        creators["halazzi first assist tank attack spirit lynx"] =
            &RaidZulAmanActionContext::halazzi_first_assist_tank_attack_spirit_lynx;

        creators["halazzi assign dps priority"] =
            &RaidZulAmanActionContext::halazzi_assign_dps_priority;

        // Hex Lord Malacrass
        creators["hex lord malacrass misdirect boss to main tank"] =
            &RaidZulAmanActionContext::hex_lord_malacrass_misdirect_boss_to_main_tank;

        creators["hex lord malacrass assign dps priority"] =
            &RaidZulAmanActionContext::hex_lord_malacrass_assign_dps_priority;

        creators["hex lord malacrass run away from whirlwind"] =
            &RaidZulAmanActionContext::hex_lord_malacrass_run_away_from_whirlwind;

        creators["hex lord malacrass casters stop attacking"] =
            &RaidZulAmanActionContext::hex_lord_malacrass_casters_stop_attacking;

        creators["hex lord malacrass move away from freezing trap"] =
            &RaidZulAmanActionContext::hex_lord_malacrass_move_away_from_freezing_trap;

        // Zul'jin
        creators["zul'jin misdirect boss to main tank"] =
            &RaidZulAmanActionContext::zuljin_misdirect_boss_to_main_tank;

        creators["zul'jin tanks position boss"] =
            &RaidZulAmanActionContext::zuljin_tanks_position_boss;

        creators["zul'jin run away from whirlwind"] =
            &RaidZulAmanActionContext::zuljin_run_away_from_whirlwind;

        creators["zul'jin avoid cyclones"] =
            &RaidZulAmanActionContext::zuljin_avoid_cyclones;

        creators["zul'jin spread ranged"] =
            &RaidZulAmanActionContext::zuljin_spread_ranged;
    }

private:
    // Trash
    static Action* amanishi_medicine_man_mark_ward(
        PlayerbotAI* botAI) { return new AmanishiMedicineManMarkWardAction(botAI); }

    // Akil'zon <Eagle Avatar>
    static Action* akilzon_misdirect_boss_to_main_tank(
        PlayerbotAI* botAI) { return new AkilzonMisdirectBossToMainTankAction(botAI); }

    static Action* akilzon_tanks_position_boss(
        PlayerbotAI* botAI) { return new AkilzonTanksPositionBossAction(botAI); }

    static Action* akilzon_spread_ranged(
        PlayerbotAI* botAI) { return new AkilzonSpreadRangedAction(botAI); }

    static Action* akilzon_move_to_eye_of_the_storm(
        PlayerbotAI* botAI) { return new AkilzonMoveToEyeOfTheStormAction(botAI); }

    static Action* akilzon_manage_electrical_storm_timer(
        PlayerbotAI* botAI) { return new AkilzonManageElectricalStormTimerAction(botAI); }

    // Nalorakk <Bear Avatar>
    static Action* nalorakk_misdirect_boss_to_main_tank(
        PlayerbotAI* botAI) { return new NalorakkMisdirectBossToMainTankAction(botAI); }

    static Action* nalorakk_tanks_position_boss(
        PlayerbotAI* botAI) { return new NalorakkTanksPositionBossAction(botAI); }

    static Action* nalorakk_spread_ranged(
        PlayerbotAI* botAI) { return new NalorakkSpreadRangedAction(botAI); }

    // Jan'alai <Dragonhawk Avatar>
    static Action* janalai_misdirect_boss_to_main_tank(
        PlayerbotAI* botAI) { return new JanalaiMisdirectBossToMainTankAction(botAI); }

    static Action* janalai_tanks_position_boss(
        PlayerbotAI* botAI) { return new JanalaiTanksPositionBossAction(botAI); }

    static Action* janalai_spread_ranged_in_circle(
        PlayerbotAI* botAI) { return new JanalaiSpreadRangedInCircleAction(botAI); }

    static Action* janalai_avoid_fire_bombs(
        PlayerbotAI* botAI) { return new JanalaiAvoidFireBombsAction(botAI); }

    static Action* janalai_mark_amanishi_hatchers(
        PlayerbotAI* botAI) { return new JanalaiMarkAmanishiHatchersAction(botAI); }

    // Halazzi <Lynx Avatar>
    static Action* halazzi_misdirect_boss_to_main_tank(
        PlayerbotAI* botAI) { return new HalazziMisdirectBossToMainTankAction(botAI); }

    static Action* halazzi_main_tank_position_boss(
        PlayerbotAI* botAI) { return new HalazziMainTankPositionBossAction(botAI); }

    static Action* halazzi_first_assist_tank_attack_spirit_lynx(
        PlayerbotAI* botAI) { return new HalazziFirstAssistTankAttackSpiritLynxAction(botAI); }

    static Action* halazzi_assign_dps_priority(
        PlayerbotAI* botAI) { return new HalazziAssignDpsPriorityAction(botAI); }

    // Hex Lord Malacrass
    static Action* hex_lord_malacrass_misdirect_boss_to_main_tank(
        PlayerbotAI* botAI) { return new HexLordMalacrassMisdirectBossToMainTankAction(botAI); }

    static Action* hex_lord_malacrass_assign_dps_priority(
        PlayerbotAI* botAI) { return new HexLordMalacrassAssignDpsPriorityAction(botAI); }

    static Action* hex_lord_malacrass_run_away_from_whirlwind(
        PlayerbotAI* botAI) { return new HexLordMalacrassRunAwayFromWhirlwindAction(botAI); }

    static Action* hex_lord_malacrass_casters_stop_attacking(
        PlayerbotAI* botAI) { return new HexLordMalacrassCastersStopAttackingAction(botAI); }

    static Action* hex_lord_malacrass_move_away_from_freezing_trap(
        PlayerbotAI* botAI) { return new HexLordMalacrassMoveAwayFromFreezingTrapAction(botAI); }

    // Zul'jin
    static Action* zuljin_misdirect_boss_to_main_tank(
        PlayerbotAI* botAI) { return new ZuljinMisdirectBossToMainTankAction(botAI); }

    static Action* zuljin_tanks_position_boss(
        PlayerbotAI* botAI) { return new ZuljinTanksPositionBossAction(botAI); }

    static Action* zuljin_run_away_from_whirlwind(
        PlayerbotAI* botAI) { return new ZuljinRunAwayFromWhirlwindAction(botAI); }

    static Action* zuljin_avoid_cyclones(
        PlayerbotAI* botAI) { return new ZuljinAvoidCyclonesAction(botAI); }

    static Action* zuljin_spread_ranged(
        PlayerbotAI* botAI) { return new ZuljinSpreadRangedAction(botAI); }
};

#endif
