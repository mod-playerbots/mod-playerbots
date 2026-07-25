/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_TELLEMBLEMSACTION_H
#define PLAYERBOTS_TELLEMBLEMSACTION_H

#include "InventoryAction.h"

class PlayerbotAI;

class TellEmblemsAction : public InventoryAction
{
public:
    TellEmblemsAction(PlayerbotAI* botAI) : InventoryAction(botAI, "emblems") {}

    bool Execute(Event event) override;
};

#endif
