/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_BANKACTION_H
#define PLAYERBOTS_BANKACTION_H

#include "InventoryAction.h"

class Item;
class PlayerbotAI;

class BankAction : public InventoryAction
{
public:
    BankAction(PlayerbotAI* botAI) : InventoryAction(botAI, "bank") {}

    bool Execute(Event event) override;

private:
    bool ExecuteBank(std::string const text, Unit* bank);
    void ListItems();
    bool Withdraw(uint32 itemid);
    bool Deposit(Item* pItem);
    Item* FindItemInBank(uint32 ItemId);
};

#endif
