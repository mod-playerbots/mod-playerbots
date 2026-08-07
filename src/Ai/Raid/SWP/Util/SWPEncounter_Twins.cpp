/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "SWPEncounter_Twins.h"
#include "AiObjectContext.h"
#include "CellImpl.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "NearestGameObjects.h"
#include "Playerbots.h"
#include "Spell.h"
#include "ThreatManager.h"
#include <list>

namespace SwpHelpers
{

// Note: Alythess and Sacrolash each have a CombatReach of 2.5f

namespace
{

std::unordered_map<ObjectGuid, ObjectGuid> alythessTankLastBlazeGuid;

// Adjusted positions are to address the occasional bug (?) where Alythess moves
Position GetAdjustedPosition(Unit* alythess, Position const& basePosition)
{
    if (!alythess)
        return basePosition;

    Position const& alythessPosition = alythess->GetPosition();
    Position const& startPosition = ALYTHESS_START_POSITION;

    float const offsetX = alythessPosition.GetPositionX() - startPosition.GetPositionX();
    float const offsetY = alythessPosition.GetPositionY() - startPosition.GetPositionY();
    float const offsetZ = alythessPosition.GetPositionZ() - startPosition.GetPositionZ();

    float const baseX = basePosition.GetPositionX();
    float const baseY = basePosition.GetPositionY();
    float const baseZ = basePosition.GetPositionZ();

    return { baseX + offsetX, baseY + offsetY, baseZ + offsetZ };
}

} // end anonymous namespace

std::unordered_map<uint32, EredarTwinsIncomingConflagrationState>
	eredarTwinsIncomingConflagrationStates;

std::unordered_map<uint32, EredarTwinsBlazeTargetState> eredarTwinsBlazeTargetStates;

std::unordered_map<uint32, time_t> eredarTwinsDpsHoldTimer;

Position GetAlythessTankPosition(Unit* alythess, uint8 index)
{
    if (index >= ALYTHESS_TANK_POSITIONS.size())
        index = 0;

    return GetAdjustedPosition(alythess, ALYTHESS_TANK_POSITIONS[index]);
}

Position GetEredarTwinsP2MeleePosition(Unit* alythess)
{
    return GetAdjustedPosition(alythess, EREDAR_TWINS_P2_MELEE_POSITION);
}

Position GetEredarTwinsP2RangedPosition(Unit* alythess)
{
    return GetAdjustedPosition(alythess, EREDAR_TWINS_P2_RANGED_POSITION);
}

bool IsAnySacrolashTank(Player* bot)
{
    return PlayerbotAI::IsMainTank(bot) || PlayerbotAI::IsAssistTankOfIndex(bot, 1, false);
}

bool IsAlythessTank(Player* bot)
{
    return PlayerbotAI::IsAssistTankOfIndex(bot, 0, false);
}

bool ShouldHoldTwinThreat(
    Player* bot, Unit* boss, float threatHoldRatio, bool (*isTwinTank)(Player*))
{
    if (!boss || isTwinTank(bot))
        return false;

    float twinTankThreat = 0.0f;
    float botThreat = 0.0f;
    bool foundTwinTankThreat = false;
    bool foundBotThreat = false;

    auto const threatList = boss->GetThreatMgr().GetSortedThreatList();
    for (auto itr = threatList.begin(); itr != threatList.end(); ++itr)
    {
        ThreatReference const* threatRef = *itr;
        if (!threatRef || !threatRef->IsAvailable())
            continue;

        Unit* victim = threatRef->GetVictim();
        if (!victim)
            continue;

        Player* threatPlayer = victim->ToPlayer();
        if (!threatPlayer || !threatPlayer->IsAlive())
            continue;

        float const threat = threatRef->GetThreat();

        if (isTwinTank(threatPlayer) &&
            (!foundTwinTankThreat || threat < twinTankThreat))
        {
            twinTankThreat = threat;
            foundTwinTankThreat = true;
        }

        if (threatPlayer == bot)
        {
            botThreat = threat;
            foundBotThreat = true;
        }
    }

    if (!foundTwinTankThreat || !foundBotThreat || twinTankThreat <= 0.0f)
        return false;

    return botThreat >= twinTankThreat * threatHoldRatio;
}

bool IsAlythessTankPositionSafe(Player* bot, Position const& position)
{
    constexpr float blazeDangerRadius = 4.5f;
    constexpr float blazeSearchRadius = 30.0f;

    std::list<GameObject*> targets;
    AnyGameObjectInObjectRangeCheck u_check(bot, blazeSearchRadius);
    Acore::GameObjectListSearcher<AnyGameObjectInObjectRangeCheck> searcher(bot, targets, u_check);
    Cell::VisitObjects(bot, searcher, blazeSearchRadius);

    for (GameObject* go : targets)
    {
        if (!go || go->GetEntry() != Id(SwpObjects::GO_BLAZE))
            continue;

        if (go->GetExactDist2d(position) <= blazeDangerRadius)
            return false;
    }

    return true;
}

bool ShouldAdvanceAlythessTankPosition(Unit* alythess, Player* bot)
{
    if (!alythess)
        return false;

    ObjectGuid const botGuid = bot->GetGUID();
    constexpr float blazeObjectRadius = 5.0f;

    GameObject* blazeObject = bot->FindNearestGameObject(
        Id(SwpObjects::GO_BLAZE), blazeObjectRadius);

    if (!blazeObject)
    {
        alythessTankLastBlazeGuid.erase(botGuid);
        return false;
    }

    ObjectGuid const blazeGuid = blazeObject->GetGUID();
    auto const lastBlaze = alythessTankLastBlazeGuid.find(botGuid);
    if (lastBlaze != alythessTankLastBlazeGuid.end() && lastBlaze->second == blazeGuid)
        return false;

    alythessTankLastBlazeGuid[botGuid] = blazeGuid;
    return true;
}

void RecordIncomingEredarTwinsConflagrationTarget(Player* target)
{
    if (!target)
        return;

    uint32 const now = getMSTime();
    EredarTwinsIncomingConflagrationState& state =
        eredarTwinsIncomingConflagrationStates[target->GetInstanceId()];

    constexpr uint32 conflagrationDelayMs = 300;
    if (state.targetGuid != target->GetGUID())
        state.delayMs = now + conflagrationDelayMs;

    constexpr uint32 durationMs = 2000;
    state.targetGuid = target->GetGUID();
    state.expireMs = now + durationMs;
}

Player* GetEredarTwinsConflagrationTarget(Player* bot)
{
    auto const incomingItr = eredarTwinsIncomingConflagrationStates.find(bot->GetInstanceId());

    if (incomingItr == eredarTwinsIncomingConflagrationStates.end())
        return nullptr;

    EredarTwinsIncomingConflagrationState const& state = incomingItr->second;
    uint32 const now = getMSTime();

    if (state.expireMs <= now)
    {
        eredarTwinsIncomingConflagrationStates.erase(incomingItr);
        return nullptr;
    }

    if (state.delayMs > now)
        return nullptr;

    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->GetGUID() == state.targetGuid)
            return member;
    }

    return nullptr;
}

void RecordEredarTwinsBlazeTarget(Player* target)
{
    if (!target)
        return;

    constexpr uint32 durationMs = 2000;
    uint32 const now = getMSTime();
    EredarTwinsBlazeTargetState& state = eredarTwinsBlazeTargetStates[target->GetInstanceId()];
    state.targetGuid = target->GetGUID();
    state.expireMs = now + durationMs;
}

Player* GetEredarTwinsBlazeTarget(Player* bot)
{
    auto const itr = eredarTwinsBlazeTargetStates.find(bot->GetInstanceId());
    if (itr == eredarTwinsBlazeTargetStates.end())
        return nullptr;

    EredarTwinsBlazeTargetState const& state = itr->second;
    if (state.expireMs <= getMSTime())
    {
        eredarTwinsBlazeTargetStates.erase(itr);
        return nullptr;
    }

    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->GetGUID() == state.targetGuid)
            return member;
    }

    return nullptr;
}

}
