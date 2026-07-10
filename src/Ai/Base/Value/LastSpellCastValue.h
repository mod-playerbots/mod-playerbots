/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_LASTSPELLCASTVALUE_H
#define PLAYERBOTS_LASTSPELLCASTVALUE_H

#include "Value.h"

class PlayerbotAI;

class LastSpellCast
{
public:
    LastSpellCast();

    void Set(uint32 id, ObjectGuid target, time_t time);
    void Reset();

    uint32 id;
    ObjectGuid target;
    time_t timer;
};

class LastSpellCastValue : public ManualSetValue<LastSpellCast&>
{
public:
    LastSpellCastValue(PlayerbotAI* botAI, std::string const name = "last spell cast")
        : ManualSetValue<LastSpellCast&>(botAI, data, name)
    {
    }

private:
    LastSpellCast data;
};

#endif
