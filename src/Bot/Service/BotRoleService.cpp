/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "BotRoleService.h"

#include "PlayerbotAI.h"

bool BotRoleService::IsTank(Player* player, bool bySpec) const
{
    // Delegate to PlayerbotAI static method during transition
    return PlayerbotAI::IsTank(player, bySpec);
}

bool BotRoleService::IsHeal(Player* player, bool bySpec) const
{
    return PlayerbotAI::IsHeal(player, bySpec);
}

bool BotRoleService::IsDps(Player* player, bool bySpec) const
{
    return PlayerbotAI::IsDps(player, bySpec);
}

bool BotRoleService::IsRanged(Player* player, bool bySpec) const
{
    return PlayerbotAI::IsRanged(player, bySpec);
}

bool BotRoleService::IsMelee(Player* player, bool bySpec) const
{
    return PlayerbotAI::IsMelee(player, bySpec);
}

bool BotRoleService::IsCaster(Player* player, bool bySpec) const
{
    return PlayerbotAI::IsCaster(player, bySpec);
}

bool BotRoleService::IsRangedDps(Player* player, bool bySpec) const
{
    return PlayerbotAI::IsRangedDps(player, bySpec);
}

bool BotRoleService::IsCombo(Player* player) const
{
    return PlayerbotAI::IsCombo(player);
}

bool BotRoleService::IsBotMainTank(Player* player) const
{
    return PlayerbotAI::IsBotMainTank(player);
}

bool BotRoleService::IsMainTank(Player* player) const
{
    return PlayerbotAI::IsMainTank(player);
}

bool BotRoleService::IsAssistTank(Player* player) const
{
    return PlayerbotAI::IsAssistTank(player);
}

bool BotRoleService::IsAssistTankOfIndex(Player* player, int index, bool ignoreDeadPlayers) const
{
    return PlayerbotAI::IsAssistTankOfIndex(player, index, ignoreDeadPlayers);
}

uint32 BotRoleService::GetGroupTankNum(Player* player) const
{
    return PlayerbotAI::GetGroupTankNum(player);
}

int32 BotRoleService::GetAssistTankIndex(Player* player) const
{
    return PlayerbotAI::GetAssistTankIndex(player);
}

int32 BotRoleService::GetGroupSlotIndex(Player* player) const
{
    // This is an instance method in PlayerbotAI, needs bot context
    if (botAI_)
    {
        return botAI_->GetGroupSlotIndex(player);
    }
    return -1;
}

int32 BotRoleService::GetRangedIndex(Player* player) const
{
    // This is an instance method in PlayerbotAI, needs bot context
    if (botAI_)
    {
        return botAI_->GetRangedIndex(player);
    }
    return -1;
}

int32 BotRoleService::GetRangedDpsIndex(Player* player) const
{
    // This is an instance method in PlayerbotAI, needs bot context
    if (botAI_)
    {
        return botAI_->GetRangedDpsIndex(player);
    }
    return -1;
}

int32 BotRoleService::GetMeleeIndex(Player* player) const
{
    // This is an instance method in PlayerbotAI, needs bot context
    if (botAI_)
    {
        return botAI_->GetMeleeIndex(player);
    }
    return -1;
}

int32 BotRoleService::GetClassIndex(Player* player, uint8 cls) const
{
    // This is an instance method in PlayerbotAI, needs bot context
    if (botAI_)
    {
        return botAI_->GetClassIndex(player, cls);
    }
    return -1;
}

bool BotRoleService::IsHealAssistantOfIndex(Player* player, int index) const
{
    return PlayerbotAI::IsHealAssistantOfIndex(player, index);
}

bool BotRoleService::IsRangedDpsAssistantOfIndex(Player* player, int index) const
{
    return PlayerbotAI::IsRangedDpsAssistantOfIndex(player, index);
}

bool BotRoleService::HasAggro(Unit* unit) const
{
    // This is an instance method in PlayerbotAI, needs bot context
    if (botAI_)
    {
        return botAI_->HasAggro(unit);
    }
    return false;
}
