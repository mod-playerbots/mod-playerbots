/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "FourHorsemen40.h"

#include <algorithm>
#include <array>
#include <atomic>
#include <cmath>
#include <cstddef>
#include <limits>
#include <list>
#include <memory>
#include <optional>
#include <span>
#include <string_view>
#include <utility>

#include "AiFactory.h"
#include "AttackAction.h"
#include "CellImpl.h"
#include "DisableMgr.h"
#include "DruidShapeshiftActions.h"
#include "EncounterHelpers.h"
#include "FourHorsemen40Hazards.h"
#include "GenericSpellActions.h"
#include "GridNotifiersImpl.h"
#include "Group.h"
#include "InstanceScript.h"
#include "Item.h"
#include "LastMovementValue.h"
#include "Log.h"
#include "Map.h"
#include "MotionMaster.h"
#include "MoveSpline.h"
#include "MoveSplineInit.h"
#include "Multiplier.h"
#include "Naxx40Horsemen.h"
#include "ObjectAccessor.h"
#include "PathGenerator.h"
#include "PetsAction.h"
#include "PlayerbotWorldThreadProcessor.h"
#include "Playerbots.h"
#include "Spell.h"
#include "SpellAuraEffects.h"
#include "SpellAuras.h"
#include "SpellMgr.h"
#include "Strategy.h"
#include "ThreatManager.h"
#include "Timer.h"
#include "UnitScript.h"
#include "UseItemAction.h"

// Adapted from reddevilnlp/mod-playerbots at 13492578.
namespace
{
constexpr uint32 NaxxramasMap = 533;
std::string const PlanKey = "playerbots.naxx40.horsemen.plan";
std::string const CreatureCacheKey = "playerbots.naxx40.horsemen.creatures";
std::string const StanceKey = "playerbots.naxx40.horsemen.stance";
enum class Job : uint8
{
    Assign,
    VoidZone,
    Meteor,
    ClearMarks,
    RearTankCoverage,
    TankControl,
    Position,
    Attack,
    Count
};

constexpr std::array<char const*, static_cast<std::size_t>(Job::Count)> JobNames = {
    "four horsemen 40 assign",      "four horsemen 40 avoid void zone", "four horsemen 40 soak meteor",
    "four horsemen 40 clear marks", "four horsemen 40 cover rear tank", "four horsemen 40 control tank",
    "four horsemen 40 position",    "four horsemen 40 attack"};

char const* JobName(Job job) { return JobNames[static_cast<std::size_t>(job)]; }
constexpr uint32 Encounter = 12;
constexpr uint8 CornerCount = 4;
constexpr uint8 NoCorner = CornerCount;
constexpr uint8 KorthazzCorner = 0;
constexpr uint8 MograineCorner = 1;
constexpr uint8 BlaumeuxCorner = 2;
constexpr uint8 ZeliekCorner = 3;
constexpr uint8 NoSupportSlot = 5;
constexpr uint32 RotationStacks = 2;
constexpr uint32 TankSwapStacks = 6;
constexpr uint32 RearPrepareStacks = 3;
constexpr uint32 RearSwapStacks = 4;
constexpr uint32 RearMarkInterval = 12000;
constexpr uint32 EarlyRearReserveLead = 3000;
constexpr uint32 EarlyRearProbeInterval = 1000;
constexpr uint32 EarlyRearTimeout = 20000;
constexpr uint32 EarlyRearCoverageGrace = 4000;
constexpr float RearExchangePadFallback = 6.0f;
constexpr uint32 DpsRetreatStacks = 5;
constexpr uint32 BlaumeuxDpsRetreatStacks = 4;
constexpr uint32 ZeliekMeleeRetreatStacks = 3;
constexpr float ZeliekMeleeSpacing = 4.0f;
constexpr float ZeliekHealerSpacing = 8.0f;
constexpr float ZeliekOuterRangeBand = 8.0f;
constexpr uint32 SnapshotInterval = 200;
constexpr uint32 RearReliefRetryInterval = 1000;
constexpr uint32 RearReserveRouteGrace = 3000;
constexpr uint32 LocalTauntConfirmWindow = 2000;
constexpr uint32 RetryDelay = 5000;
constexpr uint32 OpeningThreatWindow = 12000;
constexpr uint32 OpeningRearHealingWindow = 45000;
constexpr uint32 FrontOpeningThreatWindow = 8000;
constexpr uint32 FrontOpeningShotWait = 900;
constexpr uint32 FrontHandoffThreatWindow = 2000;
constexpr float FrontThreatLead = 500.0f;
constexpr uint32 ITEM_SHADOW_PROTECTION = 13459;
constexpr uint32 SPELL_SHADOW_PROTECTION = 17548;
constexpr uint32 ITEM_FLASK = 13510;
constexpr uint32 SPELL_FLASK = 17626;
constexpr uint32 ITEM_FOOD = 21023;
constexpr uint32 SPELL_FOOD = 25661;
constexpr uint32 SPELL_EATING = 25660;
constexpr int32 MinimumFlaskMs = 180000;
constexpr int32 MinimumFoodMs = 180000;
constexpr uint32 TankSuppliesWaitMs = 20000;
constexpr uint32 TankFoodWaitMs = 11500;
constexpr uint32 ShadowProtectionWaitMs = 2500;
constexpr uint32 ItemRetryMs = 1000;
constexpr float CornerArrival = 2.0f;
constexpr float LocalAnchorAdjustment = 12.0f;
constexpr float BossCornerRange = 9.0f;
constexpr float AttackRange = 28.0f;
constexpr uint32 DpsRouteRetry = 750;
constexpr float DpsPositionArrival = 0.5f;
constexpr float HolyWrathClearance = 15.0f;
constexpr uint32 ZeliekHolyWrath = 28883;
constexpr float ZeliekWrathDefaultJump = 10.0f;
constexpr float ZeliekWrathPositionMargin = 1.0f;
constexpr float ZeliekRangedMinimumRange = 20.5f;
constexpr float TauntRange = 29.0f;
constexpr float PlatformExitZ = 243.5f;
constexpr float VoidEscapeCornerRadius = 6.0f;
constexpr float RearCornerRange = 14.0f;
constexpr float HealerCoverageRange = 35.0f;
constexpr float RearHealerFollowRange = 30.0f;
constexpr float RearHealerFloorTolerance = 1.0f;
constexpr float HealerCoverageMana = 10.0f;
constexpr float RearTankUrgentHealth = 85.0f;
constexpr uint32 RearHealerConfirmationWindow = 5000;
constexpr float SupportTransferRange = 40.0f;
constexpr float MeteorSoakRange = 8.0f;
constexpr uint8 MinimumMeteorSoak = 5;
constexpr uint32 KorthMeteor = 28884;
constexpr float MeteorHoldRadius = 6.75f;
constexpr float MeteorApproachRadius = 5.75f;
constexpr uint32 MeteorArrivalMarginMs = 150;
// The two rear marks have a 45-yard radius. Leave room for combat reach.
constexpr float RearMarkClearance = 48.0f;
constexpr uint32 SPELL_TAUNT = 355;
constexpr uint32 SPELL_MOCKING_BLOW = 694;
constexpr uint32 SPELL_SHOOT_RANGED_WEAPON = 3018;
constexpr uint32 SPELL_BLOODRAGE = 2687;
constexpr uint32 SPELL_DEFENSIVE_STANCE = 71;
constexpr uint32 SPELL_BATTLE_STANCE = 2457;
constexpr uint32 SPELL_BERSERKER_STANCE = 2458;
constexpr uint32 SPELL_GROWL = 6795;
constexpr uint32 SPELL_FERAL_FAERIE_FIRE = 16857;
constexpr uint32 SPELL_BEAR_FORM = 5487;
constexpr uint32 SPELL_DIRE_BEAR_FORM = 9634;
constexpr uint32 SPELL_HAND_OF_RECKONING = 62124;
constexpr uint32 SPELL_RIGHTEOUS_DEFENSE = 31789;
constexpr uint32 SPELL_HAMMER_OF_THE_RIGHTEOUS = 53595;
constexpr uint32 SPELL_RIGHTEOUS_FURY = 25780;
constexpr uint32 SPELL_DARK_COMMAND = 56222;
constexpr uint32 SPELL_FROST_PRESENCE = 48263;
constexpr uint32 SPELL_BLOOD_PRESENCE = 48266;
constexpr uint32 SPELL_UNHOLY_PRESENCE = 48265;

struct Horseman
{
    char const* name;
    uint32 data;
    uint32 entry;
    uint32 mark;
    Position corner;
};

// Individual Progression instance data keys and the final floor waypoints
// from the encounter script in the same Naxxramas WMO.
std::array<Horseman, CornerCount> const Horsemen = {{
    {"Korth'azz", 109, 351039, 28832, {2542.9f, -3015.0f, 241.35f}},
    {"Mograine", 106, 351037, 28834, {2583.9f, -2971.6f, 241.35f}},
    {"Blaumeux", 108, 351040, 28833, {2469.4f, -2947.6f, 241.28f}},
    {"Zeliek", 107, 351038, 28835, {2517.8f, -2896.6f, 241.28f}},
}};

// Both dry paths from the front exchange keep the marks apart even when
// the two riders stop within melee reach of their tanks.
std::array<Position, 2> const FrontHolderPads = {{{2534.5f, -3014.7f, 241.33f}, {2591.5f, -2964.0f, 241.3401f}}};
// Dry MMAP approaches from the rear exchange point. Try the side that walks
// around a pool at the nominal corner before the outward pads.
std::array<std::array<Position, 6>, 2> const RearHolderPads = {{
    {{{2460.92f, -2947.82f, 241.28f},
      {2466.30f, -2950.86f, 241.28f},
      {2465.27f, -2951.95f, 241.28f},
      {2463.55f, -2953.77f, 241.28f},
      {2461.14f, -2956.30f, 241.28f},
      {2469.62f, -2956.08f, 241.28f}}},
    {{{2517.95f, -2888.62f, 241.28f},
      {2526.28f, -2896.38f, 241.28f},
      {2520.90f, -2893.34f, 241.28f},
      {2521.93f, -2892.25f, 241.28f},
      {2523.65f, -2890.43f, 241.28f},
      {2525.99f, -2888.06f, 241.28f}}},
}};
std::array<uint32, CornerCount> constexpr MarkSpirits = {16778, 16775, 16776, 16777};
// The four starting spots are 19 yards from the holder. Use the last slot
// only for a reinforcement after filling the established formation.
std::array<std::array<Position, NoSupportSlot>, 2> const RearSupport = {{
    {{{2459.90f, -2931.15f, 241.28f},
      {2469.40f, -2966.60f, 241.28f},
      {2488.40f, -2947.60f, 241.28f},
      {2478.90f, -2931.15f, 241.28f},
      {2478.90f, -2964.05f, 241.28f}}},
    {{{2501.35f, -2887.10f, 241.28f},
      {2536.80f, -2896.60f, 241.28f},
      {2501.35f, -2906.10f, 241.28f},
      {2527.30f, -2913.05f, 241.28f},
      {2511.85f, -2918.82f, 241.28f}}},
}};
Position const RoomCenter(2540.0f, -2955.0f, 241.30f);
Position const TransitionCenter(2522.6848f, -2952.8777f, 245.5518f);
std::array<Position, 6> const ZeliekRaidHealPads = {
    Position(2531.0f, -2946.5f, 245.90f), Position(2528.0f, -2949.5f, 245.86f),
    Position(2532.0f, -2943.4f, 245.31f), Position(2532.5f, -2945.0f, 245.95f),
    Position(2510.0f, -2947.5f, 245.80f), Position(2530.0f, -2945.0f, 245.91f)};
std::array<Position, 4> const ZeliekRangedPads = {
    Position(2528.75f, -2923.75f, 241.42f), Position(2541.25f, -2913.75f, 241.47f),
    Position(2512.5f, -2925.0f, 241.42f), Position(2498.75f, -2917.5f, 241.42f)};
// Ground-level points at the feet of the two ramps leading to the center platform.
std::array<Position, 2> const ZeliekRangedLane = {{{2532.20f, -2937.0f, 241.62f}, {2535.0f, -2933.5f, 241.54f}}};
std::array<Position, 2> const ZeliekMeleeLane = {{{2506.0f, -2937.0f, 241.68f}, {2504.8f, -2933.0f, 241.62f}}};
std::array<Position, 2> const ZeliekWestTankPads = {{{2514.93f, -2899.94f, 241.28f}, {2515.0f, -2896.75f, 241.28f}}};
std::array<Position, 2> const RearExchangePads = {{{2484.9f, -2927.4f, 241.28f}, {2499.0f, -2914.0f, 241.28f}}};
Position const WaitingPosition(2535.0f, -2941.0f, 241.48f);
Position const OpeningCamp(2555.9f, -2996.4f, 241.4f);
Position const ZeliekOpeningHealPad(2546.0f, -2902.0f, 241.28f);
std::array<Position, 2> const FrontSupportPads = {{{2537.5f, -3015.5f, 241.34f}, {2588.0f, -2968.0f, 241.34f}}};
Position const KorthPullPoint(2516.0f, -2983.0f, 241.276f);
Position const MograinePullPoint(2538.50f, -2970.00f, 241.2943f);

struct OpeningFlank
{
    std::array<Position, 2> waypoints;
    std::array<Position, 2> healers;
    Position pull;
};

std::array<OpeningFlank, 2> const OpeningFlanks = {{
    {{{{2519.0f, -2995.0f, 241.29f}, {2484.0f, -2978.0f, 241.28f}}},
     {{{2474.0f, -2968.0f, 241.28f}, {2468.0f, -2935.0f, 241.28f}}},
     {2488.50f, -2958.25f, 241.28f}},
    {{{{2561.0f, -2947.0f, 241.28f}, {2550.0f, -2914.0f, 241.28f}}},
     {{{2550.0f, -2914.0f, 241.28f}, {2532.0f, -2916.0f, 241.28f}}},
     {2552.50f, -2940.50f, 241.28f}},
}};
constexpr float OpeningArrival = 2.5f;
constexpr float MograinePullArrival = 0.20f;
constexpr float OpeningAggroMargin = 5.0f;
constexpr float OpeningTauntRange = 30.0f;
constexpr uint8 BothFlanks = 0x3;

using Bosses = std::array<Creature*, CornerCount>;

struct OpeningMember
{
    ObjectGuid guid;
    uint8 corner;
    uint8 healerSlot;
    std::size_t waypoint = 0;
    uint32 spline = 0;
    uint32 lastMove = 0;
    bool arrived = false;
};

struct OpeningPlan
{
    ObjectGuid leader;
    std::vector<OpeningMember> members;
    OpeningMember pullHolder{};
    std::array<uint32, CornerCount> suppliesAttempt{};
    std::array<uint8, CornerCount> suppliesUsed{};
    std::array<uint32, CornerCount> eatingStarted{};
    uint32 suppliesStarted = 0;
    bool suppliesChecked = false;
    uint32 startedAt = 0;
    uint32 lastDrive = 0;
    uint32 lastReport = 0;
    uint32 pullRequestedAt = 0;
    uint8 firstTag = NoCorner;
    uint8 readyFlanks = 0;
    bool mograineReady = false;
    bool pulled = false;
};

struct RearProtection
{
    ObjectGuid tank;
    uint32 startedAt = 0;
    uint32 attemptedAt = 0;
    int32 remaining = -1;
};

struct Member
{
    ObjectGuid guid;
    uint8 home;
    uint8 supportSlot = NoSupportSlot;
    uint8 assignedCorner = NoCorner;
    uint8 assignedSlot = NoSupportSlot;
    uint8 clearingMarks = 0;
    uint8 reinforcementHome = NoCorner;
    ObjectGuid coveringTank{};
    ObjectGuid damageTarget{};
    ObjectGuid returnDamageTarget{};
    std::optional<Position> damagePosition{};
    std::optional<Position> zeliekRaidHealPosition{};
    uint8 damageMarks = 0;
    uint8 centerDestination = NoCorner;
    uint8 centerOrigin = NoCorner;
    uint8 centerMarks = 0;
    bool centerInitialized = false;
    bool centerPending = false;
    uint8 releasedTankMarks = 0;
};

struct TankHandoff
{
    ObjectGuid boss{};
    ObjectGuid incoming{};
    std::optional<Position> approach{};
    uint32 checkedAt = 0;
    uint32 tauntAt = 0;
    uint32 releasedAt = 0;
    uint32 preparedAt = 0;
    bool protectionReady = false;
    bool active = false;
    bool returning = false;
};

struct TankHealerAssignment
{
    ObjectGuid healer;
    ObjectGuid target;
};

struct EarlyRearHealerAssignment
{
    ObjectGuid healer;
    ObjectGuid tank;
    ObjectGuid preparedHealer;
    ObjectGuid reserve;
    ObjectGuid reserveTarget;
    uint32 reserveHealAt = 0;
    bool rotated = false;
    bool rotationPending = false;
    bool preparingExchange = false;
    std::optional<Position> exchangePosition;
    uint8 corner = NoCorner;
};

struct InterruptedRearTank
{
    ObjectGuid guid;
    uint32 spline = 0;
};

enum class ZeliekWrathJump : uint8
{
    Unknown,
    Blocked,
    Clear
};

struct ZeliekWrathTarget
{
    ObjectGuid guid;
    std::array<float, 6> geometry;
    std::vector<ZeliekWrathJump> jumps;
};

struct ZeliekWrathSnapshot
{
    std::vector<ZeliekWrathTarget> targets;
    float jumpRadius = 0.0f;
    uint32 maxTargets = 0;
};

struct RaidPlan : DataMap::Base
{
    ObjectGuid group;
    std::vector<Member> members;
    std::array<ObjectGuid, CornerCount> holders;
    std::array<ObjectGuid, CornerCount> openingHolders;
    std::array<TankHandoff, CornerCount> tankHandoffs;
    uint32 earlyRearSwapAt = 0;
    uint32 earlyRearMarkWindow = 0;
    uint32 earlyRearSecuredAt = 0;
    uint32 earlyRearProbeAt = 0;
    uint32 earlyRearProbeMarks = 0;
    uint32 earlyRearUncoveredAt = 0;
    std::array<uint32, 2> earlyRearSplines{};
    std::array<Position, 2> earlyRearDestinations{};
    std::array<bool, 2> earlyRearCrossings{};
    std::array<std::string_view, 2> earlyRearRouteBlocks{};
    std::array<InterruptedRearTank, 2> earlyRearInterruptedInPool{};
    std::array<uint32, 2> earlyRearReplanAt{};
    std::array<uint32, 2> earlyRearTaunts{};
    uint8 earlyRearReady = 0xff;
    uint8 earlyRearBlocked = 0;
    uint32 combatStartedAt = 0;
    uint32 sampledAt = 0;
    std::array<bool, CornerCount> reached = {};
    bool rearReleased = false;
    bool individualArmed = false;
    std::array<TankHealerAssignment, 2> rearHealing = {};
    std::array<EarlyRearHealerAssignment, 2> earlyRearHealing{};
    TankHealerAssignment mograineHealing{};
    std::array<ObjectGuid, 2> zeliekMelee;
    ZeliekWrathSnapshot zeliekWrath;
    std::size_t nextZeliekMelee = 0;
    Position zeliekMeleeWait = WaitingPosition;
    std::optional<OpeningPlan> opening;
    RearProtection rearProtection;
};

struct Assignment
{
    uint8 corner = NoCorner;
    uint8 supportSlot = NoSupportSlot;
    bool holder = false;
    bool opening = false;
    ObjectGuid targetHolder{};
    bool standby = false;
    uint8 clearingMarks = 0;
    bool individualArmed = false;
    std::optional<Position> stagedHealer{};
    bool stagedTank = false;
    bool openingThreat = false;
    bool sideHealer = false;
    ObjectGuid healingTarget{};
    uint32 frontOpeningAt = 0;
    bool frontHandoffThreat = false;
    bool managedTank = false;
    bool tankStandby = false;
    bool tankIncoming = false;
    uint8 tankReleasedMarks = 0;
    bool earlyRearSwap = false;
    bool earlyRearHealer = false;
    bool dedicatedRearHealer = false;
    bool reserveRearHealer = false;
};

struct WarriorStanceChange : DataMap::Base
{
    ShapeshiftForm original = FORM_NONE;
    uint32 instanceId = 0;
    uint32 openingStartedAt = 0;
};

RaidPlan* GetPlan(Player const& player)
{
    Map* const map = player.FindMap();
    return map ? map->CustomData.Get<RaidPlan>(PlanKey) : nullptr;
}

struct CreatureLookup
{
    ObjectGuid guid;
    uint32 sampledAt = 0;
    uint32 seenAt = 0;
};

struct CreatureCache : DataMap::Base
{
    std::array<CreatureLookup, CornerCount * 2> entries;
};

Creature* FindEncounterCreature(Player& player, uint8 slot, uint32 entry)
{
    constexpr uint32 CacheMs = 1000;
    constexpr uint32 MissingCacheMs = 3000;
    constexpr uint32 RespawnWatchMs = 15000;
    constexpr float SearchRange = 250.0f;
    Map* map = player.GetMap();
    CreatureLookup& cached = map->CustomData.GetDefault<CreatureCache>(CreatureCacheKey)->entries[slot];
    uint32 const now = getMSTime();
    bool const missing = cached.guid.IsEmpty() && (!cached.seenAt || now - cached.seenAt > RespawnWatchMs);
    if (cached.sampledAt && now - cached.sampledAt <= (missing ? MissingCacheMs : CacheMs))
        return cached.guid.IsEmpty() ? nullptr : map->GetCreature(cached.guid);

    Creature* found = nullptr;
    for (auto const& [spawnId, creature] : map->GetCreatureBySpawnIdStore())
        if (creature->GetEntry() == entry)
        {
            found = creature;
            break;
        }
    if (!found)
        found = player.FindNearestCreature(entry, SearchRange);

    cached.sampledAt = now;
    cached.guid = found ? found->GetGUID() : ObjectGuid::Empty;
    if (found)
        cached.seenAt = now;
    return found;
}

bool IsAutonomousBot(Player* player)
{
    return player && !IsRealPlayer(player) && !IsSelfBot(player) && GET_PLAYERBOT_AI(player);
}

bool CanAssign(Player* member, Player const& anchor)
{
    return member && member->IsInWorld() && !member->IsDuringRemoveFromWorld() && member->IsInMap(&anchor) &&
           member->InSamePhase(&anchor) && GET_PLAYERBOT_AI(member);
}

InstanceScript* ActiveEncounter(Player* bot)
{
    if (!Naxx40Horsemen::IsNaxx40(bot) || !bot->GetGroup() || bot->GetExactDist2d(RoomCenter) > 110.0f)
        return nullptr;
    InstanceScript* instance = bot->GetInstanceScript();
    return instance && instance->GetBossState(Encounter) == IN_PROGRESS ? instance : nullptr;
}

InstanceScript* OpeningInstance(Player* bot)
{
    if (!bot || !Naxx40Horsemen::IsNaxx40(bot) || !bot->GetGroup() || !bot->IsAlive() || bot->IsInCombat() ||
        bot->GetExactDist2d(RoomCenter) > 110.0f)
        return nullptr;
    InstanceScript* instance = bot->GetInstanceScript();
    return instance && instance->GetBossState(Encounter) == NOT_STARTED ? instance : nullptr;
}

Bosses ReadBosses(Player* bot, InstanceScript& instance)
{
    Bosses bosses = {};
    for (uint8 corner = 0; corner < CornerCount; ++corner)
    {
        Creature* boss = instance.GetCreature(Horsemen[corner].data);
        if (!boss || boss->GetEntry() != Horsemen[corner].entry)
            boss = FindEncounterCreature(*bot, corner, Horsemen[corner].entry);
        if (boss && boss->IsInWorld() && boss->IsAlive())
            bosses[corner] = boss;
    }
    return bosses;
}

bool IsTank(Player* player) { return PlayerbotAI::IsTank(player) || PlayerbotAI::IsExplicitMainTank(player); }

bool CanHold(Player* player)
{
    if (!IsAutonomousBot(player) || PlayerbotAI::IsHeal(player))
        return false;
    switch (player->getClass())
    {
        case CLASS_WARRIOR:
            return player->HasSpell(SPELL_TAUNT) && player->HasSpell(SPELL_DEFENSIVE_STANCE);
        case CLASS_PALADIN:
            return player->HasSpell(SPELL_HAND_OF_RECKONING);
        case CLASS_DRUID:
            return player->HasSpell(SPELL_GROWL) &&
                   (player->HasSpell(SPELL_DIRE_BEAR_FORM) || player->HasSpell(SPELL_BEAR_FORM));
        case CLASS_DEATH_KNIGHT:
            return player->HasSpell(SPELL_DARK_COMMAND);
        default:
            return false;
    }
}

uint32 RequiredHolderBuff(Player const& player)
{
    ShapeshiftForm const form = player.GetShapeshiftForm();
    switch (player.getClass())
    {
        case CLASS_WARRIOR:
            return form == FORM_DEFENSIVESTANCE ? 0 : SPELL_DEFENSIVE_STANCE;
        case CLASS_DRUID:
            if (form == FORM_BEAR || form == FORM_DIREBEAR)
                return 0;
            return player.HasSpell(SPELL_DIRE_BEAR_FORM) ? SPELL_DIRE_BEAR_FORM : SPELL_BEAR_FORM;
        case CLASS_PALADIN:
            return player.HasSpell(SPELL_RIGHTEOUS_FURY) && !player.HasAura(SPELL_RIGHTEOUS_FURY) ? SPELL_RIGHTEOUS_FURY
                                                                                                  : 0;
        case CLASS_DEATH_KNIGHT:
            return player.HasSpell(SPELL_FROST_PRESENCE) && !player.HasAura(SPELL_FROST_PRESENCE) ? SPELL_FROST_PRESENCE
                                                                                                  : 0;
        default:
            return 0;
    }
}

Player* PlayerInRaid(Player& bot, ObjectGuid guid)
{
    Player* player = guid.IsEmpty() ? nullptr : ObjectAccessor::GetPlayer(bot, guid);
    return CanAssign(player, bot) && player->IsAlive() && player->GetGroup() == bot.GetGroup() ? player : nullptr;
}

Player* HolderInRaid(Player& bot, ObjectGuid guid)
{
    Player* const player = PlayerInRaid(bot, guid);
    return IsAutonomousBot(player) ? player : nullptr;
}

Player* RearHealingTarget(Player const& observer, Unit* victim)
{
    Player* const player = victim ? victim->ToPlayer() : nullptr;
    return player && player->IsInWorld() && player->IsAlive() && player->GetGroup() == observer.GetGroup() &&
                   player->IsInMap(&observer) && player->InSamePhase(&observer)
               ? player
               : nullptr;
}

bool RearTankCanBeHealedBy(Player const& healer, Player const* tank)
{
    return tank && healer.GetPowerPct(POWER_MANA) >= HealerCoverageMana && !healer.HasUnitFlag(UNIT_FLAG_SILENCED) &&
           healer.IsWithinDistInMap(tank, HealerCoverageRange) && healer.IsWithinLOSInMap(tank);
}

bool CanSustainHealingCoverage(Player const* healer)
{
    return healer && healer->IsAlive() && !healer->HasSpiritOfRedemptionAura();
}

bool CanCoverZeliekRaid(Player const* healer)
{
    return CanSustainHealingCoverage(healer) && healer->GetPowerPct(POWER_MANA) >= HealerCoverageMana &&
           !healer->HasUnitFlag(UNIT_FLAG_SILENCED);
}

bool ApplyHolderBuff(PlayerbotAI& ai, uint32 spell, uint32 openingStartedAt = 0)
{
    Player* const tank = ai.GetBot();
    ShapeshiftForm const original = tank->GetShapeshiftForm();
    if (!IsAutonomousBot(tank) || !ai.CastSpell(spell, tank))
        return false;

    if (spell == SPELL_DEFENSIVE_STANCE && original != FORM_DEFENSIVESTANCE)
    {
        if (!tank->CustomData.Get<WarriorStanceChange>(StanceKey))
        {
            WarriorStanceChange& saved = *tank->CustomData.GetDefault<WarriorStanceChange>(StanceKey);
            saved.original = original;
            saved.instanceId = tank->GetInstanceId();
            saved.openingStartedAt = openingStartedAt;
        }
    }
    return true;
}

bool PrepareWarriorStances(Player& leader, RaidPlan const& plan, OpeningPlan const& opening)
{
    bool ready = true;
    for (Member const& member : plan.members)
    {
        Player* const warrior = PlayerInRaid(leader, member.guid);
        if (!IsAutonomousBot(warrior) || warrior->getClass() != CLASS_WARRIOR)
            continue;

        WarriorStanceChange const* const previous = warrior->CustomData.Get<WarriorStanceChange>(StanceKey);
        if (previous && previous->openingStartedAt != opening.startedAt)
        {
            ready = false;
            continue;
        }

        // Recover DPS from an older build or a worldserver restart without a stance record.
        if (warrior->GetShapeshiftForm() != FORM_DEFENSIVESTANCE || IsTank(warrior) ||
            std::ranges::find(plan.holders, member.guid) != plan.holders.end())
            continue;

        uint8 const spec = AiFactory::GetPlayerSpecTab(warrior);
        if (spec != WARRIOR_TAB_ARMS && spec != WARRIOR_TAB_FURY)
            continue;
        uint32 const stance = spec == WARRIOR_TAB_FURY && warrior->HasSpell(SPELL_BERSERKER_STANCE)
                                  ? SPELL_BERSERKER_STANCE
                                  : SPELL_BATTLE_STANCE;
        if (!warrior->HasSpell(stance))
            continue;

        ready = false;
        if (!warrior->IsInCombat() && GET_PLAYERBOT_AI(warrior)->CastSpell(stance, warrior))
            LOG_INFO("playerbots", "[Naxx40] Horsemen instance={} {} returned to DPS stance {} before the pull",
                     leader.GetInstanceId(), warrior->GetName(), stance);
    }
    return ready;
}

Creature const* MarkCaster(Player* bot, Bosses const& bosses, uint8 corner);
float MarkRadius(uint8 corner);
uint8 MarksToClear(Player const& player, uint32 threshold = RotationStacks, uint8 clearing = 0);
bool OnlyMograineInFront(Bosses const& bosses);
bool OnlyZeliekAlive(Bosses const& bosses);

uint8 CollectProtectedMarks(Player& bot, Bosses const& bosses, uint8 marks, std::span<FourHorsemenMarkSource> sources)
{
    uint8 count = 0;
    for (uint8 corner = 0; corner < CornerCount; ++corner)
        if (marks & (1u << corner))
            if (Creature const* caster = MarkCaster(&bot, bosses, corner); caster && caster->IsAlive())
                sources[count++] = {caster->GetPosition(), MarkRadius(corner) + CornerArrival};
    return count;
}

std::optional<Position> TankReliefPosition(Player& tank, Creature const& boss, Bosses const& bosses)
{
    PlayerbotAI* const ai = GET_PLAYERBOT_AI(&tank);
    if (!ai->CanMove())
        return std::nullopt;

    std::array<FourHorsemenMarkSource, CornerCount> marks;
    uint8 const count = CollectProtectedMarks(tank, bosses, MarksToClear(tank), marks);
    std::span const avoid(marks.data(), count);
    FourHorsemen40Hazards hazards(ai);
    if (hazards.CanAttackFrom(tank, boss, tank, TauntRange, avoid, {}))
        return tank.GetPosition();
    return hazards.FindAttackPosition(tank, boss, TauntRange, true, avoid, {});
}

struct RosterMember
{
    ObjectGuid guid;
    bool assistant;
};

struct OpeningRoster
{
    ObjectGuid mainTank;
    std::vector<RosterMember> members;
    std::atomic<bool> ready{false};
};

constexpr uint32 RosterRetryMs = 250;
constexpr uint32 RosterLifetimeMs = 1000;

class ReadOpeningRoster final : public PlayerbotOperation
{
public:
    ReadOpeningRoster(Player const& bot, ObjectGuid group, uint32 requestedAt, std::weak_ptr<OpeningRoster> roster)
        : _bot(bot.GetGUID()),
          _group(group),
          _instance(bot.GetInstanceId()),
          _requestedAt(requestedAt),
          _roster(std::move(roster))
    {
    }

    bool Execute() override
    {
        auto roster = _roster.lock();
        if (!roster || getMSTimeDiff(_requestedAt, getMSTime()) >= RosterLifetimeMs)
            return true;

        Player const* bot = ObjectAccessor::FindPlayer(_bot);
        Group const* group = bot ? bot->GetGroup() : nullptr;
        if (!group || group->GetGUID() != _group || bot->GetMapId() != NaxxramasMap ||
            bot->GetInstanceId() != _instance)
            return true;

        for (auto const& slot : group->GetMemberSlots())
            if (slot.flags & MEMBER_FLAG_MAINTANK)
            {
                roster->mainTank = slot.guid;
                break;
            }

        for (GroupReference const* ref = group->GetFirstMember(); ref; ref = ref->next())
            if (Player const* member = ref->GetSource())
                roster->members.push_back({member->GetGUID(), group->IsAssistant(member->GetGUID())});

        roster->ready.store(true, std::memory_order_release);
        return true;
    }

