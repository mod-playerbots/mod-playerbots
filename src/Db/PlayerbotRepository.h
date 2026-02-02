/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_PLAYERBOTREPOSITORY_H
#define _PLAYERBOT_PLAYERBOTREPOSITORY_H

#include <cstdint>
#include <string>
#include <vector>

#include "PlayerbotAI.h"

class PlayerbotRepository
{
public:
    static PlayerbotRepository& instance()
    {
        static PlayerbotRepository instance;

        return instance;
    }

    void Save(PlayerbotAI* botAI);
    void Load(PlayerbotAI* botAI);
    void Reset(PlayerbotAI* botAI);

private:
    PlayerbotRepository() = default;
    ~PlayerbotRepository() = default;

    PlayerbotRepository(const PlayerbotRepository&) = delete;
    PlayerbotRepository& operator=(const PlayerbotRepository&) = delete;

    PlayerbotRepository(PlayerbotRepository&&) = delete;
    PlayerbotRepository& operator=(PlayerbotRepository&&) = delete;

    void SaveValue(uint32_t guid, std::string const key, std::string const value);
    std::string const FormatStrategies(std::string const type, std::vector<std::string> strategies);
};

#endif
