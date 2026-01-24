/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_IRANDOM_BOT_MANAGER_H
#define _PLAYERBOT_IRANDOM_BOT_MANAGER_H

#include "Common.h"
#include "ObjectGuid.h"
#include <vector>
#include <string>

class Player;

/**
 * @brief Interface for random bot lifecycle management
 *
 * This interface abstracts random bot operations,
 * enabling isolated testing and loose coupling.
 */
class IRandomBotManager
{
public:
    virtual ~IRandomBotManager() = default;

    // Bot queries
    virtual bool IsRandomBot(Player* bot) = 0;
    virtual bool IsRandomBot(ObjectGuid::LowType guid) = 0;
    virtual Player* GetRandomPlayer() = 0;
    virtual std::vector<Player*> GetPlayers() = 0;
    virtual uint32 GetActiveBotCount() const = 0;
    virtual uint32 GetMaxAllowedBotCount() = 0;

    // Bot lifecycle
    virtual void Randomize(Player* bot) = 0;
    virtual void RandomizeFirst(Player* bot) = 0;
    virtual void Refresh(Player* bot) = 0;
    virtual void Revive(Player* bot) = 0;
    virtual void Remove(Player* bot) = 0;
    virtual void Clear(Player* bot) = 0;

    // Scheduling
    virtual void ScheduleTeleport(uint32 bot, uint32 time = 0) = 0;
    virtual void ScheduleChangeStrategy(uint32 bot, uint32 time = 0) = 0;

    // Events
    virtual void OnPlayerLogin(Player* player) = 0;
    virtual void OnPlayerLogout(Player* player) = 0;

    // Value storage (bot-specific persistent data)
    virtual uint32 GetValue(Player* bot, std::string const& type) = 0;
    virtual uint32 GetValue(uint32 bot, std::string const& type) = 0;
    virtual void SetValue(Player* bot, std::string const& type, uint32 value, std::string const& data = "") = 0;
    virtual void SetValue(uint32 bot, std::string const& type, uint32 value, std::string const& data = "") = 0;

    // Trading
    virtual double GetBuyMultiplier(Player* bot) = 0;
    virtual double GetSellMultiplier(Player* bot) = 0;

    // Statistics
    virtual void PrintStats() = 0;
    virtual float GetActivityPercentage() = 0;
    virtual void SetActivityPercentage(float percentage) = 0;
};

#endif
