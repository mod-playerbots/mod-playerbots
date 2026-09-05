/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RANGEVALUES_H
#define PLAYERBOTS_RANGEVALUES_H

#include "NamedObjectContext.h"
#include "Value.h"

class PlayerbotAI;

class RangeValue : public ManualSetValue<float>, public Qualified
{
public:
    RangeValue(PlayerbotAI* botAI);

    std::string const Save() override;
    bool Load(std::string const value) override;
};

#endif
