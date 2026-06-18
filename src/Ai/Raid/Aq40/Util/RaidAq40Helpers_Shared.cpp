#include "RaidAq40Helpers_Shared.h"

#include <cctype>
#include <mutex>
#include <sstream>
#include <unordered_map>

#include "Playerbots.h"
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

void LogAq40(Player* bot, std::string const& eventKey, std::string const& stateKey,
             std::string const& fields, uint32 throttleMs, bool warn)
{
    if (!sPlayerbotAIConfig.aq40StrategyLog || !bot)
        return;

    uint32 const instanceId = GetAq40LogInstanceId(bot);
    uint64 const botGuid = bot->GetGUID().GetRawValue();
    uint32 const effectiveThrottleMs = throttleMs ? throttleMs : sPlayerbotAIConfig.aq40StrategyLogThrottleMs;

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

bool ShouldRunOutOfCombatMaintenance(Player* bot, PlayerbotAI* botAI)
{
    if (!bot || !botAI)
        return false;

    bool const hasManagedResistanceStrategy = HasManagedResistanceStrategy(bot, botAI);
    bool const hasTwinLocalCleanupState = Aq40TwinEncounter::HasTwinLocalCleanupState(bot);
    Aq40TwinEncounter::TwinEncounterState const* twinState = Aq40TwinEncounter::GetEncounterState(bot);
    bool const isTwinPrePullReady =
        twinState &&
        twinState->mode == Aq40TwinEncounter::TwinStrategyMode::StandardCompReady &&
        twinState->phase == Aq40TwinEncounter::TwinEncounterPhase::PrePull &&
        Aq40TwinEncounter::HasDeterministicAssignments(*twinState);
    bool const hasPersistentEncounterState = HasPersistentEncounterState(bot);

    if (hasManagedResistanceStrategy)
        return true;

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
