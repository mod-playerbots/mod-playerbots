/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "mocks/MockBotServices.h"

using ::testing::Return;
using ::testing::_;

/**
 * @brief Tests for BotItemService
 *
 * These tests verify the item service functionality
 * for bot inventory and item management operations.
 */
class ItemServiceTest : public ::testing::Test
{
protected:
    MockItemService mockItemService;
};

TEST_F(ItemServiceTest, CanMockFindPoison)
{
    EXPECT_CALL(mockItemService, FindPoison())
        .WillOnce(Return(nullptr));

    EXPECT_EQ(nullptr, mockItemService.FindPoison());
}

TEST_F(ItemServiceTest, CanMockFindAmmo)
{
    EXPECT_CALL(mockItemService, FindAmmo())
        .WillOnce(Return(nullptr));

    EXPECT_EQ(nullptr, mockItemService.FindAmmo());
}

TEST_F(ItemServiceTest, CanMockFindBandage)
{
    EXPECT_CALL(mockItemService, FindBandage())
        .WillOnce(Return(nullptr));

    EXPECT_EQ(nullptr, mockItemService.FindBandage());
}

TEST_F(ItemServiceTest, CanMockFindOpenableItem)
{
    EXPECT_CALL(mockItemService, FindOpenableItem())
        .WillOnce(Return(nullptr));

    EXPECT_EQ(nullptr, mockItemService.FindOpenableItem());
}

TEST_F(ItemServiceTest, CanMockFindLockedItem)
{
    EXPECT_CALL(mockItemService, FindLockedItem())
        .WillOnce(Return(nullptr));

    EXPECT_EQ(nullptr, mockItemService.FindLockedItem());
}

TEST_F(ItemServiceTest, CanMockFindConsumable)
{
    EXPECT_CALL(mockItemService, FindConsumable(12345))
        .WillOnce(Return(nullptr));

    EXPECT_EQ(nullptr, mockItemService.FindConsumable(12345));
}

TEST_F(ItemServiceTest, CanMockFindStoneFor)
{
    EXPECT_CALL(mockItemService, FindStoneFor(_))
        .WillOnce(Return(nullptr));

    EXPECT_EQ(nullptr, mockItemService.FindStoneFor(nullptr));
}

TEST_F(ItemServiceTest, CanMockFindOilFor)
{
    EXPECT_CALL(mockItemService, FindOilFor(_))
        .WillOnce(Return(nullptr));

    EXPECT_EQ(nullptr, mockItemService.FindOilFor(nullptr));
}

TEST_F(ItemServiceTest, CanMockGetInventoryAndEquippedItems)
{
    std::vector<Item*> emptyItems;
    EXPECT_CALL(mockItemService, GetInventoryAndEquippedItems())
        .WillOnce(Return(emptyItems));

    auto result = mockItemService.GetInventoryAndEquippedItems();
    EXPECT_TRUE(result.empty());
}

TEST_F(ItemServiceTest, CanMockGetInventoryItems)
{
    std::vector<Item*> emptyItems;
    EXPECT_CALL(mockItemService, GetInventoryItems())
        .WillOnce(Return(emptyItems));

    auto result = mockItemService.GetInventoryItems();
    EXPECT_TRUE(result.empty());
}

TEST_F(ItemServiceTest, CanMockGetInventoryItemsCountWithId)
{
    EXPECT_CALL(mockItemService, GetInventoryItemsCountWithId(12345))
        .WillOnce(Return(5));

    EXPECT_EQ(5u, mockItemService.GetInventoryItemsCountWithId(12345));
}

TEST_F(ItemServiceTest, CanMockHasItemInInventory)
{
    EXPECT_CALL(mockItemService, HasItemInInventory(12345))
        .WillOnce(Return(true));

    EXPECT_TRUE(mockItemService.HasItemInInventory(12345));
}

