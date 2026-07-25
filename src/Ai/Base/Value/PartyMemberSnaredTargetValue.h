/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_PARTYMEMBERSNAREDTARGETVALUE_H
#define PLAYERBOTS_PARTYMEMBERSNAREDTARGETVALUE_H

#include "NamedObjectContext.h"
#include "PartyMemberValue.h"

class PartyMemberSnaredTargetValue : public PartyMemberValue
{
public:
    PartyMemberSnaredTargetValue(PlayerbotAI* botAI, std::string const name = "party member snared target")
        : PartyMemberValue(botAI, name)
    {
    }

protected:
    Unit* Calculate() override;
};

#endif
