/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <memory>

/**
 * @brief Integration tests for complete action chains
 */

/**
 * @brief Tests for Trigger -> Action chain
 */
class TriggerActionChainTest : public ::testing::Test
{
protected:
    struct Trigger
    {
        std::string name;
        std::function<bool()> isActive;
        std::vector<std::string> boundActions;
        float priority;
    };

    struct Action
    {
        std::string name;
        std::function<bool()> isPossible;
        std::function<bool()> execute;
        float basePriority;
    };

    struct GameState
    {
        float healthPercent;
        float manaPercent;
        bool hasTarget;
        bool inCombat;
        float targetDistance;
        float targetHealthPercent;
    };

    std::vector<Trigger> triggers_;
    std::map<std::string, Action> actions_;
    GameState state_;

    void SetUp() override
    {
        state_ = {100.0f, 100.0f, true, true, 10.0f, 100.0f};

        // Define actions
        actions_["heal_self"] = {
            "heal_self",
            [this]() { return state_.manaPercent >= 20.0f; },
            [this]() { state_.healthPercent += 30.0f; return true; },
            80.0f
        };

        actions_["attack"] = {
            "attack",
            [this]() { return state_.hasTarget && state_.targetDistance <= 30.0f; },
            [this]() { state_.targetHealthPercent -= 10.0f; return true; },
            50.0f
        };

        actions_["flee"] = {
            "flee",
            [this]() { return true; },
            [this]() { state_.targetDistance += 20.0f; return true; },
            30.0f
        };

        // Define triggers
        triggers_ = {
            {
                "critical_health",
                [this]() { return state_.healthPercent < 20.0f; },
                {"heal_self", "flee"},
                100.0f
            },
            {
                "low_health",
                [this]() { return state_.healthPercent < 50.0f; },
                {"heal_self"},
                80.0f
            },
            {
                "has_target",
                [this]() { return state_.hasTarget && state_.inCombat; },
                {"attack"},
                50.0f
            },
        };
    }

    std::vector<Trigger*> GetActiveTriggers()
    {
        std::vector<Trigger*> active;
        for (auto& trigger : triggers_)
        {
            if (trigger.isActive())
                active.push_back(&trigger);
        }

        // Sort by priority
        std::sort(active.begin(), active.end(),
            [](Trigger* a, Trigger* b) { return a->priority > b->priority; });

        return active;
    }

    std::string ExecuteBestAction()
    {
        auto activeTriggers = GetActiveTriggers();

        for (auto* trigger : activeTriggers)
        {
            for (auto const& actionName : trigger->boundActions)
            {
                auto it = actions_.find(actionName);
                if (it != actions_.end() && it->second.isPossible())
                {
                    it->second.execute();
                    return actionName;
                }
            }
        }

        return "";
    }
};

TEST_F(TriggerActionChainTest, NoTriggerAtFullHealth)
{
    auto active = GetActiveTriggers();

    // Only has_target trigger should be active
    EXPECT_EQ(1u, active.size());
    EXPECT_EQ("has_target", active[0]->name);
}

TEST_F(TriggerActionChainTest, LowHealthTriggerActivates)
{
    state_.healthPercent = 40.0f;
    auto active = GetActiveTriggers();

    EXPECT_EQ(2u, active.size());
    EXPECT_EQ("low_health", active[0]->name);  // Higher priority
}

TEST_F(TriggerActionChainTest, CriticalHealthTriggerHighestPriority)
{
    state_.healthPercent = 15.0f;
    auto active = GetActiveTriggers();

    EXPECT_EQ(3u, active.size());
    EXPECT_EQ("critical_health", active[0]->name);
}

TEST_F(TriggerActionChainTest, ExecutesHealWhenLowHealth)
{
    state_.healthPercent = 40.0f;
    std::string executed = ExecuteBestAction();

    EXPECT_EQ("heal_self", executed);
    EXPECT_FLOAT_EQ(70.0f, state_.healthPercent);  // Healed
}

