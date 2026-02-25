/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "ItemUsageValue.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <initializer_list>
#include <string>

#include "AiFactory.h"
#include "ChatHelper.h"
#include "Group.h"
#include "GuildTaskMgr.h"
#include "Item.h"
#include "ItemTemplate.h"
#include "LootObjectStack.h"
#include "LootRollAction.h"
#include "SpellAuraDefines.h"
#include "PlayerbotAIConfig.h"
#include "PlayerbotFactory.h"
#include "Playerbots.h"
#include "RandomItemMgr.h"
#include "ServerFacade.h"
#include "SharedDefines.h"
#include "LootAction.h"
#include "StatsWeightCalculator.h"
#include "Util.h"
#include "World.h"

namespace
{
ParsedItemUsage ParseItemUsageQualifier(std::string const& qualifier)
{
    ParsedItemUsage parsed;
    size_t const pos = qualifier.find(',');

    if (pos != std::string::npos)
    {
        parsed.itemId = atoi(qualifier.substr(0, pos).c_str());
        parsed.randomPropertyId = atoi(qualifier.substr(pos + 1).c_str());
        return parsed;
    }

    parsed.itemId = atoi(qualifier.c_str());

    return parsed;
}
} // namespace

// Lowercase helper for item names using core UTF-8 utilities.
std::string ToLowerUtf8(std::string const& s)
{
    if (s.empty())
        return s;

    std::wstring w;
    if (!Utf8toWStr(s, w))
        return s;

    wstrToLower(w);

    std::string lowered;
    if (!WStrToUtf8(w, lowered))
        return s;

    return lowered;
}

uint32 GetRecipeSkill(ItemTemplate const* proto)
{
    if (!proto)
        return 0;

    // Primary path: DB usually sets RequiredSkill on recipe items.
    if (proto->RequiredSkill)
        return proto->RequiredSkill;

    // Fallback heuristic on SubClass (books used by professions).
    switch (proto->SubClass)
    {
        case ITEM_SUBCLASS_BOOK: // e.g. Book of Glyph Mastery
        {
            // If the name hints glyphs, assume Inscription.
            std::string const lowered = ToLowerUtf8(proto->Name1);
            if (lowered.find("glyph") != std::string::npos)
                return SKILL_INSCRIPTION;
            break;
        }
        case ITEM_SUBCLASS_LEATHERWORKING_PATTERN:
            return SKILL_LEATHERWORKING;
        case ITEM_SUBCLASS_TAILORING_PATTERN:
            return SKILL_TAILORING;
        case ITEM_SUBCLASS_ENGINEERING_SCHEMATIC:
            return SKILL_ENGINEERING;
        case ITEM_SUBCLASS_BLACKSMITHING:
            return SKILL_BLACKSMITHING;
        case ITEM_SUBCLASS_COOKING_RECIPE:
            return SKILL_COOKING;
        case ITEM_SUBCLASS_ALCHEMY_RECIPE:
            return SKILL_ALCHEMY;
        case ITEM_SUBCLASS_FIRST_AID_MANUAL:
            return SKILL_FIRST_AID;
        case ITEM_SUBCLASS_ENCHANTING_FORMULA:
            return SKILL_ENCHANTING;
        case ITEM_SUBCLASS_FISHING_MANUAL:
            return SKILL_FISHING;
        case ITEM_SUBCLASS_JEWELCRAFTING_RECIPE:
            return SKILL_JEWELCRAFTING;
        default:
            break;
    }

    return 0;
}

ItemUsage ItemUsageValue::Calculate()
{
    ParsedItemUsage parsed = ParseItemUsageQualifier(qualifier);
    uint32 itemId = parsed.itemId;
    uint32 randomPropertyId = parsed.randomPropertyId;

    if (!itemId)
        return ITEM_USAGE_NONE;

    ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemId);
    if (!proto)
        return ITEM_USAGE_NONE;

    if (botAI->HasActivePlayerMaster())
    {
        if (IsItemUsefulForSkill(proto) || IsItemNeededForSkill(proto))
            return ITEM_USAGE_SKILL;
    }
    else
    {
        bool needItem = false;

        if (IsItemNeededForSkill(proto))
            needItem = true;
        else
        {
            bool lowBagSpace = AI_VALUE(uint8, "bag space") > 50;

            if (proto->Class == ITEM_CLASS_TRADE_GOODS || proto->Class == ITEM_CLASS_MISC ||
                proto->Class == ITEM_CLASS_REAGENT)
                needItem = IsItemNeededForUsefullSpell(proto, lowBagSpace);
            else if (proto->Class == ITEM_CLASS_RECIPE)
            {
                if (bot->HasSpell(proto->Spells[2].SpellId))
                    needItem = false;
                else
                    needItem = bot->BotCanUseItem(proto) == EQUIP_ERR_OK;
            }
        }

        if (needItem)
        {
            float stacks = CurrentStacks(proto);
            if (stacks < 1)
                return ITEM_USAGE_SKILL;  // Buy more.
            if (stacks < 2)
                return ITEM_USAGE_KEEP;  // Keep current amount.
        }
    }

    if (proto->Class == ITEM_CLASS_KEY)
        return ITEM_USAGE_USE;

    if (proto->Class == ITEM_CLASS_CONSUMABLE &&
        (proto->MaxCount == 0 || bot->GetItemCount(itemId, false) < proto->MaxCount))
    {
        std::string const foodType = GetConsumableType(proto, bot->GetPower(POWER_MANA));

        if (!foodType.empty() && bot->CanUseItem(proto) == EQUIP_ERR_OK)
        {
            float stacks = BetterStacks(proto, foodType);
            if (stacks < 2)
            {
                stacks += CurrentStacks(proto);

                if (stacks < 2)
                    return ITEM_USAGE_USE;  // Buy some to get to 2 stacks
                else if (stacks < 3)        // Keep the item if less than 3 stacks
                    return ITEM_USAGE_KEEP;
            }
        }
    }

    if (bot->GetGuildId() && GuildTaskMgr::instance().IsGuildTaskItem(itemId, bot->GetGuildId()))
        return ITEM_USAGE_GUILD_TASK;

    // First, check if the item is interesting as equipment (upgrade, bad-equip, etc.)
    ItemUsage equip = QueryItemUsageForEquip(proto, randomPropertyId);
    if (equip != ITEM_USAGE_NONE)
        return equip;

    // Get item instance to check if it's soulbound (used for disenchant heuristics)
    Item* item = bot->GetItemByEntry(proto->ItemId);
    bool isSoulbound = item && item->IsSoulBound();

    // Enchanting fallback: only consider disenchant when the item has no direct equipment usage.
    if ((proto->Class == ITEM_CLASS_ARMOR || proto->Class == ITEM_CLASS_WEAPON) &&
        botAI->HasSkill(SKILL_ENCHANTING) &&
        proto->Quality >= ITEM_QUALITY_UNCOMMON)
    {
        uint32 enchantingSkill = bot->GetSkillValue(SKILL_ENCHANTING);

        // Only allow disenchant if the bot has the required skill.
        bool canDisenchant = (proto->RequiredDisenchantSkill == 0 ||
                              enchantingSkill >= proto->RequiredDisenchantSkill);

        // BoP, or BoE that is already soulbound: safe to treat as DE-only usage.
        if (canDisenchant &&
            (proto->Bonding == BIND_WHEN_PICKED_UP ||
             (proto->Bonding == BIND_WHEN_EQUIPPED && isSoulbound)))
        {
            return ITEM_USAGE_DISENCHANT;
        }
    }

    Player* master = botAI->GetMaster();
    bool isSelfBot = (master == bot);
    bool botNeedsItemForQuest = IsItemUsefulForQuest(bot, proto);
    bool masterNeedsItemForQuest = master && sPlayerbotAIConfig.syncQuestWithPlayer && IsItemUsefulForQuest(master, proto);

    // Identify the source of loot
    LootObject lootObject = AI_VALUE(LootObject, "loot target");

    // Get GUID of loot source
    ObjectGuid lootGuid = lootObject.guid;

    // Check if loot source is an item
    bool isLootFromItem = lootGuid.IsItem();

    // If the loot is from an item in the bot’s bags, ignore syncQuestWithPlayer
    if (isLootFromItem && botNeedsItemForQuest)
        return ITEM_USAGE_QUEST;

    // If the bot is NOT acting alone and the master needs this quest item, defer to the master
    if (!isSelfBot && masterNeedsItemForQuest)
        return ITEM_USAGE_NONE;

    // If the bot itself needs the item for a quest, allow looting
    if (botNeedsItemForQuest)
        return ITEM_USAGE_QUEST;

    if (proto->Class == ITEM_CLASS_PROJECTILE && bot->CanUseItem(proto) == EQUIP_ERR_OK)
    {
        ItemUsage ammoUsage = QueryItemUsageForAmmo(proto);
        if (ammoUsage != ITEM_USAGE_NONE)
            return ammoUsage;
    }
    // Need to add something like free bagspace or item value.
    if (proto->SellPrice > 0)
    {
        if (proto->Quality >= ITEM_QUALITY_NORMAL && !isSoulbound)
            return ITEM_USAGE_AH;

        else
            return ITEM_USAGE_VENDOR;
    }

    return ITEM_USAGE_NONE;
}

std::string ItemUsageValue::BuildItemUsageParam(uint32 itemId, int32 randomPropertyId)
{
    if (randomPropertyId != 0)
        return std::to_string(itemId) + "," + std::to_string(randomPropertyId);

    return std::to_string(itemId);
}

