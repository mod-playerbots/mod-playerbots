/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_GIVEITEMACTION_H
#define PLAYERBOTS_GIVEITEMACTION_H

#include "InventoryAction.h"

class PlayerbotAI;

class GiveItemAction : public InventoryAction
{
public:
    GiveItemAction(PlayerbotAI* botAI, std::string const name, std::string const item)
        : InventoryAction(botAI, name), item(item)
    {
    }

    bool Execute(Event event) override;
    bool isUseful() override;
    Unit* GetTarget() override;

protected:
    std::string const item;
};

class GiveFoodAction : public GiveItemAction
{
public:
    GiveFoodAction(PlayerbotAI* botAI) : GiveItemAction(botAI, "give food", "conjured food") {}

    bool isUseful() override;
    Unit* GetTarget() override;
};

class GiveWaterAction : public GiveItemAction
{
public:
    GiveWaterAction(PlayerbotAI* botAI) : GiveItemAction(botAI, "give water", "conjured water") {}

    bool isUseful() override;
    Unit* GetTarget() override;
};

#endif
