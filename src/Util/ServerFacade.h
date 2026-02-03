/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_SERVERFACADE_H
#define _PLAYERBOT_SERVERFACADE_H

class Player;
class Unit;
class WorldObject;
class WorldPacket;

/**
 * @brief Provides a simplified interface to server engine operations.
 *
 * ServerFacade acts as a wrapper around common server functions used by
 * the Playerbot system. It centralizes utility methods for distance
 * calculations, facing, chase target retrieval, and packet sending.
 */
class ServerFacade
{
public:
    ServerFacade() {}
    virtual ~ServerFacade() {}

    /**
     * @brief Get singleton instance.
     *
     * @return ServerFacade& Reference to the singleton instance.
     */
    static ServerFacade& instance()
    {
        static ServerFacade instance;

        return instance;
    }

public:
    /**
     * @brief Get 2D distance between a unit and a world object.
     *
     * The result is rounded to one decimal place.
     *
     * @param unit Source unit.
     * @param wo   Target world object.
     * @return float Distance in yards.
     */
    float GetDistance2d(Unit* unit, WorldObject* wo);

    /**
     * @brief Get 2D distance between a unit and coordinates.
     *
     * The result is rounded to one decimal place.
     *
     * @param unit Source unit.
     * @param x    Target X coordinate.
     * @param y    Target Y coordinate.
     * @return float Distance in yards.
     */
    float GetDistance2d(Unit* unit, float x, float y);

    /**
     * @brief Compare two distances.
     *
     * @param dist1 First distance.
     * @param dist2 Second distance.
     * @return true if dist1 < dist2.
     */
    bool IsDistanceLessThan(float dist1, float dist2);

    /**
     * @brief Compare two distances.
     *
     * @param dist1 First distance.
     * @param dist2 Second distance.
     * @return true if dist1 > dist2.
     */
    bool IsDistanceGreaterThan(float dist1, float dist2);

    /**
     * @brief Compare two distances.
     *
     * @param dist1 First distance.
     * @param dist2 Second distance.
     * @return true if dist1 >= dist2.
     */
    bool IsDistanceGreaterOrEqualThan(float dist1, float dist2);

    /**
     * @brief Compare two distances.
     *
     * @param dist1 First distance.
     * @param dist2 Second distance.
     * @return true if dist1 <= dist2.
     */
    bool IsDistanceLessOrEqualThan(float dist1, float dist2);

    /**
     * @brief Set bot facing towards a world object.
     *
     * @param bot   Player bot to rotate.
     * @param wo    Target world object.
     * @param force If true, force facing even while moving.
     */
    void SetFacingTo(Player* bot, WorldObject* wo, bool force = false);

    /**
     * @brief Get the current chase target of a unit.
     *
     * @param target Unit that is chasing.
     * @return Unit* The chase target, or nullptr if not chasing.
     */
    Unit* GetChaseTarget(Unit* target);

    /**
     * @brief Send a raw packet to a player.
     *
     * @param player Player to receive the packet.
     * @param packet Packet to send.
     */
    void SendPacket(Player* player, WorldPacket* packet);
};

#endif