TEST_F(TriggerActionChainTest, ExecutesAttackWhenHealthy)
{
    std::string executed = ExecuteBestAction();

    EXPECT_EQ("attack", executed);
    EXPECT_FLOAT_EQ(90.0f, state_.targetHealthPercent);  // Damaged target
}

TEST_F(TriggerActionChainTest, FallsBackToFleeWhenCantHeal)
{
    state_.healthPercent = 15.0f;
    state_.manaPercent = 10.0f;  // Not enough mana to heal

    std::string executed = ExecuteBestAction();

    EXPECT_EQ("flee", executed);
}

/**
 * @brief Tests for Trigger -> Multiplier -> Action chain
 */
class TriggerMultiplierActionTest : public ::testing::Test
{
protected:
    struct Multiplier
    {
        std::string name;
        std::function<float()> getValue;
    };

    struct ActionWithMultipliers
    {
        std::string name;
        float basePriority;
        std::vector<std::string> multiplierNames;
        std::function<bool()> execute;
    };

    struct MultiplierContext
    {
        float healthPercent;
        float manaPercent;
        bool targetIsElite;
        int aoeTargetCount;
    };

    std::map<std::string, Multiplier> multipliers_;
    std::vector<ActionWithMultipliers> actions_;
    MultiplierContext context_;

    void SetUp() override
    {
        context_ = {100.0f, 100.0f, false, 1};

        multipliers_["health_urgency"] = {
            "health_urgency",
            [this]() {
                if (context_.healthPercent < 20.0f) return 3.0f;
                if (context_.healthPercent < 50.0f) return 2.0f;
                return 1.0f;
            }
        };

        multipliers_["mana_conservation"] = {
            "mana_conservation",
            [this]() {
                if (context_.manaPercent < 20.0f) return 0.5f;
                if (context_.manaPercent < 50.0f) return 0.8f;
                return 1.0f;
            }
        };

        multipliers_["aoe_bonus"] = {
            "aoe_bonus",
            [this]() {
                return 1.0f + (context_.aoeTargetCount - 1) * 0.5f;
            }
        };

        multipliers_["elite_bonus"] = {
            "elite_bonus",
            [this]() {
                return context_.targetIsElite ? 1.5f : 1.0f;
            }
        };

        actions_ = {
            {"heal", 50.0f, {"health_urgency", "mana_conservation"}, []() { return true; }},
            {"single_target_attack", 40.0f, {"elite_bonus"}, []() { return true; }},
            {"aoe_attack", 30.0f, {"aoe_bonus"}, []() { return true; }},
        };
    }

    float CalculateActionPriority(ActionWithMultipliers const& action)
    {
        float priority = action.basePriority;

        for (auto const& multName : action.multiplierNames)
        {
            auto it = multipliers_.find(multName);
            if (it != multipliers_.end())
            {
                priority *= it->second.getValue();
            }
        }

        return priority;
    }

    std::string SelectBestAction()
    {
        ActionWithMultipliers const* best = nullptr;
        float bestPriority = -1.0f;

        for (auto const& action : actions_)
        {
            float priority = CalculateActionPriority(action);
            if (priority > bestPriority)
            {
                best = &action;
                bestPriority = priority;
            }
        }

        return best ? best->name : "";
    }
};

TEST_F(TriggerMultiplierActionTest, NormalPriority)
{
    // All multipliers are 1.0
    float healPriority = CalculateActionPriority(actions_[0]);
    EXPECT_FLOAT_EQ(50.0f, healPriority);
}

TEST_F(TriggerMultiplierActionTest, LowHealthBoostsPriority)
{
    context_.healthPercent = 30.0f;

    float healPriority = CalculateActionPriority(actions_[0]);
    EXPECT_FLOAT_EQ(100.0f, healPriority);  // 50 * 2.0 * 1.0
}

TEST_F(TriggerMultiplierActionTest, LowManaReducesPriority)
{
    context_.manaPercent = 30.0f;

    float healPriority = CalculateActionPriority(actions_[0]);
    EXPECT_FLOAT_EQ(40.0f, healPriority);  // 50 * 1.0 * 0.8
}

