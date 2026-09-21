/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_INVENTORYCHANGEFAILUREACTION_H
#define PLAYERBOTS_INVENTORYCHANGEFAILUREACTION_H

#include "Action.h"
#include "Item.h"

class PlayerbotAI;

class InventoryChangeFailureAction : public Action
{
public:
    InventoryChangeFailureAction(PlayerbotAI* botAI) : Action(botAI, "inventory change failure") {}

    bool Execute(Event event) override;

private:
    static std::map<InventoryResult, std::string> messages;
};

#endif
