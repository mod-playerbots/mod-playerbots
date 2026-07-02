/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ITEMFORSPELLVALUE_H
#define PLAYERBOTS_ITEMFORSPELLVALUE_H

#include "NamedObjectContext.h"
#include "Value.h"

class Item;
class PlayerbotAI;
class SpellInfo;

class ItemForSpellValue : public CalculatedValue<Item*>, public Qualified
{
public:
    ItemForSpellValue(PlayerbotAI* botAI, std::string const name = "item for spell")
        : CalculatedValue<Item*>(botAI, name, 1)
    {
    }

    Item* Calculate() override;

private:
    Item* GetItemFitsToSpellRequirements(uint8 slot, SpellInfo const* spellInfo);
};

#endif
