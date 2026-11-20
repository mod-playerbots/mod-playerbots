/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RAID_TOCTRIGGERS_ICEHOWL_H
#define PLAYERBOTS_RAID_TOCTRIGGERS_ICEHOWL_H

#include "ToCTriggers_NorthrendBeasts.h"
#include "Trigger.h"

class IcehowlTankDutyTrigger : public Trigger
{
public:
    static constexpr char const* Name = "icehowl tank duty";

    IcehowlTankDutyTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class IcehowlChargeIncomingTrigger : public Trigger
{
public:
    static constexpr char const* Name = "icehowl charge incoming";

    IcehowlChargeIncomingTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class IcehowlFrothingRageTrigger : public Trigger
{
public:
    static constexpr char const* Name = "icehowl frothing rage";

    IcehowlFrothingRageTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class IcehowlBreathSpreadTrigger : public Trigger
{
public:
    static constexpr char const* Name = "icehowl breath spread";

    IcehowlBreathSpreadTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class IcehowlDazeNitroTrigger : public Trigger
{
public:
    static constexpr char const* Name = "icehowl daze nitro";

    IcehowlDazeNitroTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

#endif
