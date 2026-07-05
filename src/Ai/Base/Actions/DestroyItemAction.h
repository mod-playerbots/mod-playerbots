/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef PLAYERBOTS_DESTROYITEMACTION_H
#define PLAYERBOTS_DESTROYITEMACTION_H

#include "InventoryAction.h"

class FindItemVisitor;
class PlayerbotAI;

class DestroyItemAction : public InventoryAction
{
public:
    DestroyItemAction(PlayerbotAI* botAI, std::string const name = "destroy") : InventoryAction(botAI, name) {}

    bool Execute(Event event) override;

protected:
    void DestroyItem(FindItemVisitor* visitor);
};

class SmartDestroyItemAction : public DestroyItemAction
{
public:
    SmartDestroyItemAction(PlayerbotAI* botAI) : DestroyItemAction(botAI, "smart destroy") {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

#endif
