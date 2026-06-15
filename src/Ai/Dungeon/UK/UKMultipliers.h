/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_UKMULTIPLIERS_H
#define _PLAYERBOT_UKMULTIPLIERS_H

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
