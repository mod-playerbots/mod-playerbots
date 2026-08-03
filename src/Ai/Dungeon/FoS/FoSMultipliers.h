/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_FOSMULTIPLIERS_H
#define PLAYERBOTS_FOSMULTIPLIERS_H

#include "Multiplier.h"

class BronjahmMultiplier : public Multiplier
{
    public:
    BronjahmMultiplier(PlayerbotAI* ai) : Multiplier(ai, "bronjahm") {}

    public:
        virtual float GetValue(Action* action);
};

class AttackFragmentMultiplier : public Multiplier
{
public:
    AttackFragmentMultiplier(PlayerbotAI* ai) : Multiplier(ai, "attack fragment") { }

    float GetValue(Action* action) override;
};

#endif
