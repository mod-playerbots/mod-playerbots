/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_BUFFACTION_H
#define PLAYERBOTS_BUFFACTION_H

#include "InventoryAction.h"

class PlayerbotAI;

class BuffAction : public InventoryAction
{
public:
    BuffAction(PlayerbotAI* botAI) : InventoryAction(botAI, "buff") {}

    bool Execute(Event event) override;

private:
    void TellHeader(uint32 subClass);
};

#endif
