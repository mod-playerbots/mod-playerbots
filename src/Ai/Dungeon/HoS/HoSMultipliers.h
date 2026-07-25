/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_HOSMULTIPLIERS_H
#define PLAYERBOTS_HOSMULTIPLIERS_H

#include "Multiplier.h"

class KrystallusMultiplier : public Multiplier
{
    public:
        KrystallusMultiplier(PlayerbotAI* ai) : Multiplier(ai, "krystallus") {}

    public:
        virtual float GetValue(Action* action);
};

class SjonnirMultiplier : public Multiplier
{
    public:
        SjonnirMultiplier(PlayerbotAI* ai) : Multiplier(ai, "sjonnir the ironshaper") {}

    public:
        virtual float GetValue(Action* action);
};

#endif
