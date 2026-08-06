/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_UKMULTIPLIERS_H
#define PLAYERBOTS_UKMULTIPLIERS_H

#include "Multiplier.h"

class PrinceKelesethMultiplier : public Multiplier
{
    public:
        PrinceKelesethMultiplier(PlayerbotAI* ai) : Multiplier(ai, "prince keleseth") {}

    public:
        virtual float GetValue(Action* action);
};

class SkarvaldAndDalronnMultiplier : public Multiplier
{
    public:
        SkarvaldAndDalronnMultiplier(PlayerbotAI* ai) : Multiplier(ai, "skarvald and dalronn") {}

    public:
        virtual float GetValue(Action* action);
};

class IngvarThePlundererMultiplier : public Multiplier
{
    public:
        IngvarThePlundererMultiplier(PlayerbotAI* ai) : Multiplier(ai, "ingvar the plunderer") {}

    public:
        virtual float GetValue(Action* action);
};

#endif
