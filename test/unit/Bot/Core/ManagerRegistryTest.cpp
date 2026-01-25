/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "mocks/MockManagers.h"
#include "Bot/Core/ManagerRegistry.h"

using ::testing::Return;
using ::testing::_;

/**
 * @brief Tests for the ManagerRegistry
 *
 * These tests demonstrate how to use the manager registry
 * with mock implementations for testing.
 */
class ManagerRegistryTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Create a fresh registry for each test
        registry_ = std::make_unique<ManagerRegistry>();
    }

    void TearDown() override
    {
        registry_.reset();
    }

    std::unique_ptr<ManagerRegistry> registry_;
};

TEST_F(ManagerRegistryTest, InitiallyEmpty)
{
    EXPECT_FALSE(registry_->HasTravelManager());
    EXPECT_FALSE(registry_->HasRandomBotManager());
    EXPECT_FALSE(registry_->HasBotRepository());
    EXPECT_FALSE(registry_->IsInitialized());
}

TEST_F(ManagerRegistryTest, CanRegisterTravelManager)
{
    auto mockTravel = std::make_shared<MockTravelManager>();
    registry_->SetTravelManager(mockTravel);

    EXPECT_TRUE(registry_->HasTravelManager());
    EXPECT_FALSE(registry_->IsInitialized());  // Still missing other managers
}

TEST_F(ManagerRegistryTest, CanRegisterRandomBotManager)
{
    auto mockRandomBot = std::make_shared<MockRandomBotManager>();
    registry_->SetRandomBotManager(mockRandomBot);

    EXPECT_TRUE(registry_->HasRandomBotManager());
    EXPECT_FALSE(registry_->IsInitialized());
}

TEST_F(ManagerRegistryTest, CanRegisterBotRepository)
{
    auto mockRepo = std::make_shared<MockBotRepository>();
    registry_->SetBotRepository(mockRepo);

    EXPECT_TRUE(registry_->HasBotRepository());
    EXPECT_FALSE(registry_->IsInitialized());
}

TEST_F(ManagerRegistryTest, IsInitializedWhenAllManagersSet)
{
    auto mockTravel = std::make_shared<MockTravelManager>();
    auto mockRandomBot = std::make_shared<MockRandomBotManager>();
    auto mockRepo = std::make_shared<MockBotRepository>();

    registry_->SetTravelManager(mockTravel);
    registry_->SetRandomBotManager(mockRandomBot);
    registry_->SetBotRepository(mockRepo);

    EXPECT_TRUE(registry_->IsInitialized());
}

TEST_F(ManagerRegistryTest, CanAccessTravelManagerThroughInterface)
{
    auto mockTravel = std::make_shared<MockTravelManager>();
    registry_->SetTravelManager(mockTravel);

    EXPECT_CALL(*mockTravel, RandomTeleportForLevel(_)).Times(1);

    registry_->GetTravelManager().RandomTeleportForLevel(nullptr);
}

TEST_F(ManagerRegistryTest, CanAccessRandomBotManagerThroughInterface)
{
    auto mockRandomBot = std::make_shared<MockRandomBotManager>();
    registry_->SetRandomBotManager(mockRandomBot);

    EXPECT_CALL(*mockRandomBot, GetActiveBotCount()).WillOnce(Return(50u));

    EXPECT_EQ(50u, registry_->GetRandomBotManager().GetActiveBotCount());
}

TEST_F(ManagerRegistryTest, CanAccessBotRepositoryThroughInterface)
{
    auto mockRepo = std::make_shared<MockBotRepository>();
    registry_->SetBotRepository(mockRepo);

    EXPECT_CALL(*mockRepo, HasSavedData(123)).WillOnce(Return(true));

    EXPECT_TRUE(registry_->GetBotRepository().HasSavedData(123));
}

TEST_F(ManagerRegistryTest, TemplateAccessors)
{
    auto mockTravel = std::make_shared<MockTravelManager>();
    auto mockRandomBot = std::make_shared<MockRandomBotManager>();
    auto mockRepo = std::make_shared<MockBotRepository>();

    registry_->Register<ITravelManager>(mockTravel);
    registry_->Register<IRandomBotManager>(mockRandomBot);
    registry_->Register<IBotRepository>(mockRepo);

    EXPECT_TRUE(registry_->IsInitialized());

    // Access through template
    EXPECT_CALL(*mockRandomBot, GetMaxAllowedBotCount()).WillOnce(Return(100u));
    EXPECT_EQ(100u, registry_->Get<IRandomBotManager>().GetMaxAllowedBotCount());
}

/**
 * @brief Integration-style test demonstrating mock manager usage
 */
class ManagerIntegrationTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        mockTravel_ = std::make_shared<MockTravelManager>();
        mockRandomBot_ = std::make_shared<MockRandomBotManager>();
        mockRepo_ = std::make_shared<MockBotRepository>();

        registry_.SetTravelManager(mockTravel_);
        registry_.SetRandomBotManager(mockRandomBot_);
        registry_.SetBotRepository(mockRepo_);
    }

    ManagerRegistry registry_;
    std::shared_ptr<MockTravelManager> mockTravel_;
    std::shared_ptr<MockRandomBotManager> mockRandomBot_;
    std::shared_ptr<MockBotRepository> mockRepo_;
};

TEST_F(ManagerIntegrationTest, SimulateBotTeleportFlow)
{
    // Simulate: Check if bot is random, then teleport
    EXPECT_CALL(*mockRandomBot_, IsRandomBot(testing::A<Player*>())).WillOnce(Return(true));
    EXPECT_CALL(*mockTravel_, RandomTeleportForLevel(_)).Times(1);

    // Simulated flow
    if (registry_.GetRandomBotManager().IsRandomBot(static_cast<Player*>(nullptr)))
    {
        registry_.GetTravelManager().RandomTeleportForLevel(nullptr);
    }
}

TEST_F(ManagerIntegrationTest, SimulateBotPersistence)
{
    // Simulate: Save bot data if bot exists
    EXPECT_CALL(*mockRandomBot_, GetActiveBotCount()).WillOnce(Return(1u));
    EXPECT_CALL(*mockRepo_, Save(_)).Times(1);

    // Simulated flow
    if (registry_.GetRandomBotManager().GetActiveBotCount() > 0)
    {
        registry_.GetBotRepository().Save(nullptr);
    }
}

TEST_F(ManagerIntegrationTest, SimulateBotActivityScaling)
{
    EXPECT_CALL(*mockRandomBot_, GetActivityPercentage()).WillOnce(Return(75.0f));
    EXPECT_CALL(*mockRandomBot_, GetMaxAllowedBotCount()).WillOnce(Return(100u));

    float activity = registry_.GetRandomBotManager().GetActivityPercentage();
    uint32 maxBots = registry_.GetRandomBotManager().GetMaxAllowedBotCount();

    // Calculated expected active bots
    uint32 expectedActive = static_cast<uint32>(maxBots * (activity / 100.0f));
    EXPECT_EQ(75u, expectedActive);
}

/**
 * @brief Tests demonstrating the mock injection pattern for testing code
 * that depends on ManagerRegistry
 */
TEST_F(ManagerRegistryTest, CanInjectMockRandomBotManager)
{
    // This test demonstrates the pattern from the migration plan
    auto mockMgr = std::make_shared<MockRandomBotManager>();
    registry_->SetRandomBotManager(mockMgr);

    EXPECT_CALL(*mockMgr, IsRandomBot(testing::A<Player*>())).WillOnce(Return(true));

    EXPECT_TRUE(registry_->GetRandomBotManager().IsRandomBot(nullptr));
}

TEST_F(ManagerRegistryTest, CanInjectMockTravelManager)
{
    auto mockMgr = std::make_shared<MockTravelManager>();
    registry_->SetTravelManager(mockMgr);

    EXPECT_CALL(*mockMgr, ClearDestination(_)).Times(1);

    registry_->GetTravelManager().ClearDestination(nullptr);
}

TEST_F(ManagerRegistryTest, CanInjectMockBotRepository)
{
    auto mockRepo = std::make_shared<MockBotRepository>();
    registry_->SetBotRepository(mockRepo);

    EXPECT_CALL(*mockRepo, Save(_)).Times(1);

    registry_->GetBotRepository().Save(nullptr);
}

TEST_F(ManagerIntegrationTest, SimulateRandomBotValueStorage)
{
    // Test Get/SetValue which is commonly used
    EXPECT_CALL(*mockRandomBot_, GetValue(testing::A<Player*>(), "testKey"))
        .WillOnce(Return(42u));
    EXPECT_CALL(*mockRandomBot_, SetValue(testing::A<Player*>(), "testKey", 100u, ""))
        .Times(1);

    uint32 value = registry_.GetRandomBotManager().GetValue(nullptr, "testKey");
    EXPECT_EQ(42u, value);

    registry_.GetRandomBotManager().SetValue(nullptr, "testKey", 100u);
}

TEST_F(ManagerIntegrationTest, SimulateTradeMultipliers)
{
    // Test GetBuyMultiplier/GetSellMultiplier commonly used in trade actions
    EXPECT_CALL(*mockRandomBot_, GetBuyMultiplier(_)).WillOnce(Return(0.95));
    EXPECT_CALL(*mockRandomBot_, GetSellMultiplier(_)).WillOnce(Return(1.05));

    double buyMult = registry_.GetRandomBotManager().GetBuyMultiplier(nullptr);
    double sellMult = registry_.GetRandomBotManager().GetSellMultiplier(nullptr);

    EXPECT_DOUBLE_EQ(0.95, buyMult);
    EXPECT_DOUBLE_EQ(1.05, sellMult);
}
