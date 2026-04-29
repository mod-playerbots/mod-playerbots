/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_RAIDHYJALSUMMITTRIGGERCONTEXT_H
#define _PLAYERBOT_RAIDHYJALSUMMITTRIGGERCONTEXT_H

#include "RaidHyjalSummitTriggers.h"
#include "NamedObjectContext.h"

class RaidHyjalSummitTriggerContext : public NamedObjectContext<Trigger>
{
public:
    RaidHyjalSummitTriggerContext()
    {
        // General
        creators["hyjal summit bot is not in combat"] =
            &RaidHyjalSummitTriggerContext::hyjal_summit_bot_is_not_in_combat;

        // Rage Winterchill
        creators["rage winterchill pulling boss"] =
            &RaidHyjalSummitTriggerContext::rage_winterchill_pulling_boss;

        creators["rage winterchill boss engaged by main tank"] =
            &RaidHyjalSummitTriggerContext::rage_winterchill_boss_engaged_by_main_tank;

        creators["rage winterchill boss casts death and decay on ranged"] =
            &RaidHyjalSummitTriggerContext::rage_winterchill_boss_casts_death_and_decay_on_ranged;

        creators["rage winterchill melee is standing in death and decay"] =
            &RaidHyjalSummitTriggerContext::rage_winterchill_melee_is_standing_in_death_and_decay;

        // Anetheron
        creators["anetheron pulling boss or infernal"] =
            &RaidHyjalSummitTriggerContext::anetheron_pulling_boss_or_infernal;

        creators["anetheron boss engaged by main tank"] =
            &RaidHyjalSummitTriggerContext::anetheron_boss_engaged_by_main_tank;

        creators["anetheron boss casts carrion swarm"] =
            &RaidHyjalSummitTriggerContext::anetheron_boss_casts_carrion_swarm;

        creators["anetheron bot is targeted by infernal"] =
            &RaidHyjalSummitTriggerContext::anetheron_bot_is_targeted_by_infernal;

        creators["anetheron infernals need to be kept away from raid"] =
            &RaidHyjalSummitTriggerContext::anetheron_infernals_need_to_be_kept_away_from_raid;

        creators["anetheron infernals continue to spawn"] =
            &RaidHyjalSummitTriggerContext::anetheron_infernals_continue_to_spawn;

        // Kaz'rogal
        creators["kaz'rogal pulling boss"] =
            &RaidHyjalSummitTriggerContext::kazrogal_pulling_boss;

        creators["kaz'rogal boss engaged by main tank"] =
            &RaidHyjalSummitTriggerContext::kazrogal_boss_engaged_by_main_tank;

        creators["kaz'rogal boss engaged by assist tanks"] =
            &RaidHyjalSummitTriggerContext::kazrogal_boss_engaged_by_assist_tanks;

        creators["kaz'rogal bot is low on mana"] =
            &RaidHyjalSummitTriggerContext::kazrogal_bot_is_low_on_mana;

        creators["kaz'rogal low mana bots need escape path"] =
            &RaidHyjalSummitTriggerContext::kazrogal_low_mana_bots_need_escape_path;

        creators["kaz'rogal mark deals shadow damage"] =
            &RaidHyjalSummitTriggerContext::kazrogal_mark_deals_shadow_damage;

        // Azgalor
        creators["azgalor pulling boss"] =
            &RaidHyjalSummitTriggerContext::azgalor_pulling_boss;

        creators["azgalor boss engaged by main tank"] =
            &RaidHyjalSummitTriggerContext::azgalor_boss_engaged_by_main_tank;

        creators["azgalor main tank is positioning boss"] =
            &RaidHyjalSummitTriggerContext::azgalor_main_tank_is_positioning_boss;

        creators["azgalor boss engaged by ranged"] =
            &RaidHyjalSummitTriggerContext::azgalor_boss_engaged_by_ranged;

        creators["azgalor boss casts rain of fire on melee"] =
            &RaidHyjalSummitTriggerContext::azgalor_boss_casts_rain_of_fire_on_melee;

        creators["azgalor bot is doomed"] =
            &RaidHyjalSummitTriggerContext::azgalor_bot_is_doomed;

        creators["azgalor doomguards must be controlled"] =
            &RaidHyjalSummitTriggerContext::azgalor_doomguards_must_be_controlled;

        creators["azgalor doomguards must die"] =
            &RaidHyjalSummitTriggerContext::azgalor_doomguards_must_die;

        // Archimonde
        creators["archimonde pulling boss"] =
            &RaidHyjalSummitTriggerContext::archimonde_pulling_boss;

        creators["archimonde boss engaged by main tank"] =
            &RaidHyjalSummitTriggerContext::archimonde_boss_engaged_by_main_tank;

        creators["archimonde boss casts fear"] =
            &RaidHyjalSummitTriggerContext::archimonde_boss_casts_fear;

        creators["archimonde boss casts air burst"] =
            &RaidHyjalSummitTriggerContext::archimonde_boss_casts_air_burst;

        creators["archimonde boss summoned doomfire"] =
            &RaidHyjalSummitTriggerContext::archimonde_boss_summoned_doomfire;

        creators["archimonde bot stood in doomfire"] =
            &RaidHyjalSummitTriggerContext::archimonde_bot_stood_in_doomfire;
    }

private:
    // General
    static Trigger* hyjal_summit_bot_is_not_in_combat(
        PlayerbotAI* botAI) { return new HyjalSummitBotIsNotInCombatTrigger(botAI); }