namespace
{
static bool EnableGroupUsageChecks()
{
    return sPlayerbotAIConfig.rollUseGroupUsageChecks;
}

static bool IsPrimaryForSpec(Player* bot, ItemTemplate const* proto);

static bool HasAnyStat(ItemTemplate const* proto, std::initializer_list<ItemModType> mods)
{
    if (!proto)
        return false;

    for (auto const mod : mods)
    {
        for (uint8 i = 0; i < MAX_ITEM_PROTO_STATS; ++i)
        {
            if (proto->ItemStat[i].ItemStatType == mod && proto->ItemStat[i].ItemStatValue != 0)
                return true;
        }
    }

    return false;
}

static bool HasAnyTankAvoidance(ItemTemplate const* proto)
{
    return HasAnyStat(proto, {ITEM_MOD_DEFENSE_SKILL_RATING, ITEM_MOD_DODGE_RATING, ITEM_MOD_PARRY_RATING,
                              ITEM_MOD_BLOCK_RATING});
}

static bool IsRelicForClass(ItemTemplate const* proto, uint8 cls)
{
    if (!proto || proto->InventoryType != INVTYPE_RELIC)
        return false;

    switch (proto->SubClass)
    {
        case ITEM_SUBCLASS_ARMOR_IDOL:
            return cls == CLASS_DRUID;
        case ITEM_SUBCLASS_ARMOR_TOTEM:
            return cls == CLASS_SHAMAN;
        case ITEM_SUBCLASS_ARMOR_LIBRAM:
            return cls == CLASS_PALADIN;
        case ITEM_SUBCLASS_ARMOR_SIGIL:
            return cls == CLASS_DEATH_KNIGHT;
        default:
            return false;
    }
}

static bool IsBodyArmorInvType(uint8 invType)
{
    switch (invType)
    {
        case INVTYPE_HEAD:
        case INVTYPE_SHOULDERS:
        case INVTYPE_CHEST:
        case INVTYPE_ROBE:
        case INVTYPE_WAIST:
        case INVTYPE_LEGS:
        case INVTYPE_FEET:
        case INVTYPE_WRISTS:
        case INVTYPE_HANDS:
            return true;
        default:
            return false;
    }
}

static bool IsJewelryOrCloak(ItemTemplate const* proto)
{
    if (!proto)
        return false;

    switch (proto->InventoryType)
    {
        case INVTYPE_TRINKET:
        case INVTYPE_FINGER:
        case INVTYPE_NECK:
        case INVTYPE_CLOAK:
            return true;
        default:
            return false;
    }
}

static uint8 PreferredArmorSubclassFor(Player* bot)
{
    if (!bot)
        return ITEM_SUBCLASS_ARMOR_CLOTH;

    uint8 cls = bot->getClass();
    uint32 lvl = bot->GetLevel();

    if (cls == CLASS_MAGE || cls == CLASS_PRIEST || cls == CLASS_WARLOCK)
        return ITEM_SUBCLASS_ARMOR_CLOTH;

    if (cls == CLASS_DRUID || cls == CLASS_ROGUE)
        return ITEM_SUBCLASS_ARMOR_LEATHER;

    if (cls == CLASS_HUNTER || cls == CLASS_SHAMAN)
        return (lvl >= 40u) ? ITEM_SUBCLASS_ARMOR_MAIL : ITEM_SUBCLASS_ARMOR_LEATHER;

    if (cls == CLASS_WARRIOR || cls == CLASS_PALADIN)
        return (lvl >= 40u) ? ITEM_SUBCLASS_ARMOR_PLATE : ITEM_SUBCLASS_ARMOR_MAIL;

    if (cls == CLASS_DEATH_KNIGHT)
        return ITEM_SUBCLASS_ARMOR_PLATE;

    return ITEM_SUBCLASS_ARMOR_CLOTH;
}

static bool IsLowerTierArmorForBot(Player* bot, ItemTemplate const* proto)
{
    if (!bot || !proto)
        return false;
    if (proto->Class != ITEM_CLASS_ARMOR)
        return false;
    if (!IsBodyArmorInvType(proto->InventoryType))
        return false;
    if (proto->SubClass == ITEM_SUBCLASS_ARMOR_SHIELD || proto->InventoryType == INVTYPE_RELIC ||
        proto->InventoryType == INVTYPE_HOLDABLE)
        return false;

    uint8 preferred = PreferredArmorSubclassFor(bot);
    return proto->SubClass < preferred;
}

static bool IsStrictCrossArmorContext(Player* bot)
{
    if (!bot)
        return true;

    // Keep cross-armor strictly disabled at level cap and in raids to avoid
    // plate/mail healers rolling NEED on cloth/leather endgame loot.
    if (bot->GetLevel() >= static_cast<uint32>(sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL)))
        return true;

    if (Group* group = bot->GetGroup())
        if (group->isRaidGroup())
            return true;

    return false;
}

namespace
{
    static bool GroupMemberWouldEquipArmor(Player* member, PlayerbotAI* memberAI, ItemTemplate const* proto, std::string const& param)
    {
        if (!member || !memberAI || !proto)
            return false;

        if (IsLowerTierArmorForBot(member, proto))
            return false;

        AiObjectContext* ctx = memberAI->GetAiObjectContext();
        if (!ctx)
            return false;

        ItemUsage const otherUsage = ctx->GetValue<ItemUsage>("item usage", param)->Get();
        return otherUsage == ITEM_USAGE_EQUIP || otherUsage == ITEM_USAGE_REPLACE;
    }
} // namespace

static bool GroupHasPrimaryArmorUserLikelyToNeed(Player* self, ItemTemplate const* proto, int32 randomProperty)
{
    if (!self || !proto)
        return false;

    if (proto->Class != ITEM_CLASS_ARMOR || !IsBodyArmorInvType(proto->InventoryType))
        return false;

    std::string const param = ItemUsageValue::BuildItemUsageParam(proto->ItemId, randomProperty);

    return ForEachBotGroupMember(self, [&](Player* member, PlayerbotAI* memberAI) -> bool
    {
        return GroupMemberWouldEquipArmor(member, memberAI, proto, param);
    });
}

static bool GroupHasDesperateUpgradeUser(Player* self, ItemTemplate const* proto, int32 randomProperty)
{
    if (!self || !proto)
        return false;

    if (proto->Class != ITEM_CLASS_ARMOR || !IsBodyArmorInvType(proto->InventoryType))
        return false;

    std::string const param = ItemUsageValue::BuildItemUsageParam(proto->ItemId, randomProperty);

    return ForEachBotGroupMember(self,
        [&](Player* member, PlayerbotAI* memberAI) -> bool
        {
            AiObjectContext* ctx = memberAI->GetAiObjectContext();
            if (!ctx)
                return false;

            ItemUsage usage = ctx->GetValue<ItemUsage>("item usage", param)->Get();
            if (usage != ITEM_USAGE_EQUIP && usage != ITEM_USAGE_REPLACE)
                return false;

            ItemTemplate const* bestProto = nullptr;
            for (uint8 slot = EQUIPMENT_SLOT_START; slot < EQUIPMENT_SLOT_END; ++slot)
            {
                Item* oldItem = member->GetItemByPos(INVENTORY_SLOT_BAG_0, slot);
                if (!oldItem)
                    continue;

                ItemTemplate const* oldProto = oldItem->GetTemplate();
                if (!oldProto)
                    continue;

                if (oldProto->Class != ITEM_CLASS_ARMOR)
                    continue;

                if (oldProto->InventoryType != proto->InventoryType)
                    continue;

                if (!bestProto || oldProto->ItemLevel > bestProto->ItemLevel)
                    bestProto = oldProto;
            }

            bool hasVeryBadItem = !bestProto || bestProto->Quality <= ITEM_QUALITY_NORMAL;
            return hasVeryBadItem;
        });
}

static bool IsDesperateJewelryUpgradeForBot(Player* bot, ItemTemplate const* proto, int32 randomProperty)
{
    if (!bot || !proto)
        return false;

    uint8 jewelrySlots[2];
    uint8 slotsCount = 0;

    switch (proto->InventoryType)
    {
        case INVTYPE_NECK:
            jewelrySlots[0] = EQUIPMENT_SLOT_NECK;
            slotsCount = 1;
            break;
        case INVTYPE_FINGER:
            jewelrySlots[0] = EQUIPMENT_SLOT_FINGER1;
            jewelrySlots[1] = EQUIPMENT_SLOT_FINGER2;
            slotsCount = 2;
            break;
        case INVTYPE_TRINKET:
            jewelrySlots[0] = EQUIPMENT_SLOT_TRINKET1;
            jewelrySlots[1] = EQUIPMENT_SLOT_TRINKET2;
            slotsCount = 2;
            break;
        case INVTYPE_CLOAK:
            jewelrySlots[0] = EQUIPMENT_SLOT_BACK;
            slotsCount = 1;
            break;
        default:
            return false;
    }

    PlayerbotAI* ai = GET_PLAYERBOT_AI(bot);
    if (!ai)
        return false;

    AiObjectContext* ctx = ai->GetAiObjectContext();
    if (!ctx)
        return false;

    std::string const param = ItemUsageValue::BuildItemUsageParam(proto->ItemId, randomProperty);
    ItemUsage const usage = ctx->GetValue<ItemUsage>("item usage", param)->Get();
    if (usage != ITEM_USAGE_EQUIP && usage != ITEM_USAGE_REPLACE)
        return false;

    ItemTemplate const* bestProto = nullptr;
    for (uint8 i = 0; i < slotsCount; ++i)
    {
        uint8 const slot = jewelrySlots[i];
        Item* oldItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slot);
        if (!oldItem)
            continue;

        ItemTemplate const* oldProto = oldItem->GetTemplate();
        if (!oldProto)
            continue;

        if (!bestProto || oldProto->ItemLevel > bestProto->ItemLevel)
            bestProto = oldProto;
    }

    if (!bestProto)
        return true;

    return bestProto->Quality <= ITEM_QUALITY_NORMAL;
}

static bool GroupHasPrimarySpecUpgradeCandidate(Player* self, ItemTemplate const* proto, int32 randomProperty)
{
    if (!self || !proto)
        return false;

    std::string const param = ItemUsageValue::BuildItemUsageParam(proto->ItemId, randomProperty);

    return ForEachBotGroupMember(self,
        [&](Player* member, PlayerbotAI* memberAI) -> bool
        {
            AiObjectContext* ctx = memberAI->GetAiObjectContext();
            if (!ctx)
                return false;

            ItemUsage otherUsage = ctx->GetValue<ItemUsage>("item usage", param)->Get();
            if (otherUsage != ITEM_USAGE_EQUIP && otherUsage != ITEM_USAGE_REPLACE)
                return false;

            if (!IsPrimaryForSpec(member, proto))
                return false;

            return true;
        });
}

static bool IsFallbackNeedReasonableForSpec(Player* bot, ItemTemplate const* proto)
{
    if (!bot || !proto)
        return false;

    SpecTraits const traits = GetSpecTraits(bot);
    uint32 const profile = StatsWeightCalculator::BuildSmartStatMask(bot);
    if (profile == SMARTSTAT_NONE)
        return true;

    ItemStatProfile const stats = BuildItemStatProfile(proto);
    bool const hasAnyStat = stats.hasINT || stats.hasSPI || stats.hasMP5 || stats.hasSP || stats.hasSTR ||
        stats.hasAGI || stats.hasSTA || stats.hasAP || stats.hasARP || stats.hasEXP || stats.hasHIT ||
        stats.hasHASTE || stats.hasCRIT || stats.hasDef || stats.hasAvoid || stats.hasBlockValue;

    if (!hasAnyStat)
        return true;
    if (traits.isTank && !stats.hasSTA && !stats.hasDef && !stats.hasAvoid && !stats.hasBlockValue)
        return false;
    if ((profile & SMARTSTAT_HIT) && stats.hasHIT)
        return true;
    if ((profile & SMARTSTAT_SPELL_POWER) && stats.hasSP)
        return true;
    if ((profile & SMARTSTAT_HASTE) && stats.hasHASTE)
        return true;
    if ((profile & SMARTSTAT_CRIT) && stats.hasCRIT)
        return true;
    if ((profile & SMARTSTAT_INTELLECT) && stats.hasINT)
        return true;
    if ((profile & SMARTSTAT_SPIRIT) && stats.hasSPI)
        return true;
    if ((profile & SMARTSTAT_EXPERTISE) && stats.hasEXP)
        return true;
    if ((profile & SMARTSTAT_ATTACK_POWER) && stats.hasAP)
        return true;
    if ((profile & SMARTSTAT_ARMOR_PEN) && stats.hasARP)
        return true;
    if ((profile & SMARTSTAT_AGILITY) && stats.hasAGI)
        return true;
    if ((profile & SMARTSTAT_STAMINA) && stats.hasSTA)
        return true;
    if ((profile & SMARTSTAT_AVOIDANCE) && (stats.hasAvoid || stats.hasDef))
        return true;
    if ((profile & SMARTSTAT_MP5) && stats.hasMP5)
        return true;
    if ((profile & SMARTSTAT_STRENGTH) && stats.hasSTR)
        return true;

    return false;
}
} // namespace

