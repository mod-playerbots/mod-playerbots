/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_SPELLIDVALUE_H
#define PLAYERBOTS_SPELLIDVALUE_H

#include "NamedObjectContext.h"
#include "Value.h"

class PlayerbotAI;

class SpellIdValue : public CalculatedValue<uint32>, public Qualified
{
public:
    SpellIdValue(PlayerbotAI* botAI);

    uint32 Calculate() override;
};

class VehicleSpellIdValue : public CalculatedValue<uint32>, public Qualified
{
public:
    VehicleSpellIdValue(PlayerbotAI* botAI);

    uint32 Calculate() override;
};

#endif
