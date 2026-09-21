/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_SPELLCASTUSEFULVALUE_H
#define PLAYERBOTS_SPELLCASTUSEFULVALUE_H

#include "NamedObjectContext.h"
#include "Value.h"

class PlayerbotAI;

class SpellCastUsefulValue : public BoolCalculatedValue, public Qualified
{
public:
    SpellCastUsefulValue(PlayerbotAI* botAI, std::string const name = "spell cast useful")
        : BoolCalculatedValue(botAI, name)
    {
    }

    bool Calculate() override;
};

#endif
