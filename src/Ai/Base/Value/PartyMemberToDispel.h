/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_PARTYMEMBERTODISPEL_H
#define PLAYERBOTS_PARTYMEMBERTODISPEL_H

#include "NamedObjectContext.h"
#include "PartyMemberValue.h"

class PlayerbotAI;
class Unit;

class PartyMemberToDispel : public PartyMemberValue, public Qualified
{
public:
    PartyMemberToDispel(PlayerbotAI* botAI, std::string const name = "party member to dispel")
        : PartyMemberValue(botAI, name, 1000), Qualified()
    {
    }

protected:
    Unit* Calculate() override;
};

#endif
