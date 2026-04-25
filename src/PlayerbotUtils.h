/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#pragma once

#include "Creature.h"
#include "ObjectGuid.h"
#include "Player.h"

namespace ai::npc
{
    inline Creature* FindNpcByFlag(Player* bot, uint32 npcFlag, GuidVector const& candidates, ObjectGuid entryHint = ObjectGuid::Empty,
                                   float entryRadius = 100.0f)
    {
        if (!bot)
            return nullptr;

        if (uint32 entry = entryHint.GetEntry())
        {
            if (Creature* creature = bot->FindNearestCreature(entry, entryRadius))
                return creature;
        }

        for (ObjectGuid const& guid : candidates)
        {
            if (Creature* creature = bot->GetNPCIfCanInteractWith(guid, npcFlag))
                return creature;
        }

        return nullptr;
    }
}
