/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_BTTRIGGERCONTEXT_H
#define PLAYERBOTS_BTTRIGGERCONTEXT_H

#include "BTTriggers.h"
#include "NamedObjectContext.h"

class RaidBlackTempleTriggerContext : public NamedObjectContext<Trigger>
{
public:
    RaidBlackTempleTriggerContext()
    {
        // Shared
        creators["black temple no encounter in progress"] =
            &RaidBlackTempleTriggerContext::black_temple_no_encounter_in_progress;

        creators["black temple class roles should be assigned"] =
            &RaidBlackTempleTriggerContext::black_temple_class_roles_should_be_assigned;

        // Trash
        creators["sister of pleasure should be focused"] =
            &RaidBlackTempleTriggerContext::sister_of_pleasure_should_be_focused;

        creators["shadowmoon reaver wand builds charges"] =
            &RaidBlackTempleTriggerContext::shadowmoon_reaver_wand_builds_charges;

        creators["shadowmoon reaver should control caster pet"] =
            &RaidBlackTempleTriggerContext::shadowmoon_reaver_should_control_caster_pet;

        // High Warlord Naj'entus
        creators["high warlord naj'entus hunter should misdirect"] =
            &RaidBlackTempleTriggerContext::high_warlord_najentus_hunter_should_misdirect;

        creators["high warlord naj'entus should be tanked"] =
            &RaidBlackTempleTriggerContext::high_warlord_najentus_should_be_tanked;

        creators["high warlord naj'entus ranged should spread"] =
            &RaidBlackTempleTriggerContext::high_warlord_najentus_ranged_should_spread;

        creators["high warlord naj'entus impaled player needs remover"] =
            &RaidBlackTempleTriggerContext::high_warlord_najentus_impaled_player_needs_remover;

        creators["high warlord naj'entus impaling spine on group member"] =
            &RaidBlackTempleTriggerContext::high_warlord_najentus_impaling_spine_on_group_member;

        creators["high warlord naj'entus needs spine thrower"] =
            &RaidBlackTempleTriggerContext::high_warlord_najentus_needs_spine_thrower;

        creators["high warlord naj'entus has tidal shield"] =
            &RaidBlackTempleTriggerContext::high_warlord_najentus_has_tidal_shield;

        // Supremus
        creators["supremus hunter should misdirect"] =
            &RaidBlackTempleTriggerContext::supremus_hunter_should_misdirect;

        creators["supremus should be pulled back by tank"] =
            &RaidBlackTempleTriggerContext::supremus_should_be_pulled_back_by_tank;

        creators["supremus ranged should spread"] =
            &RaidBlackTempleTriggerContext::supremus_ranged_should_spread;

        creators["supremus fixated on bot"] =
            &RaidBlackTempleTriggerContext::supremus_fixated_on_bot;

        creators["supremus near fire"] = &RaidBlackTempleTriggerContext::supremus_near_fire;

        creators["supremus reach blocked by fire"] =
            &RaidBlackTempleTriggerContext::supremus_reach_blocked_by_fire;

        // Shade of Akama
        creators["shade of akama should attack channelers and sorcerers"] =
            &RaidBlackTempleTriggerContext::shade_of_akama_should_attack_channelers_and_sorcerers;

        // Teron Gorefiend
        creators["teron gorefiend hunter should misdirect"] =
            &RaidBlackTempleTriggerContext::teron_gorefiend_hunter_should_misdirect;

        creators["teron gorefiend should be tanked"] =
            &RaidBlackTempleTriggerContext::teron_gorefiend_should_be_tanked;

        creators["teron gorefiend ranged should position on balcony"] =
            &RaidBlackTempleTriggerContext::teron_gorefiend_ranged_should_position_on_balcony;

        creators["teron gorefiend casts shadow of death"] =
            &RaidBlackTempleTriggerContext::teron_gorefiend_casts_shadow_of_death;

        creators["teron gorefiend should position for vengeful spirit"] =
            &RaidBlackTempleTriggerContext::teron_gorefiend_should_position_for_vengeful_spirit;

        creators["teron gorefiend transformed into vengeful spirit"] =
            &RaidBlackTempleTriggerContext::teron_gorefiend_transformed_into_vengeful_spirit;

        // Gurtogg Bloodboil
        creators["gurtogg bloodboil hunter should misdirect"] =
            &RaidBlackTempleTriggerContext::gurtogg_bloodboil_hunter_should_misdirect;

        creators["gurtogg bloodboil should be tanked"] =
            &RaidBlackTempleTriggerContext::gurtogg_bloodboil_should_be_tanked;

        creators["gurtogg bloodboil should position for bloodboil"] =
            &RaidBlackTempleTriggerContext::gurtogg_bloodboil_should_position_for_bloodboil;

        creators["gurtogg bloodboil melee should stay behind"] =
            &RaidBlackTempleTriggerContext::gurtogg_bloodboil_melee_should_stay_behind;

        creators["gurtogg bloodboil ranged should avoid frontal cone"] =
            &RaidBlackTempleTriggerContext::gurtogg_bloodboil_ranged_should_avoid_frontal_cone;

        creators["gurtogg bloodboil fel rage on bot"] =
            &RaidBlackTempleTriggerContext::gurtogg_bloodboil_fel_rage_on_bot;

        // Reliquary of Souls
        creators["reliquary of souls hunter should misdirect"] =
            &RaidBlackTempleTriggerContext::reliquary_of_souls_hunter_should_misdirect;

        creators["reliquary of souls should position for suffering"] =
            &RaidBlackTempleTriggerContext::reliquary_of_souls_should_position_for_suffering;

        creators["reliquary of souls healers should attack suffering"] =
            &RaidBlackTempleTriggerContext::reliquary_of_souls_healers_should_attack_suffering;

        creators["reliquary of souls essence of desire has rune shield"] =
            &RaidBlackTempleTriggerContext::reliquary_of_souls_essence_of_desire_has_rune_shield;

        creators["reliquary of souls essence of desire casting deaden"] =
            &RaidBlackTempleTriggerContext::reliquary_of_souls_essence_of_desire_casting_deaden;

        // Mother Shahraz
        creators["mother shahraz hunter should misdirect"] =
            &RaidBlackTempleTriggerContext::mother_shahraz_hunter_should_misdirect;

        creators["mother shahraz should be tanked"] =
            &RaidBlackTempleTriggerContext::mother_shahraz_should_be_tanked;

        creators["mother shahraz being positioned"] =
            &RaidBlackTempleTriggerContext::mother_shahraz_being_positioned;

        creators["mother shahraz ranged should position under statue"] =
            &RaidBlackTempleTriggerContext::mother_shahraz_ranged_should_position_under_statue;

        creators["mother shahraz fatal attraction"] =
            &RaidBlackTempleTriggerContext::mother_shahraz_fatal_attraction;

        // Illidari Council
        creators["illidari council hunter should misdirect"] =
            &RaidBlackTempleTriggerContext::illidari_council_hunter_should_misdirect;

        creators["illidari council gathios should be tanked"] =
            &RaidBlackTempleTriggerContext::illidari_council_gathios_should_be_tanked;

        creators["illidari council gathios casting judgement of command"] =
            &RaidBlackTempleTriggerContext::illidari_council_gathios_casting_judgement_of_command;

        creators["illidari council malande casting circle of healing"] =
            &RaidBlackTempleTriggerContext::illidari_council_malande_casting_circle_of_healing;

        creators["illidari council malande should be tanked"] =
            &RaidBlackTempleTriggerContext::illidari_council_malande_should_be_tanked;

        creators["illidari council veras should be tanked"] =
            &RaidBlackTempleTriggerContext::illidari_council_veras_should_be_tanked;

        creators["illidari council zerevor should be tanked by mage"] =
            &RaidBlackTempleTriggerContext::illidari_council_zerevor_should_be_tanked_by_mage;

        creators["illidari council mage tank healer should reposition"] =
            &RaidBlackTempleTriggerContext::illidari_council_mage_tank_healer_should_reposition;

        creators["illidari council ranged should spread"] =
            &RaidBlackTempleTriggerContext::illidari_council_ranged_should_spread;

        creators["illidari council pet should switch target"] =
            &RaidBlackTempleTriggerContext::illidari_council_pet_should_switch_target;

        creators["illidari council should assign dps priority"] =
            &RaidBlackTempleTriggerContext::illidari_council_should_assign_dps_priority;

        creators["illidari council should manage dps timer"] =
            &RaidBlackTempleTriggerContext::illidari_council_should_manage_dps_timer;

        // Illidan Stormrage <The Betrayer>
        creators["illidan stormrage should position for pull"] =
            &RaidBlackTempleTriggerContext::illidan_stormrage_should_position_for_pull;

        creators["illidan stormrage hunter should misdirect"] =
            &RaidBlackTempleTriggerContext::illidan_stormrage_hunter_should_misdirect;

        creators["illidan stormrage should be tanked"] =
            &RaidBlackTempleTriggerContext::illidan_stormrage_should_be_tanked;

        creators["illidan stormrage parasitic shadowfiend on group member"] =
            &RaidBlackTempleTriggerContext::illidan_stormrage_parasitic_shadowfiend_on_group_member;

        creators["illidan stormrage should track infections"] =
            &RaidBlackTempleTriggerContext::illidan_stormrage_should_track_infections;

        creators["illidan stormrage flames of azzinoth should be tanked"] =
            &RaidBlackTempleTriggerContext::illidan_stormrage_flames_of_azzinoth_should_be_tanked;

        creators["illidan stormrage pets die to hazards"] =
            &RaidBlackTempleTriggerContext::illidan_stormrage_pets_die_to_hazards;

        creators["illidan stormrage pet should attack shadowfiend"] =
            &RaidBlackTempleTriggerContext::illidan_stormrage_pet_should_attack_shadowfiend;

        creators["illidan stormrage pet should attack shadow demon"] =
            &RaidBlackTempleTriggerContext::illidan_stormrage_pet_should_attack_shadow_demon;

        creators["illidan stormrage should position above grate"] =
            &RaidBlackTempleTriggerContext::illidan_stormrage_should_position_above_grate;

        creators["illidan stormrage immunity can clear debuff"] =
            &RaidBlackTempleTriggerContext::illidan_stormrage_immunity_can_clear_debuff;

        creators["illidan stormrage immunity no longer needed"] =
            &RaidBlackTempleTriggerContext::illidan_stormrage_immunity_no_longer_needed;

        creators["illidan stormrage shaman has totems before tanks have aggro"] =
            &RaidBlackTempleTriggerContext::
                illidan_stormrage_shaman_has_totems_before_tanks_have_aggro;

        creators["illidan stormrage should position for landing"] =
            &RaidBlackTempleTriggerContext::illidan_stormrage_should_position_for_landing;

        creators["illidan stormrage melee should stop attacking"] =
            &RaidBlackTempleTriggerContext::illidan_stormrage_melee_should_stop_attacking;

        creators["illidan stormrage should stop shooting"] =
            &RaidBlackTempleTriggerContext::illidan_stormrage_should_stop_shooting;

        creators["illidan stormrage ranged should spread"] =
            &RaidBlackTempleTriggerContext::illidan_stormrage_ranged_should_spread;

        creators["illidan stormrage ranged should reach shadow demon"] =
            &RaidBlackTempleTriggerContext::illidan_stormrage_ranged_should_reach_shadow_demon;

        creators["illidan stormrage melee should stay away from demon form"] =
            &RaidBlackTempleTriggerContext::
                illidan_stormrage_melee_should_stay_away_from_demon_form;

        creators["illidan stormrage warlock should tank demon form"] =
            &RaidBlackTempleTriggerContext::illidan_stormrage_warlock_should_tank_demon_form;

        creators["illidan stormrage warlock should seed shadow demons"] =
            &RaidBlackTempleTriggerContext::illidan_stormrage_warlock_should_seed_shadow_demons;

        creators["illidan stormrage should assign target priority"] =
            &RaidBlackTempleTriggerContext::illidan_stormrage_should_assign_target_priority;

        creators["illidan stormrage should manage dps timers"] =
            &RaidBlackTempleTriggerContext::illidan_stormrage_should_manage_dps_timers;

        creators["illidan stormrage should clear hazards before landing"] =
            &RaidBlackTempleTriggerContext::illidan_stormrage_should_clear_hazards_before_landing;
    }

private:
    // Shared
    static Trigger* black_temple_no_encounter_in_progress(PlayerbotAI* botAI)
    {
        return new BlackTempleNoEncounterInProgressTrigger(botAI);
    }