    std::string GetName() const override { return "FourHorsemenRoster"; }

private:
    ObjectGuid _bot;
    ObjectGuid _group;
    uint32 _instance;
    uint32 _requestedAt;
    std::weak_ptr<OpeningRoster> _roster;
};

OpeningRoster const* GetOpeningRoster(Player& bot)
{
    struct Request : DataMap::Base
    {
        ObjectGuid group;
        uint32 requestedAt = 0;
        std::shared_ptr<OpeningRoster> roster;
    };
    static std::string const key = "playerbots.naxx40.horsemen.roster";
    auto& request = *bot.GetMap()->CustomData.GetDefault<Request>(key);
    ObjectGuid const group = bot.GetGroup()->GetGUID();
    uint32 const now = getMSTime();
    uint32 const age = getMSTimeDiff(request.requestedAt, now);

    if (request.group != group || age >= RosterLifetimeMs)
        request.roster.reset();
    if (request.roster && request.roster->ready.load(std::memory_order_acquire))
    {
        // A new arrival on this map may have joined after the world snapshot.
        bool current = true;
        for (auto const& ref : bot.GetMap()->GetPlayers())
            if (Player const* member = ref.GetSource();
                member->IsInWorld() && member->GetGroup() == bot.GetGroup() &&
                std::ranges::find(request.roster->members, member->GetGUID(), &RosterMember::guid) ==
                    request.roster->members.end())
            {
                current = false;
                break;
            }
        if (current)
            return request.roster.get();
        request.roster.reset();
    }

    if (!request.roster && (!request.group || age >= RosterRetryMs))
    {
        request.group = group;
        request.requestedAt = now;
        auto roster = std::make_shared<OpeningRoster>();
        if (PlayerbotWorldThreadProcessor::instance().QueueOperation(
                std::make_unique<ReadOpeningRoster>(bot, group, now, roster)))
            request.roster = std::move(roster);
    }
    return nullptr;
}
void SelectOpeningHolders(Player& bot, RaidPlan& plan, OpeningRoster const& roster)
{
    std::vector<Player*> candidates;
    for (Member const& member : plan.members)
        if (Player* player = HolderInRaid(bot, member.guid); CanHold(player))
            candidates.push_back(player);

    ObjectGuid mainTank = roster.mainTank;
    if (!mainTank)
        for (RosterMember const& member : roster.members)
            if (Player* player = ObjectAccessor::GetPlayer(bot, member.guid);
                player && player->GetGroup() == bot.GetGroup() && player->IsAlive() && PlayerbotAI::IsTank(player))
            {
                mainTank = member.guid;
                break;
            }

    auto const isTank = [&roster](Player* player)
    { return PlayerbotAI::IsTank(player) || player->GetGUID() == roster.mainTank; };
    std::sort(
        candidates.begin(), candidates.end(),
        [&](Player* lhs, Player* rhs)
        {
            auto priority = [&](Player* player)
            {
                if (player->GetGUID() == mainTank)
                    return 0;
                if (!isTank(player))
                    return 3;
                auto const member = std::ranges::find(roster.members, player->GetGUID(), &RosterMember::guid);
                return member->assistant ? 1 : 2;
            };
            if (priority(lhs) != priority(rhs))
                return priority(lhs) < priority(rhs);
            if (!isTank(lhs) && !isTank(rhs) && (lhs->getClass() == CLASS_DRUID) != (rhs->getClass() == CLASS_DRUID))
                return lhs->getClass() == CLASS_DRUID;
            if (lhs->GetMaxHealth() != rhs->GetMaxHealth())
                return lhs->GetMaxHealth() > rhs->GetMaxHealth();
            return lhs->GetGUID() < rhs->GetGUID();
        });

    for (uint8 corner = 0; corner < CornerCount && corner < candidates.size(); ++corner)
    {
        plan.holders[corner] = candidates[corner]->GetGUID();
        auto member = std::ranges::find(plan.members, plan.holders[corner], &Member::guid);
        member->home = corner;
    }
    plan.openingHolders = plan.holders;
}

bool StartPlan(Player& bot, RaidPlan& plan, bool preparing = false)
{
    OpeningRoster const* roster = GetOpeningRoster(bot);
    if (!roster)
        return false;

    plan = {};
    Group* group = bot.GetGroup();
    plan.group = group->GetGUID();
    for (RosterMember const& entry : roster->members)
    {
        Player* member = ObjectAccessor::GetPlayer(bot, entry.guid);
        if (!member || member->GetGroup() != group)
            continue;
        if (CanAssign(member, bot) && (!preparing || (IsAutonomousBot(member) && member->IsAlive())))
            plan.members.push_back({member->GetGUID(), NoCorner});
        else if (!GET_PLAYERBOT_AI(member))
            LOG_INFO("playerbots",
                     "[Naxx40] Horsemen manual player {} at ({:.1f}, {:.1f}, {:.1f}); outside the bot formation",
                     member->GetName(), member->GetPositionX(), member->GetPositionY(), member->GetPositionZ());
    }

    SelectOpeningHolders(bot, plan, *roster);
    std::array<uint8, 3> counts = {};  // healers, ranged, melee and spare tanks
    std::array<uint8, CornerCount> cornerSizes = {};
    for (Member& member : plan.members)
    {
        if (Player* player = ObjectAccessor::GetPlayer(bot, member.guid); player && member.home == NoCorner)
        {
            if (PlayerbotAI::IsHeal(player))
            {
                uint8 const index = counts[0]++;
                member.home = index < 4 ? 2 + (index % 2) : index % 2;
                if (member.home >= 2)
                    member.supportSlot = index / 2;
            }
            else if (PlayerbotAI::IsRangedDps(player))
            {
                uint8 const index = counts[1]++;
                member.home = index < 4 ? 2 + (index % 2) : (index % 3 == 0);
                if (member.home >= 2)
                    member.supportSlot = 2 + index / 2;
            }
            else
                member.home = counts[2]++ % 3 == 2;
        }
        if (member.home < CornerCount)
            ++cornerSizes[member.home];
    }

    for (uint8 corner = 0; corner < CornerCount; ++corner)
    {
        Player* player = HolderInRaid(bot, plan.holders[corner]);
        Position const& position = Horsemen[corner].corner;
        LOG_INFO(
            "playerbots", "[Naxx40] Horsemen instance={} {} corner=({:.1f}, {:.1f}, {:.2f}) bots={} holder={} role={}",
            bot.GetInstanceId(), Horsemen[corner].name, position.GetPositionX(), position.GetPositionY(),
            position.GetPositionZ(), cornerSizes[corner], player ? player->GetName() : "none",
            player ? (PlayerbotAI::IsTank(player) || player->GetGUID() == roster->mainTank ? "tank"
                                                                                           : "taunt-capable reserve")
                   : "none");
    }
    return true;
}

void JoinLivePlan(Player& bot, RaidPlan& plan)
{
    if (std::any_of(plan.members.begin(), plan.members.end(),
                    [&bot](Member const& member) { return member.guid == bot.GetGUID(); }))
        return;

    std::array<uint32, 2> front{};
    std::array<std::array<bool, NoSupportSlot>, 2> rear{};
    for (Member const& member : plan.members)
    {
        if (member.home < 2)
            ++front[member.home];
        else if (member.home < CornerCount && member.supportSlot < NoSupportSlot)
            rear[member.home - 2][member.supportSlot] = true;
    }
    Member joined{bot.GetGUID(), uint8(front[1] < front[0])};
    joined.centerPending = true;
    bool const healer = PlayerbotAI::IsHeal(&bot);
    if (healer || PlayerbotAI::IsRangedDps(&bot))
    {
        uint8 const first = healer ? 0 : 2;
        uint8 const last = healer ? 2 : 4;
        for (uint8 slot = first; slot < last && joined.home < 2; ++slot)
            for (uint8 side = 0; side < rear.size(); ++side)
                if (!rear[side][slot])
                {
                    joined.home = 2 + side;
                    joined.supportSlot = slot;
                    break;
                }
    }
    plan.members.push_back(joined);
    plan.sampledAt = 0;
    LOG_INFO("playerbots", "[Naxx40] Horsemen instance={} {} joined the live plan at {}", bot.GetInstanceId(),
             bot.GetName(), Horsemen[joined.home].name);
}

bool StartOpening(Player& coordinator, RaidPlan& plan, uint32 now)
{
    if (!StartPlan(coordinator, plan, true))
        return false;
    Player* const firstHolder = HolderInRaid(coordinator, plan.holders[0]);
    if (!CanHold(firstHolder))
        return false;
    Player& leader = *firstHolder;
    OpeningPlan& opening = plan.opening.emplace();
    opening.leader = leader.GetGUID();
    opening.pullHolder.guid = opening.leader;
    opening.startedAt = opening.lastDrive = now;
    for (Member const& member : plan.members)
    {
        if (member.home == 0 || member.home >= CornerCount)
            continue;
        if (Player* player = PlayerInRaid(leader, member.guid); IsAutonomousBot(player))
        {
            bool const tank = plan.holders[member.home] == member.guid;
            if (tank || (member.home >= 2 && PlayerbotAI::IsHeal(player) && member.supportSlot < 2))
                opening.members.push_back({member.guid, member.home, tank ? NoSupportSlot : member.supportSlot});
        }
    }
    LOG_INFO("playerbots", "[Naxx40] Horsemen opening instance={} starts flank staging for {} bots",
             leader.GetInstanceId(), opening.members.size());
    return true;
}

bool NeedsAdditionalOpeningHealers(Player& coordinator, OpeningPlan const& opening)
{
    constexpr std::ptrdiff_t rearHealers = 4;
    if (std::ranges::count_if(opening.members, [](OpeningMember const& member)
                              { return member.healerSlot != NoSupportSlot; }) >= rearHealers)
        return false;

    std::ptrdiff_t healers = 0;
    for (auto const& ref : coordinator.GetMap()->GetPlayers())
        if (Player* member = ref.GetSource();
            CanAssign(member, coordinator) && member->GetGroup() == coordinator.GetGroup() && IsAutonomousBot(member) &&
            member->IsAlive() && PlayerbotAI::IsHeal(member) && ++healers == rearHealers)
            return true;
    return false;
}

class HorsemenTankConsumable final : public UseItemAction
{
public:
    explicit HorsemenTankConsumable(PlayerbotAI* ai) : UseItemAction(ai, "horsemen tank consumable", true) {}

    bool Use(uint32 itemId)
    {
        if (bot->isMoving() || bot->IsMounted() || bot->IsNonMeleeSpellCast(false) || !botAI->CanMove())
            return false;
        Item* item = bot->GetItemByEntry(itemId);
        if (!item || bot->HasSpellCooldown(item->GetTemplate()->Spells[0].SpellId))
            return false;
        return UseItemAuto(item);
    }
};

bool OpeningExpired(OpeningPlan const& opening, uint32 now)
{
    if (opening.suppliesStarted && !opening.suppliesChecked && now - opening.suppliesStarted < TankSuppliesWaitMs)
        return false;
    return now - opening.lastDrive > RetryDelay;
}

bool HasTimeRemaining(Player const& player, uint32 auraId, int32 minimumMs)
{
    Aura const* const aura = player.GetAura(auraId);
    return aura && (aura->IsPermanent() || aura->GetDuration() >= minimumMs);
}

bool PrepareTankSupplies(Player& leader, RaidPlan const& plan, OpeningPlan& opening, uint32 now)
{
    if (opening.suppliesChecked)
        return true;
    if (!opening.suppliesStarted)
        opening.suppliesStarted = now;

    uint8 ready = 0;
    bool pending = false;
    for (uint8 corner = 0; corner < CornerCount; ++corner)
    {
        Player* tank = HolderInRaid(leader, plan.holders[corner]);
        if (!tank)
            continue;
        bool const flask = HasTimeRemaining(*tank, SPELL_FLASK, MinimumFlaskMs);
        bool const food = HasTimeRemaining(*tank, SPELL_FOOD, MinimumFoodMs);
        if (flask && food)
        {
            ++ready;
            continue;
        }
        constexpr uint8 FlaskUsed = 1u;
        constexpr uint8 FoodUsed = 2u;
        uint8& used = opening.suppliesUsed[corner];
        bool const canFlask = !flask && !(used & FlaskUsed) && tank->HasItemCount(ITEM_FLASK, 1);
        bool const canEat =
            !food && !(used & FoodUsed) && !tank->HasAura(SPELL_EATING) && tank->HasItemCount(ITEM_FOOD, 1);
        uint32 const item = canFlask ? ITEM_FLASK : canEat ? ITEM_FOOD : 0;
        uint32& eating = opening.eatingStarted[corner];
        if (!food && tank->HasAura(SPELL_EATING) && !eating)
        {
            eating = now;
            GET_PLAYERBOT_AI(tank)->SetActionDuration(TankFoodWaitMs);
        }
        pending |= item || (!food && eating && now - eating < TankFoodWaitMs);
        uint32& last = opening.suppliesAttempt[corner];
        if (item && (!last || now - last >= ItemRetryMs))
        {
            last = now;
            uint32 const before = tank->GetItemCount(item);
            PlayerbotAI* const tankAI = GET_PLAYERBOT_AI(tank);
            HorsemenTankConsumable(tankAI).Use(item);
            if (tank->GetItemCount(item) < before)
            {
                used |= item == ITEM_FLASK ? FlaskUsed : FoodUsed;
                if (item == ITEM_FOOD)
                {
                    eating = now;
                    tankAI->SetActionDuration(TankFoodWaitMs);
                    LOG_INFO("playerbots", "[Naxx40] Horsemen instance={} {} started tank food (eating aura={})",
                             leader.GetInstanceId(), tank->GetName(), tank->HasAura(SPELL_EATING));
                }
            }
        }
    }

    if (pending && now - opening.suppliesStarted < TankSuppliesWaitMs)
        return false;
    opening.suppliesChecked = true;
    for (ObjectGuid holder : plan.holders)
        if (Player* tank = HolderInRaid(leader, holder))
            LOG_INFO("playerbots", "[Naxx40] Horsemen instance={} tank {} supplies flask={} food={} health={}/{}",
                     leader.GetInstanceId(), tank->GetName(), tank->HasAura(SPELL_FLASK), tank->HasAura(SPELL_FOOD),
                     tank->GetHealth(), tank->GetMaxHealth());
    LOG_INFO("playerbots", "[Naxx40] Horsemen instance={} tank supplies confirmed {}/{}", leader.GetInstanceId(), ready,
             CornerCount);
    return true;
}

bool ConfirmFirstShadowProtection(Player& tank, RearProtection& protection, uint32 now)
{
    if (protection.tank != tank.GetGUID())
    {
        protection = {};
        protection.tank = tank.GetGUID();
        protection.startedAt = now;
    }
    if (AuraEffect const* absorb = tank.GetAuraEffect(SPELL_SHADOW_PROTECTION, EFFECT_0))
    {
        protection.remaining = absorb->GetAmount();
        LOG_INFO("playerbots",
                 "[Naxx40] Horsemen instance={} {} Shadow Protection before first rear swap "
                 "remaining={} health={}/{} form={} usedPotion={}",
                 tank.GetInstanceId(), tank.GetName(), protection.remaining, tank.GetHealth(), tank.GetMaxHealth(),
                 uint32(tank.GetShapeshiftForm()), tank.GetLastPotionId());
        return true;
    }
    if ((!protection.attemptedAt && (!tank.HasItemCount(ITEM_SHADOW_PROTECTION, 1) || tank.GetLastPotionId() ||
                                     tank.HasSpellCooldown(SPELL_SHADOW_PROTECTION))) ||
        now - protection.startedAt >= ShadowProtectionWaitMs)
    {
        LOG_INFO("playerbots",
                 "[Naxx40] Horsemen instance={} {} first rear swap without Shadow Protection "
                 "(items={}, usedPotion={})",
                 tank.GetInstanceId(), tank.GetName(), tank.GetItemCount(ITEM_SHADOW_PROTECTION),
                 tank.GetLastPotionId());
        return true;
    }
    if (!tank.GetLastPotionId() && (!protection.attemptedAt || now - protection.attemptedAt >= ItemRetryMs))
    {
        protection.attemptedAt = now;
        HorsemenTankConsumable(GET_PLAYERBOT_AI(&tank)).Use(ITEM_SHADOW_PROTECTION);
    }
    return false;
}

void MeasureShadowProtection(Player& bot, RearProtection& protection)
{
    if (protection.remaining < 0)
        return;
    Player* tank = ObjectAccessor::GetPlayer(bot, protection.tank);
    if (!tank)
        return;
    AuraEffect const* absorb = tank->GetAuraEffect(SPELL_SHADOW_PROTECTION, EFFECT_0);
    int32 const remaining = absorb ? absorb->GetAmount() : -1;
    if (remaining == protection.remaining)
        return;
    LOG_INFO("playerbots",
             "[Naxx40] Horsemen instance={} {} Shadow Protection {} remaining={} previous={} "
             "health={}/{}",
             bot.GetInstanceId(), tank->GetName(), absorb ? "active" : "ended", remaining, protection.remaining,
             tank->GetHealth(), tank->GetMaxHealth());
    protection.remaining = remaining;
}

bool OutsideOpeningAggro(Player const& member, Bosses const& bosses, G3D::Vector3 const& from, G3D::Vector3 const& to,
                         float margin)
{
    G3D::Vector3 const leg(to.x - from.x, to.y - from.y, 0.0f);
    for (Creature const* boss : bosses)
    {
        if (!boss)
            return false;
        G3D::Vector3 const towardsBoss(boss->GetPositionX() - from.x, boss->GetPositionY() - from.y, 0.0f);
        float const legSquared = leg.squaredLength();
        float const along = legSquared > 0.0f ? std::clamp(towardsBoss.dot(leg) / legSquared, 0.0f, 1.0f) : 0.0f;
        float const clearance = boss->GetAggroRange(&member) + margin;
        if ((towardsBoss - leg * along).squaredLength() < clearance * clearance)
            return false;
    }
    return true;
}

bool WalkOpeningLeg(PlayerbotAI& ai, Player& member, Position const& destination, Bosses const& bosses,
                    OpeningMember& moving, float margin)
{
    PathGenerator path(&member);
    path.SetExcludeFlags(NAV_WATER | NAV_MAGMA | NAV_SLIME | NAV_GROUND_STEEP);
    G3D::Vector3 const end(destination.GetPositionX(), destination.GetPositionY(), destination.GetPositionZ());
    if (!path.CalculatePath(end.x, end.y, end.z) || path.GetPathType() != PATHFIND_NORMAL ||
        path.GetPath().size() < 2 || (path.GetActualEndPosition() - end).length() > 1.2f ||
        path.getPathLength() > 65.0f)
        return false;

    Movement::PointsArray vertices = path.GetPath();
    vertices.front() = {member.GetPositionX(), member.GetPositionY(), member.GetPositionZ()};
    vertices.back() = end;
    for (std::size_t i = 1; i < vertices.size(); ++i)
    {
        G3D::Vector3 const& from = vertices[i - 1];
        G3D::Vector3 const& to = vertices[i];
        if (to.z < 239.0f || to.z > PlatformExitZ || !OutsideOpeningAggro(member, bosses, from, to, margin) ||
            !member.GetMap()->isInLineOfSight(from.x, from.y, from.z + 0.85f, to.x, to.y, to.z + 0.85f,
                                              member.GetPhaseMask(), LINEOFSIGHT_ALL_CHECKS,
                                              VMAP::ModelIgnoreFlags::Nothing))
            return false;
    }

    if (member.IsSitState())
        member.SetStandState(UNIT_STAND_STATE_STAND);
    member.CastStop();
    member.GetMotionMaster()->Clear();
    Movement::MoveSplineInit init(&member);
    init.MovebyPath(vertices);
    init.SetWalk(false);
    int32 const duration = init.Launch();
    if (duration <= 0)
        return false;
    moving.spline = member.movespline->GetId();
    ai.GetAiObjectContext()
        ->GetValue<LastMovement&>("last movement")
        ->Get()
        .Set(member.GetMapId(), end.x, end.y, end.z, member.GetOrientation(),
             std::min(static_cast<float>(duration), static_cast<float>(sPlayerbotAIConfig.MaxWaitForMove)),
             MovementPriority::MOVEMENT_FORCED);
    return true;
}

bool PlaceOpeningMember(OpeningMember& moving, Player& member, Bosses const& bosses, uint32 now)
{
    OpeningFlank const* const flank = moving.corner >= 2 ? &OpeningFlanks[moving.corner - 2] : nullptr;
    Position const& final = !flank                  ? MograinePullPoint
                            : moving.healerSlot < 2 ? flank->healers[moving.healerSlot]
                                                    : flank->pull;
    float const arrival = flank ? OpeningArrival : MograinePullArrival;
    if (member.GetExactDist(final) <= arrival && member.movespline->Finalized())
        return true;
    PlayerbotAI* ai = GET_PLAYERBOT_AI(&member);
    if (!ai || !ai->CanMove() || member.IsInCombat() ||
        (!member.movespline->Finalized() && member.movespline->GetId() == moving.spline) ||
        (moving.lastMove && now - moving.lastMove < 1200))
        return false;

    moving.lastMove = now;
    if (!flank)
    {
        if (!moving.waypoint && member.GetExactDist(OpeningCamp) <= OpeningArrival)
            ++moving.waypoint;
        WalkOpeningLeg(*ai, member, moving.waypoint ? final : OpeningCamp, bosses, moving, OpeningAggroMargin);
        return false;
    }

    std::array<Position const*, 4> const route = {&OpeningCamp, &flank->waypoints[0], &flank->waypoints[1], &final};
    while (moving.waypoint < route.size() - 1 && member.GetExactDist(*route[moving.waypoint]) <= OpeningArrival)
        ++moving.waypoint;
    float const margin = moving.corner == 2 && moving.healerSlot == NoSupportSlot && moving.waypoint == route.size() - 1
                             ? 2.0f
                             : OpeningAggroMargin;
    WalkOpeningLeg(*ai, member, *route[moving.waypoint], bosses, moving, margin);
    return false;
}

bool HealsFromFlank(Player const& healer, Player const& tank, Position const& corner, bool& reachesCorner)
{
    if (healer.GetPowerPct(POWER_MANA) < HealerCoverageMana || healer.HasUnitFlag(UNIT_FLAG_SILENCED))
        return false;
    reachesCorner |= healer.GetExactDist(corner) <= HealerCoverageRange &&
                     healer.GetMap()->isInLineOfSight(
                         healer.GetPositionX(), healer.GetPositionY(), healer.GetPositionZ() + 1.5f,
                         corner.GetPositionX(), corner.GetPositionY(), corner.GetPositionZ() + 1.5f,
                         healer.GetPhaseMask(), LINEOFSIGHT_ALL_CHECKS, VMAP::ModelIgnoreFlags::Nothing);
    return healer.IsWithinDistInMap(&tank, HealerCoverageRange) && healer.IsWithinLOSInMap(&tank);
}

FourHorsemen40::Preparation DriveOpening(Player& leader, RaidPlan& plan, Bosses const& bosses, uint32 now)
{
    OpeningPlan& opening = *plan.opening;
    opening.lastDrive = now;
    if (!PrepareWarriorStances(leader, plan, opening))
        return FourHorsemen40::Preparation::Staging;
    if (!PrepareTankSupplies(leader, plan, opening, now))
        return FourHorsemen40::Preparation::Staging;
    if (std::any_of(bosses.begin(), bosses.end(), [](Creature const* boss) { return !boss || boss->IsInCombat(); }))
    {
        opening.readyFlanks = 0;
        opening.mograineReady = false;
        return FourHorsemen40::Preparation::Staging;
    }
    uint8 ready = 0;
    for (uint8 corner : {uint8(1), uint8(2), uint8(3)})
    {
        Player* tank = HolderInRaid(leader, plan.holders[corner]);
        bool tankReady = false;
        uint8 healers = 0;
        uint8 arrivedHealers = 0;
        bool reachesTank = false;
        bool reachesCorner = false;
        for (OpeningMember& moving : opening.members)
        {
            if (moving.corner != corner)
                continue;
            Player* member = PlayerInRaid(leader, moving.guid);
            bool const hadArrived = moving.arrived;
            moving.arrived =
                IsAutonomousBot(member) && !member->IsInCombat() && PlaceOpeningMember(moving, *member, bosses, now);
            if (moving.arrived && !hadArrived)
                LOG_INFO("playerbots",
                         "[Naxx40] Horsemen opening instance={} {} reached {} as {} at ({:.2f}, {:.2f}, {:.2f})",
                         leader.GetInstanceId(), member->GetName(), Horsemen[corner].name,
                         moving.healerSlot == NoSupportSlot ? "tank" : "healer", member->GetPositionX(),
                         member->GetPositionY(), member->GetPositionZ());
            if (moving.healerSlot == NoSupportSlot)
            {
                if (moving.arrived && member == tank)
                {
                    uint32 const form = RequiredHolderBuff(*tank);
                    if (form)
                        ApplyHolderBuff(*GET_PLAYERBOT_AI(tank), form, opening.startedAt);
                    tankReady = !RequiredHolderBuff(*tank) && bosses[corner]->GetExactDist(tank) <= OpeningTauntRange &&
                                tank->IsWithinLOSInMap(bosses[corner]);
                }
            }
            else
            {
                ++healers;
                if (moving.arrived && tank)
                {
                    ++arrivedHealers;
                    reachesTank |= HealsFromFlank(*member, *tank, Horsemen[corner].corner, reachesCorner);
                }
            }
        }
        if (corner == 1)
        {
            if (tankReady && !opening.mograineReady)
                LOG_INFO("playerbots", "[Naxx40] Horsemen opening instance={} Mograine ready tank={}",
                         leader.GetInstanceId(), tank->GetName());
            opening.mograineReady = tankReady;
            continue;
        }
        if (tankReady && healers == 2 && arrivedHealers == healers && reachesTank && reachesCorner)
        {
            uint8 const bit = 1u << (corner - 2);
            ready |= bit;
            if (!(opening.readyFlanks & bit))
                LOG_INFO("playerbots", "[Naxx40] Horsemen opening instance={} {} ready tank={} healers={}",
                         leader.GetInstanceId(), Horsemen[corner].name, tank->GetName(), arrivedHealers);
        }
    }
    if (ready == BothFlanks && opening.readyFlanks != BothFlanks)
        LOG_INFO("playerbots", "[Naxx40] Horsemen opening instance={} both flanks ready", leader.GetInstanceId());
    opening.readyFlanks = ready;
    bool const allReady = ready == BothFlanks && opening.mograineReady;
    if (!allReady && (!opening.lastReport || now - opening.lastReport >= 10000))
    {
        opening.lastReport = now;
        uint32 const arrived = std::count_if(opening.members.begin(), opening.members.end(),
                                             [](OpeningMember const& member) { return member.arrived; });
        LOG_INFO(
            "playerbots",
            "[Naxx40] Horsemen opening instance={} waiting for groups ({}/{} arrived, flanks {}, Mograine ready={})",
            leader.GetInstanceId(), arrived, opening.members.size(), ready, opening.mograineReady);
    }
    return allReady ? FourHorsemen40::Preparation::Ready : FourHorsemen40::Preparation::Staging;
}

struct OpeningTag
{
    Player* tank;
    Creature* boss;
    uint32 taunt;
    uint32 attack;
    uint8 corner;
};

class HorsemenOpeningShot final : public CastShootAction
{
public:
    HorsemenOpeningShot(PlayerbotAI* ai, Creature* rider) : CastShootAction(ai), _rider(rider) {}

    Unit* GetTarget() override { return _rider; }

private:
    Creature* const _rider;
};

std::array<uint8, CornerCount> constexpr OpeningTankCorners = {0, 1, 2, 3};

OpeningTag OpeningTagFor(Player& leader, RaidPlan const& plan, Bosses const& bosses, uint8 corner)
{
    OpeningTag tag{HolderInRaid(leader, plan.holders[corner]), bosses[corner], 0, 0, corner};
    if (!CanHold(tag.tank) || !tag.boss)
        return tag;
    switch (tag.tank->getClass())
    {
        case CLASS_WARRIOR:
            tag.taunt = SPELL_TAUNT;
            break;
        case CLASS_DRUID:
            tag.taunt = SPELL_GROWL;
            tag.attack = SPELL_FERAL_FAERIE_FIRE;
            break;
        case CLASS_PALADIN:
            tag.taunt = SPELL_HAND_OF_RECKONING;
            tag.attack = SPELL_HAND_OF_RECKONING;
            break;
        case CLASS_DEATH_KNIGHT:
            tag.taunt = SPELL_DARK_COMMAND;
            break;
        default:
            break;
    }
    return tag;
}

bool BuildFeralOpeningThreat(PlayerbotAI& ai, Creature& boss)
{
    Player& tank = *ai.GetBot();
    ShapeshiftForm const form = tank.GetShapeshiftForm();
    if ((form != FORM_BEAR && form != FORM_DIREBEAR) || !tank.HasSpell(SPELL_FERAL_FAERIE_FIRE) ||
        tank.HasSpellCooldown(SPELL_FERAL_FAERIE_FIRE) || tank.GetExactDist(&boss) > OpeningTauntRange ||
        !tank.IsWithinLOSInMap(&boss))
        return false;

    float const before = boss.GetThreatMgr().GetThreat(&tank);
    if (!ai.CastSpell(SPELL_FERAL_FAERIE_FIRE, &boss))
        return false;

    Unit const* victim = boss.GetThreatMgr().GetCurrentVictim();
    LOG_INFO("playerbots",
             "[Naxx40] Horsemen opening instance={} {} uses Faerie Fire on {} threat {:.0f}->{:.0f}, victim={}",
             tank.GetInstanceId(), tank.GetName(), boss.GetName(), before, boss.GetThreatMgr().GetThreat(&tank),
             victim ? victim->GetName() : "none");
    return true;
}

void StartFrontOpeningShot(OpeningTag const& tag)
{
    if (!tag.tank || !tag.boss || tag.tank->getClass() != CLASS_WARRIOR)
        return;

    Player& tank = *tag.tank;
    Creature& boss = *tag.boss;
    Item const* weapon = tank.GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_RANGED);
    SpellInfo const* shot = sSpellMgr->GetSpellInfo(SPELL_SHOOT_RANGED_WEAPON);
    if (!weapon || !shot)
        return;

    uint32 const subclass = weapon->GetTemplate()->SubClass;
    if (subclass != ITEM_SUBCLASS_WEAPON_BOW && subclass != ITEM_SUBCLASS_WEAPON_GUN &&
        subclass != ITEM_SUBCLASS_WEAPON_CROSSBOW)
        return;
    if (tank.IsWithinRange(&boss, tank.GetSpellMinRangeForTarget(&boss, shot) + tank.GetMeleeRange(&boss)) ||
        !tank.IsWithinCombatRange(&boss, tank.GetSpellMaxRangeForTarget(&boss, shot)) || !tank.IsWithinLOSInMap(&boss))
        return;

    PlayerbotAI* ai = GET_PLAYERBOT_AI(&tank);
    if (ai->CanCastSpell(SPELL_SHOOT_RANGED_WEAPON, &boss, false) && ai->CastSpell(SPELL_SHOOT_RANGED_WEAPON, &boss))
        LOG_INFO("playerbots", "[Naxx40] Horsemen opening instance={} {} readies Shoot on {}", tank.GetInstanceId(),
                 tank.GetName(), boss.GetName());
}

void FinishOpeningTags(Player& leader, RaidPlan& plan, Bosses const& bosses)
{
    OpeningPlan& opening = *plan.opening;
    if (opening.firstTag == NoCorner)
        return;
    uint8 const first = opening.firstTag;
    opening.firstTag = NoCorner;
    for (uint8 corner : OpeningTankCorners)
    {
        if (corner == first)
            continue;
        OpeningTag const tag = OpeningTagFor(leader, plan, bosses, corner);
        if (!tag.taunt || tag.boss->GetThreatMgr().IsThreatListEmpty())
            continue;
        PlayerbotAI* ai = GET_PLAYERBOT_AI(tag.tank);
        if (tag.boss->GetExactDist(tag.tank) > OpeningTauntRange || !tag.tank->IsWithinLOSInMap(tag.boss))
            continue;
        bool const cast = ai->CastSpell(tag.taunt, tag.boss);
        Unit const* victim = tag.boss->GetThreatMgr().GetCurrentVictim();
        LOG_INFO("playerbots", "[Naxx40] Horsemen opening instance={} {} provokes {} spell={} cast={} threat victim={}",
                 leader.GetInstanceId(), tag.tank->GetName(), tag.boss->GetName(), tag.taunt, cast,
                 victim ? victim->GetName() : "none");
    }

    // The opening taunts establish first contact. Normal ranged attacks give
    // the tanks time to build threat before the first healing casts land.
    for (uint8 corner : std::array<uint8, 2>{0, 1})
        if (first != corner)
            StartFrontOpeningShot(OpeningTagFor(leader, plan, bosses, corner));
    OpeningTag const zeliek = OpeningTagFor(leader, plan, bosses, ZeliekCorner);
    if (first != ZeliekCorner && zeliek.attack == SPELL_FERAL_FAERIE_FIRE)
        BuildFeralOpeningThreat(*GET_PLAYERBOT_AI(zeliek.tank), *zeliek.boss);
}

bool TauntHorseman(PlayerbotAI& ai, Unit& target)
{
    if (ai.GetBot()->GetDistance(&target) > OpeningTauntRange)
        return false;
    for (char const* taunt : {"taunt", "growl", "hand of reckoning", "dark command"})
        if (ai.CanCastSpell(taunt, &target) && ai.CastSpell(taunt, &target))
            return true;
    return false;
}

bool CanRetakeWithRighteousDefense(Unit const& victim, Creature& target)
{
    auto const& attackers = victim.getAttackers();
    return attackers.size() == 1 && attackers.contains(&target);
}

bool CastRearTaunt(PlayerbotAI& ai, Creature& boss)
{
    Player& tank = *ai.GetBot();
    auto const cast = [&](uint32 id, Unit& target)
    {
        SpellInfo const* const spell = sSpellMgr->GetSpellInfo(id);
        return spell && tank.HasSpell(id) && !tank.HasSpellCooldown(id) &&
               tank.IsWithinCombatRange(&target, tank.GetSpellMaxRangeForTarget(&target, spell)) &&
               tank.IsWithinLOSInMap(&target) && ai.CanCastSpell(id, &target) && ai.CastSpell(id, &target);
    };
    for (uint32 spell : {SPELL_TAUNT, SPELL_GROWL, SPELL_HAND_OF_RECKONING, SPELL_DARK_COMMAND})
        if (cast(spell, boss))
            return true;

    Unit* const victim = boss.GetVictim();
    return tank.getClass() == CLASS_PALADIN && victim && CanRetakeWithRighteousDefense(*victim, boss) &&
           cast(SPELL_RIGHTEOUS_DEFENSE, *victim);
}

bool IsIsolatedForHammer(Creature const& target, Bosses const& bosses)
{
    // Hammer searches ten yards per jump, including an extra jump with the glyph.
    return std::ranges::none_of(bosses, [&target](Creature const* other)
                                { return other && other != &target && target.IsWithinDist(other, 30.0f); });
}

bool IsTauntAction(Action* action)
{
    constexpr std::array<std::string_view, 9> taunts = {
        "taunt",           "growl",        "hand of reckoning", "dark command",
        "death grip",      "mocking blow", "righteous defense", "challenging shout",
        "challenging roar"};
    auto* const cast = dynamic_cast<CastSpellAction*>(action);
    return std::ranges::find(taunts, cast ? cast->getSpell() : action->getName()) != taunts.end();
}

bool OnlyMograineInFront(Bosses const& bosses) { return bosses[MograineCorner] && !bosses[KorthazzCorner]; }

bool EarlyRearPhase(RaidPlan const& plan, Bosses const& bosses)
{
    return bosses[KorthazzCorner] || bosses[MograineCorner] || plan.earlyRearSwapAt;
}

bool OnlyZeliekAlive(Bosses const& bosses)
{
    return bosses[ZeliekCorner] && !bosses[0] && !bosses[1] && !bosses[BlaumeuxCorner];
}

uint8 FirstLivingCorner(Bosses const& bosses)
{
    for (uint8 corner = 0; corner < CornerCount; ++corner)
        if (bosses[corner])
            return corner;
    return NoCorner;
}

uint8 RearFocus(Bosses const& bosses)
{
    if (bosses[KorthazzCorner] || bosses[MograineCorner])
        return NoCorner;
    if (bosses[BlaumeuxCorner])
        return BlaumeuxCorner;
    return bosses[ZeliekCorner] ? ZeliekCorner : NoCorner;
}

Creature const* MarkCaster(Player* bot, Bosses const& bosses, uint8 corner)
{
    return bosses[corner] ? bosses[corner] : FindEncounterCreature(*bot, CornerCount + corner, MarkSpirits[corner]);
}

float MarkRadius(uint8 corner) { return corner < 2 ? 55.0f : 45.0f; }

uint8 MarksToClear(Player const& player, uint32 threshold, uint8 clearing)
{
    uint8 marks = 0;
    for (uint8 index = 0; index < CornerCount; ++index)
    {
        uint32 const stacks = player.GetAuraCount(Horsemen[index].mark);
        uint8 const mask = 1u << index;
        if (stacks >= threshold || (stacks && (clearing & mask)))
            marks |= mask;
    }
    return marks;
}

uint8 DpsMarksToClear(Player& player, Bosses const& bosses, uint8 clearing)
{
    uint8 marks = MarksToClear(player, DpsRetreatStacks, clearing);
    if (RearFocus(bosses) == BlaumeuxCorner &&
        player.GetAuraCount(Horsemen[BlaumeuxCorner].mark) >= BlaumeuxDpsRetreatStacks)
        marks |= 1u << BlaumeuxCorner;
    if (OnlyZeliekAlive(bosses) && !PlayerbotAI::IsRangedDps(&player) &&
        player.GetAuraCount(Horsemen[ZeliekCorner].mark) >= ZeliekMeleeRetreatStacks)
        marks |= 1u << ZeliekCorner;
    return marks;
}

bool DpsBossControlled(RaidPlan const& plan, Creature const* boss, uint8 corner)
{
    if (!boss || !boss->IsAlive() || !boss->IsInCombat() || boss->GetPositionZ() > PlatformExitZ)
        return false;

    Player* const victim = boss->GetVictim() ? boss->GetVictim()->ToPlayer() : nullptr;
    if (!IsAutonomousBot(victim) || !victim->IsAlive())
        return false;

    ObjectGuid const owner = victim->GetGUID();
    TankHandoff const& handoff = plan.tankHandoffs[corner];
    if (corner >= BlaumeuxCorner && plan.earlyRearSwapAt)
        return owner == plan.holders[corner] || owner == plan.holders[corner ^ 1];
    return plan.holders[corner] == owner || (handoff.tauntAt && handoff.incoming == owner);
}

Position const& FrontSupportPosition(uint8 corner, Player* player)
{
    return PlayerbotAI::IsHeal(player) || PlayerbotAI::IsRangedDps(player) ? FrontSupportPads[corner]
                                                                           : FrontHolderPads[corner];
}

bool AtCorner(Creature const* boss, uint8 corner);

void StopRiderDamage(Player& player, Creature const* permitted)
{
    auto const isWrongRider = [permitted](Unit const* target)
    {
        return target && target != permitted &&
               std::ranges::any_of(Horsemen,
                                   [target](Horseman const& rider) { return rider.entry == target->GetEntry(); });
    };
    if (!permitted)
        EncounterHelpers::StopAttackingAndChanneling(&player);
    for (CurrentSpellTypes slot : {CURRENT_GENERIC_SPELL, CURRENT_CHANNELED_SPELL, CURRENT_AUTOREPEAT_SPELL})
        if (Spell const* const spell = player.GetCurrentSpell(slot);
            spell && !spell->GetSpellInfo()->IsPositive() && isWrongRider(spell->m_targets.GetUnitTarget()))
            player.InterruptSpell(slot);
    if (std::ranges::any_of(player.m_Controlled, [&isWrongRider](Unit* unit)
                            { return unit->GetCharmInfo() && unit->IsAlive() && isWrongRider(unit->GetVictim()); }))
        EncounterHelpers::CallBackPets(&player);
}

bool IsEarlyRearHealer(RaidPlan const& plan, ObjectGuid guid)
{
    return !guid.IsEmpty() &&
           std::ranges::any_of(plan.earlyRearHealing, [guid](EarlyRearHealerAssignment const& binding)
                               { return binding.healer == guid || binding.reserve == guid; });
}

bool HasDedicatedRearHealing(RaidPlan const& plan)
{
    return std::ranges::any_of(plan.earlyRearHealing, [](EarlyRearHealerAssignment const& binding)
                               { return !binding.healer.IsEmpty() && !binding.tank.IsEmpty(); });
}

Player* ResolveEarlyHealer(Player& observer, ObjectGuid guid)
{
    Player* const healer = PlayerInRaid(observer, guid);
    if (!CanSustainHealingCoverage(healer) || !IsAutonomousBot(healer) || !PlayerbotAI::IsHeal(healer) ||
        healer->HasUnitState(UNIT_STATE_STUNNED | UNIT_STATE_CONFUSED | UNIT_STATE_FLEEING) ||
        healer->GetPowerPct(POWER_MANA) < HealerCoverageMana || healer->HasUnitFlag(UNIT_FLAG_SILENCED))
        return nullptr;
    return healer;
}

std::array<char const*, 2> RearHealingSpells(Player const& healer)
{
    switch (healer.getClass())
    {
        case CLASS_DRUID:
            return {"swiftmend", "regrowth"};
        case CLASS_PALADIN:
            return {"holy shock", "flash of light"};
        case CLASS_PRIEST:
            return {"penance", "flash heal"};
        case CLASS_SHAMAN:
            return {"riptide", "lesser healing wave"};
        default:
            return {};
    }
}

