/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ITEMCOUNTVALUE_H
#define PLAYERBOTS_ITEMCOUNTVALUE_H

#include "InventoryAction.h"
#include "Item.h"
#include "NamedObjectContext.h"

class PlayerbotAI;

class InventoryItemValueBase : public InventoryAction
{
public:
    InventoryItemValueBase(PlayerbotAI* botAI) : InventoryAction(botAI, "empty") {}

    bool Execute(Event /*event*/) override { return false; }

protected:
    std::vector<Item*> Find(std::string const qualifier);
};

class ItemCountValue : public Uint32CalculatedValue, public Qualified, InventoryItemValueBase
{
public:
    ItemCountValue(PlayerbotAI* botAI, std::string const name = "inventory items")
        : Uint32CalculatedValue(botAI, name), InventoryItemValueBase(botAI)
    {
    }

    uint32 Calculate() override;
};

class InventoryItemValue : public CalculatedValue<std::vector<Item*>>, public Qualified, InventoryItemValueBase
{
public:
    InventoryItemValue(PlayerbotAI* botAI) : CalculatedValue<std::vector<Item*>>(botAI), InventoryItemValueBase(botAI)
    {
    }

    std::vector<Item*> Calculate() override;
};

#endif
