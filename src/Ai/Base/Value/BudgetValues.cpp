/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "BudgetValues.h"

#include "Playerbots.h"

uint32_t MaxGearRepairCostValue::Calculate()
{
    uint32_t totalCost = 0;

    for (uint8_t i = EQUIPMENT_SLOT_START; i < INVENTORY_SLOT_ITEM_END; ++i)
    {
        const uint16_t pos = ((INVENTORY_SLOT_BAG_0 << 8) | i);
        const Item* const item = this->bot->GetItemByPos(pos);

        if (item == nullptr)
        {
            continue;
        }

        const uint32_t maxDurability = item->GetUInt32Value(ITEM_FIELD_MAXDURABILITY);

        if (maxDurability == 0)
        {
            continue;
        }

        const uint32_t curDurability = item->GetUInt32Value(ITEM_FIELD_DURABILITY);

        // Only count items equiped or already damanged.
        if (i >= EQUIPMENT_SLOT_END && curDurability >= maxDurability)
        {
            continue;
        }

        const ItemTemplate* const ditemProto = item->GetTemplate();

        if (ditemProto == nullptr)
        {
            continue;
        }

        const DurabilityCostsEntry* const dcost = sDurabilityCostsStore.LookupEntry(ditemProto->ItemLevel);

        if (dcost == nullptr)
        {
            continue;
        }

        const uint32_t dQualitymodEntryId = (ditemProto->Quality + 1) * 2;
        const DurabilityQualityEntry* const dQualitymodEntry = sDurabilityQualityStore.LookupEntry(dQualitymodEntryId);

        if (dQualitymodEntry == nullptr)
        {
            continue;
        }

        const uint8_t index = ItemSubClassToDurabilityMultiplierId(ditemProto->Class, ditemProto->SubClass);
        const uint32_t dmultiplier = dcost->multiplier[index];
        const uint32_t costs = uint32(maxDurability * dmultiplier * double(dQualitymodEntry->quality_mod));

        totalCost += costs;
    }

    return totalCost;
}

uint32_t RepairCostValue::Calculate()
{
    uint32_t totalCost = 0;

    for (uint8_t i = EQUIPMENT_SLOT_START; i < INVENTORY_SLOT_ITEM_END; ++i)
    {
        const uint16_t pos = ((INVENTORY_SLOT_BAG_0 << 8) | i);
        const Item* const item = this->bot->GetItemByPos(pos);

        if (item == nullptr)
        {
            continue;
        }

        const uint32_t maxDurability = item->GetUInt32Value(ITEM_FIELD_MAXDURABILITY);

        if (maxDurability == 0)
        {
            continue;
        }

        const uint32_t curDurability = item->GetUInt32Value(ITEM_FIELD_DURABILITY);
        const uint32_t lostDurability = maxDurability - curDurability;

        if (lostDurability == 0)
        {
            continue;
        }

        const ItemTemplate* const ditemProto = item->GetTemplate();

        if (ditemProto == nullptr)
        {
            continue;
        }

        const DurabilityCostsEntry* const dcost = sDurabilityCostsStore.LookupEntry(ditemProto->ItemLevel);

        if (dcost == nullptr)
        {
            continue;
        }

        const uint32 dQualitymodEntryId = (ditemProto->Quality + 1) * 2;
        const DurabilityQualityEntry* const dQualitymodEntry = sDurabilityQualityStore.LookupEntry(dQualitymodEntryId);

        if (dQualitymodEntry == nullptr)
        {
            continue;
        }

        const uint8_t index = ItemSubClassToDurabilityMultiplierId(ditemProto->Class, ditemProto->SubClass);
        const uint32_t dmultiplier = dcost->multiplier[index];
        const uint32_t costs = uint32_t(lostDurability * dmultiplier * double(dQualitymodEntry->quality_mod));

        totalCost += costs;
    }

    return totalCost;
}

uint32_t TrainCostValue::Calculate()
{
    uint32_t totalCost{};
    std::unordered_set<uint32_t> spells{};
    ObjectMgr* const objectMgr = ObjectMgr::instance();

    if (objectMgr == nullptr)
    {
        return 0;
    }

    const CreatureTemplateContainer* const creatureTemplateContainer = objectMgr->GetCreatureTemplates();

    if (creatureTemplateContainer == nullptr)
    {
        return 0;
    }

    for (const CreatureTemplateContainer::value_type& item : *creatureTemplateContainer)
    {
        if (!(item.second.npcflag & UNIT_NPC_FLAG_TRAINER))
        {
            continue;
        }

        Trainer::Trainer* const trainer = objectMgr->GetTrainer(item.first);

        if (trainer == nullptr)
        {
            continue;
        }

        if (trainer->GetTrainerType() != Trainer::Type::Class || !trainer->IsTrainerValidForPlayer(this->bot))
        {
            continue;
        }

        for (const Trainer::Spell& spell : trainer->GetSpells())
        {
            if (!trainer->CanTeachSpell(this->bot, &spell))
            {
                continue;
            }

            if (spells.find(spell.SpellId) != spells.end())
            {
                continue;
            }

            totalCost += spell.MoneyCost;
            spells.insert(spell.SpellId);
        }
    }

    return totalCost;
}