bool RearHealerReadyToCast(Player& healer, Player& tank)
{
    if (healer.IsNonMeleeSpellCast(true))
        for (CurrentSpellTypes slot : {CURRENT_GENERIC_SPELL, CURRENT_CHANNELED_SPELL})
            if (Spell const* const spell = healer.GetCurrentSpell(slot);
                spell && spell->GetSpellInfo()->IsPositive() && spell->m_targets.GetUnitTarget() == &tank)
                return true;

    PlayerbotAI* const ai = GET_PLAYERBOT_AI(&healer);
    return std::ranges::any_of(RearHealingSpells(healer),
                               [&](char const* spell) { return spell && ai->CanCastSpell(spell, &tank); });
}

bool RearReserveHealedRecently(EarlyRearHealerAssignment const& binding)
{
    return binding.reserveHealAt && getMSTimeDiff(binding.reserveHealAt, getMSTime()) <= RearHealerConfirmationWindow;
}

bool IsOnRearHealingFloor(Player const& healer)
{
    return std::abs(healer.GetPositionZ() - Horsemen[BlaumeuxCorner].corner.GetPositionZ()) <= RearHealerFloorTolerance;
}

bool IsEarlyRearReplacement(RaidPlan const& plan, uint8 corner, Unit const* victim)
{
    TankHandoff const& handoff = plan.tankHandoffs[corner];
    return handoff.active && victim && victim->GetGUID() == handoff.incoming;
}

ObjectGuid EarlyRearHealingPatient(Player& observer, RaidPlan const& plan, EarlyRearHealerAssignment const& binding,
                                   Unit const* originalVictim)
{
    TankHandoff const& relief = plan.tankHandoffs[binding.corner ^ 1];
    if (relief.active && (!originalVictim || originalVictim->GetGUID() != binding.tank))
        if (Player* const incoming = HolderInRaid(observer, relief.incoming))
            return incoming->GetGUID();
    return binding.tank;
}

ObjectGuid RearHealingPatient(Player& observer, RaidPlan const& plan, Bosses const& bosses,
                              EarlyRearHealerAssignment const& binding)
{
    if (binding.corner >= CornerCount || !bosses[binding.corner])
        return binding.tank;
    if (plan.earlyRearSwapAt)
        return EarlyRearHealingPatient(observer, plan, binding, bosses[binding.corner]->GetVictim());
    Player* const victim = RearHealingTarget(observer, bosses[binding.corner]->GetVictim());
    return victim ? victim->GetGUID() : binding.tank;
}

Player* RearExchangeHealer(Player& observer, RaidPlan const& plan, Player const& tank)
{
    for (EarlyRearHealerAssignment const& binding : plan.earlyRearHealing)
    {
        if (binding.corner >= CornerCount)
            continue;
        if (plan.earlyRearSwapAt)
        {
            Creature const* const rider =
                FindEncounterCreature(observer, binding.corner, Horsemen[binding.corner].entry);
            if (EarlyRearHealingPatient(observer, plan, binding, rider ? rider->GetVictim() : nullptr) !=
                tank.GetGUID())
                continue;
        }
        else if (binding.tank != tank.GetGUID())
        {
            Creature const* const rider =
                FindEncounterCreature(observer, binding.corner, Horsemen[binding.corner].entry);
            if (!rider || !rider->IsAlive() || rider->GetVictim() != &tank)
                continue;
        }
        if (Player* const healer = ResolveEarlyHealer(observer, binding.healer);
            healer && RearTankCanBeHealedBy(*healer, &tank))
            return healer;
    }
    for (EarlyRearHealerAssignment const& binding : plan.earlyRearHealing)
    {
        if (binding.reserveTarget != tank.GetGUID())
            continue;
        if (Player* const reserve = ResolveEarlyHealer(observer, binding.reserve);
            reserve && IsOnRearHealingFloor(*reserve) && RearTankCanBeHealedBy(*reserve, &tank))
        {
            if (RearReserveHealedRecently(binding))
                return reserve;
            if (Player* const patient = tank.IsFullHealth() ? PlayerInRaid(observer, tank.GetGUID()) : nullptr;
                patient && RearHealerReadyToCast(*reserve, *patient))
                return reserve;
        }
    }
    return nullptr;
}

Player* RearHandoffHealer(Player& observer, RaidPlan const& plan, uint8 corner)
{
    if (plan.earlyRearSwapAt)
    {
        Player const* const incoming = HolderInRaid(observer, plan.tankHandoffs[corner].incoming);
        return incoming ? RearExchangeHealer(observer, plan, *incoming) : nullptr;
    }
    auto const binding = std::ranges::find(plan.earlyRearHealing, corner, &EarlyRearHealerAssignment::corner);
    if (binding == plan.earlyRearHealing.end())
        return nullptr;
    Creature* const rider = FindEncounterCreature(observer, corner, Horsemen[corner].entry);
    Player* const victim = rider && rider->IsAlive() ? RearHealingTarget(observer, rider->GetVictim()) : nullptr;
    Player* const tank = victim ? victim : HolderInRaid(observer, binding->tank);
    return tank ? RearExchangeHealer(observer, plan, *tank) : nullptr;
}

bool RearExchangeCovered(Player& observer, RaidPlan const& plan, Player const& tank)
{
    return RearExchangeHealer(observer, plan, tank) != nullptr;
}

class RearHealerConfirmationScript final : public UnitScript
{
public:
    RearHealerConfirmationScript() : UnitScript("naxx40_rear_healer_confirmation", true, {UNITHOOK_ON_HEAL}) {}

    void OnHeal(Unit* healer, Unit* receiver, uint32& gain) override
    {
        if (!gain || !healer || !receiver || receiver->GetMapId() != NaxxramasMap)
            return;
        Player* const tank = receiver->ToPlayer();
        Player* const reserve = healer->ToPlayer();
        if (!tank || !reserve || tank == reserve || !tank->IsInWorld() || !reserve->IsInWorld())
            return;
        RaidPlan* const plan = GetPlan(*tank);
        if (!plan || !ActiveEncounter(tank) || !IsOnRearHealingFloor(*reserve) || !CanSustainHealingCoverage(reserve) ||
            !RearTankCanBeHealedBy(*reserve, tank))
            return;
        for (EarlyRearHealerAssignment& binding : plan->earlyRearHealing)
            if (binding.reserve == reserve->GetGUID() && binding.reserveTarget == tank->GetGUID())
                binding.reserveHealAt = getMSTime();
    }
};

enum class RearHealerCoverage
{
    Unavailable,
    Planned,
    Live
};

RearHealerCoverage PrepareRearExchangeHealer(Player& observer, RaidPlan& plan, Player const& tank,
                                            Position const& exchangePad, Creature const* zeliek)
{
    auto const binding = std::ranges::find(plan.earlyRearHealing, tank.GetGUID(), &EarlyRearHealerAssignment::tank);
    if (binding == plan.earlyRearHealing.end())
        return RearHealerCoverage::Unavailable;
    binding->exchangePosition.reset();
    binding->preparedHealer.Clear();
    binding->preparingExchange = false;

    Player* const healer = RearExchangeHealer(observer, plan, tank);
    if (!healer)
        return RearHealerCoverage::Unavailable;

    constexpr float followDistance = 25.0f;
    float const rearBearing = exchangePad.GetAbsoluteAngle(&tank);
    Unit const* const wrathTarget = zeliek ? zeliek->GetVictim() : nullptr;
    FourHorsemenMarkSource const wrath = {wrathTarget ? wrathTarget->GetPosition() : Position{}, HolyWrathClearance,
                                          false};
    std::span const avoid(&wrath, wrathTarget ? 1u : 0u);
    FourHorsemen40Hazards hazards(GET_PLAYERBOT_AI(healer));
    if (healer->GetExactDist(exchangePad) <= HealerCoverageRange &&
        (!wrathTarget || healer->GetExactDist2d(wrathTarget) >= HolyWrathClearance) &&
        healer->GetExactDist2d(exchangePad) >= HolyWrathClearance &&
        healer->IsWithinLOS(exchangePad.GetPositionX(), exchangePad.GetPositionY(), exchangePad.GetPositionZ()) &&
        hazards.IsSafeFromVoidZones(*healer, healer->GetPosition()))
        return RearHealerCoverage::Live;

    Movement::PointsArray path;
    std::optional<Position> plannedStation;
    for (int step : {0, 1, -1, 2, -2})
    {
        float const angle = rearBearing + float(step) * float(M_PI) / 4.0f;
        float const x = exchangePad.GetPositionX() + std::cos(angle) * followDistance;
        float const y = exchangePad.GetPositionY() + std::sin(angle) * followDistance;
        float const z = healer->GetMapHeight(x, y, exchangePad.GetPositionZ() + CornerArrival, true, 5.0f);
        if (z <= INVALID_HEIGHT ||
            !healer->GetMap()->isInLineOfSight(x, y, z + 1.5f, exchangePad.GetPositionX(), exchangePad.GetPositionY(),
                                              exchangePad.GetPositionZ() + 1.5f, healer->GetPhaseMask(),
                                              LINEOFSIGHT_ALL_CHECKS, VMAP::ModelIgnoreFlags::Nothing))
            continue;
        Position const station(x, y, z);
        if (!plan.earlyRearSwapAt &&
            (station.GetExactDist(tank) > RearHealerFollowRange ||
             !tank.IsWithinLOS(station.GetPositionX(), station.GetPositionY(), station.GetPositionZ())))
            continue;
        if (!hazards.IsSafeFromVoidZones(*healer, station) ||
            (wrathTarget && station.GetExactDist2d(wrathTarget) < HolyWrathClearance))
            continue;
        bool crossesPool = false;
        if (!(healer->GetExactDist(station) <= CornerArrival &&
              hazards.IsSafeFromVoidZones(*healer, healer->GetPosition())) &&
            !hazards.PrepareRearExchangePath(*healer, station, CornerArrival, path, {true}, &crossesPool, nullptr,
                                             avoid))
            continue;
        if (!plannedStation || !crossesPool)
            plannedStation = station;
        if (!crossesPool)
            break;
    }
    if (!plannedStation)
        return RearHealerCoverage::Unavailable;
    binding->exchangePosition = plannedStation;
    binding->preparedHealer = healer->GetGUID();
    binding->preparingExchange = !plan.earlyRearSwapAt;
    return RearHealerCoverage::Planned;
}

uint8 EarlyHealerHighMarks(Player& healer, Bosses const& bosses)
{
    uint8 exposed = 0;
    bool highStacks = false;
    for (uint8 corner = 0; corner < CornerCount; ++corner)
    {
        uint32 const stacks = healer.GetAuraCount(Horsemen[corner].mark);
        Creature const* const caster = MarkCaster(&healer, bosses, corner);
        if (stacks < RotationStacks || !caster || healer.GetExactDist(caster) >= MarkRadius(corner) + CornerArrival)
            continue;
        exposed |= 1u << corner;
        highStacks |= stacks > RotationStacks;
    }
    return highStacks || (exposed & (exposed - 1)) ? exposed : 0;
}

bool IsRearSpareHealer(Member const& member, RaidPlan const& plan, Bosses const& bosses)
{
    if (member.home == NoCorner || IsEarlyRearHealer(plan, member.guid) || plan.mograineHealing.healer == member.guid)
        return false;
    for (uint8 corner : {KorthazzCorner, MograineCorner})
        if (bosses[corner] &&
            (member.home == corner || member.assignedCorner == corner || member.reinforcementHome == corner ||
             (!member.coveringTank.IsEmpty() && member.coveringTank == plan.holders[corner])))
            return false;
    return !member.centerPending && !member.centerMarks;
}

void UpdateEarlyRearHealing(Player& observer, RaidPlan& plan, Bosses const& bosses)
{
    if (!bosses[BlaumeuxCorner] && !bosses[ZeliekCorner])
    {
        plan.earlyRearHealing = {};
        return;
    }
    if (!plan.rearReleased || (bosses[BlaumeuxCorner] && !plan.reached[BlaumeuxCorner]) ||
        (bosses[ZeliekCorner] && !plan.reached[ZeliekCorner]))
        return;

    // Preserve the healer by tank when the two holders exchange corners.
    for (EarlyRearHealerAssignment& binding : plan.earlyRearHealing)
    {
        if (binding.tank.IsEmpty())
            continue;
        if (bosses[BlaumeuxCorner] && binding.tank == plan.holders[BlaumeuxCorner])
            binding.corner = BlaumeuxCorner;
        else if (bosses[ZeliekCorner] && binding.tank == plan.holders[ZeliekCorner])
            binding.corner = ZeliekCorner;
        else
        {
            binding.tank.Clear();
            binding.preparedHealer.Clear();
            binding.exchangePosition.reset();
        }
    }
    for (EarlyRearHealerAssignment& binding : plan.earlyRearHealing)
        if (binding.tank.IsEmpty() && binding.corner < CornerCount &&
            (!bosses[binding.corner] || std::ranges::any_of(
                                            plan.earlyRearHealing, [&](auto const& other)
                                            { return other.corner == binding.corner && !other.tank.IsEmpty(); })))
            binding = {};

    for (uint8 corner : {BlaumeuxCorner, ZeliekCorner})
    {
        Player* const tank = HolderInRaid(observer, plan.holders[corner]);
        if (!bosses[corner] || !tank)
            continue;

        auto binding = std::ranges::find(plan.earlyRearHealing, tank->GetGUID(), &EarlyRearHealerAssignment::tank);
        if (binding != plan.earlyRearHealing.end() && PlayerInRaid(observer, binding->healer))
            continue;
        if (binding == plan.earlyRearHealing.end())
            binding = std::ranges::find(plan.earlyRearHealing, corner, &EarlyRearHealerAssignment::corner);
        if (binding == plan.earlyRearHealing.end())
            binding = std::ranges::find_if(plan.earlyRearHealing,
                                           [](EarlyRearHealerAssignment const& row) { return row.corner == NoCorner; });
        if (binding == plan.earlyRearHealing.end())
            continue;

        bool const hasHealer = PlayerInRaid(observer, binding->healer) != nullptr;
        if (!hasHealer)
        {
            Player* selected = nullptr;
            int selectedPriority = -1;
            for (Member const& member : plan.members)
            {
                if (!IsRearSpareHealer(member, plan, bosses))
                    continue;
                Player* const healer = ResolveEarlyHealer(observer, member.guid);
                if (!healer)
                    continue;
                int const priority = (RearTankCanBeHealedBy(*healer, tank) ? 4 : 0) +
                                     (plan.rearHealing[corner - BlaumeuxCorner].healer == member.guid ? 2 : 0) +
                                     (member.home == corner ? 1 : 0);
                if (priority > selectedPriority)
                {
                    selected = healer;
                    selectedPriority = priority;
                }
            }
            binding->healer = selected ? selected->GetGUID() : ObjectGuid::Empty;
            if (binding->healer.IsEmpty() && binding->tank == tank->GetGUID())
                continue;
        }
        binding->corner = corner;
        binding->tank = tank->GetGUID();
        LOG_INFO("playerbots", "[Naxx40] Horsemen instance={} rear healer {} assigned to {}",
                 observer.GetInstanceId(), binding->healer.ToString(), tank->GetName());
    }

    bool const early = EarlyRearPhase(plan, bosses);
    uint32 highestMarks = 0;
    for (uint8 corner : {BlaumeuxCorner, ZeliekCorner})
        if (Player* const holder = HolderInRaid(observer, plan.holders[corner]))
            highestMarks = std::max(highestMarks, holder->GetAuraCount(Horsemen[corner].mark));

    Unit const* const wrathVictim = bosses[ZeliekCorner] ? bosses[ZeliekCorner]->GetVictim() : nullptr;
    for (EarlyRearHealerAssignment& binding : plan.earlyRearHealing)
    {
        if (binding.corner >= CornerCount || binding.healer.IsEmpty())
            continue;
        ObjectGuid const patient = RearHealingPatient(observer, plan, bosses, binding);
        Player* const tank = PlayerInRaid(observer, patient);
        if (!tank)
            continue;
        Player* const healer = ResolveEarlyHealer(observer, binding.healer);
        bool const inCoverage = healer && RearTankCanBeHealedBy(*healer, tank);
        bool const needsRelief = !inCoverage || EarlyHealerHighMarks(*healer, bosses);
        uint32 const marks = early ? highestMarks : tank->GetAuraCount(Horsemen[binding.corner].mark);
        if (marks < RearPrepareStacks && !plan.earlyRearSwapAt)
            binding.rotated = false;
        binding.rotationPending |=
            !binding.rotated &&
            (plan.earlyRearSwapAt || (!early && (marks >= RearSwapStacks || plan.tankHandoffs[binding.corner].active)));
        bool const preparing = binding.rotationPending || (!binding.rotated && marks >= RearPrepareStacks);

        if (!PlayerInRaid(observer, binding.reserve))
        {
            binding.reserve.Clear();
            binding.reserveTarget.Clear();
            binding.reserveHealAt = 0;
            Player* closest = nullptr;
            for (Member const& member : plan.members)
                if (IsRearSpareHealer(member, plan, bosses))
                    if (Player* const candidate = ResolveEarlyHealer(observer, member.guid);
                        candidate && !MarksToClear(*candidate, 1) &&
                        (!closest ||
                         candidate->GetExactDist(TransitionCenter) < closest->GetExactDist(TransitionCenter)))
                        closest = candidate;
            if (closest)
            {
                binding.reserve = closest->GetGUID();
                LOG_INFO("playerbots", "[Naxx40] Horsemen instance={} {} is healer reserve for {}",
                         observer.GetInstanceId(), closest->GetName(), tank->GetName());
            }
        }

        Player* const reserve = ResolveEarlyHealer(observer, binding.reserve);
        bool const alreadyFollowing = binding.reserveTarget == patient;
        ObjectGuid const target =
            reserve && (preparing || needsRelief) &&
                    (alreadyFollowing ? !EarlyHealerHighMarks(*reserve, bosses) : !MarksToClear(*reserve, 1))
                ? patient
                : ObjectGuid::Empty;
        if (target != binding.reserveTarget)
        {
            binding.reserveTarget = target;
            binding.reserveHealAt = 0;
        }
        if (!reserve || target.IsEmpty() || (!needsRelief && !binding.rotationPending) ||
            !IsOnRearHealingFloor(*reserve) || reserve->GetExactDist(tank) > RearHealerFollowRange ||
            !RearTankCanBeHealedBy(*reserve, tank) ||
            (wrathVictim && reserve->GetExactDist2d(wrathVictim) < HolyWrathClearance) ||
            !(RearReserveHealedRecently(binding) || (tank->IsFullHealth() && RearHealerReadyToCast(*reserve, *tank))))
            continue;

        uint8 const destination = plan.earlyRearSwapAt ? binding.corner ^ 1 : binding.corner;
        Position const& corner = Horsemen[destination].corner;
        FourHorsemenMarkSource const wrath = {wrathVictim ? wrathVictim->GetPosition() : Position{}, HolyWrathClearance,
                                              false};
        std::span const avoid(&wrath, wrathVictim ? 1u : 0u);
        FourHorsemen40Hazards hazards(GET_PLAYERBOT_AI(reserve));
        if (!hazards.IsSafeFromVoidDamage(*reserve, reserve->GetPosition()))
            continue;
        Movement::PointsArray path;
        std::optional<Position> station;
        for (Position const& pad : RearSupport[destination - BlaumeuxCorner])
        {
            if (pad.GetExactDist2d(corner) < HolyWrathClearance || pad.GetExactDist(corner) > RearHealerFollowRange ||
                (wrathVictim && pad.GetExactDist2d(wrathVictim) < HolyWrathClearance) ||
                !reserve->GetMap()->isInLineOfSight(pad.GetPositionX(), pad.GetPositionY(), pad.GetPositionZ() + 1.5f,
                                                    corner.GetPositionX(), corner.GetPositionY(),
                                                    corner.GetPositionZ() + 1.5f, reserve->GetPhaseMask(),
                                                    LINEOFSIGHT_ALL_CHECKS, VMAP::ModelIgnoreFlags::Nothing))
                continue;
            if (hazards.IsSafeFromVoidZones(*reserve, pad) &&
                (reserve->GetExactDist(pad) <= CornerArrival ||
                 hazards.PrepareRearExchangePath(*reserve, pad, CornerArrival, path, {true}, nullptr, nullptr, avoid)))
            {
                station = pad;
                break;
            }
        }
        if (!station)
            continue;

        std::swap(binding.healer, binding.reserve);
        binding.reserveTarget.Clear();
        binding.reserveHealAt = 0;
        binding.rotated = true;
        binding.rotationPending = false;
        binding.preparingExchange = false;
        binding.preparedHealer = binding.healer;
        binding.exchangePosition = station;
        LOG_INFO("playerbots", "[Naxx40] Horsemen instance={} {} takes over healing {} toward {}",
                 observer.GetInstanceId(), reserve->GetName(), tank->GetName(), Horsemen[destination].name);
    }
}

void AllocateSupport(Player& bot, RaidPlan& plan, Bosses const& bosses)
{
    struct Candidate
    {
        Member& member;
        Player* player;
        uint8 homeCorner;
        uint8 corner = NoCorner;
        uint8 slot = NoSupportSlot;
        bool healer = false;
        uint8 marks = 0;
        bool leaveForMarks = false;
        bool authorizedToLeave = false;
        bool earlyRearHealer = false;
        bool earlyFrontSupport = false;
        bool urgentMarks = false;
        bool safeRearHealer = false;
        std::array<bool, 2> canHealRear{};
    };
    std::vector<Candidate> support;
    support.reserve(plan.members.size());
    std::array<uint8, CornerCount> healerCount = {};
    std::array<std::array<uint8, NoSupportSlot>, CornerCount> occupiedSlots = {};
    std::array<bool, CornerCount> liveHolder = {};
    std::array<Player*, CornerCount> tanks = {};
    std::array<Creature const*, CornerCount> casters = {};
    bool const dedicatedRearHealing = HasDedicatedRearHealing(plan);
    for (uint8 corner = 0; corner < CornerCount; ++corner)
    {
        if (bosses[corner])
            tanks[corner] = HolderInRaid(bot, plan.holders[corner]);
        liveHolder[corner] = tanks[corner] != nullptr;
        casters[corner] = MarkCaster(&bot, bosses, corner);
    }

    std::array<Player*, 2> rearVictims{};
    for (uint8 corner : {BlaumeuxCorner, ZeliekCorner})
        if (Creature const* rider = bosses[corner])
        {
            Player* target = RearHealingTarget(bot, rider->GetVictim());
            rearVictims[corner - BlaumeuxCorner] = target ? target : tanks[corner];
        }
    if (rearVictims[0] && rearVictims[0] == rearVictims[1])
        rearVictims[1] = tanks[ZeliekCorner] != rearVictims[0] ? tanks[ZeliekCorner] : tanks[BlaumeuxCorner];
    Player* const mograineVictim =
        OnlyMograineInFront(bosses) ? RearHealingTarget(bot, bosses[MograineCorner]->GetVictim()) : nullptr;

    auto coveredTank = [&](uint8 corner) -> Player*
    {
        if (corner == MograineCorner && mograineVictim)
            return mograineVictim;
        if (corner >= CornerCount)
            return nullptr;
        return corner < BlaumeuxCorner ? tanks[corner] : rearVictims[corner - BlaumeuxCorner];
    };

    auto release = [&](Candidate& candidate)
    {
        if (candidate.corner != NoCorner)
        {
            healerCount[candidate.corner] -= candidate.healer;
            if (candidate.slot < NoSupportSlot)
                --occupiedSlots[candidate.corner][candidate.slot];
        }
        candidate.corner = NoCorner;
        candidate.slot = NoSupportSlot;
    };
    auto occupy = [&](Candidate& candidate, uint8 corner, uint8 slot)
    {
        release(candidate);
        candidate.corner = corner;
        candidate.slot = slot;
        healerCount[corner] += candidate.healer;
        if (slot < NoSupportSlot)
            ++occupiedSlots[corner][slot];
    };

    for (Member& member : plan.members)
    {
        Player* player = PlayerInRaid(bot, member.guid);
        if (!CanSustainHealingCoverage(player))
        {
            member.coveringTank.Clear();
            continue;
        }
        if (member.home == NoCorner || std::ranges::find(plan.holders, member.guid) != plan.holders.end() ||
            std::ranges::find(plan.openingHolders, member.guid) != plan.openingHolders.end() ||
            std::ranges::find(plan.tankHandoffs, member.guid, &TankHandoff::incoming) != plan.tankHandoffs.end() ||
            (dedicatedRearHealing && IsEarlyRearHealer(plan, member.guid)))
            continue;

        if (dedicatedRearHealing && !member.coveringTank.IsEmpty() &&
            (member.coveringTank == plan.holders[BlaumeuxCorner] || member.coveringTank == plan.holders[ZeliekCorner]))
            member.coveringTank.Clear();
        Candidate& candidate = support.emplace_back(Candidate{member, player, member.home});
        candidate.healer = PlayerbotAI::IsHeal(player);
    }

    auto freeSlot = [&](uint8 corner, bool healer)
    {
        if (corner >= 2)
            for (uint8 slot = healer ? 0 : 2; slot < (healer ? 2 : NoSupportSlot); ++slot)
                if (!occupiedSlots[corner][slot])
                    return slot;
        return NoSupportSlot;
    };

    bool const rearHealersCanAnchor = !dedicatedRearHealing &&
                                     (plan.rearReleased || (rearVictims[0] && !tanks[BlaumeuxCorner]) ||
                                      (rearVictims[1] && !tanks[ZeliekCorner]));
    if (dedicatedRearHealing)
        plan.rearHealing = {};
    if (rearHealersCanAnchor)
    {
        auto const previous = plan.rearHealing;
        plan.rearHealing = {};
        std::array<int, 2> available{};
        for (Candidate& candidate : support)
        {
            if (!candidate.healer || candidate.player->GetPowerPct(POWER_MANA) < HealerCoverageMana ||
                candidate.player->HasUnitFlag(UNIT_FLAG_SILENCED))
                continue;
            uint8 doubled = 0;
            candidate.safeRearHealer =
                !candidate.member.clearingMarks &&
                std::ranges::none_of(Horsemen,
                                     [&](Horseman const& rider)
                                     {
                                         uint32 const stacks = candidate.player->GetAuraCount(rider.mark);
                                         return stacks > RotationStacks || (stacks == RotationStacks && ++doubled >= 2);
                                     });
            for (std::size_t side = 0; side < rearVictims.size(); ++side)
            {
                candidate.canHealRear[side] = RearTankCanBeHealedBy(*candidate.player, rearVictims[side]);
                if (candidate.canHealRear[side])
                    ++available[side];
            }
        }

        // Preserve existing coverage before sending anyone to an uncovered victim.
        std::array<uint8, 2> order = {0, 1};
        if (available[1] < available[0])
            std::swap(order[0], order[1]);
        for (bool const ready : {true, false})
            for (uint8 side : order)
            {
                Player const* target = rearVictims[side];
                if (!target || !plan.rearHealing[side].healer.IsEmpty())
                    continue;
                uint8 const corner = BlaumeuxCorner + side;
                uint8 const other = side ^ 1;
                auto const incumbent = std::ranges::find(previous, target->GetGUID(), &TankHealerAssignment::target);
                ObjectGuid const otherTank = rearVictims[other] ? rearVictims[other]->GetGUID() : ObjectGuid::Empty;
                auto const otherIncumbent = std::ranges::find(previous, otherTank, &TankHealerAssignment::target);
                Candidate* choice = nullptr;
                int best = std::numeric_limits<int>::max();
                float closest = std::numeric_limits<float>::max();
                for (Candidate& candidate : support)
                {
                    if (!candidate.healer || candidate.corner != NoCorner ||
                        candidate.player->GetPowerPct(POWER_MANA) < HealerCoverageMana ||
                        candidate.player->HasUnitFlag(UNIT_FLAG_SILENCED))
                        continue;
                    bool const stays = incumbent != previous.end() && incumbent->healer == candidate.member.guid;
                    bool const arrived = candidate.canHealRear[side];
                    if (arrived != ready || (!candidate.safeRearHealer && !stays && !arrived))
                        continue;
                    if (available[other] == 1 && plan.rearHealing[other].healer.IsEmpty() &&
                        otherIncumbent != previous.end() && otherIncumbent->healer == candidate.member.guid &&
                        candidate.canHealRear[other])
                        continue;
                    int const priority = candidate.safeRearHealer ? (stays ? 0 : 1) : 2;
                    float const distance = candidate.player->GetExactDist(target);
                    if (priority < best || (priority == best && distance < closest))
                    {
                        choice = &candidate;
                        best = priority;
                        closest = distance;
                    }
                }
                if (!choice)
                    continue;

                auto const& pads = RearSupport[side];
                uint8 const slot =
                    choice->player->GetExactDist(pads[0]) <= choice->player->GetExactDist(pads[1]) ? 0 : 1;
                occupy(*choice, corner, slot);
                choice->member.coveringTank.Clear();
                plan.rearHealing[side] = {choice->member.guid, target->GetGUID()};
                if (incumbent == previous.end() || incumbent->healer != choice->member.guid)
                    LOG_INFO("playerbots", "[Naxx40] Horsemen instance={} healer {} assigned to {} on {}",
                             bot.GetInstanceId(), choice->player->GetName(), target->GetName(), Horsemen[corner].name);
            }
    }
    ObjectGuid const previousMograineHealer = plan.mograineHealing.healer;
    plan.mograineHealing = {};
    if (mograineVictim)
    {
        Candidate* choice = nullptr;
        int best = std::numeric_limits<int>::max();
        float closest = std::numeric_limits<float>::max();
        for (Candidate& candidate : support)
        {
            if (!candidate.healer || candidate.corner != NoCorner ||
                candidate.player->GetPowerPct(POWER_MANA) < HealerCoverageMana ||
                candidate.player->HasUnitFlag(UNIT_FLAG_SILENCED))
                continue;
            bool const ready =
                !candidate.player->isMoving() && RearTankCanBeHealedBy(*candidate.player, mograineVictim);
            bool const stays = candidate.member.guid == previousMograineHealer;
            if (candidate.member.centerMarks || candidate.member.centerPending ||
                (!ready && candidate.member.clearingMarks))
                continue;
            int const priority = (ready ? 0 : 2) + (stays ? 0 : 1);
            float const distance = candidate.player->GetExactDist(mograineVictim);
            if (priority < best || (priority == best && distance < closest))
            {
                choice = &candidate;
                best = priority;
                closest = distance;
            }
        }
        if (choice)
        {
            occupy(*choice, MograineCorner, NoSupportSlot);
            choice->member.coveringTank.Clear();
            plan.mograineHealing = {choice->member.guid, mograineVictim->GetGUID()};
        }
    }

    auto holdsTankSide = [&](Candidate const& candidate)
    {
        if (candidate.corner == MograineCorner)
            return plan.mograineHealing.healer == candidate.member.guid;
        return candidate.corner >= BlaumeuxCorner && candidate.corner < CornerCount &&
               plan.rearHealing[candidate.corner - BlaumeuxCorner].healer == candidate.member.guid;
    };

    // Keep tank coverage until the holder leaves or both of their riders fall.
    for (Candidate& candidate : support)
    {
        ObjectGuid const covering = candidate.member.coveringTank;
        if (covering.IsEmpty())
            continue;
        auto const home = std::find(plan.holders.begin(), plan.holders.end(), covering);
        uint8 const homeIndex = static_cast<uint8>(home - plan.holders.begin());
        if (home == plan.holders.end() || !PlayerInRaid(bot, covering) ||
            (!bosses[homeIndex] && !bosses[homeIndex ^ 1]))
        {
            LOG_INFO("playerbots", "[Naxx40] Horsemen instance={} healer {} released from former tank coverage",
                     bot.GetInstanceId(), candidate.player->GetName());
            candidate.member.coveringTank.Clear();
            continue;
        }
        uint8 const corner = homeIndex;
        uint8 slot = freeSlot(corner, true);
        if (!plan.individualArmed && corner >= 2)
        {
            if (candidate.member.assignedSlot < NoSupportSlot && !occupiedSlots[corner][candidate.member.assignedSlot])
                slot = candidate.member.assignedSlot;
            if (slot == NoSupportSlot)
                slot = freeSlot(corner, false);
        }
        if (corner < 2 || slot < NoSupportSlot)
            occupy(candidate, corner, slot);
    }
    for (Candidate& candidate : support)
    {
        uint8 const corner = candidate.homeCorner;
        uint8 const receivingHome = candidate.member.reinforcementHome;
        if (receivingHome != NoCorner && !bosses[receivingHome] && !bosses[receivingHome ^ 1])
            candidate.member.reinforcementHome = NoCorner;
        if (candidate.corner != NoCorner || !bosses[corner])
            continue;
        if (candidate.member.reinforcementHome != NoCorner)
            continue;
        uint8 slot = candidate.member.supportSlot;
        if (corner >= 2 && slot < NoSupportSlot && occupiedSlots[corner][slot])
            slot = plan.individualArmed ? freeSlot(corner, candidate.healer) : NoSupportSlot;
        if (corner < 2 || slot < NoSupportSlot)
            occupy(candidate, corner, slot);
    }

    // A reinforcement follows the receiving group's rotation. On the Zeliek
    // side it needs an individual ranged/healer position to join the fight.
    uint8 sharedBlaumeuxRanged = 0;
    for (Candidate& candidate : support)
    {
        uint8 const home = candidate.member.reinforcementHome;
        if (home == NoCorner || candidate.corner != NoCorner)
            continue;
        uint8 const corner = home;
        if (!liveHolder[corner])
            continue;
        if (corner < 2)
        {
            occupy(candidate, corner, NoSupportSlot);
            continue;
        }
        bool const ranged = PlayerbotAI::IsRangedDps(candidate.player);
        if (!candidate.healer && !ranged)
        {
            if (corner == 2)
                occupy(candidate, corner, NoSupportSlot);
            continue;
        }
        uint8 slot = freeSlot(corner, candidate.healer);
        if (slot == NoSupportSlot && corner == 2 && ranged && !candidate.healer)
            slot = 2 + sharedBlaumeuxRanged++ % (NoSupportSlot - 2);
        if (slot != NoSupportSlot)
            occupy(candidate, corner, slot);
    }

    // First cover every living holder. A healer already at a living corner can
    // move only if their group has a second healer.
    for (uint8 minimum : std::array<uint8, 2>{1, 2})
        for (uint8 corner = 0; corner < CornerCount; ++corner)
        {
            if (!liveHolder[corner] || healerCount[corner] >= minimum ||
                (dedicatedRearHealing && corner >= BlaumeuxCorner))
                continue;
            uint8 const slot = freeSlot(corner, true);
            if (corner >= 2 && slot == NoSupportSlot)
                continue;

            Candidate* choice = nullptr;
            auto priority = [corner](Candidate const& candidate)
            { return (candidate.corner != NoCorner ? 2 : 0) + (candidate.member.assignedCorner != corner); };
            for (Candidate& candidate : support)
                if (candidate.healer && !holdsTankSide(candidate) && candidate.member.coveringTank.IsEmpty() &&
                    candidate.corner != corner &&
                    (candidate.corner == NoCorner || (minimum == 1 && healerCount[candidate.corner] > 1)) &&
                    (!choice || priority(candidate) < priority(*choice)))
                    choice = &candidate;
            if (choice)
                occupy(*choice, corner, slot);
        }

    for (Candidate& candidate : support)
    {
        if (candidate.healer || candidate.corner != NoCorner || candidate.member.reinforcementHome != NoCorner)
            continue;

        uint8 const opposite = candidate.homeCorner ^ 1;
        if (opposite < 2 && liveHolder[opposite])
            occupy(candidate, opposite, NoSupportSlot);
        else if (liveHolder[0] || liveHolder[1])
            occupy(candidate, liveHolder[0] ? 0 : 1, NoSupportSlot);
        else if (PlayerbotAI::IsRangedDps(candidate.player))
        {
            for (uint8 corner : std::array<uint8, 2>{2, 3})
                if (liveHolder[corner])
                    if (uint8 const slot = freeSlot(corner, false); slot != NoSupportSlot)
                    {
                        occupy(candidate, corner, slot);
                        break;
                    }
            // Blaumeux can take a full ranged group. Reserve the individual
            // spacing restriction for Zeliek, where Holy Wrath bounces.
            if (candidate.corner == NoCorner && liveHolder[2])
                occupy(candidate, 2, 2 + sharedBlaumeuxRanged++ % (NoSupportSlot - 2));
        }
        else if (liveHolder[2])
            occupy(candidate, 2, NoSupportSlot);
    }

    if (plan.individualArmed)
        for (Candidate& candidate : support)
        {
            uint8 const corner = candidate.corner;
            if (corner == NoCorner || bosses[candidate.homeCorner] || candidate.member.reinforcementHome != NoCorner ||
                !candidate.member.coveringTank.IsEmpty())
                continue;
            uint8 const home = corner;
            candidate.member.reinforcementHome = home;
            LOG_INFO("playerbots", "[Naxx40] Horsemen instance={} {} joined {} rotation after {} fell",
                     bot.GetInstanceId(), candidate.player->GetName(), Horsemen[corner].name,
                     Horsemen[candidate.homeCorner].name);
        }

    uint8 const focus = RearFocus(bosses);
    if (focus != NoCorner)
        for (Candidate& candidate : support)
            if (candidate.healer && candidate.member.guid != plan.rearHealing[ZeliekCorner - BlaumeuxCorner].healer &&
                candidate.member.guid != plan.rearHealing[focus - BlaumeuxCorner].healer)
                occupy(candidate, focus, NoSupportSlot);

    auto isWithinMark = [&](Position const& position, uint8 mark)
    { return casters[mark] && position.GetExactDist(casters[mark]) < MarkRadius(mark) + 3.0f; };
    auto supportPosition = [](uint8 corner, uint8 slot, Player* player) -> Position const&
    {
        if (corner < 2)
            return FrontSupportPosition(corner, player);
        return slot < NoSupportSlot ? RearSupport[corner - 2][slot] : Horsemen[corner].corner;
    };
    auto clearsMarks = [&](Position const& position, uint8 marks)
    {
        for (uint8 mark = 0; mark < CornerCount; ++mark)
            if ((marks & (1u << mark)) && isWithinMark(position, mark))
                return false;
        return true;
    };

    bool const frontReady = plan.reached[0] && plan.reached[1];
    bool const rearReady = plan.rearReleased && plan.reached[2] && plan.reached[3];
    for (Candidate& candidate : support)
    {
        if (!candidate.healer)
            continue;

        candidate.earlyFrontSupport =
            !plan.individualArmed &&
            ((frontReady && candidate.corner < 2) || (candidate.homeCorner < 2 && candidate.member.clearingMarks));
        bool const rearPost = candidate.corner >= BlaumeuxCorner && candidate.corner < CornerCount;
        candidate.earlyRearHealer = !dedicatedRearHealing && EarlyRearPhase(plan, bosses) &&
                                    !candidate.earlyFrontSupport &&
                                    ((rearReady && rearPost) ||
                                     (candidate.homeCorner >= BlaumeuxCorner && candidate.member.clearingMarks));
        if (!plan.individualArmed && !candidate.earlyRearHealer && !candidate.earlyFrontSupport)
            continue;
        Position const& destination = candidate.corner == NoCorner
                                          ? candidate.player->GetPosition()
                                          : supportPosition(candidate.corner, candidate.slot, candidate.player);
        uint8 doubled = 0;
        uint8 clearsAtNextPad = 0;
        bool unsafeAtNextPad = false;
        for (uint8 mark = 0; mark < CornerCount; ++mark)
        {
            uint32 const stacks = candidate.player->GetAuraCount(Horsemen[mark].mark);
            uint8 const mask = 1u << mark;
            bool const stillClearing = stacks && (candidate.member.clearingMarks & mask);
            bool const inNextMark = isWithinMark(destination, mark);
            if (!stillClearing && (stacks < RotationStacks || (!inNextMark && !isWithinMark(*candidate.player, mark))))
                continue;
            candidate.marks |= mask;
            if (!inNextMark)
                clearsAtNextPad |= mask;
            else if (stillClearing || stacks > RotationStacks)
                unsafeAtNextPad = true;
            doubled += stacks >= RotationStacks;
            candidate.leaveForMarks |= stillClearing || stacks > RotationStacks;
            candidate.urgentMarks |= stacks > RotationStacks + 1;
        }
        candidate.leaveForMarks |= doubled >= 2;
        // A normal swap can clear an old mark while taking two stacks on the new side.
        if (candidate.earlyFrontSupport && clearsAtNextPad && !unsafeAtNextPad)
            candidate.marks = clearsAtNextPad;
    }

    auto coveringHealer = [&](uint8 corner, Candidate const& leaving) -> Candidate const*
    {
        Player const* const tank = coveredTank(corner);
        if (!tank)
            return nullptr;
        bool const requireStationary =
            corner < BlaumeuxCorner ? corner == MograineCorner && mograineVictim : rearHealersCanAnchor;
        auto const witness = std::find_if(
            support.begin(), support.end(),
            [&](Candidate const& witness)
            {
                return &witness != &leaving && witness.healer && witness.corner == corner &&
                       !witness.authorizedToLeave && !witness.member.clearingMarks &&
                       !witness.member.centerMarks && !witness.member.centerPending &&
                       (!requireStationary || !witness.player->isMoving()) &&
                       (!leaving.earlyRearHealer || !witness.marks) &&
                       (witness.member.coveringTank.IsEmpty() || witness.member.coveringTank == tank->GetGUID()) &&
                       RearTankCanBeHealedBy(*witness.player, tank);
            });
        return witness == support.end() ? nullptr : &*witness;
    };
    auto covered = [&](uint8 corner, Candidate const& leaving)
    {
        Player const* const tank = coveredTank(corner);
        return !tank || (dedicatedRearHealing && corner >= BlaumeuxCorner && RearExchangeCovered(bot, plan, *tank)) ||
               coveringHealer(corner, leaving);
    };

    Unit const* meteorTarget = !plan.individualArmed && bosses[0] ? bosses[0]->GetVictim() : nullptr;
    auto isMeteorSoaker = [&](Candidate const& candidate)
    {
        return meteorTarget && candidate.player != meteorTarget && candidate.corner == 0 &&
               candidate.player->GetExactDist(meteorTarget) <= MeteorSoakRange &&
               (candidate.healer
                    ? (!candidate.member.clearingMarks || candidate.member.assignedCorner == candidate.corner)
                    : !MarksToClear(*candidate.player, DpsRetreatStacks, candidate.member.damageMarks));
    };
    auto remainingSoak = std::count_if(support.begin(), support.end(), isMeteorSoaker);
    if (meteorTarget)
        ++remainingSoak;

    std::array<bool, CornerCount> departureThisSnapshot = {};
    for (Candidate& candidate : support)
    {
        if (!candidate.marks)
            continue;
        uint8 const from = candidate.corner;
        bool const urgentRearRetreat = focus != NoCorner && candidate.healer && candidate.urgentMarks;
        bool const startingRearRelief =
            candidate.earlyRearHealer && from >= 2 && from < CornerCount && !candidate.member.clearingMarks;
        if (startingRearRelief && !urgentRearRetreat &&
            (plan.earlyRearSwapAt || clearsMarks(supportPosition(from, candidate.slot, candidate.player), candidate.marks)))
        {
            candidate.leaveForMarks = false;
            continue;
        }
        if (candidate.healer && from != NoCorner && departureThisSnapshot[from] && !candidate.member.clearingMarks &&
            !urgentRearRetreat)
        {
            candidate.leaveForMarks = false;
            continue;
        }
        if (candidate.healer && !candidate.member.clearingMarks && from != NoCorner && !covered(from, candidate))
        {
            // Prepare a spare at two stacks. No marked healer leaves a tank
            // until someone assigned to that tank can actually heal it.
            uint8 slot = freeSlot(from, true);
            if (from >= 2 && slot == NoSupportSlot)
                slot = freeSlot(from, false);
            if ((from < 2 || slot != NoSupportSlot) && (!dedicatedRearHealing || from < BlaumeuxCorner))
            {
                Candidate* spare = nullptr;
                auto priority = [from](Candidate const& p)
                { return (p.corner != NoCorner ? 2 : 0) + (p.member.assignedCorner != from); };
                auto readyToRelieve = [&](Candidate const& donor)
                {
                    if (!candidate.earlyRearHealer && !candidate.earlyFrontSupport)
                        return true;
                    return std::none_of(Horsemen.begin(), Horsemen.end(), [&](Horseman const& rider)
                                        { return donor.player->GetAuraCount(rider.mark) > RotationStacks; });
                };
                for (Candidate& donor : support)
                    if (donor.healer && !holdsTankSide(donor) && donor.member.coveringTank.IsEmpty() &&
                        donor.corner != from && !donor.marks &&
                        donor.player->GetPowerPct(POWER_MANA) >= HealerCoverageMana &&
                        !donor.player->HasUnitFlag(UNIT_FLAG_SILENCED) &&
                        donor.player->GetAuraCount(Horsemen[from].mark) < RotationStacks && readyToRelieve(donor) &&
                        covered(donor.corner, donor) && (!spare || priority(donor) < priority(*spare)))
                        spare = &donor;
                if (spare)
                {
                    occupy(*spare, from, slot);
                    if (candidate.earlyRearHealer || candidate.earlyFrontSupport || (rearHealersCanAnchor && from >= 2))
                    {
                        // Keep the incoming healer with this tank while they get into casting range.
                        Player const* const tank = coveredTank(from);
                        spare->member.coveringTank = tank->GetGUID();
                        LOG_INFO("playerbots",
                                 "[Naxx40] Horsemen instance={} healer {} moving to cover {} before {} rotates",
                                 bot.GetInstanceId(), spare->player->GetName(), tank->GetName(),
                                 candidate.player->GetName());
                    }
                }
            }
            if (!covered(from, candidate) && !urgentRearRetreat)
            {
                candidate.leaveForMarks = false;
                continue;
            }
        }

        if (!candidate.leaveForMarks)
            continue;

        bool const anchor = holdsTankSide(candidate);
        Candidate const* const anchorBackup = anchor ? coveringHealer(from, candidate) : nullptr;
        if (anchor && !anchorBackup && !urgentRearRetreat)
        {
            candidate.leaveForMarks = false;
            continue;
        }

        if (startingRearRelief && !urgentRearRetreat && bosses[from] &&
            (!tanks[from] || bosses[from]->GetVictim() != tanks[from] ||
             tanks[from]->GetExactDist(Horsemen[from].corner) > RearCornerRange))
        {
            candidate.leaveForMarks = false;
            continue;
        }

        auto canTransfer = [&](uint8 to, uint8 slot)
        {
            if (to == NoCorner || (focus != NoCorner && candidate.healer && to != focus) || !liveHolder[to] ||
                candidate.player->GetAuraCount(Horsemen[to].mark) >= RotationStacks ||
                (to >= 2 &&
                 (slot == NoSupportSlot || (!candidate.healer && !PlayerbotAI::IsRangedDps(candidate.player)))))
                return false;
            Position const& destination = supportPosition(to, slot, candidate.player);
            return candidate.player->GetExactDist(destination) <= SupportTransferRange &&
                   clearsMarks(destination, candidate.marks);
        };
        uint8 const prior = candidate.member.assignedCorner;
        uint8 const priorSlot = candidate.member.assignedSlot;
        bool const retainPrior = candidate.member.clearingMarks && prior != from && canTransfer(prior, priorSlot) &&
                                 (prior < 2 || !occupiedSlots[prior][priorSlot]);
        if (!retainPrior && from != NoCorner &&
            clearsMarks(supportPosition(from, candidate.slot, candidate.player), candidate.marks))
            continue;

        // Let the cohort make its normal swap before peeling a soaker off Korth'azz.
        if (candidate.earlyFrontSupport && isMeteorSoaker(candidate))
        {
            if (remainingSoak <= MinimumMeteorSoak)
            {
                candidate.leaveForMarks = false;
                continue;
            }
            --remainingSoak;
        }

        if (candidate.healer && (!candidate.member.clearingMarks || anchorBackup) &&
            (!dedicatedRearHealing || from < BlaumeuxCorner))
            if (Candidate const* cover = anchorBackup ? anchorBackup : coveringHealer(from, candidate))
            {
                candidate.authorizedToLeave = true;
                departureThisSnapshot[from] = true;
                candidate.member.coveringTank.Clear();
                Player const* const tank = coveredTank(from);
                cover->member.coveringTank = tank->GetGUID();
                if (anchor && from == MograineCorner)
                    plan.mograineHealing = {cover->member.guid, tank->GetGUID()};
                else if (anchor)
                    plan.rearHealing[from - BlaumeuxCorner] = {cover->member.guid, tank->GetGUID()};
                LOG_INFO("playerbots",
                         "[Naxx40] Horsemen instance={} healer {} authorized to rotate from {} after {} covers {}",
                         bot.GetInstanceId(), candidate.player->GetName(), Horsemen[from].name,
                         cover->player->GetName(), tank->GetName());
            }

        if (retainPrior)
        {
            occupy(candidate, prior, priorSlot);
            continue;
        }

        // Try the prior safe assignment, the other side of this pair, then
        // the remaining living corners. Rear destinations consume a unique slot.
        uint8 const origin = from == NoCorner ? candidate.homeCorner : from;
        std::array<uint8, 6> const order = {candidate.member.assignedCorner, uint8(origin ^ 1), 0, 1, 2, 3};
        bool moved = false;
        for (uint8 to : order)
        {
            if (to == from || to == NoCorner)
                continue;
            uint8 const slot = freeSlot(to, candidate.healer);
            if (!canTransfer(to, slot))
                continue;
            occupy(candidate, to, slot);
            moved = true;
            break;
        }
        if (!moved)
            release(candidate);
    }

    std::array<Creature const*, CornerCount> controlled{};
    for (uint8 corner = 0; corner < CornerCount; ++corner)
        if (DpsBossControlled(plan, bosses[corner], corner))
            controlled[corner] = bosses[corner];
    Unit const* const wrathVictim =
        bosses[ZeliekCorner] && focus != ZeliekCorner ? bosses[ZeliekCorner]->GetVictim() : nullptr;

    uint8 const firstLivingCorner = FirstLivingCorner(bosses);
    Creature const* const damageFocus = firstLivingCorner == NoCorner ? nullptr : controlled[firstLivingCorner];

    for (Candidate const& candidate : support)
    {
        Member& member = candidate.member;
        if (candidate.corner != NoCorner && candidate.corner != candidate.homeCorner &&
            (member.assignedCorner != candidate.corner || member.assignedSlot != candidate.slot))
            LOG_INFO("playerbots", "[Naxx40] Horsemen instance={} support reassigned {} ({}) {} -> {}{}",
                     bot.GetInstanceId(), candidate.player->GetName(), candidate.healer ? "healer" : "dps",
                     Horsemen[candidate.homeCorner].name, Horsemen[candidate.corner].name,
                     candidate.leaveForMarks ? " for individual marks"
                     : candidate.healer      ? " for tank coverage"
                                             : " after losing the assigned target");
        uint8 const clearing = candidate.leaveForMarks ? candidate.marks : 0;
        if (clearing != member.clearingMarks)
            LOG_INFO("playerbots", "[Naxx40] Horsemen instance={} {} individual marks mask={} -> {}",
                     bot.GetInstanceId(), candidate.player->GetName(), clearing,
                     candidate.corner == NoCorner ? "mark retreat" : Horsemen[candidate.corner].name);
        member.clearingMarks = clearing;
        member.assignedCorner = candidate.corner;
        member.assignedSlot = candidate.slot;
        // A bot waiting for a cast cannot run its own hold action yet.
        if (IsAutonomousBot(candidate.player))
        {
            Creature const* permitted = nullptr;
            if (candidate.healer && candidate.corner < CornerCount &&
                AtCorner(controlled[candidate.corner], candidate.corner))
                permitted = controlled[candidate.corner];
            else if (!candidate.healer && member.damagePosition && !member.centerPending && !member.centerMarks &&
                     (focus != ZeliekCorner || PlayerbotAI::IsRangedDps(candidate.player) ||
                      std::ranges::find(plan.zeliekMelee, member.guid) != plan.zeliekMelee.end()) &&
                     !DpsMarksToClear(*candidate.player, bosses, member.damageMarks) &&
                     (!wrathVictim || wrathVictim == candidate.player ||
                      candidate.player->GetExactDist2d(wrathVictim) >= HolyWrathClearance))
                for (Creature const* boss : controlled)
                    if (boss && boss->GetGUID() == member.damageTarget &&
                        (firstLivingCorner == KorthazzCorner || boss == damageFocus))
                        permitted = boss;
            StopRiderDamage(*candidate.player, permitted);
        }
    }
}

