/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_WHOACTION_H
#define PLAYERBOTS_WHOACTION_H

#include "InventoryAction.h"

class PlayerbotAI;

class WhoAction : public InventoryAction
{
public:
    WhoAction(PlayerbotAI* botAI) : InventoryAction(botAI, "who") {}

    bool Execute(Event event) override;

private:
    std::string const QueryTrade(std::string const text);
    std::string const QuerySkill(std::string const text);
    std::string const QuerySpec(std::string const text);
};

#endif
