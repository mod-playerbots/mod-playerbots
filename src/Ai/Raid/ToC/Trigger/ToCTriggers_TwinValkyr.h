/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RAID_TOCTRIGGERS_TWINVALKYR_H
#define PLAYERBOTS_RAID_TOCTRIGGERS_TWINVALKYR_H

#include <vector>

#include "NamedObjectContext.h"
#include "Trigger.h"

class TwinValkyrPactInterruptDutyTrigger : public Trigger
{
public:
    TwinValkyrPactInterruptDutyTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "twin valkyr pact interrupt duty") {}
    bool IsActive() override;
};

class TwinValkyrTouchedRequiresEssenceTrigger : public Trigger
{
public:
    TwinValkyrTouchedRequiresEssenceTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "twin valkyr touched requires essence") {}
    bool IsActive() override;
};

class TwinValkyrVortexRequiresEssenceTrigger : public Trigger
{
public:
    TwinValkyrVortexRequiresEssenceTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "twin valkyr vortex requires essence") {}
    bool IsActive() override;
};

class TwinValkyrOrbIncomingTrigger : public Trigger
{
public:
    TwinValkyrOrbIncomingTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "twin valkyr orb incoming") {}
    bool IsActive() override;
};

class TwinValkyrVortexRaidCoverTrigger : public Trigger
{
public:
    TwinValkyrVortexRaidCoverTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "twin valkyr vortex raid cover") {}
    bool IsActive() override;
};

class TwinValkyrVortexDefensiveTrigger : public Trigger
{
public:
    TwinValkyrVortexDefensiveTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "twin valkyr vortex defensive") {}
    bool IsActive() override;
};

class TwinValkyrNextPortalDueTrigger : public Trigger
{
public:
    TwinValkyrNextPortalDueTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "twin valkyr next portal due") {}
    bool IsActive() override;
};

class TwinValkyrNeedsBaseEssenceTrigger : public Trigger
{
public:
    TwinValkyrNeedsBaseEssenceTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "twin valkyr needs base essence") {}
    bool IsActive() override;
};

class TwinValkyrEngagedByMainTankTrigger : public Trigger
{
public:
    TwinValkyrEngagedByMainTankTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "twin valkyr engaged by main tank") {}
    bool IsActive() override;
};

class TwinValkyrDarkbaneNeedsAssistTankTrigger : public Trigger
{
public:
    TwinValkyrDarkbaneNeedsAssistTankTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "twin valkyr darkbane needs assist tank") {}
    bool IsActive() override;
};

class TwinValkyrRedirectThreatTrigger : public Trigger
{
public:
    TwinValkyrRedirectThreatTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "twin valkyr redirect threat") {}
    bool IsActive() override;
};

class TwinValkyrDoorStandTrigger : public Trigger
{
public:
    TwinValkyrDoorStandTrigger(PlayerbotAI* botAI) : Trigger(botAI, "twin valkyr door stand") {}
    bool IsActive() override;
};

class TwinValkyrDpsTargetTrigger : public Trigger
{
public:
    TwinValkyrDpsTargetTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "twin valkyr dps target") {}
    bool IsActive() override;
};

class ToCTwinValkyrTriggerContext : public NamedObjectContext<Trigger>
{
public:
    ToCTwinValkyrTriggerContext() : NamedObjectContext<Trigger>()
    {
        creators["twin valkyr pact interrupt duty"] =
            &ToCTwinValkyrTriggerContext::twin_valkyr_pact_interrupt_duty;
        creators["twin valkyr touched requires essence"] =
            &ToCTwinValkyrTriggerContext::twin_valkyr_touched_requires_essence;
        creators["twin valkyr vortex requires essence"] =
            &ToCTwinValkyrTriggerContext::twin_valkyr_vortex_requires_essence;
        creators["twin valkyr orb incoming"] =
            &ToCTwinValkyrTriggerContext::twin_valkyr_orb_incoming;
        creators["twin valkyr vortex raid cover"] =
            &ToCTwinValkyrTriggerContext::twin_valkyr_vortex_raid_cover;
        creators["twin valkyr vortex defensive"] =
            &ToCTwinValkyrTriggerContext::twin_valkyr_vortex_defensive;
        creators["twin valkyr next portal due"] =
            &ToCTwinValkyrTriggerContext::twin_valkyr_next_portal_due;
        creators["twin valkyr needs base essence"] =
            &ToCTwinValkyrTriggerContext::twin_valkyr_needs_base_essence;
        creators["twin valkyr engaged by main tank"] =
            &ToCTwinValkyrTriggerContext::twin_valkyr_engaged_by_main_tank;
        creators["twin valkyr darkbane needs assist tank"] =
            &ToCTwinValkyrTriggerContext::twin_valkyr_darkbane_needs_assist_tank;
        creators["twin valkyr redirect threat"] =
            &ToCTwinValkyrTriggerContext::twin_valkyr_redirect_threat;
        creators["twin valkyr door stand"] =
            &ToCTwinValkyrTriggerContext::twin_valkyr_door_stand;
        creators["twin valkyr dps target"] =
            &ToCTwinValkyrTriggerContext::twin_valkyr_dps_target;
    }

private:
    static Trigger* twin_valkyr_pact_interrupt_duty(PlayerbotAI* botAI) {
        return new TwinValkyrPactInterruptDutyTrigger(botAI);
    }

    static Trigger* twin_valkyr_touched_requires_essence(PlayerbotAI* botAI) {
        return new TwinValkyrTouchedRequiresEssenceTrigger(botAI);
    }

    static Trigger* twin_valkyr_vortex_requires_essence(PlayerbotAI* botAI) {
        return new TwinValkyrVortexRequiresEssenceTrigger(botAI);
    }

    static Trigger* twin_valkyr_orb_incoming(PlayerbotAI* botAI) {
        return new TwinValkyrOrbIncomingTrigger(botAI);
    }

    static Trigger* twin_valkyr_vortex_raid_cover(PlayerbotAI* botAI) {
        return new TwinValkyrVortexRaidCoverTrigger(botAI);
    }

    static Trigger* twin_valkyr_vortex_defensive(PlayerbotAI* botAI) {
        return new TwinValkyrVortexDefensiveTrigger(botAI);
    }

    static Trigger* twin_valkyr_next_portal_due(PlayerbotAI* botAI) {
        return new TwinValkyrNextPortalDueTrigger(botAI);
    }

    static Trigger* twin_valkyr_needs_base_essence(PlayerbotAI* botAI) {
        return new TwinValkyrNeedsBaseEssenceTrigger(botAI);
    }

    static Trigger* twin_valkyr_engaged_by_main_tank(PlayerbotAI* botAI) {
        return new TwinValkyrEngagedByMainTankTrigger(botAI);
    }

    static Trigger* twin_valkyr_darkbane_needs_assist_tank(PlayerbotAI* botAI) {
        return new TwinValkyrDarkbaneNeedsAssistTankTrigger(botAI);
    }

    static Trigger* twin_valkyr_redirect_threat(PlayerbotAI* botAI) {
        return new TwinValkyrRedirectThreatTrigger(botAI);
    }

    static Trigger* twin_valkyr_door_stand(PlayerbotAI* botAI) {
        return new TwinValkyrDoorStandTrigger(botAI);
    }

    static Trigger* twin_valkyr_dps_target(PlayerbotAI* botAI) {
        return new TwinValkyrDpsTargetTrigger(botAI);
    }
};

void AddToCTwinValkyrTriggerNodes(std::vector<TriggerNode*>& triggers);

#endif