bool OwnsTarget(Creature const* boss, ObjectGuid holder)
{
    return !boss || (!holder.IsEmpty() && boss->GetVictim() && boss->GetVictim()->GetGUID() == holder);
}

bool AtCorner(Creature const* boss, uint8 corner)
{
    Position const& pad = corner < 2 ? FrontHolderPads[corner] : Horsemen[corner].corner;
    return !boss || boss->GetExactDist(pad) <= (corner < 2 ? BossCornerRange : RearCornerRange);
}

bool InZeliekDpsFlank(Position const& position, Unit const& zeliek, bool ranged)
{
    float const east = position.GetPositionX() - zeliek.GetPositionX();
    return ranged ? east >= HolyWrathClearance : east <= -1.0f;
}

bool InZeliekTankFlank(Position const& position, Creature const& zeliek)
{
    return position.GetPositionX() <= zeliek.GetPositionX() + CornerArrival;
}

Creature const* RearMarkSource(Player* bot, Bosses const& bosses, uint8 corner)
{
    return MarkCaster(bot, bosses, corner);
}

float MarkDistance(Position const& position, Creature const* source)
{
    return source ? position.GetExactDist(source) : -1.0f;
}

bool OutsideRearMark(Position const& position, Creature const* source)
{
    return !source || MarkDistance(position, source) >= RearMarkClearance;
}

bool InHandoffCorner(Position const& position, Creature const& boss, uint8 corner)
{
    Position const& pad = corner < BlaumeuxCorner ? FrontHolderPads[corner] : Horsemen[corner].corner;
    float const radius = corner < BlaumeuxCorner ? BossCornerRange : RearCornerRange;
    float const minimumClearance = std::min(boss.GetExactDist2d(&TransitionCenter), MarkRadius(corner) + 3.0f);
    return position.GetExactDist(pad) <= radius && position.GetExactDist2d(&TransitionCenter) >= minimumClearance;
}

bool HealerCoversHandoffPad(Player const* healer, Player const& holder, Position const& pad,
                            Player const* incoming = nullptr)
{
    return CanSustainHealingCoverage(healer) && !healer->isMoving() && RearTankCanBeHealedBy(*healer, &holder) &&
           healer->GetExactDist(pad) <
               HealerCoverageRange + healer->GetObjectSize() + (incoming ? incoming : &holder)->GetObjectSize() &&
           healer->IsWithinLOS(pad.GetPositionX(), pad.GetPositionY(), pad.GetPositionZ());
}

Player* LocalHolderHealer(Player& observer, RaidPlan const& plan, Player const& holder, uint8 corner)
{
    TankHealerAssignment const& coverage =
        corner == MograineCorner ? plan.mograineHealing : plan.rearHealing[corner - BlaumeuxCorner];
    Player* const healer = coverage.target == holder.GetGUID() ? PlayerInRaid(observer, coverage.healer) : nullptr;
    return CanSustainHealingCoverage(healer) ? healer : nullptr;
}

bool LocalIncomingHealthy(Player const& incoming, Player const& holder)
{
    float const minimum = holder.GetHealthPct() <= sPlayerbotAIConfig.CriticalHealth ? sPlayerbotAIConfig.LowHealth
                                                                                     : RearTankUrgentHealth;
    return incoming.GetHealthPct() >= minimum;
}

bool LocalHandoffReady(Player const& incoming, Player const& holder, Player const* healer, Position const& pad,
                       bool spreadForWrath)
{
    return LocalIncomingHealthy(incoming, holder) &&
           (holder.GetHealthPct() <= sPlayerbotAIConfig.CriticalHealth ||
            (HealerCoversHandoffPad(healer, holder, pad) &&
             (!spreadForWrath || healer->GetExactDist2d(&incoming) >= HolyWrathClearance) &&
             RearTankCanBeHealedBy(*healer, &incoming)));
}

void ReleaseTank(RaidPlan& plan, ObjectGuid guid, uint8 corner)
{
    if (guid.IsEmpty())
        return;
    if (auto const member = std::ranges::find(plan.members, guid, &Member::guid); member != plan.members.end())
        member->releasedTankMarks |= 1u << corner;
}

ObjectGuid ScheduledFrontReserve(RaidPlan const& plan, Bosses const& bosses, uint8 corner, ObjectGuid holder)
{
    if (corner != MograineCorner || bosses[KorthazzCorner] || !bosses[MograineCorner])
        return ObjectGuid::Empty;
    ObjectGuid const main = plan.openingHolders[KorthazzCorner];
    ObjectGuid const assist = plan.openingHolders[MograineCorner];
    return holder == assist ? main : assist;
}

bool TankAvailable(RaidPlan const& plan, ObjectGuid guid, uint8 corner)
{
    if (guid.IsEmpty())
        return false;
    for (uint8 other = 0; other < CornerCount; ++other)
        if (other != corner && (plan.holders[other] == guid || plan.tankHandoffs[other].incoming == guid))
            return false;
    return true;
}

void RecoverHolder(Player& bot, RaidPlan& plan, Bosses const& bosses, uint8 corner)
{
    Creature const& boss = *bosses[corner];
    TankHandoff& handoff = plan.tankHandoffs[corner];
    uint32 const now = getMSTime();
    Player* replacement = HolderInRaid(bot, handoff.incoming);
    if (!CanHold(replacement) || !TankAvailable(plan, handoff.incoming, corner))
        replacement = nullptr;
    if (!replacement)
    {
        if (handoff.checkedAt && getMSTimeDiff(handoff.checkedAt, now) < RearReliefRetryInterval)
            return;
        handoff.checkedAt = now;
        float shortest = std::numeric_limits<float>::max();
        for (Member const& member : plan.members)
        {
            if (!TankAvailable(plan, member.guid, corner))
                continue;
            Player* const candidate = HolderInRaid(bot, member.guid);
            if (!CanHold(candidate) || candidate->GetHealthPct() < sPlayerbotAIConfig.LowHealth ||
                candidate->GetExactDist(&boss) - TauntRange >= shortest)
                continue;
            if (auto const approach = TankReliefPosition(*candidate, boss, bosses))
            {
                float const distance = candidate->GetExactDist(*approach);
                if (distance < shortest)
                {
                    shortest = distance;
                    replacement = candidate;
                }
            }
        }
    }
    if (!replacement)
        return;
    plan.holders[corner] = replacement->GetGUID();
    handoff = {.boss = boss.GetGUID()};
    LOG_INFO("playerbots", "[Naxx40] Horsemen instance={} {} emergency holder {}", bot.GetInstanceId(),
             Horsemen[corner].name, replacement->GetName());
}

void StopRearExchangePath(Player* tank, uint32 splineId)
{
    if (!tank || !splineId || tank->movespline->Finalized() || tank->movespline->GetId() != splineId)
        return;
    tank->StopMoving();
    GET_PLAYERBOT_AI(tank)->GetAiObjectContext()->GetValue<LastMovement&>("last movement")->Get().clear();
}

void CloseEarlyRearExchange(RaidPlan& plan, uint32 now)
{
    plan.earlyRearSwapAt = 0;
    plan.earlyRearMarkWindow = 0;
    plan.earlyRearSecuredAt = now;
    plan.earlyRearUncoveredAt = 0;
    plan.earlyRearSplines = {};
    plan.earlyRearCrossings = {};
    plan.earlyRearRouteBlocks = {};
    plan.earlyRearReplanAt = {};
    plan.earlyRearTaunts = {};
    plan.earlyRearReady = 0xff;
    plan.earlyRearBlocked = 0;
}

bool AtRearExchangePad(Player const& tank, RaidPlan const& plan, uint8 side)
{
    return tank.GetExactDist(plan.earlyRearDestinations[side]) <= CornerArrival;
}

struct EarlyRearRoute
{
    Position destination;
    Movement::PointsArray path;
    std::string_view block;
    bool crossesPool = false;
};

EarlyRearRoute PrepareEarlyRearRoute(Player& tank, FourHorsemen40Hazards& hazards, uint8 side, Position const& current,
                                     Player const* healer, float secondsToArrive = 0.0f)
{
    EarlyRearRoute route;
    auto const prepare = [&](Position const& destination)
    {
        if (!hazards.PrepareRearExchangePath(tank, destination, CornerArrival, route.path,
                                             {healer != nullptr, healer, HealerCoverageRange, secondsToArrive},
                                             &route.crossesPool, &route.block))
            return false;
        route.destination = destination;
        return true;
    };

    if (hazards.IsSafeFromVoidZones(tank, current))
    {
        prepare(current);
        return route;
    }

    Position const& nominal = RearExchangePads[side];
    if (current.GetExactDist(nominal) > 0.01f && hazards.IsSafeFromVoidZones(tank, nominal) && prepare(nominal))
        return route;

    // A pool over the meeting point can leave a nearby dry place on the same side.
    float const bearing = nominal.GetAbsoluteAngle(&tank);
    std::optional<EarlyRearRoute> crossing;
    route.block = "no dry meeting point within six yards";
    for (float radius : {CornerArrival / 2.0f, CornerArrival, 4.0f, RearExchangePadFallback})
        for (int step : {0, 1, -1, 2, -2, 3, -3, 4})
        {
            float const angle = bearing + static_cast<float>(step) * static_cast<float>(M_PI) / 4.0f;
            float const x = nominal.GetPositionX() + std::cos(angle) * radius;
            float const y = nominal.GetPositionY() + std::sin(angle) * radius;
            float const z = tank.GetMapHeight(x, y, nominal.GetPositionZ() + CornerArrival, true, 5.0f);
            Position const destination(x, y, z);
            if (z <= INVALID_HEIGHT || !hazards.IsSafeFromVoidZones(tank, destination) || !prepare(destination))
                continue;
            if (!route.crossesPool)
                return route;
            if (!crossing)
                crossing = std::move(route);
        }
    if (crossing)
        return std::move(*crossing);
    return route;
}

void LogEarlyRearRouteBlock(Player const& tank, RaidPlan& plan, uint8 side, std::string_view reason)
{
    if (!reason.empty() && plan.earlyRearRouteBlocks[side] != reason)
        LOG_INFO("playerbots", "[Naxx40] Horsemen instance={} {} rear route blocked for {}: {}", tank.GetInstanceId(),
                 Horsemen[BlaumeuxCorner + side].name, tank.GetName(), reason);
    plan.earlyRearRouteBlocks[side] = reason;
}

void SetEarlyRearRoute(Player const& tank, RaidPlan& plan, uint8 side, EarlyRearRoute const& route)
{
    Position const& destination = route.destination;
    plan.earlyRearDestinations[side] = destination;
    plan.earlyRearCrossings[side] = route.crossesPool;
    if (route.crossesPool || destination.GetExactDist(RearExchangePads[side]) > 0.01f)
        LOG_INFO("playerbots",
                 "[Naxx40] Horsemen instance={} {} rear route to ({:.2f}, {:.2f}, {:.2f}), pool crossing={}",
                 tank.GetInstanceId(), tank.GetName(), destination.GetPositionX(), destination.GetPositionY(),
                 destination.GetPositionZ(), route.crossesPool);
}

bool HasSustainedHorsemanAggro(Creature const& rider, Player const& tank)
{
    if (rider.GetVictim() != &tank)
        return false;
    auto const& threat = rider.GetThreatMgr();
    for (ThreatReference const* reference : threat.GetUnsortedThreatList())
        if (reference->GetVictim() == &tank)
            return !reference->IsTaunting() && threat.GetLastVictim() == &tank;
    return false;
}

uint32 TimeToRearMark(Player const& tank, uint8 corner)
{
    Aura const* const mark = tank.GetAura(Horsemen[corner].mark);
    if (!mark)
        return RearMarkInterval;
    int32 const age = mark->GetMaxDuration() - mark->GetDuration();
    return static_cast<uint32>(
        std::clamp(static_cast<int32>(RearMarkInterval) - age, 0, static_cast<int32>(RearMarkInterval)));
}

uint32 RearExchangeTimeRemaining(RaidPlan const& plan, uint32 now)
{
    uint32 const elapsed = getMSTimeDiff(plan.earlyRearSwapAt, now);
    return elapsed < plan.earlyRearMarkWindow ? plan.earlyRearMarkWindow - elapsed : 0;
}

Player* FindRearReserve(Player& bot, RaidPlan const& plan, Bosses const& bosses, uint8 corner, ObjectGuid skip);
bool RearReserveAvailable(RaidPlan const& plan, Bosses const& bosses, Player* candidate, uint8 corner);

void PreparePartialRearRelief(Player& bot, RaidPlan& plan, Bosses const& bosses, uint8 corner, uint32 now)
{
    TankHandoff& handoff = plan.tankHandoffs[corner];
    ObjectGuid const previous = handoff.incoming;
    Player* const staged = HolderInRaid(bot, previous);
    bool const available = RearReserveAvailable(plan, bosses, staged, corner);
    if (available &&
        (handoff.approach || handoff.tauntAt || getMSTimeDiff(handoff.preparedAt, now) < RearReserveRouteGrace))
        return;
    if (!available && !previous.IsEmpty())
    {
        ReleaseTank(plan, previous, corner);
        handoff = {.boss = handoff.boss, .releasedAt = handoff.releasedAt};
    }
    if (available)
        handoff.preparedAt = now;
    else
    {
        if (handoff.checkedAt && getMSTimeDiff(handoff.checkedAt, now) < RearReliefRetryInterval)
            return;
        handoff.checkedAt = now;
    }
    if (Player* const reserve = FindRearReserve(bot, plan, bosses, corner, available ? previous : ObjectGuid::Empty))
    {
        ReleaseTank(plan, handoff.incoming, corner);
        handoff = {.boss = bosses[corner]->GetGUID(),
                   .incoming = reserve->GetGUID(),
                   .releasedAt = handoff.releasedAt,
                   .preparedAt = now,
                   .active = true};
        LOG_INFO("playerbots", "[Naxx40] Horsemen instance={} {} requests {} to finish partial rear exchange",
                 bot.GetInstanceId(), Horsemen[corner].name, reserve->GetName());
    }
}

void UpdateEarlyRearSwap(Player& bot, RaidPlan& plan, Bosses const& bosses, uint32 now)
{
    Creature const* const blaumeux = bosses[BlaumeuxCorner];
    Creature const* const zeliek = bosses[ZeliekCorner];
    ObjectGuid& left = plan.holders[BlaumeuxCorner];
    ObjectGuid& right = plan.holders[ZeliekCorner];
    Player* const leftTank = HolderInRaid(bot, left);
    Player* const rightTank = HolderInRaid(bot, right);
    if (plan.earlyRearSwapAt)
    {
        auto const receivingTank = [&](uint8 corner, Player* partner)
        {
            Unit const* const victim = bosses[corner] ? bosses[corner]->GetVictim() : nullptr;
            return IsEarlyRearReplacement(plan, corner, victim) ? HolderInRaid(bot, victim->GetGUID()) : partner;
        };
        Player* const leftReceiver = receivingTank(BlaumeuxCorner, rightTank);
        Player* const rightReceiver = receivingTank(ZeliekCorner, leftTank);
        bool const reserveMoving = plan.tankHandoffs[BlaumeuxCorner].active || plan.tankHandoffs[ZeliekCorner].active;
        auto const hasCoverage = [&](Creature const* rider, Player* recipient)
        {
            Player* const patient = reserveMoving && rider ? RearHealingTarget(bot, rider->GetVictim()) : recipient;
            return patient && RearExchangeCovered(bot, plan, *patient);
        };
        bool const covered = hasCoverage(blaumeux, leftReceiver) && hasCoverage(zeliek, rightReceiver);
        if (covered)
            plan.earlyRearUncoveredAt = 0;
        else if (!plan.earlyRearUncoveredAt)
            plan.earlyRearUncoveredAt = now;
        bool const coverageLost = plan.earlyRearUncoveredAt &&
                                  getMSTimeDiff(plan.earlyRearUncoveredAt, now) >= EarlyRearCoverageGrace;
        bool const leftDelivered = blaumeux && CanHold(leftReceiver) && blaumeux->GetVictim() == leftReceiver;
        bool const rightDelivered = zeliek && CanHold(rightReceiver) && zeliek->GetVictim() == rightReceiver;
        bool const leftChanged = leftDelivered && HasSustainedHorsemanAggro(*blaumeux, *leftReceiver);
        bool const rightChanged = rightDelivered && HasSustainedHorsemanAggro(*zeliek, *rightReceiver);
        uint32 const remaining = RearExchangeTimeRemaining(plan, now);
        bool const fallbackDue = remaining <= EarlyRearReserveLead;
        bool const secured = leftChanged && rightChanged && covered;
        bool const interrupted = !blaumeux || !zeliek || !CanHold(leftTank) || !CanHold(rightTank) || coverageLost ||
                                 (fallbackDue && !leftDelivered && !rightDelivered) ||
                                 getMSTimeDiff(plan.earlyRearSwapAt, now) >= EarlyRearTimeout;
        if (!secured && !interrupted)
        {
            if (fallbackDue && leftDelivered != rightDelivered)
            {
                uint8 const unresolved = leftDelivered ? ZeliekCorner : BlaumeuxCorner;
                uint8 const side = (unresolved ^ 1) - BlaumeuxCorner;
                if (plan.tankHandoffs[unresolved].active || !plan.earlyRearRouteBlocks[side].empty() ||
                    remaining <= EarlyRearProbeInterval)
                    PreparePartialRearRelief(bot, plan, bosses, unresolved, now);
            }
            return;
        }

        ObjectGuid const originalLeft = left;
        ObjectGuid const originalRight = right;
        if (leftChanged)
            left = leftReceiver->GetGUID();
        if (rightChanged)
            right = rightReceiver->GetGUID();
        if (left == right)
        {
            if (leftChanged)
                right.Clear();
            else
                left.Clear();
        }
        for (uint8 side : {0, 1})
        {
            Player* const tank = side == 0 ? leftTank : rightTank;
            if (CanHold(tank) && !FourHorsemen40Hazards(GET_PLAYERBOT_AI(tank)).IsSafeFromVoidDamage(*tank, *tank))
                plan.earlyRearInterruptedInPool[side] = {tank->GetGUID(), plan.earlyRearSplines[side]};
            else
                StopRearExchangePath(tank, plan.earlyRearSplines[side]);
            uint8 const corner = BlaumeuxCorner + side;
            TankHandoff& handoff = plan.tankHandoffs[corner];
            if (handoff.incoming != plan.holders[corner])
                ReleaseTank(plan, handoff.incoming, corner);
            handoff = {.boss = handoff.boss, .releasedAt = handoff.releasedAt, .returning = true};
        }
        if (originalLeft != left && originalLeft != right)
            ReleaseTank(plan, originalLeft, BlaumeuxCorner);
        if (originalRight != left && originalRight != right)
            ReleaseTank(plan, originalRight, ZeliekCorner);
        for (EarlyRearHealerAssignment& binding : plan.earlyRearHealing)
        {
            if (rightChanged && binding.tank == originalLeft && originalLeft != left && originalLeft != right)
                binding.tank = right;
            else if (leftChanged && binding.tank == originalRight && originalRight != left && originalRight != right)
                binding.tank = left;
        }

        uint8 const blocked = (!plan.earlyRearRouteBlocks[0].empty() ? 1u << BlaumeuxCorner : 0) |
                              (!plan.earlyRearRouteBlocks[1].empty() ? 1u << ZeliekCorner : 0);
        CloseEarlyRearExchange(plan, now);
        if (!leftDelivered && !rightDelivered && (fallbackDue || coverageLost))
            plan.earlyRearBlocked = blocked ? blocked : (1u << BlaumeuxCorner) | (1u << ZeliekCorner);
        if (secured)
            LOG_INFO(
                "playerbots", "[Naxx40] Horsemen instance={} early rear exchange secured, {} on {} marks, {} on {}",
                bot.GetInstanceId(), leftReceiver->GetName(), leftReceiver->GetAuraCount(Horsemen[BlaumeuxCorner].mark),
                rightReceiver->GetName(), rightReceiver->GetAuraCount(Horsemen[ZeliekCorner].mark));
        else
            LOG_INFO(
                "playerbots",
                "[Naxx40] Horsemen instance={} early rear exchange interrupted, coverage lost={}, reserve deadline={}",
                bot.GetInstanceId(), coverageLost, fallbackDue);
        return;
    }
    auto const clearHealerPreparation = [&]
    {
        for (EarlyRearHealerAssignment& binding : plan.earlyRearHealing)
        {
            if (!binding.preparingExchange)
                continue;
            binding.preparingExchange = false;
            binding.preparedHealer.Clear();
            binding.exchangePosition.reset();
        }
    };
    if (!EarlyRearPhase(plan, bosses) || !plan.rearReleased || plan.tankHandoffs[BlaumeuxCorner].active ||
        plan.tankHandoffs[ZeliekCorner].active ||
        (plan.earlyRearSecuredAt && getMSTimeDiff(plan.earlyRearSecuredAt, now) < RetryDelay))
    {
        clearHealerPreparation();
        return;
    }

    uint32 const leftMarks = leftTank ? leftTank->GetAuraCount(Horsemen[BlaumeuxCorner].mark) : 0;
    uint32 const rightMarks = rightTank ? rightTank->GetAuraCount(Horsemen[ZeliekCorner].mark) : 0;
    uint32 const highestMarks = std::max(leftMarks, rightMarks);
    if (highestMarks < RearPrepareStacks)
    {
        clearHealerPreparation();
        plan.earlyRearBlocked = 0;
        plan.earlyRearProbeMarks = highestMarks;
        return;
    }
    if (plan.earlyRearProbeAt && getMSTimeDiff(plan.earlyRearProbeAt, now) < EarlyRearProbeInterval &&
        highestMarks == plan.earlyRearProbeMarks)
        return;
    plan.earlyRearProbeAt = now;
    plan.earlyRearProbeMarks = highestMarks;
    clearHealerPreparation();

    auto const readyToLeave = [&](Player* tank, uint8 corner)
    {
        return bosses[corner] && CanHold(tank) && OwnsTarget(bosses[corner], plan.holders[corner]) &&
               AtCorner(bosses[corner], corner) && tank->GetExactDist(Horsemen[corner].corner) <= RearCornerRange &&
               OutsideRearMark(*tank, bosses[corner ^ 1]) &&
               tank->GetAuraCount(Horsemen[corner ^ 1].mark) < RotationStacks;
    };
    bool const leftReady = readyToLeave(leftTank, BlaumeuxCorner);
    bool const rightReady = readyToLeave(rightTank, ZeliekCorner);
    plan.earlyRearBlocked = (!leftReady ? 1u << BlaumeuxCorner : 0) | (!rightReady ? 1u << ZeliekCorner : 0);
    if (!blaumeux || !zeliek || !CanHold(leftTank) || !CanHold(rightTank))
        plan.earlyRearBlocked = (1u << BlaumeuxCorner) | (1u << ZeliekCorner);
    if (!leftReady || !rightReady)
        return;

    bool const protectionReady =
        plan.earlyRearSecuredAt || ConfirmFirstShadowProtection(*rightTank, plan.rearProtection, now);
    FourHorsemen40Hazards leftHazards(GET_PLAYERBOT_AI(leftTank));
    FourHorsemen40Hazards rightHazards(GET_PLAYERBOT_AI(rightTank));
    uint32 const markWindow =
        std::min(leftMarks == highestMarks ? TimeToRearMark(*leftTank, BlaumeuxCorner) : RearMarkInterval,
                 rightMarks == highestMarks ? TimeToRearMark(*rightTank, ZeliekCorner) : RearMarkInterval);
    float const secondsToArrive = static_cast<float>(std::max(1u, markWindow)) / 1000.0f;
    EarlyRearRoute const leftApproach = PrepareEarlyRearRoute(
        *leftTank, leftHazards, 0, RearExchangePads[0], RearExchangeHealer(bot, plan, *leftTank), secondsToArrive);
    EarlyRearRoute const rightApproach = PrepareEarlyRearRoute(
        *rightTank, rightHazards, 1, RearExchangePads[1], RearExchangeHealer(bot, plan, *rightTank), secondsToArrive);
    bool const leftRoute = leftApproach.block.empty();
    bool const rightRoute = rightApproach.block.empty();
    bool const leftCovered =
        leftRoute && PrepareRearExchangeHealer(bot, plan, *leftTank, leftApproach.destination, zeliek) ==
                         RearHealerCoverage::Live;
    bool const rightCovered =
        rightRoute && PrepareRearExchangeHealer(bot, plan, *rightTank, rightApproach.destination, zeliek) ==
                          RearHealerCoverage::Live;
    plan.earlyRearBlocked =
        (!leftCovered ? 1u << BlaumeuxCorner : 0) | (!rightCovered || !protectionReady ? 1u << ZeliekCorner : 0);
    LogEarlyRearRouteBlock(*leftTank, plan, 0,
                           leftRoute && !leftCovered ? "healer cannot cover the meeting point" : leftApproach.block);
    LogEarlyRearRouteBlock(*rightTank, plan, 1,
                           rightRoute && !rightCovered ? "healer cannot cover the meeting point" : rightApproach.block);
    uint8 const ready = uint8(leftRoute) | (uint8(rightRoute) << 1) | (uint8(leftCovered) << 2) |
                        (uint8(rightCovered) << 3);
    if (ready != plan.earlyRearReady)
    {
        plan.earlyRearReady = ready;
        LOG_INFO("playerbots", "[Naxx40] Horsemen instance={} early rear readiness routes={}/{} healers={}/{}",
                 bot.GetInstanceId(), leftRoute, rightRoute, leftCovered, rightCovered);
    }
    if (highestMarks < RearSwapStacks || plan.earlyRearBlocked)
        return;

    if (!leftHazards.StartRearExchangePath(*leftTank, leftApproach.path, plan.earlyRearSplines[0]))
    {
        plan.earlyRearBlocked |= 1u << BlaumeuxCorner;
        LogEarlyRearRouteBlock(*leftTank, plan, 0, "could not launch movement");
        return;
    }
    if (!rightHazards.StartRearExchangePath(*rightTank, rightApproach.path, plan.earlyRearSplines[1]))
    {
        StopRearExchangePath(leftTank, plan.earlyRearSplines[0]);
        plan.earlyRearSplines = {};
        plan.earlyRearBlocked |= 1u << ZeliekCorner;
        LogEarlyRearRouteBlock(*rightTank, plan, 1, "could not launch movement, paired launch cancelled");
        return;
    }
    SetEarlyRearRoute(*leftTank, plan, 0, leftApproach);
    SetEarlyRearRoute(*rightTank, plan, 1, rightApproach);
    for (uint8 corner : {BlaumeuxCorner, ZeliekCorner})
    {
        TankHandoff& handoff = plan.tankHandoffs[corner];
        ReleaseTank(plan, handoff.incoming, corner);
        handoff = {.boss = handoff.boss, .releasedAt = handoff.releasedAt};
    }
    plan.earlyRearSwapAt = now;
    plan.earlyRearMarkWindow = markWindow;
    LOG_INFO("playerbots",
             "[Naxx40] Horsemen instance={} early rear exchange launched both tanks, Blaumeux={} Zeliek={}",
             bot.GetInstanceId(), leftMarks, rightMarks);
}

