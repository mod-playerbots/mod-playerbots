/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_SKIPSPELLSLISTVALUE_H
#define PLAYERBOTS_SKIPSPELLSLISTVALUE_H

#include <set>

#include "Value.h"

class PlayerbotAI;

class SkipSpellsListValue : public ManualSetValue<std::set<uint32>&>
{
public:
    SkipSpellsListValue(PlayerbotAI* botAI, std::string const name = "skip spells list")
        : ManualSetValue<std::set<uint32>&>(botAI, list, name)
    {
    }

    std::string const Save() override;
    bool Load(std::string const value) override;

private:
    std::set<uint32> list;
};

#endif