SpecTraits GetSpecTraits(Player* bot)
{
    SpecTraits t;
    if (!bot)
        return t;
    t.cls = bot->getClass();
    t.spec = AiFactory::GetPlayerSpecName(bot);

    auto specIs = [&](char const* s) { return t.spec == s; };

    const bool pureCasterClass = (t.cls == CLASS_MAGE || t.cls == CLASS_WARLOCK || t.cls == CLASS_PRIEST);

    const bool holyPal = (t.cls == CLASS_PALADIN && specIs("holy"));
    const bool protPal = (t.cls == CLASS_PALADIN && (specIs("prot") || specIs("protection")));
    t.isProtPal = protPal;
    t.isRetPal = (t.cls == CLASS_PALADIN && !holyPal && !protPal);
    const bool dk = (t.cls == CLASS_DEATH_KNIGHT);
    const bool dkBlood = dk && specIs("blood");
    const bool dkFrost = dk && specIs("frost");
    const bool dkUH = dk && (specIs("unholy") || specIs("uh"));
    t.isDKTank = (dkBlood || dkFrost) && !dkUH;
    t.isWarrior = (t.cls == CLASS_WARRIOR);
    t.isWarProt = t.isWarrior && (specIs("prot") || specIs("protection"));
    t.isHunter = (t.cls == CLASS_HUNTER);
    t.isRogue = (t.cls == CLASS_ROGUE);
    const bool eleSham = (t.cls == CLASS_SHAMAN && specIs("elemental"));
    const bool restoSh = (t.cls == CLASS_SHAMAN && (specIs("resto") || specIs("restoration")));
    t.isEnhSham = (t.cls == CLASS_SHAMAN && (specIs("enhance") || specIs("enhancement")));
    const bool balance = (t.cls == CLASS_DRUID && specIs("balance"));
    const bool restoDr = (t.cls == CLASS_DRUID && (specIs("resto") || specIs("restoration")));
    t.isFeralTk = (t.cls == CLASS_DRUID && (specIs("feraltank") || specIs("bear")));
    t.isFeralDps = (t.cls == CLASS_DRUID && (specIs("feraldps") || specIs("cat") || specIs("kitty")));

    t.isHealer = holyPal || restoSh || restoDr || (t.cls == CLASS_PRIEST && !specIs("shadow"));
    t.isTank = protPal || t.isWarProt || t.isFeralTk || t.isDKTank;
    t.isCaster = pureCasterClass || holyPal || eleSham || balance || restoDr || restoSh ||
                 (t.cls == CLASS_PRIEST && specIs("shadow"));
    t.isPhysical = !t.isCaster;
    return t;
}

namespace
{
    static void UpdateItemStatProfileFromStats(ItemTemplate const* proto, ItemStatProfile& s)
    {
        for (uint8 i = 0; i < MAX_ITEM_PROTO_STATS; ++i)
        {
            if (proto->ItemStat[i].ItemStatValue == 0)
                continue;

            switch (proto->ItemStat[i].ItemStatType)
            {
                case ITEM_MOD_INTELLECT:
                    s.hasINT = true;
                    break;
                case ITEM_MOD_SPIRIT:
                    s.hasSPI = true;
                    break;
                case ITEM_MOD_SPELL_POWER:
                    s.hasSP = true;
                    break;
                case ITEM_MOD_SPELL_HEALING_DONE:
                    s.hasSP = true;
                    break;
                case ITEM_MOD_HIT_RATING:
                    s.hasHIT = true;
                    break;
                case ITEM_MOD_CRIT_RATING:
                    s.hasCRIT = true;
                    break;
                case ITEM_MOD_HASTE_RATING:
                    s.hasHASTE = true;
                    break;
                case ITEM_MOD_MANA_REGENERATION:
                    s.hasMP5 = true;
                    break;
                case ITEM_MOD_STRENGTH:
                    s.hasSTR = true;
                    break;
                case ITEM_MOD_AGILITY:
                    s.hasAGI = true;
                    break;
                case ITEM_MOD_ATTACK_POWER:
                    s.hasAP = true;
                    break;
                case ITEM_MOD_ARMOR_PENETRATION_RATING:
                    s.hasARP = true;
                    break;
                case ITEM_MOD_DEFENSE_SKILL_RATING:
                    s.hasDef = true;
                    break;
                case ITEM_MOD_DODGE_RATING:
                case ITEM_MOD_PARRY_RATING:
                case ITEM_MOD_BLOCK_RATING:
                    s.hasAvoid = true;
                    break;
                case ITEM_MOD_BLOCK_VALUE:
                    s.hasBlockValue = true;
                    break;
                default:
                    break;
            }
        }
    }

    static void UpdateItemStatProfileFromSpells(ItemTemplate const* proto, ItemStatProfile& s)
    {
        for (int i = 0; i < MAX_ITEM_PROTO_SPELLS; ++i)
        {
            auto const& spell = proto->Spells[i];
            if (!spell.SpellId ||
                (spell.SpellTrigger != ITEM_SPELLTRIGGER_ON_EQUIP && spell.SpellTrigger != ITEM_SPELLTRIGGER_ON_USE))
                continue;

            SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spell.SpellId);
            if (!spellInfo)
                continue;

            for (int eff = 0; eff < MAX_SPELL_EFFECTS; ++eff)
            {
                SpellEffectInfo const& effectInfo = spellInfo->Effects[eff];
                if (effectInfo.Effect != SPELL_EFFECT_APPLY_AURA)
                    continue;

                if (effectInfo.ApplyAuraName == SPELL_AURA_MOD_HEALING_DONE)
                {
                    s.hasSP = true;
                    break;
                }

                if (effectInfo.ApplyAuraName == SPELL_AURA_MOD_DAMAGE_DONE &&
                    (effectInfo.MiscValue & SPELL_SCHOOL_MASK_MAGIC) == SPELL_SCHOOL_MASK_MAGIC)
                {
                    s.hasSP = true;
                    break;
                }

                if (effectInfo.ApplyAuraName == SPELL_AURA_MOD_RESISTANCE &&
                    (effectInfo.MiscValue & SPELL_SCHOOL_MASK_NORMAL) == SPELL_SCHOOL_MASK_NORMAL)
                {
                    s.hasAvoid = true;
                    break;
                }

                if (effectInfo.ApplyAuraName == SPELL_AURA_MOD_RATING &&
                    (effectInfo.MiscValue == CR_DODGE || effectInfo.MiscValue == CR_PARRY ||
                     effectInfo.MiscValue == CR_BLOCK))
                {
                    s.hasAvoid = true;
                    break;
                }

                if (effectInfo.ApplyAuraName == SPELL_AURA_MOD_RATING &&
                    (effectInfo.MiscValue == CR_HIT_SPELL || effectInfo.MiscValue == CR_CRIT_SPELL ||
                     effectInfo.MiscValue == CR_HASTE_SPELL))
                {
                    s.hasHIT = s.hasHIT || (effectInfo.MiscValue == CR_HIT_SPELL);
                    s.hasCRIT = s.hasCRIT || (effectInfo.MiscValue == CR_CRIT_SPELL);
                    s.hasHASTE = s.hasHASTE || (effectInfo.MiscValue == CR_HASTE_SPELL);
                    break;
                }

                if (effectInfo.ApplyAuraName == SPELL_AURA_MOD_ATTACK_POWER ||
                    effectInfo.ApplyAuraName == SPELL_AURA_MOD_RANGED_ATTACK_POWER)
                {
                    s.hasAP = true;
                    break;
                }
            }

            // Preserve original behavior: stop scanning item spells as soon as SP is detected.
            if (s.hasSP)
                break;
        }
    }
} // namespace

ItemStatProfile BuildItemStatProfile(ItemTemplate const* proto)
{
    ItemStatProfile s;
    if (!proto)
        return s;

    UpdateItemStatProfileFromStats(proto, s);

    s.hasAvoid = s.hasAvoid || HasAnyTankAvoidance(proto);

    UpdateItemStatProfileFromSpells(proto, s);

    return s;
}

namespace
{
static bool IsPrimaryForSpec(Player* bot, ItemTemplate const* proto)
{
    if (!bot || !proto)
        return false;

    const SpecTraits traits = GetSpecTraits(bot);
    ItemStatProfile const stats = BuildItemStatProfile(proto);
    bool const hasPhysical = stats.hasSTR || stats.hasAGI || stats.hasAP || stats.hasARP;
    bool const hasCasterPrimary = stats.hasINT || stats.hasSP || stats.hasMP5;
    bool const hasCasterRatings = stats.hasHIT || stats.hasCRIT || stats.hasHASTE;
    bool const hasCaster = hasCasterPrimary || (hasCasterRatings && !hasPhysical);

    // Caster offense ratings should only be considered in the caster context.
    bool const hasCasterOffense = hasCaster && hasCasterRatings;

    if (proto->InventoryType == INVTYPE_RELIC)
    {
        if (!IsRelicForClass(proto, traits.cls))
            return false;

        return IsFallbackNeedReasonableForSpec(bot, proto);
    }

    if (proto->Class == ITEM_CLASS_WEAPON)
    {
        if ((traits.isHunter || traits.isRogue || traits.isEnhSham) && stats.hasSTR)
            return false;

        if ((traits.isProtPal || traits.isWarProt) && proto->InventoryType == INVTYPE_2HWEAPON)
            return false;

        if (traits.isTank && hasCaster)
            return false;

        if (!traits.isCaster && hasCaster && !hasPhysical)
            return false;

        if (traits.isCaster && hasPhysical && !hasCaster)
            return false;

        if (traits.isCaster)
        {
            bool const isCasterWeapon =
                proto->SubClass == ITEM_SUBCLASS_WEAPON_STAFF || proto->SubClass == ITEM_SUBCLASS_WEAPON_DAGGER ||
                proto->SubClass == ITEM_SUBCLASS_WEAPON_SWORD || proto->SubClass == ITEM_SUBCLASS_WEAPON_MACE ||
                proto->SubClass == ITEM_SUBCLASS_WEAPON_WAND;
             return isCasterWeapon && (hasCaster || (hasCasterOffense && !hasPhysical));
        }

        return hasPhysical;
    }

    if (proto->Class != ITEM_CLASS_ARMOR)
        return true;

    if (traits.cls == CLASS_DEATH_KNIGHT && stats.hasAGI)
        return false;

    if ((traits.isHunter || traits.isEnhSham) && stats.hasSTR)
        return false;

    if ((traits.isHunter || traits.isRogue || traits.isEnhSham) && stats.hasSTR)
        return false;

    if (IsJewelryOrCloak(proto))
    {
        if (traits.isCaster != hasCaster)
            return false;
        return IsFallbackNeedReasonableForSpec(bot, proto);
    }

    if (IsLowerTierArmorForBot(bot, proto))
        return false;

    if (!traits.isTank && HasAnyTankAvoidance(proto))
        return false;

    if (traits.isTank && HasAnyTankAvoidance(proto))
        return true;

    if (traits.isHealer && stats.hasHIT)
        return false;

    if (traits.isCaster)
        return hasCaster || (hasCasterOffense && !hasPhysical);

    if (traits.isPhysical)
        return hasPhysical && !hasCaster;

    return true;
}

static ItemUsage AdjustUsageForOffspec(Player* bot, ItemTemplate const* proto, int32 randomProperty, ItemUsage usage)
{
    if (!bot || !proto)
        return usage;

    if (!sPlayerbotAIConfig.smartNeedBySpec)
        return usage;

    if (usage != ITEM_USAGE_EQUIP && usage != ITEM_USAGE_REPLACE)
        return usage;

    if (IsPrimaryForSpec(bot, proto))
        return usage;

    if (!IsFallbackNeedReasonableForSpec(bot, proto))
        return ITEM_USAGE_BAD_EQUIP;

    if (IsJewelryOrCloak(proto) && !IsDesperateJewelryUpgradeForBot(bot, proto, randomProperty))
        return ITEM_USAGE_BAD_EQUIP;

    return ITEM_USAGE_BAD_EQUIP;
}

static ItemUsage AdjustUsageForCrossArmor(Player* bot, ItemTemplate const* proto, int32 randomProperty, ItemUsage usage)
{
    if (!bot || !proto)
        return usage;

    if (usage != ITEM_USAGE_BAD_EQUIP)
        return usage;

    if (proto->Class != ITEM_CLASS_ARMOR || !IsLowerTierArmorForBot(bot, proto))
        return usage;

    // Endgame etiquette: do not allow cross-armor upgrades to turn into NEED
    // at level cap or in raids.
    if (IsStrictCrossArmorContext(bot))
        return usage;

    if (sPlayerbotAIConfig.crossArmorGreedIsPass)
        return ITEM_USAGE_NONE;

    if (EnableGroupUsageChecks() && GroupHasPrimaryArmorUserLikelyToNeed(bot, proto, randomProperty))
        return usage;

    if (EnableGroupUsageChecks() && GroupHasDesperateUpgradeUser(bot, proto, randomProperty))
        return usage;

    if (!IsFallbackNeedReasonableForSpec(bot, proto))
        return usage;

    float newScore = sRandomItemMgr.CalculateItemWeight(bot, proto->ItemId, randomProperty);
    if (newScore <= 0.0f)
        return usage;
    float bestOld = 0.0f;

    for (uint8 slot = EQUIPMENT_SLOT_START; slot < EQUIPMENT_SLOT_END; ++slot)
    {
        Item* oldItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slot);
        if (!oldItem)
            continue;

        ItemTemplate const* oldProto = oldItem->GetTemplate();
        if (!oldProto)
            continue;

        if (oldProto->Class != ITEM_CLASS_ARMOR)
            continue;

        if (oldProto->InventoryType != proto->InventoryType)
            continue;

        if (oldProto->Quality <= ITEM_QUALITY_NORMAL)
            continue;

        float oldScore = sRandomItemMgr.CalculateItemWeight(
            bot, oldProto->ItemId, oldItem->GetInt32Value(ITEM_FIELD_RANDOM_PROPERTIES_ID));

        if (oldScore > bestOld)
            bestOld = oldScore;
    }

