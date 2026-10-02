/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_LOOTOBJECTSTACK_H
#define PLAYERBOTS_LOOTOBJECTSTACK_H

#include "ObjectGuid.h"
#include <unordered_map>

class AiObjectContext;
class GameObject;
class Player;
class WorldObject;

struct ItemTemplate;

// Client parity: whether the game object's faction is hostile to the player,
// including forced-reaction overrides (e.g. disguises).
bool IsGameObjectHostileTo(GameObject const* go, Player const* player);

class LootStrategy
{
public:
    LootStrategy() {}
    virtual ~LootStrategy(){};
    virtual bool CanLoot(ItemTemplate const* proto, AiObjectContext* context) = 0;
    // True for the all/* strategy: loot everything the interaction gates allow.
    virtual bool AlwaysLoot() const { return false; }
    virtual std::string const GetName() = 0;
};

class LootObject
{
public:
    LootObject() : skillId(0), reqSkillValue(0), reqItem(0) {}
    LootObject(Player* bot, ObjectGuid guid);
    LootObject(LootObject const& other);
    LootObject& operator=(LootObject const& other) = default;

    bool IsEmpty() { return !guid; }
    bool IsLootPossible(Player* bot);
    void Refresh(Player* bot, ObjectGuid guid);
    WorldObject* GetWorldObject(Player* bot);
    ObjectGuid guid;

    uint32 skillId;
    uint32 reqSkillValue;
    uint32 reqItem;

private:
    static bool IsNeededForQuest(Player* bot, uint32 itemId);
};

class LootTarget
{
public:
    LootTarget(ObjectGuid guid);
    LootTarget(LootTarget const& other);

public:
    LootTarget& operator=(LootTarget const& other);
    bool operator<(LootTarget const& other) const;

public:
    ObjectGuid guid;
    time_t asOfTime;
};

class LootTargetList : public std::set<LootTarget>
{
public:
    void shrink(time_t fromTime);
};

class LootObjectStack
{
public:
    LootObjectStack(Player* bot) : bot(bot) {}

    static constexpr time_t SkipDuration = 30;

    bool Add(ObjectGuid guid);
    void Remove(ObjectGuid guid);
    void Skip(ObjectGuid guid);
    void Unskip(ObjectGuid guid);
    void Clear();
    bool CanLoot(float maxDistance);
    LootObject GetLoot(float maxDistance = 0);
    // Cached per bot, per item (AiPlayerbot.LootItemStoreableCache*). Targeting only - the
    // autostore re-checks uncached, so a stale verdict can waste an open but never a store.
    bool IsItemStoreable(uint32 itemId);

private:
    LootObject GetNearest(float maxDistance = 0);

    Player* bot;
    LootTargetList availableLoot;
    LootTargetList skippedLoot;
    std::unordered_map<uint32, std::pair<bool, time_t>> itemStoreableCache;
};

#endif