    static Trigger* black_temple_class_roles_should_be_assigned(PlayerbotAI* botAI)
    {
        return new BlackTempleClassRolesShouldBeAssignedTrigger(botAI);
    }

    // Trash
    static Trigger* sister_of_pleasure_should_be_focused(PlayerbotAI* botAI)
    {
        return new SisterOfPleasureShouldBeFocusedTrigger(botAI);
    }
    static Trigger* shadowmoon_reaver_wand_builds_charges(PlayerbotAI* botAI)
    {
        return new ShadowmoonReaverWandBuildsChargesTrigger(botAI);
    }
    static Trigger* shadowmoon_reaver_should_control_caster_pet(PlayerbotAI* botAI)
    {
        return new ShadowmoonReaverShouldControlCasterPetTrigger(botAI);
    }

    // High Warlord Naj'entus
    static Trigger* high_warlord_najentus_hunter_should_misdirect(PlayerbotAI* botAI)
    {
        return new BlackTempleHunterShouldMisdirectTrigger(
            botAI, "high warlord naj'entus hunter should misdirect",
            BtHelpers::BtBosses::DATA_HIGH_WARLORD_NAJENTUS);
    }
    static Trigger* high_warlord_najentus_should_be_tanked(PlayerbotAI* botAI)
    {
        return new HighWarlordNajentusShouldBeTankedTrigger(botAI);
    }
    static Trigger* high_warlord_najentus_ranged_should_spread(PlayerbotAI* botAI)
    {
        return new HighWarlordNajentusRangedShouldSpreadTrigger(botAI);
    }
    static Trigger* high_warlord_najentus_impaled_player_needs_remover(PlayerbotAI* botAI)
    {
        return new HighWarlordNajentusImpaledPlayerNeedsRemoverTrigger(botAI);
    }
    static Trigger* high_warlord_najentus_impaling_spine_on_group_member(PlayerbotAI* botAI)
    {
        return new HighWarlordNajentusImpalingSpineOnGroupMemberTrigger(botAI);
    }
    static Trigger* high_warlord_najentus_needs_spine_thrower(PlayerbotAI* botAI)
    {
        return new HighWarlordNajentusNeedsSpineThrowerTrigger(botAI);
    }
    static Trigger* high_warlord_najentus_has_tidal_shield(PlayerbotAI* botAI)
    {
        return new HighWarlordNajentusHasTidalShieldTrigger(botAI);
    }

