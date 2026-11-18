/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RAID_TOCTRIGGERS_ANUBARAK_H
#define PLAYERBOTS_RAID_TOCTRIGGERS_ANUBARAK_H

#include <vector>

#include "NamedObjectContext.h"
#include "Trigger.h"

class AnubarakKiterNeedsImmunityTrigger : public Trigger
{
public:
    AnubarakKiterNeedsImmunityTrigger(PlayerbotAI* botAI) : Trigger(botAI, "anubarak kiter needs immunity") {}
    bool IsActive() override;
};

class AnubarakPursuedBySpikeTrigger : public Trigger
{
public:
    AnubarakPursuedBySpikeTrigger(PlayerbotAI* botAI) : Trigger(botAI, "anubarak pursued by spike") {}
    bool IsActive() override;
};

class AnubarakKiterNeedsProtectionTrigger : public Trigger
{
public:
    AnubarakKiterNeedsProtectionTrigger(PlayerbotAI* botAI) : Trigger(botAI, "anubarak kiter needs protection") {}
    bool IsActive() override;
};

class AnubarakKiterNeedsFreedomTrigger : public Trigger
{
public:
    AnubarakKiterNeedsFreedomTrigger(PlayerbotAI* botAI) : Trigger(botAI, "anubarak kiter needs freedom") {}
    bool IsActive() override;
};

class AnubarakHopEscortTrigger : public Trigger
{
public:
    AnubarakHopEscortTrigger(PlayerbotAI* botAI) : Trigger(botAI, "anubarak hop escort") {}
    bool IsActive() override;
};

class AnubarakRedirectThreatTrigger : public Trigger
{
public:
    AnubarakRedirectThreatTrigger(PlayerbotAI* botAI) : Trigger(botAI, "anubarak redirect threat") {}
    bool IsActive() override;
};

class AnubarakSpikeNearbyTrigger : public Trigger
{
public:
    AnubarakSpikeNearbyTrigger(PlayerbotAI* botAI) : Trigger(botAI, "anubarak spike nearby") {}
    bool IsActive() override;
};

class AnubarakBurrowerCastingShadowStrikeTrigger : public Trigger
{
public:
    AnubarakBurrowerCastingShadowStrikeTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "anubarak burrower casting shadow strike") {}
    bool IsActive() override;
};

class AnubarakLeechingSwarmOnTankTrigger : public Trigger
{
public:
    AnubarakLeechingSwarmOnTankTrigger(PlayerbotAI* botAI) : Trigger(botAI, "anubarak leeching swarm on tank") {}
    bool IsActive() override;
};

class AnubarakRangedShouldSeedPermafrostTrigger : public Trigger
{
public:
    AnubarakRangedShouldSeedPermafrostTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "anubarak ranged should seed permafrost") {}
    bool IsActive() override;
};

class AnubarakDpsTargetTrigger : public Trigger
{
public:
    AnubarakDpsTargetTrigger(PlayerbotAI* botAI) : Trigger(botAI, "anubarak dps target") {}
    bool IsActive() override;
};

class AnubarakBurrowerNeedsAssistTankTrigger : public Trigger
{
public:
    AnubarakBurrowerNeedsAssistTankTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "anubarak burrower needs assist tank") {}
    bool IsActive() override;
};

class AnubarakSideTankReadyTrigger : public Trigger
{
public:
    AnubarakSideTankReadyTrigger(PlayerbotAI* botAI) : Trigger(botAI, "anubarak side tank ready") {}
    bool IsActive() override;
};

class AnubarakSideTankAreaThreatTrigger : public Trigger
{
public:
    AnubarakSideTankAreaThreatTrigger(PlayerbotAI* botAI) : Trigger(botAI, "anubarak side tank area threat") {}
    bool IsActive() override;
};

class AnubarakCleaveBuffNearBurrowerTrigger : public Trigger
{
public:
    AnubarakCleaveBuffNearBurrowerTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "anubarak cleave buff near burrower") {}
    bool IsActive() override;
};

class AnubarakSideTankNeedsDefensiveTrigger : public Trigger
{
public:
    AnubarakSideTankNeedsDefensiveTrigger(PlayerbotAI* botAI) : Trigger(botAI, "anubarak side tank needs defensive") {}
    bool IsActive() override;
};

class AnubarakColdSelfSaveTrigger : public Trigger
{
public:
    AnubarakColdSelfSaveTrigger(PlayerbotAI* botAI) : Trigger(botAI, "anubarak penetrating cold self save") {}
    bool IsActive() override;
};

