/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_DISTANCEVALUE_H
#define PLAYERBOTS_DISTANCEVALUE_H

#include "NamedObjectContext.h"
#include "Value.h"

class PlayerbotAI;

class DistanceValue : public FloatCalculatedValue, public Qualified
{
public:
    DistanceValue(PlayerbotAI* botAI, std::string const name = "distance") : FloatCalculatedValue(botAI, name) {}

    float Calculate() override;
};

class InsideTargetValue : public BoolCalculatedValue, public Qualified
{
public:
    InsideTargetValue(PlayerbotAI* botAI, std::string const name = "inside target") : BoolCalculatedValue(botAI, name)
    {
    }

    bool Calculate() override;
};

#endif
