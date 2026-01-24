/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <string>
#include <vector>
#include <functional>
#include <algorithm>

/**
 * @brief Tests for engine tick and action selection logic
 */

/**
 * @brief Tests for action relevance checking
 */
class ActionRelevanceTest : public ::testing::Test
{
protected:
    struct ActionContext
    {
        bool hasTarget;
        bool inCombat;
        bool hasResources;
        float healthPercent;
        float manaPercent;
        float targetDistance;
    };

    struct ActionDefinition
    {
        std::string name;
        std::function<bool(ActionContext const&)> isRelevant;
        std::function<bool(ActionContext const&)> isPossible;
        float basePriority;
    };

    std::vector<ActionDefinition> actions_;
    ActionContext context_;

    void SetUp() override
    {
        // Define test actions
        actions_ = {
            {"heal_self",
                [](ActionContext const& ctx) { return ctx.healthPercent < 50.0f; },
                [](ActionContext const& ctx) { return ctx.manaPercent >= 10.0f; },
                90.0f},
            {"attack",
                [](ActionContext const& ctx) { return ctx.hasTarget && ctx.inCombat; },
                [](ActionContext const& ctx) { return ctx.targetDistance <= 30.0f; },
                50.0f},
            {"follow",
                [](ActionContext const& ctx) { return !ctx.inCombat && ctx.targetDistance > 5.0f; },
                [](ActionContext const& ctx) { return true; },
                20.0f},
            {"emergency_heal",
                [](ActionContext const& ctx) { return ctx.healthPercent < 20.0f; },
                [](ActionContext const& ctx) { return ctx.manaPercent >= 5.0f; },
                100.0f},
        };

        // Default context
        context_ = {true, true, true, 80.0f, 100.0f, 10.0f};
    }

    std::vector<std::string> GetRelevantActions()
    {
        std::vector<std::string> result;
        for (auto const& action : actions_)
        {
            if (action.isRelevant(context_))
                result.push_back(action.name);
        }
        return result;
    }

    std::vector<std::string> GetPossibleActions()
    {
        std::vector<std::string> result;
        for (auto const& action : actions_)
        {
            if (action.isRelevant(context_) && action.isPossible(context_))
                result.push_back(action.name);
        }
        return result;
    }

    std::string SelectBestAction()
    {
        ActionDefinition const* best = nullptr;
        float bestPriority = -1.0f;

        for (auto const& action : actions_)
        {
            if (action.isRelevant(context_) && action.isPossible(context_))
            {
                if (action.basePriority > bestPriority)
                {
                    best = &action;
                    bestPriority = action.basePriority;
                }
            }
        }

        return best ? best->name : "";
    }
};

TEST_F(ActionRelevanceTest, FullHealthNoHealRelevant)
{
    context_.healthPercent = 100.0f;
    auto relevant = GetRelevantActions();

    bool hasHeal = std::find(relevant.begin(), relevant.end(), "heal_self") != relevant.end();
    EXPECT_FALSE(hasHeal);
}

TEST_F(ActionRelevanceTest, LowHealthHealRelevant)
{
    context_.healthPercent = 40.0f;
    auto relevant = GetRelevantActions();

    bool hasHeal = std::find(relevant.begin(), relevant.end(), "heal_self") != relevant.end();
    EXPECT_TRUE(hasHeal);
}

TEST_F(ActionRelevanceTest, CriticalHealthEmergencyHeal)
{
    context_.healthPercent = 15.0f;
    std::string best = SelectBestAction();
    EXPECT_EQ("emergency_heal", best);
}

TEST_F(ActionRelevanceTest, InCombatAttackRelevant)
{
    auto relevant = GetRelevantActions();
    bool hasAttack = std::find(relevant.begin(), relevant.end(), "attack") != relevant.end();
    EXPECT_TRUE(hasAttack);
}

TEST_F(ActionRelevanceTest, OutOfCombatAttackNotRelevant)
{
    context_.inCombat = false;
    auto relevant = GetRelevantActions();

    bool hasAttack = std::find(relevant.begin(), relevant.end(), "attack") != relevant.end();
    EXPECT_FALSE(hasAttack);
}

TEST_F(ActionRelevanceTest, OutOfRangeAttackNotPossible)
{
    context_.targetDistance = 50.0f;
    auto possible = GetPossibleActions();

    bool hasAttack = std::find(possible.begin(), possible.end(), "attack") != possible.end();
    EXPECT_FALSE(hasAttack);
}

/**
 * @brief Tests for engine tick timing
 */
class EngineTickTimingTest : public ::testing::Test
{
protected:
    struct TickState
    {
        uint32 lastTickTime;
        uint32 tickInterval;
        uint32 minTickInterval;
        bool isPaused;
    };

    TickState state_;

    void SetUp() override
    {
        state_ = {0, 100, 50, false};
    }

