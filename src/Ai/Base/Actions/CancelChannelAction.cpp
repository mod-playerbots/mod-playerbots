/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "CancelChannelAction.h"
#include "Player.h"
#include "PlayerbotAI.h"

bool CancelChannelAction::Execute(Event /*event*/)
{
    if (bot->GetCurrentSpell(CURRENT_CHANNELED_SPELL))
    {
        bot->InterruptSpell(CURRENT_CHANNELED_SPELL);

        return true;
    }

    return false;
}