    // Rage Winterchill
    static Trigger* rage_winterchill_pulling_boss(
        PlayerbotAI* botAI) { return new RageWinterchillPullingBossTrigger(botAI); }

    static Trigger* rage_winterchill_boss_engaged_by_main_tank(
        PlayerbotAI* botAI) { return new RageWinterchillBossEngagedByMainTankTrigger(botAI); }

    static Trigger* rage_winterchill_boss_casts_death_and_decay_on_ranged(
        PlayerbotAI* botAI) { return new RageWinterchillBossCastsDeathAndDecayOnRangedTrigger(botAI); }

    static Trigger* rage_winterchill_melee_is_standing_in_death_and_decay(
        PlayerbotAI* botAI) { return new RageWinterchillMeleeIsStandingInDeathAndDecayTrigger(botAI); }

    // Anetheron
    static Trigger* anetheron_pulling_boss_or_infernal(
        PlayerbotAI* botAI) { return new AnetheronPullingBossOrInfernalTrigger(botAI); }

    static Trigger* anetheron_boss_engaged_by_main_tank(
        PlayerbotAI* botAI) { return new AnetheronBossEngagedByMainTankTrigger(botAI); }

    static Trigger* anetheron_boss_casts_carrion_swarm(
        PlayerbotAI* botAI) { return new AnetheronBossCastsCarrionSwarmTrigger(botAI); }

    static Trigger* anetheron_bot_is_targeted_by_infernal(
        PlayerbotAI* botAI) { return new AnetheronBotIsTargetedByInfernalTrigger(botAI); }

    static Trigger* anetheron_infernals_need_to_be_kept_away_from_raid(
        PlayerbotAI* botAI) { return new AnetheronInfernalsNeedToBeKeptAwayFromRaidTrigger(botAI); }

    static Trigger* anetheron_infernals_continue_to_spawn(
        PlayerbotAI* botAI) { return new AnetheronInfernalsContinueToSpawnTrigger(botAI); }

    // Kaz'rogal
    static Trigger* kazrogal_pulling_boss(
        PlayerbotAI* botAI) { return new KazrogalPullingBossTrigger(botAI); }

    static Trigger* kazrogal_boss_engaged_by_main_tank(
        PlayerbotAI* botAI) { return new KazrogalBossEngagedByMainTankTrigger(botAI); }

    static Trigger* kazrogal_boss_engaged_by_assist_tanks(
        PlayerbotAI* botAI) { return new KazrogalBossEngagedByAssistTanksTrigger(botAI); }

    static Trigger* kazrogal_low_mana_bots_need_escape_path(
        PlayerbotAI* botAI) { return new KazrogalLowManaBotsNeedEscapePathTrigger(botAI); }

    static Trigger* kazrogal_bot_is_low_on_mana(
        PlayerbotAI* botAI) { return new KazrogalBotIsLowOnManaTrigger(botAI); }

    static Trigger* kazrogal_mark_deals_shadow_damage(
        PlayerbotAI* botAI) { return new KazrogalMarkDealsShadowDamageTrigger(botAI); }

    // Azgalor
    static Trigger* azgalor_pulling_boss(
        PlayerbotAI* botAI) { return new AzgalorPullingBossTrigger(botAI); }

    static Trigger* azgalor_boss_engaged_by_main_tank(
        PlayerbotAI* botAI) { return new AzgalorBossEngagedByMainTankTrigger(botAI); }

    static Trigger* azgalor_main_tank_is_positioning_boss(
        PlayerbotAI* botAI) { return new AzgalorMainTankIsPositioningBossTrigger(botAI); }

    static Trigger* azgalor_boss_engaged_by_ranged(
        PlayerbotAI* botAI) { return new AzgalorBossEngagedByRangedTrigger(botAI); }

    static Trigger* azgalor_boss_casts_rain_of_fire_on_melee(
        PlayerbotAI* botAI) { return new AzgalorBossCastsRainOfFireOnMeleeTrigger(botAI); }

    static Trigger* azgalor_bot_is_doomed(
        PlayerbotAI* botAI) { return new AzgalorBotIsDoomedTrigger(botAI); }

    static Trigger* azgalor_doomguards_must_be_controlled(
        PlayerbotAI* botAI) { return new AzgalorDoomguardsMustBeControlledTrigger(botAI); }

    static Trigger* azgalor_doomguards_must_die(
        PlayerbotAI* botAI) { return new AzgalorDoomguardsMustDieTrigger(botAI); }

    // Archimonde
    static Trigger* archimonde_pulling_boss(
        PlayerbotAI* botAI) { return new ArchimondePullingBossTrigger(botAI); }

    static Trigger* archimonde_boss_engaged_by_main_tank(
        PlayerbotAI* botAI) { return new ArchimondeBossEngagedByMainTankTrigger(botAI); }

    static Trigger* archimonde_boss_casts_fear(
        PlayerbotAI* botAI) { return new ArchimondeBossCastsFearTrigger(botAI); }

    static Trigger* archimonde_boss_casts_air_burst(
        PlayerbotAI* botAI) { return new ArchimondeBossCastsAirBurstTrigger(botAI); }

    static Trigger* archimonde_boss_summoned_doomfire(
        PlayerbotAI* botAI) { return new ArchimondeBossSummonedDoomfireTrigger(botAI); }

    static Trigger* archimonde_bot_stood_in_doomfire(
        PlayerbotAI* botAI) { return new ArchimondeBotStoodInDoomfireTrigger(botAI); }
};

#endif
