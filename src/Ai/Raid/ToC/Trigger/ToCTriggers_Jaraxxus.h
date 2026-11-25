/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RAID_TOCTRIGGERS_JARAXXUS_H
#define PLAYERBOTS_RAID_TOCTRIGGERS_JARAXXUS_H

#include "Trigger.h"

class JaraxxusFelFireballInterruptibleTrigger : public Trigger
{
public:
    static constexpr char const* Name = "jaraxxus fel fireball interruptible";

    JaraxxusFelFireballInterruptibleTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class JaraxxusLegionFlameNearbyTrigger : public Trigger
{
public:
    static constexpr char const* Name = "jaraxxus legion flame nearby";

    JaraxxusLegionFlameNearbyTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

// A bot mid-cast runs no triggers of its own, so another bot's tick has to spot it.
class JaraxxusPinnedCastTrigger : public Trigger
{
public:
    static constexpr char const* Name = "jaraxxus pinned cast";

    JaraxxusPinnedCastTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class JaraxxusPortalSpawnTrigger : public Trigger
{
public:
    static constexpr char const* Name = "jaraxxus portal spawn";

    JaraxxusPortalSpawnTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

// Every bot's tick runs it, which also drops a finished deal
class JaraxxusRaidCoverTrigger : public Trigger
{
public:
    static constexpr char const* Name = "jaraxxus raid cover";

    JaraxxusRaidCoverTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class JaraxxusPersonalDefensiveTrigger : public Trigger
{
public:
    static constexpr char const* Name = "jaraxxus personal defensive";

    JaraxxusPersonalDefensiveTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class JaraxxusEngagedByMainTankTrigger : public Trigger
{
public:
    static constexpr char const* Name = "jaraxxus engaged by main tank";

    JaraxxusEngagedByMainTankTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class JaraxxusAddNeedsAssistTankTrigger : public Trigger
{
public:
    static constexpr char const* Name = "jaraxxus add needs assist tank";

    JaraxxusAddNeedsAssistTankTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class JaraxxusSecondAddNeedsAssistTankTrigger : public Trigger
{
public:
    static constexpr char const* Name = "jaraxxus second add needs assist tank";

    JaraxxusSecondAddNeedsAssistTankTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class JaraxxusNetherPowerActiveTrigger : public Trigger
{
public:
    static constexpr char const* Name = "jaraxxus nether power active";

    JaraxxusNetherPowerActiveTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class JaraxxusDpsTargetTrigger : public Trigger
{
public:
    static constexpr char const* Name = "jaraxxus dps target";

    JaraxxusDpsTargetTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class JaraxxusIncinerateFleshOnRaidTrigger : public Trigger
{
public:
    static constexpr char const* Name = "jaraxxus incinerate flesh on raid";

    JaraxxusIncinerateFleshOnRaidTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

#endif
