/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_BOT_ROLE_SERVICE_H
#define _PLAYERBOT_BOT_ROLE_SERVICE_H

#include "Bot/Interface/IRoleService.h"

class PlayerbotAI;

/**
 * @brief Implementation of IRoleService
 *
 * This service provides role detection and management for bots,
 * extracting this functionality from PlayerbotAI for better testability.
 *
 * The service delegates to PlayerbotAI's static methods during the
 * transition period, allowing gradual migration.
 */
class BotRoleService : public IRoleService
{
public:
    BotRoleService() = default;
    ~BotRoleService() override = default;

    // Basic role detection
    bool IsTank(Player* player, bool bySpec = false) const override;
    bool IsHeal(Player* player, bool bySpec = false) const override;
    bool IsDps(Player* player, bool bySpec = false) const override;

    // Combat style detection
    bool IsRanged(Player* player, bool bySpec = false) const override;
    bool IsMelee(Player* player, bool bySpec = false) const override;
    bool IsCaster(Player* player, bool bySpec = false) const override;
    bool IsRangedDps(Player* player, bool bySpec = false) const override;

    // Hybrid detection
    bool IsCombo(Player* player) const override;

    // Tank hierarchy
    bool IsBotMainTank(Player* player) const override;
    bool IsMainTank(Player* player) const override;
    bool IsAssistTank(Player* player) const override;
    bool IsAssistTankOfIndex(Player* player, int index, bool ignoreDeadPlayers = false) const override;

    // Group role queries
    uint32 GetGroupTankNum(Player* player) const override;
    int32 GetAssistTankIndex(Player* player) const override;
    int32 GetGroupSlotIndex(Player* player) const override;
    int32 GetRangedIndex(Player* player) const override;
    int32 GetRangedDpsIndex(Player* player) const override;
    int32 GetMeleeIndex(Player* player) const override;
    int32 GetClassIndex(Player* player, uint8 cls) const override;

    // Heal assistant
    bool IsHealAssistantOfIndex(Player* player, int index) const override;
    bool IsRangedDpsAssistantOfIndex(Player* player, int index) const override;

    // Aggro
    bool HasAggro(Unit* unit) const override;

    // Optional: Set the PlayerbotAI context for non-static operations
    // This is needed during the transition period
    void SetBotContext(PlayerbotAI* ai) { botAI_ = ai; }

private:
    PlayerbotAI* botAI_ = nullptr;
};

#endif
