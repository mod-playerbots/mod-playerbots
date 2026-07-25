/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_PETTARGETVALUE_H
#define PLAYERBOTS_PETTARGETVALUE_H

#include "Value.h"

class PlayerbotAI;
class Unit;

class PetTargetValue : public UnitCalculatedValue
{
public:
    PetTargetValue(PlayerbotAI* botAI, std::string const name = "pet target") : UnitCalculatedValue(botAI, name) {}

    Unit* Calculate() override;
};

#endif
