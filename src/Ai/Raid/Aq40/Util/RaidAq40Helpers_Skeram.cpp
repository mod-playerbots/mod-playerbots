#include "RaidAq40Helpers_Skeram.h"

#include <unordered_map>
#include <vector>

#include "../RaidAq40BossHelper.h"
#include "Timer.h"

namespace Aq40Helpers
{

namespace
{
std::unordered_map<uint32, uint32> sSkeramPostBlinkHoldUntilByInstance;

GuidVector GetRawObservedSkeramEncounterUnits(Player* bot, PlayerbotAI* botAI, GuidVector const& attackers)
{
    GuidVector observedUnits;
    if (!bot || !botAI || !Aq40BossHelper::IsInAq40(bot))
        return observedUnits;

    GuidVector encounterUnits = Aq40BossHelper::GetEncounterUnits(botAI, attackers);
    for (ObjectGuid const guid : encounterUnits)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsInWorld() || !unit->IsAlive() || unit->GetMapId() != bot->GetMapId() || unit->IsFriendlyTo(bot))
            continue;

        if (Aq40BossHelper::IsUnitNamedAny(botAI, unit, { "the prophet skeram" }))
            observedUnits.push_back(guid);
    }

    return observedUnits;
}
}    // namespace

bool IsSkeramEncounterLive(Player* bot, PlayerbotAI* botAI, GuidVector const& attackers)
{
    if (!bot || !botAI || !Aq40BossHelper::IsInAq40(bot))
        return false;

    GuidVector const activeUnits = Aq40BossHelper::GetActiveCombatUnits(botAI, attackers);
    return Aq40BossHelper::HasAnyNamedUnit(botAI, activeUnits, { "the prophet skeram" });
}

GuidVector GetObservedSkeramEncounterUnits(Player* bot, PlayerbotAI* botAI, GuidVector const& attackers)
{
    if (!IsSkeramEncounterLive(bot, botAI, attackers))
        return {};

    return GetRawObservedSkeramEncounterUnits(bot, botAI, attackers);
}

bool IsSkeramPostBlinkHoldActive(Player* bot, PlayerbotAI* botAI, GuidVector const& attackers)
{
    if (!bot || !botAI || !bot->GetMap())
        return false;

    uint32 const instanceId = bot->GetMap()->GetInstanceId();
    if (!Aq40BossHelper::IsEncounterCombatActive(bot))
    {
        sSkeramPostBlinkHoldUntilByInstance.erase(instanceId);
        return false;
    }

    GuidVector skeramUnits = GetObservedSkeramEncounterUnits(bot, botAI, attackers);
    std::vector<Unit*> skerams = Aq40BossHelper::FindUnitsByAnyName(botAI, skeramUnits, { "the prophet skeram" });
    if (skerams.empty())
    {
        sSkeramPostBlinkHoldUntilByInstance.erase(instanceId);
        return false;
    }

    Player* primaryTank = Aq40BossHelper::GetEncounterPrimaryTank(bot);
    if (!primaryTank || !primaryTank->IsAlive())
    {
        sSkeramPostBlinkHoldUntilByInstance.erase(instanceId);
        return false;
    }

    bool primaryTankOwnsSkeram = false;
    for (Unit* skeram : skerams)
    {
        if (Aq40BossHelper::IsUnitHeldByEncounterTank(bot, skeram, true))
        {
            primaryTankOwnsSkeram = true;
            break;
        }
    }

    uint32 const now = getMSTime();
    auto itr = sSkeramPostBlinkHoldUntilByInstance.find(instanceId);
    if (!primaryTankOwnsSkeram)
    {
        if (itr == sSkeramPostBlinkHoldUntilByInstance.end())
        {
            sSkeramPostBlinkHoldUntilByInstance[instanceId] = now + 2500;
            return true;
        }

        if (now < itr->second)
            return true;

        sSkeramPostBlinkHoldUntilByInstance.erase(itr);
        return false;
    }

    if (itr == sSkeramPostBlinkHoldUntilByInstance.end())
        return false;

    if (now >= itr->second)
    {
        sSkeramPostBlinkHoldUntilByInstance.erase(itr);
        return false;
    }

    return true;
}

bool ResetSkeramEncounterState(Player* bot)
{
    if (!bot || !bot->GetMap())
        return false;

    uint32 const instanceId = bot->GetMap()->GetInstanceId();
    return sSkeramPostBlinkHoldUntilByInstance.erase(instanceId) > 0;
}

bool HasSkeramEncounterState(Player* bot)
{
    if (!bot || !bot->GetMap())
        return false;

    uint32 const instanceId = bot->GetMap()->GetInstanceId();
    return sSkeramPostBlinkHoldUntilByInstance.find(instanceId) != sSkeramPostBlinkHoldUntilByInstance.end();
}

void ClearSkeramPostBlinkHold(Player* bot)
{
    if (!bot || !bot->GetMap())
        return;

    uint32 const instanceId = bot->GetMap()->GetInstanceId();
    auto itr = sSkeramPostBlinkHoldUntilByInstance.find(instanceId);
    if (itr != sSkeramPostBlinkHoldUntilByInstance.end())
        sSkeramPostBlinkHoldUntilByInstance.erase(itr);
}

}    // namespace Aq40Helpers
