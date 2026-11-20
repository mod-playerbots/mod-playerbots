/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_KARAACTIONCONTEXT_H
#define PLAYERBOTS_KARAACTIONCONTEXT_H

#include "KaraActions.h"
#include "NamedObjectContext.h"

class RaidKarazhanActionContext : public NamedObjectContext<Action>
{
public:
    RaidKarazhanActionContext()
    {
        // Shared
        creators["karazhan reset encounter states"] =
            &RaidKarazhanActionContext::karazhan_reset_encounter_states;

        creators["karazhan set tremor totem"] =
            &RaidKarazhanActionContext::karazhan_set_tremor_totem;

        // Trash
        creators["mana warp cast stun spell"] =
            &RaidKarazhanActionContext::mana_warp_cast_stun_spell;

        creators["mana warp stand by to stun"] =
            &RaidKarazhanActionContext::mana_warp_stand_by_to_stun;

        // Attumen the Huntsman
        creators["attumen the huntsman assist tank position unmounted"] =
            &RaidKarazhanActionContext::attumen_the_huntsman_assist_tank_position_unmounted;

        creators["attumen the huntsman attack midnight"] =
            &RaidKarazhanActionContext::attumen_the_huntsman_attack_midnight;

        creators["attumen the huntsman tank position mounted"] =
            &RaidKarazhanActionContext::attumen_the_huntsman_tank_position_mounted;

        creators["attumen the huntsman stack behind"] =
            &RaidKarazhanActionContext::attumen_the_huntsman_stack_behind;

        creators["attumen the huntsman stop attacking"] =
            &RaidKarazhanActionContext::attumen_the_huntsman_stop_attacking;

        creators["attumen the huntsman select target for hold"] =
            &RaidKarazhanActionContext::attumen_the_huntsman_select_target_for_hold;

        creators["attumen the huntsman manage dps timer"] =
            &RaidKarazhanActionContext::attumen_the_huntsman_manage_dps_timer;

        // Moroes
        creators["moroes mark target"] = &RaidKarazhanActionContext::moroes_mark_target;

        // Maiden of Virtue
        creators["maiden of virtue tank position boss"] =
            &RaidKarazhanActionContext::maiden_of_virtue_tank_position_boss;

        creators["maiden of virtue move boss to stunned healer"] =
            &RaidKarazhanActionContext::maiden_of_virtue_move_boss_to_stunned_healer;

        creators["maiden of virtue position ranged between pillars"] =
            &RaidKarazhanActionContext::maiden_of_virtue_position_ranged_between_pillars;

        creators["maiden of virtue set grounding totem"] =
            &RaidKarazhanActionContext::maiden_of_virtue_set_grounding_totem;

        // The Big Bad Wolf
        creators["big bad wolf tank position boss"] =
            &RaidKarazhanActionContext::big_bad_wolf_tank_position_boss;

        creators["big bad wolf red hood use escape ability"] =
            &RaidKarazhanActionContext::big_bad_wolf_red_hood_use_escape_ability;

        creators["big bad wolf red hood run away"] =
            &RaidKarazhanActionContext::big_bad_wolf_red_hood_run_away;

        creators["big bad wolf protect red hood"] =
            &RaidKarazhanActionContext::big_bad_wolf_protect_red_hood;

        // Romulo and Julianne
        creators["romulo and julianne skull higher health"] =
            &RaidKarazhanActionContext::romulo_and_julianne_skull_higher_health;

        // The Wizard of Oz
        creators["wizard of oz mark target"] =
            &RaidKarazhanActionContext::wizard_of_oz_mark_target;

        creators["wizard of oz cast scorch on strawman"] =
            &RaidKarazhanActionContext::wizard_of_oz_cast_scorch_on_strawman;

        // The Curator
        creators["the curator mark astral flare"] =
            &RaidKarazhanActionContext::the_curator_mark_astral_flare;

        creators["the curator tank position boss"] =
            &RaidKarazhanActionContext::the_curator_tank_position_boss;

        creators["the curator spread ranged"] =
            &RaidKarazhanActionContext::the_curator_spread_ranged;

        // Terestian Illhoof
        creators["terestian illhoof mark target"] =
            &RaidKarazhanActionContext::terestian_illhoof_mark_target;

        // Shade of Aran
        creators["shade of aran run away from arcane explosion"] =
            &RaidKarazhanActionContext::shade_of_aran_run_away_from_arcane_explosion;

        creators["shade of aran move behind blizzard"] =
            &RaidKarazhanActionContext::shade_of_aran_move_behind_blizzard;

        creators["shade of aran stop moving"] =
            &RaidKarazhanActionContext::shade_of_aran_stop_moving;

        creators["shade of aran mark elemental"] =
            &RaidKarazhanActionContext::shade_of_aran_mark_elemental;

        creators["shade of aran ranged maintain distance"] =
            &RaidKarazhanActionContext::shade_of_aran_ranged_maintain_distance;

        // Netherspite
        creators["netherspite block red beam"] =
            &RaidKarazhanActionContext::netherspite_block_red_beam;

        creators["netherspite block blue or green beam"] =
            &RaidKarazhanActionContext::netherspite_block_blue_or_green_beam;

        creators["netherspite wait to block beam"] =
            &RaidKarazhanActionContext::netherspite_wait_to_block_beam;

        creators["netherspite move between blue and green beams"] =
            &RaidKarazhanActionContext::netherspite_move_between_blue_and_green_beams;

        creators["netherspite move to banish slot"] =
            &RaidKarazhanActionContext::netherspite_move_to_banish_slot;

        creators["netherspite stop attacking"] =
            &RaidKarazhanActionContext::netherspite_stop_attacking;

        creators["netherspite select target for hold"] =
            &RaidKarazhanActionContext::netherspite_select_target_for_hold;

        creators["netherspite set pet stance"] =
            &RaidKarazhanActionContext::netherspite_set_pet_stance;

        creators["netherspite manage timers and trackers"] =
            &RaidKarazhanActionContext::netherspite_manage_timers_and_trackers;

        // Prince Malchezaar
        creators["prince malchezaar enfeebled bot run to safety"] =
            &RaidKarazhanActionContext::prince_malchezaar_enfeebled_bot_run_to_safety;

        creators["prince malchezaar move clear of hazards"] =
            &RaidKarazhanActionContext::prince_malchezaar_move_clear_of_hazards;

        creators["prince malchezaar tank position boss"] =
            &RaidKarazhanActionContext::prince_malchezaar_tank_position_boss;

        // Nightbane
        creators["nightbane tank position boss"] =
            &RaidKarazhanActionContext::nightbane_tank_position_boss;

        creators["nightbane ranged move from charred earth"] =
            &RaidKarazhanActionContext::nightbane_ranged_move_from_charred_earth;

        creators["nightbane melee move from charred earth"] =
            &RaidKarazhanActionContext::nightbane_melee_move_from_charred_earth;

        creators["nightbane set pet stance"] = &RaidKarazhanActionContext::nightbane_set_pet_stance;

        creators["nightbane stop attacking"] = &RaidKarazhanActionContext::nightbane_stop_attacking;

        creators["nightbane select target for hold"] =
            &RaidKarazhanActionContext::nightbane_select_target_for_hold;

        creators["nightbane stack away from rain of bones"] =
            &RaidKarazhanActionContext::nightbane_stack_away_from_rain_of_bones;

        creators["nightbane leave combat for landing"] =
            &RaidKarazhanActionContext::nightbane_leave_combat_for_landing;

        creators["nightbane manage timers and trackers"] =
            &RaidKarazhanActionContext::nightbane_manage_timers_and_trackers;
    }

private:
    // Shared
    static Action* karazhan_reset_encounter_states(PlayerbotAI* botAI)
    {
        return new KarazhanResetEncounterStatesAction(botAI);
    }
    static Action* karazhan_set_tremor_totem(PlayerbotAI* botAI)
    {
        return new KarazhanSetTremorTotemAction(botAI);
    }