    if (bestOld <= 0.0f)
        return ITEM_USAGE_EQUIP;

    uint32 const maxLevel = static_cast<uint32>(sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL));
    bool const isLeveling = bot->GetLevel() < maxLevel;
    SpecTraits const traits = GetSpecTraits(bot);
    float const margin = (traits.isHealer && isLeveling) ? sPlayerbotAIConfig.equipUpgradeThreshold
                                                         : sPlayerbotAIConfig.crossArmorExtraMargin;

    if (bestOld > 0.0f && newScore >= bestOld * margin)
        return ITEM_USAGE_EQUIP;

    return usage;
}

static bool IsUniqueItemAlreadyEquipped(Player* bot, ItemTemplate const* proto, InventoryResult equipResult)
{
    if (!bot || !proto)
        return false;

    // CanEquipItem may return EQUIP_ERR_CANT_CARRY_MORE_OF_THIS for unique-equip items,
    // but it can also succeed while the item is flagged UNIQUE_EQUIPPABLE.
    bool const needToCheckUnique =
        (equipResult == EQUIP_ERR_CANT_CARRY_MORE_OF_THIS) || proto->HasFlag(ITEM_FLAG_UNIQUE_EQUIPPABLE);

    if (!needToCheckUnique)
        return false;

    // Count the total number of the item (equipped + in bags)
    uint32 const totalItemCount = bot->GetItemCount(proto->ItemId, true);
    // Count the number of the item in bags only
    uint32 const bagItemCount = bot->GetItemCount(proto->ItemId, false);

    // If total > bag-only, at least one copy is equipped.
    return totalItemCount > bagItemCount;
}
} // namespace

ItemUsage LootUsageValue::Calculate()
{
    ParsedItemUsage parsed = ParseItemUsageQualifier(qualifier);
    if (!parsed.itemId)
        return ITEM_USAGE_NONE;

    ItemUsage usage = ItemUsageValue::Calculate();
    ItemTemplate const* proto = sObjectMgr->GetItemTemplate(parsed.itemId);
    if (!proto)
        return usage;

    usage = AdjustUsageForOffspec(bot, proto, parsed.randomPropertyId, usage);
    usage = AdjustUsageForCrossArmor(bot, proto, parsed.randomPropertyId, usage);

    return usage;
}

ItemUsage ItemUsageValue::QueryItemUsageForEquip(ItemTemplate const* itemProto, int32 randomPropertyId)
{
    if (!sRandomItemMgr.CanEquipForBot(bot, itemProto))
        return ITEM_USAGE_NONE;

    Item* pItem = Item::CreateItem(itemProto->ItemId, 1, nullptr, false, 0, true);
    if (!pItem)
        return ITEM_USAGE_NONE;

    uint16 dest = 0;
    InventoryResult result = botAI->CanEquipItem(NULL_SLOT, dest, pItem, true, true);
    delete pItem;

    if (result != EQUIP_ERR_OK && result != EQUIP_ERR_CANT_CARRY_MORE_OF_THIS)
    {
        return ITEM_USAGE_NONE;
    }

    if (IsUniqueItemAlreadyEquipped(bot, itemProto, result))
        return ITEM_USAGE_NONE;

    if (itemProto->Class == ITEM_CLASS_QUIVER)
        if (bot->getClass() != CLASS_HUNTER)
            return ITEM_USAGE_NONE;

    if (itemProto->Class == ITEM_CLASS_CONTAINER)
    {
        if (itemProto->SubClass != ITEM_SUBCLASS_CONTAINER)
            return ITEM_USAGE_NONE;  // Todo add logic for non-bag containers. We want to look at professions/class and
                                     // only replace if non-bag is larger than bag.

        if (GetSmallestBagSize() >= itemProto->ContainerSlots)
            return ITEM_USAGE_NONE;

        return ITEM_USAGE_EQUIP;
    }

    bool shouldEquip = false;
    // uint32 statWeight = sRandomItemMgr.GetLiveStatWeight(bot, itemProto->ItemId);
    StatsWeightCalculator calculator(bot);
    calculator.SetItemSetBonus(false);
    calculator.SetOverflowPenalty(false);

    float itemScore = calculator.CalculateItem(itemProto->ItemId, randomPropertyId);

    if (itemScore)
        shouldEquip = true;

    if (itemProto->Class == ITEM_CLASS_WEAPON && !sRandomItemMgr.CanEquipWeapon(bot->getClass(), itemProto))
        shouldEquip = false;

    if ((itemProto->Class == ITEM_CLASS_ARMOR &&
         !sRandomItemMgr.CanEquipArmor(bot->getClass(), bot->GetLevel(), itemProto)) ||
        !sRandomItemMgr.CanEquipForBot(bot, itemProto))
        shouldEquip = false;

    uint8 possibleSlots = 1;
    uint8 dstSlot = botAI->FindEquipSlot(itemProto, NULL_SLOT, true);
    // Check if dest wasn't set correctly by CanEquipItem and use FindEquipSlot instead
    // This occurs with unique items that are already in the bots bags when CanEquipItem is called
    if (dest == 0)
    {
        if (dstSlot != NULL_SLOT)
        {
            // Construct dest from dstSlot
            dest = (INVENTORY_SLOT_BAG_0 << 8) | dstSlot;
        }
    }

    if (dstSlot == EQUIPMENT_SLOT_FINGER1 || dstSlot == EQUIPMENT_SLOT_TRINKET1)
    {
        possibleSlots = 2;
    }

    // Check weapon case separately to keep things a bit cleaner
    bool have2HWeapon = false;
    bool isValidTGWeapon = false;

    if (dstSlot == EQUIPMENT_SLOT_MAINHAND)
    {
        Item* currentWeapon = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
        have2HWeapon = currentWeapon && currentWeapon->GetTemplate()->InventoryType == INVTYPE_2HWEAPON;

        // Determine if the new weapon is a valid Titan Grip weapon
        isValidTGWeapon = (itemProto->SubClass == ITEM_SUBCLASS_WEAPON_AXE2 ||
                           itemProto->SubClass == ITEM_SUBCLASS_WEAPON_MACE2 ||
                           itemProto->SubClass == ITEM_SUBCLASS_WEAPON_SWORD2);

        // If the bot can Titan Grip, ignore any 2H weapon that isn't a 2H sword, mace, or axe.
        if (bot->CanTitanGrip())
        {
            // If this weapon is 2H but not one of the valid TG weapon types, do not equip it at all.
            if (itemProto->InventoryType == INVTYPE_2HWEAPON && !isValidTGWeapon)
            {
                return ITEM_USAGE_NONE;
            }
        }

        // Now handle the logic for equipping and possible offhand slots
        // If the bot can Dual Wield and:
        // - The weapon is not 2H and we currently don't have a 2H weapon equipped
        // OR
        // - The bot can Titan Grip and it is a valid TG weapon
        // Then we can consider the offhand slot as well.
        if (bot->CanDualWield() &&
            ((itemProto->InventoryType != INVTYPE_2HWEAPON && !have2HWeapon) ||
             (bot->CanTitanGrip() && isValidTGWeapon)))
        {
            possibleSlots = 2;
        }
    }

    for (uint8 i = 0; i < possibleSlots; i++)
    {
        bool shouldEquipInSlot = shouldEquip;
        Item* oldItem = bot->GetItemByPos(dest + i);

        // No item equipped
        if (!oldItem)
        {
            if (shouldEquipInSlot || IsFallbackNeedReasonableForSpec(bot, itemProto))
                return ITEM_USAGE_EQUIP;

            return ITEM_USAGE_BAD_EQUIP;
        }

        ItemTemplate const* oldItemProto = oldItem->GetTemplate();
        if (oldItemProto && oldItemProto->Quality <= ITEM_QUALITY_POOR &&
            itemProto->Quality >= ITEM_QUALITY_UNCOMMON &&
            IsFallbackNeedReasonableForSpec(bot, itemProto))
            return ITEM_USAGE_EQUIP;

        float oldScore = calculator.CalculateItem(oldItemProto->ItemId, oldItem->GetInt32Value(ITEM_FIELD_RANDOM_PROPERTIES_ID));

        // uint32 oldStatWeight = sRandomItemMgr->GetLiveStatWeight(bot, oldItemProto->ItemId);
        if (itemScore || oldScore)
            shouldEquipInSlot = itemScore > oldScore * sPlayerbotAIConfig.equipUpgradeThreshold;

        // Bigger quiver
        if (itemProto->Class == ITEM_CLASS_QUIVER)
        {
            if (!oldItem || oldItemProto->ContainerSlots < itemProto->ContainerSlots)
                return ITEM_USAGE_EQUIP;

            return ITEM_USAGE_NONE;
        }

        bool existingShouldEquip = true;

        if (oldItemProto->Class == ITEM_CLASS_WEAPON && !sRandomItemMgr.CanEquipWeapon(bot->getClass(), oldItemProto))
            existingShouldEquip = false;

        if ((oldItemProto->Class == ITEM_CLASS_ARMOR &&
             !sRandomItemMgr.CanEquipArmor(bot->getClass(), bot->GetLevel(), oldItemProto)) ||
            !sRandomItemMgr.CanEquipForBot(bot, oldItemProto))
            existingShouldEquip = false;

        // Compare items based on item level, quality or itemId.
        const bool isBetter = itemScore > oldScore;

        Item* item = CurrentItem(itemProto);
        bool itemIsBroken =
            item && item->GetUInt32Value(ITEM_FIELD_DURABILITY) == 0 && item->GetUInt32Value(ITEM_FIELD_MAXDURABILITY) > 0;
        bool oldItemIsBroken =
            oldItem->GetUInt32Value(ITEM_FIELD_DURABILITY) == 0 && oldItem->GetUInt32Value(ITEM_FIELD_MAXDURABILITY) > 0;

        if (itemProto->ItemId != oldItemProto->ItemId && (shouldEquipInSlot || !existingShouldEquip) && isBetter)
        {
            switch (itemProto->Class)
            {
                case ITEM_CLASS_ARMOR:
                    if (oldItemProto->SubClass <= itemProto->SubClass)
                    {
                        // Need to add some logic to check second slot before returning, but as it happens, all three of these
                        // return vals will result in an attempted equip action so it wouldn't have much effect currently
                        if (itemIsBroken && !oldItemIsBroken)
                            return ITEM_USAGE_BROKEN_EQUIP;
                        if (shouldEquipInSlot)
                            return ITEM_USAGE_REPLACE;

                        return ITEM_USAGE_BAD_EQUIP;
                    }
                default:
                {
                    if (itemIsBroken && !oldItemIsBroken)
                        return ITEM_USAGE_BROKEN_EQUIP;

                    return shouldEquipInSlot ? ITEM_USAGE_EQUIP : ITEM_USAGE_BAD_EQUIP;
                }
            }
        }

        // Item is not better but current item is broken and new one is not.
        if (oldItemIsBroken && !itemIsBroken)
            return ITEM_USAGE_EQUIP;
    }
    return ITEM_USAGE_NONE;
}

