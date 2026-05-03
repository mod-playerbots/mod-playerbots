#ifndef _PLAYERBOT_RAIDTEMPESTKEEPTRIGGERCONTEXT_H
#define _PLAYERBOT_RAIDTEMPESTKEEPTRIGGERCONTEXT_H

#include "RaidTempestKeepTriggers.h"
#include "NamedObjectContext.h"

class RaidTempestKeepTriggerContext : public NamedObjectContext<Trigger>
{
public:
    RaidTempestKeepTriggerContext()
    {
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

        creators["al'ar phase 2 encounter is at room center"] =
            &RaidTempestKeepTriggerContext::alar_phase_2_encounter_is_at_room_center;

        creators["al'ar strategy changes between phases"] =
            &RaidTempestKeepTriggerContext::alar_strategy_changes_between_phases;

        // Void Reaver
        creators["void reaver boss casts pounding"] =
            &RaidTempestKeepTriggerContext::void_reaver_boss_casts_pounding;

        creators["void reaver knock away reduces tank aggro"] =
            &RaidTempestKeepTriggerContext::void_reaver_knock_away_reduces_tank_aggro;

        creators["void reaver boss launches arcane orbs"] =
            &RaidTempestKeepTriggerContext::void_reaver_boss_launches_arcane_orbs;

        creators["void reaver arcane orb is incoming"] =
            &RaidTempestKeepTriggerContext::void_reaver_arcane_orb_is_incoming;

        creators["void reaver bot is not in combat"] =
            &RaidTempestKeepTriggerContext::void_reaver_bot_is_not_in_combat;

        // High Astromancer Solarian
        creators["high astromancer solarian boss casts wrath of the astromancer"] =
            &RaidTempestKeepTriggerContext::high_astromancer_solarian_boss_casts_wrath_of_the_astromancer;

        creators["high astromancer solarian bot has wrath of the astromancer"] =
            &RaidTempestKeepTriggerContext::high_astromancer_solarian_bot_has_wrath_of_the_astromancer;

        creators["high astromancer solarian boss has vanished"] =
            &RaidTempestKeepTriggerContext::high_astromancer_solarian_boss_has_vanished;

        creators["high astromancer solarian solarium priests spawned"] =
            &RaidTempestKeepTriggerContext::high_astromancer_solarian_solarium_priests_spawned;

        creators["high astromancer solarian boss casts psychic scream"] =
            &RaidTempestKeepTriggerContext::high_astromancer_solarian_boss_casts_psychic_scream;

        // Kael'thas Sunstrider <Lord of the Blood Elves>
        creators["kael'thas sunstrider thaladred is fixated on bot"] =
            &RaidTempestKeepTriggerContext::kaelthas_sunstrider_thaladred_is_fixated_on_bot;

        creators["kael'thas sunstrider pulling tankable advisors"] =
            &RaidTempestKeepTriggerContext::kaelthas_sunstrider_pulling_tankable_advisors;

        creators["kael'thas sunstrider sanguinar engaged by main tank"] =
            &RaidTempestKeepTriggerContext::kaelthas_sunstrider_sanguinar_engaged_by_main_tank;

        creators["kael'thas sunstrider sanguinar casts bellowing roar"] =
            &RaidTempestKeepTriggerContext::kaelthas_sunstrider_sanguinar_casts_bellowing_roar;

        creators["kael'thas sunstrider capernian should be tanked by a warlock"] =
            &RaidTempestKeepTriggerContext::kaelthas_sunstrider_capernian_should_be_tanked_by_a_warlock;

        creators["kael'thas sunstrider capernian casts arcane burst and conflagration"] =
            &RaidTempestKeepTriggerContext::kaelthas_sunstrider_capernian_casts_arcane_burst_and_conflagration;

        creators["kael'thas sunstrider telonicus engaged by first assist tank"] =
            &RaidTempestKeepTriggerContext::kaelthas_sunstrider_telonicus_engaged_by_first_assist_tank;

        creators["kael'thas sunstrider bots have specific roles in phase 3"] =
            &RaidTempestKeepTriggerContext::kaelthas_sunstrider_bots_have_specific_roles_in_phase_3;

        creators["kael'thas sunstrider determining advisor kill order"] =
            &RaidTempestKeepTriggerContext::kaelthas_sunstrider_determining_advisor_kill_order;

        creators["kael'thas sunstrider waiting for tanks to get aggro on advisors"] =
            &RaidTempestKeepTriggerContext::kaelthas_sunstrider_waiting_for_tanks_to_get_aggro_on_advisors;

        creators["kael'thas sunstrider legendary weapons are alive"] =
            &RaidTempestKeepTriggerContext::kaelthas_sunstrider_legendary_weapons_are_alive;

        creators["kael'thas sunstrider legendary axe casts whirlwind"] =
            &RaidTempestKeepTriggerContext::kaelthas_sunstrider_legendary_axe_casts_whirlwind;

        creators["kael'thas sunstrider legendary weapons are dead and lootable"] =
            &RaidTempestKeepTriggerContext::kaelthas_sunstrider_legendary_weapons_are_dead_and_lootable;

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

        creators["kael'thas sunstrider boss is casting pyroblast"] =
            &RaidTempestKeepTriggerContext::kaelthas_sunstrider_boss_is_casting_pyroblast;

        creators["kael'thas sunstrider boss is manipulating gravity"] =
            &RaidTempestKeepTriggerContext::kaelthas_sunstrider_boss_is_manipulating_gravity;
    }

private:
    // Trash
    static Trigger* crimson_hand_centurion_casts_arcane_volley(
        PlayerbotAI* botAI) { return new CrimsonHandCenturionCastsArcaneVolleyTrigger(botAI); }

