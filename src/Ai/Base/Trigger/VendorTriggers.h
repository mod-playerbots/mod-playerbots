/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_VENDORTRIGGERS_H
#define PLAYERBOTS_VENDORTRIGGERS_H

#include "Trigger.h"

class PlayerbotAI;

class CanSellAtVendorTrigger : public Trigger
{
public:
    CanSellAtVendorTrigger(PlayerbotAI* botAI) : Trigger(botAI, "can sell at vendor", 5) {}

    bool IsActive() override;
};

#endif
