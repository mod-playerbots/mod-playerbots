/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "NewPlayerNearbyValue.h"

#include "Playerbots.h"

ObjectGuid NewPlayerNearbyValue::Calculate()
{
    GuidVector players = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest friendly players")->Get();
    GuidSet& alreadySeenPlayers = botAI->GetAiObjectContext()->GetValue<GuidSet&>("already seen players")->Get();
    for (ObjectGuid const guid : players)
    {
        if (alreadySeenPlayers.find(guid) == alreadySeenPlayers.end())
            return guid;
    }

    return ObjectGuid::Empty;
}
