/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_PARTYMEMBERSNAREDTARGETVALUE_H
#define _PLAYERBOT_PARTYMEMBERSNAREDTARGETVALUE_H

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
