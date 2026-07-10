/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_OUTFITLISTVALUE_H
#define PLAYERBOTS_OUTFITLISTVALUE_H

#include "Value.h"

class PlayerbotAI;

typedef std::vector<std::string> Outfit;

class OutfitListValue : public ManualSetValue<Outfit&>
{
public:
    OutfitListValue(PlayerbotAI* botAI, std::string const name = "outfit list")
        : ManualSetValue<Outfit&>(botAI, list, name)
    {
    }

    std::string const Save() override;
    bool Load(std::string const value) override;

private:
    Outfit list;
};

#endif
