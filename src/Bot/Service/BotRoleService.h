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
 * This service provides role detection and management for bots.
 * Role detection is based on player spec, talents, and group position.
 *
 * Static methods are provided for direct access without needing a service instance.
 * Instance methods implement the IRoleService interface for testability/mockability.
 */
class BotRoleService : public IRoleService
{
public:
    BotRoleService() = default;
    ~BotRoleService() override = default;

    // ========================================================================
    // Static methods for direct access (main implementations)
    // These can be called without a service instance
    // ========================================================================

    // Basic role detection (based on strategy or spec)
    static bool IsTankStatic(Player* player, bool bySpec = false);
    static bool IsHealStatic(Player* player, bool bySpec = false);
    static bool IsDpsStatic(Player* player, bool bySpec = false);

    // Combat style detection
    static bool IsRangedStatic(Player* player, bool bySpec = false);
    static bool IsMeleeStatic(Player* player, bool bySpec = false);
    static bool IsCasterStatic(Player* player, bool bySpec = false);
    static bool IsRangedDpsStatic(Player* player, bool bySpec = false);

    // Hybrid detection
    static bool IsComboStatic(Player* player);

    // Tank hierarchy
    static bool IsBotMainTankStatic(Player* player);
    static bool IsMainTankStatic(Player* player);
    static bool IsAssistTankStatic(Player* player);
    static bool IsAssistTankOfIndexStatic(Player* player, int index, bool ignoreDeadPlayers = false);

    // Group role queries (static - use player's group)
    static uint32 GetGroupTankNumStatic(Player* player);
    static int32 GetAssistTankIndexStatic(Player* player);

    // Heal/DPS assistant detection
    static bool IsAssistHealOfIndexStatic(Player* player, int index, bool ignoreDeadPlayers = false);
    static bool IsAssistRangedDpsOfIndexStatic(Player* player, int index, bool ignoreDeadPlayers = false);

    // ========================================================================
    // Instance methods (IRoleService interface implementation)
    // These call the static methods internally, but provide mockable interface
    // ========================================================================

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

    // Role assistant index queries
    bool IsAssistHealOfIndex(Player* player, int index, bool ignoreDeadPlayers = false) const override;
    bool IsAssistRangedDpsOfIndex(Player* player, int index, bool ignoreDeadPlayers = false) const override;

    // Aggro
    bool HasAggro(Unit* unit) const override;

    // Set the bot context for instance methods that need the bot
    void SetBotContext(PlayerbotAI* ai) { _botAI = ai; }
    PlayerbotAI* GetBotContext() const { return _botAI; }

private:
    PlayerbotAI* _botAI = nullptr;
};

#endif
