/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_OPENITEMACTION_H
#define PLAYERBOTS_OPENITEMACTION_H

#include "Action.h"

class Player;
class Item;
class Event;

class OpenItemAction : public Action
{
public:
    OpenItemAction(PlayerbotAI* botAI) : Action(botAI, "open item") { }

    // The main function that is executed when the action is triggered
    bool Execute(Event event) override;

private:
    // Performs the action of opening the item
    void OpenItem(Item* item, uint8 bag, uint8 slot);
};

#endif
