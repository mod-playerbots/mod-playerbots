/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_RANDOM_BOT_MANAGER_ADAPTER_H
#define _PLAYERBOT_RANDOM_BOT_MANAGER_ADAPTER_H

#include "Bot/Interface/IRandomBotManager.h"

/**
 * @brief Adapter that wraps RandomPlayerbotMgr singleton behind IRandomBotManager interface
 *
 * This adapter allows code to use the IRandomBotManager interface while
 * delegating to the existing RandomPlayerbotMgr singleton.
 */
class RandomBotManagerAdapter : public IRandomBotManager
{
public:
    RandomBotManagerAdapter() = default;
    ~RandomBotManagerAdapter() override = default;

    // Bot queries
    bool IsRandomBot(Player* bot) override;
    bool IsRandomBot(ObjectGuid::LowType guid) override;
    Player* GetRandomPlayer() override;
    std::vector<Player*> GetPlayers() override;
    uint32 GetActiveBotCount() const override;
    uint32 GetMaxAllowedBotCount() override;

    // Bot lifecycle
    void Randomize(Player* bot) override;
    void RandomizeFirst(Player* bot) override;
    void Refresh(Player* bot) override;
    void Revive(Player* bot) override;
    void Remove(Player* bot) override;
    void Clear(Player* bot) override;

    // Scheduling
    void ScheduleTeleport(uint32 bot, uint32 time = 0) override;
    void ScheduleChangeStrategy(uint32 bot, uint32 time = 0) override;

    // Events
    void OnPlayerLogin(Player* player) override;
    void OnPlayerLogout(Player* player) override;

    // Value storage
    uint32 GetValue(Player* bot, std::string const& type) override;
    uint32 GetValue(uint32 bot, std::string const& type) override;
    void SetValue(Player* bot, std::string const& type, uint32 value, std::string const& data = "") override;
    void SetValue(uint32 bot, std::string const& type, uint32 value, std::string const& data = "") override;

    // Trading
    double GetBuyMultiplier(Player* bot) override;
    double GetSellMultiplier(Player* bot) override;

    // Statistics
    void PrintStats() override;
    float GetActivityPercentage() override;
    void SetActivityPercentage(float percentage) override;
};

#endif
