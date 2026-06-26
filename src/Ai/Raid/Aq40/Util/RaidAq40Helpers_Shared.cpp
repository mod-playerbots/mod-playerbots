#include "RaidAq40Helpers_Shared.h"

#include <algorithm>
#include <cctype>
#include <mutex>
#include <sstream>
#include <unordered_map>

#include "Event.h"
#include "FollowActions.h"
#include "LastMovementValue.h"
#include "MotionMaster.h"
#include "Playerbots.h"
#include "PositionValue.h"
#include "../RaidAq40BossHelper.h"
#include "RaidAq40Helpers_Cthun.h"
#include "RaidAq40Helpers_Skeram.h"
#include "RaidAq40TwinEncounter.h"
#include "Timer.h"

namespace Aq40Helpers
{

namespace
{
std::unordered_map<std::string, uint32> sAq40LogLastMsByKey;
std::mutex sAq40LogMutex;
uint32 constexpr kAq40StrategyLogThrottleMs = 5000;

std::string ToAq40LogToken(std::string value)
{
    std::string token;
    bool lastWasSeparator = false;
    for (char ch : value)
    {
        unsigned char const uch = static_cast<unsigned char>(ch);
        if (std::isalnum(uch))
        {
            token.push_back(static_cast<char>(std::tolower(uch)));
            lastWasSeparator = false;
        }
        else if (!lastWasSeparator && !token.empty())
        {
            token.push_back('_');
            lastWasSeparator = true;
        }
    }

    while (!token.empty() && token.back() == '_')
        token.pop_back();

    return token.empty() ? "unknown" : token;
}

uint32 GetAq40LogInstanceId(Player* bot)
{
    if (!bot)
        return 0;

    if (bot->GetMap())
        return bot->GetMap()->GetInstanceId();

    return bot->GetMapId();
}

Player* GetAq40FollowLeader(PlayerbotAI* botAI)
{
    if (!botAI)
        return nullptr;

    if (Player* master = botAI->GetMaster())
        return master;

    return botAI->GetGroupLeader();
}

bool IsAq40FollowRecoveryCandidate(Player* bot, PlayerbotAI* botAI)
{
    if (!bot || !botAI || !botAI->GetAiObjectContext() || !Aq40BossHelper::IsInAq40(bot) || bot->IsInCombat() ||
        bot->IsBeingTeleported() || bot->HasUnitState(UNIT_STATE_IN_FLIGHT) || bot->IsNonMeleeSpellCast(true) ||
        !botAI->HasStrategy("follow", BOT_STATE_NON_COMBAT) || bot->isMoving())
    {
        return false;
    }

    FollowAction followAction(botAI);
    return followAction.isUseful();
}

void LogAq40(Player* bot, std::string const& eventKey, std::string const& stateKey,
             std::string const& fields, uint32 throttleMs, bool warn)
{
    if (!sPlayerbotAIConfig.aq40StrategyLog || !bot)
        return;

    uint32 const instanceId = GetAq40LogInstanceId(bot);
    uint64 const botGuid = bot->GetGUID().GetRawValue();
    uint32 effectiveThrottleMs = throttleMs;
    if (uint32 const configuredThrottleMs = sPlayerbotAIConfig.aq40StrategyLogThrottleMs)
        effectiveThrottleMs = effectiveThrottleMs ? std::max(effectiveThrottleMs, configuredThrottleMs) : configuredThrottleMs;
    else if (!effectiveThrottleMs)
        effectiveThrottleMs = kAq40StrategyLogThrottleMs;

    std::ostringstream key;
    key << instanceId << ":" << botGuid << ":" << ToAq40LogToken(eventKey) << ":" << stateKey;
    std::string const logKey = key.str();

    uint32 const now = getMSTime();
    {
        std::lock_guard<std::mutex> guard(sAq40LogMutex);
        auto const itr = sAq40LogLastMsByKey.find(logKey);
        if (effectiveThrottleMs > 0 && itr != sAq40LogLastMsByKey.end() && now - itr->second < effectiveThrottleMs)
            return;

        sAq40LogLastMsByKey[logKey] = now;
    }

    std::ostringstream line;
    line << "event=" << ToAq40LogToken(eventKey)
         << " bot=" << ToAq40LogToken(bot->GetName())
         << " role=" << GetAq40LogRole(bot, GET_PLAYERBOT_AI(bot))
         << " instance=" << instanceId;
    if (!fields.empty())
        line << " " << fields;

    if (warn)
        LOG_WARN("playerbots_aq40", "AQ40 {}", line.str());
    else
        LOG_INFO("playerbots_aq40", "AQ40 {}", line.str());
}
}    // namespace

std::string GetAq40LogToken(std::string value)
{
    return ToAq40LogToken(value);
}

std::string GetAq40LogUnit(Unit* unit)
{
    if (!unit)
        return "none";

    std::ostringstream out;
    out << ToAq40LogToken(unit->GetName()) << ":" << unit->GetGUID().GetCounter();
    return out.str();
}

std::string GetAq40LogRole(Player* bot, PlayerbotAI* botAI)
{
    if (!bot || !botAI)
        return "unknown";

    if (Aq40BossHelper::IsEncounterTank(bot, bot))
        return "tank";
    if (botAI->IsHeal(bot))
        return "healer";
    if (botAI->IsRanged(bot))
        return "ranged";
    return "melee";
}

void LogAq40Info(Player* bot, std::string const& eventKey, std::string const& stateKey,
                 std::string const& fields, uint32 throttleMs)
{
    LogAq40(bot, eventKey, stateKey, fields, throttleMs, false);
}

void LogAq40Warn(Player* bot, std::string const& eventKey, std::string const& stateKey,
                 std::string const& fields, uint32 throttleMs)
{
    LogAq40(bot, eventKey, stateKey, fields, throttleMs, true);
}

void LogAq40Target(Player* bot, std::string const& boss, std::string const& reason, Unit* target,
                   uint32 throttleMs)
{
    std::ostringstream fields;
    fields << "boss=" << ToAq40LogToken(boss)
           << " target=" << GetAq40LogUnit(target)
           << " reason=" << ToAq40LogToken(reason);
    LogAq40Info(bot, "target_change", boss + ":" + reason + ":" + GetAq40LogUnit(target),
                fields.str(), throttleMs);
}

bool TryRecoverAq40FollowState(Player* bot, PlayerbotAI* botAI, std::string const& eventKey,
                               std::string const& stateKey, bool executeFollowMovement)
{
    if (!IsAq40FollowRecoveryCandidate(bot, botAI))
        return false;

    auto* context = botAI->GetAiObjectContext();
    PositionMap& positionMap = context->GetValue<PositionMap&>("position")->Get();
    PositionInfo stayPosition = positionMap["stay"];
    PositionInfo returnPosition = positionMap["return"];
    PositionInfo randomPosition = positionMap["random"];
    PositionInfo singleStayPosition = context->GetValue<PositionInfo>("pos", "stay")->Get();
    Player* const followLeader = GetAq40FollowLeader(botAI);

    bool const hadStayStrategyNonCombat = botAI->HasStrategy("stay", BOT_STATE_NON_COMBAT);
    bool const hadStayStrategyCombat = botAI->HasStrategy("stay", BOT_STATE_COMBAT);
    bool const hadStayPosition = stayPosition.isSet();
    bool const hadReturnPosition = returnPosition.isSet();
    bool const hadRandomPosition = randomPosition.isSet();
    bool const hadSingleStayPosition = singleStayPosition.isSet();
    time_t const stayTime = context->GetValue<time_t>("stay time")->Get();
    LastMovement& lastMovement = context->GetValue<LastMovement&>("last movement")->Get();
    LastMovement& lastAreaTrigger = context->GetValue<LastMovement&>("last area trigger")->Get();
    LastMovement& lastTaxi = context->GetValue<LastMovement&>("last taxi")->Get();
    bool const hadLastMovement = lastMovement.lastFollow || lastMovement.lastMoveToMapId || lastMovement.msTime ||
                                 lastMovement.lastFlee || !lastMovement.taxiNodes.empty();
    bool const hadLastAreaTrigger = lastAreaTrigger.lastAreaTrigger || lastAreaTrigger.msTime;
    bool const hadLastTaxi = lastTaxi.lastMoveToMapId || lastTaxi.msTime || !lastTaxi.taxiNodes.empty();
    MovementGeneratorType const motionType =
        bot->GetMotionMaster() ? bot->GetMotionMaster()->GetCurrentMovementGeneratorType() : IDLE_MOTION_TYPE;
    bool const hadMotion = motionType != IDLE_MOTION_TYPE;

    bool changed = false;
    if (hadStayStrategyNonCombat)
    {
        botAI->ChangeStrategy("-stay", BOT_STATE_NON_COMBAT);
        changed = true;
    }

    if (hadStayStrategyCombat)
    {
        botAI->ChangeStrategy("-stay", BOT_STATE_COMBAT);
        changed = true;
    }

    if (hadStayPosition)
    {
        stayPosition.Reset();
        positionMap["stay"] = stayPosition;
        changed = true;
    }

    if (hadReturnPosition)
    {
        returnPosition.Reset();
        positionMap["return"] = returnPosition;
        changed = true;
    }

    if (hadRandomPosition)
    {
        randomPosition.Reset();
        positionMap["random"] = randomPosition;
        changed = true;
    }

    if (hadSingleStayPosition)
    {
        context->GetValue<PositionInfo>("pos", "stay")->Reset();
        changed = true;
    }

    if (stayTime)
    {
        context->GetValue<time_t>("stay time")->Set(0);
        changed = true;
    }

    if (hadLastMovement)
    {
        lastMovement.clear();
        changed = true;
    }

    if (hadLastAreaTrigger)
    {
        lastAreaTrigger.clear();
        changed = true;
    }

    if (hadLastTaxi)
    {
        lastTaxi.clear();
        changed = true;
    }

    if (hadMotion && bot->GetMotionMaster())
    {
        bot->GetMotionMaster()->Clear();
        changed = true;
    }

    bot->ClearUnitState(UNIT_STATE_CHASE);
    bot->ClearUnitState(UNIT_STATE_FOLLOW);

    FollowAction followAction(botAI);
    bool const followUsefulAfterReset = followAction.isUseful();
    bool const executedFollow = executeFollowMovement && followUsefulAfterReset && followAction.Execute(Event());

    if (!changed && !executedFollow)
        return false;

    std::ostringstream fields;
    fields << "boss=shared state=follow_recovery"
           << " trigger=" << GetAq40LogToken(stateKey)
           << " follow_target=" << GetAq40LogUnit(followLeader)
           << " follow_useful=" << (followUsefulAfterReset ? 1 : 0)
           << " stay_strategy_nc=" << (hadStayStrategyNonCombat ? 1 : 0)
           << " stay_strategy_combat=" << (hadStayStrategyCombat ? 1 : 0)
           << " stay_position_set=" << (hadStayPosition ? 1 : 0)
           << " return_position_set=" << (hadReturnPosition ? 1 : 0)
           << " random_position_set=" << (hadRandomPosition ? 1 : 0)
           << " single_stay_position_set=" << (hadSingleStayPosition ? 1 : 0)
           << " stay_time_set=" << (stayTime ? 1 : 0)
           << " last_movement_set=" << (hadLastMovement ? 1 : 0)
           << " last_area_trigger_set=" << (hadLastAreaTrigger ? 1 : 0)
           << " last_taxi_set=" << (hadLastTaxi ? 1 : 0)
           << " motion_type=" << static_cast<uint32>(motionType)
           << " executed_follow=" << (executedFollow ? 1 : 0);
    if (followLeader)
        fields << " follow_distance=" << bot->GetDistance2d(followLeader);

    LogAq40Info(bot, eventKey, stateKey, fields.str(), 1000);
    return true;
}

bool HasManagedResistanceStrategy(Player* bot, PlayerbotAI* botAI)
{
    if (!bot || !botAI)
        return false;

    return HasManagedResistanceState(bot);
}

bool IsResistanceManagementNeeded(Player* bot, PlayerbotAI* botAI, GuidVector const& attackers)
{
    if (!bot || !botAI || !Aq40BossHelper::IsInAq40(bot))
        return false;

    GuidVector const activeUnits = Aq40BossHelper::GetActiveCombatUnits(botAI, attackers);
    bool const needNatureResistance =
        Aq40BossHelper::HasAnyNamedUnit(botAI, activeUnits,
            { "princess huhuran", "viscidus", "glob of viscidus", "toxic slime" });
    Aq40TwinEncounter::TwinEncounterState const* twinState = Aq40TwinEncounter::GetEncounterState(bot);
    bool const needTwinShadowResistance =
        twinState &&
        Aq40TwinEncounter::HasDeterministicAssignments(*twinState) &&
        Aq40TwinEncounter::GetAssignmentForMember(*twinState, bot->GetGUID()) &&
        !Aq40TwinEncounter::IsTerminalPhase(twinState->phase) &&
        (((twinState->mode == Aq40TwinEncounter::TwinStrategyMode::StandardCompReady) &&
          twinState->phase == Aq40TwinEncounter::TwinEncounterPhase::PrePull) ||
         Aq40TwinEncounter::IsActivePhase(twinState->phase) ||
         twinState->phase == Aq40TwinEncounter::TwinEncounterPhase::Degraded);

    switch (bot->getClass())
    {
        case CLASS_HUNTER:
        case CLASS_SHAMAN:
            return needNatureResistance || HasManagedResistanceStrategy(bot, botAI);
        case CLASS_PRIEST:
        case CLASS_PALADIN:
            return needTwinShadowResistance || HasManagedResistanceStrategy(bot, botAI);
        default:
            return false;
    }
}

bool ResetEncounterState(Player* bot)
{
    bool const hadCthunState = ResetCthunEncounterState(bot);
    bool const hadSkeramState = ResetSkeramEncounterState(bot);
    bool const hadTwinState = Aq40TwinEncounter::ResetState(bot);
    bool const erased = hadCthunState || hadSkeramState || hadTwinState;

    if (erased && bot && bot->GetMap())
    {
        uint32 const instanceId = bot->GetMap()->GetInstanceId();
        LogAq40Info(bot, "encounter_reset", "shared:" + std::to_string(instanceId),
            "boss=shared state=reset instance=" + std::to_string(instanceId), 30000);
    }

    return erased;
}

bool HasPersistentEncounterState(Player* bot)
{
    return HasCthunEncounterState(bot) || HasSkeramEncounterState(bot) || Aq40TwinEncounter::HasPersistentState(bot);
}

bool ShouldSuppressTwinPrePullMaintenance(Player* bot, PlayerbotAI* botAI, char const* trigger)
{
    if (!bot || !botAI)
        return false;

    Aq40TwinEncounter::TwinEncounterState const* twinState = Aq40TwinEncounter::GetEncounterState(bot);
    if (!twinState || !Aq40TwinEncounter::HasDeterministicAssignments(*twinState) ||
        twinState->phase != Aq40TwinEncounter::TwinEncounterPhase::PrePull ||
        Aq40TwinEncounter::IsTerminalPhase(twinState->phase))
    {
        return false;
    }

    bool const followRecoveryCandidate = IsAq40FollowRecoveryCandidate(bot, botAI);
    bool const hasTwinLocalCleanupState = Aq40TwinEncounter::HasTwinLocalCleanupState(bot);
    if (!followRecoveryCandidate && !hasTwinLocalCleanupState)
        return false;

    std::ostringstream fields;
    fields << "boss=twin state=cleanup_suppressed"
           << " trigger=" << GetAq40LogToken(trigger ? trigger : "maintenance")
           << " phase=" << Aq40TwinEncounter::ToString(twinState->phase)
           << " mode=" << Aq40TwinEncounter::ToString(twinState->mode)
           << " approach=" << twinState->approachMemberCount
           << " staged=" << twinState->stagedMemberCount
           << " center_committed=" << twinState->centerCommittedMemberCount
           << " strict_ready=" << twinState->strictReadyMemberCount
           << " assigned=" << twinState->assignments.size()
           << " follow_recovery_candidate=" << (followRecoveryCandidate ? 1 : 0)
           << " local_cleanup_state=" << (hasTwinLocalCleanupState ? 1 : 0);
    LogAq40Info(bot, "encounter_reset", "twin:cleanup_suppressed", fields.str(), 1000);
    return true;
}

bool ShouldRunOutOfCombatMaintenance(Player* bot, PlayerbotAI* botAI)
{
    if (!bot || !botAI)
        return false;

    bool const hasManagedResistanceStrategy = HasManagedResistanceStrategy(bot, botAI);
    bool const hasTwinLocalCleanupState = Aq40TwinEncounter::HasTwinLocalCleanupState(bot);
    Aq40TwinEncounter::TwinEncounterState const* twinState = Aq40TwinEncounter::GetEncounterState(bot);
    bool const suppressTwinPrePullMaintenance =
        ShouldSuppressTwinPrePullMaintenance(bot, botAI, "out_of_combat_maintenance");

    if (hasManagedResistanceStrategy)
        return true;

    if (suppressTwinPrePullMaintenance)
        return false;

    if (IsAq40FollowRecoveryCandidate(bot, botAI))
        return true;

    bool const isTwinPrePullReady =
        twinState &&
        twinState->mode == Aq40TwinEncounter::TwinStrategyMode::StandardCompReady &&
        twinState->phase == Aq40TwinEncounter::TwinEncounterPhase::PrePull &&
        Aq40TwinEncounter::HasDeterministicAssignments(*twinState);
    bool const hasPersistentEncounterState = HasPersistentEncounterState(bot);

    if (hasTwinLocalCleanupState && !isTwinPrePullReady)
        return true;

    if (!hasPersistentEncounterState)
        return false;

    GuidVector const attackers = botAI->GetAiObjectContext()->GetValue<GuidVector>("attackers")->Get();

    if (!Aq40BossHelper::GetActiveCombatUnits(botAI, attackers).empty())
        return false;

    if (IsSkeramEncounterLive(bot, botAI, attackers))
        return false;

    ClearSkeramPostBlinkHold(bot);
    return true;
}

}    // namespace Aq40Helpers
