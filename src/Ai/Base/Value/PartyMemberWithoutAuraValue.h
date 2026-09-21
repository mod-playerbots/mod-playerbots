/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_PARTYMEMBERWITHOUTAURAVALUE_H
#define PLAYERBOTS_PARTYMEMBERWITHOUTAURAVALUE_H

#include "NamedObjectContext.h"
#include "PartyMemberValue.h"

class PlayerbotAI;
class Unit;

class PartyMemberWithoutAuraValue : public PartyMemberValue, public Qualified
{
public:
    PartyMemberWithoutAuraValue(PlayerbotAI* botAI, std::string const name = "party member without aura")
        : PartyMemberValue(botAI, name)
    {
    }

protected:
    Unit* Calculate() override;
};

#endif
