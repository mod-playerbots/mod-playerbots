/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RAID_TOCTRIGGERS_TWINVALKYR_H
#define PLAYERBOTS_RAID_TOCTRIGGERS_TWINVALKYR_H

#include "Trigger.h"

class TwinValkyrPactInterruptDutyTrigger : public Trigger
{
public:
    static constexpr char const* Name = "twin valkyr pact interrupt duty";

    TwinValkyrPactInterruptDutyTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class TwinValkyrTouchedRequiresEssenceTrigger : public Trigger
{
public:
    static constexpr char const* Name = "twin valkyr touched requires essence";

    TwinValkyrTouchedRequiresEssenceTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class TwinValkyrVortexRequiresEssenceTrigger : public Trigger
{
public:
    static constexpr char const* Name = "twin valkyr vortex requires essence";

    TwinValkyrVortexRequiresEssenceTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class TwinValkyrOrbIncomingTrigger : public Trigger
{
public:
    static constexpr char const* Name = "twin valkyr orb incoming";

    TwinValkyrOrbIncomingTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class TwinValkyrVortexRaidCoverTrigger : public Trigger
{
public:
    static constexpr char const* Name = "twin valkyr vortex raid cover";

    TwinValkyrVortexRaidCoverTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class TwinValkyrVortexDefensiveTrigger : public Trigger
{
public:
    static constexpr char const* Name = "twin valkyr vortex defensive";

    TwinValkyrVortexDefensiveTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class TwinValkyrNextPortalDueTrigger : public Trigger
{
public:
    static constexpr char const* Name = "twin valkyr next portal due";

    TwinValkyrNextPortalDueTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class TwinValkyrNeedsBaseEssenceTrigger : public Trigger
{
public:
    static constexpr char const* Name = "twin valkyr needs base essence";

    TwinValkyrNeedsBaseEssenceTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class TwinValkyrEngagedByMainTankTrigger : public Trigger
{
public:
    static constexpr char const* Name = "twin valkyr engaged by main tank";

    TwinValkyrEngagedByMainTankTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class TwinValkyrDarkbaneNeedsAssistTankTrigger : public Trigger
{
public:
    static constexpr char const* Name = "twin valkyr darkbane needs assist tank";

    TwinValkyrDarkbaneNeedsAssistTankTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class TwinValkyrRedirectThreatTrigger : public Trigger
{
public:
    static constexpr char const* Name = "twin valkyr redirect threat";

    TwinValkyrRedirectThreatTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class TwinValkyrDoorStandTrigger : public Trigger
{
public:
    static constexpr char const* Name = "twin valkyr door stand";

    TwinValkyrDoorStandTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class TwinValkyrDpsTargetTrigger : public Trigger
{
public:
    static constexpr char const* Name = "twin valkyr dps target";

    TwinValkyrDpsTargetTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

#endif