ItemUsage ItemUsageValue::QueryItemUsageForAmmo(ItemTemplate const* proto)
{
    // Class gate for ammo logic:
    // proceed only for Hunter/Rogue/Warrior.
    // This must use && on the "!=" checks, otherwise the condition is always true
    // and QueryItemUsageForAmmo() always returns ITEM_USAGE_NONE.	
    if (bot->getClass() != CLASS_HUNTER && bot->getClass() != CLASS_ROGUE && bot->getClass() != CLASS_WARRIOR)
        return ITEM_USAGE_NONE;

    Item* rangedWeapon = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_RANGED);
    uint32 requiredSubClass = 0;

    if (rangedWeapon)
    {
        switch (rangedWeapon->GetTemplate()->SubClass)
        {
            case ITEM_SUBCLASS_WEAPON_GUN:
                requiredSubClass = ITEM_SUBCLASS_BULLET;
                break;
            case ITEM_SUBCLASS_WEAPON_BOW:
            case ITEM_SUBCLASS_WEAPON_CROSSBOW:
                requiredSubClass = ITEM_SUBCLASS_ARROW;
                break;
        }
    }

    // Ensure the item is the correct ammo type for the equipped ranged weapon
    if (proto->SubClass == requiredSubClass)
    {
        float ammoCount = BetterStacks(proto, "ammo");
        float requiredAmmo = (bot->getClass() == CLASS_HUNTER) ? 8 : 2; // Hunters get 8 stacks, others 2
        uint32 currentAmmoId = bot->GetUInt32Value(PLAYER_AMMO_ID);

        // Check if the bot has an ammo type assigned
        if (currentAmmoId == 0)
            return ITEM_USAGE_EQUIP;  // Equip the ammo if no ammo
        // Compare new ammo vs current equipped ammo
        ItemTemplate const* currentAmmoProto = sObjectMgr->GetItemTemplate(currentAmmoId);
        if (currentAmmoProto)
        {
            uint32 currentAmmoDPS = (currentAmmoProto->Damage[0].DamageMin + currentAmmoProto->Damage[0].DamageMax) * 1000 / 2;
            uint32 newAmmoDPS = (proto->Damage[0].DamageMin + proto->Damage[0].DamageMax) * 1000 / 2;

            if (newAmmoDPS > currentAmmoDPS) // New ammo meets upgrade condition
                return ITEM_USAGE_EQUIP;

            if (newAmmoDPS < currentAmmoDPS) // New ammo is worse
                return ITEM_USAGE_NONE;
        }
        // Ensure we have enough ammo in the inventory
        if (ammoCount < requiredAmmo)
        {
            ammoCount += CurrentStacks(proto);

            if (ammoCount < requiredAmmo)  // Buy ammo to reach the proper supply
                return ITEM_USAGE_AMMO;
            else if (ammoCount < requiredAmmo + 1)
                return ITEM_USAGE_KEEP;  // Keep the ammo if we don't have too much.
        }
    }
    return ITEM_USAGE_NONE;
}

// Return smaltest bag size equipped
uint32 ItemUsageValue::GetSmallestBagSize()
{
    int8 curSlot = 0;
    uint32 curSlots = 0;
    for (uint8 bag = INVENTORY_SLOT_BAG_START + 1; bag < INVENTORY_SLOT_BAG_END; ++bag)
    {
        if (Bag const* pBag = (Bag*)bot->GetItemByPos(INVENTORY_SLOT_BAG_0, bag))
        {
            if (curSlot > 0 && curSlots < pBag->GetBagSize())
                continue;

            curSlot = pBag->GetSlot();
            curSlots = pBag->GetBagSize();
        }
        else
            return 0;
    }

    return curSlots;
}

bool ItemUsageValue::IsItemUsefulForQuest(Player* player, ItemTemplate const* proto)
{
    PlayerbotAI* botAI = GET_PLAYERBOT_AI(player);
    if (!botAI)
        return false;

    for (uint8 slot = 0; slot < MAX_QUEST_LOG_SIZE; ++slot)
    {
        uint32 entry = player->GetQuestSlotQuestId(slot);
        Quest const* quest = sObjectMgr->GetQuestTemplate(entry);
        if (!quest)
            continue;

        // Check if the item itself is needed for the quest
        for (uint8 i = 0; i < 4; i++)
        {
            if (quest->RequiredItemId[i] == proto->ItemId)
            {
                if (player->GetItemCount(proto->ItemId, false) >= quest->RequiredItemCount[i])
                    continue;

                return true; // Item is directly required for a quest
            }
        }

        // Check if the item has spells that create a required quest item
        for (uint8 i = 0; i < MAX_ITEM_SPELLS; i++)
        {
            uint32 spellId = proto->Spells[i].SpellId;
            if (!spellId)
                continue;

            SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
            if (!spellInfo)
                continue;

            for (uint8 effectIndex = 0; effectIndex < MAX_SPELL_EFFECTS; effectIndex++)
            {
                if (spellInfo->Effects[effectIndex].Effect == SPELL_EFFECT_CREATE_ITEM)
                {
                    uint32 createdItemId = spellInfo->Effects[effectIndex].ItemType;

                    // Check if the created item is required for a quest
                    for (uint8 j = 0; j < 4; j++)
                    {
                        if (quest->RequiredItemId[j] == createdItemId)
                        {
                            if (player->GetItemCount(createdItemId, false) >= quest->RequiredItemCount[j])
                                continue;

                            return true; // Item is useful because it creates a required quest item
                        }
                    }
                }
            }
        }
    }

    return false; // Item is not useful for any active quests
}

namespace
{
    // Tools / profession items used by IsItemNeededForSkill().
    constexpr uint32 ITEM_TUNNEL_PICK                = 756;
    constexpr uint32 ITEM_KOBOLD_EXCAVATION_PICK     = 778;
    constexpr uint32 ITEM_GOUGING_PICK               = 1819;
    constexpr uint32 ITEM_MINERS_REVENGE             = 1893;
    constexpr uint32 ITEM_COLD_IRON_PICK             = 1959;
    constexpr uint32 ITEM_MINING_PICK                = 2901;
    constexpr uint32 ITEM_DIGMASTER_5000             = 9465;
    constexpr uint32 ITEM_BRANNS_TRUSTY_PICK          = 20723;
    constexpr uint32 ITEM_GNOMISH_ARMY_KNIFE          = 40772;
    constexpr uint32 ITEM_HAMMER_PICK                = 40892;
    constexpr uint32 ITEM_BLADED_PICKAXE             = 40893;
    constexpr uint32 ITEM_BLACKSMITH_HAMMER          = 5956;
    constexpr uint32 ITEM_ARCLIGHT_SPANNER           = 6219;
    constexpr uint32 ITEM_RUNED_COPPER_ROD           = 6218;
    constexpr uint32 ITEM_RUNED_SILVER_ROD           = 6339;
    constexpr uint32 ITEM_RUNED_GOLDEN_ROD           = 11130;
    constexpr uint32 ITEM_RUNED_TRUESILVER_ROD       = 11145;
    constexpr uint32 ITEM_RUNED_ARCANITE_ROD         = 16207;
    constexpr uint32 ITEM_SKINNING_KNIFE             = 7005;
    constexpr uint32 ITEM_SKINNING_KNIFE_ZULIAN      = 12709;
    constexpr uint32 ITEM_SKINNING_KNIFE_ZULIAN_2    = 19901;
    constexpr uint32 ITEM_FLINT_AND_TINDER           = 4471;
    constexpr uint32 ITEM_SIMPLE_WOOD                = 4470;
    constexpr uint32 ITEM_FISHING_ROD                = 6256;
} // namespace