TEST_F(TriggerMultiplierActionTest, CombinedMultipliers)
{
    context_.healthPercent = 30.0f;  // 2.0x
    context_.manaPercent = 30.0f;    // 0.8x

    float healPriority = CalculateActionPriority(actions_[0]);
    EXPECT_FLOAT_EQ(80.0f, healPriority);  // 50 * 2.0 * 0.8
}

TEST_F(TriggerMultiplierActionTest, AoeBonusWithMultipleTargets)
{
    context_.aoeTargetCount = 5;

    float aoePriority = CalculateActionPriority(actions_[2]);
    EXPECT_FLOAT_EQ(90.0f, aoePriority);  // 30 * (1.0 + 4*0.5) = 30 * 3.0
}

TEST_F(TriggerMultiplierActionTest, EliteBonusApplied)
{
    context_.targetIsElite = true;

    float stPriority = CalculateActionPriority(actions_[1]);
    EXPECT_FLOAT_EQ(60.0f, stPriority);  // 40 * 1.5
}

TEST_F(TriggerMultiplierActionTest, SelectsAoeWithManyTargets)
{
    context_.aoeTargetCount = 5;

    std::string best = SelectBestAction();
    EXPECT_EQ("aoe_attack", best);
}

TEST_F(TriggerMultiplierActionTest, SelectsHealWhenLowHealth)
{
    context_.healthPercent = 25.0f;

    std::string best = SelectBestAction();
    EXPECT_EQ("heal", best);
}

/**
 * @brief Tests for strategy stacking behavior
 */
class StrategyStackingTest : public ::testing::Test
{
protected:
    struct Strategy
    {
        std::string name;
        int priority;
        std::vector<std::string> triggers;
        std::vector<std::string> actions;
        std::map<std::string, float> actionMultipliers;
    };

    std::vector<Strategy> activeStrategies_;

    void SetUp() override
    {
        activeStrategies_ = {
            {
                "combat",
                100,
                {"enemy_in_range", "low_health"},
                {"attack", "heal"},
                {{"attack", 1.0f}, {"heal", 1.0f}}
            },
            {
                "tank",
                90,
                {"enemy_targeting_ally"},
                {"taunt", "shield_block"},
                {{"attack", 0.8f}, {"taunt", 2.0f}}
            },
            {
                "defensive",
                80,
                {},
                {"heal", "shield"},
                {{"heal", 1.5f}, {"attack", 0.5f}}
            },
        };
    }

    std::vector<std::string> GetAllTriggers()
    {
        std::set<std::string> triggerSet;
        for (auto const& strategy : activeStrategies_)
        {
            for (auto const& trigger : strategy.triggers)
            {
                triggerSet.insert(trigger);
            }
        }
        return std::vector<std::string>(triggerSet.begin(), triggerSet.end());
    }

    std::vector<std::string> GetAllActions()
    {
        std::set<std::string> actionSet;
        for (auto const& strategy : activeStrategies_)
        {
            for (auto const& action : strategy.actions)
            {
                actionSet.insert(action);
            }
        }
        return std::vector<std::string>(actionSet.begin(), actionSet.end());
    }

    float GetCombinedMultiplier(std::string const& actionName)
    {
        float combined = 1.0f;
        for (auto const& strategy : activeStrategies_)
        {
            auto it = strategy.actionMultipliers.find(actionName);
            if (it != strategy.actionMultipliers.end())
            {
                combined *= it->second;
            }
        }
        return combined;
    }

    bool HasConflict(std::string const& action1, std::string const& action2)
    {
        // Check if actions are mutually exclusive
        // Simplified: attack and heal can't happen same GCD
        if ((action1 == "attack" && action2 == "heal") ||
            (action1 == "heal" && action2 == "attack"))
            return true;
        return false;
    }
};

TEST_F(StrategyStackingTest, CombinesTriggers)
{
    auto triggers = GetAllTriggers();

    EXPECT_EQ(3u, triggers.size());
}

TEST_F(StrategyStackingTest, CombinesActions)
{
    auto actions = GetAllActions();

    // attack, heal, taunt, shield_block, shield
    EXPECT_EQ(5u, actions.size());
}

