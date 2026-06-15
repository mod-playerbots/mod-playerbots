/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_NEXMULTIPLIERS_H
#define _PLAYERBOT_NEXMULTIPLIERS_H

#include "Multiplier.h"

class FactionCommanderMultiplier : public Multiplier
{
    public:
        FactionCommanderMultiplier(PlayerbotAI* ai) : Multiplier(ai, "faction commander") {}

    public:
        virtual float GetValue(Action* action);
};

class TelestraMultiplier : public Multiplier
{
    public:
        TelestraMultiplier(PlayerbotAI* ai) : Multiplier(ai, "grand magus telestra") {}

    public:
        virtual float GetValue(Action* action);
};

class AnomalusMultiplier : public Multiplier
{
    public:
        AnomalusMultiplier(PlayerbotAI* ai) : Multiplier(ai, "anomalus") {}

    public:
        virtual float GetValue(Action* action);
};

class OrmorokMultiplier : public Multiplier
{
    public:
        OrmorokMultiplier(PlayerbotAI* ai) : Multiplier(ai, "ormorok the tree-shaper") {}

    public:
        virtual float GetValue(Action* action);
};

#endif
