/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_BWLACTIONS_H
#define _PLAYERBOT_BWLACTIONS_H

#include "Action.h"

// General

class BwlOnyxiaScaleCloakAuraCheckAction : public Action
{
public:
    BwlOnyxiaScaleCloakAuraCheckAction(PlayerbotAI* botAI) : Action(botAI, "bwl onyxia scale cloak aura check") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class BwlTurnOffSuppressionDeviceAction : public Action
{
public:
    BwlTurnOffSuppressionDeviceAction(PlayerbotAI* botAI) : Action(botAI, "bwl turn off suppression device") {}
    bool Execute(Event event) override;
};

// Chromaggus

class BwlUseHourglassSandAction : public Action
{
public:
    BwlUseHourglassSandAction(PlayerbotAI* botAI) : Action(botAI, "bwl use hourglass sand") {}
    bool Execute(Event event) override;
};

class BwlNefarianFearWardAction : public Action
{
public:
    BwlNefarianFearWardAction(PlayerbotAI* botAI) : Action(botAI, "bwl nefarian fear ward") {}
    bool Execute(Event event) override;
};

#endif
