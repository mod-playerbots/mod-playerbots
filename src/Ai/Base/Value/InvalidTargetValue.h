/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_INVALIDTARGETVALUE_H
#define PLAYERBOTS_INVALIDTARGETVALUE_H

#include "NamedObjectContext.h"
#include "Value.h"

class PlayerbotAI;

class InvalidTargetValue : public BoolCalculatedValue, public Qualified
{
public:
    InvalidTargetValue(PlayerbotAI* botAI, std::string const name = "invalid target") : BoolCalculatedValue(botAI, name)
    {
    }

    bool Calculate() override;
};

#endif
