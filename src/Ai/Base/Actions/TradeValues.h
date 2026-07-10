/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_TRADEVALUES_H
#define PLAYERBOTS_TRADEVALUES_H

#include "NamedObjectContext.h"
#include "Value.h"

class Item;
class PlayerbotAI;

class ItemsUsefulToGiveValue : public CalculatedValue<std::vector<Item*>>, public Qualified
{
public:
    ItemsUsefulToGiveValue(PlayerbotAI* botAI, std::string const name = "useful to give") : CalculatedValue(botAI, name)
    {
    }

    std::vector<Item*> Calculate();
};

#endif
