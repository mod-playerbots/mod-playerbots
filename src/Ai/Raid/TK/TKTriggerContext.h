/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_TKTRIGGERCONTEXT_H
#define PLAYERBOTS_TKTRIGGERCONTEXT_H

#include "NamedObjectContext.h"
#include "TKTriggers.h"

class RaidTempestKeepTriggerContext : public NamedObjectContext<Trigger>
{
public:
    RaidTempestKeepTriggerContext()
    {
        // General
        creators["tempest keep bot is not in combat"] =
            &RaidTempestKeepTriggerContext::tempest_keep_bot_is_not_in_combat;

        // Trash
        creators["crimson hand centurion casts arcane volley"] =
            &RaidTempestKeepTriggerContext::crimson_hand_centurion_casts_arcane_volley;

        // Al'ar <Phoenix God>
        creators["al'ar pulling boss"] =
            &RaidTempestKeepTriggerContext::alar_pulling_boss;

        creators["al'ar boss is flying between platforms"] =
            &RaidTempestKeepTriggerContext::alar_boss_is_flying_between_platforms;

        creators["al'ar embers of al'ar explode upon death"] =
            &RaidTempestKeepTriggerContext::alar_embers_of_alar_explode_upon_death;

        creators["al'ar killing embers of al'ar damages boss"] =
            &RaidTempestKeepTriggerContext::alar_killing_embers_of_alar_damages_boss;

        creators["al'ar incoming flame quills"] =
            &RaidTempestKeepTriggerContext::alar_incoming_flame_quills;

        creators["al'ar rising from the ashes"] =
            &RaidTempestKeepTriggerContext::alar_rising_from_the_ashes;

        creators["al'ar everything is on fire in phase 2"] =
            &RaidTempestKeepTriggerContext::alar_everything_is_on_fire_in_phase_2;

        creators["al'ar should manage phase tracker"] =
            &RaidTempestKeepTriggerContext::alar_should_manage_phase_tracker;

        // Void Reaver
        creators["void reaver boss casts pounding"] =
            &RaidTempestKeepTriggerContext::void_reaver_boss_casts_pounding;

        creators["void reaver knock away reduces tank aggro"] =
            &RaidTempestKeepTriggerContext::void_reaver_knock_away_reduces_tank_aggro;

        creators["void reaver ranged should stand back"] =
            &RaidTempestKeepTriggerContext::void_reaver_ranged_should_stand_back;

        creators["void reaver arcane orb is incoming"] =
            &RaidTempestKeepTriggerContext::void_reaver_arcane_orb_is_incoming;

        // High Astromancer Solarian
        creators["high astromancer solarian engaged by main tank"] =
            &RaidTempestKeepTriggerContext::high_astromancer_solarian_engaged_by_main_tank;

        creators["high astromancer solarian bot has wrath of the astromancer"] =
            &RaidTempestKeepTriggerContext::high_astromancer_solarian_bot_has_wrath_of_the_astromancer;

        creators["high astromancer solarian solarium priests spawned"] =
            &RaidTempestKeepTriggerContext::high_astromancer_solarian_solarium_priests_spawned;

        creators["high astromancer solarian boss casts psychic scream"] =
            &RaidTempestKeepTriggerContext::high_astromancer_solarian_boss_casts_psychic_scream;

        // Kael'thas Sunstrider <Lord of the Blood Elves>
        creators["kael'thas sunstrider thaladred is fixated on bot"] =
            &RaidTempestKeepTriggerContext::kaelthas_sunstrider_thaladred_is_fixated_on_bot;

        creators["kael'thas sunstrider pulling tankable advisors"] =
            &RaidTempestKeepTriggerContext::kaelthas_sunstrider_pulling_tankable_advisors;

        creators["kael'thas sunstrider sanguinar or telonicus is active"] =
            &RaidTempestKeepTriggerContext::kaelthas_sunstrider_sanguinar_or_telonicus_is_active;

        creators["kael'thas sunstrider sanguinar casts bellowing roar"] =
            &RaidTempestKeepTriggerContext::kaelthas_sunstrider_sanguinar_casts_bellowing_roar;

        creators["kael'thas sunstrider capernian should be tanked by warlock"] =
            &RaidTempestKeepTriggerContext::kaelthas_sunstrider_capernian_should_be_tanked_by_warlock;

        creators["kael'thas sunstrider capernian blows up near and far"] =
            &RaidTempestKeepTriggerContext::kaelthas_sunstrider_capernian_blows_up_near_and_far;

        creators["kael'thas sunstrider bots have specific roles in phase 3"] =
            &RaidTempestKeepTriggerContext::kaelthas_sunstrider_bots_have_specific_roles_in_phase_3;

        creators["kael'thas sunstrider determining advisor kill order"] =
            &RaidTempestKeepTriggerContext::kaelthas_sunstrider_determining_advisor_kill_order;

        creators["kael'thas sunstrider should manage advisor dps timer"] =
            &RaidTempestKeepTriggerContext::kaelthas_sunstrider_should_manage_advisor_dps_timer;

        creators["kael'thas sunstrider legendary weapons are alive"] =
            &RaidTempestKeepTriggerContext::kaelthas_sunstrider_legendary_weapons_are_alive;

        creators["kael'thas sunstrider legendary axe casts whirlwind"] =
            &RaidTempestKeepTriggerContext::kaelthas_sunstrider_legendary_axe_casts_whirlwind;

        creators["kael'thas sunstrider legendary weapons are dead"] =
            &RaidTempestKeepTriggerContext::kaelthas_sunstrider_legendary_weapons_are_dead;

        creators["kael'thas sunstrider legendary weapons are equipped"] =
            &RaidTempestKeepTriggerContext::kaelthas_sunstrider_legendary_weapons_are_equipped;

        creators["kael'thas sunstrider legendary weapons were lost"] =
            &RaidTempestKeepTriggerContext::kaelthas_sunstrider_legendary_weapons_were_lost;

        creators["kael'thas sunstrider boss has entered the fight"] =
            &RaidTempestKeepTriggerContext::kaelthas_sunstrider_boss_has_entered_the_fight;

        creators["kael'thas sunstrider raid member is mind controlled"] =
            &RaidTempestKeepTriggerContext::kaelthas_sunstrider_raid_member_is_mind_controlled;

        creators["kael'thas sunstrider phoenixes and eggs are spawning"] =
            &RaidTempestKeepTriggerContext::kaelthas_sunstrider_phoenixes_and_eggs_are_spawning;

        creators["kael'thas sunstrider boss is manipulating gravity"] =
            &RaidTempestKeepTriggerContext::kaelthas_sunstrider_boss_is_manipulating_gravity;
    }

private:
    // General
    static Trigger* tempest_keep_bot_is_not_in_combat(PlayerbotAI* botAI) {
        return new TempestKeepBotIsNotInCombatTrigger(botAI);
    }

