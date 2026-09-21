/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RANDOMBOTUPDATEVALUE_H
#define PLAYERBOTS_RANDOMBOTUPDATEVALUE_H

#include "Value.h"

class PlayerbotAI;

class RandomBotUpdateValue : public ManualSetValue<bool>
{
public:
    RandomBotUpdateValue(PlayerbotAI* botAI, std::string const name = "random bot update")
        : ManualSetValue<bool>(botAI, false, name)
    {
    }
};

#endif
