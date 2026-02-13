#ifndef _PLAYERBOT_RAIDSSCTRIGGERCONTEXT_H
#define _PLAYERBOT_RAIDSSCTRIGGERCONTEXT_H

#include "RaidSSCTriggers.h"
#include "AiObjectContext.h"

class RaidSSCTriggerContext : public NamedObjectContext<Trigger>
{
public:
    RaidSSCTriggerContext()
    {
        // General
        creators["serpent shrine cavern bot is not in combat"] =
            &RaidSSCTriggerContext::serpent_shrine_cavern_bot_is_not_in_combat;

        // Trash
        creators["underbog colossus spawned toxic pool after death"] =
            &RaidSSCTriggerContext::underbog_colossus_spawned_toxic_pool_after_death;

        creators["greyheart tidecaller water elemental totem spawned"] =
            &RaidSSCTriggerContext::greyheart_tidecaller_water_elemental_totem_spawned;

        // Hydross the Unstable <Duke of Currents>
        creators["hydross the unstable bot is frost tank"] =
            &RaidSSCTriggerContext::hydross_the_unstable_bot_is_frost_tank;

        creators["hydross the unstable bot is nature tank"] =
            &RaidSSCTriggerContext::hydross_the_unstable_bot_is_nature_tank;

        creators["hydross the unstable elementals spawned"] =
            &RaidSSCTriggerContext::hydross_the_unstable_elementals_spawned;

        creators["hydross the unstable danger from water tombs"] =
            &RaidSSCTriggerContext::hydross_the_unstable_danger_from_water_tombs;

        creators["hydross the unstable tank needs aggro upon phase change"] =
            &RaidSSCTriggerContext::hydross_the_unstable_tank_needs_aggro_upon_phase_change;

        creators["hydross the unstable aggro resets upon phase change"] =
            &RaidSSCTriggerContext::hydross_the_unstable_aggro_resets_upon_phase_change;

        creators["hydross the unstable need to manage timers"] =
            &RaidSSCTriggerContext::hydross_the_unstable_need_to_manage_timers;

        // The Lurker Below
        creators["the lurker below spout is active"] =
            &RaidSSCTriggerContext::the_lurker_below_spout_is_active;

        creators["the lurker below boss is active for main tank"] =
            &RaidSSCTriggerContext::the_lurker_below_boss_is_active_for_main_tank;

        creators["the lurker below boss casts geyser"] =
            &RaidSSCTriggerContext::the_lurker_below_boss_casts_geyser;

        creators["the lurker below boss is submerged"] =
            &RaidSSCTriggerContext::the_lurker_below_boss_is_submerged;

        creators["the lurker below need to prepare timer for spout"] =
            &RaidSSCTriggerContext::the_lurker_below_need_to_prepare_timer_for_spout;

        // Leotheras the Blind
        creators["leotheras the blind boss is inactive"] =
            &RaidSSCTriggerContext::leotheras_the_blind_boss_is_inactive;

        creators["leotheras the blind boss transformed into demon form"] =
            &RaidSSCTriggerContext::leotheras_the_blind_boss_transformed_into_demon_form;

        creators["leotheras the blind only warlock should tank demon form"] =
            &RaidSSCTriggerContext::leotheras_the_blind_only_warlock_should_tank_demon_form;

        creators["leotheras the blind boss engaged by ranged"] =
            &RaidSSCTriggerContext::leotheras_the_blind_boss_engaged_by_ranged;

        creators["leotheras the blind boss channeling whirlwind"] =
            &RaidSSCTriggerContext::leotheras_the_blind_boss_channeling_whirlwind;

        creators["leotheras the blind bot has too many chaos blast stacks"] =
            &RaidSSCTriggerContext::leotheras_the_blind_bot_has_too_many_chaos_blast_stacks;

        creators["leotheras the blind inner demon has awakened"] =
            &RaidSSCTriggerContext::leotheras_the_blind_inner_demon_has_awakened;

        creators["leotheras the blind entered final phase"] =
            &RaidSSCTriggerContext::leotheras_the_blind_entered_final_phase;

        creators["leotheras the blind demon form tank needs aggro"] =
            &RaidSSCTriggerContext::leotheras_the_blind_demon_form_tank_needs_aggro;

        creators["leotheras the blind boss wipes aggro upon phase change"] =
            &RaidSSCTriggerContext::leotheras_the_blind_boss_wipes_aggro_upon_phase_change;

        // Fathom-Lord Karathress
        creators["fathom-lord karathress boss engaged by main tank"] =
            &RaidSSCTriggerContext::fathom_lord_karathress_boss_engaged_by_main_tank;

        creators["fathom-lord karathress caribdis engaged by first assist tank"] =
            &RaidSSCTriggerContext::fathom_lord_karathress_caribdis_engaged_by_first_assist_tank;

        creators["fathom-lord karathress sharkkis engaged by second assist tank"] =
            &RaidSSCTriggerContext::fathom_lord_karathress_sharkkis_engaged_by_second_assist_tank;

        creators["fathom-lord karathress tidalvess engaged by third assist tank"] =
            &RaidSSCTriggerContext::fathom_lord_karathress_tidalvess_engaged_by_third_assist_tank;

        creators["fathom-lord karathress caribdis tank needs dedicated healer"] =
            &RaidSSCTriggerContext::fathom_lord_karathress_caribdis_tank_needs_dedicated_healer;

        creators["fathom-lord karathress pulling bosses"] =
            &RaidSSCTriggerContext::fathom_lord_karathress_pulling_bosses;

        creators["fathom-lord karathress determining kill order"] =
            &RaidSSCTriggerContext::fathom_lord_karathress_determining_kill_order;

        creators["fathom-lord karathress tanks need to establish aggro"] =
            &RaidSSCTriggerContext::fathom_lord_karathress_tanks_need_to_establish_aggro;

        // Morogrim Tidewalker
        creators["morogrim tidewalker boss engaged by main tank"] =
            &RaidSSCTriggerContext::morogrim_tidewalker_boss_engaged_by_main_tank;

        creators["morogrim tidewalker pulling boss"] =
            &RaidSSCTriggerContext::morogrim_tidewalker_pulling_boss;

        creators["morogrim tidewalker water globules are incoming"] =
            &RaidSSCTriggerContext::morogrim_tidewalker_water_globules_are_incoming;

        // Lady Vashj <Coilfang Matron>
        creators["lady vashj boss engaged by main tank"] =
            &RaidSSCTriggerContext::lady_vashj_boss_engaged_by_main_tank;

        creators["lady vashj boss engaged by ranged in phase 1"] =
            &RaidSSCTriggerContext::lady_vashj_boss_engaged_by_ranged_in_phase_1;

        creators["lady vashj casts shock blast on highest aggro"] =
            &RaidSSCTriggerContext::lady_vashj_casts_shock_blast_on_highest_aggro;

        creators["lady vashj bot has static charge"] =
            &RaidSSCTriggerContext::lady_vashj_bot_has_static_charge;

        creators["lady vashj pulling boss in phase 1 and phase 3"] =
            &RaidSSCTriggerContext::lady_vashj_pulling_boss_in_phase_1_and_phase_3;

        creators["lady vashj adds spawn in phase 2 and phase 3"] =
            &RaidSSCTriggerContext::lady_vashj_adds_spawn_in_phase_2_and_phase_3;

        creators["lady vashj coilfang strider is approaching"] =
            &RaidSSCTriggerContext::lady_vashj_coilfang_strider_is_approaching;

        creators["lady vashj tainted elemental cheat"] =
            &RaidSSCTriggerContext::lady_vashj_tainted_elemental_cheat;

        creators["lady vashj tainted core was looted"] =
            &RaidSSCTriggerContext::lady_vashj_tainted_core_was_looted;

        creators["lady vashj tainted core is unusable"] =
            &RaidSSCTriggerContext::lady_vashj_tainted_core_is_unusable;

        creators["lady vashj need to reset core passing trackers"] =
            &RaidSSCTriggerContext::lady_vashj_need_to_reset_core_passing_trackers;

        creators["lady vashj toxic sporebats are spewing poison clouds"] =
            &RaidSSCTriggerContext::lady_vashj_toxic_sporebats_are_spewing_poison_clouds;

        creators["lady vashj bot is entangled in toxic spores or static charge"] =
            &RaidSSCTriggerContext::lady_vashj_bot_is_entangled_in_toxic_spores_or_static_charge;
    }

private:
    // General
    static Trigger* serpent_shrine_cavern_bot_is_not_in_combat(
        PlayerbotAI* botAI) { return new SerpentShrineCavernBotIsNotInCombatTrigger(botAI); }