class AnubarakSideTankDeadTrigger : public Trigger
{
public:
    AnubarakSideTankDeadTrigger(PlayerbotAI* botAI) : Trigger(botAI, "anubarak side tank dead") {}
    bool IsActive() override;
};

class AnubarakGripFrostSphereTrigger : public Trigger
{
public:
    AnubarakGripFrostSphereTrigger(PlayerbotAI* botAI) : Trigger(botAI, "anubarak grip frost sphere") {}
    bool IsActive() override;
};

class AnubarakRaidGathersTrigger : public Trigger
{
public:
    AnubarakRaidGathersTrigger(PlayerbotAI* botAI) : Trigger(botAI, "anubarak raid gathers") {}
    bool IsActive() override;
};

class AnubarakHealerStandTrigger : public Trigger
{
public:
    AnubarakHealerStandTrigger(PlayerbotAI* botAI) : Trigger(botAI, "anubarak healer stand") {}
    bool IsActive() override;
};

class AnubarakRangedStandTrigger : public Trigger
{
public:
    AnubarakRangedStandTrigger(PlayerbotAI* botAI) : Trigger(botAI, "anubarak ranged stand") {}
    bool IsActive() override;
};

class AnubarakOnPermafrostTrigger : public Trigger
{
public:
    AnubarakOnPermafrostTrigger(PlayerbotAI* botAI) : Trigger(botAI, "anubarak on permafrost") {}
    bool IsActive() override;
};

class AnubarakMeleeBurrowerStandTrigger : public Trigger
{
public:
    AnubarakMeleeBurrowerStandTrigger(PlayerbotAI* botAI) : Trigger(botAI, "anubarak melee burrower stand") {}
    bool IsActive() override;
};

class AnubarakBurrowerLeadTrigger : public Trigger
{
public:
    AnubarakBurrowerLeadTrigger(PlayerbotAI* botAI) : Trigger(botAI, "anubarak burrower lead") {}
    bool IsActive() override;
};

class AnubarakKiteCatDropTrigger : public Trigger
{
public:
    AnubarakKiteCatDropTrigger(PlayerbotAI* botAI) : Trigger(botAI, "anubarak kite cat drop") {}
    bool IsActive() override;
};

class AnubarakTankNeedsRescueTrigger : public Trigger
{
public:
    AnubarakTankNeedsRescueTrigger(PlayerbotAI* botAI) : Trigger(botAI, "anubarak tank needs rescue") {}
    bool IsActive() override;
};

class AnubarakLustDueTrigger : public Trigger
{
public:
    AnubarakLustDueTrigger(PlayerbotAI* botAI) : Trigger(botAI, "anubarak lust due") {}
    bool IsActive() override;
};

class AnubarakDemonicCircleDueTrigger : public Trigger
{
public:
    AnubarakDemonicCircleDueTrigger(PlayerbotAI* botAI) : Trigger(botAI, "anubarak demonic circle due") {}
    bool IsActive() override;
};

class AnubarakScarabOnRaidTrigger : public Trigger
{
public:
    AnubarakScarabOnRaidTrigger(PlayerbotAI* botAI) : Trigger(botAI, "anubarak scarab on raid") {}
    bool IsActive() override;
};

class AnubarakEngagedByMainTankTrigger : public Trigger
{
public:
    AnubarakEngagedByMainTankTrigger(PlayerbotAI* botAI) : Trigger(botAI, "anubarak engaged by main tank") {}
    bool IsActive() override;
};

class AnubarakPenetratingColdOnRaidTrigger : public Trigger
{
public:
    AnubarakPenetratingColdOnRaidTrigger(PlayerbotAI* botAI) : Trigger(botAI, "anubarak penetrating cold on raid") {}
    bool IsActive() override;
};

class AnubarakSwarmHealUrgentTrigger : public Trigger
{
public:
    AnubarakSwarmHealUrgentTrigger(PlayerbotAI* botAI) : Trigger(botAI, "anubarak swarm heal urgent") {}
    bool IsActive() override;
};

class AnubarakSwarmHealTrigger : public Trigger
{
public:
    AnubarakSwarmHealTrigger(PlayerbotAI* botAI) : Trigger(botAI, "anubarak swarm heal") {}
    bool IsActive() override;
};

class AnubarakAuraMasteryDueTrigger : public Trigger
{
public:
    AnubarakAuraMasteryDueTrigger(PlayerbotAI* botAI) : Trigger(botAI, "anubarak aura mastery due") {}
    bool IsActive() override;
};