    // Supremus
    static Trigger* supremus_hunter_should_misdirect(PlayerbotAI* botAI)
    {
        return new BlackTempleHunterShouldMisdirectTrigger(
            botAI, "supremus hunter should misdirect", BtHelpers::BtBosses::DATA_SUPREMUS);
    }
    static Trigger* supremus_should_be_pulled_back_by_tank(PlayerbotAI* botAI)
    {
        return new SupremusShouldBePulledBackByTankTrigger(botAI);
    }
    static Trigger* supremus_ranged_should_spread(PlayerbotAI* botAI)
    {
        return new SupremusRangedShouldSpreadTrigger(botAI);
    }
    static Trigger* supremus_fixated_on_bot(PlayerbotAI* botAI)
    {
        return new SupremusFixatedOnBotTrigger(botAI);
    }
    static Trigger* supremus_near_fire(PlayerbotAI* botAI)
    {
        return new SupremusNearFireTrigger(botAI);
    }
    static Trigger* supremus_reach_blocked_by_fire(PlayerbotAI* botAI)
    {
        return new SupremusReachBlockedByFireTrigger(botAI);
    }

    // Shade of Akama
    static Trigger* shade_of_akama_should_attack_channelers_and_sorcerers(PlayerbotAI* botAI)
    {
        return new ShadeOfAkamaShouldAttackChannelersAndSorcerersTrigger(botAI);
    }

