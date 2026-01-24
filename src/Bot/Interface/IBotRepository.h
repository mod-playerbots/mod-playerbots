/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_IBOT_REPOSITORY_H
#define _PLAYERBOT_IBOT_REPOSITORY_H

#include "Common.h"
#include <string>

class PlayerbotAI;
class Player;
class ObjectGuid;

/**
 * @brief Interface for bot data persistence
 *
 * This interface abstracts database operations for bots,
 * enabling isolated testing and loose coupling.
 */
class IBotRepository
{
public:
    virtual ~IBotRepository() = default;

    // Bot strategy persistence
    virtual void Save(PlayerbotAI* botAI) = 0;
    virtual void Load(PlayerbotAI* botAI) = 0;
    virtual void Reset(PlayerbotAI* botAI) = 0;

    // Bot data queries
    virtual bool HasSavedData(uint32 guid) = 0;
    virtual std::string GetSavedValue(uint32 guid, std::string const& key) = 0;
    virtual void SetSavedValue(uint32 guid, std::string const& key, std::string const& value) = 0;
    virtual void DeleteSavedData(uint32 guid) = 0;
};

/**
 * @brief Interface for spell repository operations
 */
class ISpellRepository
{
public:
    virtual ~ISpellRepository() = default;

    virtual bool HasSpellData(uint32 spellId) = 0;
    virtual void LoadSpellData() = 0;
};

/**
 * @brief Interface for dungeon repository operations
 */
class IDungeonRepository
{
public:
    virtual ~IDungeonRepository() = default;

    virtual bool HasDungeonData(uint32 mapId) = 0;
    virtual void LoadDungeonData() = 0;
};

#endif
