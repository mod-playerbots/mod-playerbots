/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RAID_TOCTRIGGERS_ICEHOWL_H
#define PLAYERBOTS_RAID_TOCTRIGGERS_ICEHOWL_H

#include <vector>

#include "NamedObjectContext.h"
#include "ToCTriggers_NorthrendBeasts.h"
#include "Trigger.h"

class IcehowlTankDutyTrigger : public Trigger
{
public:
    IcehowlTankDutyTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "icehowl tank duty") {}
    bool IsActive() override;
};

class IcehowlChargeIncomingTrigger : public Trigger
{
public:
    IcehowlChargeIncomingTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "icehowl charge incoming") {}
    bool IsActive() override;
};

class IcehowlFrothingRageTrigger : public Trigger
{
public:
    IcehowlFrothingRageTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "icehowl frothing rage") {}
    bool IsActive() override;
};

class IcehowlBreathSpreadTrigger : public Trigger
{
public:
    IcehowlBreathSpreadTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "icehowl breath spread") {}
    bool IsActive() override;
};

class IcehowlDazeNitroTrigger : public Trigger
{
public:
    IcehowlDazeNitroTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "icehowl daze nitro") {}
    bool IsActive() override;
};

class ToCIcehowlTriggerContext : public NamedObjectContext<Trigger>
{
public:
    ToCIcehowlTriggerContext() : NamedObjectContext<Trigger>()
    {
        creators["icehowl daze nitro"] =
            &ToCIcehowlTriggerContext::icehowl_daze_nitro;
        creators["icehowl tank duty"] =
            &ToCIcehowlTriggerContext::icehowl_tank_duty;
        creators["icehowl charge incoming"] =
            &ToCIcehowlTriggerContext::icehowl_charge_incoming;
        creators["icehowl frothing rage"] =
            &ToCIcehowlTriggerContext::icehowl_frothing_rage;
        creators["icehowl breath spread"] =
            &ToCIcehowlTriggerContext::icehowl_breath_spread;
        creators["icehowl dps target"] =
            &ToCIcehowlTriggerContext::icehowl_dps_target;
    }

private:
    static Trigger* icehowl_tank_duty(PlayerbotAI* botAI) {
        return new IcehowlTankDutyTrigger(botAI);
    }

    static Trigger* icehowl_charge_incoming(PlayerbotAI* botAI) {
        return new IcehowlChargeIncomingTrigger(botAI);
    }

    static Trigger* icehowl_frothing_rage(PlayerbotAI* botAI) {
        return new IcehowlFrothingRageTrigger(botAI);
    }

    static Trigger* icehowl_breath_spread(PlayerbotAI* botAI) {
        return new IcehowlBreathSpreadTrigger(botAI);
    }

    static Trigger* icehowl_daze_nitro(PlayerbotAI* botAI) {
        return new IcehowlDazeNitroTrigger(botAI);
    }

    static Trigger* icehowl_dps_target(PlayerbotAI* botAI) {
        return new NorthrendBeastsDpsTargetTrigger(botAI, "icehowl dps target", NorthrendBeastsStem::Icehowl);
    }
};

void AddToCIcehowlTriggerNodes(std::vector<TriggerNode*>& triggers);

#endif