    // Teron Gorefiend
    static Trigger* teron_gorefiend_hunter_should_misdirect(PlayerbotAI* botAI)
    {
        return new BlackTempleHunterShouldMisdirectTrigger(
            botAI, "teron gorefiend hunter should misdirect",
            BtHelpers::BtBosses::DATA_TERON_GOREFIEND);
    }
    static Trigger* teron_gorefiend_should_be_tanked(PlayerbotAI* botAI)
    {
        return new TeronGorefiendShouldBeTankedTrigger(botAI);
    }
    static Trigger* teron_gorefiend_ranged_should_position_on_balcony(PlayerbotAI* botAI)
    {
        return new TeronGorefiendRangedShouldPositionOnBalconyTrigger(botAI);
    }
    static Trigger* teron_gorefiend_casts_shadow_of_death(PlayerbotAI* botAI)
    {
        return new TeronGorefiendCastsShadowOfDeathTrigger(botAI);
    }
    static Trigger* teron_gorefiend_should_position_for_vengeful_spirit(PlayerbotAI* botAI)
    {
        return new TeronGorefiendShouldPositionForVengefulSpiritTrigger(botAI);
    }
    static Trigger* teron_gorefiend_transformed_into_vengeful_spirit(PlayerbotAI* botAI)
    {
        return new TeronGorefiendTransformedIntoVengefulSpiritTrigger(botAI);
    }