TEST_F(StrategyStackingTest, MultiplierStacking)
{
    // Attack: combat(1.0) * tank(0.8) * defensive(0.5) = 0.4
    float attackMult = GetCombinedMultiplier("attack");
    EXPECT_FLOAT_EQ(0.4f, attackMult);

    // Heal: combat(1.0) * defensive(1.5) = 1.5
    float healMult = GetCombinedMultiplier("heal");
    EXPECT_FLOAT_EQ(1.5f, healMult);

    // Taunt: tank(2.0) = 2.0
    float tauntMult = GetCombinedMultiplier("taunt");
    EXPECT_FLOAT_EQ(2.0f, tauntMult);
}

TEST_F(StrategyStackingTest, ActionConflicts)
{
    EXPECT_TRUE(HasConflict("attack", "heal"));
    EXPECT_FALSE(HasConflict("attack", "taunt"));
    EXPECT_FALSE(HasConflict("heal", "shield"));
}

/**
 * @brief Tests for complete combat simulation
 */
class CombatSimulationTest : public ::testing::Test
{
protected:
    struct CombatState
    {
        float botHealth;
        float botMana;
        float targetHealth;
        int tickCount;
        std::vector<std::string> actionLog;
    };

    CombatState state_;

    void SetUp() override
    {
        state_ = {100.0f, 100.0f, 100.0f, 0, {}};
    }

    std::string DetermineAction()
    {
        // Priority: Critical heal > Normal heal > Attack
        if (state_.botHealth < 20.0f && state_.botMana >= 30.0f)
            return "emergency_heal";
        if (state_.botHealth < 50.0f && state_.botMana >= 20.0f)
            return "heal";
        if (state_.targetHealth > 0.0f && state_.botMana >= 10.0f)
            return "attack";
        return "wand";  // No mana, use wand
    }

    void ExecuteAction(std::string const& action)
    {
        state_.actionLog.push_back(action);

        if (action == "emergency_heal")
        {
            state_.botHealth = std::min(100.0f, state_.botHealth + 40.0f);
            state_.botMana -= 30.0f;
        }
        else if (action == "heal")
        {
            state_.botHealth = std::min(100.0f, state_.botHealth + 25.0f);
            state_.botMana -= 20.0f;
        }
        else if (action == "attack")
        {
            state_.targetHealth -= 15.0f;
            state_.botMana -= 10.0f;
        }
        else if (action == "wand")
        {
            state_.targetHealth -= 5.0f;
        }
    }

    void SimulateTick()
    {
        state_.tickCount++;

        // Bot takes damage
        state_.botHealth -= 8.0f;

        // Regenerate some mana
        state_.botMana = std::min(100.0f, state_.botMana + 2.0f);

        // Execute action
        std::string action = DetermineAction();
        ExecuteAction(action);
    }

    bool SimulateCombat(int maxTicks)
    {
        while (state_.tickCount < maxTicks)
        {
            SimulateTick();

            // Check end conditions
            if (state_.botHealth <= 0.0f)
                return false;  // Bot died
            if (state_.targetHealth <= 0.0f)
                return true;   // Target died
        }

        return state_.targetHealth <= 0.0f;
    }
};

TEST_F(CombatSimulationTest, BotSurvivesCombat)
{
    bool victory = SimulateCombat(20);

    EXPECT_TRUE(victory);
    EXPECT_GT(state_.botHealth, 0.0f);
}

TEST_F(CombatSimulationTest, HealsWhenLow)
{
    SimulateCombat(20);

    // Should have healed at some point
    bool healed = false;
    for (auto const& action : state_.actionLog)
    {
        if (action == "heal" || action == "emergency_heal")
        {
            healed = true;
            break;
        }
    }

    EXPECT_TRUE(healed);
}

TEST_F(CombatSimulationTest, UsesWandWhenOom)
{
    state_.botMana = 5.0f;

    std::string action = DetermineAction();
    EXPECT_EQ("wand", action);
}