    // Trash
    static Action* mana_warp_cast_stun_spell(PlayerbotAI* botAI)
    {
        return new ManaWarpCastStunSpellAction(botAI);
    }
    static Action* mana_warp_stand_by_to_stun(PlayerbotAI* botAI)
    {
        return new ManaWarpStandByToStunAction(botAI);
    }

    // Attumen the Huntsman
    static Action* attumen_the_huntsman_assist_tank_position_unmounted(PlayerbotAI* botAI)
    {
        return new AttumenTheHuntsmanAssistTankPositionUnmountedAction(botAI);
    }
    static Action* attumen_the_huntsman_attack_midnight(PlayerbotAI* botAI)
    {
        return new AttumenTheHuntsmanAttackMidnightAction(botAI);
    }
    static Action* attumen_the_huntsman_tank_position_mounted(PlayerbotAI* botAI)
    {
        return new AttumenTheHuntsmanTankPositionMountedAction(botAI);
    }
    static Action* attumen_the_huntsman_stack_behind(PlayerbotAI* botAI)
    {
        return new AttumenTheHuntsmanStackBehindAction(botAI);
    }
    static Action* attumen_the_huntsman_stop_attacking(PlayerbotAI* botAI)
    {
        return new AttumenTheHuntsmanStopAttackingAction(botAI);
    }
    static Action* attumen_the_huntsman_select_target_for_hold(PlayerbotAI* botAI)
    {
        return new AttumenTheHuntsmanSelectTargetForHoldAction(botAI);
    }
    static Action* attumen_the_huntsman_manage_dps_timer(PlayerbotAI* botAI)
    {
        return new AttumenTheHuntsmanManageDpsTimerAction(botAI);
    }

