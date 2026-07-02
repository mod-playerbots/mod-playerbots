/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_TANKTARGETVALUE_H
#define PLAYERBOTS_TANKTARGETVALUE_H

#include "RtiTargetValue.h"

class PlayerbotAI;

class TankTargetValue : public RtiTargetValue
{
public:
    TankTargetValue(PlayerbotAI* botAI, std::string const name = "tank target") : RtiTargetValue(botAI, "rti", name) {}

    Unit* Calculate() override;
};

#endif
