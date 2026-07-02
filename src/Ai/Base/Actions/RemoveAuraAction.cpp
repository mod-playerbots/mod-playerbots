/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "RemoveAuraAction.h"

#include "Event.h"
#include "Playerbots.h"

RemoveAuraAction::RemoveAuraAction(PlayerbotAI* botAI) : Action(botAI, "ra") {}

bool RemoveAuraAction::Execute(Event event)
{
    botAI->RemoveAura(event.getParam());
    return true;
}
