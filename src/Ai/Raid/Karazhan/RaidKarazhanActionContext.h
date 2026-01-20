#ifndef _PLAYERBOT_RAIDKARAZHANACTIONCONTEXT_H
#define _PLAYERBOT_RAIDKARAZHANACTIONCONTEXT_H

#include "RaidKarazhanActions.h"
#include "NamedObjectContext.h"

class RaidKarazhanActionContext : public NamedObjectContext<Action>
{
public:
    RaidKarazhanActionContext()
    {
        // Trash
        creators["mana warp stun creature before warp breach"] =
            &RaidKarazhanActionContext::mana_warp_stun_creature_before_warp_breach;

        // Attumen the Huntsman
        creators["attumen the huntsman mark target"] =
            &RaidKarazhanActionContext::attumen_the_huntsman_mark_target;

        creators["attumen the huntsman split bosses"] =
            &RaidKarazhanActionContext::attumen_the_huntsman_split_bosses;

        creators["attumen the huntsman stack behind"] =
            &RaidKarazhanActionContext::attumen_the_huntsman_stack_behind;

        creators["attumen the huntsman manage dps timer"] =
            &RaidKarazhanActionContext::attumen_the_huntsman_manage_dps_timer;

        // Moroes
        creators["moroes main tank attack boss"] =
            &RaidKarazhanActionContext::moroes_main_tank_attack_boss;

        creators["moroes mark target"] =
            &RaidKarazhanActionContext::moroes_mark_target;

        // Maiden of Virtue
        creators["maiden of virtue move boss to healer"] =
            &RaidKarazhanActionContext::maiden_of_virtue_move_boss_to_healer;

        creators["maiden of virtue position ranged"] =
            &RaidKarazhanActionContext::maiden_of_virtue_position_ranged;

        // The Big Bad Wolf
        creators["big bad wolf position boss"] =
            &RaidKarazhanActionContext::big_bad_wolf_position_boss;

        creators["big bad wolf run away from boss"] =
            &RaidKarazhanActionContext::big_bad_wolf_run_away_from_boss;

        // Romulo and Julianne
        creators["romulo and julianne mark target"] =
            &RaidKarazhanActionContext::romulo_and_julianne_mark_target;

        // The Wizard of Oz
        creators["wizard of oz mark target"] =
            &RaidKarazhanActionContext::wizard_of_oz_mark_target;

        creators["wizard of oz scorch strawman"] =
            &RaidKarazhanActionContext::wizard_of_oz_scorch_strawman;

        // The Curator
        creators["the curator mark astral flare"] =
            &RaidKarazhanActionContext::the_curator_mark_astral_flare;

        creators["the curator position boss"] =
            &RaidKarazhanActionContext::the_curator_position_boss;

        creators["the curator spread ranged"] =
            &RaidKarazhanActionContext::the_curator_spread_ranged;

        // Terestian Illhoof
        creators["terestian illhoof mark target"] =
            &RaidKarazhanActionContext::terestian_illhoof_mark_target;

        // Shade of Aran
        creators["shade of aran run away from arcane explosion"] =
            &RaidKarazhanActionContext::shade_of_aran_run_away_from_arcane_explosion;

        creators["shade of aran stop moving during flame wreath"] =
            &RaidKarazhanActionContext::shade_of_aran_stop_moving_during_flame_wreath;

        creators["shade of aran mark conjured elemental"] =
            &RaidKarazhanActionContext::shade_of_aran_mark_conjured_elemental;

        creators["shade of aran ranged maintain distance"] =
            &RaidKarazhanActionContext::shade_of_aran_ranged_maintain_distance;

        // Netherspite
        creators["netherspite block red beam"] =
            &RaidKarazhanActionContext::netherspite_block_red_beam;

        creators["netherspite block blue beam"] =
            &RaidKarazhanActionContext::netherspite_block_blue_beam;

        creators["netherspite block green beam"] =
            &RaidKarazhanActionContext::netherspite_block_green_beam;

        creators["netherspite avoid beam and void zone"] =
            &RaidKarazhanActionContext::netherspite_avoid_beam_and_void_zone;

        creators["netherspite banish phase avoid void zone"] =
            &RaidKarazhanActionContext::netherspite_banish_phase_avoid_void_zone;

        creators["netherspite manage timers and trackers"] =
            &RaidKarazhanActionContext::netherspite_manage_timers_and_trackers;

        // Prince Malchezaar
        creators["prince malchezaar enfeebled avoid hazard"] =
            &RaidKarazhanActionContext::prince_malchezaar_enfeebled_avoid_hazard;

        creators["prince malchezaar non tank avoid infernal"] =
            &RaidKarazhanActionContext::prince_malchezaar_non_tank_avoid_infernal;

        creators["prince malchezaar main tank movement"] =
            &RaidKarazhanActionContext::prince_malchezaar_main_tank_movement;

        // Nightbane
        creators["nightbane ground phase position boss"] =
            &RaidKarazhanActionContext::nightbane_ground_phase_position_boss;

        creators["nightbane ground phase rotate ranged positions"] =
            &RaidKarazhanActionContext::nightbane_ground_phase_rotate_ranged_positions;

        creators["nightbane cast fear ward on main tank"] =
            &RaidKarazhanActionContext::nightbane_cast_fear_ward_on_main_tank;

        creators["nightbane control pet aggression"] =
            &RaidKarazhanActionContext::nightbane_control_pet_aggression;

        creators["nightbane flight phase movement"] =
            &RaidKarazhanActionContext::nightbane_flight_phase_movement;

        creators["nightbane manage timers and trackers"] =
            &RaidKarazhanActionContext::nightbane_manage_timers_and_trackers;
    }

private:
    // Trash
    static Action* mana_warp_stun_creature_before_warp_breach(
        PlayerbotAI* botAI) { return new ManaWarpStunCreatureBeforeWarpBreachAction(botAI); }

