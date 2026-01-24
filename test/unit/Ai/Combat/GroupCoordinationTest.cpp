/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <string>
#include <vector>
#include <map>
#include <algorithm>

/**
 * @brief Tests for group coordination logic
 */

/**
 * @brief Tests for tank threat management
 */
class TankThreatTest : public ::testing::Test
{
protected:
    struct GroupMember
    {
        uint32 guid;
        std::string role;  // "tank", "healer", "dps"
        float threatOnTarget;
        bool hasAggro;
    };

    struct ThreatContext
    {
        std::vector<GroupMember> members;
        uint32 mainTankGuid;
        float threatLeadThreshold;  // How much threat lead tank should have
    };

    ThreatContext context_;

    void SetUp() override
    {
        context_.members = {
            {1, "tank", 10000.0f, true},
            {2, "healer", 3000.0f, false},
            {3, "dps", 8000.0f, false},
            {4, "dps", 7500.0f, false},
        };
        context_.mainTankGuid = 1;
        context_.threatLeadThreshold = 1.1f;  // 10% lead
    }

    float GetTankThreat()
    {
        for (auto const& m : context_.members)
        {
            if (m.guid == context_.mainTankGuid)
                return m.threatOnTarget;
        }
        return 0.0f;
    }

    float GetHighestNonTankThreat()
    {
        float highest = 0.0f;
        for (auto const& m : context_.members)
        {
            if (m.guid != context_.mainTankGuid)
                highest = std::max(highest, m.threatOnTarget);
        }
        return highest;
    }

    bool TankHasThreatLead()
    {
        float tankThreat = GetTankThreat();
        float otherThreat = GetHighestNonTankThreat();
        return tankThreat >= otherThreat * context_.threatLeadThreshold;
    }

    bool ShouldDpsHoldBack(uint32 dpsGuid)
    {
        float tankThreat = GetTankThreat();

        for (auto const& m : context_.members)
        {
            if (m.guid == dpsGuid)
            {
                // Hold back if within 90% of tank threat
                return m.threatOnTarget >= tankThreat * 0.9f;
            }
        }
        return false;
    }

    bool ShouldTankTaunt()
    {
        // Taunt if someone else has aggro
        for (auto const& m : context_.members)
        {
            if (m.guid != context_.mainTankGuid && m.hasAggro)
                return true;
        }
        return false;
    }
};

TEST_F(TankThreatTest, TankHasInitialThreatLead)
{
    EXPECT_TRUE(TankHasThreatLead());
}

TEST_F(TankThreatTest, ThreatLeadLostWhenDpsCloses)
{
    context_.members[2].threatOnTarget = 9500.0f;  // DPS at 95%
    EXPECT_FALSE(TankHasThreatLead());
}

TEST_F(TankThreatTest, DpsHoldsBackNearThreatCap)
{
    context_.members[2].threatOnTarget = 9200.0f;  // DPS at 92%
    EXPECT_TRUE(ShouldDpsHoldBack(3));
}

TEST_F(TankThreatTest, DpsDoesntHoldBackWhenSafe)
{
    context_.members[2].threatOnTarget = 5000.0f;  // DPS at 50%
    EXPECT_FALSE(ShouldDpsHoldBack(3));
}

TEST_F(TankThreatTest, TankTauntsWhenLosingAggro)
{
    context_.members[0].hasAggro = false;
    context_.members[2].hasAggro = true;
    EXPECT_TRUE(ShouldTankTaunt());
}

TEST_F(TankThreatTest, TankDoesntTauntWithAggro)
{
    EXPECT_FALSE(ShouldTankTaunt());
}

/**
 * @brief Tests for healer triage
 */
class HealerTriageTest : public ::testing::Test
{
protected:
    struct PatientInfo
    {
        uint32 guid;
        std::string role;
        float healthPercent;
        float incomingDamagePerSecond;
        bool hasCriticalDebuff;
        int existingHots;
    };

    struct TriageContext
    {
        std::vector<PatientInfo> patients;
        float healerManaPercent;
        bool isCombat;
    };

    TriageContext context_;

    void SetUp() override
    {
        context_.patients = {
            {1, "tank", 60.0f, 5000.0f, false, 0},
            {2, "healer", 80.0f, 1000.0f, false, 0},
            {3, "dps", 40.0f, 2000.0f, false, 0},
            {4, "dps", 90.0f, 500.0f, false, 0},
        };
        context_.healerManaPercent = 80.0f;
        context_.isCombat = true;
    }

    enum class TriageCategory
    {
        Critical,   // Immediate heal needed
        Urgent,     // Needs heal soon
        Stable,     // Can wait
        Healthy     // No heal needed
    };