    // Moroes
    static Action* moroes_mark_target(PlayerbotAI* botAI)
    {
        return new MoroesMarkTargetAction(botAI);
    }

    // Maiden of Virtue
    static Action* maiden_of_virtue_tank_position_boss(PlayerbotAI* botAI)
    {
        return new MaidenOfVirtueTankPositionBossAction(botAI);
    }
    static Action* maiden_of_virtue_move_boss_to_stunned_healer(PlayerbotAI* botAI)
    {
        return new MaidenOfVirtueMoveBossToStunnedHealerAction(botAI);
    }
    static Action* maiden_of_virtue_position_ranged_between_pillars(PlayerbotAI* botAI)
    {
        return new MaidenOfVirtuePositionRangedBetweenPillarsAction(botAI);
    }
    static Action* maiden_of_virtue_set_grounding_totem(PlayerbotAI* botAI)
    {
        return new MaidenOfVirtueSetGroundingTotemAction(botAI);
    }

    // The Big Bad Wolf
    static Action* big_bad_wolf_tank_position_boss(PlayerbotAI* botAI)
    {
        return new BigBadWolfTankPositionBossAction(botAI);
    }
    static Action* big_bad_wolf_red_hood_use_escape_ability(PlayerbotAI* botAI)
    {
        return new BigBadWolfRedHoodUseEscapeAbilityAction(botAI);
    }
    static Action* big_bad_wolf_red_hood_run_away(PlayerbotAI* botAI)
    {
        return new BigBadWolfRedHoodRunAwayAction(botAI);
    }
    static Action* big_bad_wolf_protect_red_hood(PlayerbotAI* botAI)
    {
        return new BigBadWolfProtectRedHoodAction(botAI);
    }

    // Romulo and Julianne
    static Action* romulo_and_julianne_skull_higher_health(PlayerbotAI* botAI)
    {
        return new RomuloAndJulianneSkullHigherHealthAction(botAI);
    }

    // The Wizard of Oz
    static Action* wizard_of_oz_mark_target(PlayerbotAI* botAI)
    {
        return new WizardOfOzMarkTargetAction(botAI);
    }
    static Action* wizard_of_oz_cast_scorch_on_strawman(PlayerbotAI* botAI)
    {
        return new WizardOfOzCastScorchOnStrawmanAction(botAI);
    }

    // The Curator
    static Action* the_curator_mark_astral_flare(PlayerbotAI* botAI)
    {
        return new TheCuratorMarkAstralFlareAction(botAI);
    }
    static Action* the_curator_tank_position_boss(PlayerbotAI* botAI)
    {
        return new TheCuratorTankPositionBossAction(botAI);
    }
    static Action* the_curator_spread_ranged(PlayerbotAI* botAI)
    {
        return new TheCuratorSpreadRangedAction(botAI);
    }

    // Terestian Illhoof
    static Action* terestian_illhoof_mark_target(PlayerbotAI* botAI)
    {
        return new TerestianIllhoofMarkTargetAction(botAI);
    }