    // Attumen the Huntsman
    static Action* attumen_the_huntsman_mark_target(
        PlayerbotAI* botAI) { return new AttumenTheHuntsmanMarkTargetAction(botAI); }

    static Action* attumen_the_huntsman_split_bosses(
        PlayerbotAI* botAI) { return new AttumenTheHuntsmanSplitBossesAction(botAI); }

    static Action* attumen_the_huntsman_stack_behind(
        PlayerbotAI* botAI) { return new AttumenTheHuntsmanStackBehindAction(botAI); }

    static Action* attumen_the_huntsman_manage_dps_timer(
        PlayerbotAI* botAI) { return new AttumenTheHuntsmanManageDpsTimerAction(botAI); }

    // Moroes
    static Action* moroes_main_tank_attack_boss(
        PlayerbotAI* botAI) { return new MoroesMainTankAttackBossAction(botAI); }

    static Action* moroes_mark_target(
        PlayerbotAI* botAI) { return new MoroesMarkTargetAction(botAI); }

    // Maiden of Virtue
    static Action* maiden_of_virtue_move_boss_to_healer(
        PlayerbotAI* botAI) { return new MaidenOfVirtueMoveBossToHealerAction(botAI); }

    static Action* maiden_of_virtue_position_ranged(
        PlayerbotAI* botAI) { return new MaidenOfVirtuePositionRangedAction(botAI); }

    // The Big Bad Wolf
    static Action* big_bad_wolf_position_boss(
        PlayerbotAI* botAI) { return new BigBadWolfPositionBossAction(botAI); }

    static Action* big_bad_wolf_run_away_from_boss(
        PlayerbotAI* botAI) { return new BigBadWolfRunAwayFromBossAction(botAI); }

    // Romulo and Julianne
    static Action* romulo_and_julianne_mark_target(
        PlayerbotAI* botAI) { return new RomuloAndJulianneMarkTargetAction(botAI); }

    // The Wizard of Oz
    static Action* wizard_of_oz_mark_target(
        PlayerbotAI* botAI) { return new WizardOfOzMarkTargetAction(botAI); }

    static Action* wizard_of_oz_scorch_strawman(
        PlayerbotAI* botAI) { return new WizardOfOzScorchStrawmanAction(botAI); }