    // Trash
    static Trigger* underbog_colossus_spawned_toxic_pool_after_death(
        PlayerbotAI* botAI) { return new UnderbogColossusSpawnedToxicPoolAfterDeathTrigger(botAI); }

    static Trigger* greyheart_tidecaller_water_elemental_totem_spawned(
        PlayerbotAI* botAI) { return new GreyheartTidecallerWaterElementalTotemSpawnedTrigger(botAI); }

    // Hydross the Unstable <Duke of Currents>
    static Trigger* hydross_the_unstable_bot_is_frost_tank(
        PlayerbotAI* botAI) { return new HydrossTheUnstableBotIsFrostTankTrigger(botAI); }

    static Trigger* hydross_the_unstable_bot_is_nature_tank(
        PlayerbotAI* botAI) { return new HydrossTheUnstableBotIsNatureTankTrigger(botAI); }

    static Trigger* hydross_the_unstable_elementals_spawned(
        PlayerbotAI* botAI) { return new HydrossTheUnstableElementalsSpawnedTrigger(botAI); }

    static Trigger* hydross_the_unstable_danger_from_water_tombs(
        PlayerbotAI* botAI) { return new HydrossTheUnstableDangerFromWaterTombsTrigger(botAI); }

    static Trigger* hydross_the_unstable_tank_needs_aggro_upon_phase_change(
        PlayerbotAI* botAI) { return new HydrossTheUnstableTankNeedsAggroUponPhaseChangeTrigger(botAI); }

