/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
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
