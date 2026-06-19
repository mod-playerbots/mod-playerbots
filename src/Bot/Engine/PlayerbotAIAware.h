/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef PLAYERBOTS_PLAYERBOTAIAWARE_H
#define PLAYERBOTS_PLAYERBOTAIAWARE_H

class PlayerbotAI;

class PlayerbotAIAware
{
public:
    PlayerbotAIAware(PlayerbotAI* botAI) : botAI(botAI) {}

protected:
    PlayerbotAI* botAI;
};

#endif
