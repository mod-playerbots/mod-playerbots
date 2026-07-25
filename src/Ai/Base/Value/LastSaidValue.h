/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_LASTSAIDVALUE_H
#define PLAYERBOTS_LASTSAIDVALUE_H

#include "NamedObjectContext.h"
#include "Value.h"

class PlayerbotAI;

class LastSaidValue : public ManualSetValue<time_t>, public Qualified
{
public:
    LastSaidValue(PlayerbotAI* botAI) : ManualSetValue<time_t>(botAI, time(nullptr) - 120, "last said") {}
};

class LastEmoteValue : public ManualSetValue<time_t>, public Qualified
{
public:
    LastEmoteValue(PlayerbotAI* botAI) : ManualSetValue<time_t>(botAI, time(nullptr) - 120, "last emote") {}
};

#endif
