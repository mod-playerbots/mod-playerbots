/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef PLAYERBOTS_NEARESTREALPLAYERINZONEVALUE_H
#define PLAYERBOTS_NEARESTREALPLAYERINZONEVALUE_H

#include "Value.h"

class PlayerbotAI;

// Faction-agnostic, sight-range-independent "nearest real (non-bot) player in the bot's assigned
// world-PvP zone" - used as a long-range movement destination for world PvP bots closing the
// 1000+ yard gap left by their initial teleport, which the usual grid/sight-limited "nearest
// player" Values can't see across.
class NearestRealPlayerInZoneValue : public ObjectGuidCalculatedValue
{
public:
    NearestRealPlayerInZoneValue(PlayerbotAI* botAI)
        : ObjectGuidCalculatedValue(botAI, "nearest real player in zone", 2)
    {
    }

    ObjectGuid Calculate() override;
};

#endif