    // Gurtogg Bloodboil
    static Trigger* gurtogg_bloodboil_hunter_should_misdirect(PlayerbotAI* botAI)
    {
        return new BlackTempleHunterShouldMisdirectTrigger(
            botAI, "gurtogg bloodboil hunter should misdirect",
            BtHelpers::BtBosses::DATA_GURTOGG_BLOODBOIL);
    }
    static Trigger* gurtogg_bloodboil_should_be_tanked(PlayerbotAI* botAI)
    {
        return new GurtoggBloodboilShouldBeTankedTrigger(botAI);
    }
    static Trigger* gurtogg_bloodboil_should_position_for_bloodboil(PlayerbotAI* botAI)
    {
        return new GurtoggBloodboilShouldPositionForBloodboilTrigger(botAI);
    }
    static Trigger* gurtogg_bloodboil_melee_should_stay_behind(PlayerbotAI* botAI)
    {
        return new GurtoggBloodboilMeleeShouldStayBehindTrigger(botAI);
    }
    static Trigger* gurtogg_bloodboil_ranged_should_avoid_frontal_cone(PlayerbotAI* botAI)
    {
        return new GurtoggBloodboilRangedShouldAvoidFrontalConeTrigger(botAI);
    }
    static Trigger* gurtogg_bloodboil_fel_rage_on_bot(PlayerbotAI* botAI)
    {
        return new GurtoggBloodboilFelRageOnBotTrigger(botAI);
    }

    // Reliquary of Souls
    static Trigger* reliquary_of_souls_hunter_should_misdirect(PlayerbotAI* botAI)
    {
        return new ReliquaryOfSoulsHunterShouldMisdirectTrigger(botAI);
    }
    static Trigger* reliquary_of_souls_should_position_for_suffering(PlayerbotAI* botAI)
    {
        return new ReliquaryOfSoulsShouldPositionForSufferingTrigger(botAI);
    }
    static Trigger* reliquary_of_souls_healers_should_attack_suffering(PlayerbotAI* botAI)
    {
        return new ReliquaryOfSoulsHealersShouldAttackSufferingTrigger(botAI);
    }
    static Trigger* reliquary_of_souls_essence_of_desire_has_rune_shield(PlayerbotAI* botAI)
    {
        return new ReliquaryOfSoulsEssenceOfDesireHasRuneShieldTrigger(botAI);
    }
    static Trigger* reliquary_of_souls_essence_of_desire_casting_deaden(PlayerbotAI* botAI)
    {
        return new ReliquaryOfSoulsEssenceOfDesireCastingDeadenTrigger(botAI);
    }

