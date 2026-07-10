/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_CUSTOMSTRATEGY_H
#define PLAYERBOTS_CUSTOMSTRATEGY_H

#include <map>

#include "Strategy.h"

class PlayerbotAI;

class CustomStrategy : public Strategy, public Qualified
{
public:
    CustomStrategy(PlayerbotAI* botAI);

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return std::string("custom::" + qualifier); }
    void Reset();

    static std::map<std::string, std::string> actionLinesCache;

private:
    std::vector<std::string> actionLines;
    void LoadActionLines(uint32 owner);
};

#endif
