/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <cmath>
#include <vector>
#include <algorithm>

/**
 * @brief Tests for positioning and movement logic
 */

/**
 * @brief Tests for formation positioning
 */
class FormationPositioningTest : public ::testing::Test
{
protected:
    struct Position
    {
        float x, y, z;
        float orientation;
    };

    struct FormationSlot
    {
        float offsetX;  // Relative to leader, in leader's local space
        float offsetY;
        int priority;   // Lower = closer to leader
    };

    Position leaderPos_;
    std::vector<FormationSlot> formationSlots_;

    void SetUp() override
    {
        leaderPos_ = {100.0f, 100.0f, 0.0f, 0.0f};

        // V-formation behind leader
        formationSlots_ = {
            {-2.0f, -2.0f, 1},   // Back-left
            {2.0f, -2.0f, 2},    // Back-right
            {-4.0f, -4.0f, 3},   // Far back-left
            {4.0f, -4.0f, 4},    // Far back-right
        };
    }

    float Distance(Position const& a, Position const& b)
    {
        float dx = a.x - b.x;
        float dy = a.y - b.y;
        return std::sqrt(dx * dx + dy * dy);
    }

    Position CalculateWorldPosition(FormationSlot const& slot, Position const& leader)
    {
        // Rotate offset by leader's orientation
        float cosO = std::cos(leader.orientation);
        float sinO = std::sin(leader.orientation);

        float worldOffsetX = slot.offsetX * cosO - slot.offsetY * sinO;
        float worldOffsetY = slot.offsetX * sinO + slot.offsetY * cosO;

        return {
            leader.x + worldOffsetX,
            leader.y + worldOffsetY,
            leader.z,
            leader.orientation
        };
    }

    bool IsInFormation(Position const& current, FormationSlot const& slot, float tolerance = 1.0f)
    {
        Position target = CalculateWorldPosition(slot, leaderPos_);
        return Distance(current, target) <= tolerance;
    }
};

TEST_F(FormationPositioningTest, CalculatesWorldPosition)
{
    // Leader facing east (orientation = 0)
    Position pos = CalculateWorldPosition(formationSlots_[0], leaderPos_);

    EXPECT_NEAR(98.0f, pos.x, 0.01f);  // -2 X offset
    EXPECT_NEAR(98.0f, pos.y, 0.01f);  // -2 Y offset (behind)
}

TEST_F(FormationPositioningTest, RotatesWithLeader)
{
    leaderPos_.orientation = M_PI / 2.0f;  // Facing north

    Position pos = CalculateWorldPosition(formationSlots_[0], leaderPos_);

    // When facing north, local (-2, -2) rotates to world (2, -2)
    EXPECT_NEAR(102.0f, pos.x, 0.01f);
    EXPECT_NEAR(98.0f, pos.y, 0.01f);
}

TEST_F(FormationPositioningTest, InFormationCheck)
{
    Position target = CalculateWorldPosition(formationSlots_[0], leaderPos_);
    Position current = target;

    EXPECT_TRUE(IsInFormation(current, formationSlots_[0]));

    current.x += 2.0f;  // Move out of tolerance
    EXPECT_FALSE(IsInFormation(current, formationSlots_[0]));
}

/**
 * @brief Tests for combat positioning
 */
class CombatPositioningTest : public ::testing::Test
{
protected:
    struct Position
    {
        float x, y;
    };

    struct CombatContext
    {
        Position bossPos;
        float bossOrientation;
        Position tankPos;
        bool isMelee;
        bool isRanged;
        float optimalRange;
    };

    CombatContext context_;

    void SetUp() override
    {
        context_ = {
            {100.0f, 100.0f},  // Boss at center
            0.0f,              // Boss facing east
            {105.0f, 100.0f},  // Tank in front (east of boss)
            true,              // Is melee
            false,
            5.0f               // Optimal range
        };
    }

    float Distance(Position const& a, Position const& b)
    {
        float dx = a.x - b.x;
        float dy = a.y - b.y;
        return std::sqrt(dx * dx + dy * dy);
    }

    float AngleFromBoss(Position const& pos)
    {
        float dx = pos.x - context_.bossPos.x;
        float dy = pos.y - context_.bossPos.y;
        return std::atan2(dy, dx) - context_.bossOrientation;
    }

    bool IsBehindBoss(Position const& pos)
    {
        float angle = AngleFromBoss(pos);
        // Behind is roughly 120-240 degrees from front
        float absAngle = std::abs(angle);
        return absAngle > 2.0f && absAngle < 4.2f;  // ~115-240 degrees
    }

    bool IsInFront(Position const& pos)
    {
        float angle = std::abs(AngleFromBoss(pos));
        return angle < 1.0f;  // ~60 degrees
    }

