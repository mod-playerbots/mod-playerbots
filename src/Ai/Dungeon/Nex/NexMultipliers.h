/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_NEXMULTIPLIERS_H
#define PLAYERBOTS_NEXMULTIPLIERS_H

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