class AnubarakDivinePleaTrigger : public Trigger
{
public:
    AnubarakDivinePleaTrigger(PlayerbotAI* botAI) : Trigger(botAI, "anubarak divine plea") {}
    bool IsActive() override;
};

class AnubarakInnervateHealerTrigger : public Trigger
{
public:
    AnubarakInnervateHealerTrigger(PlayerbotAI* botAI) : Trigger(botAI, "anubarak innervate healer") {}
    bool IsActive() override;
};

class AnubarakScarabKiteTrigger : public Trigger
{
public:
    AnubarakScarabKiteTrigger(PlayerbotAI* botAI) : Trigger(botAI, "anubarak scarab kite") {}
    bool IsActive() override;
};

class AnubarakManaTopUpTrigger : public Trigger
{
public:
    AnubarakManaTopUpTrigger(PlayerbotAI* botAI) : Trigger(botAI, "anubarak mana top up") {}
    bool IsActive() override;
};

class AnubarakLightJudgeDueTrigger : public Trigger
{
public:
    AnubarakLightJudgeDueTrigger(PlayerbotAI* botAI) : Trigger(botAI, "anubarak light judge due") {}
    bool IsActive() override;
};

class AnubarakHealingStreamDueTrigger : public Trigger
{
public:
    AnubarakHealingStreamDueTrigger(PlayerbotAI* botAI) : Trigger(botAI, "anubarak healing stream due") {}
    bool IsActive() override;
};

class ToCAnubarakTriggerContext : public NamedObjectContext<Trigger>
{
public:
    ToCAnubarakTriggerContext() : NamedObjectContext<Trigger>()
    {
        creators["anubarak kiter needs immunity"] =
            &ToCAnubarakTriggerContext::anubarak_kiter_needs_immunity;
        creators["anubarak pursued by spike"] =
            &ToCAnubarakTriggerContext::anubarak_pursued_by_spike;
        creators["anubarak kiter needs protection"] =
            &ToCAnubarakTriggerContext::anubarak_kiter_needs_protection;
        creators["anubarak kiter needs freedom"] =
            &ToCAnubarakTriggerContext::anubarak_kiter_needs_freedom;
        creators["anubarak hop escort"] =
            &ToCAnubarakTriggerContext::anubarak_hop_escort;
        creators["anubarak redirect threat"] =
            &ToCAnubarakTriggerContext::anubarak_redirect_threat;
        creators["anubarak spike nearby"] =
            &ToCAnubarakTriggerContext::anubarak_spike_nearby;
        creators["anubarak burrower casting shadow strike"] =
            &ToCAnubarakTriggerContext::anubarak_burrower_casting_shadow_strike;
        creators["anubarak leeching swarm on tank"] =
            &ToCAnubarakTriggerContext::anubarak_leeching_swarm_on_tank;
        creators["anubarak ranged should seed permafrost"] =
            &ToCAnubarakTriggerContext::anubarak_ranged_should_seed_permafrost;
        creators["anubarak dps target"] =
            &ToCAnubarakTriggerContext::anubarak_dps_target;
        creators["anubarak burrower needs assist tank"] =
            &ToCAnubarakTriggerContext::anubarak_burrower_needs_assist_tank;
        creators["anubarak side tank ready"] =
            &ToCAnubarakTriggerContext::anubarak_side_tank_ready;
        creators["anubarak side tank area threat"] =
            &ToCAnubarakTriggerContext::anubarak_side_tank_area_threat;
        creators["anubarak cleave buff near burrower"] =
            &ToCAnubarakTriggerContext::anubarak_cleave_buff_near_burrower;
        creators["anubarak side tank needs defensive"] =
            &ToCAnubarakTriggerContext::anubarak_side_tank_needs_defensive;
        creators["anubarak penetrating cold self save"] =
            &ToCAnubarakTriggerContext::anubarak_penetrating_cold_self_save;
        creators["anubarak side tank dead"] =
            &ToCAnubarakTriggerContext::anubarak_side_tank_dead;
        creators["anubarak grip frost sphere"] =
            &ToCAnubarakTriggerContext::anubarak_grip_frost_sphere;
        creators["anubarak raid gathers"] =
            &ToCAnubarakTriggerContext::anubarak_raid_gathers;
        creators["anubarak healer stand"] =
            &ToCAnubarakTriggerContext::anubarak_healer_stand;
        creators["anubarak ranged stand"] =
            &ToCAnubarakTriggerContext::anubarak_ranged_stand;
        creators["anubarak on permafrost"] =
            &ToCAnubarakTriggerContext::anubarak_on_permafrost;
        creators["anubarak melee burrower stand"] =
            &ToCAnubarakTriggerContext::anubarak_melee_burrower_stand;
        creators["anubarak burrower lead"] =
            &ToCAnubarakTriggerContext::anubarak_burrower_lead;
        creators["anubarak kite cat drop"] =
            &ToCAnubarakTriggerContext::anubarak_kite_cat_drop;
        creators["anubarak tank needs rescue"] =
            &ToCAnubarakTriggerContext::anubarak_tank_needs_rescue;
        creators["anubarak lust due"] =
            &ToCAnubarakTriggerContext::anubarak_lust_due;
        creators["anubarak demonic circle due"] =
            &ToCAnubarakTriggerContext::anubarak_demonic_circle_due;
        creators["anubarak scarab on raid"] =
            &ToCAnubarakTriggerContext::anubarak_scarab_on_raid;
        creators["anubarak engaged by main tank"] =
            &ToCAnubarakTriggerContext::anubarak_engaged_by_main_tank;
        creators["anubarak penetrating cold on raid"] =
            &ToCAnubarakTriggerContext::anubarak_penetrating_cold_on_raid;
        creators["anubarak swarm heal urgent"] =
            &ToCAnubarakTriggerContext::anubarak_swarm_heal_urgent;
        creators["anubarak swarm heal"] =
            &ToCAnubarakTriggerContext::anubarak_swarm_heal;
        creators["anubarak aura mastery due"] =
            &ToCAnubarakTriggerContext::anubarak_aura_mastery_due;
        creators["anubarak divine plea"] =
            &ToCAnubarakTriggerContext::anubarak_divine_plea;
        creators["anubarak innervate healer"] =
            &ToCAnubarakTriggerContext::anubarak_innervate_healer;
        creators["anubarak scarab kite"] =
            &ToCAnubarakTriggerContext::anubarak_scarab_kite;
        creators["anubarak mana top up"] =
            &ToCAnubarakTriggerContext::anubarak_mana_top_up;
        creators["anubarak light judge due"] =
            &ToCAnubarakTriggerContext::anubarak_light_judge_due;
        creators["anubarak healing stream due"] =
            &ToCAnubarakTriggerContext::anubarak_healing_stream_due;
    }

private:
    static Trigger* anubarak_kiter_needs_immunity(PlayerbotAI* botAI) {
        return new AnubarakKiterNeedsImmunityTrigger(botAI);
    }

