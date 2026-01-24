/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "RandomBotManagerAdapter.h"

#include "RandomPlayerbotMgr.h"

bool RandomBotManagerAdapter::IsRandomBot(Player* bot)
{
    return sRandomPlayerbotMgr->IsRandomBot(bot);
}

bool RandomBotManagerAdapter::IsRandomBot(ObjectGuid::LowType guid)
{
    return sRandomPlayerbotMgr->IsRandomBot(guid);
}

Player* RandomBotManagerAdapter::GetRandomPlayer()
{
    return sRandomPlayerbotMgr->GetRandomPlayer();
}

std::vector<Player*> RandomBotManagerAdapter::GetPlayers()
{
    return sRandomPlayerbotMgr->GetPlayers();
}

uint32 RandomBotManagerAdapter::GetActiveBotCount() const
{
    return sRandomPlayerbotMgr->activeBots;
}

uint32 RandomBotManagerAdapter::GetMaxAllowedBotCount()
{
    return sRandomPlayerbotMgr->GetMaxAllowedBotCount();
}

void RandomBotManagerAdapter::Randomize(Player* bot)
{
    sRandomPlayerbotMgr->Randomize(bot);
}

void RandomBotManagerAdapter::RandomizeFirst(Player* bot)
{
    sRandomPlayerbotMgr->RandomizeFirst(bot);
}

void RandomBotManagerAdapter::Refresh(Player* bot)
{
    sRandomPlayerbotMgr->Refresh(bot);
}

void RandomBotManagerAdapter::Revive(Player* bot)
{
    sRandomPlayerbotMgr->Revive(bot);
}

void RandomBotManagerAdapter::Remove(Player* bot)
{
    sRandomPlayerbotMgr->Remove(bot);
}

void RandomBotManagerAdapter::Clear(Player* bot)
{
    sRandomPlayerbotMgr->Clear(bot);
}

void RandomBotManagerAdapter::ScheduleTeleport(uint32 bot, uint32 time)
{
    sRandomPlayerbotMgr->ScheduleTeleport(bot, time);
}

void RandomBotManagerAdapter::ScheduleChangeStrategy(uint32 bot, uint32 time)
{
    sRandomPlayerbotMgr->ScheduleChangeStrategy(bot, time);
}

void RandomBotManagerAdapter::OnPlayerLogin(Player* player)
{
    sRandomPlayerbotMgr->OnPlayerLogin(player);
}

void RandomBotManagerAdapter::OnPlayerLogout(Player* player)
{
    sRandomPlayerbotMgr->OnPlayerLogout(player);
}

uint32 RandomBotManagerAdapter::GetValue(Player* bot, std::string const& type)
{
    return sRandomPlayerbotMgr->GetValue(bot, type);
}

uint32 RandomBotManagerAdapter::GetValue(uint32 bot, std::string const& type)
{
    return sRandomPlayerbotMgr->GetValue(bot, type);
}

void RandomBotManagerAdapter::SetValue(Player* bot, std::string const& type, uint32 value, std::string const& data)
{
    sRandomPlayerbotMgr->SetValue(bot, type, value, data);
}

void RandomBotManagerAdapter::SetValue(uint32 bot, std::string const& type, uint32 value, std::string const& data)
{
    sRandomPlayerbotMgr->SetValue(bot, type, value, data);
}

double RandomBotManagerAdapter::GetBuyMultiplier(Player* bot)
{
    return sRandomPlayerbotMgr->GetBuyMultiplier(bot);
}

double RandomBotManagerAdapter::GetSellMultiplier(Player* bot)
{
    return sRandomPlayerbotMgr->GetSellMultiplier(bot);
}

void RandomBotManagerAdapter::PrintStats()
{
    sRandomPlayerbotMgr->PrintStats();
}

float RandomBotManagerAdapter::GetActivityPercentage()
{
    return sRandomPlayerbotMgr->getActivityPercentage();
}

void RandomBotManagerAdapter::SetActivityPercentage(float percentage)
{
    sRandomPlayerbotMgr->setActivityPercentage(percentage);
}
