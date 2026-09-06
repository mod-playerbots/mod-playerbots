/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "SWPEncounter_Twins.h"
#include "AiObjectContext.h"
#include "CellImpl.h"
#include "EncounterHelpers.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "NearestGameObjects.h"
#include "Playerbots.h"
#include "ThreatManager.h"
#include <list>

using namespace EncounterHelpers;

namespace SwpHelpers
{

// Note: Alythess and Sacrolash each have a CombatReach of 2.5f

namespace
{

std::vector<Position> const& GetCachedBlazePositions(PlayerbotAI* botAI)
{
    return botAI->GetAiObjectContext()
        ->GetValue<std::vector<Position>>("eredar twins blaze")->RefGet();
}

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

// If the main tank is not a Paladin tank, then this picks the present Paladin tank with the highest
// max health.
Player* FindBestPaladinTank(Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    Player* best = nullptr;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || member->GetMapId() != SWP_MAP_ID ||
            member->getClass() != CLASS_PALADIN || !PlayerbotAI::IsTank(member))
        {
            continue;
        }

        if (!best || member->GetMaxHealth() > best->GetMaxHealth() ||
            (member->GetMaxHealth() == best->GetMaxHealth() &&
             member->GetGUID() < best->GetGUID()))
        {
            best = member;
        }
    }

    return best;
}

EredarTwinsTankAssignment const emptyTankAssignment;

EredarTwinsTankAssignment const& GetTankAssignment(Player* bot)
{
    auto const itr = eredarTwinsTankAssignments.find(bot->GetInstanceId());
    return itr != eredarTwinsTankAssignments.end() ? itr->second : emptyTankAssignment;
}

// Hold once the bot has closed to within the ratio of the lowest tank threat on that boss.
bool HasClosedOnTankThreat(Unit* boss, Player* bot, float tankThreat, float threatHoldRatio)
{
    return tankThreat > 0.0f &&
        boss->GetThreatMgr().GetThreat(bot) >= tankThreat * threatHoldRatio;
}

bool CanHoldTwinThreat(Player* bot, Unit* boss)
{
    return boss && bot->IsAlive() && !PlayerbotAI::IsHeal(bot) &&
        boss->GetThreatMgr().IsThreatenedBy(bot);
}

} // end anonymous namespace

std::unordered_map<uint32, EredarTwinsIncomingConflagrationState>
	eredarTwinsIncomingConflagrationStates;

std::unordered_map<uint32, EredarTwinsBlazeTargetState> eredarTwinsBlazeTargetStates;

std::unordered_map<uint32, uint32> eredarTwinsDpsHoldStartMs;

std::unordered_map<uint32, EredarTwinsTankAssignment> eredarTwinsTankAssignments;

std::unordered_map<ObjectGuid, ObjectGuid> alythessTankLastBlazeGuid;

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

void ResolveEredarTwinsTankAssignment(Player* bot)
{
    if (GetTankAssignment(bot).source != AlythessTankSource::Unresolved)
        return;

    Player* mainTank = GetGroupMainTank(bot);

    Player* alythessTank = nullptr;
    AlythessTankSource source = AlythessTankSource::Unresolved;

    if (mainTank && mainTank->getClass() == CLASS_PALADIN && PlayerbotAI::IsTank(mainTank))
    {
        alythessTank = mainTank;
        source = AlythessTankSource::MainTankPaladin;
    }
    else if (Player* paladinTank = FindBestPaladinTank(bot))
    {
        alythessTank = paladinTank;
        source = AlythessTankSource::PaladinTank;
    }
    else if (mainTank && PlayerbotAI::IsTank(mainTank))
    {
        alythessTank = mainTank;
        source = AlythessTankSource::MainTankFallback;
    }

    if (!alythessTank)
        return;

    EredarTwinsTankAssignment& assignment = eredarTwinsTankAssignments[bot->GetInstanceId()];
    assignment.alythessTankGuid = alythessTank->GetGUID();
    assignment.source = source;
}

Player* GetAlythessTank(Player* bot)
{
    ObjectGuid const guid = GetTankAssignment(bot).alythessTankGuid;
    return guid.IsEmpty() ? nullptr : ObjectAccessor::FindPlayer(guid);
}

bool IsAlythessTank(Player* bot)
{
    return PlayerbotAI::IsTank(bot) && GetAlythessTank(bot) == bot;
}

AlythessTankSource GetAlythessTankSource(Player* bot)
{
    return GetTankAssignment(bot).source;
}

// This ordering is needed only to determine Misdirection assignments.
Player* GetSacrolashTank(Player* bot, uint8 index)
{
    Player* const alythessTank = GetAlythessTank(bot);

    Player* mainTank = GetGroupMainTank(bot);
    if (mainTank && (!PlayerbotAI::IsTank(mainTank) || mainTank == alythessTank))
        mainTank = nullptr;

    uint8 found = 0;
    if (mainTank)
    {
        if (index == 0)
            return mainTank;

        found = 1;
    }

    for (uint8 assistIndex = 0;; ++assistIndex)
    {
        Player* assistTank = GetGroupAssistTank(bot, assistIndex);
        if (!assistTank)
            return nullptr;

        if (assistTank == alythessTank)
            continue;

        if (found == index)
            return assistTank;

        ++found;
    }
}