    // Trash
    static Trigger* crimson_hand_centurion_casts_arcane_volley(PlayerbotAI* botAI) {
        return new CrimsonHandCenturionCastsArcaneVolleyTrigger(botAI);
    }

    // Al'ar <Phoenix God>
    static Trigger* alar_pulling_boss(PlayerbotAI* botAI) {
        return new AlarPullingBossTrigger(botAI);
    }
    static Trigger* alar_boss_is_flying_between_platforms(PlayerbotAI* botAI) {
        return new AlarBossIsFlyingBetweenPlatformsTrigger(botAI);
    }
    static Trigger* alar_embers_of_alar_explode_upon_death(PlayerbotAI* botAI) {
        return new AlarEmbersOfAlarExplodeUponDeathTrigger(botAI);
    }
    static Trigger* alar_killing_embers_of_alar_damages_boss(PlayerbotAI* botAI) {
        return new AlarKillingEmbersOfAlarDamagesBossTrigger(botAI);
    }
    static Trigger* alar_incoming_flame_quills(PlayerbotAI* botAI) {
        return new AlarIncomingFlameQuillsTrigger(botAI);
    }
    static Trigger* alar_rising_from_the_ashes(PlayerbotAI* botAI) {
        return new AlarRisingFromTheAshesTrigger(botAI);
    }
    static Trigger* alar_everything_is_on_fire_in_phase_2(PlayerbotAI* botAI) {
        return new AlarEverythingIsOnFireInPhase2Trigger(botAI);
    }
    static Trigger* alar_should_manage_phase_tracker(PlayerbotAI* botAI) {
        return new AlarShouldManagePhaseTrackerTrigger(botAI);
    }

    // Void Reaver
    static Trigger* void_reaver_boss_casts_pounding(PlayerbotAI* botAI) {
        return new VoidReaverBossCastsPoundingTrigger(botAI);
    }
    static Trigger* void_reaver_knock_away_reduces_tank_aggro(PlayerbotAI* botAI) {
        return new VoidReaverKnockAwayReducesTankAggroTrigger(botAI);
    }
    static Trigger* void_reaver_ranged_should_stand_back(PlayerbotAI* botAI) {
        return new VoidReaverRangedShouldStandBackTrigger(botAI);
    }
    static Trigger* void_reaver_arcane_orb_is_incoming(PlayerbotAI* botAI) {
        return new VoidReaverArcaneOrbIsIncomingTrigger(botAI);
    }

