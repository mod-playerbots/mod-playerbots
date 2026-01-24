/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_BOT_REPOSITORY_ADAPTER_H
#define _PLAYERBOT_BOT_REPOSITORY_ADAPTER_H

#include "Bot/Interface/IBotRepository.h"

/**
 * @brief Adapter that wraps PlayerbotRepository singleton behind IBotRepository interface
 *
 * This adapter allows code to use the IBotRepository interface while
 * delegating to the existing PlayerbotRepository singleton.
 */
class BotRepositoryAdapter : public IBotRepository
{
public:
    BotRepositoryAdapter() = default;
    ~BotRepositoryAdapter() override = default;

    // Bot strategy persistence
    void Save(PlayerbotAI* botAI) override;
    void Load(PlayerbotAI* botAI) override;
    void Reset(PlayerbotAI* botAI) override;

    // Bot data queries
    bool HasSavedData(uint32 guid) override;
    std::string GetSavedValue(uint32 guid, std::string const& key) override;
    void SetSavedValue(uint32 guid, std::string const& key, std::string const& value) override;
    void DeleteSavedData(uint32 guid) override;
};

#endif