    // Al'ar <Phoenix God>
    static Trigger* alar_pulling_boss(
        PlayerbotAI* botAI) { return new AlarPullingBossTrigger(botAI); }

    static Trigger* alar_boss_is_flying_between_platforms(
        PlayerbotAI* botAI) { return new AlarBossIsFlyingBetweenPlatformsTrigger(botAI); }

    static Trigger* alar_embers_of_alar_explode_upon_death(
        PlayerbotAI* botAI) { return new AlarEmbersOfAlarExplodeUponDeathTrigger(botAI); }

    static Trigger* alar_killing_embers_of_alar_damages_boss(
        PlayerbotAI* botAI) { return new AlarKillingEmbersOfAlarDamagesBossTrigger(botAI); }

    static Trigger* alar_incoming_flame_quills(
        PlayerbotAI* botAI) { return new AlarIncomingFlameQuillsTrigger(botAI); }

    static Trigger* alar_rising_from_the_ashes(
        PlayerbotAI* botAI) { return new AlarRisingFromTheAshesTrigger(botAI); }

    static Trigger* alar_everything_is_on_fire_in_phase_2(
        PlayerbotAI* botAI) { return new AlarEverythingIsOnFireInPhase2Trigger(botAI); }

    static Trigger* alar_phase_2_encounter_is_at_room_center(
        PlayerbotAI* botAI) { return new AlarPhase2EncounterIsAtRoomCenterTrigger(botAI); }

    static Trigger* alar_strategy_changes_between_phases(
        PlayerbotAI* botAI) { return new AlarStrategyChangesBetweenPhasesTrigger(botAI); }

    // Void Reaver
    static Trigger* void_reaver_boss_casts_pounding(
        PlayerbotAI* botAI) { return new VoidReaverBossCastsPoundingTrigger(botAI); }

    static Trigger* void_reaver_knock_away_reduces_tank_aggro(
        PlayerbotAI* botAI) { return new VoidReaverKnockAwayReducesTankAggroTrigger(botAI); }

    static Trigger* void_reaver_boss_launches_arcane_orbs(
        PlayerbotAI* botAI) { return new VoidReaverBossLaunchesArcaneOrbsTrigger(botAI); }

    static Trigger* void_reaver_arcane_orb_is_incoming(
        PlayerbotAI* botAI) { return new VoidReaverArcaneOrbIsIncomingTrigger(botAI); }

    static Trigger* void_reaver_bot_is_not_in_combat(
        PlayerbotAI* botAI) { return new VoidReaverBotIsNotInCombatTrigger(botAI); }

    // High Astromancer Solarian
    static Trigger* high_astromancer_solarian_boss_casts_wrath_of_the_astromancer(
        PlayerbotAI* botAI) { return new HighAstromancerSolarianBossCastsWrathOfTheAstromancerTrigger(botAI); }

    static Trigger* high_astromancer_solarian_bot_has_wrath_of_the_astromancer(
        PlayerbotAI* botAI) { return new HighAstromancerSolarianBotHasWrathOfTheAstromancerTrigger(botAI); }

    static Trigger* high_astromancer_solarian_boss_has_vanished(
        PlayerbotAI* botAI) { return new HighAstromancerSolarianBossHasVanishedTrigger(botAI); }

    static Trigger* high_astromancer_solarian_solarium_priests_spawned(
        PlayerbotAI* botAI) { return new HighAstromancerSolarianSolariumPriestsSpawnedTrigger(botAI); }

    static Trigger* high_astromancer_solarian_boss_casts_psychic_scream(
        PlayerbotAI* botAI) { return new HighAstromancerSolarianBossCastsPsychicScreamTrigger(botAI); }