bool ItemUsageValue::IsItemNeededForSkill(ItemTemplate const* proto)
{
    switch (proto->ItemId)
    {
        case ITEM_TUNNEL_PICK:
            return botAI->HasSkill(SKILL_MINING);

        case ITEM_KOBOLD_EXCAVATION_PICK:
            return botAI->HasSkill(SKILL_MINING);

        case ITEM_GOUGING_PICK:
            return botAI->HasSkill(SKILL_MINING);

        case ITEM_MINERS_REVENGE:
            return botAI->HasSkill(SKILL_MINING);

        case ITEM_COLD_IRON_PICK:
            return botAI->HasSkill(SKILL_MINING);

        case ITEM_MINING_PICK:
            return botAI->HasSkill(SKILL_MINING);

        case ITEM_DIGMASTER_5000:
            return botAI->HasSkill(SKILL_MINING);

        case ITEM_BRANNS_TRUSTY_PICK:
            return botAI->HasSkill(SKILL_MINING);

        case ITEM_GNOMISH_ARMY_KNIFE:
            return botAI->HasSkill(SKILL_MINING) || botAI->HasSkill(SKILL_ENGINEERING) ||
                   botAI->HasSkill(SKILL_BLACKSMITHING) || botAI->HasSkill(SKILL_COOKING) ||
                   botAI->HasSkill(SKILL_SKINNING);

        case ITEM_HAMMER_PICK:
            return botAI->HasSkill(SKILL_MINING) || botAI->HasSkill(SKILL_BLACKSMITHING);

        case ITEM_BLADED_PICKAXE:
            return botAI->HasSkill(SKILL_MINING) || botAI->HasSkill(SKILL_SKINNING);

        case ITEM_BLACKSMITH_HAMMER:
            return botAI->HasSkill(SKILL_BLACKSMITHING) || botAI->HasSkill(SKILL_ENGINEERING);

        case ITEM_ARCLIGHT_SPANNER:
            return botAI->HasSkill(SKILL_ENGINEERING);

        case ITEM_RUNED_COPPER_ROD:
            return botAI->HasSkill(SKILL_ENCHANTING);

        case ITEM_RUNED_SILVER_ROD:
            return botAI->HasSkill(SKILL_ENCHANTING);

        case ITEM_RUNED_GOLDEN_ROD:
            return botAI->HasSkill(SKILL_ENCHANTING);

        case ITEM_RUNED_TRUESILVER_ROD:
            return botAI->HasSkill(SKILL_ENCHANTING);

        case ITEM_RUNED_ARCANITE_ROD:
            return botAI->HasSkill(SKILL_ENCHANTING);

        case ITEM_SKINNING_KNIFE:
            return botAI->HasSkill(SKILL_SKINNING);

        case ITEM_SKINNING_KNIFE_ZULIAN:
            return botAI->HasSkill(SKILL_SKINNING);

        case ITEM_SKINNING_KNIFE_ZULIAN_2:
            return botAI->HasSkill(SKILL_SKINNING);

        case ITEM_FLINT_AND_TINDER:
            return botAI->HasSkill(SKILL_COOKING);

        case ITEM_SIMPLE_WOOD:
            return botAI->HasSkill(SKILL_COOKING);

        case ITEM_FISHING_ROD:
            return botAI->HasSkill(SKILL_FISHING);
    }

    return false;
}

bool ItemUsageValue::IsItemUsefulForSkill(ItemTemplate const* proto)
{
    switch (proto->Class)
    {
        case ITEM_CLASS_TRADE_GOODS:
        case ITEM_CLASS_MISC:
        case ITEM_CLASS_REAGENT:
        case ITEM_CLASS_GEM:
        {
            if (botAI->HasSkill(SKILL_TAILORING) && RandomItemMgr::IsUsedBySkill(proto, SKILL_TAILORING))
                return true;
            if (botAI->HasSkill(SKILL_LEATHERWORKING) && RandomItemMgr::IsUsedBySkill(proto, SKILL_LEATHERWORKING))
                return true;
            if (botAI->HasSkill(SKILL_ENGINEERING) && RandomItemMgr::IsUsedBySkill(proto, SKILL_ENGINEERING))
                return true;
            if (botAI->HasSkill(SKILL_BLACKSMITHING) && RandomItemMgr::IsUsedBySkill(proto, SKILL_BLACKSMITHING))
                return true;
            if (botAI->HasSkill(SKILL_ALCHEMY) && RandomItemMgr::IsUsedBySkill(proto, SKILL_ALCHEMY))
                return true;
            if (botAI->HasSkill(SKILL_ENCHANTING) && RandomItemMgr::IsUsedBySkill(proto, SKILL_ENCHANTING))
                return true;
            if (botAI->HasSkill(SKILL_FISHING) && RandomItemMgr::IsUsedBySkill(proto, SKILL_FISHING))
                return true;
            if (botAI->HasSkill(SKILL_FIRST_AID) && RandomItemMgr::IsUsedBySkill(proto, SKILL_FIRST_AID))
                return true;
            if (botAI->HasSkill(SKILL_COOKING) && RandomItemMgr::IsUsedBySkill(proto, SKILL_COOKING))
                return true;
            if (botAI->HasSkill(SKILL_JEWELCRAFTING) && RandomItemMgr::IsUsedBySkill(proto, SKILL_JEWELCRAFTING))
                return true;
            if (botAI->HasSkill(SKILL_MINING) && (RandomItemMgr::IsUsedBySkill(proto, SKILL_MINING) ||
                                                  RandomItemMgr::IsUsedBySkill(proto, SKILL_BLACKSMITHING) ||
                                                  RandomItemMgr::IsUsedBySkill(proto, SKILL_JEWELCRAFTING) ||
                                                  RandomItemMgr::IsUsedBySkill(proto, SKILL_ENGINEERING)))
                return true;
            if (botAI->HasSkill(SKILL_SKINNING) && (RandomItemMgr::IsUsedBySkill(proto, SKILL_SKINNING) ||
                                                    RandomItemMgr::IsUsedBySkill(proto, SKILL_LEATHERWORKING)))
                return true;
            if (botAI->HasSkill(SKILL_HERBALISM) && (RandomItemMgr::IsUsedBySkill(proto, SKILL_HERBALISM) ||
                                                     RandomItemMgr::IsUsedBySkill(proto, SKILL_ALCHEMY)))
                return true;

            return false;
        }
        case ITEM_CLASS_RECIPE:
        {
            if (bot->HasSpell(proto->Spells[2].SpellId))
                break;

            switch (proto->SubClass)
            {
                case ITEM_SUBCLASS_LEATHERWORKING_PATTERN:
                    return botAI->HasSkill(SKILL_LEATHERWORKING);
                case ITEM_SUBCLASS_TAILORING_PATTERN:
                    return botAI->HasSkill(SKILL_TAILORING);
                case ITEM_SUBCLASS_ENGINEERING_SCHEMATIC:
                    return botAI->HasSkill(SKILL_ENGINEERING);
                case ITEM_SUBCLASS_BLACKSMITHING:
                    return botAI->HasSkill(SKILL_BLACKSMITHING);
                case ITEM_SUBCLASS_COOKING_RECIPE:
                    return botAI->HasSkill(SKILL_COOKING);
                case ITEM_SUBCLASS_ALCHEMY_RECIPE:
                    return botAI->HasSkill(SKILL_ALCHEMY);
                case ITEM_SUBCLASS_FIRST_AID_MANUAL:
                    return botAI->HasSkill(SKILL_FIRST_AID);
                case ITEM_SUBCLASS_ENCHANTING_FORMULA:
                    return botAI->HasSkill(SKILL_ENCHANTING);
                case ITEM_SUBCLASS_FISHING_MANUAL:
                    return botAI->HasSkill(SKILL_FISHING);
            }
        }
    }

    return false;
}

bool ItemUsageValue::IsItemNeededForUsefullSpell(ItemTemplate const* proto, bool checkAllReagents)
{
    for (auto spellId : SpellsUsingItem(proto->ItemId, bot))
    {
        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
        if (!spellInfo)
            continue;

        if (checkAllReagents && !HasItemsNeededForSpell(spellId, proto))
            continue;

        if (SpellGivesSkillUp(spellId, bot))
            return true;

        uint32 newItemId = spellInfo->Effects[EFFECT_0].ItemType;
        if (newItemId && newItemId != proto->ItemId)
        {
            ItemUsage usage = AI_VALUE2(ItemUsage, "item usage", newItemId);

            if (usage != ITEM_USAGE_REPLACE && usage != ITEM_USAGE_EQUIP && usage != ITEM_USAGE_AMMO &&
                usage != ITEM_USAGE_QUEST && usage != ITEM_USAGE_SKILL && usage != ITEM_USAGE_USE)
                continue;

            return true;
        }
    }

    return false;
}

bool ItemUsageValue::HasItemsNeededForSpell(uint32 spellId, ItemTemplate const* proto)
{
    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
    if (!spellInfo)
        return false;

    for (uint8 i = 0; i < MAX_SPELL_REAGENTS; i++)
        if (spellInfo->ReagentCount[i] > 0 && spellInfo->Reagent[i])
        {
            if (proto && proto->ItemId == spellInfo->Reagent[i] &&
                spellInfo->ReagentCount[i] == 1)  // If we only need 1 item then current item does not need to be
                                                  // checked since we are looting/buying or already have it.
                continue;

            ItemTemplate const* reqProto = sObjectMgr->GetItemTemplate(spellInfo->Reagent[i]);

            uint32 count = AI_VALUE2(uint32, "item count", reqProto->Name1);

            if (count < spellInfo->ReagentCount[i])
                return false;
        }

    return true;
}

Item* ItemUsageValue::CurrentItem(ItemTemplate const* proto)
{
    Item* bestItem = nullptr;
    std::vector<Item*> found = AI_VALUE2(std::vector<Item*>, "inventory items", chat->FormatItem(proto));
    for (auto item : found)
    {
        if (bestItem && item->GetUInt32Value(ITEM_FIELD_DURABILITY) < bestItem->GetUInt32Value(ITEM_FIELD_DURABILITY))
            continue;

        if (bestItem && item->GetCount() < bestItem->GetCount())
            continue;

        bestItem = item;
    }

    return bestItem;
}

float ItemUsageValue::CurrentStacks(ItemTemplate const* proto)
{
    uint32 maxStack = proto->GetMaxStackSize();

    std::vector<Item*> found = AI_VALUE2(std::vector<Item*>, "inventory items", chat->FormatItem(proto));

    float itemCount = 0;

    for (auto stack : found)
    {
        itemCount += stack->GetCount();
    }

    return itemCount / maxStack;
}

float ItemUsageValue::BetterStacks(ItemTemplate const* proto, std::string const itemType)
{
    std::vector<Item*> items = AI_VALUE2(std::vector<Item*>, "inventory items", itemType);

    float stacks = 0;

    for (auto& otherItem : items)
    {
        ItemTemplate const* otherProto = otherItem->GetTemplate();

        if (otherProto->Class != proto->Class || otherProto->SubClass != proto->SubClass)
            continue;

        if (otherProto->ItemLevel < proto->ItemLevel)
            continue;

        if (otherProto->ItemId == proto->ItemId)
            continue;

        stacks += CurrentStacks(otherProto);
    }

    return stacks;
}

std::vector<uint32> ItemUsageValue::SpellsUsingItem(uint32 itemId, Player* bot)
{
    std::vector<uint32> retSpells;

    PlayerSpellMap const& spellMap = bot->GetSpellMap();

    for (auto& spell : spellMap)
    {
        uint32 spellId = spell.first;

        if (spell.second->State == PLAYERSPELL_REMOVED || !spell.second->Active)
            continue;

        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
        if (!spellInfo)
            continue;

        if (spellInfo->IsPassive())
            continue;

        if (spellInfo->Effects[EFFECT_0].Effect != SPELL_EFFECT_CREATE_ITEM)
            continue;

        for (uint8 i = 0; i < MAX_SPELL_REAGENTS; i++)
            if (spellInfo->ReagentCount[i] > 0 && spellInfo->Reagent[i] == itemId)
                retSpells.push_back(spellId);
    }

    return retSpells;
}

