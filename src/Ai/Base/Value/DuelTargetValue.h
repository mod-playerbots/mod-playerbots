/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_DUELTARGETVALUE_H
#define PLAYERBOTS_DUELTARGETVALUE_H

#include "TargetValue.h"

class PlayerbotAI;
class Unit;

class DuelTargetValue : public TargetValue
{
public:
    DuelTargetValue(PlayerbotAI* botAI, std::string const name = "duel target") : TargetValue(botAI, name) {}

    Unit* Calculate() override;
};

#endif