    bool ShouldTick(uint32 currentTime)
    {
        if (state_.isPaused)
            return false;

        uint32 elapsed = currentTime - state_.lastTickTime;
        return elapsed >= state_.tickInterval;
    }

    void DoTick(uint32 currentTime)
    {
        state_.lastTickTime = currentTime;
    }

    uint32 GetNextTickTime()
    {
        return state_.lastTickTime + state_.tickInterval;
    }

    void SetTickInterval(uint32 interval)
    {
        state_.tickInterval = std::max(interval, state_.minTickInterval);
    }
};

TEST_F(EngineTickTimingTest, TicksAtInterval)
{
    EXPECT_TRUE(ShouldTick(100));
    DoTick(100);
    EXPECT_FALSE(ShouldTick(150));
    EXPECT_TRUE(ShouldTick(200));
}

TEST_F(EngineTickTimingTest, PausedDoesNotTick)
{
    state_.isPaused = true;
    EXPECT_FALSE(ShouldTick(1000));
}

TEST_F(EngineTickTimingTest, NextTickTimeCalculation)
{
    DoTick(100);
    EXPECT_EQ(200u, GetNextTickTime());
}

TEST_F(EngineTickTimingTest, MinTickIntervalEnforced)
{
    SetTickInterval(10);
    EXPECT_EQ(state_.minTickInterval, state_.tickInterval);
}

/**
 * @brief Tests for action execution queue
 */
class ActionExecutionQueueTest : public ::testing::Test
{
protected:
    enum class ActionResult
    {
        Success,
        Failed,
        Impossible,
        InProgress
    };

    struct QueuedAction
    {
        std::string name;
        float priority;
        int retryCount;
        int maxRetries;
        ActionResult lastResult;
    };

    std::vector<QueuedAction> queue_;

    void AddAction(std::string const& name, float priority, int maxRetries = 3)
    {
        queue_.push_back({name, priority, 0, maxRetries, ActionResult::InProgress});
    }

    void SortQueue()
    {
        std::sort(queue_.begin(), queue_.end(),
            [](QueuedAction const& a, QueuedAction const& b) {
                return a.priority > b.priority;
            });
    }

    QueuedAction* GetNextAction()
    {
        SortQueue();
        for (auto& action : queue_)
        {
            if (action.lastResult != ActionResult::Impossible &&
                action.retryCount < action.maxRetries)
            {
                return &action;
            }
        }
        return nullptr;
    }

    void RecordResult(std::string const& name, ActionResult result)
    {
        for (auto& action : queue_)
        {
            if (action.name == name)
            {
                action.lastResult = result;
                if (result == ActionResult::Failed)
                    action.retryCount++;
                break;
            }
        }
    }

    void ClearCompleted()
    {
        queue_.erase(
            std::remove_if(queue_.begin(), queue_.end(),
                [](QueuedAction const& a) {
                    return a.lastResult == ActionResult::Success ||
                           a.lastResult == ActionResult::Impossible ||
                           a.retryCount >= a.maxRetries;
                }),
            queue_.end());
    }
};

TEST_F(ActionExecutionQueueTest, SelectsHighestPriority)
{
    AddAction("low", 10.0f);
    AddAction("high", 100.0f);
    AddAction("medium", 50.0f);

    auto* next = GetNextAction();
    ASSERT_NE(nullptr, next);
    EXPECT_EQ("high", next->name);
}

TEST_F(ActionExecutionQueueTest, SkipsImpossibleActions)
{
    AddAction("impossible", 100.0f);
    AddAction("possible", 50.0f);
    RecordResult("impossible", ActionResult::Impossible);

    auto* next = GetNextAction();
    ASSERT_NE(nullptr, next);
    EXPECT_EQ("possible", next->name);
}

TEST_F(ActionExecutionQueueTest, RetriesFailedActions)
{
    AddAction("flaky", 100.0f, 3);
    RecordResult("flaky", ActionResult::Failed);
    RecordResult("flaky", ActionResult::Failed);

    auto* next = GetNextAction();
    ASSERT_NE(nullptr, next);
    EXPECT_EQ("flaky", next->name);
    EXPECT_EQ(2, next->retryCount);
}

TEST_F(ActionExecutionQueueTest, StopsAfterMaxRetries)
{
    AddAction("always_fails", 100.0f, 2);
    RecordResult("always_fails", ActionResult::Failed);
    RecordResult("always_fails", ActionResult::Failed);

    auto* next = GetNextAction();
    EXPECT_EQ(nullptr, next);
}

TEST_F(ActionExecutionQueueTest, ClearCompletedRemovesSuccessful)
{
    AddAction("success", 100.0f);
    AddAction("pending", 50.0f);
    RecordResult("success", ActionResult::Success);
    ClearCompleted();

    EXPECT_EQ(1u, queue_.size());
    EXPECT_EQ("pending", queue_[0].name);
}

