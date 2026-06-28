#ifndef PLAYERBOTS_BWLTRIGGERS_H
#define PLAYERBOTS_BWLTRIGGERS_H

#include "Trigger.h"

// General

class BwlSuppressionDeviceTrigger : public Trigger
{
public:
    BwlSuppressionDeviceTrigger(PlayerbotAI* botAI) : Trigger(botAI, "bwl suppression device") {}
    bool IsActive() override;
};

// Razorgore the Untamed

class BwlRazorgoreNotMindControlledTrigger : public Trigger
{
public:
    BwlRazorgoreNotMindControlledTrigger(PlayerbotAI* botAI, const std::string& name = "bwl razorgore not mind controlled") : Trigger(botAI, name) {}
    bool IsActive() override;
};

// Vaelastrasz the Corrupt

class BwlVaelastraszPositioningTrigger : public Trigger
{
public:
    BwlVaelastraszPositioningTrigger(PlayerbotAI* botAI) : Trigger(botAI, "bwl vaelastrasz positioning") {}
    bool IsActive() override;
};

class BwlVaelastraszBurningAdrenalineTrigger : public Trigger
{
public:
    BwlVaelastraszBurningAdrenalineTrigger(PlayerbotAI* botAI) : Trigger(botAI, "bwl vaelastrasz burning adrenaline") {}
    bool IsActive() override;
};

// Chromaggus

class BwlAfflictionBronzeTrigger : public Trigger
{
public:
    BwlAfflictionBronzeTrigger(PlayerbotAI* botAI) : Trigger(botAI, "bwl affliction bronze") {}
    bool IsActive() override;
};

// Nefarian

class BwlWildMagicTrigger : public Trigger
{
public:
    BwlWildMagicTrigger(PlayerbotAI* botAI) : Trigger(botAI, "bwl wild magic") {}
    bool IsActive() override;
};

class BwlNefarianFearWardTrigger : public Trigger
{
public:
    BwlNefarianFearWardTrigger(PlayerbotAI* botAI) : Trigger(botAI, "bwl nefarian fear ward") {}
    bool IsActive() override;
};

// Trash

class BwlDeathTalonWyrmguardTankTrigger : public Trigger
{
public:
    BwlDeathTalonWyrmguardTankTrigger(PlayerbotAI* botAI) : Trigger(botAI, "bwl death talon wyrmguard tank") {}
    bool IsActive() override;
};

class BwlDeathTalonWyrmguardRangedTrigger : public Trigger
{
public:
    BwlDeathTalonWyrmguardRangedTrigger(PlayerbotAI* botAI) : Trigger(botAI, "bwl death talon wyrmguard ranged") {}
    bool IsActive() override;
};

#endif
