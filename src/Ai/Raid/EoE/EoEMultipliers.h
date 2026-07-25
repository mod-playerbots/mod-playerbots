/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_EOEMULTIPLIERS_H
#define PLAYERBOTS_EOEMULTIPLIERS_H

#include "Multiplier.h"

class MalygosMultiplier : public Multiplier
{
public:
    MalygosMultiplier(PlayerbotAI* ai) : Multiplier(ai, "malygos") {}

public:
    virtual float GetValue(Action* action);
};

#endif
