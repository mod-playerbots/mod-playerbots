/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <string>
#include <vector>
#include <map>
#include <set>

/**
 * @brief Tests for bot state machine transitions
 */

/**
 * @brief Basic state machine tests
 */
class BotStateMachineTest : public ::testing::Test
{
protected:
    enum class BotState
    {
        Idle,
        Following,
        Combat,
        Fleeing,
        Dead,
        Looting,
        Resting,
        Mounted
    };

    struct StateContext
    {
        bool isAlive;
        bool inCombat;
        bool hasLoot;
        bool needsRest;
        bool masterMounted;
        float healthPercent;
        float manaPercent;
        float distanceToMaster;
        float distanceToEnemy;
    };

    BotState currentState_;
    StateContext context_;
    std::map<BotState, std::set<BotState>> validTransitions_;

    void SetUp() override
    {
        currentState_ = BotState::Idle;
        context_ = {true, false, false, false, false, 100.0f, 100.0f, 5.0f, 50.0f};

        // Define valid state transitions
        validTransitions_[BotState::Idle] = {
            BotState::Following, BotState::Combat, BotState::Dead,
            BotState::Resting, BotState::Mounted
        };
        validTransitions_[BotState::Following] = {
            BotState::Idle, BotState::Combat, BotState::Dead,
            BotState::Mounted
        };
        validTransitions_[BotState::Combat] = {
            BotState::Idle, BotState::Fleeing, BotState::Dead,
            BotState::Looting
        };
        validTransitions_[BotState::Fleeing] = {
            BotState::Combat, BotState::Dead, BotState::Idle
        };
        validTransitions_[BotState::Dead] = {
            BotState::Idle  // After resurrection
        };
        validTransitions_[BotState::Looting] = {
            BotState::Idle, BotState::Combat
        };
        validTransitions_[BotState::Resting] = {
            BotState::Idle, BotState::Combat
        };
        validTransitions_[BotState::Mounted] = {
            BotState::Idle, BotState::Following, BotState::Combat
        };
    }

    bool CanTransition(BotState from, BotState to)
    {
        auto it = validTransitions_.find(from);
        if (it == validTransitions_.end())
            return false;
        return it->second.find(to) != it->second.end();
    }

    bool TryTransition(BotState newState)
    {
        if (!CanTransition(currentState_, newState))
            return false;
        currentState_ = newState;
        return true;
    }

    BotState DetermineDesiredState()
    {
        // Death takes priority
        if (!context_.isAlive)
            return BotState::Dead;

        // Combat check
        if (context_.inCombat)
        {
            if (context_.healthPercent < 20.0f)
                return BotState::Fleeing;
            return BotState::Combat;
        }

        // Post-combat
        if (context_.hasLoot)
            return BotState::Looting;

        // Rest if needed
        if (context_.needsRest &&
            (context_.healthPercent < 80.0f || context_.manaPercent < 80.0f))
            return BotState::Resting;

        // Follow master
        if (context_.masterMounted)
            return BotState::Mounted;

        if (context_.distanceToMaster > 10.0f)
            return BotState::Following;

        return BotState::Idle;
    }
};

TEST_F(BotStateMachineTest, InitialStateIsIdle)
{
    EXPECT_EQ(BotState::Idle, currentState_);
}

TEST_F(BotStateMachineTest, ValidTransitionSucceeds)
{
    EXPECT_TRUE(TryTransition(BotState::Following));
    EXPECT_EQ(BotState::Following, currentState_);
}

TEST_F(BotStateMachineTest, InvalidTransitionFails)
{
    currentState_ = BotState::Dead;
    EXPECT_FALSE(TryTransition(BotState::Combat));  // Can't fight when dead
    EXPECT_EQ(BotState::Dead, currentState_);
}

TEST_F(BotStateMachineTest, DeathTakesPriority)
{
    context_.isAlive = false;
    context_.inCombat = true;  // Even in combat
    EXPECT_EQ(BotState::Dead, DetermineDesiredState());
}

TEST_F(BotStateMachineTest, FleeAtLowHealth)
{
    context_.inCombat = true;
    context_.healthPercent = 15.0f;
    EXPECT_EQ(BotState::Fleeing, DetermineDesiredState());
}

TEST_F(BotStateMachineTest, CombatAtNormalHealth)
{
    context_.inCombat = true;
    context_.healthPercent = 80.0f;
    EXPECT_EQ(BotState::Combat, DetermineDesiredState());
}

TEST_F(BotStateMachineTest, LootAfterCombat)
{
    context_.hasLoot = true;
    EXPECT_EQ(BotState::Looting, DetermineDesiredState());
}

TEST_F(BotStateMachineTest, RestWhenNeeded)
{
    context_.needsRest = true;
    context_.manaPercent = 30.0f;
    EXPECT_EQ(BotState::Resting, DetermineDesiredState());
}

TEST_F(BotStateMachineTest, MountWhenMasterMounts)
{
    context_.masterMounted = true;
    EXPECT_EQ(BotState::Mounted, DetermineDesiredState());
}

TEST_F(BotStateMachineTest, FollowWhenFar)
{
    context_.distanceToMaster = 25.0f;
    EXPECT_EQ(BotState::Following, DetermineDesiredState());
}

/**
 * @brief Tests for combat state substates
 */
class CombatSubstateTest : public ::testing::Test
{
protected:
    enum class CombatSubstate
    {
        Engaging,       // Moving to target
        Fighting,       // In melee range, attacking
        Kiting,        // Keeping distance
        Positioning,   // Moving to optimal position
        Waiting,       // Waiting for cooldowns/resources
        Interrupting   // Priority interrupt
    };