    Position CalculateMeleePosition(bool preferBehind)
    {
        float angle;
        if (preferBehind)
        {
            angle = context_.bossOrientation + M_PI;  // Behind
        }
        else
        {
            angle = context_.bossOrientation + M_PI / 2.0f;  // Side
        }

        return {
            context_.bossPos.x + std::cos(angle) * context_.optimalRange,
            context_.bossPos.y + std::sin(angle) * context_.optimalRange
        };
    }

    Position CalculateRangedPosition()
    {
        // Max range, spread from others
        float angle = context_.bossOrientation + M_PI * 0.75f;  // Back-side
        return {
            context_.bossPos.x + std::cos(angle) * context_.optimalRange,
            context_.bossPos.y + std::sin(angle) * context_.optimalRange
        };
    }
};

TEST_F(CombatPositioningTest, TankIsInFront)
{
    EXPECT_TRUE(IsInFront(context_.tankPos));
    EXPECT_FALSE(IsBehindBoss(context_.tankPos));
}

TEST_F(CombatPositioningTest, CalculateBehindPosition)
{
    Position behind = CalculateMeleePosition(true);
    EXPECT_TRUE(IsBehindBoss(behind));
}

TEST_F(CombatPositioningTest, MeleeOptimalRange)
{
    Position pos = CalculateMeleePosition(true);
    float dist = Distance(pos, context_.bossPos);
    EXPECT_NEAR(context_.optimalRange, dist, 0.01f);
}

/**
 * @brief Tests for AoE avoidance
 */
class AoeAvoidanceTest : public ::testing::Test
{
protected:
    struct Position
    {
        float x, y;
    };

    struct AoeZone
    {
        Position center;
        float radius;
        uint32 durationMs;
        float damagePerSecond;
    };

    std::vector<AoeZone> aoeZones_;

    void SetUp() override
    {
        aoeZones_ = {
            {{100.0f, 100.0f}, 10.0f, 5000, 1000.0f},
            {{120.0f, 100.0f}, 8.0f, 3000, 2000.0f},
        };
    }

    float Distance(Position const& a, Position const& b)
    {
        float dx = a.x - b.x;
        float dy = a.y - b.y;
        return std::sqrt(dx * dx + dy * dy);
    }

    bool IsInAoe(Position const& pos)
    {
        for (auto const& zone : aoeZones_)
        {
            if (Distance(pos, zone.center) <= zone.radius)
                return true;
        }
        return false;
    }

    Position FindSafePosition(Position const& current, float moveSpeed, uint32 reactionTime)
    {
        if (!IsInAoe(current))
            return current;

        // Find direction away from nearest AoE
        float nearestDist = 9999.0f;
        Position nearestCenter{0, 0};

        for (auto const& zone : aoeZones_)
        {
            float dist = Distance(current, zone.center);
            if (dist < nearestDist)
            {
                nearestDist = dist;
                nearestCenter = zone.center;
            }
        }

        // Move directly away
        float dx = current.x - nearestCenter.x;
        float dy = current.y - nearestCenter.y;
        float len = std::sqrt(dx * dx + dy * dy);
        if (len < 0.01f)
        {
            dx = 1.0f;
            len = 1.0f;
        }

        float moveDistance = moveSpeed * (reactionTime / 1000.0f);

        return {
            current.x + (dx / len) * moveDistance,
            current.y + (dy / len) * moveDistance
        };
    }

    bool WillExitAoeInTime(Position const& current, float moveSpeed, AoeZone const& zone)
    {
        float distToEdge = zone.radius - Distance(current, zone.center);
        if (distToEdge <= 0)
            return true;  // Already out

        float timeToExit = distToEdge / moveSpeed;
        return timeToExit * 1000.0f < zone.durationMs;
    }
};

TEST_F(AoeAvoidanceTest, DetectsInAoe)
{
    EXPECT_TRUE(IsInAoe({100.0f, 100.0f}));
    EXPECT_TRUE(IsInAoe({105.0f, 100.0f}));
    EXPECT_FALSE(IsInAoe({150.0f, 150.0f}));
}

TEST_F(AoeAvoidanceTest, FindsSafePosition)
{
    // Start at edge of first AoE, away from second AoE
    Position unsafe{95.0f, 100.0f};  // 5 yards from center, need to move 5+ yards to escape
    Position safe = FindSafePosition(unsafe, 7.0f, 1000);  // 7 yards/sec, 1000ms = 7 yards movement

    EXPECT_FALSE(IsInAoe(safe));
}

TEST_F(AoeAvoidanceTest, AlreadySafeReturnsCurrentPosition)
{
    Position safe{150.0f, 150.0f};
    Position result = FindSafePosition(safe, 7.0f, 500);

    EXPECT_FLOAT_EQ(safe.x, result.x);
    EXPECT_FLOAT_EQ(safe.y, result.y);
}