    static Trigger* hydross_the_unstable_aggro_resets_upon_phase_change(
        PlayerbotAI* botAI) { return new HydrossTheUnstableAggroResetsUponPhaseChangeTrigger(botAI); }

    static Trigger* hydross_the_unstable_need_to_manage_timers(
        PlayerbotAI* botAI) { return new HydrossTheUnstableNeedToManageTimersTrigger(botAI); }

    // The Lurker Below
    static Trigger* the_lurker_below_spout_is_active(
        PlayerbotAI* botAI) { return new TheLurkerBelowSpoutIsActiveTrigger(botAI); }

    static Trigger* the_lurker_below_boss_is_active_for_main_tank(
        PlayerbotAI* botAI) { return new TheLurkerBelowBossIsActiveForMainTankTrigger(botAI); }

    static Trigger* the_lurker_below_boss_casts_geyser(
        PlayerbotAI* botAI) { return new TheLurkerBelowBossCastsGeyserTrigger(botAI); }

    static Trigger* the_lurker_below_boss_is_submerged(
        PlayerbotAI* botAI) { return new TheLurkerBelowBossIsSubmergedTrigger(botAI); }

    static Trigger* the_lurker_below_need_to_prepare_timer_for_spout(
        PlayerbotAI* botAI) { return new TheLurkerBelowNeedToPrepareTimerForSpoutTrigger(botAI); }

    // Leotheras the Blind
    static Trigger* leotheras_the_blind_boss_is_inactive(
        PlayerbotAI* botAI) { return new LeotherasTheBlindBossIsInactiveTrigger(botAI); }

    static Trigger* leotheras_the_blind_boss_transformed_into_demon_form(
        PlayerbotAI* botAI) { return new LeotherasTheBlindBossTransformedIntoDemonFormTrigger(botAI); }

    static Trigger* leotheras_the_blind_only_warlock_should_tank_demon_form(
        PlayerbotAI* botAI) { return new LeotherasTheBlindOnlyWarlockShouldTankDemonFormTrigger(botAI); }

    static Trigger* leotheras_the_blind_boss_engaged_by_ranged(
        PlayerbotAI* botAI) { return new LeotherasTheBlindBossEngagedByRangedTrigger(botAI); }

    static Trigger* leotheras_the_blind_boss_channeling_whirlwind(
        PlayerbotAI* botAI) { return new LeotherasTheBlindBossChannelingWhirlwindTrigger(botAI); }

    static Trigger* leotheras_the_blind_bot_has_too_many_chaos_blast_stacks(
        PlayerbotAI* botAI) { return new LeotherasTheBlindBotHasTooManyChaosBlastStacksTrigger(botAI); }

    static Trigger* leotheras_the_blind_inner_demon_has_awakened(
        PlayerbotAI* botAI) { return new LeotherasTheBlindInnerDemonHasAwakenedTrigger(botAI); }

    static Trigger* leotheras_the_blind_entered_final_phase(
        PlayerbotAI* botAI) { return new LeotherasTheBlindEnteredFinalPhaseTrigger(botAI); }

    static Trigger* leotheras_the_blind_demon_form_tank_needs_aggro(
        PlayerbotAI* botAI) { return new LeotherasTheBlindDemonFormTankNeedsAggro(botAI); }

    static Trigger* leotheras_the_blind_boss_wipes_aggro_upon_phase_change(
        PlayerbotAI* botAI) { return new LeotherasTheBlindBossWipesAggroUponPhaseChangeTrigger(botAI); }

    // Fathom-Lord Karathress
    static Trigger* fathom_lord_karathress_boss_engaged_by_main_tank(
        PlayerbotAI* botAI) { return new FathomLordKarathressBossEngagedByMainTankTrigger(botAI); }

    static Trigger* fathom_lord_karathress_caribdis_engaged_by_first_assist_tank(
        PlayerbotAI* botAI) { return new FathomLordKarathressCaribdisEngagedByFirstAssistTankTrigger(botAI); }