bool RearReserveAvailable(RaidPlan const& plan, Bosses const& bosses, Player* candidate, uint8 corner,
                          Member const& member)
{
    if (!CanHold(candidate) || !GET_PLAYERBOT_AI(candidate)->CanMove() ||
        candidate->GetHealthPct() < sPlayerbotAIConfig.LowHealth ||
        MarksToClear(*candidate, RotationStacks, member.releasedTankMarks | member.centerMarks))
        return false;
    ObjectGuid const guid = member.guid;
    if (guid == plan.holders[corner] || !TankAvailable(plan, guid, corner) ||
        guid == ScheduledFrontReserve(plan, bosses, MograineCorner, plan.holders[MograineCorner]))
        return false;
    return std::ranges::none_of(bosses,
                                [candidate](Creature const* boss) { return boss && boss->GetVictim() == candidate; });
}

bool RearReserveAvailable(RaidPlan const& plan, Bosses const& bosses, Player* candidate, uint8 corner)
{
    if (!candidate)
        return false;
    auto const member = std::ranges::find(plan.members, candidate->GetGUID(), &Member::guid);
    return member != plan.members.end() && RearReserveAvailable(plan, bosses, candidate, corner, *member);
}

Player* FindRearReserve(Player& bot, RaidPlan const& plan, Bosses const& bosses, uint8 corner,
                        ObjectGuid skip = ObjectGuid::Empty)
{
    ObjectGuid const preferred = plan.openingHolders[corner - BlaumeuxCorner];
    Player* closest = nullptr;
    for (Member const& member : plan.members)
    {
        if (member.guid == skip)
            continue;
        Player* const candidate = HolderInRaid(bot, member.guid);
        if (!RearReserveAvailable(plan, bosses, candidate, corner, member))
            continue;
        if (member.guid == preferred)
            return candidate;
        if (!closest || (IsTank(candidate) && !IsTank(closest)) ||
            (IsTank(candidate) == IsTank(closest) &&
             candidate->GetExactDist(bosses[corner]) < closest->GetExactDist(bosses[corner])))
            closest = candidate;
    }
    return closest;
}

void UpdateRearTankAssignments(Player& bot, RaidPlan& plan, Bosses const& bosses, uint32 now)
{
    if (plan.earlyRearSwapAt)
        return;

    struct Relief
    {
        Player* holder = nullptr;
        uint32 marks = 0;
        uint32 nextMark = 0;
        bool prepare = false;
        bool activate = false;
    };
    std::array<Relief, 2> relief{};
    bool const early = EarlyRearPhase(plan, bosses);
    uint32 const highestMarks = plan.earlyRearProbeMarks;
    for (uint8 corner : {BlaumeuxCorner, ZeliekCorner})
    {
        Creature const* boss = bosses[corner];
        if (!boss)
            continue;
        TankHandoff& handoff = plan.tankHandoffs[corner];
        if (handoff.boss != boss->GetGUID())
            handoff = {.boss = boss->GetGUID()};

        ObjectGuid& owner = plan.holders[corner];
        Player* const holder = HolderInRaid(bot, owner);
        Player* const incoming = HolderInRaid(bot, handoff.incoming);
        if (handoff.active && handoff.incoming != owner && CanHold(incoming) && boss->GetVictim() == incoming)
        {
            Player const* const healer = RearHandoffHealer(bot, plan, corner);
            bool const spread = corner != ZeliekCorner || OnlyZeliekAlive(bosses) ||
                                (healer && healer->GetExactDist2d(incoming) >= HolyWrathClearance);
            if (HasSustainedHorsemanAggro(*boss, *incoming) && healer && spread &&
                RearTankCanBeHealedBy(*healer, incoming))
            {
                ReleaseTank(plan, owner, corner);
                owner = handoff.incoming;
                handoff = {.boss = boss->GetGUID(), .releasedAt = now, .returning = true};
                LOG_INFO(
                    "playerbots",
                    "[Naxx40] Horsemen instance={} {} rear handoff confirmed for {}, former holder marks={}, healer={}",
                    bot.GetInstanceId(), Horsemen[corner].name, incoming->GetName(),
                    holder ? holder->GetAuraCount(Horsemen[corner].mark) : 0, healer->GetName());
            }
            continue;
        }
        if (!CanHold(holder))
        {
            RecoverHolder(bot, plan, bosses, corner);
            continue;
        }
        if (handoff.returning && AtCorner(boss, corner) &&
            holder->GetExactDist(Horsemen[corner].corner) <= RearCornerRange)
        {
            handoff.returning = false;
            LOG_INFO("playerbots", "[Naxx40] Horsemen instance={} {} back at {} corner on {} marks",
                     bot.GetInstanceId(), holder->GetName(), Horsemen[corner].name,
                     holder->GetAuraCount(Horsemen[corner].mark));
        }
        if (boss->GetVictim() != holder)
            continue;

        uint32 const marks = holder->GetAuraCount(Horsemen[corner].mark);
        bool const urgent = !early && holder->GetHealthPct() <= sPlayerbotAIConfig.LowHealth;
        bool const blocked = early && (plan.earlyRearBlocked & (1u << corner));
        bool const prepare =
            early ? blocked && highestMarks >= RearPrepareStacks : marks >= RearPrepareStacks || urgent;
        bool const activate = early ? blocked && highestMarks >= RearSwapStacks : marks >= RearSwapStacks || urgent;
        if ((!prepare && !handoff.active) ||
            (!handoff.incoming.IsEmpty() && !RearReserveAvailable(plan, bosses, incoming, corner)))
        {
            ReleaseTank(plan, handoff.incoming, corner);
            handoff = {.boss = boss->GetGUID(), .releasedAt = handoff.releasedAt, .returning = handoff.returning};
        }
        relief[corner - BlaumeuxCorner] = {holder, marks, TimeToRearMark(*holder, corner), prepare, activate};
    }

    std::array<uint8, 2> order = {BlaumeuxCorner, ZeliekCorner};
    Relief const& left = relief[0];
    Relief const& right = relief[1];
    if (right.prepare &&
        (!left.prepare || (right.activate && !left.activate) ||
         (right.activate == left.activate &&
          (right.nextMark < left.nextMark ||
           (right.nextMark == left.nextMark && right.holder->GetHealthPct() < left.holder->GetHealthPct())))))
        std::swap(order[0], order[1]);

    for (uint8 corner : order)
    {
        Relief const& request = relief[corner - BlaumeuxCorner];
        TankHandoff& handoff = plan.tankHandoffs[corner];
        if (!request.prepare)
            continue;
        if (!handoff.incoming.IsEmpty() && !handoff.approach && handoff.preparedAt &&
            getMSTimeDiff(handoff.preparedAt, now) >= RearReserveRouteGrace)
        {
            handoff.preparedAt = now;
            if (Player const* const alternate = FindRearReserve(bot, plan, bosses, corner, handoff.incoming))
            {
                ReleaseTank(plan, handoff.incoming, corner);
                LOG_INFO("playerbots", "[Naxx40] Horsemen instance={} {} reserve {} has no approach, trying {}",
                         bot.GetInstanceId(), Horsemen[corner].name, handoff.incoming.ToString(), alternate->GetName());
                handoff.incoming = alternate->GetGUID();
                handoff.checkedAt = 0;
                handoff.tauntAt = 0;
            }
        }
        if (handoff.active)
            continue;
        if (handoff.incoming.IsEmpty())
        {
            if (handoff.checkedAt && getMSTimeDiff(handoff.checkedAt, now) < RearReliefRetryInterval)
                continue;
            handoff.checkedAt = now;
            Player const* incoming = FindRearReserve(bot, plan, bosses, corner);
            if (!incoming && corner == order[0])
            {
                uint8 const other = corner ^ 1;
                TankHandoff& staged = plan.tankHandoffs[other];
                bool const blocked = request.activate && !staged.approach && !staged.tauntAt && staged.preparedAt &&
                                     getMSTimeDiff(staged.preparedAt, now) >= RearReserveRouteGrace;
                if ((!staged.active || blocked) && !staged.incoming.IsEmpty())
                {
                    ObjectGuid const candidate = staged.incoming;
                    staged.incoming.Clear();
                    Player* const tank = HolderInRaid(bot, candidate);
                    if (RearReserveAvailable(plan, bosses, tank, corner))
                    {
                        incoming = tank;
                        staged = {.boss = staged.boss, .releasedAt = staged.releasedAt, .returning = staged.returning};
                    }
                    else
                        staged.incoming = candidate;
                }
            }
            if (!incoming)
            {
                LOG_INFO("playerbots", "[Naxx40] Horsemen instance={} {} rear handoff pending, no eligible reserve",
                         bot.GetInstanceId(), Horsemen[corner].name);
                continue;
            }
            handoff.incoming = incoming->GetGUID();
            handoff.checkedAt = 0;
            handoff.preparedAt = now;
            LOG_INFO("playerbots", "[Naxx40] Horsemen instance={} {} prepares {} to relieve {} on {} marks",
                     bot.GetInstanceId(), Horsemen[corner].name, incoming->GetName(), request.holder->GetName(),
                     request.marks);
        }
        if (request.activate)
        {
            handoff.active = true;
            LOG_INFO("playerbots", "[Naxx40] Horsemen instance={} {} activates {} rear handoff on {} marks for {}",
                     bot.GetInstanceId(), Horsemen[corner].name, early ? "fallback" : "scheduled", request.marks,
                     handoff.incoming.ToString());
        }
    }
}

void UpdateTankAssignments(Player& bot, RaidPlan& plan, Bosses const& bosses, uint32 now)
{
    UpdateEarlyRearSwap(bot, plan, bosses, now);
    for (uint8 corner = 0; corner < CornerCount; ++corner)
    {
        if (bosses[corner])
            continue;
        ReleaseTank(plan, plan.holders[corner], corner);
        ReleaseTank(plan, plan.tankHandoffs[corner].incoming, corner);
        plan.holders[corner].Clear();
        plan.tankHandoffs[corner] = {};
    }

    for (uint8 corner = 0; corner < BlaumeuxCorner; ++corner)
    {
        Creature const* const boss = bosses[corner];
        if (!boss)
            continue;

        TankHandoff& handoff = plan.tankHandoffs[corner];
        if (handoff.boss != boss->GetGUID())
            handoff = {.boss = boss->GetGUID()};

        ObjectGuid& owner = plan.holders[corner];
        Player* const holder = HolderInRaid(bot, owner);
        if (!CanHold(holder))
        {
            RecoverHolder(bot, plan, bosses, corner);
            continue;
        }
        ObjectGuid const reserve = ScheduledFrontReserve(plan, bosses, corner, owner);
        Player* const incoming = HolderInRaid(bot, handoff.incoming);
        bool const acquired = handoff.tauntAt && CanHold(incoming) && boss->GetVictim() == incoming;
        if (acquired)
        {
            if (!HasSustainedHorsemanAggro(*boss, *incoming))
                continue;

            ReleaseTank(plan, owner, corner);
            owner = handoff.incoming;
            handoff = {.boss = boss->GetGUID(), .releasedAt = now};
            LOG_INFO("playerbots", "[Naxx40] Horsemen instance={} {} tank handoff confirmed for {}",
                     bot.GetInstanceId(), Horsemen[corner].name, incoming->GetName());
            continue;
        }

        if (handoff.incoming == reserve && CanHold(incoming) && TankAvailable(plan, reserve, corner))
            continue;
        if (!handoff.incoming.IsEmpty())
        {
            ReleaseTank(plan, handoff.incoming, corner);
            handoff = {.boss = boss->GetGUID(), .releasedAt = handoff.releasedAt};
        }

        uint32 const stacks = holder->GetAuraCount(Horsemen[corner].mark);
        if (boss->GetVictim() != holder || !TankAvailable(plan, reserve, corner) ||
            !CanHold(HolderInRaid(bot, reserve)) || stacks < TankSwapStacks)
            continue;
        handoff.incoming = reserve;
        LOG_INFO("playerbots", "[Naxx40] Horsemen instance={} {} requests tank handoff at {} marks on {}",
                 bot.GetInstanceId(), Horsemen[corner].name, stacks, holder->GetName());
    }
    UpdateRearTankAssignments(bot, plan, bosses, now);
}

void UpdateZeliekMelee(Player& bot, RaidPlan& plan, Bosses const& bosses)
{
    if (!OnlyZeliekAlive(bosses))
        return;

    Creature const& zeliek = *bosses[ZeliekCorner];
    std::array<FourHorsemenMarkSource, CornerCount> markSources;
    constexpr uint8 allMarks = (1u << CornerCount) - 1;
    uint8 const markCount = CollectProtectedMarks(bot, bosses, allMarks, markSources);
    auto clearance = [&](Position const& position)
    {
        float result = std::numeric_limits<float>::max();
        for (uint8 i = 0; i < markCount; ++i)
            result = std::min(result, position.GetExactDist2d(&markSources[i].location) - markSources[i].radius);
        return result;
    };
    plan.zeliekMeleeWait = clearance(RoomCenter) > clearance(WaitingPosition) ? RoomCenter : WaitingPosition;

    auto eligible = [&](Player* player)
    {
        if (!IsAutonomousBot(player) || IsTank(player) || PlayerbotAI::IsHeal(player) ||
            PlayerbotAI::IsRangedDps(player))
            return false;
        ObjectGuid const guid = player->GetGUID();
        return std::ranges::find(plan.holders, guid) == plan.holders.end() &&
               std::ranges::find(plan.openingHolders, guid) == plan.openingHolders.end() &&
               std::ranges::find(plan.tankHandoffs, guid, &TankHandoff::incoming) == plan.tankHandoffs.end() &&
               zeliek.GetVictim() != player;
    };
    for (ObjectGuid& slot : plan.zeliekMelee)
    {
        Player* const player = PlayerInRaid(bot, slot);
        if (!eligible(player) || (DpsMarksToClear(*player, bosses, 0) &&
                                  player->GetExactDist(&zeliek) > player->GetMeleeRange(&zeliek) + CornerArrival))
            slot.Clear();
    }
    for (ObjectGuid& slot : plan.zeliekMelee)
    {
        if (!slot.IsEmpty())
            continue;
        for (std::size_t count = 0; count < plan.members.size(); ++count)
        {
            Member const& member = plan.members[plan.nextZeliekMelee++ % plan.members.size()];
            Player* const player = PlayerInRaid(bot, member.guid);
            if (member.home == NoCorner || !eligible(player) ||
                std::ranges::find(plan.zeliekMelee, member.guid) != plan.zeliekMelee.end() ||
                player->HasAura(Horsemen[ZeliekCorner].mark) || DpsMarksToClear(*player, bosses, member.damageMarks))
                continue;
            slot = member.guid;
            LOG_INFO("playerbots", "[Naxx40] Horsemen instance={} {} takes a Zeliek melee slot", bot.GetInstanceId(),
                     player->GetName());
            break;
        }
    }
}

std::vector<Unit const*> UpdateZeliekWrathChain(ZeliekWrathSnapshot& wrath, Creature& zeliek)
{
    Unit* const victim = zeliek.GetVictim();
    SpellInfo const* const spell = sSpellMgr->GetSpellInfo(ZeliekHolyWrath);
    if (!spell || !victim)
        return {};

    // Holy Wrath uses a damaging magic chain that starts at Zeliek's victim.
    SpellEffectInfo const& effect = spell->GetEffect(EFFECT_0);
    float const jumpRadius = spell->JumpDistance > 0.0f ? spell->JumpDistance : ZeliekWrathDefaultJump;
    if (effect.ChainTarget <= 1)
        return {};

    float const searchRadius = jumpRadius * (effect.ChainTarget - 1);
    std::list<WorldObject*> nearby;
    Acore::WorldObjectSpellAreaTargetCheck check(
        searchRadius, victim, &zeliek, &zeliek, spell, effect.TargetA.GetCheckType(), effect.ImplicitTargetConditions,
        Acore::WorldObjectSpellAreaTargetSearchReason::Chain);
    Acore::WorldObjectListSearcher<Acore::WorldObjectSpellAreaTargetCheck> searcher(
        &zeliek, nearby, check, GRID_MAP_TYPE_MASK_PLAYER | GRID_MAP_TYPE_MASK_CREATURE);
    Cell::VisitObjects(victim->GetPositionX(), victim->GetPositionY(), zeliek.GetMap(), searcher, searchRadius);

    std::vector<Unit const*> units{victim};
    for (WorldObject const* object : nearby)
        if (Unit const* unit = object->ToUnit(); unit && unit != victim && unit->IsAlive())
            units.push_back(unit);
    std::sort(units.begin() + 1, units.end(),
              [](Unit const* left, Unit const* right) { return left->GetGUID() < right->GetGUID(); });

    if (wrath.jumpRadius != jumpRadius || wrath.targets.size() != units.size() ||
        !std::ranges::equal(wrath.targets, units, {}, &ZeliekWrathTarget::guid,
                            [](Unit const* unit) { return unit->GetGUID(); }))
        wrath.targets.assign(units.size(), {});
    wrath.maxTargets = effect.ChainTarget;
    wrath.jumpRadius = jumpRadius;
    for (std::size_t index = 0; index < units.size(); ++index)
    {
        Unit const& unit = *units[index];
        ZeliekWrathTarget& target = wrath.targets[index];
        target.guid = unit.GetGUID();
        std::array const geometry{unit.GetPositionX(),  unit.GetPositionY(),       unit.GetPositionZ(),
                                  unit.GetObjectSize(), unit.GetCollisionHeight(), unit.GetCombatReach()};
        if (target.geometry == geometry)
            continue;

        // A move can add or remove jumps in both directions, even for a previously unreachable target.
        target.geometry = geometry;
        target.jumps.clear();
        for (ZeliekWrathTarget& origin : wrath.targets)
            if (!origin.jumps.empty())
                origin.jumps[index] = ZeliekWrathJump::Unknown;
    }
    return units;
}

bool CanJumpZeliekWrath(ZeliekWrathSnapshot& wrath, std::span<Unit const* const> units, std::size_t origin,
                        std::size_t next)
{
    Unit const& source = *units[origin];
    Unit const& target = *units[next];
    if (source.GetTransport() || target.GetTransport())
        return source.IsWithinDist(&target, wrath.jumpRadius) &&
               source.IsWithinLOSInMap(&target, VMAP::ModelIgnoreFlags::M2);

    auto& jumps = wrath.targets[origin].jumps;
    if (jumps.empty())
        jumps.resize(units.size(), ZeliekWrathJump::Unknown);
    ZeliekWrathJump& jump = jumps[next];
    if (jump == ZeliekWrathJump::Unknown)
        jump = source.IsWithinDist(&target, wrath.jumpRadius) &&
                       source.IsWithinLOSInMap(&target, VMAP::ModelIgnoreFlags::M2, LINEOFSIGHT_CHECK_VMAP)
                   ? ZeliekWrathJump::Clear
                   : ZeliekWrathJump::Blocked;

    // Game objects can block or unblock a jump while both units stand still.
    return jump == ZeliekWrathJump::Clear &&
           source.IsWithinLOSInMap(&target, VMAP::ModelIgnoreFlags::M2, LINEOFSIGHT_CHECK_GOBJECT_ALL);
}

std::optional<Position> RearOpeningHealPosition(Player& healer, RaidPlan const& plan, uint8 corner, uint8 slot)
{
    std::size_t const side = corner - 2;
    if (!plan.rearReleased || !plan.reached[corner])
        return OpeningFlanks[side].healers[slot];

    Player const* tank = HolderInRaid(healer, plan.holders[corner]);
    if (!tank)
        return {};

    uint8 const otherSlot = slot ^ 1;
    Player* other = nullptr;
    for (OpeningMember const& staged : plan.opening->members)
        if (staged.arrived && staged.corner == corner && staged.healerSlot == otherSlot)
        {
            auto const assignment = std::find_if(plan.members.begin(), plan.members.end(),
                                                 [&staged, corner, otherSlot](Member const& member)
                                                 {
                                                     return member.guid == staged.guid &&
                                                            member.assignedCorner == corner &&
                                                            member.assignedSlot == otherSlot && !member.clearingMarks;
                                                 });
            if (assignment != plan.members.end())
                other = PlayerInRaid(healer, staged.guid);
            break;
        }

    auto combatPadFor = [side](uint8 healerSlot) -> Position const&
    { return side == 1 && healerSlot == 0 ? ZeliekOpeningHealPad : RearSupport[side][healerSlot]; };
    bool const otherCovers =
        CanSustainHealingCoverage(other) && !other->isMoving() && RearTankCanBeHealedBy(*other, tank);
    bool const canHeal = RearTankCanBeHealedBy(healer, tank);
    bool const otherSettled = otherCovers && other->GetExactDist(combatPadFor(otherSlot)) <= CornerArrival;
    if (otherSettled || (slot == 0 && canHeal && otherCovers))
        return combatPadFor(slot);
    if (canHeal && healer.GetExactDist(tank) <= HealerCoverageRange - CornerArrival)
        return healer.GetPosition();

    return combatPadFor(slot);
}

Assignment AssignmentFor(Player& bot, Bosses const& bosses, Member*& plannedMember)
{
    uint32 const now = getMSTime();
    RaidPlan& plan = *bot.GetMap()->CustomData.GetDefault<RaidPlan>(PlanKey);
    bool const openingPull = plan.opening && !plan.opening->pulled;
    bool const fullRoster = std::ranges::all_of(bosses, [](Creature const* boss) { return boss != nullptr; });
    bool const riderReturned = fullRoster && plan.individualArmed;
    if (plan.group != bot.GetGroup()->GetGUID() ||
        (!openingPull && (riderReturned || (fullRoster && now - plan.sampledAt > RetryDelay))))
    {
        if (!StartPlan(bot, plan))
            return {};
        if (!fullRoster)
            for (Member& member : plan.members)
                member.centerPending = true;
    }
    if (!plan.combatStartedAt)
        plan.combatStartedAt = now;
    if (plan.opening && !plan.opening->pulled)
    {
        FinishOpeningTags(bot, plan, bosses);
        plan.opening->pulled = true;
        for (OpeningMember const& member : plan.opening->members)
            if (Player* player = PlayerInRaid(bot, member.guid); IsAutonomousBot(player))
            {
                if (!player->movespline->Finalized() && player->movespline->GetId() == member.spline)
                    player->StopMoving();
                PlayerbotAI* ai = GET_PLAYERBOT_AI(player);
                ai->GetAiObjectContext()->GetValue<LastMovement&>("last movement")->Get().clear();
            }
        LOG_INFO("playerbots", "[Naxx40] Horsemen opening instance={} entered combat (flank readiness {})",
                 bot.GetInstanceId(), plan.opening->readyFlanks);
    }
    JoinLivePlan(bot, plan);

    if (!plan.sampledAt || now - plan.sampledAt >= SnapshotInterval)
    {
        plan.sampledAt = now;
        MeasureShadowProtection(bot, plan.rearProtection);
        UpdateEarlyRearHealing(bot, plan, bosses);
        UpdateTankAssignments(bot, plan, bosses, now);
        if (!plan.individualArmed)
            for (uint8 corner = 0; corner < CornerCount; ++corner)
                if (!bosses[corner])
                    if (Creature const* spirit = FindEncounterCreature(bot, CornerCount + corner, MarkSpirits[corner]);
                        spirit && spirit->IsInWorld() && spirit->IsAlive())
                    {
                        plan.individualArmed = true;
                        LOG_INFO("playerbots", "[Naxx40] Horsemen instance={} individual marks enabled after {} fell",
                                 bot.GetInstanceId(), Horsemen[corner].name);
                        break;
                    }
        if (!plan.rearReleased)
        {
            bool ownedBelowPlatform = true;
            for (uint8 i = 0; i < CornerCount; ++i)
                ownedBelowPlatform &= OwnsTarget(bosses[i], plan.holders[i]) &&
                                      (!bosses[i] || bosses[i]->GetPositionZ() <= PlatformExitZ);
            plan.rearReleased = ownedBelowPlatform;
            if (plan.rearReleased)
                LOG_INFO("playerbots", "[Naxx40] Horsemen instance={} platform clear, releasing rear support",
                         bot.GetInstanceId());
        }
        UpdateZeliekMelee(bot, plan, bosses);
        AllocateSupport(bot, plan, bosses);
        for (uint8 corner = 0; corner < CornerCount; ++corner)
        {
            Creature* boss = bosses[corner];
            if (plan.reached[corner] || !boss || !AtCorner(boss, corner))
                continue;
            plan.reached[corner] = true;
            LOG_INFO("playerbots", "[Naxx40] Horsemen instance={} {} reached its corner at ({:.1f}, {:.1f}, {:.2f})",
                     bot.GetInstanceId(), Horsemen[corner].name, boss->GetPositionX(), boss->GetPositionY(),
                     boss->GetPositionZ());
        }
    }

    auto member = std::find_if(plan.members.begin(), plan.members.end(),
                               [&bot](Member const& candidate) { return candidate.guid == bot.GetGUID(); });
    if (member == plan.members.end() || member->home == NoCorner)
        return {};
    plannedMember = &*member;

    ObjectGuid const guid = bot.GetGUID();
    if (HasDedicatedRearHealing(plan) && IsEarlyRearHealer(plan, guid))
    {
        auto const binding = std::ranges::find_if(plan.earlyRearHealing, [guid](EarlyRearHealerAssignment const& row)
                                                  { return row.healer == guid || row.reserve == guid; });
        bool const reserve = binding->reserve == guid;
        ObjectGuid const tank = reserve ? binding->reserveTarget : RearHealingPatient(bot, plan, bosses, *binding);
        uint8 const marks = reserve && tank.IsEmpty() ? MarksToClear(bot, 1) : 0;
        uint8 const corner = binding->corner;
        return {.corner = corner,
                .standby = tank.IsEmpty() || marks,
                .clearingMarks = marks,
                .sideHealer = !tank.IsEmpty() && !marks,
                .healingTarget = marks ? ObjectGuid::Empty : tank,
                .earlyRearHealer = true,
                .dedicatedRearHealer = !reserve,
                .reserveRearHealer = reserve};
    }

    for (uint8 corner = 0; corner < CornerCount; ++corner)
        if (plan.tankHandoffs[corner].incoming == guid)
            return {.corner = corner,
                    .holder = true,
                    .targetHolder = plan.holders[corner],
                    .managedTank = true,
                    .tankIncoming = true,
                    .tankReleasedMarks = MarksToClear(bot, RotationStacks, member->releasedTankMarks)};

    auto const held = std::ranges::find(plan.holders, guid);
    bool const holder = held != plan.holders.end();
    member->releasedTankMarks = MarksToClear(bot, std::numeric_limits<uint32>::max(), member->releasedTankMarks);
    bool const openingTank = std::ranges::find(plan.openingHolders, guid) != plan.openingHolders.end();
    if (!holder && (openingTank || member->releasedTankMarks))
    {
        uint8 destination = member->home;
        for (uint8 corner = 0; corner < CornerCount; ++corner)
            if (bosses[corner] && ScheduledFrontReserve(plan, bosses, corner, plan.holders[corner]) == guid)
            {
                destination = corner;
                break;
            }
        return {.corner = destination,
                .standby = true,
                .managedTank = true,
                .tankStandby = true,
                .tankReleasedMarks = MarksToClear(bot, RotationStacks, member->releasedTankMarks)};
    }

    uint8 const homeCorner = member->home;
    bool const standby = !holder && member->assignedCorner == NoCorner;
    uint8 const receivingHome = member->reinforcementHome;
    bool const followsReceiving = !holder && receivingHome != NoCorner;
    uint8 const receivingCorner = followsReceiving ? receivingHome : homeCorner;
    uint8 corner = holder ? static_cast<uint8>(held - plan.holders.begin())
                          : standby ? receivingCorner : member->assignedCorner;
    bool const earlyRearSwap = holder && corner >= BlaumeuxCorner && plan.earlyRearSwapAt;
    if (earlyRearSwap)
        corner ^= 1;
    bool const earlyRearHealer = !HasDedicatedRearHealing(plan) && !holder && PlayerbotAI::IsHeal(&bot) &&
                                 EarlyRearPhase(plan, bosses) &&
                                 (corner >= BlaumeuxCorner || member->home >= BlaumeuxCorner ||
                                  (member->centerDestination >= BlaumeuxCorner &&
                                   member->centerDestination < CornerCount));
    std::optional<Position> stagedHealer;
    bool stagedTank = false;
    if (plan.opening && corner >= 2)
        for (OpeningMember const& prepared : plan.opening->members)
            if (prepared.arrived && prepared.guid == member->guid && prepared.corner == corner)
            {
                if (!holder && !standby && !member->clearingMarks && prepared.healerSlot < 2 &&
                    member->assignedSlot == prepared.healerSlot && !plan.tankHandoffs[corner].releasedAt &&
                    now - plan.combatStartedAt < OpeningRearHealingWindow)
                    stagedHealer = RearOpeningHealPosition(bot, plan, corner, prepared.healerSlot);
                stagedTank =
                    holder && prepared.healerSlot == NoSupportSlot && (!plan.rearReleased || !plan.reached[corner]);
                break;
            }
    bool const openingThreat = holder && corner == ZeliekCorner && now - plan.combatStartedAt < OpeningThreatWindow;
    bool const frontWarrior = holder && corner < 2 && bot.getClass() == CLASS_WARRIOR;
    uint32 const frontOpeningAt =
        frontWarrior && plan.opening && plan.opening->pulled && now - plan.combatStartedAt < FrontOpeningThreatWindow
            ? plan.combatStartedAt
            : 0;
    uint32 const releasedAt = plan.tankHandoffs[corner].releasedAt;
    bool const frontHandoffThreat = frontWarrior && releasedAt && now - releasedAt < FrontHandoffThreatWindow;
    ObjectGuid healingTarget;
    if (!holder && !standby && PlayerbotAI::IsHeal(&bot))
    {
        if (corner == MograineCorner && plan.mograineHealing.healer == member->guid)
            healingTarget = plan.mograineHealing.target;
        else if (corner >= BlaumeuxCorner)
        {
            TankHealerAssignment const& coverage = plan.rearHealing[corner - BlaumeuxCorner];
            healingTarget = coverage.healer == member->guid ? coverage.target : member->coveringTank;
        }
    }
    return {.corner = corner,
            .supportSlot = member->assignedSlot,
            .holder = holder,
            .opening = corner >= BlaumeuxCorner && !plan.rearReleased,
            .targetHolder = plan.holders[corner],
            .standby = standby,
            .clearingMarks = member->clearingMarks,
            .individualArmed = plan.individualArmed,
            .stagedHealer = stagedHealer,
            .stagedTank = stagedTank,
            .openingThreat = openingThreat,
            .sideHealer = !healingTarget.IsEmpty(),
            .healingTarget = healingTarget,
            .frontOpeningAt = frontOpeningAt,
            .frontHandoffThreat = frontHandoffThreat,
            .managedTank = holder,
            .earlyRearSwap = earlyRearSwap,
            .earlyRearHealer = earlyRearHealer};
}

class AssignHorsemenAction final : public MovementAction
{
    class HorsemenAttack final : public AttackAction
    {
    public:
        explicit HorsemenAttack(PlayerbotAI* ai) : AttackAction(ai, JobName(Job::Attack)) {}
        using AttackAction::Attack;
    };

    enum class MeteorResponse
    {
        Ignore,
        Hold,
        Move
    };

    enum class RearHealCast
    {
        Normal,
        StopIfNeeded,
        InstantOnly
    };

    struct TickPlan
    {
        Assignment assignment;
        std::array<ObjectGuid, CornerCount> bosses;
        ObjectGuid target;
        Position position;
        std::array<FourHorsemenMarkSource, CornerCount + 4> protectedMarks;
        uint8 protectedMarkCount = 0;
        bool dynamicDamage = false;
        bool movingToWaitingPosition = false;
        bool zeliekRanged = false;
        std::array<Position, 2> const* zeliekLane = nullptr;
        bool returningFromZeliek = false;
        std::optional<std::size_t> zeliekRaidHealPad;
        bool markRest = false;
        bool centerMovement = false;
        bool centerRetreat = false;
        uint8 centerOrigin = NoCorner;
        bool ready = false;
        bool finished = false;
    };

public:
    explicit AssignHorsemenAction(PlayerbotAI* ai) : MovementAction(ai, JobName(Job::Assign)), _attack(ai), _hazards(ai)
    {
    }

    bool isUseful() override { return ActiveEncounter(bot) && bot->IsAlive() && bot->IsInCombat(); }