TEST_F(AoeAvoidanceTest, CanExitInTime)
{
    Position nearEdge{109.0f, 100.0f};  // 1 yard from edge
    EXPECT_TRUE(WillExitAoeInTime(nearEdge, 7.0f, aoeZones_[0]));

    Position center{100.0f, 100.0f};  // 10 yards from edge
    // At 7 yards/sec, 10 yards takes 1.43 seconds, which is under 5s duration
    EXPECT_TRUE(WillExitAoeInTime(center, 7.0f, aoeZones_[0]));

    // Test case where you can't make it: slow movement, short duration
    // aoeZones_[1] has 3000ms duration. At 1 yard/sec, 8 yards takes 8 seconds > 3 seconds
    Position inSecondAoe{120.0f, 100.0f};  // Center of second AoE (8 yard radius)
    EXPECT_FALSE(WillExitAoeInTime(inSecondAoe, 1.0f, aoeZones_[1]));
}

/**
 * @brief Tests for spread/stack mechanics
 */
class SpreadStackTest : public ::testing::Test
{
protected:
    struct Position
    {
        float x, y;
    };

    struct GroupMember
    {
        uint32 guid;
        Position pos;
    };

    std::vector<GroupMember> group_;
    float spreadDistance_;
    float stackDistance_;

    void SetUp() override
    {
        group_ = {
            {1, {100.0f, 100.0f}},
            {2, {105.0f, 100.0f}},
            {3, {100.0f, 105.0f}},
            {4, {110.0f, 110.0f}},
        };
        spreadDistance_ = 8.0f;
        stackDistance_ = 3.0f;
    }

    float Distance(Position const& a, Position const& b)
    {
        float dx = a.x - b.x;
        float dy = a.y - b.y;
        return std::sqrt(dx * dx + dy * dy);
    }

    bool IsSpreadCorrectly(uint32 guid)
    {
        Position myPos{0, 0};
        for (auto const& m : group_)
        {
            if (m.guid == guid)
            {
                myPos = m.pos;
                break;
            }
        }

        for (auto const& m : group_)
        {
            if (m.guid == guid)
                continue;

            if (Distance(myPos, m.pos) < spreadDistance_)
                return false;
        }
        return true;
    }

    bool IsStackedCorrectly(uint32 guid, Position const& stackPoint)
    {
        for (auto const& m : group_)
        {
            if (m.guid == guid)
                return Distance(m.pos, stackPoint) <= stackDistance_;
        }
        return false;
    }

    Position CalculateSpreadPosition(uint32 guid, Position const& center)
    {
        // Find an angle that puts us away from others
        int memberIndex = 0;
        for (size_t i = 0; i < group_.size(); i++)
        {
            if (group_[i].guid == guid)
            {
                memberIndex = static_cast<int>(i);
                break;
            }
        }

        float angle = (2.0f * M_PI * memberIndex) / group_.size();
        return {
            center.x + std::cos(angle) * spreadDistance_,
            center.y + std::sin(angle) * spreadDistance_
        };
    }

    Position CalculateGroupCenter()
    {
        float sumX = 0, sumY = 0;
        for (auto const& m : group_)
        {
            sumX += m.pos.x;
            sumY += m.pos.y;
        }
        return {sumX / group_.size(), sumY / group_.size()};
    }
};

TEST_F(SpreadStackTest, DetectsNotSpread)
{
    // Members 1 and 2 are only 5 yards apart
    EXPECT_FALSE(IsSpreadCorrectly(1));
    EXPECT_FALSE(IsSpreadCorrectly(2));
}

TEST_F(SpreadStackTest, DetectsProperlySpread)
{
    // Move member 4 far away
    group_[3].pos = {150.0f, 150.0f};
    EXPECT_TRUE(IsSpreadCorrectly(4));
}

TEST_F(SpreadStackTest, StackCheck)
{
    Position stackPoint{102.0f, 102.0f};

    EXPECT_TRUE(IsStackedCorrectly(1, stackPoint));   // 2.8 yards away
    EXPECT_FALSE(IsStackedCorrectly(4, stackPoint));  // Too far
}

TEST_F(SpreadStackTest, CalculatesSpreadPositions)
{
    Position center = CalculateGroupCenter();

    // All spread positions should be spreadDistance from center
    for (auto const& m : group_)
    {
        Position spreadPos = CalculateSpreadPosition(m.guid, center);
        float dist = Distance(spreadPos, center);
        EXPECT_NEAR(spreadDistance_, dist, 0.01f);
    }
}

TEST_F(SpreadStackTest, SpreadPositionsAreSeparated)
{
    Position center = CalculateGroupCenter();

    std::vector<Position> spreadPositions;
    for (auto const& m : group_)
    {
        spreadPositions.push_back(CalculateSpreadPosition(m.guid, center));
    }

    // Check all pairs are adequately separated
    for (size_t i = 0; i < spreadPositions.size(); i++)
    {
        for (size_t j = i + 1; j < spreadPositions.size(); j++)
        {
            float dist = Distance(spreadPositions[i], spreadPositions[j]);
            EXPECT_GT(dist, spreadDistance_ * 0.5f);  // At least half spread distance apart
        }
    }
}

