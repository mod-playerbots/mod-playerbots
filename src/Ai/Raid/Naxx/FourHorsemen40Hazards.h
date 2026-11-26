/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 * Ported from reddevilnlp/mod-playerbots, commit 13492578.
 */

#ifndef PLAYERBOTS_FOURHORSEMEN40HAZARDS_H
#define PLAYERBOTS_FOURHORSEMEN40HAZARDS_H

#include <functional>
#include <optional>
#include <span>
#include <string_view>
#include <vector>

#include "MoveSplineInitArgs.h"
#include "ObjectGuid.h"
#include "Position.h"

class Player;
class PlayerbotAI;
class Unit;

struct FourHorsemenMarkSource
{
    Position location;
    float radius;
    bool isMark = true;
    ObjectGuid wrathSource{};
};

struct FourHorsemenRearCrossing
{
    bool allowed;
    Player const* healer = nullptr;
    float healRange = 0.0f;
    float secondsToArrive = 0.0f;
    bool allowDryApproachAcrossMark = false;
};

class FourHorsemen40Hazards
{
public:
    explicit FourHorsemen40Hazards(PlayerbotAI* ai) : _ai(ai) {}

    static void ObserveVoidExposure(Player& bot, bool active);

    bool EscapeVoidZones(Player& bot, Position const& corner, float cornerRadius);
    bool EscapeCornerVoidZones(Player& bot, Position const& corner, Position const& center, float cornerRadius);
    bool EscapeRearExchangeVoidZone(Player& bot, Position const* toward = nullptr);
    bool HasActiveRearExchangeExit(Player const& bot) const;
    bool IsEscapingRearExchangeVoidZone(Player& bot);
    bool HasVoidZones(Player& bot);
    bool IsSafeFromVoidZones(Player& bot, Position const& destination);
    bool IsSafeFromVoidDamage(Player& bot, Position const& position);
    bool ContinueCenterRetreat(Player& bot, bool clearingMarks, bool& crossesVoid);
    bool WalkToFixedPoint(Player& bot, Position const& destination, float arrival, bool centerward = false,
                          std::span<Position const> alternatives = {});
    bool PrepareRearExchangePath(Player& bot, Position const& destination, float arrival, Movement::PointsArray& path,
                                 FourHorsemenRearCrossing crossing, bool* crossesVoid = nullptr,
                                 std::string_view* blockReason = nullptr,
                                 std::span<FourHorsemenMarkSource const> avoid = {},
                                 std::span<Position const> lane = {});
    bool StartRearExchangePath(Player& bot, Movement::PointsArray const& path, uint32& splineId);
    bool IsFollowingRearExchangePath(Player& bot, Position const& destination, float arrival, uint32 splineId,
                                     FourHorsemenRearCrossing crossing, bool* crossesVoid = nullptr,
                                     std::string_view* blockReason = nullptr,
                                     std::span<FourHorsemenMarkSource const> avoid = {});
    bool WalkRearExchangeToPoint(Player& bot, Position const& destination, float arrival,
                                 FourHorsemenRearCrossing crossing, std::span<Position const> lane = {});
    bool HasActiveRearRenditionPath(Player const& bot) const;
    bool AvoidVoidOnRearRenditionPath(Player& bot, FourHorsemenRearCrossing crossing);
    bool WalkRear(Player& bot, Position const& destination, float arrival, bool detour = true,
                  float maxPathLength = 0.0f, std::span<FourHorsemenMarkSource const> avoid = {},
                  Position const* via = nullptr, bool requireArrival = false, std::span<Position const> lane = {});
    bool ApproachRear(Player& bot, Unit const& target, float range, float arrival, float minimumRange = 0.0f,
                      float maxPathLength = 0.0f, std::span<FourHorsemenMarkSource const> avoid = {},
                      Position* exchangeDestination = nullptr, uint32* exchangeSpline = nullptr,
                      bool allowExchangeVoidCrossing = true);
    bool KeepRearMarkSeparation(Player& bot, Unit const& source, float minimumDistance);
    bool WalkRearOutsideMark(Player& bot, Unit const& source, float minimumDistance);
    static bool CanReceiveZeliekWrath(Player const& bot, Position const& point,
                                      std::span<FourHorsemenMarkSource const> sources, float margin = 0.0f);
    bool CanAttackFrom(Player& bot, Unit const& target, Position const& position, float range,
                       std::span<FourHorsemenMarkSource const> avoid, std::span<Position const> spread,
                       float minimumRange = 0.0f, float spreadClearance = 15.0f);
    std::optional<Position> FindAttackPosition(Player& bot, Unit const& target, float range, bool ranged,
                                               std::span<FourHorsemenMarkSource const> avoid,
                                               std::span<Position const> spread, float minimumRange = 0.0f,
                                               float spreadClearance = 15.0f,
                                               std::function<bool(Position const&)> const& usable = {},
                                               std::span<Position const> preferred = {}, float arrival = 0.0f,
                                               std::span<Position const> lane = {});
    void StopWalkTo(Player& bot, Position const& destination, float arrival);
    void StopMarkRetreat(Player& bot);
    bool EscapeMarks(Player& bot, std::span<FourHorsemenMarkSource const> sources, bool seekCompleteEscape,
                     Position const* toward = nullptr, std::span<FourHorsemenMarkSource const> otherMarks = {});

private:
    enum class VoidPath
    {
        Padded,
        DryExit,
        ForcedExit
    };

