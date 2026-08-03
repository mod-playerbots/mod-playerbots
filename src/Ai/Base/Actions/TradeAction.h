/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_TRADEACTION_H
#define PLAYERBOTS_TRADEACTION_H

#include "InventoryAction.h"

class Item;
class PlayerbotAI;

class TradeAction : public InventoryAction
{
public:
    TradeAction(PlayerbotAI* botAI) : InventoryAction(botAI, "trade") {}

    bool Execute(Event event) override;

private:
    bool TradeItem(Item const* item, int8 slot);

    static std::map<std::string, uint32> slots;
};

#endif
