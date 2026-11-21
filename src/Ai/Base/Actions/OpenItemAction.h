/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_OPENITEMACTION_H
#define PLAYERBOTS_OPENITEMACTION_H

#include "UseItemAction.h"

class Player;
class Item;
class Event;

// Opens anything a player would "open": loot containers through the open-item opcode, and
// open junk (clams and similar items whose on-use spell creates the loot) by using the item.
class OpenItemAction : public UseItemAction
{
public:
    OpenItemAction(PlayerbotAI* botAI) : UseItemAction(botAI, "open items", true) { }

    // The main function that is executed when the action is triggered
    bool Execute(Event event) override;
    bool isPossible() override;

private:
    // Performs the action of opening the item
    void OpenItem(Item* item, uint8 bag, uint8 slot);
};

#endif
