/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_AKMULTIPLIERS_H
#define PLAYERBOTS_AKMULTIPLIERS_H

#include "Multiplier.h"

class ElderNadoxMultiplier : public Multiplier
{
public:
    ElderNadoxMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "elder nadox") {}
    float GetValue(Action* action) override;
};

class JedogaShadowseekerMultiplier : public Multiplier
{
public:
    JedogaShadowseekerMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "jedoga shadowseeker") {}
    float GetValue(Action* action) override;
};

class ForgottenOneMultiplier : public Multiplier
{
public:
    ForgottenOneMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "forgotten one") {}
    float GetValue(Action* action) override;
};

#endif
