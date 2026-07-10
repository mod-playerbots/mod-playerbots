/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_OUTFITACTION_H
#define PLAYERBOTS_OUTFITACTION_H

#include "ChatHelper.h"
#include "EquipAction.h"

class PlayerbotAI;

class OutfitAction : public EquipAction
{
public:
    OutfitAction(PlayerbotAI* botAI) : EquipAction(botAI, "outfit") {}

    bool Execute(Event event) override;

private:
    void List();
    void Save(std::string const name, ItemIds outfit);
    void Update(std::string const name);
};

#endif
