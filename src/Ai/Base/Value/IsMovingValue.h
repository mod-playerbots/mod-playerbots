/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ISMOVINGVALUE_H
#define PLAYERBOTS_ISMOVINGVALUE_H

#include "NamedObjectContext.h"
#include "Value.h"

class PlayerbotAI;

class IsMovingValue : public BoolCalculatedValue, public Qualified
{
public:
    IsMovingValue(PlayerbotAI* botAI, std::string const name = "is moving") : BoolCalculatedValue(botAI, name) {}

    bool Calculate() override;
};

class IsSwimmingValue : public BoolCalculatedValue, public Qualified
{
public:
    IsSwimmingValue(PlayerbotAI* botAI, std::string const name = "is swimming") : BoolCalculatedValue(botAI, name) {}

    bool Calculate() override;
};

#endif
