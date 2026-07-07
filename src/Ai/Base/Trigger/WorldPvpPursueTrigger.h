/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef PLAYERBOTS_WORLDPVPPURSUETRIGGER_H
#define PLAYERBOTS_WORLDPVPPURSUETRIGGER_H

#include "Trigger.h"

class PlayerbotAI;

// Fires while a world-PvP bot still needs to close the distance to the nearest real player in its
// zone: not already in combat, no opposing-faction unit nearby, and not yet within
// AiPlayerbot.SyncBotsWithPlayerReachDistance of that player.
class WorldPvpPursueTrigger : public Trigger
{
public:
    WorldPvpPursueTrigger(PlayerbotAI* botAI) : Trigger(botAI, "world pvp pursue", 1) {}

    bool IsActive() override;
};

#endif
