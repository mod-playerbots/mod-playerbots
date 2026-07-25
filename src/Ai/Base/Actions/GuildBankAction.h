/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_GUILDBANKACTION_H
#define PLAYERBOTS_GUILDBANKACTION_H

#include "InventoryAction.h"

class GameObject;
class Item;
class PlayerbotAI;

class GuildBankAction : public InventoryAction
{
public:
    GuildBankAction(PlayerbotAI* botAI) : InventoryAction(botAI, "guild bank") {}

    bool Execute(Event event) override;

private:
    bool Execute(std::string const text, GameObject* bank);
    bool MoveFromCharToBank(Item* item, GameObject* bank);
};

#endif