    static Trigger* fathom_lord_karathress_sharkkis_engaged_by_second_assist_tank(
        PlayerbotAI* botAI) { return new FathomLordKarathressSharkkisEngagedBySecondAssistTankTrigger(botAI); }

    static Trigger* fathom_lord_karathress_tidalvess_engaged_by_third_assist_tank(
        PlayerbotAI* botAI) { return new FathomLordKarathressTidalvessEngagedByThirdAssistTankTrigger(botAI); }

    static Trigger* fathom_lord_karathress_caribdis_tank_needs_dedicated_healer(
        PlayerbotAI* botAI) { return new FathomLordKarathressCaribdisTankNeedsDedicatedHealerTrigger(botAI); }

    static Trigger* fathom_lord_karathress_pulling_bosses(
        PlayerbotAI* botAI) { return new FathomLordKarathressPullingBossesTrigger(botAI); }

    static Trigger* fathom_lord_karathress_determining_kill_order(
        PlayerbotAI* botAI) { return new FathomLordKarathressDeterminingKillOrderTrigger(botAI); }

    static Trigger* fathom_lord_karathress_tanks_need_to_establish_aggro(
        PlayerbotAI* botAI) { return new FathomLordKarathressTanksNeedToEstablishAggroTrigger(botAI); }

    // Morogrim Tidewalker
    static Trigger* morogrim_tidewalker_boss_engaged_by_main_tank(
        PlayerbotAI* botAI) { return new MorogrimTidewalkerBossEngagedByMainTankTrigger(botAI); }

    static Trigger* morogrim_tidewalker_pulling_boss(
        PlayerbotAI* botAI) { return new MorogrimTidewalkerPullingBossTrigger(botAI); }

    static Trigger* morogrim_tidewalker_water_globules_are_incoming(
        PlayerbotAI* botAI) { return new MorogrimTidewalkerWaterGlobulesAreIncomingTrigger(botAI); }

    // Lady Vashj <Coilfang Matron>
    static Trigger* lady_vashj_boss_engaged_by_main_tank(
        PlayerbotAI* botAI) { return new LadyVashjBossEngagedByMainTankTrigger(botAI); }

    static Trigger* lady_vashj_boss_engaged_by_ranged_in_phase_1(
        PlayerbotAI* botAI) { return new LadyVashjBossEngagedByRangedInPhase1Trigger(botAI); }

    static Trigger* lady_vashj_casts_shock_blast_on_highest_aggro(
        PlayerbotAI* botAI) { return new LadyVashjCastsShockBlastOnHighestAggroTrigger(botAI); }

    static Trigger* lady_vashj_bot_has_static_charge(
        PlayerbotAI* botAI) { return new LadyVashjBotHasStaticChargeTrigger(botAI); }

    static Trigger* lady_vashj_pulling_boss_in_phase_1_and_phase_3(
        PlayerbotAI* botAI) { return new LadyVashjPullingBossInPhase1AndPhase3Trigger(botAI); }

    static Trigger* lady_vashj_adds_spawn_in_phase_2_and_phase_3(
        PlayerbotAI* botAI) { return new LadyVashjAddsSpawnInPhase2AndPhase3Trigger(botAI); }

    static Trigger* lady_vashj_coilfang_strider_is_approaching(
        PlayerbotAI* botAI) { return new LadyVashjCoilfangStriderIsApproachingTrigger(botAI); }

    static Trigger* lady_vashj_tainted_elemental_cheat(
        PlayerbotAI* botAI) { return new LadyVashjTaintedElementalCheatTrigger(botAI); }

    static Trigger* lady_vashj_tainted_core_was_looted(
        PlayerbotAI* botAI) { return new LadyVashjTaintedCoreWasLootedTrigger(botAI); }

    static Trigger* lady_vashj_tainted_core_is_unusable(
        PlayerbotAI* botAI) { return new LadyVashjTaintedCoreIsUnusableTrigger(botAI); }

    static Trigger* lady_vashj_need_to_reset_core_passing_trackers(
        PlayerbotAI* botAI) { return new LadyVashjNeedToResetCorePassingTrackersTrigger(botAI); }

    static Trigger* lady_vashj_toxic_sporebats_are_spewing_poison_clouds(
        PlayerbotAI* botAI) { return new LadyVashjToxicSporebatsAreSpewingPoisonCloudsTrigger(botAI); }

    static Trigger* lady_vashj_bot_is_entangled_in_toxic_spores_or_static_charge(
        PlayerbotAI* botAI) { return new LadyVashjBotIsEntangledInToxicSporesOrStaticChargeTrigger(botAI); }
};

#endif
