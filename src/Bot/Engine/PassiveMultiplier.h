/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_PASSIVEMULTIPLIER_H
#define PLAYERBOTS_PASSIVEMULTIPLIER_H

#include <vector>

#include "Multiplier.h"

class Action;
class PlayerbotAI;

class PassiveMultiplier : public Multiplier
{
public:
    PassiveMultiplier(PlayerbotAI* botAI);

    float GetValue(Action* action) override;

private:
    static std::vector<std::string> allowedActions;
    static std::vector<std::string> allowedParts;
};

#endif