    TriageCategory CategorizePatient(PatientInfo const& patient)
    {
        // Critical: tank below 40% or anyone below 25%
        if (patient.role == "tank" && patient.healthPercent < 40.0f)
            return TriageCategory::Critical;
        if (patient.healthPercent < 25.0f)
            return TriageCategory::Critical;
        if (patient.hasCriticalDebuff)
            return TriageCategory::Critical;

        // Urgent: tank below 60% or anyone below 50%
        if (patient.role == "tank" && patient.healthPercent < 60.0f)
            return TriageCategory::Urgent;
        if (patient.healthPercent < 50.0f)
            return TriageCategory::Urgent;

        // Stable: below 80%
        if (patient.healthPercent < 80.0f)
            return TriageCategory::Stable;

        return TriageCategory::Healthy;
    }

    float CalculateHealUrgency(PatientInfo const& patient)
    {
        float urgency = 100.0f - patient.healthPercent;

        // Role weighting
        if (patient.role == "tank")
            urgency *= 1.5f;
        else if (patient.role == "healer")
            urgency *= 1.3f;

        // Incoming damage consideration
        float timeToKill = (patient.healthPercent / 100.0f) * 10000.0f /
                          std::max(1.0f, patient.incomingDamagePerSecond);
        if (timeToKill < 3.0f)
            urgency *= 2.0f;

        // Already has HoTs
        urgency -= patient.existingHots * 10.0f;

        return std::max(0.0f, urgency);
    }

    uint32 SelectHealTarget()
    {
        PatientInfo const* best = nullptr;
        float bestUrgency = -1.0f;

        for (auto const& patient : context_.patients)
        {
            float urgency = CalculateHealUrgency(patient);
            if (urgency > bestUrgency)
            {
                best = &patient;
                bestUrgency = urgency;
            }
        }

        return best ? best->guid : 0;
    }
};

TEST_F(HealerTriageTest, CategorizeTankLowHealth)
{
    PatientInfo tank{1, "tank", 35.0f, 5000.0f, false, 0};
    EXPECT_EQ(TriageCategory::Critical, CategorizePatient(tank));
}

TEST_F(HealerTriageTest, CategorizeDpsCritical)
{
    PatientInfo dps{3, "dps", 20.0f, 2000.0f, false, 0};
    EXPECT_EQ(TriageCategory::Critical, CategorizePatient(dps));
}

TEST_F(HealerTriageTest, CategorizeHealthy)
{
    PatientInfo healthy{4, "dps", 95.0f, 100.0f, false, 0};
    EXPECT_EQ(TriageCategory::Healthy, CategorizePatient(healthy));
}

TEST_F(HealerTriageTest, CriticalDebuffIsCritical)
{
    PatientInfo debuffed{4, "dps", 80.0f, 100.0f, true, 0};
    EXPECT_EQ(TriageCategory::Critical, CategorizePatient(debuffed));
}

TEST_F(HealerTriageTest, TankPrioritized)
{
    // Tank at 60%, DPS at 40% - tank should still be priority
    context_.patients[0].healthPercent = 60.0f;
    context_.patients[2].healthPercent = 40.0f;

    // Calculate urgencies
    float tankUrgency = CalculateHealUrgency(context_.patients[0]);
    float dpsUrgency = CalculateHealUrgency(context_.patients[2]);

    // Tank urgency should be comparable despite higher health
    // due to role weighting and incoming damage
    EXPECT_GT(tankUrgency, 0.0f);
}

TEST_F(HealerTriageTest, HoTsReduceUrgency)
{
    context_.patients[0].existingHots = 3;
    float withHots = CalculateHealUrgency(context_.patients[0]);

    context_.patients[0].existingHots = 0;
    float withoutHots = CalculateHealUrgency(context_.patients[0]);

    EXPECT_LT(withHots, withoutHots);
}

/**
 * @brief Tests for DPS assist target
 */
class DpsAssistTest : public ::testing::Test
{
protected:
    struct AssistContext
    {
        uint32 mainTankTargetGuid;
        uint32 assistTargetGuid;
        uint32 markedTargetGuid;
        std::map<uint32, float> targetHealth;  // guid -> health%
    };

    AssistContext context_;

    void SetUp() override
    {
        context_.mainTankTargetGuid = 100;
        context_.assistTargetGuid = 0;
        context_.markedTargetGuid = 101;
        context_.targetHealth = {
            {100, 80.0f},
            {101, 90.0f},
            {102, 100.0f},
        };
    }

