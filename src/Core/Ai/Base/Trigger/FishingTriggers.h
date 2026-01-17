/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_FISHING_TRIGGER_H
#define _PLAYERBOT_FISHING_TRIGGER_H

#include "GenericTriggers.h"

class CanFishTrigger : public Trigger
{
public:
    CanFishTrigger(PlayerbotAI* ai) : Trigger(ai, "can fish") {};
    bool IsActive() override;
};

class CanUseFishingBobberTrigger : public Trigger
{
public:
    CanUseFishingBobberTrigger(PlayerbotAI* ai) : Trigger(ai, "can use fishing bobber") {};
    bool IsActive() override;
};

#endif
