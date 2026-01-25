/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "BotContext.h"

#include "PlayerbotAI.h"

Player* BotContext::GetBot()
{
    if (_botAI)
    {
        return _botAI->GetBot();
    }
    return nullptr;
}

Player* BotContext::GetMaster()
{
    if (_botAI)
    {
        return _botAI->GetMaster();
    }
    return nullptr;
}

void BotContext::SetMaster(Player* newMaster)
{
    if (_botAI)
    {
        _botAI->SetMaster(newMaster);
    }
}

BotState BotContext::GetState() const
{
    if (_botAI)
    {
        return _botAI->GetState();
    }
    return BOT_STATE_NON_COMBAT;
}

bool BotContext::IsInCombat() const
{
    if (_botAI)
    {
        return _botAI->GetState() == BOT_STATE_COMBAT;
    }
    return false;
}

bool BotContext::IsRealPlayer() const
{
    if (_botAI)
    {
        return _botAI->IsRealPlayer();
    }
    return false;
}

bool BotContext::HasRealPlayerMaster() const
{
    if (_botAI)
    {
        return _botAI->HasRealPlayerMaster();
    }
    return false;
}

bool BotContext::HasActivePlayerMaster() const
{
    if (_botAI)
    {
        return _botAI->HasActivePlayerMaster();
    }
    return false;
}

bool BotContext::IsAlt() const
{
    if (_botAI)
    {
        return _botAI->IsAlt();
    }
    return false;
}

Creature* BotContext::GetCreature(ObjectGuid guid)
{
    if (_botAI)
    {
        return _botAI->GetCreature(guid);
    }
    return nullptr;
}

Unit* BotContext::GetUnit(ObjectGuid guid)
{
    if (_botAI)
    {
        return _botAI->GetUnit(guid);
    }
    return nullptr;
}

Player* BotContext::GetPlayer(ObjectGuid guid)
{
    if (_botAI)
    {
        return _botAI->GetPlayer(guid);
    }
    return nullptr;
}

GameObject* BotContext::GetGameObject(ObjectGuid guid)
{
    if (_botAI)
    {
        return _botAI->GetGameObject(guid);
    }
    return nullptr;
}

WorldObject* BotContext::GetWorldObject(ObjectGuid guid)
{
    if (_botAI)
    {
        return _botAI->GetWorldObject(guid);
    }
    return nullptr;
}

AreaTableEntry const* BotContext::GetCurrentArea() const
{
    if (_botAI)
    {
        return _botAI->GetCurrentArea();
    }
    return nullptr;
}

AreaTableEntry const* BotContext::GetCurrentZone() const
{
    if (_botAI)
    {
        return _botAI->GetCurrentZone();
    }
    return nullptr;
}

std::vector<Player*> BotContext::GetPlayersInGroup()
{
    if (_botAI)
    {
        return _botAI->GetPlayersInGroup();
    }
    return {};
}

Player* BotContext::GetGroupLeader()
{
    if (_botAI)
    {
        return _botAI->GetGroupLeader();
    }
    return nullptr;
}

bool BotContext::IsSafe(Player* player) const
{
    if (_botAI)
    {
        return _botAI->IsSafe(player);
    }
    return false;
}

bool BotContext::IsSafe(WorldObject* obj) const
{
    if (_botAI)
    {
        return _botAI->IsSafe(obj);
    }
    return false;
}

bool BotContext::IsOpposing(Player* player) const
{
    if (_botAI)
    {
        return _botAI->IsOpposing(player);
    }
    return false;
}

bool BotContext::CanMove() const
{
    if (_botAI)
    {
        return _botAI->CanMove();
    }
    return false;
}

bool BotContext::HasPlayerNearby(float range) const
{
    if (_botAI)
    {
        return _botAI->HasPlayerNearby(range);
    }
    return false;
}

bool BotContext::HasPlayerNearby(WorldPosition* pos, float range) const
{
    if (_botAI)
    {
        return _botAI->HasPlayerNearby(pos, range);
    }
    return false;
}

bool BotContext::HasManyPlayersNearby(uint32 triggerValue, float range) const
{
    if (_botAI)
    {
        return _botAI->HasManyPlayersNearby(triggerValue, range);
    }
    return false;
}
