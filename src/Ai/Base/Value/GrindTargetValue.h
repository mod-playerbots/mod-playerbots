/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_GRINDTARGETVALUE_H
#define PLAYERBOTS_GRINDTARGETVALUE_H

#include "TargetValue.h"

class PlayerbotAI;
class Unit;

class GrindTargetValue : public TargetValue
{
public:
    GrindTargetValue(PlayerbotAI* botAI, std::string const name = "grind target") : TargetValue(botAI, name) {}

    Unit* Calculate() override;

private:
    uint32 GetTargetingPlayerCount(Unit* unit);
    Unit* FindTargetForGrinding(uint32 assistCount);
    bool needForQuest(Unit* target);
};

#endif
