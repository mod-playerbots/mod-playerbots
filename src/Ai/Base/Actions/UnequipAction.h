/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_UNEQUIPACTION_H
#define PLAYERBOTS_UNEQUIPACTION_H

#include "InventoryAction.h"

class FindItemVisitor;
class Item;
class PlayerbotAI;

class UnequipAction : public InventoryAction
{
public:
    UnequipAction(PlayerbotAI* botAI) : InventoryAction(botAI, "unequip") {}

    bool Execute(Event event) override;

private:
    void UnequipItem(Item* item);
    void UnequipItem(FindItemVisitor* visitor);
};

#endif