//@ TODO: refactor in a separate file and split in methods.
uint32_t MoneyNeededForValue::Calculate()
{
    const NeedMoneyFor needMoneyFor = needMoneyForFromIntegerLikeString(this->getQualifier());
    PlayerbotAI* const botAI = PlayerbotsMgr::instance().GetPlayerbotAI(bot);
    AiObjectContext* const context = botAI->GetAiObjectContext();
    const uint8_t level = bot->GetLevel();

    switch (needMoneyFor)
    {
        case NeedMoneyFor::none:
        {
            return 0;
        }

        case NeedMoneyFor::repair:
        {
            Value<uint32_t>* const repairCostValue = context->GetValue<uint32_t>("max repair cost");

            if (repairCostValue == nullptr)
            {
                return 0;
            }

            return repairCostValue->Get();
        }

        case NeedMoneyFor::ammo:
        {
            if (this->bot->getClass() != CLASS_HUNTER)
            {
                return 0;
            }

            // Or level^3 (1s @ lvl10, 30s @ lvl30, 2g @ lvl60, 5g @ lvl80): Todo replace (Should be best ammo buyable x 8 stacks cost)
            // (should be best ammo buyable x 8 stacks cost)
            return level * level * level / 10;
        }

        case NeedMoneyFor::spells:
        {
            Value<uint32_t>* const trainCostValue = context->GetValue<uint32_t>("train cost");

            if (trainCostValue == nullptr)
            {
                return 0;
            }

            return trainCostValue->Get();
        }

        case NeedMoneyFor::travel:
        {
            if (this->bot->isTaxiCheater())
            {
                return 0;
            }

            // 15s for traveling half a continent. Todo: Add better calculation (Should be ???)
            return 1500;
        }

        case NeedMoneyFor::gear:
        {
            // Todo replace (Should be ~total cost of all >green gear equiped)
            // Or level^3 (10s @ lvl10, 3g @ lvl30, 20g @ lvl60, 50g @ lvl80):
            return level * level * level;
        }

        case NeedMoneyFor::consumables:
        {
            // Or level^3 (1s @ lvl10, 30s @ lvl30, 2g @ lvl60, 5g @ lvl80): Todo
            // replace (Should be best food/drink x 2 stacks cost)
            return (level * level * level) / 10;
        }

        case NeedMoneyFor::guild:
        {
            if (!botAI->HasStrategy("guild", BOT_STATE_NON_COMBAT))
            {
                return 0;
            }

            if (this->bot->GetGuildId())
            {
                Value<uint32_t>* const tabardCountValue = this->context->GetValue<uint32_t>("item count", this->chat->FormatQItem(5976));

                if (tabardCountValue == nullptr || tabardCountValue->Get() == 0)
                {
                    // 1g (tabard)
                    return 10000;
                }

                return 0;
            }

            Value<uint32_t>* const guildCharterCountValue = this->context->GetValue<uint32_t>("item count", this->chat->FormatQItem(5863));

            if (guildCharterCountValue == nullptr || guildCharterCountValue->Get() == 0)
            {
                // 10s (guild charter)
                return 1000;
            }
        }

        case NeedMoneyFor::tradeskill:
        {
            // Or level^3 (10s @ lvl10, 3g @ lvl30, 20g @ lvl60, 50g @ lvl80): Todo replace
            // (Should be buyable reagents that combined allow crafting of usefull items)
            return (level * level * level);
        }

        default:
        {
            return 0;
        }
    }
};

uint32 TotalMoneyNeededForValue::Calculate()
{
    NeedMoneyFor needMoneyFor = NeedMoneyFor(stoi(getQualifier()));

    uint32 moneyWanted = AI_VALUE2(uint32, "money needed for", (uint32)needMoneyFor);

    auto needPtr = std::find(saveMoneyFor.begin(), saveMoneyFor.end(), needMoneyFor);

    while (needPtr != saveMoneyFor.begin())
    {
        needPtr--;

        NeedMoneyFor alsoNeed = *needPtr;

        moneyWanted = moneyWanted + AI_VALUE2(uint32, "money needed for", (uint32)alsoNeed);
    }

    return moneyWanted;
}

uint32 FreeMoneyForValue::Calculate()
{
    uint32 money = bot->GetMoney();

    if (botAI->HasCheat(BotCheatMask::gold))
        return 10000000;

    if (botAI->HasActivePlayerMaster())
        return money;

    uint32 savedMoney = AI_VALUE2(uint32, "total money needed for", getQualifier()) -
                        AI_VALUE2(uint32, "money needed for", getQualifier());

    if (savedMoney > money)
        return 0;

    return money - savedMoney;
};

bool ShouldGetMoneyValue::Calculate() { return !AI_VALUE2(uint32, "free money for", (uint32)NeedMoneyFor::anything); };
