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

Position const ALYTHESS_START_POSITION = { 1819.180f, 625.539f, 33.4038f };
std::array<Position, ALYTHESS_TANK_POSITION_COUNT> const alythessTankPositions =
{{
    { 1816.830f, 620.792f, 33.404f },
    { 1824.211f, 625.169f, 33.404f },
    { 1818.701f, 631.196f, 33.404f },
    { 1829.375f, 631.110f, 33.404f },
    { 1830.007f, 620.924f, 33.404f }
}};

std::unordered_map<ObjectGuid, ObjectGuid> alythessTankLastBlazeGuid;

// Adjusted positions are to address the occasional bug (?) where Alythess moves
Position GetAdjustedPosition(Unit* alythess, Position const& basePosition)
{
    if (!alythess)
        return basePosition;

    float const offsetX = alythess->GetPositionX() - ALYTHESS_START_POSITION.GetPositionX();
    float const offsetY = alythess->GetPositionY() - ALYTHESS_START_POSITION.GetPositionY();
    float const offsetZ = alythess->GetPositionZ() - ALYTHESS_START_POSITION.GetPositionZ();

    return {
        basePosition.GetPositionX() + offsetX,
        basePosition.GetPositionY() + offsetY,
        basePosition.GetPositionZ() + offsetZ,
    };
}

} // end anonymous namespace

Position const SACROLASH_TANK_POSITION  =             { 1804.255f, 630.193f, 33.404f };
Position const EREDAR_TWINS_P1_RANGED_POSITION =      { 1808.076f, 603.460f, 51.684f };
Position const EREDAR_TWINS_RANGED_CONFLAG_POSITION = { 1801.133f, 584.456f, 50.696f };
Position const EREDAR_TWINS_MELEE_CONFLAG_POSITION =  { 1812.842f, 611.147f, 33.404f };

std::unordered_map<uint32, EredarTwinsIncomingConflagrationState>
    eredarTwinsIncomingConflagrationStates;

std::unordered_map<uint32, EredarTwinsBlazeTargetState>
    eredarTwinsBlazeTargetStates;

std::unordered_map<uint32, time_t> eredarTwinsDpsHoldTimer;

Position GetAlythessTankPosition(Unit* alythess, uint8 index)
{
    if (index >= alythessTankPositions.size())
        index = 0;

    return GetAdjustedPosition(alythess, alythessTankPositions[index]);
}

Position GetEredarTwinsP2MeleeStackPosition(Unit* alythess)
{
    Position const basePosition = { 1814.327f, 625.645f, 33.404f };
    return GetAdjustedPosition(alythess, basePosition);
}

Position GetEredarTwinsP2RangedStackPosition(Unit* alythess)
{
    Position const basePosition = { 1805.587f, 625.653f, 33.404f };
    return GetAdjustedPosition(alythess, basePosition);
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
        if (!go || go->GetEntry() != static_cast<uint32>(SwpObjects::GO_BLAZE))
            continue;

        if (go->GetExactDist2d(
                position.GetPositionX(), position.GetPositionY()) <= blazeDangerRadius)
        {
            return false;
        }
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
        static_cast<uint32>(SwpObjects::GO_BLAZE), blazeObjectRadius);

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

    constexpr uint32 durationMs = 2000;
    uint32 const now = getMSTime();
    EredarTwinsIncomingConflagrationState& state =
        eredarTwinsIncomingConflagrationStates[target->GetInstanceId()];

    if (state.targetGuid != target->GetGUID())
        state.delayMs = now + EREDAR_TWINS_INCOMING_CONFLAGRATION_DELAY_MS;

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
    EredarTwinsBlazeTargetState& state =
        eredarTwinsBlazeTargetStates[target->GetInstanceId()];
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
