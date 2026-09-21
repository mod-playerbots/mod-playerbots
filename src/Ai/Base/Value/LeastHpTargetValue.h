/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_LEASTHPTARGETVALUE_H
#define PLAYERBOTS_LEASTHPTARGETVALUE_H

#include "TargetValue.h"

class PlayerbotAI;
class Unit;

class LeastHpTargetValue : public TargetValue
{
public:
    LeastHpTargetValue(PlayerbotAI* botAI, std::string const name = "least hp target") : TargetValue(botAI, name) {}

    Unit* Calculate() override;
};

#endif
