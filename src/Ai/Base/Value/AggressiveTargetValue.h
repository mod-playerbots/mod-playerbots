/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_AGGRESSIVETARGETVALUE_H
#define PLAYERBOTS_AGGRESSIVETARGETVALUE_H

#include "TargetValue.h"

class PlayerbotAI;
class Unit;

class AggressiveTargetValue : public TargetValue
{
public:
    AggressiveTargetValue(PlayerbotAI* botAI, std::string const name = "aggressive target") : TargetValue(botAI, name) {}

    Unit* Calculate() override;
};

#endif