    bool Execute(Event /*event*/) override
    {
        _tick = {};
        _zeliekWrathAvoid.clear();
        InstanceScript* instance = ActiveEncounter(bot);
        if (!instance || !bot->IsAlive() || !bot->IsInCombat() ||
            !EncounterHelpers::IsEncounterInProgress(bot, NaxxramasMap))
            return false;

        Bosses const bosses = ReadBosses(bot, *instance);
        Member* plannedMember = nullptr;
        Assignment const& assignment = _tick.assignment = AssignmentFor(*bot, bosses, plannedMember);
        _corner = assignment.corner;
        _holder = assignment.holder;
        _fighting = false;
        if (!plannedMember || _corner == NoCorner)
            return false;
        bool const soloZeliek = OnlyZeliekAlive(bosses);
        if (!soloZeliek)
            _zeliekEnteredLane = nullptr;
        bool const raidHealing = soloZeliek && !_holder && !assignment.sideHealer && !assignment.earlyRearHealer &&
                                 PlayerbotAI::IsHeal(bot);
        if (!raidHealing)
            plannedMember->zeliekRaidHealPosition.reset();
        if (soloZeliek && assignment.managedTank)
            _tick.zeliekLane = &ZeliekMeleeLane;

        for (uint8 index = 0; index < CornerCount; ++index)
            if (bosses[index])
                _tick.bosses[index] = bosses[index]->GetGUID();

        bool const ownsLiveRider = assignment.managedTank &&
                                  std::ranges::any_of(bosses, [&](Creature const* rider)
                                  { return rider && rider->GetVictim() == bot; });
        if (assignment.tankStandby && ownsLiveRider)
        {
            _tick.position = bot->GetPosition();
            _tick.ready = true;
            _tick.markRest = true;
            return false;
        }
        bool const earlyRearTank = _holder && _corner >= BlaumeuxCorner && EarlyRearPhase(*GetPlan(*bot), bosses);
        bool const rearRelief = assignment.tankIncoming && _corner >= BlaumeuxCorner && !assignment.tankReleasedMarks &&
                                !plannedMember->centerMarks;
        if (rearRelief)
        {
            plannedMember->centerPending = false;
            plannedMember->centerInitialized = true;
            plannedMember->centerDestination = _corner;
        }
        if (assignment.managedTank && !ownsLiveRider && !earlyRearTank && !rearRelief &&
            PrepareCenter(*plannedMember, assignment.corner, assignment.tankReleasedMarks, assignment.tankStandby))
        {
            _tick.returningFromZeliek = soloZeliek && (_tick.centerOrigin == ZeliekCorner ||
                                                     (assignment.tankReleasedMarks & (1u << ZeliekCorner)));
            return false;
        }
        if (assignment.managedTank && (!plannedMember->centerInitialized || earlyRearTank) && !assignment.earlyRearSwap)
        {
            plannedMember->centerInitialized = true;
            plannedMember->centerDestination = assignment.corner;
        }
        if (assignment.reserveRearHealer && assignment.standby &&
            PrepareCenter(*plannedMember, NoCorner, assignment.clearingMarks, true))
            return false;
        if (assignment.earlyRearHealer)
        {
            if (!assignment.standby)
            {
                plannedMember->centerInitialized = true;
                plannedMember->centerDestination = _corner;
            }
            plannedMember->centerPending = false;
            plannedMember->centerMarks = 0;
        }
        if (!_holder && PlayerbotAI::IsHeal(bot) && !assignment.earlyRearHealer)
        {
            uint8 const destination = raidHealing ? ZeliekCorner : (assignment.standby ? NoCorner : _corner);
            bool const standby = !raidHealing && assignment.standby && plannedMember->centerInitialized;
            if (PrepareCenter(*plannedMember, destination, assignment.clearingMarks, standby))
            {
                plannedMember->zeliekRaidHealPosition.reset();
                return false;
            }
            if (raidHealing)
            {
                PlanZeliekRaidHealing(*plannedMember, bosses);
                return false;
            }
        }
        if (!_holder && !PlayerbotAI::IsHeal(bot))
        {
            PlanDps(*plannedMember, bosses);
            return false;
        }

        uint8 const corner = _corner;
        bool const holder = _holder;
        _tick.markRest = assignment.earlyRearHealer && assignment.standby && assignment.clearingMarks;
        _tick.ready = true;
        if (_tick.markRest)
        {
            DropAttack();
            _tick.position = bot->GetPosition();
            return false;
        }

        bool const standby = assignment.standby;
        Creature* const target = standby ? nullptr : bosses[corner];
        bool const pairAlive = target || bosses[corner ^ 1];
        bool const rearHolder = holder && corner >= 2;
        uint8 const supportCorner = corner;
        Position const& cornerSpot = !holder && corner >= 2 && assignment.supportSlot < RearSupport[0].size()
                                         ? RearSupport[supportCorner - 2][assignment.supportSlot]
                                         : Horsemen[corner].corner;
        Position position = !holder && assignment.opening ? OpeningCamp
                            : !standby && pairAlive       ? cornerSpot
                                                          : WaitingPosition;
        if (holder && corner < 2 && pairAlive)
            position = FrontHolderPads[corner];
        if (!holder && corner < 2 && !standby && pairAlive)
            position = FrontSupportPosition(corner, bot);
        if (assignment.stagedHealer)
            position = *assignment.stagedHealer;
        if (assignment.sideHealer)
            position = bot->GetPosition();
        else if (!holder && target && corner == 2 && assignment.supportSlot == NoSupportSlot &&
                 AtCorner(target, corner))
            position = assignment.individualArmed ? SpacedBlaumeuxPosition(target->GetPosition(), 3.0f)
                                                  : target->GetPosition();
        if (rearHolder && pairAlive)
        {
            position = RearDestination(corner, RearMarkSource(bot, bosses, corner ^ 1));
            if (!_lastRearDestination || _lastRearDestination->GetExactDist(position) > CornerArrival)
            {
                _lastRearDestination = position;
                LOG_INFO("playerbots",
                         "[Naxx40] Horsemen rear instance={} {} routes to {} pad ({:.2f}, {:.2f}, {:.2f})",
                         bot->GetInstanceId(), bot->GetName(), Horsemen[corner].name, position.GetPositionX(),
                         position.GetPositionY(), position.GetPositionZ());
            }
        }
        if (holder && target && RearFocus(bosses) == corner && target->GetVictim() == bot && AtCorner(target, corner))
            position = soloZeliek && !InZeliekTankFlank(*bot, *target) ? Horsemen[corner].corner : bot->GetPosition();
        if (holder && target)
        {
            Unit const* const victim = bot->GetVictim();
            Unit const* const cachedTarget = AI_VALUE(Unit*, "current target");
            if ((victim && victim != target) || (cachedTarget && cachedTarget != target))
                DropAttack();
        }
        if (holder && target)
        {
            TankHandoff const& handoff = GetPlan(*bot)->tankHandoffs[corner];
            bool const takingOver = handoff.tauntAt && target->GetVictim() &&
                                    target->GetVictim()->GetGUID() == handoff.incoming;
            _fighting = assignment.earlyRearSwap ? target->GetVictim() == bot && bot->IsWithinMeleeRange(target)
                        : takingOver             ? target->GetVictim() == bot
                                                 : !assignment.tankIncoming;
            if (!_fighting)
                DropAttack();
        }
        else
            _fighting = target && AtCorner(target, corner) && OwnsTarget(target, assignment.targetHolder);
        _tick.position = position;
        if (target)
            _tick.target = target->GetGUID();
        return false;
    }

    bool IsUseful(Job job) const
    {
        if (!_tick.ready || _tick.finished || !ActiveEncounter(bot) || !bot->IsAlive() || !bot->IsInCombat())
            return false;
        if (_tick.centerRetreat)
            return job == Job::ClearMarks;
        if (job == Job::VoidZone &&
            (IsMovingAlongRearExchangeHealerPath() || _hazards.HasActiveRearExchangeExit(*bot) ||
             _hazards.HasActiveRearRenditionPath(*bot) || (_tick.centerMovement && InterruptedRearTankInPool())))
            return true;
        if (_tick.centerMovement)
            return job == Job::ClearMarks ||
                   (job == Job::VoidZone && !_tick.centerRetreat && _tick.centerOrigin >= BlaumeuxCorner &&
                    _tick.centerOrigin < CornerCount);
        if (_tick.markRest)
            return job == Job::VoidZone || job == Job::ClearMarks ||
                   (_tick.dynamicDamage && !_tick.assignment.clearingMarks && job == Job::Meteor);
        switch (job)
        {
            case Job::VoidZone:
                return true;
            case Job::Position:
                return !_tick.assignment.tankIncoming && !_tick.assignment.earlyRearSwap &&
                       (_tick.movingToWaitingPosition || !_tick.dynamicDamage ||
                        (!_fighting && !_tick.target.IsEmpty()));
            case Job::Meteor:
                return !_holder && (_tick.dynamicDamage ||
                                    (_corner < 2 && !_tick.assignment.standby && !_tick.assignment.clearingMarks));
            case Job::RearTankCoverage:
                return _tick.assignment.sideHealer;
            case Job::TankControl:
                return _holder && !_tick.target.IsEmpty();
            case Job::Attack:
                return _fighting && !_tick.target.IsEmpty() && !PlayerbotAI::IsHeal(bot);
            default:
                return false;
        }
    }

    bool Execute(Job job)
    {
        if (!IsUseful(job))
            return false;
        switch (job)
        {
            case Job::VoidZone:
                return AvoidVoidZone();
            case Job::ClearMarks:
                return _tick.centerMovement ? MoveThroughCenter() : ClearMarks();
            case Job::Meteor:
                return SoakMeteor();
            case Job::RearTankCoverage:
                return CoverRearTank();
            case Job::TankControl:
                return ControlTank();
            case Job::Position:
                return TakePosition();
            case Job::Attack:
                return AttackAssignedHorseman();
            default:
                return false;
        }
    }

    bool AllowsOffense(Unit const* target) const
    {
        if (_tick.centerMovement || !_fighting || !target || !target->IsAlive())
            return false;
        if (!_tick.dynamicDamage)
            return _corner != NoCorner && target->GetEntry() == Horsemen[_corner].entry;
        RaidPlan const* const plan = GetPlan(*bot);
        return target->GetGUID() == _tick.target && plan && DpsBossControlled(*plan, target->ToCreature(), _corner);
    }

    bool HoldingCorner() const { return _holder; }
    bool HoldingRearCorner() const { return _holder && _corner >= 2 && _corner < CornerCount; }
    bool DamageWaitingAtCenter() const { return _tick.dynamicDamage && _tick.centerMovement; }
    bool AllowsGroupHeal(Unit const* target) const
    {
        Assignment const& assignment = _tick.assignment;
        return (!assignment.dedicatedRearHealer && !assignment.reserveRearHealer) || target == bot ||
               (target && target->GetGUID() == assignment.healingTarget);
    }

private:
    bool PrepareCenter(Member& member, uint8 destination, uint8 marks, bool standby = false)
    {
        bool const firstAssignment = !member.centerInitialized;
        if (destination != NoCorner)
            member.centerInitialized = true;
        bool const changed = destination != NoCorner && member.centerDestination != destination;
        if (changed && !member.centerPending)
            member.centerOrigin = member.centerDestination;
        if (destination != NoCorner)
            member.centerDestination = destination;

        uint8 const previousMarks = member.centerMarks;
        member.centerMarks |= marks;
        for (uint8 corner = 0; corner < CornerCount; ++corner)
            if ((member.centerMarks & (1u << corner)) && !bot->HasAura(Horsemen[corner].mark))
                member.centerMarks &= ~(1u << corner);

        bool const startingRetreat = member.centerMarks && !previousMarks;
        bool const atCenter = bot->GetExactDist(TransitionCenter) <= CornerArrival;
        bool const startingStandby = standby && !atCenter && !member.centerPending;
        if (startingStandby && !changed)
            member.centerOrigin = member.centerDestination;
        if ((changed && !firstAssignment) || startingRetreat || startingStandby)
        {
            member.centerPending = true;
            _centerRouteBlocked = false;
            LogCenter("starts", member);
        }
        if (previousMarks && !member.centerMarks)
            LogCenter("marks expired", member);

        if (member.centerPending && atCenter)
        {
            member.centerPending = false;
            _centerRouteBlocked = false;
            LogCenter("arrived", member);
        }
        if (!member.centerPending && !member.centerMarks && !standby)
        {
            _centerRouteBlocked = false;
            return false;
        }

        if (!PlayerbotAI::IsHeal(bot))
            DropAttack();
        _fighting = false;
        _tick.ready = true;
        _tick.centerMovement = true;
        _tick.centerRetreat = member.centerMarks != 0;
        _tick.centerOrigin = member.centerOrigin;
        return true;
    }

    void LogCenter(char const* event, Member const& member) const
    {
        char const* destination =
            member.centerDestination < CornerCount ? Horsemen[member.centerDestination].name : "standby";
        LOG_INFO("playerbots",
                 "[Naxx40] Horsemen instance={} center {} {} destination={} marks={} ({:.2f}, {:.2f}, {:.2f})",
                 bot->GetInstanceId(), event, bot->GetName(), destination, member.centerMarks,
                 bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ());
    }

    bool MoveThroughCenter()
    {
        bool crossesVoid = false;
        if (_hazards.ContinueCenterRetreat(*bot, _tick.centerRetreat, crossesVoid))
            return crossesVoid || !PlayerbotAI::IsHeal(bot);

        bool const atCenter = bot->GetExactDist(TransitionCenter) <= CornerArrival;
        std::span<Position const> const lane = ZeliekRetreatLane();
        if (_hazards.HasActiveRearExchangeExit(*bot) || !_hazards.IsSafeFromVoidDamage(*bot, bot->GetPosition()))
        {
            if (_tick.centerRetreat && !atCenter && !_hazards.IsEscapingRearExchangeVoidZone(*bot))
            {
                for (Position const& waypoint : lane)
                    if (_hazards.WalkToFixedPoint(*bot, waypoint, CornerArrival, true, lane))
                        return true;
                if (_hazards.WalkToFixedPoint(*bot, TransitionCenter, CornerArrival, true))
                    return true;
            }
            return _hazards.EscapeRearExchangeVoidZone(*bot, atCenter ? nullptr : &TransitionCenter);
        }

        if (atCenter)
        {
            if (bot->isMoving())
            {
                bot->GetMotionMaster()->Clear();
                bot->StopMoving();
            }
            bool const unsafe = !_hazards.IsSafeFromVoidZones(*bot, bot->GetPosition());
            if (unsafe && !_centerRouteBlocked)
                LOG_INFO("playerbots", "[Naxx40] Horsemen instance={} center occupied by a Void Zone for {}",
                         bot->GetInstanceId(), bot->GetName());
            _centerRouteBlocked = unsafe;
            return false;
        }
        if (!_tick.centerRetreat && PlayerbotAI::IsHeal(bot))
            for (CurrentSpellTypes slot : {CURRENT_GENERIC_SPELL, CURRENT_CHANNELED_SPELL})
                if (Spell const* spell = bot->GetCurrentSpell(slot); spell && spell->GetSpellInfo()->IsPositive())
                    return false;

        // Merge the two flank routes only on the final approach to the fixed center.
        if (_tick.returningFromZeliek && _tick.zeliekLane)
        {
            if (_hazards.WalkRear(*bot, TransitionCenter, CornerArrival, true, 0.0f, {}, nullptr, true, lane))
                return true;
            if (_tick.centerRetreat)
                for (Position const& waypoint : lane)
                    if (_hazards.WalkToFixedPoint(*bot, waypoint, CornerArrival, true, lane))
                        return true;
        }

        uint32 const previousSpline = bot->movespline->GetId();
        bool const walking = _hazards.WalkToFixedPoint(*bot, TransitionCenter, CornerArrival, _tick.centerRetreat);
        if (!walking && !_centerRouteBlocked)
            LOG_INFO("playerbots", "[Naxx40] Horsemen instance={} center route blocked for {} ({:.2f}, {:.2f}, {:.2f})",
                     bot->GetInstanceId(), bot->GetName(),
                     bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ());
        _centerRouteBlocked = !walking;
        return walking && (!PlayerbotAI::IsHeal(bot) || bot->movespline->GetId() != previousSpline);
    }

    bool AvoidVoidZone()
    {
        if (_hazards.HasActiveRearExchangeExit(*bot) && _hazards.EscapeRearExchangeVoidZone(*bot))
            return true;
        if (InterruptedRearTank* const interrupted = InterruptedRearTankInPool())
        {
            if (_hazards.EscapeRearExchangeVoidZone(*bot))
                return true;
            StopRearExchangePath(bot, interrupted->spline);
            *interrupted = {};
        }
        if (_tick.assignment.earlyRearSwap)
        {
            if (_hazards.IsSafeFromVoidZones(*bot, bot->GetPosition()))
                return false;
            RaidPlan& plan = *GetPlan(*bot);
            uint8 const side = (_corner ^ 1) - BlaumeuxCorner;
            if (FollowingEarlyRearRoute(plan, side))
                return !_hazards.IsSafeFromVoidDamage(*bot, bot->GetPosition());
            return _hazards.EscapeRearExchangeVoidZone(*bot);
        }
        if (_hazards.HasActiveRearRenditionPath(*bot))
        {
            RaidPlan const& plan = *GetPlan(*bot);
            Player const* const healer =
                _tick.assignment.tankIncoming && _corner >= BlaumeuxCorner && _corner < CornerCount
                    ? RearHandoffHealer(*bot, plan, _corner)
                    : RearExchangeHealer(*bot, plan, *bot);
            FourHorsemenRearCrossing crossing{healer != nullptr, healer, HealerCoverageRange};
            if (_tick.zeliekLane && _tick.assignment.tankIncoming)
                if (Player const* holder = HolderInRaid(*bot, plan.holders[_corner]))
                {
                    crossing.secondsToArrive = std::max(1u, TimeToRearMark(*holder, _corner)) / 1000.0f;
                    crossing.allowDryApproachAcrossMark = true;
                }
            return _hazards.AvoidVoidOnRearRenditionPath(*bot, crossing);
        }
        if (_tick.assignment.tankIncoming && _corner >= BlaumeuxCorner && _corner < CornerCount &&
            !_hazards.IsSafeFromVoidDamage(*bot, bot->GetPosition()))
            return _hazards.EscapeRearExchangeVoidZone(*bot);
        if (!IsMovingAlongRearExchangeHealerPath())
            _earlyRearHealerSpline = 0;
        if (_earlyRearHealerSpline || _tick.assignment.dedicatedRearHealer || _tick.assignment.reserveRearHealer)
        {
            bool const helpingExchange = IsRearExchangeHealer(GetPlan(*bot));
            if (_earlyRearHealerSpline || helpingExchange)
                return AvoidRearExchangeHealerPool(helpingExchange);
        }
        if (_tick.assignment.reserveRearHealer && _tick.assignment.standby)
            return _hazards.EscapeVoidZones(*bot, TransitionCenter, VoidEscapeCornerRadius);
        uint8 const cornerIndex = _tick.centerMovement ? _tick.centerOrigin : _corner;
        if (cornerIndex >= BlaumeuxCorner && cornerIndex < CornerCount)
        {
            Position const& corner = !_tick.centerMovement && _tick.assignment.supportSlot < NoSupportSlot && !_holder
                                         ? RearSupport[cornerIndex - BlaumeuxCorner][_tick.assignment.supportSlot]
                                         : Horsemen[cornerIndex].corner;
            if (_tick.centerMovement && bot->GetExactDist(corner) > RearCornerRange)
                return false;
            return _hazards.EscapeCornerVoidZones(*bot, corner, TransitionCenter, RearCornerRange);
        }
        return _hazards.EscapeVoidZones(*bot, _tick.position, VoidEscapeCornerRadius);
    }

    Creature* Resolve(ObjectGuid guid) const { return guid.IsEmpty() ? nullptr : bot->GetMap()->GetCreature(guid); }

    InterruptedRearTank* InterruptedRearTankInPool() const
    {
        RaidPlan* const plan = GetPlan(*bot);
        if (!plan)
            return nullptr;
        auto const tank =
            std::ranges::find(plan->earlyRearInterruptedInPool, bot->GetGUID(), &InterruptedRearTank::guid);
        return tank != plan->earlyRearInterruptedInPool.end() ? &*tank : nullptr;
    }

    std::span<FourHorsemenMarkSource const> ProtectedMarks() const
    {
        if (!_zeliekWrathAvoid.empty())
            return _zeliekWrathAvoid;
        return {_tick.protectedMarks.data(), _tick.protectedMarkCount};
    }

    void PrepareZeliekWrathAvoidance(RaidPlan& plan, Creature& zeliek)
    {
        auto const marks = std::span(_tick.protectedMarks.data(), _tick.protectedMarkCount);
        _zeliekWrathAvoid.assign(marks.begin(), marks.end());
        ZeliekWrathSnapshot& wrath = plan.zeliekWrath;
        std::vector<Unit const*> const units = UpdateZeliekWrathChain(wrath, zeliek);
        if (units.empty() || units.front() == bot)
            return;

        // Remove this bot from the chain so its proposed move can break the connection to the players behind it.
        std::vector<uint32> hops(wrath.targets.size(), wrath.maxTargets);
        hops.front() = 0;
        std::vector<std::size_t> pending{0};
        for (std::size_t i = 0; i < pending.size(); ++i)
        {
            std::size_t const origin = pending[i];
            uint32 const nextHop = hops[origin] + 1;
            Unit const& source = *units[origin];
            float const radius = wrath.jumpRadius + source.GetObjectSize() + bot->GetObjectSize();
            _zeliekWrathAvoid.push_back({source.GetPosition(), radius, false, source.GetGUID()});
            if (nextHop == wrath.maxTargets - 1)
                continue;

            for (std::size_t next = 1; next < units.size(); ++next)
                if (hops[next] == wrath.maxTargets && units[next] != bot &&
                    CanJumpZeliekWrath(wrath, units, origin, next))
                {
                    hops[next] = nextHop;
                    pending.push_back(next);
                }
        }
    }

    void PlanZeliekRaidHealing(Member& member, Bosses const& bosses)
    {
        constexpr uint8 allMarks = (1u << CornerCount) - 1;
        _tick.protectedMarkCount = CollectProtectedMarks(*bot, bosses, allMarks, _tick.protectedMarks);
        RaidPlan& plan = *GetPlan(*bot);
        PrepareZeliekWrathAvoidance(plan, *bosses[ZeliekCorner]);
        auto const canUsePad = [&](Position const& pad, bool current)
        {
            float const margin =
                current && bot->GetExactDist(pad) <= DpsPositionArrival ? 0.0f : ZeliekWrathPositionMargin;
            if (!_hazards.IsSafeFromVoidZones(*bot, pad) ||
                FourHorsemen40Hazards::CanReceiveZeliekWrath(*bot, pad, ProtectedMarks(), margin) ||
                std::ranges::any_of(ProtectedMarks(), [&](FourHorsemenMarkSource const& mark)
                { return mark.isMark && pad.GetExactDist2d(&mark.location) < mark.radius + DpsPositionArrival; }))
                return false;

            for (Member const& other : plan.members)
            {
                if (other.guid == member.guid)
                    continue;
                Player* const player = PlayerInRaid(*bot, other.guid);
                if (!player || !player->IsAlive())
                    continue;
                // The center serves as the shared fallback when the individual posts are full.
                if (CanCoverZeliekRaid(player) && other.zeliekRaidHealPosition &&
                    other.zeliekRaidHealPosition->GetExactDist(&TransitionCenter) > CornerArrival &&
                    pad.GetExactDist2d(&*other.zeliekRaidHealPosition) < HolyWrathClearance)
                    return false;
            }
            return true;
        };

        uint32 const now = getMSTime();
        if (CanCoverZeliekRaid(bot))
            for (std::size_t index = 0; index < ZeliekRaidHealPads.size(); ++index)
            {
                Position const& pad = ZeliekRaidHealPads[index];
                bool const current = member.zeliekRaidHealPosition &&
                                     pad.GetExactDist(&*member.zeliekRaidHealPosition) <= DpsPositionArrival;
                if ((_tick.zeliekRaidHealPad && !current) ||
                    (_blockedZeliekHealAt[index] && getMSTimeDiff(_blockedZeliekHealAt[index], now) < DpsRouteRetry) ||
                    !canUsePad(pad, current))
                    continue;
                _tick.zeliekRaidHealPad = index;
                if (current)
                    break;
            }

        if (!_tick.zeliekRaidHealPad)
        {
            member.zeliekRaidHealPosition = TransitionCenter;
            PrepareCenter(member, ZeliekCorner, 0, true);
            return;
        }

        Position const& station = ZeliekRaidHealPads[*_tick.zeliekRaidHealPad];
        if (!member.zeliekRaidHealPosition || member.zeliekRaidHealPosition->GetExactDist(station) > DpsPositionArrival)
            LOG_INFO("playerbots", "[Naxx40] Horsemen instance={} {} takes a Zeliek raid healing position",
                     bot->GetInstanceId(), bot->GetName());
        member.zeliekRaidHealPosition = station;
        _tick.position = station;
        _tick.ready = true;
    }

    std::span<Position const> ZeliekApproachLane()
    {
        if (!_tick.zeliekLane)
        {
            _zeliekEnteredLane = nullptr;
            return {};
        }
        bool const onFlank = (bot->GetPositionX() > TransitionCenter.GetPositionX()) == _tick.zeliekRanged;
        bool const pastRamp = bot->GetPositionZ() <= PlatformExitZ &&
                              bot->GetPositionY() + DpsPositionArrival >= _tick.zeliekLane->front().GetPositionY();
        if (!pastRamp)
            _zeliekEnteredLane = nullptr;
        else if (onFlank)
            _zeliekEnteredLane = _tick.zeliekLane;
        return _zeliekEnteredLane == _tick.zeliekLane ? std::span<Position const>{}
                                                     : std::span<Position const>(*_tick.zeliekLane);
    }

    std::span<Position const> ZeliekRetreatLane() const
    {
        if (!_tick.returningFromZeliek || !_tick.zeliekLane || bot->GetPositionZ() > PlatformExitZ)
            return {};
        auto const& lane = *_tick.zeliekLane;
        bool const onFlank = (bot->GetPositionX() > TransitionCenter.GetPositionX()) == _tick.zeliekRanged;
        if (onFlank && bot->GetPositionY() <= lane.front().GetPositionY() + CornerArrival)
            return {};
        return lane;
    }

    std::string_view ZeliekRangedRetreatReason(RaidPlan const& plan, Creature const& zeliek)
    {
        Unit const* const victim = zeliek.GetVictim();
        bool const onEastSide = bot->GetPositionX() > TransitionCenter.GetPositionX();
        bool const atCenter = bot->GetExactDist(TransitionCenter) <= CornerArrival;
        if (!onEastSide && !atCenter)
            return "outside the ranged flank";
        if (!AtCorner(&zeliek, ZeliekCorner))
            return "Zeliek left the corner";
        if (!victim || !DpsBossControlled(plan, &zeliek, ZeliekCorner) || !bot->IsValidAttackTarget(&zeliek))
            return "Zeliek has no confirmed tank or is not attackable";
        if (bot->GetHealthPct() <= sPlayerbotAIConfig.LowHealth)
            return "low health";
        if (!_hazards.IsSafeFromVoidZones(*bot, *bot))
            return "Void Zone";
        if (FourHorsemen40Hazards::CanReceiveZeliekWrath(*bot, *bot, ProtectedMarks()))
            return "Holy Wrath clearance";
        return {};
    }

    bool TryDpsTarget(RaidPlan const& plan, Member& member, Creature* target, uint8 corner, bool ranged,
                      Unit const* wrathVictim, bool finalZeliek)
    {
        if (!DpsBossControlled(plan, target, corner) || (corner == ZeliekCorner && !ranged && !finalZeliek) ||
            !bot->IsValidAttackTarget(target))
            return false;

        bool const useLanes = _tick.zeliekLane && finalZeliek;
        bool const groupRanged = useLanes && ranged;
        if (groupRanged && !AtCorner(target, corner))
            return false;

        std::vector<Position> spread;
        std::vector<Player const*> raidHealers;
        std::array<FourHorsemenMarkSource, CornerCount> allMarkSources;
        constexpr uint8 allMarks = (1u << CornerCount) - 1;
        uint8 const markCount = finalZeliek && ranged
                                    ? CollectProtectedMarks(*bot, ResolveBosses(), allMarks, allMarkSources)
                                    : 0;
        if (finalZeliek)
        {
            for (Member const& other : plan.members)
            {
                if (other.guid == member.guid)
                    continue;
                Player* const player = PlayerInRaid(*bot, other.guid);
                if (!player || !player->IsAlive() ||
                    (!ranged && target->GetVictim() != player &&
                     std::ranges::find(plan.zeliekMelee, other.guid) == plan.zeliekMelee.end()))
                    continue;
                if (!groupRanged)
                {
                    spread.push_back(player->GetPosition());
                    if (other.damageTarget == target->GetGUID() && other.damagePosition)
                        spread.push_back(*other.damagePosition);
                }
                if (ranged && other.zeliekRaidHealPosition)
                {
                    if (!groupRanged)
                        spread.push_back(*other.zeliekRaidHealPosition);
                    if (CanCoverZeliekRaid(player) &&
                        player->GetExactDist(&*other.zeliekRaidHealPosition) <=
                            (other.zeliekRaidHealPosition->GetExactDist(&TransitionCenter) <= CornerArrival
                                 ? CornerArrival
                                 : DpsPositionArrival) &&
                        std::ranges::none_of(std::span(allMarkSources.data(), markCount),
                                             [player](FourHorsemenMarkSource const& mark)
                                             { return player->GetExactDist2d(&mark.location) < mark.radius; }))
                        raidHealers.push_back(player);
                }
            }
        }
        else if (wrathVictim &&
                 (corner == ZeliekCorner || target->GetExactDist2d(wrathVictim) < 2.0f * HolyWrathClearance))
        {
            spread.reserve(plan.members.size());
            for (Member const& other : plan.members)
            {
                if (other.guid == member.guid || other.guid == wrathVictim->GetGUID())
                    continue;
                if (Player* player = PlayerInRaid(*bot, other.guid))
                {
                    spread.push_back(player->GetPosition());
                    if (other.damageTarget == target->GetGUID() && other.damagePosition)
                        spread.push_back(*other.damagePosition);
                }
            }
        }

        if (finalZeliek && ranged && raidHealers.empty())
            return false;

        float const range = ranged ? botAI->GetRange("spell") : bot->GetMeleeRange(target);
        float minimumRange = finalZeliek && ranged ? std::max(0.0f, range - ZeliekOuterRangeBand) : 0.0f;
        if (groupRanged)
            minimumRange = std::min(minimumRange, ZeliekRangedMinimumRange);
        if (ranged && bot->getClass() == CLASS_HUNTER)
            if (SpellInfo const* shot = sSpellMgr->GetSpellInfo(AI_VALUE2(uint32, "spell id", "auto shot")))
                minimumRange = std::max(minimumRange, bot->GetSpellMinRangeForTarget(target, shot) +
                                                          bot->GetMeleeRange(target) + DpsPositionArrival);
        float const spreadClearance = finalZeliek && !ranged ? ZeliekMeleeSpacing : HolyWrathClearance;
        auto const hasRaidHealing = [&](Position const& point)
        {
            return std::ranges::any_of(raidHealers, [&](Player const* healer)
            {
                return healer->GetExactDist(&point) <= HealerCoverageRange &&
                       healer->IsWithinLOS(point.GetPositionX(), point.GetPositionY(), point.GetPositionZ() + 1.5f);
            });
        };
        auto const usable = [&](Position const& point)
        {
            return (!(finalZeliek && ranged) || hasRaidHealing(point)) &&
                   (!useLanes || InZeliekDpsFlank(point, *target, ranged));
        };
        auto const usableDestination = [&](Position const& point)
        {
            return usable(point) &&
                   (!groupRanged || !FourHorsemen40Hazards::CanReceiveZeliekWrath(
                                        *bot, point, ProtectedMarks(), ZeliekWrathPositionMargin));
        };
        auto const safeToFight = [&](Position const& point)
        {
            return usable(point) &&
                   _hazards.CanAttackFrom(*bot, *target, point, range, ProtectedMarks(), spread, minimumRange,
                                          spreadClearance);
        };
        std::span<Position const> const lane = useLanes ? ZeliekApproachLane() : std::span<Position const>{};
        bool const threatened = groupRanged && lane.empty() &&
                                FourHorsemen40Hazards::CanReceiveZeliekWrath(*bot, *bot, ProtectedMarks());
        bool const canFight = lane.empty() && safeToFight(*bot);
        std::optional<Position> position;
        if (canFight)
            position = bot->GetPosition();
        else if (member.damageTarget == target->GetGUID() && member.damagePosition &&
                 usableDestination(*member.damagePosition) && safeToFight(*member.damagePosition))
            position = member.damagePosition;
        else
        {
            uint32 const now = getMSTime();
            if (threatened || !_lastDpsSearch[corner] || getMSTimeDiff(_lastDpsSearch[corner], now) >= DpsRouteRetry)
            {
                _lastDpsSearch[corner] = now;
                if (threatened)
                    DropAttack();
                if (useLanes)
                {
                    auto const atOffset = [&](float dx, float dy)
                    {
                        float const x = target->GetPositionX() + dx;
                        float const y = target->GetPositionY() + dy;
                        return Position(x, y, bot->GetMapHeight(x, y, target->GetPositionZ() + 2.0f, true, 5.0f));
                    };
                    std::array<Position, 20> pads;
                    std::size_t padCount = 0;
                    if (threatened)
                    {
                        // A short step can break a connection while the ranged flank remains in spell range.
                        for (float distance : {CornerArrival, 2.0f * CornerArrival})
                            for (uint8 step = 0; step < 8; ++step)
                            {
                                float const angle = step * float(M_PI) / 4.0f;
                                float const x = bot->GetPositionX() + std::cos(angle) * distance;
                                float const y = bot->GetPositionY() + std::sin(angle) * distance;
                                float const z = bot->GetMapHeight(x, y, bot->GetPositionZ() + 2.0f, true, 5.0f);
                                pads[padCount++] = Position(x, y, z);
                            }
                    }
                    if (ranged)
                    {
                        // Try the same firing area for the whole flank, relative to the live boss.
                        for (float offset : {0.0f, 2.0f, 4.0f})
                        {
                            pads[padCount++] = atOffset(21.6f - offset, -14.0f + offset * 0.25f);
                        }
                        // Leave a spot at the outer edge of spell range for farther healer posts.
                        float const rangeEdge = range - 0.2f;
                        float const east = HolyWrathClearance + 0.25f;
                        float const south = std::sqrt(std::max(0.0f, rangeEdge * rangeEdge - east * east));
                        pads[padCount++] = atOffset(east, -south);
                    }
                    else
                    {
                        // Leave room for the second melee slot on the same side of Zeliek.
                        float const distance = range - DpsPositionArrival;
                        pads[padCount++] = atOffset(-0.97f * distance, 0.24f * distance);
                        pads[padCount++] = atOffset(-0.36f * distance, -0.93f * distance);
                        pads[padCount++] = atOffset(-0.61f * distance, 0.79f * distance);
                        pads[padCount++] = atOffset(-distance, 0.0f);
                    }
                    position = _hazards.FindAttackPosition(*bot, *target, range, ranged, ProtectedMarks(), spread,
                                                           minimumRange, spreadClearance, usableDestination,
                                                           std::span(pads.data(), padCount),
                                                           DpsPositionArrival, lane);
                }
                else if (finalZeliek && ranged)
                    position = _hazards.FindAttackPosition(*bot, *target, range, ranged, ProtectedMarks(), spread,
                                                           minimumRange, spreadClearance, hasRaidHealing,
                                                           ZeliekRangedPads, DpsPositionArrival);
                else
                    position = _hazards.FindAttackPosition(*bot, *target, range, ranged, ProtectedMarks(), spread,
                                                           minimumRange, spreadClearance);
            }
        }
        if (!position)
            return false;

        if (!_tick.bosses[KorthazzCorner].IsEmpty() && PrepareCenter(member, corner, 0))
        {
            member.damagePosition.reset();
            return true;
        }

        if (member.damagePosition && (canFight || member.damageTarget != target->GetGUID() ||
                                      member.damagePosition->GetExactDist(*position) > DpsPositionArrival))
            _hazards.StopWalkTo(*bot, *member.damagePosition, DpsPositionArrival);
        if (AI_VALUE(Unit*, "current target") != target || !canFight)
            DropAttack();

        if (member.damageTarget != target->GetGUID())
            LOG_DEBUG("playerbots", "[Naxx40] Horsemen instance={} {} targets {} from a clear position",
                      bot->GetInstanceId(), bot->GetName(), target->GetName());
        member.damageTarget = target->GetGUID();
        member.damagePosition = position;
        _corner = corner;
        _fighting = canFight;
        _tick.target = member.damageTarget;
        _tick.position = *position;
        return true;
    }

