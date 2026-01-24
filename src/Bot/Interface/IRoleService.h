/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_IROLE_SERVICE_H
#define _PLAYERBOT_IROLE_SERVICE_H

#include "Common.h"

class Player;
class Unit;

/**
 * @brief Bot role types
 */
enum class BotRole : uint8
{
    NONE = 0x00,
    TANK = 0x01,
    HEALER = 0x02,
    DPS = 0x04
};

/**
 * @brief Interface for role detection and management
 *
 * This interface abstracts role-related operations from PlayerbotAI,
 * enabling isolated testing of role logic.
 *
 * Role detection is based on player spec, talents, and group position.
 */
class IRoleService
{
public:
    virtual ~IRoleService() = default;

    // Basic role detection
    virtual bool IsTank(Player* player, bool bySpec = false) const = 0;
    virtual bool IsHeal(Player* player, bool bySpec = false) const = 0;
    virtual bool IsDps(Player* player, bool bySpec = false) const = 0;

    // Combat style detection
    virtual bool IsRanged(Player* player, bool bySpec = false) const = 0;
    virtual bool IsMelee(Player* player, bool bySpec = false) const = 0;
    virtual bool IsCaster(Player* player, bool bySpec = false) const = 0;
    virtual bool IsRangedDps(Player* player, bool bySpec = false) const = 0;

    // Hybrid detection
    virtual bool IsCombo(Player* player) const = 0;

    // Tank hierarchy
    virtual bool IsBotMainTank(Player* player) const = 0;
    virtual bool IsMainTank(Player* player) const = 0;
    virtual bool IsAssistTank(Player* player) const = 0;
    virtual bool IsAssistTankOfIndex(Player* player, int index, bool ignoreDeadPlayers = false) const = 0;

    // Group role queries
    virtual uint32 GetGroupTankNum(Player* player) const = 0;
    virtual int32 GetAssistTankIndex(Player* player) const = 0;
    virtual int32 GetGroupSlotIndex(Player* player) const = 0;
    virtual int32 GetRangedIndex(Player* player) const = 0;
    virtual int32 GetRangedDpsIndex(Player* player) const = 0;
    virtual int32 GetMeleeIndex(Player* player) const = 0;
    virtual int32 GetClassIndex(Player* player, uint8 cls) const = 0;

    // Heal assistant
    virtual bool IsHealAssistantOfIndex(Player* player, int index) const = 0;
    virtual bool IsRangedDpsAssistantOfIndex(Player* player, int index) const = 0;

    // Aggro
    virtual bool HasAggro(Unit* unit) const = 0;
};

#endif
