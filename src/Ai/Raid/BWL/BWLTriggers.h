/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_BWLTRIGGERS_H
#define _PLAYERBOT_BWLTRIGGERS_H

#include "Trigger.h"

// General

class BwlSuppressionDeviceTrigger : public Trigger
{
public:
    BwlSuppressionDeviceTrigger(PlayerbotAI* botAI) : Trigger(botAI, "bwl suppression device") {}
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

#endif