// Sacrolash is held by every tank except the one assigned to Alythess.
bool IsAnySacrolashTank(Player* bot)
{
    return PlayerbotAI::IsTank(bot) && GetAlythessTank(bot) != bot;
}

// One tank holds Alythess, so her ceiling is read directly rather than scanned for.
bool ShouldHoldAlythessThreat(Player* bot, Unit* alythess)
{
    Player* const alythessTank = GetAlythessTank(bot);
    if (!alythessTank || alythessTank == bot || !alythessTank->IsAlive())
        return false;

    if (!CanHoldTwinThreat(bot, alythess))
        return false;

    auto& threatMgr = alythess->GetThreatMgr();
    if (!threatMgr.IsThreatenedBy(alythessTank))
        return false;

    return HasClosedOnTankThreat(
        alythess, bot, threatMgr.GetThreat(alythessTank), ALYTHESS_THREAT_HOLD_RATIO);
}

bool ShouldHoldSacrolashThreat(Player* bot, Unit* sacrolash)
{
    if (IsAnySacrolashTank(bot) || !CanHoldTwinThreat(bot, sacrolash))
        return false;

    Player* const alythessTank = GetAlythessTank(bot);

    float highestTankThreat = 0.0f;
    float secondTankThreat = 0.0f;
    uint8 tankCount = 0;

    auto const threatList = sacrolash->GetThreatMgr().GetUnsortedThreatList();
    for (auto itr = threatList.begin(); itr != threatList.end(); ++itr)
    {
        ThreatReference const* threatRef = *itr;
        if (!threatRef || !threatRef->IsAvailable())
            continue;

        Unit* victim = threatRef->GetVictim();
        Player* threatPlayer = victim ? victim->ToPlayer() : nullptr;

        if (!threatPlayer || !threatPlayer->IsAlive() || threatPlayer == alythessTank ||
            !PlayerbotAI::IsTank(threatPlayer))
        {
            continue;
        }

        float const threat = threatRef->GetThreat();
        ++tankCount;

        if (threat > highestTankThreat)
        {
            secondTankThreat = highestTankThreat;
            highestTankThreat = threat;
        }
        else if (threat > secondTankThreat)
        {
            secondTankThreat = threat;
        }
    }

    if (!tankCount)
        return false;

    float const tankThreat = tankCount > 1 ? secondTankThreat : highestTankThreat;

    return HasClosedOnTankThreat(sacrolash, bot, tankThreat, SACROLASH_THREAT_HOLD_RATIO);
}

std::vector<Position> FindEredarTwinsBlazePositions(Player* bot)
{
    std::list<GameObject*> nearbyObjects;
    AnyGameObjectInObjectRangeCheck check(bot, BLAZE_SEARCH_RADIUS);
    Acore::GameObjectListSearcher<AnyGameObjectInObjectRangeCheck> searcher(
        bot, nearbyObjects, check);
    Cell::VisitObjects(bot, searcher, BLAZE_SEARCH_RADIUS);

    std::vector<Position> positions;
    for (GameObject* nearbyObject : nearbyObjects)
    {
        if (nearbyObject && nearbyObject->GetEntry() == Id(SwpObjects::GO_BLAZE))
            positions.push_back(nearbyObject->GetPosition());
    }

    return positions;
}

bool IsAlythessTankPositionSafe(PlayerbotAI* botAI, Position const& position)
{
    for (Position const& blaze : GetCachedBlazePositions(botAI))
    {
        if (blaze.GetExactDist2d(position) <= BLAZE_DANGER_RADIUS)
            return false;
    }

    return true;
}

bool ShouldAdvanceAlythessTankPosition(Unit* alythess, Player* bot)
{
    if (!alythess)
        return false;

    ObjectGuid const botGuid = bot->GetGUID();

    GameObject* blazeObject =
        bot->FindNearestGameObject(Id(SwpObjects::GO_BLAZE), BLAZE_DANGER_RADIUS);
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

void RecordEredarTwinsDpsHoldStart(Player* bot)
{
    eredarTwinsDpsHoldStartMs.try_emplace(bot->GetInstanceId(), getMSTime());
}

void RecordIncomingEredarTwinsConflagrationTarget(Player* target)
{
    if (!target)
        return;

    uint32 const now = getMSTime();
    EredarTwinsIncomingConflagrationState& state =
        eredarTwinsIncomingConflagrationStates[target->GetInstanceId()];

    if (state.targetGuid != target->GetGUID())
        state.delayMs = now + CONFLAGRATION_DELAY_MS;

    state.targetGuid = target->GetGUID();
    state.expireMs = now + CONFLAGRATION_WINDOW_MS;
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

    EredarTwinsBlazeTargetState& state = eredarTwinsBlazeTargetStates[target->GetInstanceId()];
    state.targetGuid = target->GetGUID();
    state.startMs = getMSTime();
}

Player* GetEredarTwinsBlazeTarget(Player* bot)
{
    auto const itr = eredarTwinsBlazeTargetStates.find(bot->GetInstanceId());
    if (itr == eredarTwinsBlazeTargetStates.end())
        return nullptr;

    EredarTwinsBlazeTargetState const& state = itr->second;
    if (GetMSTimeDiffToNow(state.startMs) >= BLAZE_TARGET_WINDOW_MS)
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
