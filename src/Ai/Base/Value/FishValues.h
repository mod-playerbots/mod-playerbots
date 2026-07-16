/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_FISHVALUES_H
#define PLAYERBOTS_FISHVALUES_H

#include "Value.h"
#include "TravelMgr.h"
#include "NamedObjectContext.h"

class PlayerbotAI;

class CanFishValue : public BoolCalculatedValue
{
public:
    CanFishValue(PlayerbotAI* botAI) : BoolCalculatedValue(botAI, "can fish") {};
    bool Calculate() override;
};

class CanUseFishingBobberValue : public BoolCalculatedValue
{
public:
    CanUseFishingBobberValue(PlayerbotAI* botAI) : BoolCalculatedValue(botAI, "can use fishing bobber") {};
    bool Calculate() override;
};

class FishingSpotValue : public ManualSetValue<WorldPosition>
{
public:
    FishingSpotValue(PlayerbotAI* botAI, WorldPosition const& pos = WorldPosition(), std::string const& name = "fishing spot")
     : ManualSetValue<WorldPosition>(botAI, pos, name) {}

    void Set(WorldPosition val) override
    {
        value = val;
        _setTime = getMSTime();
    }
    uint32 lastUpdated() const {return _setTime;}
    bool IsStale(uint32 maxDuration) const { return getMSTime() - _setTime > maxDuration; }

private:
    uint32 _setTime = 0;
};

#endif
