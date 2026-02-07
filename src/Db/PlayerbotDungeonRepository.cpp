/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "Log.h"
#include "Timer.h"
#include "DatabaseEnv.h"
#include "Field.h"
#include "World.h"
// Required import due to poor implementation by AC
#include "QueryResult.h"

#include "PlayerbotDungeonRepository.h"

std::vector<DungeonSuggestion> const PlayerbotDungeonRepository::GetDungeonSuggestions()
{
    return m_dungeonSuggestions;
}

void PlayerbotDungeonRepository::LoadDungeonSuggestions()
{
    LOG_INFO("server.loading", "Loading playerbots dungeon suggestions...");
    uint32 oldMSTime = getMSTime();

    uint32 count = 0;
    auto statement = PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_SEL_DUNGEON_SUGGESTION);
    uint8 const expansion = sWorld->getIntConfig(CONFIG_EXPANSION);
    statement->SetData(0, expansion);

    PreparedQueryResult result = PlayerbotsDatabase.Query(statement);
    if (result)
    {
        do
        {
            Field* fields = result->Fetch();
            std::string const name = fields[0].Get<std::string>();
            uint8 const difficulty = fields[1].Get<uint8>();
            uint8 const min_level = fields[2].Get<uint8>();
            uint8 const max_level = fields[3].Get<uint8>();
            std::string const abbrevation = fields[4].Get<std::string>();
            std::string const strategy = fields[5].Get<std::string>();

            DungeonSuggestion const row = {
                name, static_cast<Difficulty>(difficulty), min_level, max_level, abbrevation, strategy};

            m_dungeonSuggestions.push_back(row);
            ++count;
        } while (result->NextRow());
    }

    LOG_INFO("server.loading", "{} playerbots dungeon suggestions loaded in {} ms", count,
             GetMSTimeDiffToNow(oldMSTime));
}
