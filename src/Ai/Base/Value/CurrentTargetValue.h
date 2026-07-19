/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_CURRENTTARGETVALUE_H
#define PLAYERBOTS_CURRENTTARGETVALUE_H

#include "Value.h"

class PlayerbotAI;
class Unit;

class CurrentTargetValue : public UnitManualSetValue
{
public:
    CurrentTargetValue(PlayerbotAI* botAI, std::string const name = "current target")
        : UnitManualSetValue(botAI, nullptr, name)
    {
    }

    Unit* Get() override;
    void Set(Unit* unit) override;

private:
    ObjectGuid selection;
};

#endif
