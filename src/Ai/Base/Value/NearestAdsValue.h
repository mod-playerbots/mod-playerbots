/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_NEARESTADSVALUE_H
#define PLAYERBOTS_NEARESTADSVALUE_H

#include "PlayerbotAIConfig.h"
#include "PossibleTargetsValue.h"

class PlayerbotAI;

class NearestAddsValue : public PossibleTargetsValue
{
public:
    NearestAddsValue(PlayerbotAI* botAI, float range = sPlayerbotAIConfig.tooCloseDistance)
        : PossibleTargetsValue(botAI, "nearest adds", range, true)
    {
    }

protected:
    bool AcceptUnit(Unit* unit) override;
};

#endif