    static Trigger* anubarak_pursued_by_spike(PlayerbotAI* botAI) {
        return new AnubarakPursuedBySpikeTrigger(botAI);
    }

    static Trigger* anubarak_kiter_needs_protection(PlayerbotAI* botAI) {
        return new AnubarakKiterNeedsProtectionTrigger(botAI);
    }

    static Trigger* anubarak_kiter_needs_freedom(PlayerbotAI* botAI) {
        return new AnubarakKiterNeedsFreedomTrigger(botAI);
    }

    static Trigger* anubarak_hop_escort(PlayerbotAI* botAI) {
        return new AnubarakHopEscortTrigger(botAI);
    }

    static Trigger* anubarak_redirect_threat(PlayerbotAI* botAI) {
        return new AnubarakRedirectThreatTrigger(botAI);
    }

    static Trigger* anubarak_spike_nearby(PlayerbotAI* botAI) {
        return new AnubarakSpikeNearbyTrigger(botAI);
    }

    static Trigger* anubarak_burrower_casting_shadow_strike(PlayerbotAI* botAI) {
        return new AnubarakBurrowerCastingShadowStrikeTrigger(botAI);
    }

    static Trigger* anubarak_leeching_swarm_on_tank(PlayerbotAI* botAI) {
        return new AnubarakLeechingSwarmOnTankTrigger(botAI);
    }

    static Trigger* anubarak_ranged_should_seed_permafrost(PlayerbotAI* botAI) {
        return new AnubarakRangedShouldSeedPermafrostTrigger(botAI);
    }

    static Trigger* anubarak_dps_target(PlayerbotAI* botAI) {
        return new AnubarakDpsTargetTrigger(botAI);
    }

    static Trigger* anubarak_burrower_needs_assist_tank(PlayerbotAI* botAI) {
        return new AnubarakBurrowerNeedsAssistTankTrigger(botAI);
    }

    static Trigger* anubarak_side_tank_ready(PlayerbotAI* botAI) {
        return new AnubarakSideTankReadyTrigger(botAI);
    }

    static Trigger* anubarak_side_tank_area_threat(PlayerbotAI* botAI) {
        return new AnubarakSideTankAreaThreatTrigger(botAI);
    }

