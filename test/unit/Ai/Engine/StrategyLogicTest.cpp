/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <string>
#include <vector>
#include <set>
#include <map>
#include <algorithm>

/**
 * @brief Tests for strategy activation and management logic
 */

/**
 * @brief Tests for basic strategy activation
 */
class StrategyActivationTest : public ::testing::Test
{
protected:
    struct Strategy
    {
        std::string name;
        bool enabled;
        std::vector<std::string> conflicts;  // Strategies that can't be active together
        std::vector<std::string> requires;   // Strategies that must also be active
    };

    std::map<std::string, Strategy> strategies_;

    void SetUp() override
    {
        // Define test strategies
        strategies_["dps"] = {"dps", false, {"tank", "heal"}, {}};
        strategies_["tank"] = {"tank", false, {"dps", "heal"}, {}};
        strategies_["heal"] = {"heal", false, {"dps", "tank"}, {}};
        strategies_["pvp"] = {"pvp", false, {}, {}};
        strategies_["pve"] = {"pve", false, {"pvp"}, {}};
        strategies_["aoe"] = {"aoe", false, {}, {"dps"}};  // Requires dps
        strategies_["stealth"] = {"stealth", false, {"mount"}, {}};
        strategies_["mount"] = {"mount", false, {"stealth", "combat"}, {}};
        strategies_["combat"] = {"combat", false, {"mount"}, {}};
    }

    bool HasConflict(std::string const& strategyName)
    {
        auto it = strategies_.find(strategyName);
        if (it == strategies_.end())
            return false;

        for (auto const& conflict : it->second.conflicts)
        {
            auto conflictIt = strategies_.find(conflict);
            if (conflictIt != strategies_.end() && conflictIt->second.enabled)
                return true;
        }
        return false;
    }

    bool HasRequirements(std::string const& strategyName)
    {
        auto it = strategies_.find(strategyName);
        if (it == strategies_.end())
            return false;

        for (auto const& req : it->second.requires)
        {
            auto reqIt = strategies_.find(req);
            if (reqIt == strategies_.end() || !reqIt->second.enabled)
                return false;
        }
        return true;
    }

    bool CanActivate(std::string const& strategyName)
    {
        return !HasConflict(strategyName) && HasRequirements(strategyName);
    }

    bool Activate(std::string const& strategyName)
    {
        if (!CanActivate(strategyName))
            return false;

        auto it = strategies_.find(strategyName);
        if (it != strategies_.end())
        {
            it->second.enabled = true;
            return true;
        }
        return false;
    }

    void Deactivate(std::string const& strategyName)
    {
        auto it = strategies_.find(strategyName);
        if (it != strategies_.end())
            it->second.enabled = false;
    }

    bool IsActive(std::string const& strategyName)
    {
        auto it = strategies_.find(strategyName);
        return it != strategies_.end() && it->second.enabled;
    }
};

TEST_F(StrategyActivationTest, CanActivateWithNoConflicts)
{
    EXPECT_TRUE(CanActivate("dps"));
    EXPECT_TRUE(Activate("dps"));
    EXPECT_TRUE(IsActive("dps"));
}

TEST_F(StrategyActivationTest, CannotActivateWithConflict)
{
    Activate("dps");
    EXPECT_FALSE(CanActivate("tank"));
    EXPECT_FALSE(Activate("tank"));
    EXPECT_FALSE(IsActive("tank"));
}

TEST_F(StrategyActivationTest, CanActivateAfterDeactivatingConflict)
{
    Activate("dps");
    Deactivate("dps");
    EXPECT_TRUE(CanActivate("tank"));
    EXPECT_TRUE(Activate("tank"));
}

TEST_F(StrategyActivationTest, RequirementsEnforced)
{
    // aoe requires dps
    EXPECT_FALSE(CanActivate("aoe"));
    EXPECT_FALSE(Activate("aoe"));

    Activate("dps");
    EXPECT_TRUE(CanActivate("aoe"));
    EXPECT_TRUE(Activate("aoe"));
}

TEST_F(StrategyActivationTest, MultipleNonConflictingStrategies)
{
    EXPECT_TRUE(Activate("dps"));
    EXPECT_TRUE(Activate("pve"));
    EXPECT_TRUE(IsActive("dps"));
    EXPECT_TRUE(IsActive("pve"));
}

TEST_F(StrategyActivationTest, MutualExclusion)
{
    Activate("pvp");
    EXPECT_FALSE(CanActivate("pve"));

    Deactivate("pvp");
    Activate("pve");
    EXPECT_FALSE(CanActivate("pvp"));
}