    // Shade of Aran
    static Action* shade_of_aran_run_away_from_arcane_explosion(PlayerbotAI* botAI)
    {
        return new ShadeOfAranRunAwayFromArcaneExplosionAction(botAI);
    }
    static Action* shade_of_aran_move_behind_blizzard(PlayerbotAI* botAI)
    {
        return new ShadeOfAranMoveBehindBlizzardAction(botAI);
    }
    static Action* shade_of_aran_stop_moving(PlayerbotAI* botAI)
    {
        return new ShadeOfAranStopMovingAction(botAI);
    }
    static Action* shade_of_aran_mark_elemental(PlayerbotAI* botAI)
    {
        return new ShadeOfAranMarkElementalAction(botAI);
    }
    static Action* shade_of_aran_ranged_maintain_distance(PlayerbotAI* botAI)
    {
        return new ShadeOfAranRangedMaintainDistanceAction(botAI);
    }

    // Netherspite
    static Action* netherspite_block_red_beam(PlayerbotAI* botAI)
    {
        return new NetherspiteBlockRedBeamAction(botAI);
    }
    static Action* netherspite_block_blue_or_green_beam(PlayerbotAI* botAI)
    {
        return new NetherspiteBlockBlueOrGreenBeamAction(botAI);
    }
    static Action* netherspite_wait_to_block_beam(PlayerbotAI* botAI)
    {
        return new NetherspiteWaitToBlockBeamAction(botAI);
    }
    static Action* netherspite_move_between_blue_and_green_beams(PlayerbotAI* botAI)
    {
        return new NetherspiteMoveBetweenBlueAndGreenBeamsAction(botAI);
    }
    static Action* netherspite_move_to_banish_slot(PlayerbotAI* botAI)
    {
        return new NetherspiteMoveToBanishSlotAction(botAI);
    }
    static Action* netherspite_stop_attacking(PlayerbotAI* botAI)
    {
        return new NetherspiteStopAttackingAction(botAI);
    }
    static Action* netherspite_select_target_for_hold(PlayerbotAI* botAI)
    {
        return new NetherspiteSelectTargetForHoldAction(botAI);
    }
    static Action* netherspite_set_pet_stance(PlayerbotAI* botAI)
    {
        return new NetherspiteSetPetStanceAction(botAI);
    }
    static Action* netherspite_manage_timers_and_trackers(PlayerbotAI* botAI)
    {
        return new NetherspiteManageTimersAndTrackersAction(botAI);
    }

    // Prince Malchezaar
    static Action* prince_malchezaar_enfeebled_bot_run_to_safety(PlayerbotAI* botAI)
    {
        return new PrinceMalchezaarEnfeebledBotRunToSafetyAction(botAI);
    }
    static Action* prince_malchezaar_move_clear_of_hazards(PlayerbotAI* botAI)
    {
        return new PrinceMalchezaarMoveClearOfHazardsAction(botAI);
    }
    static Action* prince_malchezaar_tank_position_boss(PlayerbotAI* botAI)
    {
        return new PrinceMalchezaarTankPositionBossAction(botAI);
    }

    // Nightbane
    static Action* nightbane_tank_position_boss(PlayerbotAI* botAI)
    {
        return new NightbaneTankPositionBossAction(botAI);
    }
    static Action* nightbane_ranged_move_from_charred_earth(PlayerbotAI* botAI)
    {
        return new NightbaneRangedMoveFromCharredEarthAction(botAI);
    }
    static Action* nightbane_melee_move_from_charred_earth(PlayerbotAI* botAI)
    {
        return new NightbaneMeleeMoveFromCharredEarthAction(botAI);
    }
    static Action* nightbane_set_pet_stance(PlayerbotAI* botAI)
    {
        return new NightbaneSetPetStanceAction(botAI);
    }
    static Action* nightbane_stop_attacking(PlayerbotAI* botAI)
    {
        return new NightbaneStopAttackingAction(botAI);
    }
    static Action* nightbane_select_target_for_hold(PlayerbotAI* botAI)
    {
        return new NightbaneSelectTargetForHoldAction(botAI);
    }
    static Action* nightbane_stack_away_from_rain_of_bones(PlayerbotAI* botAI)
    {
        return new NightbaneStackAwayFromRainOfBonesAction(botAI);
    }
    static Action* nightbane_leave_combat_for_landing(PlayerbotAI* botAI)
    {
        return new NightbaneLeaveCombatForLandingAction(botAI);
    }
    static Action* nightbane_manage_timers_and_trackers(PlayerbotAI* botAI)
    {
        return new NightbaneManageTimersAndTrackersAction(botAI);
    }
};

#endif
