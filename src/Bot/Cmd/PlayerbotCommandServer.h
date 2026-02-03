/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_PLAYERBOTCOMMANDSERVER_H
#define _PLAYERBOT_PLAYERBOTCOMMANDSERVER_H

class PlayerbotCommandServer
{
public:
    static PlayerbotCommandServer& instance()
    {
        static PlayerbotCommandServer instance;

        return instance;
    }

    void Start();

private:
    PlayerbotCommandServer() = default;
    ~PlayerbotCommandServer() = default;

    PlayerbotCommandServer(const PlayerbotCommandServer&) = delete;
    PlayerbotCommandServer& operator=(const PlayerbotCommandServer&) = delete;

    PlayerbotCommandServer(PlayerbotCommandServer&&) = delete;
    PlayerbotCommandServer& operator=(PlayerbotCommandServer&&) = delete;
};

#endif
