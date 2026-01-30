/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_PLAYERBOTDUNGEONREPOSITORY_H
#define _PLAYERBOT_PLAYERBOTDUNGEONREPOSITORY_H

#include <vector>
#include <string>

#include "DBCEnums.h"

struct DungeonSuggestion
{
    std::string name;
    Difficulty difficulty;
    uint8 min_level;
    uint8 max_level;
    std::string abbrevation;
    std::string strategy;
};

// @TODO: Completely unused at this moment.
class PlayerbotDungeonRepository
{
public:
    static PlayerbotDungeonRepository& instance()
    {
        static PlayerbotDungeonRepository instance;

        return instance;
    }

    void LoadDungeonSuggestions();
    std::vector<DungeonSuggestion> const GetDungeonSuggestions();

private:
    PlayerbotDungeonRepository() = default;
    ~PlayerbotDungeonRepository() = default;

    PlayerbotDungeonRepository(const PlayerbotDungeonRepository&) = delete;
    PlayerbotDungeonRepository& operator=(const PlayerbotDungeonRepository&) = delete;

    PlayerbotDungeonRepository(PlayerbotDungeonRepository&&) = delete;
    PlayerbotDungeonRepository& operator=(PlayerbotDungeonRepository&&) = delete;

    std::vector<DungeonSuggestion> m_dungeonSuggestions;
};

#endif