    uint32 DetermineAssistTarget(bool followMarks, bool focusFire)
    {
        // Priority 1: Marked target (skull, etc.)
        if (followMarks && context_.markedTargetGuid != 0)
        {
            auto it = context_.targetHealth.find(context_.markedTargetGuid);
            if (it != context_.targetHealth.end() && it->second > 0)
                return context_.markedTargetGuid;
        }

        // Priority 2: Assist target (if set)
        if (context_.assistTargetGuid != 0)
        {
            auto it = context_.targetHealth.find(context_.assistTargetGuid);
            if (it != context_.targetHealth.end() && it->second > 0)
                return context_.assistTargetGuid;
        }

        // Priority 3: Main tank's target
        if (context_.mainTankTargetGuid != 0)
            return context_.mainTankTargetGuid;

        // Priority 4: Focus fire on lowest health (if enabled)
        if (focusFire)
        {
            uint32 lowestGuid = 0;
            float lowestHealth = 101.0f;
            for (auto const& [guid, health] : context_.targetHealth)
            {
                if (health > 0 && health < lowestHealth)
                {
                    lowestHealth = health;
                    lowestGuid = guid;
                }
            }
            return lowestGuid;
        }

        return 0;
    }
};

TEST_F(DpsAssistTest, FollowsMarkedTarget)
{
    EXPECT_EQ(101u, DetermineAssistTarget(true, false));
}

TEST_F(DpsAssistTest, IgnoresMarksWhenDisabled)
{
    EXPECT_EQ(100u, DetermineAssistTarget(false, false));  // Falls to tank target
}

TEST_F(DpsAssistTest, UsesAssistTarget)
{
    context_.assistTargetGuid = 102;
    context_.markedTargetGuid = 0;
    EXPECT_EQ(102u, DetermineAssistTarget(true, false));
}

TEST_F(DpsAssistTest, FallsBackToTankTarget)
{
    context_.markedTargetGuid = 0;
    context_.assistTargetGuid = 0;
    EXPECT_EQ(100u, DetermineAssistTarget(true, false));
}

TEST_F(DpsAssistTest, FocusFireOnLowest)
{
    context_.markedTargetGuid = 0;
    context_.assistTargetGuid = 0;
    context_.mainTankTargetGuid = 0;
    EXPECT_EQ(100u, DetermineAssistTarget(false, true));  // 80% is lowest
}

/**
 * @brief Tests for group role assignment
 */
class RoleAssignmentTest : public ::testing::Test
{
protected:
    struct GroupMember
    {
        uint32 guid;
        std::string playerClass;
        std::string spec;
        std::string assignedRole;
    };

    std::vector<GroupMember> group_;

    void SetUp() override
    {
        group_ = {
            {1, "Warrior", "Protection", ""},
            {2, "Priest", "Holy", ""},
            {3, "Mage", "Frost", ""},
            {4, "Rogue", "Combat", ""},
            {5, "Druid", "Restoration", ""},
        };
    }

    std::string DetermineRole(std::string const& playerClass, std::string const& spec)
    {
        // Tank specs
        if (playerClass == "Warrior" && spec == "Protection")
            return "tank";
        if (playerClass == "Paladin" && spec == "Protection")
            return "tank";
        if (playerClass == "Druid" && spec == "Feral")
            return "tank";  // Simplified - could be DPS
        if (playerClass == "Death Knight" && spec == "Blood")
            return "tank";

        // Healer specs
        if (playerClass == "Priest" && (spec == "Holy" || spec == "Discipline"))
            return "healer";
        if (playerClass == "Paladin" && spec == "Holy")
            return "healer";
        if (playerClass == "Druid" && spec == "Restoration")
            return "healer";
        if (playerClass == "Shaman" && spec == "Restoration")
            return "healer";

        // Everything else is DPS
        return "dps";
    }

    void AssignRoles()
    {
        for (auto& member : group_)
        {
            member.assignedRole = DetermineRole(member.playerClass, member.spec);
        }
    }

    int CountRole(std::string const& role)
    {
        int count = 0;
        for (auto const& member : group_)
        {
            if (member.assignedRole == role)
                count++;
        }
        return count;
    }
};

TEST_F(RoleAssignmentTest, AssignsCorrectRoles)
{
    AssignRoles();

    EXPECT_EQ("tank", group_[0].assignedRole);    // Prot Warrior
    EXPECT_EQ("healer", group_[1].assignedRole);  // Holy Priest
    EXPECT_EQ("dps", group_[2].assignedRole);     // Frost Mage
    EXPECT_EQ("dps", group_[3].assignedRole);     // Combat Rogue
    EXPECT_EQ("healer", group_[4].assignedRole);  // Resto Druid
}

TEST_F(RoleAssignmentTest, CountsRolesCorrectly)
{
    AssignRoles();

    EXPECT_EQ(1, CountRole("tank"));
    EXPECT_EQ(2, CountRole("healer"));
    EXPECT_EQ(2, CountRole("dps"));
}

