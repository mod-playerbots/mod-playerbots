/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_ITEMUSAGEVALUE_H
#define _PLAYERBOT_ITEMUSAGEVALUE_H

#include <string>
#include <vector>

#include "NamedObjectContext.h"
#include "SharedDefines.h"
#include "Value.h"

// Shared UTF-8 lowercase helper used by item/loot logic.
std::string ToLowerUtf8(std::string const& s);

class Item;
class Group;
class Player;
class PlayerbotAI;

struct ItemTemplate;
enum RollVote : uint8;

struct ParsedItemUsage
{
    uint32 itemId = 0;
    int32 randomPropertyId = 0;
};

// Shared helper: infer profession SkillLine for a recipe item.
// Uses RequiredSkill when available, otherwise falls back to SubClass/name heuristics.
uint32 GetRecipeSkill(ItemTemplate const* proto);

// Shared loot/spec helpers used by ItemUsageValue and loot-roll logic
struct SpecTraits
{
    uint8 cls = 0;
    std::string spec;
    bool isCaster = false;
    bool isHealer = false;
    bool isTank = false;
    bool isPhysical = false;
    bool isDKTank = false;
    bool isWarProt = false;
    bool isEnhSham = false;
    bool isFeralTk = false;
    bool isFeralDps = false;
    bool isHunter = false;
    bool isRogue = false;
    bool isWarrior = false;
    bool isRetPal = false;
    bool isProtPal = false;
};

struct ItemStatProfile
{
    bool hasINT = false;
    bool hasSPI = false;
    bool hasMP5 = false;
    bool hasSP = false;
    bool hasSTR = false;
    bool hasAGI = false;
    bool hasSTA = false;
    bool hasAP = false;
    bool hasARP = false;
    bool hasEXP = false;
    bool hasHIT = false;
    bool hasHASTE = false;
    bool hasCRIT = false;
    bool hasDef = false;
    bool hasAvoid = false;
    bool hasBlockValue = false;
};

SpecTraits GetSpecTraits(Player* bot);
ItemStatProfile BuildItemStatProfile(ItemTemplate const* proto);

enum ItemUsage : uint32
{
    ITEM_USAGE_NONE = 0,
    ITEM_USAGE_EQUIP = 1,
    ITEM_USAGE_REPLACE = 2,
    ITEM_USAGE_BAD_EQUIP = 3,
    ITEM_USAGE_BROKEN_EQUIP = 4,
    ITEM_USAGE_QUEST = 5,
    ITEM_USAGE_SKILL = 6,
    ITEM_USAGE_USE = 7,
    ITEM_USAGE_GUILD_TASK = 8,
    ITEM_USAGE_DISENCHANT = 9,
    ITEM_USAGE_AH = 10,
    ITEM_USAGE_KEEP = 11,
    ITEM_USAGE_VENDOR = 12,
    ITEM_USAGE_AMMO = 13
};

class ItemUsageValue : public CalculatedValue<ItemUsage>, public Qualified
{
public:
    ItemUsageValue(PlayerbotAI* botAI, std::string const name = "item usage") : CalculatedValue<ItemUsage>(botAI, name)
    {
    }

    static std::string BuildItemUsageParam(uint32 itemId, int32 randomPropertyId);
    static bool IsLockboxItem(ItemTemplate const* proto);

    ItemUsage Calculate() override;

protected:
    ItemUsage QueryItemUsageForEquip(ItemTemplate const* proto, int32 randomPropertyId = 0);
    ItemUsage QueryItemUsageForAmmo(ItemTemplate const* proto);

private:
    uint32 GetSmallestBagSize();
    bool IsItemUsefulForQuest(Player* player, ItemTemplate const* proto);
    bool IsItemNeededForSkill(ItemTemplate const* proto);
    bool IsItemUsefulForSkill(ItemTemplate const* proto);
    bool IsItemNeededForUsefullSpell(ItemTemplate const* proto, bool checkAllReagents = false);
    bool HasItemsNeededForSpell(uint32 spellId, ItemTemplate const* proto);
    Item* CurrentItem(ItemTemplate const* proto);
    float CurrentStacks(ItemTemplate const* proto);
    float BetterStacks(ItemTemplate const* proto, std::string const usageType = "");

public:
    static std::vector<uint32> SpellsUsingItem(uint32 itemId, Player* bot);
    static bool SpellGivesSkillUp(uint32 spellId, Player* bot);
    static std::string const GetConsumableType(ItemTemplate const* proto, bool hasMana);
};

// Smart-loot usage value (separate from "item upgrade" refactor)
class LootUsageValue : public ItemUsageValue
{
public:
    LootUsageValue(PlayerbotAI* botAI, std::string const name = "loot usage") : ItemUsageValue(botAI, name) {}
    ItemUsage Calculate() override;
};

// Refactor EquipActions: dedicated value used for equip-upgrade triggers
class ItemUpgradeValue : public ItemUsageValue
{
public:
    ItemUpgradeValue(PlayerbotAI* botAI, std::string const name = "item upgrade") : ItemUsageValue(botAI, name) {}
    ItemUsage Calculate() override;
};

// Loot roll helpers (used by LootRollAction)
char const* RollVoteText(RollVote v);
RollVote CalculateLootRollVote(Player* bot, ItemTemplate const* proto, int32 randomProperty, ItemUsage usage,
                               Group* group);

#endif