    struct CombatContext
    {
        bool isMelee;
        bool hasTarget;
        float distanceToTarget;
        float optimalRange;
        bool spellReady;
        bool needsInterrupt;
        bool targetCasting;
    };

    CombatContext context_;

    void SetUp() override
    {
        context_ = {true, true, 5.0f, 5.0f, true, false, false};
    }

    CombatSubstate DetermineCombatSubstate()
    {
        if (!context_.hasTarget)
            return CombatSubstate::Waiting;

        // Interrupt takes priority
        if (context_.needsInterrupt && context_.targetCasting)
            return CombatSubstate::Interrupting;

        // Check range
        float rangeDiff = context_.distanceToTarget - context_.optimalRange;

        if (context_.isMelee)
        {
            if (rangeDiff > 5.0f)
                return CombatSubstate::Engaging;
            if (rangeDiff > 0.5f)
                return CombatSubstate::Positioning;
        }
        else  // Ranged
        {
            if (context_.distanceToTarget < 8.0f)
                return CombatSubstate::Kiting;
            if (context_.distanceToTarget > context_.optimalRange + 10.0f)
                return CombatSubstate::Positioning;
        }

        if (!context_.spellReady)
            return CombatSubstate::Waiting;

        return CombatSubstate::Fighting;
    }
};

TEST_F(CombatSubstateTest, MeleeEngaging)
{
    context_.distanceToTarget = 30.0f;
    EXPECT_EQ(CombatSubstate::Engaging, DetermineCombatSubstate());
}

TEST_F(CombatSubstateTest, MeleeFighting)
{
    context_.distanceToTarget = 5.0f;
    EXPECT_EQ(CombatSubstate::Fighting, DetermineCombatSubstate());
}

TEST_F(CombatSubstateTest, MeleePositioning)
{
    context_.distanceToTarget = 8.0f;  // Slightly out of range
    EXPECT_EQ(CombatSubstate::Positioning, DetermineCombatSubstate());
}

TEST_F(CombatSubstateTest, RangedKiting)
{
    context_.isMelee = false;
    context_.optimalRange = 30.0f;
    context_.distanceToTarget = 5.0f;  // Too close
    EXPECT_EQ(CombatSubstate::Kiting, DetermineCombatSubstate());
}

TEST_F(CombatSubstateTest, RangedPositioning)
{
    context_.isMelee = false;
    context_.optimalRange = 30.0f;
    context_.distanceToTarget = 50.0f;  // Too far
    EXPECT_EQ(CombatSubstate::Positioning, DetermineCombatSubstate());
}

TEST_F(CombatSubstateTest, InterruptPriority)
{
    context_.needsInterrupt = true;
    context_.targetCasting = true;
    EXPECT_EQ(CombatSubstate::Interrupting, DetermineCombatSubstate());
}

TEST_F(CombatSubstateTest, WaitingForCooldown)
{
    context_.spellReady = false;
    EXPECT_EQ(CombatSubstate::Waiting, DetermineCombatSubstate());
}

/**
 * @brief Tests for state transition events
 */
class StateTransitionEventTest : public ::testing::Test
{
protected:
    enum class StateEvent
    {
        EnterCombat,
        LeaveCombat,
        Die,
        Resurrect,
        StartResting,
        FinishResting,
        Mount,
        Dismount
    };

    struct EventLog
    {
        std::vector<StateEvent> events;
    };

    EventLog log_;

    void OnEnterCombat() { log_.events.push_back(StateEvent::EnterCombat); }
    void OnLeaveCombat() { log_.events.push_back(StateEvent::LeaveCombat); }
    void OnDie() { log_.events.push_back(StateEvent::Die); }
    void OnResurrect() { log_.events.push_back(StateEvent::Resurrect); }

    void SimulateStateChange(std::string const& from, std::string const& to)
    {
        if (to == "combat" && from != "combat")
            OnEnterCombat();
        if (from == "combat" && to != "combat")
            OnLeaveCombat();
        if (to == "dead")
            OnDie();
        if (from == "dead")
            OnResurrect();
    }

    bool HasEvent(StateEvent event)
    {
        return std::find(log_.events.begin(), log_.events.end(), event) != log_.events.end();
    }

    size_t EventCount() { return log_.events.size(); }
};

TEST_F(StateTransitionEventTest, EnterCombatEventFires)
{
    SimulateStateChange("idle", "combat");
    EXPECT_TRUE(HasEvent(StateEvent::EnterCombat));
}

TEST_F(StateTransitionEventTest, LeaveCombatEventFires)
{
    SimulateStateChange("combat", "idle");
    EXPECT_TRUE(HasEvent(StateEvent::LeaveCombat));
}

TEST_F(StateTransitionEventTest, DieEventFires)
{
    SimulateStateChange("combat", "dead");
    EXPECT_TRUE(HasEvent(StateEvent::Die));
    EXPECT_TRUE(HasEvent(StateEvent::LeaveCombat));
}

TEST_F(StateTransitionEventTest, ResurrectEventFires)
{
    SimulateStateChange("dead", "idle");
    EXPECT_TRUE(HasEvent(StateEvent::Resurrect));
}

TEST_F(StateTransitionEventTest, NoEventForSameState)
{
    SimulateStateChange("combat", "combat");
    EXPECT_EQ(0u, EventCount());
}

