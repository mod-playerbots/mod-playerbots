/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_IBOT_CONTEXT_H
#define _PLAYERBOT_IBOT_CONTEXT_H

#include "Common.h"

class Player;
class Unit;
class Creature;
class GameObject;
class WorldObject;
class ObjectGuid;
class WorldPosition;
struct AreaTableEntry;

enum BotState : int;

/**
 * @brief Interface for bot context access
 *
 * This interface provides access to the bot, master, and related game objects,
 * abstracting away direct dependencies on PlayerbotAI for testability.
 */
class IBotContext
{
public:
    virtual ~IBotContext() = default;

    // Bot and Master access
    virtual Player* GetBot() = 0;
    virtual Player* GetMaster() = 0;
    virtual void SetMaster(Player* newMaster) = 0;

    // State management
    virtual BotState GetState() const = 0;
    virtual bool IsInCombat() const = 0;

    // Player validation
    virtual bool IsRealPlayer() const = 0;
    virtual bool HasRealPlayerMaster() const = 0;
    virtual bool HasActivePlayerMaster() const = 0;
    virtual bool IsAlt() const = 0;

    // Object access
    virtual Creature* GetCreature(ObjectGuid guid) = 0;
    virtual Unit* GetUnit(ObjectGuid guid) = 0;
    virtual Player* GetPlayer(ObjectGuid guid) = 0;
    virtual GameObject* GetGameObject(ObjectGuid guid) = 0;
    virtual WorldObject* GetWorldObject(ObjectGuid guid) = 0;

    // Location info
    virtual AreaTableEntry const* GetCurrentArea() const = 0;
    virtual AreaTableEntry const* GetCurrentZone() const = 0;

    // Group access
    virtual std::vector<Player*> GetPlayersInGroup() = 0;
    virtual Player* GetGroupLeader() = 0;

    // Utility
    virtual bool IsSafe(Player* player) const = 0;
    virtual bool IsSafe(WorldObject* obj) const = 0;
    virtual bool IsOpposing(Player* player) const = 0;
    virtual bool CanMove() const = 0;

    // Player proximity
    virtual bool HasPlayerNearby(float range) const = 0;
    virtual bool HasPlayerNearby(WorldPosition* pos, float range) const = 0;
    virtual bool HasManyPlayersNearby(uint32 triggerValue, float range) const = 0;
};

#endif
