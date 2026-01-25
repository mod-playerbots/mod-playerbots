/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_BOT_CONTEXT_H
#define _PLAYERBOT_BOT_CONTEXT_H

#include "Bot/Interface/IBotContext.h"

class PlayerbotAI;

/**
 * @brief Implementation of IBotContext
 *
 * This service provides bot context access,
 * extracting this functionality from PlayerbotAI for better testability.
 *
 * The service delegates to PlayerbotAI methods during the transition period.
 */
class BotContext : public IBotContext
{
public:
    explicit BotContext(PlayerbotAI* ai) : _botAI(ai) {}
    ~BotContext() override = default;

    // Bot and Master access
    Player* GetBot() override;
    Player* GetMaster() override;
    void SetMaster(Player* newMaster) override;

    // State management
    BotState GetState() const override;
    bool IsInCombat() const override;

    // Player validation
    bool IsRealPlayer() const override;
    bool HasRealPlayerMaster() const override;
    bool HasActivePlayerMaster() const override;
    bool IsAlt() const override;

    // Object access
    Creature* GetCreature(ObjectGuid guid) override;
    Unit* GetUnit(ObjectGuid guid) override;
    Player* GetPlayer(ObjectGuid guid) override;
    GameObject* GetGameObject(ObjectGuid guid) override;
    WorldObject* GetWorldObject(ObjectGuid guid) override;

    // Location info
    AreaTableEntry const* GetCurrentArea() const override;
    AreaTableEntry const* GetCurrentZone() const override;

    // Group access
    std::vector<Player*> GetPlayersInGroup() override;
    Player* GetGroupLeader() override;

    // Utility
    bool IsSafe(Player* player) const override;
    bool IsSafe(WorldObject* obj) const override;
    bool IsOpposing(Player* player) const override;
    bool CanMove() const override;

    // Player proximity
    bool HasPlayerNearby(float range) const override;
    bool HasPlayerNearby(WorldPosition* pos, float range) const override;
    bool HasManyPlayersNearby(uint32 triggerValue, float range) const override;

private:
    PlayerbotAI* _botAI;
};

#endif