    struct VoidZone
    {
        ObjectGuid guid;
        Position position;
    };

    void RefreshVoidZones(Player& bot);
    void StopRearRenditionPath(Player& bot);
    float Clearance(Position const& point) const;
    Position const* FirstBlockedZone(Player const& bot, std::span<G3D::Vector3 const> path,
                                     VoidPath mode = VoidPath::Padded) const;
    bool IsDryRearExchangeExit(Player const& bot, std::span<G3D::Vector3 const> path, VoidPath mode) const;
    std::string_view RearExchangeCrossingBlock(Player const& bot, std::span<G3D::Vector3 const> path,
                                               FourHorsemenRearCrossing crossing, bool& crossesDamage) const;
    bool FindCenterRetreatStep(Player& bot, Position const& destination, Movement::PointsArray& path);
    bool FindLanePath(Player& bot, G3D::Vector3 const& destination, std::span<Position const> lane, float arrival,
                      std::span<FourHorsemenMarkSource const> avoid, Movement::PointsArray& path, bool detour) const;
    bool FindCenterwardDetour(Player& bot, G3D::Vector3 const& from, G3D::Vector3 const& to, float arrival,
                             Movement::PointsArray& path) const;
    bool FindVoidDetour(Player& bot, G3D::Vector3 const& from, G3D::Vector3 const& to, Position const& zone,
                        std::span<FourHorsemenMarkSource const> avoid, Movement::PointsArray& path,
                        bool centerward = false, float maxPathLength = 0.0f,
                        std::span<G3D::Vector3 const> prefix = {}) const;
    bool SafeSpline(Player const& bot, uint32 splineId, std::span<FourHorsemenMarkSource const> avoid = {},
                    bool allowDeeperInside = false, float maxPathLength = 0.0f) const;
    bool Launch(Player& bot, Movement::PointsArray const& path, uint32& splineId);

    PlayerbotAI* _ai;
    std::vector<VoidZone> _voidZones;
    Position _blockedGoal;
    Position _blockedFixedGoal;
    uint32 _instanceId = 0;
    uint32 _checkedAt = 0;
    uint32 _rearSpline = 0;
    uint32 _rearLaneSpline = 0;
    uint32 _rearRetryAt = 0;
    ObjectGuid _rearSeparationSource;
    uint32 _rearSeparationSpline = 0;
    uint32 _rearSeparationRetryAt = 0;
    uint32 _voidRetryAt = 0;
    uint32 _cornerVoidSpline = 0;
    uint32 _cornerVoidRetryAt = 0;
    uint32 _rearExchangeEscapeSpline = 0;
    uint32 _rearExchangeEscapeRetryAt = 0;
    std::optional<Position> _rearExchangeEscapeToward;
    Position _rearRenditionDestination;
    float _rearRenditionArrival = 0.0f;
    uint32 _rearRenditionSpline = 0;
    bool _rearRenditionViaLane = false;
    uint32 _rearRenditionRetryAt = 0;
    uint32 _centerRetreatSpline = 0;
    uint32 _fixedPointSpline = 0;
    uint32 _fixedPointRetryAt = 0;
    bool _fixedPointCenterward = false;
    bool _blockedFixedCenterward = false;
    uint32 _markSpline = 0;
    uint32 _markRetryAt = 0;
};

#endif