    // Mother Shahraz
    static Trigger* mother_shahraz_hunter_should_misdirect(PlayerbotAI* botAI)
    {
        return new BlackTempleHunterShouldMisdirectTrigger(
            botAI, "mother shahraz hunter should misdirect",
            BtHelpers::BtBosses::DATA_MOTHER_SHAHRAZ);
    }
    static Trigger* mother_shahraz_should_be_tanked(PlayerbotAI* botAI)
    {
        return new MotherShahrazShouldBeTankedTrigger(botAI);
    }
    static Trigger* mother_shahraz_being_positioned(PlayerbotAI* botAI)
    {
        return new MotherShahrazBeingPositionedTrigger(botAI);
    }
    static Trigger* mother_shahraz_ranged_should_position_under_statue(PlayerbotAI* botAI)
    {
        return new MotherShahrazRangedShouldPositionUnderStatueTrigger(botAI);
    }
    static Trigger* mother_shahraz_fatal_attraction(PlayerbotAI* botAI)
    {
        return new MotherShahrazFatalAttractionTrigger(botAI);
    }

    // Illidari Council
    static Trigger* illidari_council_hunter_should_misdirect(PlayerbotAI* botAI)
    {
        return new BlackTempleHunterShouldMisdirectTrigger(
            botAI, "illidari council hunter should misdirect",
            BtHelpers::BtBosses::DATA_ILLIDARI_COUNCIL,
            BtHelpers::BtCreatureData::DATA_GATHIOS_THE_SHATTERER);
    }
    static Trigger* illidari_council_gathios_should_be_tanked(PlayerbotAI* botAI)
    {
        return new IllidariCouncilGathiosShouldBeTankedTrigger(botAI);
    }
    static Trigger* illidari_council_gathios_casting_judgement_of_command(PlayerbotAI* botAI)
    {
        return new IllidariCouncilGathiosCastingJudgementOfCommandTrigger(botAI);
    }
    static Trigger* illidari_council_malande_casting_circle_of_healing(PlayerbotAI* botAI)
    {
        return new IllidariCouncilMalandeCastingCircleOfHealingTrigger(botAI);
    }
    static Trigger* illidari_council_malande_should_be_tanked(PlayerbotAI* botAI)
    {
        return new IllidariCouncilMalandeShouldBeTankedTrigger(botAI);
    }
    static Trigger* illidari_council_veras_should_be_tanked(PlayerbotAI* botAI)
    {
        return new IllidariCouncilVerasShouldBeTankedTrigger(botAI);
    }
    static Trigger* illidari_council_zerevor_should_be_tanked_by_mage(PlayerbotAI* botAI)
    {
        return new IllidariCouncilZerevorShouldBeTankedByMageTrigger(botAI);
    }
    static Trigger* illidari_council_mage_tank_healer_should_reposition(PlayerbotAI* botAI)
    {
        return new IllidariCouncilMageTankHealerShouldRepositionTrigger(botAI);
    }
    static Trigger* illidari_council_ranged_should_spread(PlayerbotAI* botAI)
    {
        return new IllidariCouncilRangedShouldSpreadTrigger(botAI);
    }
    static Trigger* illidari_council_pet_should_switch_target(PlayerbotAI* botAI)
    {
        return new IllidariCouncilPetShouldSwitchTargetTrigger(botAI);
    }
    static Trigger* illidari_council_should_assign_dps_priority(PlayerbotAI* botAI)
    {
        return new IllidariCouncilShouldAssignDpsPriorityTrigger(botAI);
    }
    static Trigger* illidari_council_should_manage_dps_timer(PlayerbotAI* botAI)
    {
        return new IllidariCouncilShouldManageDpsTimerTrigger(botAI);
    }