    void PlanDps(Member& member, Bosses const& bosses)
    {
        _tick.dynamicDamage = true;
        _tick.ready = true;
        _tick.position = bot->GetPosition();
        uint8 const focus = RearFocus(bosses);
        uint8 const marks = DpsMarksToClear(*bot, bosses, member.damageMarks);
        if (member.damageMarks && !marks)
        {
            _hazards.StopMarkRetreat(*bot);
            _lastDpsSearch = {};
        }
        member.damageMarks = marks;
        _tick.assignment.clearingMarks = marks;
        _tick.protectedMarkCount = CollectProtectedMarks(*bot, bosses, marks, _tick.protectedMarks);

        Creature const* const zeliek = bosses[ZeliekCorner];
        bool const ranged = PlayerbotAI::IsRangedDps(bot);
        bool const finalZeliek = OnlyZeliekAlive(bosses);
        bool const finalRanged = _tick.zeliekRanged = ranged && finalZeliek;
        if (finalZeliek && !_tick.assignment.managedTank)
        {
            _tick.zeliekLane = ranged ? &ZeliekRangedLane : &ZeliekMeleeLane;
            _tick.returningFromZeliek = member.damageTarget == zeliek->GetGUID();
        }
        Unit const* const wrathVictim = zeliek && focus != ZeliekCorner ? zeliek->GetVictim() : nullptr;
        if (wrathVictim && wrathVictim != bot)
            _tick.protectedMarks[_tick.protectedMarkCount++] = {wrathVictim->GetPosition(), HolyWrathClearance, false};

        uint8 const firstLivingCorner = FirstLivingCorner(bosses);
        bool const openingFocus = firstLivingCorner == KorthazzCorner;
        uint8 const destination =
            openingFocus && member.centerDestination < CornerCount && bosses[member.centerDestination]
                ? member.centerDestination
                : firstLivingCorner;
        if (marks && member.returnDamageTarget.IsEmpty())
            member.returnDamageTarget = member.damageTarget;
        if ((!openingFocus || marks || member.centerPending || member.centerMarks) &&
            PrepareCenter(member, destination, marks))
        {
            member.damagePosition.reset();
            return;
        }

        if (!openingFocus ||
            (!member.returnDamageTarget.IsEmpty() &&
             std::ranges::none_of(bosses, [&](Creature const* boss)
                                  { return boss && boss->GetGUID() == member.returnDamageTarget; })))
            member.returnDamageTarget.Clear();

        RaidPlan& plan = *GetPlan(*bot);
        if (finalRanged)
            PrepareZeliekWrathAvoidance(plan, *bosses[ZeliekCorner]);
        if (focus == ZeliekCorner && !ranged &&
            std::ranges::find(plan.zeliekMelee, member.guid) == plan.zeliekMelee.end())
        {
            if (member.damagePosition)
                _hazards.StopWalkTo(*bot, *member.damagePosition, DpsPositionArrival);
            member.damagePosition.reset();
            if (_tick.zeliekLane)
                PrepareCenter(member, ZeliekCorner, 0, true);
            else
            {
                _tick.movingToWaitingPosition = true;
                _tick.position = plan.zeliekMeleeWait;
                DropAttack();
            }
            return;
        }

        for (uint8 corner = 0; corner < CornerCount; ++corner)
            if ((openingFocus || corner == firstLivingCorner) &&
                (member.returnDamageTarget.IsEmpty() ||
                 (bosses[corner] && bosses[corner]->GetGUID() == member.returnDamageTarget)) &&
                TryDpsTarget(plan, member, bosses[corner], corner, ranged, wrathVictim, focus == ZeliekCorner))
                return;

        _tick.markRest = wrathVictim && wrathVictim != bot && bot->GetExactDist2d(wrathVictim) < HolyWrathClearance;
        if (member.damagePosition)
            _hazards.StopWalkTo(*bot, *member.damagePosition, DpsPositionArrival);
        if (finalRanged && _tick.zeliekLane)
        {
            std::string_view const reason = ZeliekRangedRetreatReason(plan, *zeliek);
            if (reason.empty())
            {
                // Retry the reserved position when a healer settles or a temporary obstruction clears.
                DropAttack();
                return;
            }
            if (bot->GetExactDist(TransitionCenter) > CornerArrival)
                LOG_INFO("playerbots", "[Naxx40] Horsemen instance={} {} leaves Zeliek ranged flank: {}",
                         bot->GetInstanceId(), bot->GetName(), reason);
        }
        member.damagePosition.reset();
        if (finalRanged || _tick.zeliekLane)
        {
            PrepareCenter(member, ZeliekCorner, 0, true);
            return;
        }
        DropAttack();
    }

    Bosses ResolveBosses() const
    {
        Bosses bosses = {};
        for (uint8 i = 0; i < CornerCount; ++i)
            bosses[i] = Resolve(_tick.bosses[i]);
        return bosses;
    }

    bool ClearMarks()
    {
        if (_tick.assignment.earlyRearHealer)
        {
            std::array<FourHorsemenMarkSource, CornerCount> marks;
            uint8 const count =
                CollectProtectedMarks(*bot, ResolveBosses(), _tick.assignment.clearingMarks, marks);
            return _hazards.EscapeMarks(*bot, std::span(marks.data(), count), false);
        }
        return _hazards.EscapeMarks(*bot, ProtectedMarks(), false);
    }

    bool SoakMeteor()
    {
        MeteorResponse const meteor = RespondToMeteor(_tick.assignment, ResolveBosses());
        if (meteor == MeteorResponse::Ignore)
            return false;
        DropAttack();
        _fighting = false;
        _tick.finished = true;
        return meteor == MeteorResponse::Move;
    }

    bool CoverRearTank()
    {
        _tick.finished = true;
        Assignment const& assignment = _tick.assignment;
        DropAttack();
        _fighting = false;
        if (assignment.dedicatedRearHealer || assignment.reserveRearHealer)
            return CoverEarlyRearTank();
        Player* const tank = RearHealingTarget(*bot, ObjectAccessor::GetPlayer(*bot, assignment.healingTarget));
        if (!tank)
            return false;
        bool const finalZeliek = assignment.corner == ZeliekCorner && OnlyZeliekAlive(ResolveBosses());
        if (RearTankCanBeHealedBy(*bot, tank))
        {
            if (finalZeliek && tank->GetHealthPct() >= RearTankUrgentHealth && MoveToZeliekHealingRange(*tank))
                return true;
            if (assignment.corner == ZeliekCorner && !finalZeliek && tank->GetHealthPct() >= RearTankUrgentHealth &&
                bot->GetExactDist2d(tank) < HolyWrathClearance + DpsPositionArrival && CanSustainHealingCoverage(bot) &&
                !bot->IsNonMeleeSpellCast(true))
                if (Creature const* const zeliek = Resolve(_tick.bosses[ZeliekCorner]);
                    zeliek && zeliek->GetVictim() == tank)
                {
                    Bosses const bosses = ResolveBosses();
                    std::array<FourHorsemenMarkSource, CornerCount> marks;
                    uint8 const count = CollectProtectedMarks(*bot, bosses, MarksToClear(*bot), marks);
                    constexpr float healingSpreadRange = 25.0f;
                    if (_hazards.ApproachRear(*bot, *tank, healingSpreadRange, DpsPositionArrival,
                                              HolyWrathClearance + DpsPositionArrival, LocalAnchorAdjustment,
                                              std::span(marks.data(), count)))
                        return true;
                }
            if (bot->isMoving())
                bot->StopMoving();
            return HealRearTank(*tank);
        }
        if (finalZeliek && MoveToZeliekHealingRange(*tank))
            return true;
        return _hazards.ApproachRear(*bot, *tank, HealerCoverageRange, CornerArrival);
    }

    bool IsRearExchangeHealer(RaidPlan const* plan) const
    {
        Assignment const& assignment = _tick.assignment;
        return plan && (assignment.dedicatedRearHealer || assignment.reserveRearHealer) && assignment.sideHealer &&
               !assignment.standby && !assignment.healingTarget.IsEmpty();
    }

    bool FollowingRearExchangeHealerPath(std::span<FourHorsemenMarkSource const> avoid)
    {
        bool crossesPool = false;
        if (!_hazards.IsFollowingRearExchangePath(*bot, _earlyRearHealerDestination, CornerArrival,
                                                  _earlyRearHealerSpline, {true}, &crossesPool, nullptr, avoid))
            return false;
        if (crossesPool && !_earlyRearHealerCrossing)
            LOG_INFO("playerbots", "[Naxx40] Horsemen instance={} {} healer crosses a pool during rear exchange",
                     bot->GetInstanceId(), bot->GetName());
        _earlyRearHealerCrossing = crossesPool;
        return true;
    }

    bool IsMovingAlongRearExchangeHealerPath() const
    {
        return _earlyRearHealerSpline && bot->isMoving() && !bot->movespline->Finalized() &&
               bot->movespline->GetId() == _earlyRearHealerSpline;
    }

    Player* DoubleTargetedRearTank(ObjectGuid const& assignedTank) const
    {
        RaidPlan const* const plan = GetPlan(*bot);
        if (!plan || !plan->earlyRearSwapAt)
            return nullptr;
        Creature const* const blaumeux = Resolve(_tick.bosses[BlaumeuxCorner]);
        Creature const* const zeliek = Resolve(_tick.bosses[ZeliekCorner]);
        Player* const victim = zeliek ? RearHealingTarget(*bot, zeliek->GetVictim()) : nullptr;
        return victim && victim->GetGUID() != assignedTank && blaumeux && blaumeux->GetVictim() == victim &&
                       RearTankCanBeHealedBy(*bot, victim)
                   ? victim
                   : nullptr;
    }

    bool AvoidRearExchangeHealerPool(bool helpingExchange)
    {
        ObjectGuid const patient = _tick.assignment.healingTarget;
        bool const sameAssignment = helpingExchange && patient == _earlyRearHealerPatient;
        if (_hazards.IsSafeFromVoidZones(*bot, bot->GetPosition()))
        {
            if (!sameAssignment && _earlyRearHealerSpline)
            {
                StopRearExchangePath(bot, _earlyRearHealerSpline);
                _earlyRearHealerSpline = 0;
            }
            return false;
        }
        if (sameAssignment)
        {
            Creature const* const zeliek = Resolve(_tick.bosses[ZeliekCorner]);
            Unit const* const victim = zeliek ? zeliek->GetVictim() : nullptr;
            FourHorsemenMarkSource const wrath = {victim ? victim->GetPosition() : Position{}, HolyWrathClearance,
                                                  false};
            if (FollowingRearExchangeHealerPath(std::span(&wrath, victim ? 1u : 0u)))
            {
                if (Player* const tank = RearHealingTarget(*bot, ObjectAccessor::GetPlayer(*bot, patient));
                    tank && RearTankCanBeHealedBy(*bot, tank) &&
                    HealRearTank(*tank, RearHealCast::InstantOnly, _tick.assignment.reserveRearHealer))
                    return true;
                if (Player* const doubleTarget = DoubleTargetedRearTank(patient))
                    HealRearTank(*doubleTarget, RearHealCast::InstantOnly);
                return true;
            }
        }
        return _hazards.EscapeRearExchangeVoidZone(*bot);
    }

    bool MoveRearExchangeHealer(Player& tank, Position const* station, bool preparingExchange, bool needsApproach,
                                float range,
                                std::span<FourHorsemenMarkSource const> avoid)
    {
        if (_earlyRearHealerSpline && _earlyRearHealerPatient == tank.GetGUID() &&
            ((station && _earlyRearHealerDestination.GetExactDist(*station) <= CornerArrival) ||
             (!preparingExchange &&
              _hazards.CanAttackFrom(*bot, tank, _earlyRearHealerDestination, range, avoid, {}, HolyWrathClearance))) &&
            FollowingRearExchangeHealerPath(avoid))
            return true;
        if (!station && !needsApproach)
            return false;
        uint32 const now = getMSTime();
        if (_earlyRearHealerProbeAt && getMSTimeDiff(_earlyRearHealerProbeAt, now) < EarlyRearProbeInterval)
            return false;
        _earlyRearHealerProbeAt = now;
        StopRearExchangePath(bot, _earlyRearHealerSpline);
        _earlyRearHealerSpline = 0;
        _rearHealerDescentCorner = NoCorner;

        Movement::PointsArray path;
        bool stationCrossesPool = false;
        bool const hasStationPath = station && bot->GetExactDist(station) > CornerArrival &&
                                    _hazards.PrepareRearExchangePath(*bot, *station, CornerArrival, path, {true},
                                                                     &stationCrossesPool, nullptr, avoid);
        auto const startStation = [&]
        {
            if (!_hazards.StartRearExchangePath(*bot, path, _earlyRearHealerSpline))
                return false;
            _earlyRearHealerDestination = *station;
            return true;
        };
        bool moving = hasStationPath && !stationCrossesPool && startStation();
        if (!moving && (needsApproach || hasStationPath))
            moving = _hazards.ApproachRear(*bot, tank, range, CornerArrival, HolyWrathClearance, 0.0f, avoid,
                                           &_earlyRearHealerDestination, &_earlyRearHealerSpline, !hasStationPath);
        if (!moving && !(hasStationPath && stationCrossesPool && needsApproach && startStation()))
            return false;
        _earlyRearHealerPatient = tank.GetGUID();
        _earlyRearHealerCrossing = false;
        return FollowingRearExchangeHealerPath(avoid);
    }

    bool DescendRearHealerReserve(Player& tank, std::span<FourHorsemenMarkSource const> avoid)
    {
        if (_rearHealerDescentCorner == _corner && _earlyRearHealerPatient == tank.GetGUID() &&
            FollowingRearExchangeHealerPath(avoid))
            return true;
        _rearHealerDescentCorner = NoCorner;
        if (IsOnRearHealingFloor(*bot))
            return false;

        uint32 const now = getMSTime();
        if (_earlyRearHealerProbeAt && getMSTimeDiff(_earlyRearHealerProbeAt, now) < EarlyRearProbeInterval)
            return false;
        _earlyRearHealerProbeAt = now;
        StopRearExchangePath(bot, _earlyRearHealerSpline);
        _earlyRearHealerSpline = 0;

        Movement::PointsArray bestPath;
        Position const* bestPad = nullptr;
        float bestLength = std::numeric_limits<float>::max();
        bool bestCrosses = true;
        for (Position const& pad : RearSupport[_corner - BlaumeuxCorner])
        {
            Movement::PointsArray path;
            bool crossesPool = false;
            if (!_hazards.PrepareRearExchangePath(*bot, pad, CornerArrival, path, {true}, &crossesPool, nullptr, avoid))
                continue;
            float length = 0.0f;
            for (std::size_t i = 1; i < path.size(); ++i)
                length += (path[i] - path[i - 1]).length();
            if (bestPad && (crossesPool != bestCrosses ? crossesPool : length >= bestLength))
                continue;
            bestPad = &pad;
            bestPath = std::move(path);
            bestLength = length;
            bestCrosses = crossesPool;
        }
        if (!bestPad || !_hazards.StartRearExchangePath(*bot, bestPath, _earlyRearHealerSpline))
            return false;
        _earlyRearHealerDestination = *bestPad;
        _earlyRearHealerPatient = tank.GetGUID();
        _earlyRearHealerCrossing = false;
        _rearHealerDescentCorner = _corner;
        LOG_INFO("playerbots",
                 "[Naxx40] Horsemen instance={} reserve healer {} descends toward {} ({:.1f}, {:.1f}, {:.2f})",
                 bot->GetInstanceId(), bot->GetName(), Horsemen[_corner].name, bestPad->GetPositionX(),
                 bestPad->GetPositionY(), bestPad->GetPositionZ());
        return true;
    }

    bool CoverEarlyRearTank()
    {
        if (bot->IsNonMeleeSpellCast(true))
            return false;
        Assignment const& assignment = _tick.assignment;
        Player* const tank = RearHealingTarget(*bot, ObjectAccessor::GetPlayer(*bot, assignment.healingTarget));
        if (!tank)
            return false;

        bool const covered = RearTankCanBeHealedBy(*bot, tank);
        Creature const* const zeliek = Resolve(_tick.bosses[ZeliekCorner]);
        Unit* const wrathTarget = zeliek ? zeliek->GetVictim() : nullptr;
        bool const tooClose = wrathTarget && bot->GetExactDist2d(wrathTarget) < HolyWrathClearance;
        bool const critical = tank->GetHealthPct() <= sPlayerbotAIConfig.CriticalHealth;
        constexpr float castWhileFollowingRange = 23.0f;
        std::array<FourHorsemenMarkSource, 1> avoid;
        uint8 count = 0;
        if (wrathTarget)
            avoid[count++] = {wrathTarget->GetPosition(), HolyWrathClearance, false};
        std::span const protectedPositions(avoid.data(), count);
        bool const confirmCoverage = assignment.reserveRearHealer;
        if (confirmCoverage && _corner >= BlaumeuxCorner && _corner < CornerCount)
        {
            if (DescendRearHealerReserve(*tank, protectedPositions))
            {
                if (covered && !tooClose)
                    HealRearTank(*tank, RearHealCast::InstantOnly, true);
                return true;
            }
            if (!IsOnRearHealingFloor(*bot))
                return covered && !tooClose && HealRearTank(*tank, RearHealCast::InstantOnly, true);
        }
        Player* const doubleTarget = DoubleTargetedRearTank(tank->GetGUID());
        if (IsMovingAlongRearExchangeHealerPath() && _earlyRearHealerPatient == tank->GetGUID() &&
            FollowingRearExchangeHealerPath(protectedPositions) && _earlyRearHealerCrossing)
        {
            if (covered && !tooClose && HealRearTank(*tank, RearHealCast::InstantOnly, confirmCoverage))
                return true;
            if (doubleTarget && !tooClose)
                HealRearTank(*doubleTarget, RearHealCast::InstantOnly);
            return true;
        }

        if (covered &&
            (critical || (!tooClose && (confirmCoverage || bot->GetExactDist(tank) <= castWhileFollowingRange))) &&
            HealRearTank(*tank, RearHealCast::StopIfNeeded, confirmCoverage))
            return true;
        if (doubleTarget && (!tooClose || doubleTarget->GetHealthPct() <= sPlayerbotAIConfig.CriticalHealth) &&
            HealRearTank(*doubleTarget, RearHealCast::StopIfNeeded))
            return true;

        RaidPlan const* const plan = GetPlan(*bot);
        if (IsRearExchangeHealer(plan))
        {
            Position const* station = nullptr;
            bool preparingExchange = false;
            auto const binding =
                std::ranges::find(plan->earlyRearHealing, tank->GetGUID(), &EarlyRearHealerAssignment::tank);
            if (binding != plan->earlyRearHealing.end() && binding->preparedHealer == bot->GetGUID() &&
                binding->exchangePosition)
            {
                Position const& prepared = *binding->exchangePosition;
                if ((!wrathTarget || prepared.GetExactDist2d(wrathTarget) >= HolyWrathClearance) &&
                    prepared.GetExactDist(tank) <= RearHealerFollowRange &&
                    tank->IsWithinLOS(prepared.GetPositionX(), prepared.GetPositionY(), prepared.GetPositionZ()))
                {
                    station = &prepared;
                    preparingExchange = binding->preparingExchange;
                }
            }
            // Follow ahead of the 35 yd coverage limit so a moving tank stays in range.
            bool const needsApproach = !covered || tooClose || bot->GetExactDist(tank) > RearHealerFollowRange;
            if (MoveRearExchangeHealer(*tank, station, preparingExchange, needsApproach, RearHealerFollowRange,
                                       protectedPositions))
                return true;
        }

        return covered && !tooClose && bot->GetExactDist(tank) <= RearHealerFollowRange &&
               HealRearTank(*tank, RearHealCast::StopIfNeeded, confirmCoverage);
    }

    bool MoveToZeliekHealingRange(Player& tank)
    {
        constexpr float innerRange = HealerCoverageRange - ZeliekOuterRangeBand;
        bool const inRange = bot->IsWithinDistInMap(&tank, HealerCoverageRange) && bot->IsWithinLOSInMap(&tank);
        if (bot->IsNonMeleeSpellCast(true) || (inRange && bot->GetExactDist2d(&tank) >= innerRange))
            return false;

        std::vector<FourHorsemenMarkSource> avoid;
        for (Member const& member : GetPlan(*bot)->members)
            if (Player* const other = PlayerInRaid(*bot, member.guid);
                other && other != bot && PlayerbotAI::IsHeal(other))
                avoid.push_back({other->GetPosition(), ZeliekHealerSpacing, false});
        return _hazards.ApproachRear(*bot, tank, HealerCoverageRange, CornerArrival, innerRange, 0.0f, avoid);
    }

    bool ControlScheduledRearTank(Creature& target, RaidPlan& plan)
    {
        uint8 const corner = _corner;
        TankHandoff& handoff = plan.tankHandoffs[corner];
        Player* const holder = HolderInRaid(*bot, plan.holders[corner]);
        if (handoff.incoming != bot->GetGUID() || handoff.boss != target.GetGUID() || !CanHold(holder) ||
            holder == bot || target.GetVictim() != holder || bot->GetAuraCount(Horsemen[corner].mark) >= RotationStacks)
            return false;

        uint32 const now = getMSTime();
        if (handoff.tauntAt && getMSTimeDiff(handoff.tauntAt, now) <= LocalTauntConfirmWindow)
            return false;
        if (uint32 const buff = RequiredHolderBuff(*bot))
            return ApplyHolderBuff(*botAI, buff);

        bool const partialRearExchange = plan.earlyRearSwapAt && handoff.active;
        Player const* const healer = RearHandoffHealer(*bot, plan, corner);
        bool const soloZeliek = corner == ZeliekCorner && _tick.zeliekLane;
        bool const spreadForWrath = corner == ZeliekCorner && !soloZeliek;
        std::span<Position const> const lane = soloZeliek ? ZeliekApproachLane() : std::span<Position const>{};
        bool const urgent = holder->GetHealthPct() <= sPlayerbotAIConfig.CriticalHealth;
        auto const inReliefZone = [&](Position const& point)
        {
            if (soloZeliek && (!InZeliekTankFlank(point, target) ||
                               (!urgent && point.GetExactDist(&target) > bot->GetMeleeRange(&target))))
                return false;
            return InHandoffCorner(point, target, corner) ||
                   (partialRearExchange &&
                    std::ranges::any_of(
                        RearExchangePads, [&](Position const& pad)
                        { return point.GetExactDist(pad) <= RearExchangePadFallback + CornerArrival; }));
        };
        auto const canCover = [&](Position const& point)
        {
            return healer &&
                   healer->GetExactDist(point) < HealerCoverageRange + healer->GetObjectSize() + bot->GetObjectSize() &&
                   (!spreadForWrath || healer->GetExactDist2d(point) >= HolyWrathClearance) &&
                   healer->IsWithinLOS(point.GetPositionX(), point.GetPositionY(), point.GetPositionZ());
        };
        auto const legal = [&](Position const& point)
        {
            return inReliefZone(point) && (partialRearExchange ? healer != nullptr : canCover(point)) &&
                   _hazards.CanAttackFrom(*bot, target, point, TauntRange, {}, {});
        };
        bool const safe = _hazards.IsSafeFromVoidDamage(*bot, *bot);
        if (safe && lane.empty() && inReliefZone(*bot) && canCover(*bot) && handoff.active &&
            LocalIncomingHealthy(*bot, *holder) &&
            (partialRearExchange ? RearExchangeCovered(*bot, plan, *holder) : RearTankCanBeHealedBy(*healer, holder)) &&
            RearTankCanBeHealedBy(*healer, bot) && CastRearTaunt(*botAI, target))
        {
            handoff.tauntAt = now;
            LOG_INFO("playerbots",
                     "[Naxx40] Horsemen instance={} {} taunts {} at rear corner from ({:.1f}, {:.1f}), holder marks={}",
                     bot->GetInstanceId(), bot->GetName(), target.GetName(), bot->GetPositionX(), bot->GetPositionY(),
                     holder->GetAuraCount(Horsemen[corner].mark));
            return true;
        }
        if (safe && lane.empty() && legal(*bot) && bot->IsWithinMeleeRange(&target))
        {
            handoff.approach = bot->GetPosition();
            return false;
        }

        uint32 const timeToArrive =
            partialRearExchange
                ? EarlyRearTimeout - std::min(EarlyRearTimeout - 1, getMSTimeDiff(plan.earlyRearSwapAt, now))
                : std::max(1u, TimeToRearMark(*holder, corner));
        // Keep a dry western approach moving even when the next mark is too close to reach the corner in time.
        FourHorsemenRearCrossing const crossing{healer != nullptr, healer, HealerCoverageRange,
                                                static_cast<float>(timeToArrive) / 1000.0f, soloZeliek};
        auto const reach = [&](Position const& point)
        {
            if (!legal(point) || !_hazards.WalkRearExchangeToPoint(*bot, point, DpsPositionArrival, crossing, lane))
                return false;
            handoff.approach = point;
            return true;
        };
        if (handoff.approach && reach(*handoff.approach))
            return true;
        if (handoff.checkedAt && getMSTimeDiff(handoff.checkedAt, now) < RearReliefRetryInterval)
            return false;
        handoff.checkedAt = now;
        handoff.approach.reset();
        if (reach(*holder))
            return true;
        if (soloZeliek)
        {
            for (Position const& pad : ZeliekWestTankPads)
                if (reach(pad))
                    return true;
            float const x = target.GetPositionX() - CornerArrival;
            for (float const y : {target.GetPositionY(), target.GetPositionY() - CornerArrival})
            {
                float const z = bot->GetMapHeight(x, y, target.GetPositionZ() + CornerArrival, true, CornerArrival);
                if (reach(Position(x, y, z)))
                    return true;
            }
        }
        if (partialRearExchange)
            for (Position const& position : plan.earlyRearDestinations)
                if (reach(position))
                    return true;
        for (Position const& pad : RearHolderPads[corner - BlaumeuxCorner])
            if (reach(pad))
                return true;
        return reach(Horsemen[corner].corner);
    }

    bool ControlScheduledTank(Creature& target, RaidPlan& plan)
    {
        uint8 const corner = _corner;
        TankHandoff& handoff = plan.tankHandoffs[corner];
        if (handoff.incoming != bot->GetGUID() || handoff.boss != target.GetGUID() || target.GetVictim() == bot)
            return false;

        Player* const holder = HolderInRaid(*bot, plan.holders[corner]);
        if (!CanHold(holder) || target.GetVictim() != holder ||
            !LocalIncomingHealthy(*bot, *holder) || !AtCorner(&target, corner))
            return false;

        uint32 const now = getMSTime();
        if (handoff.tauntAt && getMSTimeDiff(handoff.tauntAt, now) <= LocalTauntConfirmWindow)
            return false;
        if (uint32 const buff = RequiredHolderBuff(*bot))
            return ApplyHolderBuff(*botAI, buff);
        if (corner == BlaumeuxCorner && !handoff.releasedAt && !handoff.protectionReady)
        {
            handoff.protectionReady = ConfirmFirstShadowProtection(*bot, plan.rearProtection, now);
            if (!handoff.protectionReady)
                return true;
        }

        Player const* const healer = LocalHolderHealer(*bot, plan, *holder, corner);
        bool const spreadForWrath = corner == ZeliekCorner && !OnlyZeliekAlive(ResolveBosses());
        auto const covered = [&](Position const& point)
        {
            return holder->GetHealthPct() <= sPlayerbotAIConfig.CriticalHealth ||
                   (HealerCoversHandoffPad(healer, *holder, point, bot) &&
                    (!spreadForWrath || healer->GetExactDist2d(&point) >= HolyWrathClearance));
        };
        auto const legal = [&](Position const& point)
        {
            return InHandoffCorner(point, target, corner) && covered(point) &&
                   _hazards.CanAttackFrom(*bot, target, point, bot->GetMeleeRange(&target), {}, {});
        };
        if (bot->IsWithinMeleeRange(&target) && legal(*bot) &&
            LocalHandoffReady(*bot, *holder, healer, *bot, spreadForWrath))
        {
            if (bot->isMoving())
                bot->StopMoving();
            if (TauntHorseman(*botAI, target) ||
                (bot->getClass() == CLASS_PALADIN && CanRetakeWithRighteousDefense(*holder, target) &&
                 botAI->CanCastSpell(SPELL_RIGHTEOUS_DEFENSE, holder) &&
                 botAI->CastSpell(SPELL_RIGHTEOUS_DEFENSE, holder)))
            {
                handoff.tauntAt = now;
                LOG_INFO("playerbots", "[Naxx40] Horsemen instance={} {} taunt requested by {} at corner",
                         bot->GetInstanceId(), Horsemen[corner].name, bot->GetName());
                return true;
            }
            return false;
        }

        auto const reach = [&](Position const& point)
        {
            if (!legal(point) || !_hazards.WalkRear(*bot, point, DpsPositionArrival, true))
                return false;
            handoff.approach = point;
            return true;
        };
        if (handoff.approach && reach(*handoff.approach))
            return true;
        if (handoff.checkedAt && getMSTimeDiff(handoff.checkedAt, now) < RearReliefRetryInterval)
            return false;
        handoff.checkedAt = now;
        handoff.approach.reset();
        if (reach(*holder))
            return true;

        float const baseAngle = target.GetAbsoluteAngle(&TransitionCenter) + float(M_PI);
        float const radius = bot->GetMeleeRange(&target) * 0.75f;
        for (int const step : {0, 1, -1, 2, -2, 3, -3, 4})
        {
            float const angle = baseAngle + step * float(M_PI) / 4.0f;
            float const x = target.GetPositionX() + std::cos(angle) * radius;
            float const y = target.GetPositionY() + std::sin(angle) * radius;
            float const z = bot->GetMapHeight(x, y, target.GetPositionZ() + 2.0f, true, 4.0f);
            if (std::abs(z - target.GetPositionZ()) <= 1.5f && reach(Position(x, y, z)))
                return true;
        }
        return false;
    }

    bool FollowingEarlyRearRoute(RaidPlan& plan, uint8 side)
    {
        if (!plan.earlyRearSwapAt)
            return false;
        bool crossesPool = false;
        std::string_view reason;
        Player const* const healer = RearExchangeHealer(*bot, plan, *bot);
        bool const following = _hazards.IsFollowingRearExchangePath(
            *bot, plan.earlyRearDestinations[side], CornerArrival, plan.earlyRearSplines[side],
            {healer != nullptr, healer, HealerCoverageRange, EarlyRearApproachSeconds(plan)}, &crossesPool, &reason);
        if (!following)
        {
            // The route naturally ends at the meeting point.
            if (bot->isMoving() && !bot->movespline->Finalized() &&
                bot->movespline->GetId() == plan.earlyRearSplines[side])
                LogEarlyRearRouteBlock(*bot, plan, side, reason);
            return false;
        }
        if (crossesPool && !plan.earlyRearCrossings[side])
            LOG_INFO("playerbots", "[Naxx40] Horsemen instance={} {} continues rear exchange through a pool",
                     bot->GetInstanceId(), bot->GetName());
        plan.earlyRearCrossings[side] = crossesPool;
        LogEarlyRearRouteBlock(*bot, plan, side, {});
        return true;
    }

    bool ResumeEarlyRearApproach(RaidPlan& plan, uint8 side)
    {
        if (FollowingEarlyRearRoute(plan, side))
            return true;
        uint32& spline = plan.earlyRearSplines[side];
        StopRearExchangePath(bot, spline);
        uint32 const now = getMSTime();
        uint32& lastAttempt = plan.earlyRearReplanAt[side];
        if (lastAttempt && getMSTimeDiff(lastAttempt, now) < EarlyRearProbeInterval)
            return false;
        lastAttempt = now;

        Position const& previous = plan.earlyRearDestinations[side];
        EarlyRearRoute const route = PrepareEarlyRearRoute(
            *bot, _hazards, side, previous, RearExchangeHealer(*bot, plan, *bot), EarlyRearApproachSeconds(plan));
        if (!route.block.empty())
        {
            LogEarlyRearRouteBlock(*bot, plan, side, route.block);
            return false;
        }
        bool const destinationChanged = previous.GetExactDist(route.destination) > 0.01f;
        if (destinationChanged &&
            PrepareRearExchangeHealer(*bot, plan, *bot, route.destination, Resolve(_tick.bosses[ZeliekCorner])) ==
                RearHealerCoverage::Unavailable)
        {
            LogEarlyRearRouteBlock(*bot, plan, side, "healer cannot cover the new meeting point");
            return false;
        }
        if (bot->GetExactDist(route.destination) > CornerArrival &&
            !_hazards.StartRearExchangePath(*bot, route.path, spline))
        {
            LogEarlyRearRouteBlock(*bot, plan, side, "could not resume movement");
            return false;
        }
        SetEarlyRearRoute(*bot, plan, side, route);
        LogEarlyRearRouteBlock(*bot, plan, side, {});
        return true;
    }

    float EarlyRearApproachSeconds(RaidPlan const& plan) const
    {
        Creature const* const rider = Resolve(_tick.target);
        if (rider && rider->GetVictim() == bot)
            return 0.0f;
        return static_cast<float>(std::max(1u, RearExchangeTimeRemaining(plan, getMSTime()))) / 1000.0f;
    }

    bool ControlEarlyRearExchange(Creature& target, RaidPlan& plan)
    {
        uint8 const side = (_corner ^ 1) - BlaumeuxCorner;
        if (!plan.earlyRearSwapAt)
            return false;
        if (!_hazards.IsSafeFromVoidDamage(*bot, *bot))
            return FollowingEarlyRearRoute(plan, side) || _hazards.EscapeRearExchangeVoidZone(*bot);
        if (IsEarlyRearReplacement(plan, _corner, target.GetVictim()))
            return false;
        Player const* const other = HolderInRaid(*bot, plan.holders[_corner]);
        bool const inExchange = bot->GetExactDist(RearExchangePads[side]) <= RearExchangePadFallback + CornerArrival;
        uint32 const now = getMSTime();
        uint32& attemptedAt = plan.earlyRearTaunts[_corner - BlaumeuxCorner];
        if (target.GetVictim() != bot && inExchange && other &&
            (!attemptedAt || getMSTimeDiff(attemptedAt, now) > LocalTauntConfirmWindow) &&
            RearExchangeCovered(*bot, plan, *bot) && RearExchangeCovered(*bot, plan, *other) &&
            CastRearTaunt(*botAI, target))
        {
            attemptedAt = now;
            LOG_INFO("playerbots",
                     "[Naxx40] Horsemen instance={} early rear exchange {} taunts {} from ({:.1f}, {:.1f})",
                     bot->GetInstanceId(), bot->GetName(), target.GetName(), bot->GetPositionX(), bot->GetPositionY());
            return true;
        }
        if (AtRearExchangePad(*bot, plan, side) && _hazards.IsSafeFromVoidZones(*bot, plan.earlyRearDestinations[side]))
            return false;
        return ResumeEarlyRearApproach(plan, side);
    }

    bool ControlTank()
    {
        Creature* const target = Resolve(_tick.target);
        if (!target)
            return false;

        Assignment const& assignment = _tick.assignment;
        uint8 const corner = _corner;
        bool const rearHolder = corner >= BlaumeuxCorner;
        Position const& position = _tick.position;
        RaidPlan& plan = *GetPlan(*bot);
        if (assignment.earlyRearSwap)
            return ControlEarlyRearExchange(*target, plan);
        TankHandoff const& handoff = plan.tankHandoffs[corner];
        if (assignment.tankIncoming)
            return rearHolder ? ControlScheduledRearTank(*target, plan) : ControlScheduledTank(*target, plan);
        if (handoff.tauntAt && OwnsTarget(target, handoff.incoming))
            return false;

        uint32 const holderBuff = RequiredHolderBuff(*bot);
        if (holderBuff && (holderBuff != SPELL_RIGHTEOUS_FURY && holderBuff != SPELL_FROST_PRESENCE) &&
            ApplyHolderBuff(*botAI, holderBuff))
        {
            LOG_INFO("playerbots", "[Naxx40] Horsemen instance={} {} prepares for {} with spell {}",
                     bot->GetInstanceId(), bot->GetName(), Horsemen[corner].name, holderBuff);
            return true;
        }

        Unit const* const actualVictim = target->GetVictim();
        bool const needsTaunt = actualVictim != bot;
        if (needsTaunt && _tick.zeliekLane && AtCorner(target, corner))
        {
            std::span<Position const> const lane = ZeliekApproachLane();
            if (!lane.empty() || !InZeliekTankFlank(*bot, *target) ||
                bot->GetExactDist(target) > TauntRange || !bot->IsWithinLOSInMap(target))
            {
                _tick.finished = true;
                auto const walkWest = [&](Position const& pad)
                {
                    return InZeliekTankFlank(pad, *target) &&
                           _hazards.WalkRear(*bot, pad, CornerArrival, true, 0.0f, {}, nullptr, true, lane);
                };
                if (walkWest(Horsemen[corner].corner))
                    return true;
                for (Position const& pad : ZeliekWestTankPads)
                    if (walkWest(pad))
                        return true;
                float const x = target->GetPositionX() - CornerArrival;
                float const y = target->GetPositionY();
                float const z = bot->GetMapHeight(x, y, target->GetPositionZ() + CornerArrival, true, CornerArrival);
                return walkWest(Position(x, y, z));
            }
        }
        if (needsTaunt && TauntHorseman(*botAI, *target))
            return true;
        if (rearHolder && bot->getClass() == CLASS_PALADIN)
        {
            if (needsTaunt)
            {
                Player* const victim = RearHealingTarget(*bot, target->GetVictim());
                if (victim && CanRetakeWithRighteousDefense(*victim, *target) &&
                    botAI->CanCastSpell(SPELL_RIGHTEOUS_DEFENSE, victim) &&
                    botAI->CastSpell(SPELL_RIGHTEOUS_DEFENSE, victim))
                {
                    LOG_INFO("playerbots", "[Naxx40] Horsemen instance={} {} uses Righteous Defense on {} for {}",
                             bot->GetInstanceId(), bot->GetName(), victim->GetName(), target->GetName());
                    return true;
                }
            }
            else if (bot->IsWithinMeleeRange(target))
            {
                if ((bot->GetVictim() != target || !bot->HasUnitState(UNIT_STATE_MELEE_ATTACKING)) &&
                    _attack.Attack(target))
                    return true;
                if (!bot->HasSpellCooldown(SPELL_HAMMER_OF_THE_RIGHTEOUS) &&
                    bot->HasSpell(SPELL_HAMMER_OF_THE_RIGHTEOUS) && IsIsolatedForHammer(*target, ResolveBosses()) &&
                    botAI->CanCastSpell(SPELL_HAMMER_OF_THE_RIGHTEOUS, target) &&
                    botAI->CastSpell(SPELL_HAMMER_OF_THE_RIGHTEOUS, target))
                    return true;
            }
        }
        if (needsTaunt && actualVictim && corner < 2 && bot->getClass() == CLASS_WARRIOR &&
            bot->IsWithinMeleeRange(target) && bot->IsWithinLOSInMap(target))
        {
            ShapeshiftForm const form = bot->GetShapeshiftForm();
            if ((form == FORM_BATTLESTANCE || form == FORM_DEFENSIVESTANCE) &&
                botAI->CanCastSpell(SPELL_MOCKING_BLOW, target) && botAI->CastSpell(SPELL_MOCKING_BLOW, target))
            {
                LOG_INFO("playerbots", "[Naxx40] Horsemen front instance={} {} uses Mocking Blow on {} (victim {})",
                         bot->GetInstanceId(), bot->GetName(), target->GetName(), actualVictim->GetName());
                return true;
            }
        }
        if (assignment.openingThreat && BuildFeralOpeningThreat(*botAI, *target))
            return true;
        if ((assignment.frontOpeningAt || assignment.frontHandoffThreat) &&
            BuildFrontWarriorThreat(*target, assignment.frontOpeningAt))
            return true;

        if (needsTaunt)
        {
            _tick.finished = true;
            if (bot->GetExactDist(target) > TauntRange || !bot->IsWithinLOSInMap(target))
            {
                // Follow the active rider once it has left the platform, including after a missed opening taunt.
                if (rearHolder && assignment.stagedTank && target->GetPositionZ() > PlatformExitZ &&
                    WalkTo(OpeningFlanks[corner - 2].pull, true))
                    return true;
                return _hazards.ApproachRear(*bot, *target, TauntRange, CornerArrival) || WalkTo(position, true);
            }

            return AI_VALUE(Unit*, "current target") != target && _attack.Attack(target);
        }

        return (holderBuff == SPELL_RIGHTEOUS_FURY || holderBuff == SPELL_FROST_PRESENCE) &&
               botAI->CastSpell(holderBuff, bot);
    }

