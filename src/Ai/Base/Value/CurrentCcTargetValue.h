/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_CURRENTCCTARGETVALUE_H
#define PLAYERBOTS_CURRENTCCTARGETVALUE_H

#include "NamedObjectContext.h"
#include "TargetValue.h"

class PlayerbotAI;
class Unit;

class CurrentCcTargetValue : public TargetValue, public Qualified
{
public:
    CurrentCcTargetValue(PlayerbotAI* botAI, std::string const name = "current cc target") : TargetValue(botAI, name) {}

    Unit* Calculate() override;
};

#endif
