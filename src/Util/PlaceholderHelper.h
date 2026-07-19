/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_PLACEHOLDERHELPER_H
#define PLAYERBOTS_PLACEHOLDERHELPER_H

#include <map>

#include "Common.h"
#include "Player.h"
#include "PlayerbotDungeonRepository.h"

typedef std::map<std::string, std::string> PlaceholderMap;

class PlaceholderHelper
{
public:
    static void MapRole(PlaceholderMap& placeholders, Player* bot);
    static void MapDungeon(PlaceholderMap& placeholders, DungeonSuggestion const* dungeonSuggestion, Player* bot);

private:
    struct Insertion
    {
        std::ostringstream& out;
        DungeonSuggestion const* dungeonSuggestion;
    };

    static void InsertDungeonName(Insertion& insertion);
    static void InsertDungeonStrategy(Insertion& insertion);
    static void InsertDifficulty(Insertion& insertion, Player* bot);
};

#endif
