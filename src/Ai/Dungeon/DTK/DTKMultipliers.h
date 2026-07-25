/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_DTKMULTIPLIERS_H
#define PLAYERBOTS_DTKMULTIPLIERS_H

#include "Multiplier.h"

class NovosMultiplier : public Multiplier
{
    public:
        NovosMultiplier(PlayerbotAI* ai) : Multiplier(ai, "novos the summoner") {}

    public:
        virtual float GetValue(Action* action);
};

class TharonjaMultiplier : public Multiplier
{
    public:
        TharonjaMultiplier(PlayerbotAI* ai) : Multiplier(ai, "the prophet tharon'ja") {}

    public:
        virtual float GetValue(Action* action);
};

#endif
