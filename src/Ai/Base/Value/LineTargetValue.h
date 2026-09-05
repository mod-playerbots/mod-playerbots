/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_LINETARGETVALUE_H
#define PLAYERBOTS_LINETARGETVALUE_H

#include "Value.h"

class PlayerbotAI;
class Unit;

class LineTargetValue : public UnitCalculatedValue
{
public:
    LineTargetValue(PlayerbotAI* botAI, std::string const name = "line target") : UnitCalculatedValue(botAI, name) {}

    Unit* Calculate() override;
};

#endif