inline int32 SkillGainChance(uint32 SkillValue, uint32 GrayLevel, uint32 GreenLevel, uint32 YellowLevel)
{
    if (SkillValue >= GrayLevel)
        return sWorld->getIntConfig(CONFIG_SKILL_CHANCE_GREY) * 10;

    if (SkillValue >= GreenLevel)
        return sWorld->getIntConfig(CONFIG_SKILL_CHANCE_GREEN) * 10;

    if (SkillValue >= YellowLevel)
        return sWorld->getIntConfig(CONFIG_SKILL_CHANCE_YELLOW) * 10;

    return sWorld->getIntConfig(CONFIG_SKILL_CHANCE_ORANGE) * 10;
}

bool ItemUsageValue::SpellGivesSkillUp(uint32 spellId, Player* bot)
{
    SkillLineAbilityMapBounds bounds = sSpellMgr->GetSkillLineAbilityMapBounds(spellId);

    for (SkillLineAbilityMap::const_iterator _spell_idx = bounds.first; _spell_idx != bounds.second; ++_spell_idx)
    {
        SkillLineAbilityEntry const* skill = _spell_idx->second;
        if (skill->SkillLine)
        {
            uint32 SkillValue = bot->GetPureSkillValue(skill->SkillLine);

            uint32 craft_skill_gain = sWorld->getIntConfig(CONFIG_SKILL_GAIN_CRAFTING);

            if (SkillGainChance(SkillValue, skill->TrivialSkillLineRankHigh,
                                (skill->TrivialSkillLineRankHigh + skill->TrivialSkillLineRankLow) / 2,
                                skill->TrivialSkillLineRankLow) > 0)
                return true;
        }
    }

    return false;
}

std::string const ItemUsageValue::GetConsumableType(ItemTemplate const* proto, bool hasMana)
{
    std::string const foodType = "";

    if ((proto->SubClass == ITEM_SUBCLASS_CONSUMABLE || proto->SubClass == ITEM_SUBCLASS_FOOD))
    {
        if (proto->Spells[0].SpellCategory == 11)
            return "food";
        else if (proto->Spells[0].SpellCategory == 59 && hasMana)
            return "drink";
    }

    if (proto->SubClass == ITEM_SUBCLASS_POTION || proto->SubClass == ITEM_SUBCLASS_FLASK)
    {
        for (int j = 0; j < MAX_ITEM_PROTO_SPELLS; j++)
        {
            SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(proto->Spells[j].SpellId);
            if (spellInfo)
                for (int i = 0; i < 3; i++)
                {
                    if (spellInfo->Effects[i].Effect == SPELL_EFFECT_ENERGIZE && hasMana)
                        return "mana potion";

                    if (spellInfo->Effects[i].Effect == SPELL_EFFECT_HEAL)
                        return "healing potion";
                }
        }
    }

    if (proto->SubClass == ITEM_SUBCLASS_BANDAGE)
    {
        return "bandage";
    }

    return "";
}

ItemUsage ItemUpgradeValue::Calculate()
{
    ParsedItemUsage parsed = ParseItemUsageQualifier(qualifier);
    uint32 itemId = parsed.itemId;
    uint32 randomPropertyId = parsed.randomPropertyId;
    if (!itemId)
        return ITEM_USAGE_NONE;

    ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemId);
    if (!proto)
        return ITEM_USAGE_NONE;

    ItemUsage equip = QueryItemUsageForEquip(proto, randomPropertyId);
    if (equip != ITEM_USAGE_NONE)
        return equip;

    if (proto->Class == ITEM_CLASS_PROJECTILE && bot->CanUseItem(proto) == EQUIP_ERR_OK)
        return QueryItemUsageForAmmo(proto);

    return ITEM_USAGE_NONE;
}

bool ItemUsageValue::IsLockboxItem(ItemTemplate const* proto)
{
    if (!proto)
        return false;

    // Primary, data-driven detection: lockboxes with a lock ID and Misc class.
    if (proto->LockID && proto->Class == ITEM_CLASS_MISC)
        return true;

    // English-only fallback on name (aligns with loot-roll heuristics).
    std::string const nameLower = ToLowerUtf8(proto->Name1);
    if (nameLower.empty())
        return false;

    return nameLower.find("lockbox") != std::string::npos;
}

