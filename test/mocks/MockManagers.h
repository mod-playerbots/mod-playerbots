/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_MOCK_MANAGERS_H
#define _PLAYERBOT_MOCK_MANAGERS_H

#include <gmock/gmock.h>
#include "Bot/Interface/ITravelManager.h"
#include "Bot/Interface/IRandomBotManager.h"
#include "Bot/Interface/IBotRepository.h"

/**
 * @brief Mock implementation of ITravelManager for testing
 */
class MockTravelManager : public ITravelManager
{
public:
    MOCK_METHOD(void, RandomTeleport, (Player* bot), (override));
    MOCK_METHOD(void, RandomTeleportForLevel, (Player* bot), (override));
    MOCK_METHOD(void, RandomTeleportForRpg, (Player* bot), (override));
    MOCK_METHOD(uint32, GetZoneLevel, (uint16 mapId, float x, float y, float z), (override));
    MOCK_METHOD(std::vector<WorldLocation>, GetLocsForLevel, (uint8 level), (override));
    MOCK_METHOD(bool, HasDestination, (Player* bot), (override));
    MOCK_METHOD(void, SetQuestDestination, (Player* bot, Quest const* quest), (override));
    MOCK_METHOD(void, ClearDestination, (Player* bot), (override));
};

/**
 * @brief Mock implementation of IRandomBotManager for testing
 */
class MockRandomBotManager : public IRandomBotManager
{
public:
    // Bot queries
    MOCK_METHOD(bool, IsRandomBot, (Player* bot), (override));
    MOCK_METHOD(bool, IsRandomBot, (ObjectGuid::LowType guid), (override));
    MOCK_METHOD(Player*, GetRandomPlayer, (), (override));
    MOCK_METHOD(std::vector<Player*>, GetPlayers, (), (override));
    MOCK_METHOD(uint32, GetActiveBotCount, (), (const, override));
    MOCK_METHOD(uint32, GetMaxAllowedBotCount, (), (override));

    // Bot lifecycle
    MOCK_METHOD(void, Randomize, (Player* bot), (override));
    MOCK_METHOD(void, RandomizeFirst, (Player* bot), (override));
    MOCK_METHOD(void, Refresh, (Player* bot), (override));
    MOCK_METHOD(void, Revive, (Player* bot), (override));
    MOCK_METHOD(void, Remove, (Player* bot), (override));
    MOCK_METHOD(void, Clear, (Player* bot), (override));

    // Scheduling
    MOCK_METHOD(void, ScheduleTeleport, (uint32 bot, uint32 time), (override));
    MOCK_METHOD(void, ScheduleChangeStrategy, (uint32 bot, uint32 time), (override));

    // Events
    MOCK_METHOD(void, OnPlayerLogin, (Player* player), (override));
    MOCK_METHOD(void, OnPlayerLogout, (Player* player), (override));

    // Value storage
    MOCK_METHOD(uint32, GetValue, (Player* bot, std::string const& type), (override));
    MOCK_METHOD(uint32, GetValue, (uint32 bot, std::string const& type), (override));
    MOCK_METHOD(void, SetValue, (Player* bot, std::string const& type, uint32 value, std::string const& data),
                (override));
    MOCK_METHOD(void, SetValue, (uint32 bot, std::string const& type, uint32 value, std::string const& data),
                (override));

    // Trading
    MOCK_METHOD(double, GetBuyMultiplier, (Player* bot), (override));
    MOCK_METHOD(double, GetSellMultiplier, (Player* bot), (override));

    // Statistics
    MOCK_METHOD(void, PrintStats, (), (override));
    MOCK_METHOD(float, GetActivityPercentage, (), (override));
    MOCK_METHOD(void, SetActivityPercentage, (float percentage), (override));
};

/**
 * @brief Mock implementation of IBotRepository for testing
 */
class MockBotRepository : public IBotRepository
{
public:
    MOCK_METHOD(void, Save, (PlayerbotAI* botAI), (override));
    MOCK_METHOD(void, Load, (PlayerbotAI* botAI), (override));
    MOCK_METHOD(void, Reset, (PlayerbotAI* botAI), (override));
    MOCK_METHOD(bool, HasSavedData, (uint32 guid), (override));
    MOCK_METHOD(std::string, GetSavedValue, (uint32 guid, std::string const& key), (override));
    MOCK_METHOD(void, SetSavedValue, (uint32 guid, std::string const& key, std::string const& value), (override));
    MOCK_METHOD(void, DeleteSavedData, (uint32 guid), (override));
};

#endif
