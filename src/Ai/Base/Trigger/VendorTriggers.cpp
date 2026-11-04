/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "VendorTriggers.h"

#include "Playerbots.h"

bool CanSellAtVendorTrigger::IsActive()
{
    if (bot->IsInCombat() || bot->IsMounted() || bot->GetTradeData())
        return false;

    if (!AI_VALUE(bool, "can sell"))
        return false;

    for (ObjectGuid const& guid : AI_VALUE(GuidVector, "nearest npcs"))
        if (bot->GetNPCIfCanInteractWith(guid, UNIT_NPC_FLAG_VENDOR))
            return true;

    return false;
}