namespace
{
// Profession helpers: true if the item is a recipe/pattern/book (ITEM_CLASS_RECIPE).
static inline bool IsRecipeItem(ItemTemplate const* proto) { return proto && proto->Class == ITEM_CLASS_RECIPE; }

// Special-case: Book of Glyph Mastery (can own several; do not downgrade NEED on duplicates).
static bool IsGlyphMasteryBook(ItemTemplate const* proto)
{
    if (!proto)
        return false;

    if (proto->Class != ITEM_CLASS_RECIPE || proto->SubClass != ITEM_SUBCLASS_BOOK)
        return false;

    constexpr uint32 SPELL_BOOK_OF_GLYPH_MASTERY = 64323;
    for (int i = 0; i < MAX_ITEM_PROTO_SPELLS; ++i)
    {
        if (proto->Spells[i].SpellId == SPELL_BOOK_OF_GLYPH_MASTERY)
            return true;
    }

    if (proto->RequiredSkill == SKILL_INSCRIPTION)
    {
        std::string n = ToLowerUtf8(proto->Name1);
        if (n.find("glyph mastery") != std::string::npos || n.find("book of glyph mastery") != std::string::npos)
            return true;
    }

    return false;
}

// Value object for collectible cosmetics (mounts/pets) used in loot rules.
struct CollectibleInfo
{
    bool isCosmetic = false;   // true if this is a cosmetic collectible (mount/pet)
    bool alreadyOwned = false; // true if the bot already knows/owns it in a meaningful way
};

static CollectibleInfo BuildCollectibleInfo(Player* bot, ItemTemplate const* proto)
{
    CollectibleInfo info;

    if (!bot || !proto)
        return info;

    if (proto->Class != ITEM_CLASS_MISC)
        return info;

#if defined(ITEM_SUBCLASS_MISC_MOUNT) || defined(ITEM_SUBCLASS_MISC_PET)
    bool const isMount =
#  if defined(ITEM_SUBCLASS_MISC_MOUNT)
        proto->SubClass == ITEM_SUBCLASS_MISC_MOUNT
#  else
        false
#  endif
        ;
    bool const isPet =
#  if defined(ITEM_SUBCLASS_MISC_PET)
        proto->SubClass == ITEM_SUBCLASS_MISC_PET
#  else
        false
#  endif
        ;
    if (!isMount && !isPet)
        return info;
#else
    if (proto->SubClass != 2 && proto->SubClass != 5)
        return info;
#endif

    info.isCosmetic = true;

    for (int i = 0; i < MAX_ITEM_PROTO_SPELLS; ++i)
    {
        uint32 const spellId = proto->Spells[i].SpellId;
        if (!spellId)
            continue;

        if (bot->HasSpell(spellId))
        {
            info.alreadyOwned = true;
            return info;
        }
    }

    if (bot->GetItemCount(proto->ItemId, true) > 0)
        info.alreadyOwned = true;

    return info;
}

struct RecipeInfo
{
    uint32 requiredSkill = 0;
    uint32 requiredRank = 0;
    uint32 botRank = 0;
    bool botHasProfession = false;
    bool known = false;
};

static RecipeInfo BuildRecipeInfo(Player* bot, ItemTemplate const* proto)
{
    RecipeInfo info;

    if (!bot || !IsRecipeItem(proto))
        return info;

    info.requiredSkill = GetRecipeSkill(proto);
    info.requiredRank = proto->RequiredSkillRank;

    if (!info.requiredSkill)
        return info;

    info.botHasProfession = bot->HasSkill(info.requiredSkill);
    if (info.botHasProfession)
        info.botRank = bot->GetSkillValue(info.requiredSkill);

    if (bot)
    {
        for (int i = 0; i < MAX_ITEM_PROTO_SPELLS; ++i)
        {
            uint32 teach = proto->Spells[i].SpellId;
            if (!teach)
                continue;

            SpellInfo const* si = sSpellMgr->GetSpellInfo(teach);
            if (!si)
                continue;

            for (int eff = 0; eff < MAX_SPELL_EFFECTS; ++eff)
            {
                if (si->Effects[eff].Effect != SPELL_EFFECT_LEARN_SPELL)
                    continue;

                uint32 learned = si->Effects[eff].TriggerSpell;
                if (learned && bot->HasSpell(learned))
                {
                    info.known = true;
                    break;
                }
            }

            if (info.known)
                break;
        }
    }

    return info;
}

static bool IsProfessionRecipeUsefulForBot(RecipeInfo const& recipe)
{
    if (!recipe.requiredSkill)
        return false;

    if (!recipe.botHasProfession)
        return false;

    if (!sPlayerbotAIConfig.recipesIgnoreSkillRank && recipe.requiredRank && recipe.botRank < recipe.requiredRank)
        return false;

    if (recipe.known)
        return false;

    return true;
}

static bool IsClassAllowedByItemTemplate(uint8 cls, ItemTemplate const* proto)
{
    if (!proto)
        return true;

    int32 const allowable = proto->AllowableClass;
    if (allowable <= 0)
        return true;

    if (!cls)
        return false;

    uint32 const classMask = static_cast<uint32>(allowable);
    uint32 const thisClassBit = 1u << (cls - 1u);
    return (classMask & thisClassBit) != 0;
}

static bool CanBotUseToken(ItemTemplate const* proto, Player* bot);
static bool RollUniqueCheck(ItemTemplate const* proto, Player* bot);

static inline bool IsLikelyDisenchantable(ItemTemplate const* proto)
{
    if (!proto)
        return false;

    if (proto->DisenchantID > 0)
        return true;

    if (proto->DisenchantID < 0)
        return false;

    if (proto->Class != ITEM_CLASS_ARMOR && proto->Class != ITEM_CLASS_WEAPON)
        return false;

    return proto->Quality >= ITEM_QUALITY_UNCOMMON && proto->Quality <= ITEM_QUALITY_EPIC;
}

static int8 TokenSlotFromName(ItemTemplate const* proto)
{
    if (!proto)
        return -1;
    std::string n = std::string(proto->Name1);
    std::transform(n.begin(), n.end(), n.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    if (n.find("helm") != std::string::npos || n.find("head") != std::string::npos)
        return INVTYPE_HEAD;

    if (n.find("shoulder") != std::string::npos || n.find("mantle") != std::string::npos ||
        n.find("spauld") != std::string::npos)
        return INVTYPE_SHOULDERS;

    if (n.find("chest") != std::string::npos || n.find("tunic") != std::string::npos ||
        n.find("robe") != std::string::npos || n.find("breastplate") != std::string::npos ||
        n.find("chestguard") != std::string::npos)
        return INVTYPE_CHEST;

    if (n.find("glove") != std::string::npos || n.find("handguard") != std::string::npos ||
        n.find("gauntlet") != std::string::npos)
        return INVTYPE_HANDS;

    if (n.find("leg") != std::string::npos || n.find("pant") != std::string::npos ||
        n.find("trouser") != std::string::npos)
        return INVTYPE_LEGS;

    return -1;
}

static std::array<uint32, 6> const& GetSanctificationTokenIds()
{
    static std::array<uint32, 6> const sanctificationTokenIds = {
        52025,
        52026,
        52027,
        52028,
        52029,
        52030
    };

    return sanctificationTokenIds;
}

static bool IsSanctificationToken(ItemTemplate const* proto)
{
    if (!proto)
        return false;

    for (uint32 const entry : GetSanctificationTokenIds())
    {
        if (proto->ItemId == entry)
            return true;
    }

    return false;
}

static uint32 GetOwnedSanctificationTokenCount(Player* bot)
{
    if (!bot)
        return 0;

    uint32 total = 0;
    for (uint32 const entry : GetSanctificationTokenIds())
        total += bot->GetItemCount(entry, true);

    return total;
}

static uint8 EquipmentSlotByInvTypeSafe(uint8 invType)
{
    switch (invType)
    {
        case INVTYPE_HEAD:
            return EQUIPMENT_SLOT_HEAD;
        case INVTYPE_SHOULDERS:
            return EQUIPMENT_SLOT_SHOULDERS;
        case INVTYPE_CHEST:
        case INVTYPE_ROBE:
            return EQUIPMENT_SLOT_CHEST;
        case INVTYPE_HANDS:
            return EQUIPMENT_SLOT_HANDS;
        case INVTYPE_LEGS:
            return EQUIPMENT_SLOT_LEGS;
        default:
            return EQUIPMENT_SLOT_END;
    }
}

static bool IsTokenLikelyUpgrade(ItemTemplate const* token, uint8 invTypeSlot, Player* bot)
{
    if (!token || !bot)
        return false;
    uint8 eq = EquipmentSlotByInvTypeSafe(invTypeSlot);
    if (eq >= EQUIPMENT_SLOT_END)
        return true;

    Item* oldItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, eq);
    if (!oldItem)
        return true;

    ItemTemplate const* oldProto = oldItem->GetTemplate();
    if (!oldProto)
        return true;

    float margin = sPlayerbotAIConfig.tokenILevelMargin;
    return (float)token->ItemLevel >= (float)oldProto->ItemLevel + margin;
}

struct TokenInfo
{
    bool isToken = false;
    bool classCanUse = false;
    int8 invTypeSlot = -1;
    bool likelyUpgrade = false;
};

static TokenInfo BuildTokenInfo(ItemTemplate const* proto, Player* bot)
{
    TokenInfo info;

    if (!proto || !bot)
        return info;

    info.isToken = (proto->Class == ITEM_CLASS_MISC &&
                    proto->SubClass == ITEM_SUBCLASS_JUNK &&
                    proto->Quality == ITEM_QUALITY_EPIC) ||
                   IsSanctificationToken(proto);
    if (!info.isToken)
        return info;

    info.classCanUse = CanBotUseToken(proto, bot);
    info.invTypeSlot = TokenSlotFromName(proto);

    if (info.classCanUse && info.invTypeSlot >= 0)
        info.likelyUpgrade = IsTokenLikelyUpgrade(proto, static_cast<uint8>(info.invTypeSlot), bot);

    return info;
}

static bool TryTokenRollVote(ItemTemplate const* proto, Player* bot, RollVote& outVote)
{
    TokenInfo const token = BuildTokenInfo(proto, bot);
    constexpr uint32 SANCTIFICATION_TOKEN_MAX_COUNT = 5u;

    // Not a token → let other rules decide.
    if (!token.isToken)
        return false;

    if (token.classCanUse)
    {
        if (token.invTypeSlot >= 0)
        {
            // Known slot: NEED only if it looks like an upgrade, otherwise GREED.
            outVote = token.likelyUpgrade ? NEED : GREED;
        }
        else
        {
            // Unknown slot (e.g. T10 sanctification tokens).
            if (IsSanctificationToken(proto) && sPlayerbotAIConfig.sanctificationTokenRollMode == 1u)
            {
                uint32 const ownedTokens = GetOwnedSanctificationTokenCount(bot);
                outVote = (ownedTokens < SANCTIFICATION_TOKEN_MAX_COUNT) ? NEED : GREED;
            }
            else
                outVote = GREED;
        }
    }
    else
    {
        // Not eligible, so GREED.
        outVote = GREED;
    }

    return true;
}

static RollVote ApplyDisenchantPreference(RollVote currentVote, ItemTemplate const* proto, ItemUsage usage,
                                          Group* group, Player* bot)
{
    bool const isDeCandidate = IsLikelyDisenchantable(proto);
    bool const hasEnchantSkill = bot && bot->HasSkill(SKILL_ENCHANTING);

    uint8 const deMode = sPlayerbotAIConfig.deButtonMode;
    // Mode 0 = no DE button; 1 = enchanters only; 2 = all bots can DE.
    bool const deAllowedForBot = (deMode == 2u) || (deMode == 1u && hasEnchantSkill);

    if (currentVote != NEED &&
        deAllowedForBot &&
        group &&
        (group->GetLootMethod() == NEED_BEFORE_GREED || group->GetLootMethod() == GROUP_LOOT) &&
        isDeCandidate &&
        usage == ITEM_USAGE_DISENCHANT)
        return DISENCHANT;

    return currentVote;
}

static RollVote FinalizeRollVote(RollVote vote, ItemTemplate const* proto, ItemUsage usage, Group* group, Player* bot)
{
    vote = ApplyDisenchantPreference(vote, proto, usage, group, bot);

    if (sPlayerbotAIConfig.lootRollLevel == 0)
    {
        return PASS;
    }

    if (sPlayerbotAIConfig.lootRollLevel == 1)
    {
        if (vote == NEED)
            vote = RollUniqueCheck(proto, bot) ? PASS : GREED;

        else if (vote == GREED)
            vote = PASS;
    }

    return vote;
}

static RollVote CalculateBaseRollVote(Player* bot, ItemTemplate const* proto, int32 randomProperty, ItemUsage usage)
{
    // Player mimic: upgrade => NEED; useful => GREED; otherwise => PASS
    RollVote vote = PASS;

    CollectibleInfo const collectible = BuildCollectibleInfo(bot, proto);

    bool const isCollectibleCosmetic = collectible.isCosmetic;
    bool const alreadyHasCollectible = collectible.alreadyOwned;

    if (isCollectibleCosmetic)
        vote = alreadyHasCollectible ? GREED : NEED;

    bool recipeChecked = false;
    bool recipeNeed = false;
    bool recipeUseful = false;
    bool recipeKnown = false;

    // Professions: NEED on useful recipes/patterns/books when enabled.
    if (sPlayerbotAIConfig.needOnProfessionRecipes && IsRecipeItem(proto))
    {
        RecipeInfo const recipe = BuildRecipeInfo(bot, proto);

        recipeChecked = true;
        recipeKnown = recipe.known;

        recipeUseful = IsProfessionRecipeUsefulForBot(recipe);
        if (recipeUseful)
        {
            vote = NEED;
            recipeNeed = true;
        }
        else
            vote = GREED;  // recipe not for the bot -> GREED
    }

    // Do not overwrite the choice if already decided by recipe or cosmetic logic.
    if (!recipeChecked && !isCollectibleCosmetic)
    {
        switch (usage)
        {
            case ITEM_USAGE_EQUIP:
            case ITEM_USAGE_REPLACE:
                vote = NEED;
                break;
            case ITEM_USAGE_BAD_EQUIP:
            case ITEM_USAGE_GUILD_TASK:
            case ITEM_USAGE_SKILL:
            case ITEM_USAGE_USE:
            case ITEM_USAGE_DISENCHANT:
            case ITEM_USAGE_AH:
            case ITEM_USAGE_VENDOR:
            case ITEM_USAGE_KEEP:
            case ITEM_USAGE_AMMO:
                vote = GREED;
                break;
            default:
                vote = PASS;
                break;
        }
    }

    if (vote == NEED && IsJewelryOrCloak(proto) && !IsFallbackNeedReasonableForSpec(bot, proto))
        vote = GREED;

    // Lockboxes: if the item is a lockbox and the bot is a Rogue with Lockpicking, prefer NEED (ignored by BoE/BoU).
    const SpecTraits traits = GetSpecTraits(bot);
    const bool isLockbox = ItemUsageValue::IsLockboxItem(proto);
    if (isLockbox && traits.isRogue && bot->HasSkill(SKILL_LOCKPICKING))
        vote = NEED;

    // BoE/BoU rule: by default, avoid NEED on Bind-on-Equip / Bind-on-Use (raid etiquette)
    // BoE/BoU etiquette: avoid NEED on BoE/BoU, except useful profession recipes.
    constexpr uint32 BIND_WHEN_EQUIPPED = 2;  // BoE
    constexpr uint32 BIND_WHEN_USE = 3;       // BoU

    if (vote == NEED && !recipeNeed && !isLockbox && !isCollectibleCosmetic &&
        proto->Bonding == BIND_WHEN_EQUIPPED &&
        !sPlayerbotAIConfig.allowBoENeedIfUpgrade)

        vote = GREED;

    if (vote == NEED && !recipeNeed && !isLockbox && !isCollectibleCosmetic &&
        proto->Bonding == BIND_WHEN_USE &&
        !sPlayerbotAIConfig.allowBoUNeedIfUpgrade)

        vote = GREED;

    // Non-unique soft rule: NEED -> GREED on duplicates, except Book of Glyph Mastery.
    if (vote == NEED)
    {
        if (!IsGlyphMasteryBook(proto))
        {
            // includeBank=true to catch banked duplicates as well.
            if (bot->GetItemCount(proto->ItemId, true) > 0)
                vote = GREED;
        }
    }

    // Unique-equip: never NEED a duplicate (already equipped/owned)
    if (vote == NEED && RollUniqueCheck(proto, bot))
        vote = PASS;

    // Final decision (with allow/deny from loot strategy).
    RollVote finalVote = StoreLootAction::IsLootAllowed(proto->ItemId, GET_PLAYERBOT_AI(bot)) ? vote : PASS;

    return finalVote;
}

static bool CanBotUseToken(ItemTemplate const* proto, Player* bot)
{
    if (!proto || !bot)
        return false;

    return IsClassAllowedByItemTemplate(bot->getClass(), proto);
}

static bool RollUniqueCheck(ItemTemplate const* proto, Player* bot)
{
    if (!proto || !bot)
        return false;

    uint32 totalItemCount = bot->GetItemCount(proto->ItemId, true);
    uint32 bagItemCount = bot->GetItemCount(proto->ItemId, false);

    bool isEquipped = (totalItemCount > bagItemCount);
    if (isEquipped && proto->HasFlag(ITEM_FLAG_UNIQUE_EQUIPPABLE))
        return true;

    if (proto->HasFlag(ITEM_FLAG_UNIQUE_EQUIPPABLE) && bagItemCount > 0)
        return true;

    return false;
}
} // namespace

char const* RollVoteText(RollVote v)
{
    switch (v)
    {
        case NEED:
            return "NEED";
        case GREED:
            return "GREED";
        case PASS:
            return "PASS";
        case DISENCHANT:
            return "DISENCHANT";
        default:
            return "UNKNOWN";
    }
}

RollVote CalculateLootRollVote(Player* bot, ItemTemplate const* proto, int32 randomProperty, ItemUsage usage,
                               Group* group)
{
    if (!bot || !proto)
        return PASS;

    usage = AdjustUsageForOffspec(bot, proto, randomProperty, usage);

    RollVote vote = PASS;
    if (!TryTokenRollVote(proto, bot, vote))
    {
        // Let CalculateBaseRollVote decide using loot-aware item usage.
        vote = CalculateBaseRollVote(bot, proto, randomProperty, usage);
    }

    return FinalizeRollVote(vote, proto, usage, group, bot);
}