    // High Astromancer Solarian
    static Trigger* high_astromancer_solarian_engaged_by_main_tank(PlayerbotAI* botAI) {
        return new HighAstromancerSolarianEngagedByMainTankTrigger(botAI);
    }
    static Trigger* high_astromancer_solarian_bot_has_wrath_of_the_astromancer(PlayerbotAI* botAI) {
        return new HighAstromancerSolarianBotHasWrathOfTheAstromancerTrigger(botAI);
    }
    static Trigger* high_astromancer_solarian_solarium_priests_spawned(PlayerbotAI* botAI) {
        return new HighAstromancerSolarianSolariumPriestsSpawnedTrigger(botAI);
    }
    static Trigger* high_astromancer_solarian_boss_casts_psychic_scream(PlayerbotAI* botAI) {
        return new HighAstromancerSolarianBossCastsPsychicScreamTrigger(botAI);
    }

    // Kael'thas Sunstrider <Lord of the Blood Elves>
    static Trigger* kaelthas_sunstrider_thaladred_is_fixated_on_bot(PlayerbotAI* botAI) {
        return new KaelthasSunstriderThaladredIsFixatedOnBotTrigger(botAI);
    }
    static Trigger* kaelthas_sunstrider_pulling_tankable_advisors(PlayerbotAI* botAI) {
        return new KaelthasSunstriderPullingTankableAdvisorsTrigger(botAI);
    }
    static Trigger* kaelthas_sunstrider_sanguinar_or_telonicus_is_active(PlayerbotAI* botAI) {
        return new KaelthasSunstriderSanguinarOrTelonicusIsActiveTrigger(botAI);
    }
    static Trigger* kaelthas_sunstrider_sanguinar_casts_bellowing_roar(PlayerbotAI* botAI) {
        return new KaelthasSunstriderSanguinarCastsBellowingRoarTrigger(botAI);
    }
    static Trigger* kaelthas_sunstrider_capernian_should_be_tanked_by_warlock(PlayerbotAI* botAI) {
        return new KaelthasSunstriderCapernianShouldBeTankedByWarlockTrigger(botAI);
    }
    static Trigger* kaelthas_sunstrider_capernian_blows_up_near_and_far(PlayerbotAI* botAI) {
        return new KaelthasSunstriderCapernianBlowsUpNearAndFarTrigger(botAI);
    }
    static Trigger* kaelthas_sunstrider_bots_have_specific_roles_in_phase_3(PlayerbotAI* botAI) {
        return new KaelthasSunstriderBotsHaveSpecificRolesInPhase3Trigger(botAI);
    }
    static Trigger* kaelthas_sunstrider_determining_advisor_kill_order(PlayerbotAI* botAI) {
        return new KaelthasSunstriderDeterminingAdvisorKillOrderTrigger(botAI);
    }
    static Trigger* kaelthas_sunstrider_should_manage_advisor_dps_timer(PlayerbotAI* botAI) {
        return new KaelthasSunstriderShouldManageAdvisorDpsTimerTrigger(botAI);
    }
    static Trigger* kaelthas_sunstrider_legendary_weapons_are_alive(PlayerbotAI* botAI) {
        return new KaelthasSunstriderLegendaryWeaponsAreAliveTrigger(botAI);
    }
    static Trigger* kaelthas_sunstrider_legendary_axe_casts_whirlwind(PlayerbotAI* botAI) {
        return new KaelthasSunstriderLegendaryAxeCastsWhirlwindTrigger(botAI);
    }
    static Trigger* kaelthas_sunstrider_legendary_weapons_are_dead(PlayerbotAI* botAI) {
        return new KaelthasSunstriderLegendaryWeaponsAreDeadTrigger(botAI);
    }
    static Trigger* kaelthas_sunstrider_legendary_weapons_are_equipped(PlayerbotAI* botAI) {
        return new KaelthasSunstriderLegendaryWeaponsAreEquippedTrigger(botAI);
    }
    static Trigger* kaelthas_sunstrider_legendary_weapons_were_lost(PlayerbotAI* botAI) {
        return new KaelthasSunstriderLegendaryWeaponsWereLostTrigger(botAI);
    }
    static Trigger* kaelthas_sunstrider_boss_has_entered_the_fight(PlayerbotAI* botAI) {
        return new KaelthasSunstriderBossHasEnteredTheFightTrigger(botAI);
    }
    static Trigger* kaelthas_sunstrider_raid_member_is_mind_controlled(PlayerbotAI* botAI) {
        return new KaelthasSunstriderRaidMemberIsMindControlledTrigger(botAI);
    }
    static Trigger* kaelthas_sunstrider_phoenixes_and_eggs_are_spawning(PlayerbotAI* botAI) {
        return new KaelthasSunstriderPhoenixesAndEggsAreSpawningTrigger(botAI);
    }
    static Trigger* kaelthas_sunstrider_boss_is_manipulating_gravity(PlayerbotAI* botAI) {
        return new KaelthasSunstriderBossIsManipulatingGravityTrigger(botAI);
    }
};

#endif
