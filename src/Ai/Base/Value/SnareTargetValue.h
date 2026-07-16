/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_SNARETARGETVALUE_H
#define PLAYERBOTS_SNARETARGETVALUE_H

#include "NamedObjectContext.h"
#include "Value.h"

class PlayerbotAI;
class Unit;

class SnareTargetValue : public UnitCalculatedValue, public Qualified
{
public:
    SnareTargetValue(PlayerbotAI* botAI) : UnitCalculatedValue(botAI, "snare target", 1) {}

protected:
    Unit* Calculate() override;
};

#endif