/**
 * @brief Tests for strategy state management
 */
class StrategyStateTest : public ::testing::Test
{
protected:
    enum class BotState
    {
        NonCombat,
        Combat,
        Dead,
        Reaction
    };

    struct StrategySet
    {
        std::set<std::string> strategies;
    };

    std::map<BotState, StrategySet> stateStrategies_;

    void SetUp() override
    {
        stateStrategies_[BotState::NonCombat] = {{"follow", "buff", "food"}};
        stateStrategies_[BotState::Combat] = {{"dps", "threat", "interrupt"}};
        stateStrategies_[BotState::Dead] = {{"release", "revive"}};
        stateStrategies_[BotState::Reaction] = {{"flee", "emergency_heal"}};
    }

    std::set<std::string> GetStrategiesForState(BotState state)
    {
        auto it = stateStrategies_.find(state);
        if (it != stateStrategies_.end())
            return it->second.strategies;
        return {};
    }

    bool HasStrategyInState(BotState state, std::string const& strategy)
    {
        auto strategies = GetStrategiesForState(state);
        return strategies.find(strategy) != strategies.end();
    }
};

TEST_F(StrategyStateTest, NonCombatStrategies)
{
    auto strategies = GetStrategiesForState(BotState::NonCombat);
    EXPECT_EQ(3u, strategies.size());
    EXPECT_TRUE(HasStrategyInState(BotState::NonCombat, "follow"));
    EXPECT_TRUE(HasStrategyInState(BotState::NonCombat, "buff"));
    EXPECT_FALSE(HasStrategyInState(BotState::NonCombat, "dps"));
}

TEST_F(StrategyStateTest, CombatStrategies)
{
    auto strategies = GetStrategiesForState(BotState::Combat);
    EXPECT_TRUE(HasStrategyInState(BotState::Combat, "dps"));
    EXPECT_TRUE(HasStrategyInState(BotState::Combat, "interrupt"));
    EXPECT_FALSE(HasStrategyInState(BotState::Combat, "follow"));
}

TEST_F(StrategyStateTest, DeadStrategies)
{
    auto strategies = GetStrategiesForState(BotState::Dead);
    EXPECT_TRUE(HasStrategyInState(BotState::Dead, "release"));
    EXPECT_TRUE(HasStrategyInState(BotState::Dead, "revive"));
}

/**
 * @brief Tests for strategy priority ordering
 */
class StrategyPriorityTest : public ::testing::Test
{
protected:
    struct PrioritizedStrategy
    {
        std::string name;
        int priority;  // Higher = more important
        bool enabled;
    };

    std::vector<PrioritizedStrategy> strategies_;

    void SetUp() override
    {
        strategies_ = {
            {"emergency_heal", 100, true},
            {"interrupt", 90, true},
            {"dispel", 80, true},
            {"dps", 50, true},
            {"buff", 30, true},
            {"follow", 10, true},
        };
    }

    std::vector<std::string> GetOrderedStrategies()
    {
        std::vector<PrioritizedStrategy> enabled;
        std::copy_if(strategies_.begin(), strategies_.end(),
            std::back_inserter(enabled),
            [](PrioritizedStrategy const& s) { return s.enabled; });

        std::sort(enabled.begin(), enabled.end(),
            [](PrioritizedStrategy const& a, PrioritizedStrategy const& b) {
                return a.priority > b.priority;
            });

        std::vector<std::string> result;
        for (auto const& s : enabled)
            result.push_back(s.name);
        return result;
    }

    void DisableStrategy(std::string const& name)
    {
        for (auto& s : strategies_)
        {
            if (s.name == name)
                s.enabled = false;
        }
    }
};

TEST_F(StrategyPriorityTest, OrderedByPriority)
{
    auto ordered = GetOrderedStrategies();
    ASSERT_EQ(6u, ordered.size());
    EXPECT_EQ("emergency_heal", ordered[0]);
    EXPECT_EQ("interrupt", ordered[1]);
    EXPECT_EQ("follow", ordered[5]);
}

TEST_F(StrategyPriorityTest, DisabledStrategiesExcluded)
{
    DisableStrategy("interrupt");
    auto ordered = GetOrderedStrategies();
    EXPECT_EQ(5u, ordered.size());

    bool found = false;
    for (auto const& s : ordered)
    {
        if (s == "interrupt")
            found = true;
    }
    EXPECT_FALSE(found);
}

