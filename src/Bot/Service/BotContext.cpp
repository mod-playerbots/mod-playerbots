/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "BotContext.h"

#include "PlayerbotAI.h"

Player* BotContext::GetBot()
{
    if (botAI_)
    {
        return botAI_->GetBot();
    }
    return nullptr;
}

Player* BotContext::GetMaster()
{
    if (botAI_)
    {
        return botAI_->GetMaster();
    }
    return nullptr;
}

void BotContext::SetMaster(Player* newMaster)
{
    if (botAI_)
    {
        botAI_->SetMaster(newMaster);
    }
}

BotState BotContext::GetState() const
{
    if (botAI_)
    {
        return botAI_->GetState();
    }
    return BOT_STATE_NON_COMBAT;
}

bool BotContext::IsInCombat() const
{
    if (botAI_)
    {
        return botAI_->GetState() == BOT_STATE_COMBAT;
    }
    return false;
}

bool BotContext::IsRealPlayer() const
{
    if (botAI_)
    {
        return botAI_->IsRealPlayer();
    }
    return false;
}

bool BotContext::HasRealPlayerMaster() const
{
    if (botAI_)
    {
        return botAI_->HasRealPlayerMaster();
    }
    return false;
}

bool BotContext::HasActivePlayerMaster() const
{
    if (botAI_)
    {
        return botAI_->HasActivePlayerMaster();
    }
    return false;
}

bool BotContext::IsAlt() const
{
    if (botAI_)
    {
        return botAI_->IsAlt();
    }
    return false;
}

Creature* BotContext::GetCreature(ObjectGuid guid)
{
    if (botAI_)
    {
        return botAI_->GetCreature(guid);
    }
    return nullptr;
}

Unit* BotContext::GetUnit(ObjectGuid guid)
{
    if (botAI_)
    {
        return botAI_->GetUnit(guid);
    }
    return nullptr;
}

Player* BotContext::GetPlayer(ObjectGuid guid)
{
    if (botAI_)
    {
        return botAI_->GetPlayer(guid);
    }
    return nullptr;
}

GameObject* BotContext::GetGameObject(ObjectGuid guid)
{
    if (botAI_)
    {
        return botAI_->GetGameObject(guid);
    }
    return nullptr;
}

WorldObject* BotContext::GetWorldObject(ObjectGuid guid)
{
    if (botAI_)
    {
        return botAI_->GetWorldObject(guid);
    }
    return nullptr;
}

AreaTableEntry const* BotContext::GetCurrentArea() const
{
    if (botAI_)
    {
        return botAI_->GetCurrentArea();
    }
    return nullptr;
}

AreaTableEntry const* BotContext::GetCurrentZone() const
{
    if (botAI_)
    {
        return botAI_->GetCurrentZone();
    }
    return nullptr;
}

std::vector<Player*> BotContext::GetPlayersInGroup()
{
    if (botAI_)
    {
        return botAI_->GetPlayersInGroup();
    }
    return {};
}

Player* BotContext::GetGroupLeader()
{
    if (botAI_)
    {
        return botAI_->GetGroupLeader();
    }
    return nullptr;
}

bool BotContext::IsSafe(Player* player) const
{
    if (botAI_)
    {
        return botAI_->IsSafe(player);
    }
    return false;
}

bool BotContext::IsSafe(WorldObject* obj) const
{
    if (botAI_)
    {
        return botAI_->IsSafe(obj);
    }
    return false;
}

bool BotContext::IsOpposing(Player* player) const
{
    if (botAI_)
    {
        return botAI_->IsOpposing(player);
    }
    return false;
}

bool BotContext::CanMove() const
{
    if (botAI_)
    {
        return botAI_->CanMove();
    }
    return false;
}

bool BotContext::HasPlayerNearby(float range) const
{
    if (botAI_)
    {
        return botAI_->HasPlayerNearby(range);
    }
    return false;
}

bool BotContext::HasPlayerNearby(WorldPosition* pos, float range) const
{
    if (botAI_)
    {
        return botAI_->HasPlayerNearby(pos, range);
    }
    return false;
}

bool BotContext::HasManyPlayersNearby(uint32 triggerValue, float range) const
{
    if (botAI_)
    {
        return botAI_->HasManyPlayersNearby(triggerValue, range);
    }
    return false;
}
