/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef PLAYERBOTS_PULLTRIGGERS_H
#define PLAYERBOTS_PULLTRIGGERS_H

#include "Trigger.h"

class PullStartTrigger : public Trigger
{
public:
    PullStartTrigger(PlayerbotAI* botAI, std::string const name = "pull start") : Trigger(botAI, name) {}

    bool IsActive() override;
};

class PullEndTrigger : public Trigger
{
public:
    PullEndTrigger(PlayerbotAI* botAI, std::string const name = "pull end") : Trigger(botAI, name) {}

    bool IsActive() override;
};

class ReturnToPullPositionTrigger : public Trigger
{
public:
    ReturnToPullPositionTrigger(PlayerbotAI* botAI) : Trigger(botAI, "return to pull position") {}

    bool IsActive() override;
};

#endif