    // Kael'thas Sunstrider <Lord of the Blood Elves>
    static Trigger* kaelthas_sunstrider_thaladred_is_fixated_on_bot(
        PlayerbotAI* botAI) { return new KaelthasSunstriderThaladredIsFixatedOnBotTrigger(botAI); }

    static Trigger* kaelthas_sunstrider_pulling_tankable_advisors(
        PlayerbotAI* botAI) { return new KaelthasSunstriderPullingTankableAdvisorsTrigger(botAI); }

    static Trigger* kaelthas_sunstrider_sanguinar_engaged_by_main_tank(
        PlayerbotAI* botAI) { return new KaelthasSunstriderSanguinarEngagedByMainTankTrigger(botAI); }

    static Trigger* kaelthas_sunstrider_sanguinar_casts_bellowing_roar(
        PlayerbotAI* botAI) { return new KaelthasSunstriderSanguinarCastsBellowingRoarTrigger(botAI); }

    static Trigger* kaelthas_sunstrider_capernian_should_be_tanked_by_a_warlock(
        PlayerbotAI* botAI) { return new KaelthasSunstriderCapernianShouldBeTankedByAWarlockTrigger(botAI); }

    static Trigger* kaelthas_sunstrider_capernian_casts_arcane_burst_and_conflagration(
        PlayerbotAI* botAI) { return new KaelthasSunstriderCapernianCastsArcaneBurstAndConflagrationTrigger(botAI); }

    static Trigger* kaelthas_sunstrider_telonicus_engaged_by_first_assist_tank(
        PlayerbotAI* botAI) { return new KaelthasSunstriderTelonicusEngagedByFirstAssistTankTrigger(botAI); }

    static Trigger* kaelthas_sunstrider_bots_have_specific_roles_in_phase_3(
        PlayerbotAI* botAI) { return new KaelthasSunstriderBotsHaveSpecificRolesInPhase3Trigger(botAI); }

    static Trigger* kaelthas_sunstrider_determining_advisor_kill_order(
        PlayerbotAI* botAI) { return new KaelthasSunstriderDeterminingAdvisorKillOrderTrigger(botAI); }

    static Trigger* kaelthas_sunstrider_waiting_for_tanks_to_get_aggro_on_advisors(
        PlayerbotAI* botAI) { return new KaelthasSunstriderWaitingForTanksToGetAggroOnAdvisorsTrigger(botAI); }

    static Trigger* kaelthas_sunstrider_legendary_weapons_are_alive(
        PlayerbotAI* botAI) { return new KaelthasSunstriderLegendaryWeaponsAreAliveTrigger(botAI); }

    static Trigger* kaelthas_sunstrider_legendary_axe_casts_whirlwind(
        PlayerbotAI* botAI) { return new KaelthasSunstriderLegendaryAxeCastsWhirlwindTrigger(botAI); }

    static Trigger* kaelthas_sunstrider_legendary_weapons_are_dead_and_lootable(
        PlayerbotAI* botAI) { return new KaelthasSunstriderLegendaryWeaponsAreDeadAndLootableTrigger(botAI); }

    static Trigger* kaelthas_sunstrider_legendary_weapons_are_equipped(
        PlayerbotAI* botAI) { return new KaelthasSunstriderLegendaryWeaponsAreEquippedTrigger(botAI); }

    static Trigger* kaelthas_sunstrider_legendary_weapons_were_lost(
        PlayerbotAI* botAI) { return new KaelthasSunstriderLegendaryWeaponsWereLostTrigger(botAI); }

    static Trigger* kaelthas_sunstrider_boss_has_entered_the_fight(
        PlayerbotAI* botAI) { return new KaelthasSunstriderBossHasEnteredTheFightTrigger(botAI); }

    static Trigger* kaelthas_sunstrider_raid_member_is_mind_controlled(
        PlayerbotAI* botAI) { return new KaelthasSunstriderRaidMemberIsMindControlledTrigger(botAI); }

    static Trigger* kaelthas_sunstrider_phoenixes_and_eggs_are_spawning(
        PlayerbotAI* botAI) { return new KaelthasSunstriderPhoenixesAndEggsAreSpawningTrigger(botAI); }

    static Trigger* kaelthas_sunstrider_boss_is_casting_pyroblast(
        PlayerbotAI* botAI) { return new KaelthasSunstriderBossIsCastingPyroblastTrigger(botAI); }

    static Trigger* kaelthas_sunstrider_boss_is_manipulating_gravity(
        PlayerbotAI* botAI) { return new KaelthasSunstriderBossIsManipulatingGravityTrigger(botAI); }
};

#endif
