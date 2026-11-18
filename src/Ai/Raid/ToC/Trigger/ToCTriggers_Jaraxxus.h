/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RAID_TOCTRIGGERS_JARAXXUS_H
#define PLAYERBOTS_RAID_TOCTRIGGERS_JARAXXUS_H

#include <vector>

#include "NamedObjectContext.h"
#include "Trigger.h"

class JaraxxusFelFireballInterruptibleTrigger : public Trigger
{
public:
    JaraxxusFelFireballInterruptibleTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "jaraxxus fel fireball interruptible") {}
    bool IsActive() override;
};

class JaraxxusLegionFlameNearbyTrigger : public Trigger
{
public:
    JaraxxusLegionFlameNearbyTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "jaraxxus legion flame nearby") {}
    bool IsActive() override;
};

// A bot mid-cast runs no triggers of its own, so another bot's tick has to spot it.
class JaraxxusPinnedCastTrigger : public Trigger
{
public:
    JaraxxusPinnedCastTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "jaraxxus pinned cast") {}
    bool IsActive() override;
};

class JaraxxusPortalSpawnTrigger : public Trigger
{
public:
    JaraxxusPortalSpawnTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "jaraxxus portal spawn") {}
    bool IsActive() override;
};

// Every bot's tick runs it, which also drops a finished deal
class JaraxxusRaidCoverTrigger : public Trigger
{
public:
    JaraxxusRaidCoverTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "jaraxxus raid cover") {}
    bool IsActive() override;
};

class JaraxxusPersonalDefensiveTrigger : public Trigger
{
public:
    JaraxxusPersonalDefensiveTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "jaraxxus personal defensive") {}
    bool IsActive() override;
};

class JaraxxusEngagedByMainTankTrigger : public Trigger
{
public:
    JaraxxusEngagedByMainTankTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "jaraxxus engaged by main tank") {}
    bool IsActive() override;
};

class JaraxxusAddNeedsAssistTankTrigger : public Trigger
{
public:
    JaraxxusAddNeedsAssistTankTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "jaraxxus add needs assist tank") {}
    bool IsActive() override;
};

class JaraxxusSecondAddNeedsAssistTankTrigger : public Trigger
{
public:
    JaraxxusSecondAddNeedsAssistTankTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "jaraxxus second add needs assist tank") {}
    bool IsActive() override;
};

class JaraxxusNetherPowerActiveTrigger : public Trigger
{
public:
    JaraxxusNetherPowerActiveTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "jaraxxus nether power active") {}
    bool IsActive() override;
};

class JaraxxusDpsTargetTrigger : public Trigger
{
public:
    JaraxxusDpsTargetTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "jaraxxus dps target") {}
    bool IsActive() override;
};

class JaraxxusIncinerateFleshOnRaidTrigger : public Trigger
{
public:
    JaraxxusIncinerateFleshOnRaidTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "jaraxxus incinerate flesh on raid") {}
    bool IsActive() override;
};

class ToCJaraxxusTriggerContext : public NamedObjectContext<Trigger>
{
public:
    ToCJaraxxusTriggerContext() : NamedObjectContext<Trigger>()
    {
        creators["jaraxxus fel fireball interruptible"] =
            &ToCJaraxxusTriggerContext::jaraxxus_fel_fireball_interruptible;
        creators["jaraxxus legion flame nearby"] =
            &ToCJaraxxusTriggerContext::jaraxxus_legion_flame_nearby;
        creators["jaraxxus pinned cast"] =
            &ToCJaraxxusTriggerContext::jaraxxus_pinned_cast;
        creators["jaraxxus portal spawn"] =
            &ToCJaraxxusTriggerContext::jaraxxus_portal_spawn;
        creators["jaraxxus raid cover"] =
            &ToCJaraxxusTriggerContext::jaraxxus_raid_cover;
        creators["jaraxxus personal defensive"] =
            &ToCJaraxxusTriggerContext::jaraxxus_personal_defensive;
        creators["jaraxxus engaged by main tank"] =
            &ToCJaraxxusTriggerContext::jaraxxus_engaged_by_main_tank;
        creators["jaraxxus add needs assist tank"] =
            &ToCJaraxxusTriggerContext::jaraxxus_add_needs_assist_tank;
        creators["jaraxxus second add needs assist tank"] =
            &ToCJaraxxusTriggerContext::jaraxxus_second_add_needs_assist_tank;
        creators["jaraxxus nether power active"] =
            &ToCJaraxxusTriggerContext::jaraxxus_nether_power_active;
        creators["jaraxxus dps target"] =
            &ToCJaraxxusTriggerContext::jaraxxus_dps_target;
        creators["jaraxxus incinerate flesh on raid"] =
            &ToCJaraxxusTriggerContext::jaraxxus_incinerate_flesh_on_raid;
    }

private:
    static Trigger* jaraxxus_fel_fireball_interruptible(PlayerbotAI* botAI) {
        return new JaraxxusFelFireballInterruptibleTrigger(botAI);
    }

    static Trigger* jaraxxus_legion_flame_nearby(PlayerbotAI* botAI) {
        return new JaraxxusLegionFlameNearbyTrigger(botAI);
    }

    static Trigger* jaraxxus_pinned_cast(PlayerbotAI* botAI) {
        return new JaraxxusPinnedCastTrigger(botAI);
    }

    static Trigger* jaraxxus_portal_spawn(PlayerbotAI* botAI) {
        return new JaraxxusPortalSpawnTrigger(botAI);
    }

    static Trigger* jaraxxus_raid_cover(PlayerbotAI* botAI) {
        return new JaraxxusRaidCoverTrigger(botAI);
    }

    static Trigger* jaraxxus_personal_defensive(PlayerbotAI* botAI) {
        return new JaraxxusPersonalDefensiveTrigger(botAI);
    }

    static Trigger* jaraxxus_engaged_by_main_tank(PlayerbotAI* botAI) {
        return new JaraxxusEngagedByMainTankTrigger(botAI);
    }

    static Trigger* jaraxxus_add_needs_assist_tank(PlayerbotAI* botAI) {
        return new JaraxxusAddNeedsAssistTankTrigger(botAI);
    }

    static Trigger* jaraxxus_second_add_needs_assist_tank(PlayerbotAI* botAI) {
        return new JaraxxusSecondAddNeedsAssistTankTrigger(botAI);
    }

    static Trigger* jaraxxus_nether_power_active(PlayerbotAI* botAI) {
        return new JaraxxusNetherPowerActiveTrigger(botAI);
    }

    static Trigger* jaraxxus_dps_target(PlayerbotAI* botAI) {
        return new JaraxxusDpsTargetTrigger(botAI);
    }

    static Trigger* jaraxxus_incinerate_flesh_on_raid(PlayerbotAI* botAI) {
        return new JaraxxusIncinerateFleshOnRaidTrigger(botAI);
    }
};

void AddToCJaraxxusTriggerNodes(std::vector<TriggerNode*>& triggers);

#endif