TEST_F(ItemServiceTest, CanMockGetEquipGearScore)
{
    EXPECT_CALL(mockItemService, GetEquipGearScore(_))
        .WillOnce(Return(200));

    EXPECT_EQ(200u, mockItemService.GetEquipGearScore(nullptr));
}

TEST_F(ItemServiceTest, CanMockGetCurrentQuestsRequiringItemId)
{
    std::vector<std::pair<Quest const*, uint32>> emptyQuests;
    EXPECT_CALL(mockItemService, GetCurrentQuestsRequiringItemId(12345))
        .WillOnce(Return(emptyQuests));

    auto result = mockItemService.GetCurrentQuestsRequiringItemId(12345);
    EXPECT_TRUE(result.empty());
}

/**
 * @brief Tests for item behavior patterns
 */
class ItemBehaviorPatternTest : public ::testing::Test
{
protected:
    MockItemService mockItemService;

    bool HasWeaponEnhancement(MockItemService& itemService, Item* weapon)
    {
        return itemService.FindStoneFor(weapon) != nullptr ||
               itemService.FindOilFor(weapon) != nullptr;
    }

    bool HasSufficientAmmo(MockItemService& itemService, uint32 minCount)
    {
        if (Item* ammo = itemService.FindAmmo())
        {
            return itemService.GetInventoryItemsCountWithId(ammo->GetEntry()) >= minCount;
        }
        return false;
    }

    bool NeedsHealing(MockItemService& itemService)
    {
        return itemService.FindBandage() != nullptr;
    }
};

TEST_F(ItemBehaviorPatternTest, CheckNoWeaponEnhancement)
{
    EXPECT_CALL(mockItemService, FindStoneFor(_))
        .WillOnce(Return(nullptr));
    EXPECT_CALL(mockItemService, FindOilFor(_))
        .WillOnce(Return(nullptr));

    EXPECT_FALSE(HasWeaponEnhancement(mockItemService, nullptr));
}

TEST_F(ItemBehaviorPatternTest, CheckNoAmmo)
{
    EXPECT_CALL(mockItemService, FindAmmo())
        .WillOnce(Return(nullptr));

    EXPECT_FALSE(HasSufficientAmmo(mockItemService, 100));
}

TEST_F(ItemBehaviorPatternTest, CheckHasBandage)
{
    EXPECT_CALL(mockItemService, FindBandage())
        .WillOnce(Return(nullptr));

    EXPECT_FALSE(NeedsHealing(mockItemService));
}

/**
 * @brief Tests for inventory query patterns
 */
class ItemInventoryQueryTest : public ::testing::Test
{
protected:
    MockItemService mockItemService;
};

TEST_F(ItemInventoryQueryTest, CanCheckItemExists)
{
    EXPECT_CALL(mockItemService, HasItemInInventory(1234))
        .WillOnce(Return(true));
    EXPECT_CALL(mockItemService, HasItemInInventory(5678))
        .WillOnce(Return(false));

    EXPECT_TRUE(mockItemService.HasItemInInventory(1234));
    EXPECT_FALSE(mockItemService.HasItemInInventory(5678));
}

TEST_F(ItemInventoryQueryTest, CanCountItems)
{
    EXPECT_CALL(mockItemService, GetInventoryItemsCountWithId(1234))
        .WillOnce(Return(20));
    EXPECT_CALL(mockItemService, GetInventoryItemsCountWithId(5678))
        .WillOnce(Return(0));

    EXPECT_EQ(20u, mockItemService.GetInventoryItemsCountWithId(1234));
    EXPECT_EQ(0u, mockItemService.GetInventoryItemsCountWithId(5678));
}

TEST_F(ItemInventoryQueryTest, CanGetGearScore)
{
    EXPECT_CALL(mockItemService, GetEquipGearScore(nullptr))
        .WillOnce(Return(150));

    EXPECT_EQ(150u, mockItemService.GetEquipGearScore(nullptr));
}