    // Illidan Stormrage <The Betrayer>
    static Trigger* illidan_stormrage_should_position_for_pull(PlayerbotAI* botAI)
    {
        return new IllidanStormrageShouldPositionForPullTrigger(botAI);
    }
    static Trigger* illidan_stormrage_hunter_should_misdirect(PlayerbotAI* botAI)
    {
        return new IllidanStormrageHunterShouldMisdirectTrigger(botAI);
    }
    static Trigger* illidan_stormrage_should_be_tanked(PlayerbotAI* botAI)
    {
        return new IllidanStormrageShouldBeTankedTrigger(botAI);
    }
    static Trigger* illidan_stormrage_parasitic_shadowfiend_on_group_member(PlayerbotAI* botAI)
    {
        return new IllidanStormrageParasiticShadowfiendOnGroupMemberTrigger(botAI);
    }
    static Trigger* illidan_stormrage_should_track_infections(PlayerbotAI* botAI)
    {
        return new IllidanStormrageShouldTrackInfectionsTrigger(botAI);
    }
    static Trigger* illidan_stormrage_flames_of_azzinoth_should_be_tanked(PlayerbotAI* botAI)
    {
        return new IllidanStormrageFlamesOfAzzinothShouldBeTankedTrigger(botAI);
    }
    static Trigger* illidan_stormrage_pets_die_to_hazards(PlayerbotAI* botAI)
    {
        return new IllidanStormragePetsDieToHazardsTrigger(botAI);
    }
    static Trigger* illidan_stormrage_pet_should_attack_shadowfiend(PlayerbotAI* botAI)
    {
        return new IllidanStormragePetShouldAttackShadowfiendTrigger(botAI);
    }
    static Trigger* illidan_stormrage_pet_should_attack_shadow_demon(PlayerbotAI* botAI)
    {
        return new IllidanStormragePetShouldAttackShadowDemonTrigger(botAI);
    }
    static Trigger* illidan_stormrage_should_position_above_grate(PlayerbotAI* botAI)
    {
        return new IllidanStormrageShouldPositionAboveGrateTrigger(botAI);
    }
    static Trigger* illidan_stormrage_immunity_can_clear_debuff(PlayerbotAI* botAI)
    {
        return new IllidanStormrageImmunityCanClearDebuffTrigger(botAI);
    }
    static Trigger* illidan_stormrage_immunity_no_longer_needed(PlayerbotAI* botAI)
    {
        return new IllidanStormrageImmunityNoLongerNeededTrigger(botAI);
    }
    static Trigger* illidan_stormrage_shaman_has_totems_before_tanks_have_aggro(PlayerbotAI* botAI)
    {
        return new IllidanStormrageShamanHasTotemsBeforeTanksHaveAggroTrigger(botAI);
    }
    static Trigger* illidan_stormrage_should_position_for_landing(PlayerbotAI* botAI)
    {
        return new IllidanStormrageShouldPositionForLandingTrigger(botAI);
    }
    static Trigger* illidan_stormrage_melee_should_stop_attacking(PlayerbotAI* botAI)
    {
        return new IllidanStormrageMeleeShouldStopAttackingTrigger(botAI);
    }
    static Trigger* illidan_stormrage_should_stop_shooting(PlayerbotAI* botAI)
    {
        return new IllidanStormrageShouldStopShootingTrigger(botAI);
    }
    static Trigger* illidan_stormrage_ranged_should_spread(PlayerbotAI* botAI)
    {
        return new IllidanStormrageRangedShouldSpreadTrigger(botAI);
    }
    static Trigger* illidan_stormrage_ranged_should_reach_shadow_demon(PlayerbotAI* botAI)
    {
        return new IllidanStormrageRangedShouldReachShadowDemonTrigger(botAI);
    }
    static Trigger* illidan_stormrage_melee_should_stay_away_from_demon_form(PlayerbotAI* botAI)
    {
        return new IllidanStormrageMeleeShouldStayAwayFromDemonFormTrigger(botAI);
    }
    static Trigger* illidan_stormrage_warlock_should_tank_demon_form(PlayerbotAI* botAI)
    {
        return new IllidanStormrageWarlockShouldTankDemonFormTrigger(botAI);
    }
    static Trigger* illidan_stormrage_warlock_should_seed_shadow_demons(PlayerbotAI* botAI)
    {
        return new IllidanStormrageWarlockShouldSeedShadowDemonsTrigger(botAI);
    }
    static Trigger* illidan_stormrage_should_assign_target_priority(PlayerbotAI* botAI)
    {
        return new IllidanStormrageShouldAssignTargetPriorityTrigger(botAI);
    }
    static Trigger* illidan_stormrage_should_manage_dps_timers(PlayerbotAI* botAI)
    {
        return new IllidanStormrageShouldManageDpsTimersTrigger(botAI);
    }
    static Trigger* illidan_stormrage_should_clear_hazards_before_landing(PlayerbotAI* botAI)
    {
        return new IllidanStormrageShouldClearHazardsBeforeLandingTrigger(botAI);
    }
};

#endif
