/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_AKMULTIPLIERS_H
#define _PLAYERBOT_AKMULTIPLIERS_H

#include "Multiplier.h"

class ElderNadoxMultiplier : public Multiplier
{
    public:
        ElderNadoxMultiplier(PlayerbotAI* ai) : Multiplier(ai, "elder nadox") {}

    public:
        virtual float GetValue(Action* action);
};

class JedogaShadowseekerMultiplier : public Multiplier
{
    public:
        JedogaShadowseekerMultiplier(PlayerbotAI* ai) : Multiplier(ai, "jedoga shadowseeker") {}

    public:
        virtual float GetValue(Action* action);
};

class ForgottenOneMultiplier : public Multiplier
{
    public:
        ForgottenOneMultiplier(PlayerbotAI* ai) : Multiplier(ai, "forgotten one") {}

    public:
        virtual float GetValue(Action* action);
};

#endif