    bool TakePosition()
    {
        Assignment const& assignment = _tick.assignment;
        Position const& position = _tick.position;
        if (assignment.tankIncoming)
            return false;
        if (_holder)
            if (Creature const* const target = Resolve(_tick.target))
            {
                TankHandoff const& handoff = GetPlan(*bot)->tankHandoffs[_corner];
                if (handoff.tauntAt && OwnsTarget(target, handoff.incoming))
                    return false;
            }
        if (_tick.movingToWaitingPosition)
            return _hazards.WalkRear(*bot, position, CornerArrival, true, 0.0f, ProtectedMarks());
        if (_tick.zeliekRaidHealPad)
        {
            if (bot->GetExactDist(position) <= DpsPositionArrival || bot->IsNonMeleeSpellCast(true))
                return false;
            if (_hazards.WalkRear(*bot, position, DpsPositionArrival, true, 0.0f, ProtectedMarks(), nullptr, true))
                return true;
            _blockedZeliekHealAt[*_tick.zeliekRaidHealPad] = getMSTime();
            if (RaidPlan* plan = GetPlan(*bot))
                if (auto member = std::ranges::find(plan->members, bot->GetGUID(), &Member::guid);
                    member != plan->members.end())
                    member->zeliekRaidHealPosition.reset();
            return false;
        }
        if (!_holder && !assignment.standby && !assignment.sideHealer && PlayerbotAI::IsHeal(bot))
        {
            Bosses const bosses = ResolveBosses();
            if (RearFocus(bosses) == _corner)
                if (Player* const tank = RearHealingTarget(*bot, bosses[_corner]->GetVictim()))
                {
                    _tick.finished = true;
                    if (_corner == ZeliekCorner && MoveToZeliekHealingRange(*tank))
                        return true;
                    return !RearTankCanBeHealedBy(*bot, tank) &&
                           _hazards.ApproachRear(*bot, *tank, HealerCoverageRange, CornerArrival);
                }
        }
        if (_tick.dynamicDamage)
        {
            if (_hazards.WalkRear(*bot, position, DpsPositionArrival, true, 0.0f, ProtectedMarks(), nullptr,
                                  _tick.zeliekRanged || _tick.zeliekLane, ZeliekApproachLane()))
                return true;

            // Replan the route without forgetting which rider the bot was attacking.
            if (RaidPlan* plan = GetPlan(*bot))
                for (Member& member : plan->members)
                    if (member.guid == bot->GetGUID())
                    {
                        member.damagePosition.reset();
                        break;
                    }
            _lastDpsSearch[_corner] = getMSTime();
            return false;
        }
        if (!_holder && (assignment.opening || assignment.stagedHealer))
        {
            _tick.finished = true;
            DropAttack();
            if (assignment.stagedHealer && !assignment.opening)
            {
                if (bot->GetExactDist(position) <= CornerArrival)
                {
                    if (bot->isMoving())
                        bot->StopMoving();
                    return false;
                }
                return WalkTo(position, true);
            }
            return WalkTo(position);
        }

        if (!_fighting || _tick.target.IsEmpty())
        {
            _tick.finished = true;
            DropAttack();
        }
        if (_holder && _corner >= BlaumeuxCorner)
        {
            RaidPlan const& plan = *GetPlan(*bot);
            Bosses const bosses = ResolveBosses();
            Creature const* const rider = bosses[_corner];
            Creature const* const opposite = bosses[_corner ^ 1];
            if (rider && _tick.zeliekLane && rider->GetVictim() != bot && AtCorner(rider, _corner))
                return false;
            if (plan.earlyRearSecuredAt && EarlyRearPhase(plan, bosses) && rider && opposite &&
                rider->GetVictim() == bot && OwnsTarget(opposite, plan.holders[_corner ^ 1]) &&
                bot->GetAuraCount(Horsemen[_corner ^ 1].mark) >= RotationStacks)
            {
                constexpr float separation = RearMarkClearance + 5.0f;
                if (_hazards.KeepRearMarkSeparation(*bot, *opposite, separation))
                    return true;
                bool const clearOfOldMark = OutsideRearMark(*bot, opposite);
                if (clearOfOldMark && !OutsideRearMark(position, opposite))
                {
                    _hazards.StopWalkTo(*bot, position, CornerArrival);
                    return false;
                }
                bool const returning = plan.tankHandoffs[_corner].returning;
                return (returning ? WalkRearReturn(plan, position) : WalkTo(position, true)) ||
                       (!clearOfOldMark && _hazards.WalkRearOutsideMark(*bot, *opposite, separation));
            }
            if (plan.tankHandoffs[_corner].returning)
                return WalkRearReturn(plan, position);
        }
        return WalkTo(position, _holder);
    }

    bool WalkRearReturn(RaidPlan const& plan, Position const& point)
    {
        Player const* const healer = RearExchangeHealer(*bot, plan, *bot);
        return _hazards.WalkRearExchangeToPoint(*bot, point, CornerArrival,
                                                {healer != nullptr, healer, HealerCoverageRange});
    }

    bool AttackAssignedHorseman()
    {
        Creature* const target = Resolve(_tick.target);
        if (!AllowsOffense(target))
            return false;
        if ((_tick.dynamicDamage || target->GetExactDist(bot) <= AttackRange) &&
            AI_VALUE(Unit*, "current target") != target)
            return _attack.Attack(target);
        if (_holder && bot->getClass() == CLASS_DRUID && bot->IsWithinMeleeRange(target))
            return botAI->CastSpell("mangle (bear)", target) || botAI->CastSpell("maul", target);
        return false;
    }

    bool HealRearTank(Player& tank, RearHealCast cast = RearHealCast::Normal, bool confirmCoverage = false)
    {
        if ((confirmCoverage ? tank.IsFullHealth() : tank.GetHealthPct() >= RearTankUrgentHealth) ||
            bot->IsNonMeleeSpellCast(true))
            return false;
        for (char const* spell : RearHealingSpells(*bot))
        {
            if (!spell)
                return false;
            if (cast == RearHealCast::InstantOnly || (cast == RearHealCast::StopIfNeeded && bot->isMoving()))
            {
                uint32 const spellId = AI_VALUE2(uint32, "spell id", spell);
                SpellInfo const* info = sSpellMgr->GetSpellInfo(spellId);
                if (cast == RearHealCast::InstantOnly && (!info || info->IsChanneled() || info->CalcCastTime(bot) > 0))
                    continue;
                if (cast == RearHealCast::StopIfNeeded && info &&
                    (info->IsChanneled() ? info->GetDuration() != 0 : info->CalcCastTime(bot) > 0))
                {
                    if (!bot->HasSpell(spellId) || bot->HasSpellCooldown(spellId) ||
                        bot->HasUnitState(UNIT_STATE_LOST_CONTROL | UNIT_STATE_IN_FLIGHT) || bot->IsFlying() ||
                        !tank.IsInWorld() || tank.IsDuringRemoveFromWorld() || tank.IsImmunedToSpell(info) ||
                        sDisableMgr->IsDisabledFor(DISABLE_TYPE_SPELL, spellId, bot))
                        continue;

                    // Check mana separately so the probe does not register charged spell modifiers.
                    if (!bot->GetCommandStatus(CHEAT_POWER) &&
                        int32(bot->GetPower(POWER_MANA)) < info->CalcPowerCost(bot, info->GetSchoolMask()))
                        continue;

                    // CanCastSpell rejects timed heals in motion. Check the spell before stopping.
                    Spell check(bot, info, TRIGGERED_IGNORE_POWER_AND_REAGENT_COST);
                    check.m_targets.SetUnitTarget(&tank);
                    if (check.CheckCast(true) != SPELL_CAST_OK)
                        continue;

                    bot->StopMoving();
                }
            }
            if (botAI->CanCastSpell(spell, &tank) && botAI->CastSpell(spell, &tank))
                return true;
        }
        return false;
    }

    MeteorResponse RespondToMeteor(Assignment const& assignment, Bosses const& bosses)
    {
        Creature const* korth = bosses[0];
        if (!korth || assignment.holder ||
            (!_tick.dynamicDamage && (assignment.corner >= 2 || assignment.standby || assignment.clearingMarks)))
            return MeteorResponse::Ignore;
        Spell* const cast = korth->GetCurrentSpell(CURRENT_GENERIC_SPELL);
        if (!cast || cast->GetSpellInfo()->Id != KorthMeteor || cast->getState() != SPELL_STATE_PREPARING ||
            !cast->m_targets.HasDst())
            return MeteorResponse::Ignore;

        Position const& impact = *cast->m_targets.GetDstPos();
        float const distance = bot->GetExactDist(impact);
        if (assignment.corner != 0 && _corner != 0 && distance > MeteorSoakRange + CornerArrival)
            return MeteorResponse::Ignore;
        Creature const* const zeliek = bosses[ZeliekCorner];
        Unit const* const wrathVictim = zeliek ? zeliek->GetVictim() : nullptr;
        FourHorsemenMarkSource const wrath = {wrathVictim ? wrathVictim->GetPosition() : Position{},
                                              HolyWrathClearance};
        bool const spreadForWrath = wrathVictim && wrathVictim != bot;
        auto safeToSoak = [&](Position const& point)
        {
            if (spreadForWrath && point.GetExactDist2d(&wrath.location) < wrath.radius)
                return false;
            if (_tick.dynamicDamage)
                return _hazards.IsSafeFromVoidZones(*bot, point);

            uint8 nearbyMarks = 0;
            for (uint8 mark = 0; mark < CornerCount; ++mark)
            {
                uint32 const stacks = bot->GetAuraCount(Horsemen[mark].mark);
                if (stacks < RotationStacks)
                    continue;
                if (Creature const* source = MarkCaster(bot, bosses, mark);
                    source && point.GetExactDist(source) < MarkRadius(mark) + CornerArrival &&
                    (stacks > RotationStacks || ++nearbyMarks >= 2))
                    return false;
            }
            return _hazards.IsSafeFromVoidZones(*bot, point);
        };
        if (distance <= MeteorHoldRadius)
        {
            if (!safeToSoak(bot->GetPosition()))
                return MeteorResponse::Ignore;
            if (bot->isMoving())
                bot->StopMoving();
            return MeteorResponse::Hold;
        }

        int32 const remaining = cast->GetCastTimeRemaining();
        if (remaining <= int32(MeteorArrivalMarginMs))
            return MeteorResponse::Ignore;
        float const maxWalk = bot->GetSpeed(MOVE_RUN) * float(remaining - MeteorArrivalMarginMs) / 1000.0f;
        float const distance2d = bot->GetExactDist2d(impact);
        if (distance2d <= MeteorApproachRadius || distance2d - MeteorApproachRadius > maxWalk)
            return MeteorResponse::Ignore;

        if (PlayerbotAI::IsHeal(bot))
        {
            Spell const* const healing = bot->GetCurrentSpell(CURRENT_GENERIC_SPELL);
            Player const* const tank = PlayerInRaid(*bot, assignment.targetHolder);
            if ((healing && healing->GetSpellInfo()->IsPositive()) ||
                (tank && tank->GetHealthPct() < 70.0f && bot->IsWithinDistInMap(tank, HealerCoverageRange) &&
                 bot->IsWithinLOSInMap(tank)))
                return MeteorResponse::Ignore;
        }

        float const ratio = MeteorApproachRadius / distance2d;
        float const x = impact.GetPositionX() + ratio * (bot->GetPositionX() - impact.GetPositionX());
        float const y = impact.GetPositionY() + ratio * (bot->GetPositionY() - impact.GetPositionY());
        float const impactZ = impact.GetPositionZ();
        float const z = bot->GetMapHeight(x, y, impactZ + CornerArrival, true, 4.0f);
        Position const destination(x, y, z);
        if (destination.GetExactDist(impact) > MeteorHoldRadius || !safeToSoak(destination) ||
            !_hazards.WalkRear(*bot, destination, 0.75f, false, maxWalk, std::span(&wrath, spreadForWrath ? 1 : 0)))
            return MeteorResponse::Ignore;

        uint32 const now = getMSTime();
        if (!_lastMeteorMove || now - _lastMeteorMove > 2000)
        {
            _lastMeteorMove = now;
            LOG_INFO("playerbots", "[Naxx40] Horsemen instance={} {} closes to soak Meteor at {:.1f}yd ({}ms left)",
                     bot->GetInstanceId(), bot->GetName(), distance, remaining);
        }
        return MeteorResponse::Move;
    }

    bool BuildFrontWarriorThreat(Creature& target, uint32 openingAt)
    {
        ThreatManager const& threat = target.GetThreatMgr();
        float const ownThreat = threat.GetThreat(bot);
        float nextThreat = 0.0f;
        for (ThreatReference const* reference : threat.GetUnsortedThreatList())
            if (reference->IsOnline() && reference->GetVictim() != bot)
                nextThreat = std::max(nextThreat, reference->GetThreat());
        if (target.GetVictim() == bot && ownThreat >= nextThreat + FrontThreatLead)
            return false;

        bool const meleeRange = bot->IsWithinMeleeRange(&target);
        if (bot->GetCurrentSpell(CURRENT_AUTOREPEAT_SPELL))
        {
            if (!meleeRange && openingAt && getMSTimeDiff(openingAt, getMSTime()) < FrontOpeningShotWait)
                return true;
            bot->InterruptSpell(CURRENT_AUTOREPEAT_SPELL);
        }

        bool const castBloodrage = bot->GetPower(POWER_RAGE) < 150 && botAI->CanCastSpell(SPELL_BLOODRAGE, bot) &&
                                   botAI->CastSpell(SPELL_BLOODRAGE, bot);
        if (!meleeRange)
        {
            float const contactRange = std::max(8.0f, bot->GetMeleeRange(&target));
            if (_hazards.ApproachRear(*bot, target, contactRange, 1.0f))
                return true;
        }

        // AttackAction can accept the target even if the attack did not start.
        bool const startedAttack =
            bot->GetVictim() != &target && _attack.Attack(&target) && bot->GetVictim() == &target;
        if (meleeRange)
            for (char const* ability : {"shield slam", "devastate"})
                if (botAI->CanCastSpell(ability, &target) && botAI->CastSpell(ability, &target))
                {
                    LOG_INFO("playerbots", "[Naxx40] Horsemen {} instance={} {} uses {} on {} threat {:.0f}->{:.0f}",
                             openingAt ? "opening" : "front handoff", bot->GetInstanceId(), bot->GetName(), ability,
                             target.GetName(), ownThreat, threat.GetThreat(bot));
                    return true;
                }
        return castBloodrage || startedAttack;
    }

    Position SpacedBlaumeuxPosition(Position const& center, float radius)
    {
        float const angle = float(static_cast<uint32>(bot->GetGUID().GetRawValue()) % 12) * float(M_PI) / 6.0f;
        float const x = center.GetPositionX() + std::cos(angle) * radius;
        float const y = center.GetPositionY() + std::sin(angle) * radius;
        float const z = bot->GetMapHeight(x, y, center.GetPositionZ() + 2.0f, true, 4.0f);
        if (std::abs(z - center.GetPositionZ()) <= 1.5f)
        {
            Position const position(x, y, z);
            if (_hazards.IsSafeFromVoidZones(*bot, position))
                return position;
        }
        return center;
    }

    Position RearDestination(uint8 corner, Creature const* otherMark)
    {
        Position const& nominal = Horsemen[corner].corner;
        Position const* best = nullptr;
        float bestDistance = -1.0f;
        auto tryPad = [&](Position const& pad)
        {
            if (!_hazards.IsSafeFromVoidZones(*bot, pad))
                return false;
            float const distance = MarkDistance(pad, otherMark);
            if (!best || distance > bestDistance)
            {
                best = &pad;
                bestDistance = distance;
            }
            return OutsideRearMark(pad, otherMark);
        };

        if (tryPad(nominal))
            return nominal;
        for (Position const& pad : RearHolderPads[corner - 2])
            if (tryPad(pad))
                return pad;

        // Advance even if the opposite boss has not cleared the crossing yet.
        // The next snapshot measures both holders against the actual mark casters.
        return best ? *best : nominal;
    }

    bool WalkTo(Position const& position, bool checkedGround = false)
    {
        if (checkedGround || (_corner >= 2 && _corner < CornerCount && _hazards.HasVoidZones(*bot)))
            return _hazards.WalkRear(*bot, position, CornerArrival);
        if (bot->GetExactDist(position) <= CornerArrival || !_hazards.IsSafeFromVoidZones(*bot, position))
            return false;
        return MoveTo(NaxxramasMap, position.GetPositionX(), position.GetPositionY(), position.GetPositionZ(), false,
                      false, false, false, MovementPriority::MOVEMENT_COMBAT);
    }

    void DropAttack()
    {
        StopRiderDamage(*bot, nullptr);
        context->GetValue<Unit*>("current target")->Set(nullptr);
    }

    TickPlan _tick;
    uint8 _corner = NoCorner;
    bool _holder = false;
    bool _fighting = false;
    bool _centerRouteBlocked = false;
    uint32 _lastMeteorMove = 0;
    std::optional<Position> _lastRearDestination;
    Position _earlyRearHealerDestination;
    ObjectGuid _earlyRearHealerPatient;
    uint32 _earlyRearHealerSpline = 0;
    uint32 _earlyRearHealerProbeAt = 0;
    bool _earlyRearHealerCrossing = false;
    uint8 _rearHealerDescentCorner = NoCorner;
    std::array<uint32, ZeliekRaidHealPads.size()> _blockedZeliekHealAt{};
    std::array<uint32, CornerCount> _lastDpsSearch{};
    std::vector<FourHorsemenMarkSource> _zeliekWrathAvoid;
    std::array<Position, 2> const* _zeliekEnteredLane = nullptr;
    HorsemenAttack _attack;
    FourHorsemen40Hazards _hazards;
};

class FourHorsemenJobAction final : public Action
{
public:
    FourHorsemenJobAction(PlayerbotAI* ai, Job job) : Action(ai, JobName(job)), _job(job) {}

    bool isUseful() override
    {
        AssignHorsemenAction* const planner = GetPlanner();
        return planner && planner->IsUseful(_job);
    }

    bool Execute(Event /*event*/) override
    {
        AssignHorsemenAction* const planner = GetPlanner();
        return planner && planner->Execute(_job);
    }

private:
    AssignHorsemenAction* GetPlanner() const
    {
        return static_cast<AssignHorsemenAction*>(context->GetAction(JobName(Job::Assign)));
    }

    Job const _job;
};

class FourHorsemen40Trigger final : public Trigger
{
public:
    FourHorsemen40Trigger(PlayerbotAI* ai, Job job) : Trigger(ai, JobName(job)) {}

    bool IsActive() override { return ActiveEncounter(bot) && bot->IsAlive() && bot->IsInCombat(); }
};

class FourHorsemen40Multiplier final : public Multiplier
{
public:
    explicit FourHorsemen40Multiplier(PlayerbotAI* ai) : Multiplier(ai, "four horsemen 40 control") {}

    float GetValue(Action* action) override
    {
        if (!ActiveEncounter(bot) || dynamic_cast<AssignHorsemenAction*>(action) ||
            dynamic_cast<FourHorsemenJobAction*>(action))
            return 1.0f;

        bool const melee = dynamic_cast<MeleeAction*>(action) != nullptr;
        if ((bot->IsInCombat() && dynamic_cast<MovementAction*>(action) && !melee) || IsTauntAction(action) ||
            EncounterHelpers::IsAoeThreatAction(bot, action))
            return 0.0f;
        auto* tactics = static_cast<AssignHorsemenAction*>(context->GetAction(JobName(Job::Assign)));
        if (dynamic_cast<CastHealingSpellAction*>(action))
            return !tactics || tactics->AllowsGroupHeal(action->GetTarget()) ? 1.0f : 0.0f;
        if (dynamic_cast<PetAttackAction*>(action))
            return tactics && tactics->AllowsOffense(AI_VALUE(Unit*, "current target")) ? 1.0f : 0.0f;
        if (tactics && tactics->HoldingRearCorner() && bot->getClass() == CLASS_DRUID &&
            (dynamic_cast<CastCasterFormAction*>(action) || dynamic_cast<CastCancelBearFormAction*>(action) ||
             dynamic_cast<CastCancelDireBearFormAction*>(action)))
            return 0.0f;

        Unit* target = action->GetTarget();
        auto* cast = dynamic_cast<CastSpellAction*>(action);
        if (cast && tactics && tactics->DamageWaitingAtCenter())
            return 0.0f;
        SpellInfo const* spell =
            cast ? sSpellMgr->GetSpellInfo(AI_VALUE2(uint32, "spell id", cast->getSpell())) : nullptr;
        if (tactics && tactics->HoldingCorner() && spell)
        {
            if (spell->Id == SPELL_BATTLE_STANCE || spell->Id == SPELL_BERSERKER_STANCE ||
                spell->Id == SPELL_BLOOD_PRESENCE || spell->Id == SPELL_UNHOLY_PRESENCE)
                return 0.0f;
            if (bot->getClass() == CLASS_DRUID && spell->HasAura(SPELL_AURA_MOD_SHAPESHIFT) &&
                spell->Id != SPELL_BEAR_FORM && spell->Id != SPELL_DIRE_BEAR_FORM)
                return 0.0f;
        }

        if (!melee && !(target && bot->IsValidAttackTarget(target)) && !(spell && !spell->IsPositive()))
            return 1.0f;

        if (!target || !bot->IsValidAttackTarget(target))
            target = AI_VALUE(Unit*, "current target");
        return tactics && tactics->AllowsOffense(target) ? 1.0f : 0.0f;
    }
};
}  // namespace

void FourHorsemen40::AddRearHealingScript() { new RearHealerConfirmationScript(); }

void FourHorsemen40::ObserveVoidExposure(Player& bot)
{
    FourHorsemen40Hazards::ObserveVoidExposure(bot, ActiveEncounter(&bot) && bot.IsAlive());
}

FourHorsemen40::Preparation FourHorsemen40::PrepareOpening(PlayerbotAI& ai)
{
    Player* const coordinator = ai.GetBot();
    InstanceScript* const instance = OpeningInstance(coordinator);
    if (!instance || !IsAutonomousBot(coordinator))
        return Preparation::Inactive;

    RaidPlan& plan = *coordinator->GetMap()->CustomData.GetDefault<RaidPlan>(PlanKey);
    uint32 const now = getMSTime();
    bool const validHolders = std::ranges::all_of(
        plan.holders, [coordinator](ObjectGuid guid) { return CanHold(PlayerInRaid(*coordinator, guid)); });
    bool const validStaging =
        plan.opening &&
        std::ranges::all_of(plan.opening->members,
                            [coordinator](auto const& seat)
                            {
                                Player* member = PlayerInRaid(*coordinator, seat.guid);
                                return IsAutonomousBot(member) &&
                                       (seat.healerSlot == NoSupportSlot || PlayerbotAI::IsHeal(member));
                            });
    bool const ownsOpening = plan.group == coordinator->GetGroup()->GetGUID() && plan.opening;
    if (!ownsOpening || !validStaging || plan.opening->pulled || !validHolders || OpeningExpired(*plan.opening, now) ||
        NeedsAdditionalOpeningHealers(*coordinator, *plan.opening))
    {
        if (!StartOpening(*coordinator, plan, now))
            return Preparation::Inactive;
    }

    Player* const leader = HolderInRaid(*coordinator, plan.opening->leader);
    if (!leader)
        return Preparation::Inactive;
    return DriveOpening(*leader, plan, ReadBosses(leader, *instance), now);
}

ObjectGuid FourHorsemen40::GetOpeningHolder(Player const& member)
{
    RaidPlan const* const plan = GetPlan(member);
    if (!plan || !member.GetGroup() || plan->group != member.GetGroup()->GetGUID() || !plan->opening ||
        plan->opening->pulled || OpeningExpired(*plan->opening, getMSTime()))
        return ObjectGuid::Empty;
    return plan->opening->leader;
}

bool FourHorsemen40::StagePullHolder(PlayerbotAI& ai)
{
    Player* const holder = ai.GetBot();
    InstanceScript* const instance = OpeningInstance(holder);
    if (!instance || !CanHold(holder) || GetOpeningHolder(*holder) != holder->GetGUID())
        return false;
    OpeningPlan& staging = *GetPlan(*holder)->opening;
    OpeningMember& moving = staging.pullHolder;
    uint32 const now = getMSTime();
    if (staging.readyFlanks != BothFlanks || !staging.mograineReady ||
        holder->GetExactDist(KorthPullPoint) <= MograinePullArrival || !ai.CanMove() ||
        (!holder->movespline->Finalized() && holder->movespline->GetId() == moving.spline) ||
        (moving.lastMove && now - moving.lastMove < 1200))
        return false;

    moving.lastMove = now;
    return WalkOpeningLeg(ai, *holder, KorthPullPoint, ReadBosses(holder, *instance), moving, 0.0f);
}

bool FourHorsemen40::PullOpening(PlayerbotAI& coordinatorAI)
{
    Player* const coordinator = coordinatorAI.GetBot();
    InstanceScript* const instance = OpeningInstance(coordinator);
    if (!instance || !IsAutonomousBot(coordinator))
        return false;

    ObjectGuid const holder = GetOpeningHolder(*coordinator);
    Player* const leader = HolderInRaid(*coordinator, holder);
    if (!CanHold(leader) || leader->GetExactDist(KorthPullPoint) > MograinePullArrival ||
        !leader->movespline->Finalized())
        return false;

    RaidPlan& plan = *GetPlan(*coordinator);
    PlayerbotAI& ai = *GET_PLAYERBOT_AI(leader);
    Bosses const bosses = ReadBosses(leader, *instance);
    uint32 const now = getMSTime();
    if (DriveOpening(*leader, plan, bosses, now) != Preparation::Ready)
        return false;

    OpeningPlan& opening = *plan.opening;
    if (opening.pullRequestedAt && now - opening.pullRequestedAt < 4000)
        return true;
    auto wait = [&](Player const& tank, char const* reason, float distance = 0.0f)
    {
        if (!opening.lastReport || now - opening.lastReport >= 5000)
        {
            opening.lastReport = now;
            LOG_INFO("playerbots", "[Naxx40] Horsemen opening instance={} waiting for {}: {} ({:.1f}yd)",
                     leader->GetInstanceId(), tank.GetName(), reason, distance);
        }
        return false;
    };
    uint32 const form = RequiredHolderBuff(*leader);
    if (form)
        return ApplyHolderBuff(ai, form, opening.startedAt) || wait(*leader, "tank form unavailable");

    std::array<OpeningTag, OpeningTankCorners.size()> tags;
    for (std::size_t i = 0; i < tags.size(); ++i)
    {
        OpeningTag& tag = tags[i] = OpeningTagFor(*leader, plan, bosses, OpeningTankCorners[i]);
        if (!tag.taunt)
            return wait(*leader, "opening tank or taunt missing");
        float const distance = tag.boss->GetExactDist(tag.tank);
        if (distance > OpeningTauntRange || !tag.tank->IsWithinLOSInMap(tag.boss))
            return wait(*tag.tank, "taunt position", distance);
        // A reactive taunt may depend on combat threat, which this pull creates.
        if (!tag.tank->HasSpell(tag.taunt) || tag.tank->HasSpellCooldown(tag.taunt) ||
            !GET_PLAYERBOT_AI(tag.tank)->CanMove())
            return wait(*tag.tank, "taunt unavailable", distance);
        tag.tank->SetFacingToObject(tag.boss);
    }

    // The first damaging spell starts the encounter and initializes threat
    // on all riders. The other tanks can then taunt on the same map tick.
    OpeningTag const* first = nullptr;
    for (OpeningTag const& tag : tags)
        if (tag.attack && GET_PLAYERBOT_AI(tag.tank)->CanCastSpell(tag.attack, tag.boss) &&
            (!first || tag.attack == SPELL_HAND_OF_RECKONING))
            first = &tag;
    bool cast = false;
    if (first)
        cast = GET_PLAYERBOT_AI(first->tank)->CastSpell(first->attack, first->boss);
    if (!cast)
        for (OpeningTag const& tag : tags)
        {
            HorsemenOpeningShot shot(GET_PLAYERBOT_AI(tag.tank), tag.boss);
            if (shot.isPossible() && shot.Execute(Event()))
            {
                first = &tag;
                cast = true;
                break;
            }
        }
    if (!cast)
        return wait(first ? *first->tank : *leader, "damaging opener unavailable");
    opening.pullRequestedAt = now;
    opening.firstTag = first->corner;
    LOG_INFO("playerbots", "[Naxx40] Horsemen opening instance={} {} starts coordinated pull on {}",
             leader->GetInstanceId(), first->tank->GetName(), first->boss->GetName());
    if (instance->GetBossState(Encounter) == IN_PROGRESS)
        FinishOpeningTags(*leader, plan, bosses);
    return true;
}

bool FourHorsemen40::ProtectsTankSupplies(Player* player)
{
    if (!player || !Naxx40Horsemen::IsNaxx40(player) || !IsAutonomousBot(player) || !player->GetGroup())
        return false;

    RaidPlan const* const plan = GetPlan(*player);
    if (!plan || plan->group != player->GetGroup()->GetGUID() || !plan->opening || !plan->opening->suppliesStarted ||
        std::ranges::find(plan->holders, player->GetGUID()) == plan->holders.end())
        return false;

    InstanceScript const* const instance = player->GetInstanceScript();
    return instance && (instance->GetBossState(Encounter) == IN_PROGRESS ||
                        (instance->GetBossState(Encounter) == NOT_STARTED && !plan->opening->pulled &&
                         !OpeningExpired(*plan->opening, getMSTime())));
}

bool FourHorsemen40::ConflictsWithTankSupplies(uint32 spellId)
{
    SpellInfo const* const spell = sSpellMgr->GetSpellInfo(spellId);
    if (!spell || spellId == SPELL_FLASK || spellId == SPELL_FOOD)
        return false;

    auto const conflicts = [spell](uint32 supply)
    {
        SpellInfo const* const desired = sSpellMgr->GetSpellInfo(supply);
        if (!desired)
            return false;
        SpellGroupStackRule const rule = sSpellMgr->CheckSpellGroupStackRules(spell, desired);
        return rule == SPELL_GROUP_STACK_RULE_EXCLUSIVE || rule == SPELL_GROUP_STACK_RULE_EXCLUSIVE_FROM_SAME_CASTER;
    };
    return conflicts(SPELL_FLASK) || conflicts(SPELL_FOOD);
}

bool FourHorsemen40::HasTemporaryStance(Player const& player)
{
    return player.CustomData.Get<WarriorStanceChange>(StanceKey) != nullptr;
}

bool FourHorsemen40::RestoreWarriorStance(PlayerbotAI& ai)
{
    Player* const bot = ai.GetBot();
    if (bot->getClass() != CLASS_WARRIOR || !bot->IsAlive() || bot->IsInCombat())
        return false;

    WarriorStanceChange const* const previous = bot->CustomData.Get<WarriorStanceChange>(StanceKey);
    if (!previous)
        return false;
    WarriorStanceChange const saved = *previous;

    if (Naxx40Horsemen::IsNaxx40(bot) && bot->GetInstanceId() == saved.instanceId)
    {
        InstanceScript const* const instance = bot->GetInstanceScript();
        EncounterState const state = instance ? instance->GetBossState(Encounter) : TO_BE_DECIDED;
        if (state == IN_PROGRESS)
            return false;

        if (state == NOT_STARTED && saved.openingStartedAt)
        {
            RaidPlan const* const plan = GetPlan(*bot);
            if (plan && plan->opening)
            {
                OpeningPlan const& opening = *plan->opening;
                if (opening.startedAt == saved.openingStartedAt && !opening.pulled &&
                    !OpeningExpired(opening, getMSTime()))
                    return false;
            }
        }
    }

    ShapeshiftForm const current = bot->GetShapeshiftForm();
    if (current != saved.original && (current == FORM_DEFENSIVESTANCE || current == FORM_NONE))
    {
        if (saved.original == FORM_BATTLESTANCE)
            return ai.CastSpell(SPELL_BATTLE_STANCE, bot);
        if (saved.original == FORM_BERSERKERSTANCE)
            return ai.CastSpell(SPELL_BERSERKER_STANCE, bot);
        if (saved.original == FORM_NONE)
            bot->RemoveAurasDueToSpell(SPELL_DEFENSIVE_STANCE);
    }

    bot->CustomData.Erase(StanceKey);
    if (bot->GetShapeshiftForm() == saved.original)
        LOG_INFO("playerbots", "[Naxx40] Horsemen instance={} {} restored warrior stance {}", saved.instanceId,
                 bot->GetName(), static_cast<uint32>(saved.original));
    return false;
}

FourHorsemen40ActionContext::FourHorsemen40ActionContext()
{
    creators[JobName(Job::Assign)] = [](PlayerbotAI* ai) { return new AssignHorsemenAction(ai); };
    creators[JobName(Job::VoidZone)] = [](PlayerbotAI* ai) { return new FourHorsemenJobAction(ai, Job::VoidZone); };
    creators[JobName(Job::ClearMarks)] = [](PlayerbotAI* ai) { return new FourHorsemenJobAction(ai, Job::ClearMarks); };
    creators[JobName(Job::Meteor)] = [](PlayerbotAI* ai) { return new FourHorsemenJobAction(ai, Job::Meteor); };
    creators[JobName(Job::RearTankCoverage)] = [](PlayerbotAI* ai)
    { return new FourHorsemenJobAction(ai, Job::RearTankCoverage); };
    creators[JobName(Job::TankControl)] = [](PlayerbotAI* ai)
    { return new FourHorsemenJobAction(ai, Job::TankControl); };
    creators[JobName(Job::Position)] = [](PlayerbotAI* ai) { return new FourHorsemenJobAction(ai, Job::Position); };
    creators[JobName(Job::Attack)] = [](PlayerbotAI* ai) { return new FourHorsemenJobAction(ai, Job::Attack); };
}

FourHorsemen40TriggerContext::FourHorsemen40TriggerContext()
{
    creators[JobName(Job::Assign)] = [](PlayerbotAI* ai) { return new FourHorsemen40Trigger(ai, Job::Assign); };
    creators[JobName(Job::VoidZone)] = [](PlayerbotAI* ai) { return new FourHorsemen40Trigger(ai, Job::VoidZone); };
    creators[JobName(Job::ClearMarks)] = [](PlayerbotAI* ai) { return new FourHorsemen40Trigger(ai, Job::ClearMarks); };
    creators[JobName(Job::Meteor)] = [](PlayerbotAI* ai) { return new FourHorsemen40Trigger(ai, Job::Meteor); };
    creators[JobName(Job::RearTankCoverage)] = [](PlayerbotAI* ai)
    { return new FourHorsemen40Trigger(ai, Job::RearTankCoverage); };
    creators[JobName(Job::TankControl)] = [](PlayerbotAI* ai)
    { return new FourHorsemen40Trigger(ai, Job::TankControl); };
    creators[JobName(Job::Position)] = [](PlayerbotAI* ai) { return new FourHorsemen40Trigger(ai, Job::Position); };
    creators[JobName(Job::Attack)] = [](PlayerbotAI* ai) { return new FourHorsemen40Trigger(ai, Job::Attack); };
}

void AddFourHorsemen40Triggers(std::vector<TriggerNode*>& triggers)
{
    // Keep the historical safety and control order within one raid relevance band.
    for (std::size_t index = 0; index < JobNames.size(); ++index)
    {
        char const* name = JobNames[index];
        float const relevance = ACTION_RAID + 4 + float(JobNames.size() - 1 - index) * 0.125f;
        triggers.push_back(new TriggerNode(name, {NextAction(name, relevance)}));
    }
}

void AddFourHorsemen40Multipliers(PlayerbotAI* botAI, std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new FourHorsemen40Multiplier(botAI));
}