    static Trigger* anubarak_cleave_buff_near_burrower(PlayerbotAI* botAI) {
        return new AnubarakCleaveBuffNearBurrowerTrigger(botAI);
    }

    static Trigger* anubarak_side_tank_needs_defensive(PlayerbotAI* botAI) {
        return new AnubarakSideTankNeedsDefensiveTrigger(botAI);
    }

    static Trigger* anubarak_penetrating_cold_self_save(PlayerbotAI* botAI) {
        return new AnubarakColdSelfSaveTrigger(botAI);
    }

    static Trigger* anubarak_side_tank_dead(PlayerbotAI* botAI) {
        return new AnubarakSideTankDeadTrigger(botAI);
    }

    static Trigger* anubarak_grip_frost_sphere(PlayerbotAI* botAI) {
        return new AnubarakGripFrostSphereTrigger(botAI);
    }

    static Trigger* anubarak_raid_gathers(PlayerbotAI* botAI) {
        return new AnubarakRaidGathersTrigger(botAI);
    }

    static Trigger* anubarak_healer_stand(PlayerbotAI* botAI) {
        return new AnubarakHealerStandTrigger(botAI);
    }

    static Trigger* anubarak_ranged_stand(PlayerbotAI* botAI) {
        return new AnubarakRangedStandTrigger(botAI);
    }

    static Trigger* anubarak_on_permafrost(PlayerbotAI* botAI) {
        return new AnubarakOnPermafrostTrigger(botAI);
    }

    static Trigger* anubarak_melee_burrower_stand(PlayerbotAI* botAI) {
        return new AnubarakMeleeBurrowerStandTrigger(botAI);
    }

    static Trigger* anubarak_burrower_lead(PlayerbotAI* botAI) {
        return new AnubarakBurrowerLeadTrigger(botAI);
    }

    static Trigger* anubarak_kite_cat_drop(PlayerbotAI* botAI) {
        return new AnubarakKiteCatDropTrigger(botAI);
    }

    static Trigger* anubarak_tank_needs_rescue(PlayerbotAI* botAI) {
        return new AnubarakTankNeedsRescueTrigger(botAI);
    }

    static Trigger* anubarak_lust_due(PlayerbotAI* botAI) {
        return new AnubarakLustDueTrigger(botAI);
    }

    static Trigger* anubarak_demonic_circle_due(PlayerbotAI* botAI) {
        return new AnubarakDemonicCircleDueTrigger(botAI);
    }

    static Trigger* anubarak_scarab_on_raid(PlayerbotAI* botAI) {
        return new AnubarakScarabOnRaidTrigger(botAI);
    }

    static Trigger* anubarak_engaged_by_main_tank(PlayerbotAI* botAI) {
        return new AnubarakEngagedByMainTankTrigger(botAI);
    }

    static Trigger* anubarak_penetrating_cold_on_raid(PlayerbotAI* botAI) {
        return new AnubarakPenetratingColdOnRaidTrigger(botAI);
    }

    static Trigger* anubarak_swarm_heal_urgent(PlayerbotAI* botAI) {
        return new AnubarakSwarmHealUrgentTrigger(botAI);
    }

    static Trigger* anubarak_swarm_heal(PlayerbotAI* botAI) {
        return new AnubarakSwarmHealTrigger(botAI);
    }

    static Trigger* anubarak_aura_mastery_due(PlayerbotAI* botAI) {
        return new AnubarakAuraMasteryDueTrigger(botAI);
    }

    static Trigger* anubarak_divine_plea(PlayerbotAI* botAI) {
        return new AnubarakDivinePleaTrigger(botAI);
    }

    static Trigger* anubarak_innervate_healer(PlayerbotAI* botAI) {
        return new AnubarakInnervateHealerTrigger(botAI);
    }

    static Trigger* anubarak_scarab_kite(PlayerbotAI* botAI) {
        return new AnubarakScarabKiteTrigger(botAI);
    }

    static Trigger* anubarak_mana_top_up(PlayerbotAI* botAI) {
        return new AnubarakManaTopUpTrigger(botAI);
    }

    static Trigger* anubarak_light_judge_due(PlayerbotAI* botAI) {
        return new AnubarakLightJudgeDueTrigger(botAI);
    }

    static Trigger* anubarak_healing_stream_due(PlayerbotAI* botAI) {
        return new AnubarakHealingStreamDueTrigger(botAI);
    }
};

void AddToCAnubarakTriggerNodes(std::vector<TriggerNode*>& triggers);

#endif