    // The Curator
    static Action* the_curator_mark_astral_flare(
        PlayerbotAI* botAI) { return new TheCuratorMarkAstralFlareAction(botAI); }

    static Action* the_curator_position_boss(
        PlayerbotAI* botAI) { return new TheCuratorPositionBossAction(botAI); }

    static Action* the_curator_spread_ranged(
        PlayerbotAI* botAI) { return new TheCuratorSpreadRangedAction(botAI); }

    // Terestian Illhoof
    static Action* terestian_illhoof_mark_target(
        PlayerbotAI* botAI) { return new TerestianIllhoofMarkTargetAction(botAI); }

    // Shade of Aran
    static Action* shade_of_aran_run_away_from_arcane_explosion(
        PlayerbotAI* botAI) { return new ShadeOfAranRunAwayFromArcaneExplosionAction(botAI); }

    static Action* shade_of_aran_stop_moving_during_flame_wreath(
        PlayerbotAI* botAI) { return new ShadeOfAranStopMovingDuringFlameWreathAction(botAI); }

    static Action* shade_of_aran_mark_conjured_elemental(
        PlayerbotAI* botAI) { return new ShadeOfAranMarkConjuredElementalAction(botAI); }

    static Action* shade_of_aran_ranged_maintain_distance(
        PlayerbotAI* botAI) { return new ShadeOfAranRangedMaintainDistanceAction(botAI); }

    // Netherspite
    static Action* netherspite_block_red_beam(
        PlayerbotAI* botAI) { return new NetherspiteBlockRedBeamAction(botAI); }

    static Action* netherspite_block_blue_beam(
        PlayerbotAI* botAI) { return new NetherspiteBlockBlueBeamAction(botAI); }

    static Action* netherspite_block_green_beam(
        PlayerbotAI* botAI) { return new NetherspiteBlockGreenBeamAction(botAI); }

    static Action* netherspite_avoid_beam_and_void_zone(
        PlayerbotAI* botAI) { return new NetherspiteAvoidBeamAndVoidZoneAction(botAI); }

    static Action* netherspite_banish_phase_avoid_void_zone(
        PlayerbotAI* botAI) { return new NetherspiteBanishPhaseAvoidVoidZoneAction(botAI); }

    static Action* netherspite_manage_timers_and_trackers(
        PlayerbotAI* botAI) { return new NetherspiteManageTimersAndTrackersAction(botAI); }

    // Prince Malchezaar
    static Action* prince_malchezaar_enfeebled_avoid_hazard(
        PlayerbotAI* botAI) { return new PrinceMalchezaarEnfeebledAvoidHazardAction(botAI); }

    static Action* prince_malchezaar_non_tank_avoid_infernal(
        PlayerbotAI* botAI) { return new PrinceMalchezaarNonTankAvoidInfernalAction(botAI); }

    static Action* prince_malchezaar_main_tank_movement(
        PlayerbotAI* botAI) { return new PrinceMalchezaarMainTankMovementAction(botAI); }

    // Nightbane
    static Action* nightbane_ground_phase_position_boss(
        PlayerbotAI* botAI) { return new NightbaneGroundPhasePositionBossAction(botAI); }

    static Action* nightbane_ground_phase_rotate_ranged_positions(
        PlayerbotAI* botAI) { return new NightbaneGroundPhaseRotateRangedPositionsAction(botAI); }

    static Action* nightbane_cast_fear_ward_on_main_tank(
        PlayerbotAI* botAI) { return new NightbaneCastFearWardOnMainTankAction(botAI); }

    static Action* nightbane_control_pet_aggression(
        PlayerbotAI* botAI) { return new NightbaneControlPetAggressionAction(botAI); }

    static Action* nightbane_flight_phase_movement(
        PlayerbotAI* botAI) { return new NightbaneFlightPhaseMovementAction(botAI); }

    static Action* nightbane_manage_timers_and_trackers(
        PlayerbotAI* botAI) { return new NightbaneManageTimersAndTrackersAction(botAI); }
};

#endif
