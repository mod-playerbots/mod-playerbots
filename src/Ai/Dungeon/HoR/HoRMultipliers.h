/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_HORMULTIPLIERS_H
#define PLAYERBOTS_HORMULTIPLIERS_H

#include "Multiplier.h"

class PlayerbotAI;

// During the escape event the bots must never pick the Lich King as a target
// (he cannot be killed) and must not waste time on their default assist
// target. The multiplier disables the standard target-choosing actions while
// the custom escape actions take over.
class HorEscapeMultiplier : public Multiplier
{
public:
    HorEscapeMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "hor escape multiplier") {}
    float GetValue(Action* action) override;
};

#endif
