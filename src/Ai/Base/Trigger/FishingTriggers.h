/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef PLAYERBOTS_FISHINGTRIGGERS_H
#define PLAYERBOTS_FISHINGTRIGGERS_H

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
