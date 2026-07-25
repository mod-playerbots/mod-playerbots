/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_DPSTARGETVALUE_H
#define PLAYERBOTS_DPSTARGETVALUE_H

#include "RtiTargetValue.h"

class PlayerbotAI;

class DpsTargetValue : public RtiTargetValue
{
public:
    DpsTargetValue(PlayerbotAI* botAI, std::string const type = "rti", std::string const name = "dps target")
        : RtiTargetValue(botAI, type, name)
    {
    }

    Unit* Calculate() override;
};

class DpsAoeTargetValue : public RtiTargetValue
{
public:
    DpsAoeTargetValue(PlayerbotAI* botAI, std::string const type = "rti", std::string const name = "dps aoe target")
        : RtiTargetValue(botAI, type, name)
    {
    }

    Unit* Calculate() override;
};

#endif
