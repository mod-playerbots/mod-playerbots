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
#include "SWPInstanceState.h"
#include "Spell.h"
#include "SpellInfo.h"
#include "ThreatHelpers.h"
#include "ThreatManager.h"
#include <algorithm>
#include <initializer_list>
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

// Positions around Alythess follow her if she moves. Her script never chases, but the core moves
// her to a melee attack point around her victim when it is in her melee range, isn't attacking
// her, and is crowded by another attacker (MoveCircleChecks), and backs her off a victim inside
// her (MoveBackwardsChecks). I don't know if it is Blizzlike for her not to chase.
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

// True if a straight move from the bot to the destination keeps clear of a patch at every
// Alythess tank position.
bool IsClearOfAlythessTankPositions(Player* bot, Unit* alythess, Position const& destination)
{
    float const startX = bot->GetPositionX();
    float const startY = bot->GetPositionY();
    float const deltaX = destination.GetPositionX() - startX;
    float const deltaY = destination.GetPositionY() - startY;
    float const lengthSq = deltaX * deltaX + deltaY * deltaY;

    for (uint8 index = 0; index < ALYTHESS_TANK_POSITIONS.size(); ++index)
    {
        Position const position = GetAlythessTankPosition(alythess, index);
        float const toX = position.GetPositionX() - startX;
        float const toY = position.GetPositionY() - startY;
        float const t = lengthSq > 0.0f ?
            std::clamp((toX * deltaX + toY * deltaY) / lengthSq, 0.0f, 1.0f) : 0.0f;
        float const nearX = startX + t * deltaX;
        float const nearY = startY + t * deltaY;
        if (position.GetExactDist2d(nearX, nearY) < BLAZE_ROUTE_CLEARANCE)
            return false;
    }

    return true;
}

bool IsClearOfBlaze(PlayerbotAI* botAI, Position const& position, float clearance)
{
    for (Position const& blaze : GetCachedBlazePositions(botAI))
    {
        if (blaze.GetExactDist2d(position) <= clearance)
            return false;
    }

    return true;
}

// Hold once the bot has closed to within the ratio of the lowest tank threat on that boss.
bool HasClosedOnTankThreat(Unit* boss, Player* bot, float tankThreat, float threatHoldRatio)
{
    return tankThreat > 0.0f &&
        boss->GetThreatMgr().GetThreat(bot) >= tankThreat * threatHoldRatio;
}

} // end anonymous namespace

Unit* FindAlythess(Player* bot)
{
    return GetSwpBoss(bot, SwpBosses::DATA_EREDAR_TWINS);
}

Unit* FindSacrolash(Player* bot)
{
    return FindThreatTarget(bot, "lady sacrolash");
}

Position GetAlythessTankPosition(Unit* alythess, uint8 index)
{
    if (index >= ALYTHESS_TANK_POSITIONS.size())
        index = 0;

    return GetAdjustedPosition(alythess, ALYTHESS_TANK_POSITIONS[index]);
}

Position GetEredarTwinsPhase2Position(Player* bot, Unit* alythess)
{
    return GetAdjustedPosition(
        alythess, bot->getClass() == CLASS_HUNTER ?
            EREDAR_TWINS_P2_HUNTER_POSITION : EREDAR_TWINS_P2_STACK_POSITION);
}

std::optional<Position> GetEredarTwinsPhase2Fallback(PlayerbotAI* botAI, Unit* alythess)
{
    Player* bot = botAI->GetBot();
    constexpr float clearance = EREDAR_TWINS_P2_BLAZE_CLEARANCE;
    if (IsClearOfBlaze(botAI, GetEredarTwinsPhase2Position(bot, alythess), clearance))
        return std::nullopt;

    for (Position const& basePosition : EREDAR_TWINS_P2_FALLBACK_POSITIONS)
    {
        Position const fallback = GetAdjustedPosition(alythess, basePosition);
        if (IsClearOfBlaze(botAI, fallback, clearance))
            return fallback;
    }

    return std::nullopt;
}

Position GetEredarTwinsStepAsidePosition(Unit* alythess, bool onBalcony)
{
    return onBalcony ? EREDAR_TWINS_P1_RANGED_ASIDE_POSITION :
        GetAdjustedPosition(alythess, EREDAR_TWINS_FLOOR_ASIDE_POSITION);
}

// Every waypoint leads to the stack waypoint, from which the straight line to the bot's position is
// clear. The position 4 waypoint serves the landing and the floor step-aside; from Sacrolash's
// side its line crosses the tank positions, so bots there go round position 0.
Position GetEredarTwinsPhase2RouteTarget(PlayerbotAI* botAI, Unit* alythess)
{
    if (std::optional<Position> const fallback = GetEredarTwinsPhase2Fallback(botAI, alythess))
        return *fallback;

    Player* bot = botAI->GetBot();

    Position const destination = GetEredarTwinsPhase2Position(bot, alythess);
    Position const stackWaypoint = GetAdjustedPosition(alythess, EREDAR_TWINS_P2_STACK_WAYPOINT);
    Position const position0Waypoint =
        GetAdjustedPosition(alythess, EREDAR_TWINS_P2_POSITION_0_WAYPOINT);
    Position const position4Waypoint =
        GetAdjustedPosition(alythess, EREDAR_TWINS_P2_POSITION_4_WAYPOINT);
    constexpr float arrival = EREDAR_TWINS_WAYPOINT_ARRIVAL_DISTANCE;

    // A bot on the leg between two route points keeps heading for the second, since the straight
    // line to it can pass just inside the clearance there.
    auto const isOnLeg = [bot](Position const& from, Position const& to)
    {
        return bot->GetExactDist2d(from) + bot->GetExactDist2d(to) <=
            from.GetExactDist2d(to) + EREDAR_TWINS_ROUTE_LEG_TOLERANCE;
    };

    if (IsClearOfAlythessTankPositions(bot, alythess, destination) ||
        isOnLeg(stackWaypoint, destination) || bot->GetExactDist2d(stackWaypoint) <= arrival)
    {
        return destination;
    }

    if (IsClearOfAlythessTankPositions(bot, alythess, stackWaypoint) ||
        isOnLeg(position0Waypoint, stackWaypoint) || isOnLeg(position4Waypoint, stackWaypoint) ||
        bot->GetExactDist2d(position0Waypoint) <= arrival ||
        bot->GetExactDist2d(position4Waypoint) <= arrival)
    {
        return stackWaypoint;
    }

    if (IsClearOfAlythessTankPositions(bot, alythess, position4Waypoint))
        return position4Waypoint;

    return position0Waypoint;
}

bool UpdateEredarTwinsClassRoleAnnouncement(PlayerbotAI* botAI)
{
    SwpInstanceState& state = SwpState(botAI->GetBot()->GetInstanceId());
    return UpdateClassRoleAnnouncement(
        botAI, Id(SwpNpcs::NPC_GRAND_WARLOCK_ALYTHESS), ALYTHESS_START_POSITION,
        EREDAR_TWINS_CLASS_ROLE_ANNOUNCE_DISTANCE, EREDAR_TWINS_CLASS_ROLE_REARM_DISTANCE, {
            { ALYTHESS_PALADIN_TANK_ROLE, state.alythessPaladinTank },
        });
}

// A prot paladin, chosen by the class role picker rules. If there is no prot paladin, then the main
// tank, whatever class, is assigned to Alythess.
Player* GetAlythessTank(Player* bot)
{
    SwpInstanceState const* state = FindSwpState(bot->GetInstanceId());
    if (!state || !state->alythessPaladinTank.choice)
        return nullptr;

    ClassRoleAssignment const& assignment = state->alythessPaladinTank;
    if (!GetClassRolePlayerGuid(assignment).IsEmpty())
        return GetClassRolePlayer(bot, assignment);

    if (assignment.announcedChoice && !assignment.announcedChoice->guid.IsEmpty())
        return nullptr;

    Player* mainTank = GetGroupMainTank(bot);
    return mainTank && PlayerbotAI::IsTank(mainTank) ? mainTank : nullptr;
}

bool IsAlythessTank(Player* player)
{
    return player && PlayerbotAI::IsTank(player) && GetAlythessTank(player) == player;
}

bool IsHoldingAlythessAggro(Player* player, Unit* alythess)
{
    return player && alythess && alythess->GetVictim() == player && !IsAlythessTank(player);
}

// This ordering is needed only to determine Misdirection assignments.
Player* GetSacrolashTank(Player* bot, uint8 index)
{
    Player* alythessTank = GetAlythessTank(bot);

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

    for (uint8 assistIndex = 0; Player* assistTank = GetGroupAssistTank(bot, assistIndex);
         ++assistIndex)
    {
        if (assistTank == alythessTank)
            continue;

        if (found == index)
            return assistTank;

        ++found;
    }

    return nullptr;
}

// Sacrolash is held by every tank except the one assigned to Alythess.
bool IsAnySacrolashTank(Player* bot)
{
    return PlayerbotAI::IsTank(bot) && GetAlythessTank(bot) != bot;
}

// One tank holds Alythess, so her ceiling is read directly rather than scanned for.
bool ShouldHoldAlythessThreat(Player* bot, Unit* alythess)
{
    if (PlayerbotAI::IsHeal(bot))
        return false;

    Player* alythessTank = GetAlythessTank(bot);
    if (!alythessTank || alythessTank == bot || !alythessTank->IsAlive())
        return false;

    auto& threatMgr = alythess->GetThreatMgr();
    if (!threatMgr.IsThreatenedBy(alythessTank))
        return false;

    return HasClosedOnTankThreat(
        alythess, bot, threatMgr.GetThreat(alythessTank), ALYTHESS_THREAT_HOLD_RATIO);
}

bool ShouldHoldSacrolashThreat(Player* bot, Unit* sacrolash)
{
    if (PlayerbotAI::IsHeal(bot) || IsAnySacrolashTank(bot))
        return false;

    Player* tank = GetSacrolashSecondTank(bot, sacrolash);
    return tank && HasClosedOnTankThreat(
        sacrolash, bot, sacrolash->GetThreatMgr().GetThreat(tank), SACROLASH_THREAT_HOLD_RATIO);
}

// The Sacrolash tank second on her threat, whom Confounding Blow sends her to, or the only one.
Player* GetSacrolashSecondTank(Player* bot, Unit* sacrolash)
{
    Player* alythessTank = GetAlythessTank(bot);
    Player* highestTank = nullptr;
    Player* secondTank = nullptr;
    float highestThreat = 0.0f;
    float secondThreat = 0.0f;

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
        if (!highestTank || threat > highestThreat)
        {
            secondTank = highestTank;
            secondThreat = highestThreat;
            highestTank = threatPlayer;
            highestThreat = threat;
        }
        else if (!secondTank || threat > secondThreat)
        {
            secondTank = threatPlayer;
            secondThreat = threat;
        }
    }

    return secondTank ? secondTank : highestTank;
}

bool IsAlythessPullMisdirectHunter(Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive() && member->GetMapId() == SWP_MAP_ID &&
            member->getClass() == CLASS_HUNTER && GET_PLAYERBOT_AI(member))
        {
            return member == bot;
        }
    }

    return false;
}

bool IsEredarTwinsStartHoldWindowActive(Player* bot)
{
    if (!FindSacrolash(bot))
        return false;

    EredarTwinsEncounterState const* state = FindEredarTwinsState(bot->GetInstanceId());
    if (!state || !state->dpsHoldStartMs)
        return true;

    return getMSTimeDiff(*state->dpsHoldStartMs, getMSTime()) < EREDAR_TWINS_DPS_HOLD_MS;
}

bool IsEredarTwinsStartHoldActive(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    return !PlayerbotAI::IsTank(bot) && IsEredarTwinsStartHoldWindowActive(bot);
}

EredarTwinsThreatHolds FindEredarTwinsThreatHolds(Player* bot)
{
    EredarTwinsThreatHolds holds;
    if (PlayerbotAI::IsHeal(bot))
        return holds;

    // Fine to exclude all tanks for Sacrolash because the Alythess tank never attacks her.
    Unit* sacrolash = FindSacrolash(bot);
    if (sacrolash && !PlayerbotAI::IsTank(bot) && ShouldHoldSacrolashThreat(bot, sacrolash))
        holds.sacrolash = sacrolash->GetGUID();

    Unit* alythess = FindAlythess(bot);
    if (alythess && ShouldHoldAlythessThreat(bot, alythess))
        holds.alythess = alythess->GetGUID();

    return holds;
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
    return IsClearOfBlaze(botAI, position, BLAZE_CLEARANCE);
}

// Alythess casts Conflagration while Sacrolash lives; Sacrolash casts it if Alythess dies first.
// The cast is read live; once it ends, the twin's next cast replaces it, so the missile's flight
// comes from the record the spell listener writes.
ObjectGuid GetEredarTwinsConflagrationTargetGuid(Unit* alythess, Unit* sacrolash)
{
    for (Unit* twin : { alythess, sacrolash })
    {
        Spell* spell = twin ? twin->GetCurrentSpell(CURRENT_GENERIC_SPELL) : nullptr;
        if (spell && spell->m_spellInfo->Id == Id(SwpSpells::SPELL_CONFLAGRATION) &&
            spell->GetCastTime() - spell->GetCastTimeRemaining() >= CONFLAGRATION_DELAY_MS)
        {
            return spell->m_targets.GetUnitTargetGUID();
        }
    }

    Unit* twin = alythess ? alythess : sacrolash;
    EredarTwinsEncounterState const* state =
        twin ? FindEredarTwinsState(twin->GetInstanceId()) : nullptr;
    if (!state || !state->conflagrationImpact ||
        state->conflagrationImpact->expireMs <= getMSTime())
    {
        return ObjectGuid::Empty;
    }

    return state->conflagrationImpact->targetGuid;
}

bool IsEredarTwinsConflagrationTarget(Player* bot, Unit* alythess, Unit* sacrolash)
{
    ObjectGuid const targetGuid = GetEredarTwinsConflagrationTargetGuid(alythess, sacrolash);
    return !targetGuid.IsEmpty() && targetGuid == bot->GetGUID();
}

void RecordEredarTwinsConflagrationImpact(Player* target)
{
    EredarTwinsState(target->GetInstanceId()).conflagrationImpact = EredarTwinsConflagrationImpact{
        target->GetGUID(), getMSTime() + CONFLAGRATION_IMPACT_WINDOW_MS };
}

// Shared encounter state

EredarTwinsEncounterState& EredarTwinsState(uint32 instanceId)
{
    std::optional<EredarTwinsEncounterState>& state = SwpState(instanceId).eredarTwins;
    if (!state)
        state.emplace();

    return *state;
}

EredarTwinsEncounterState* FindEredarTwinsState(uint32 instanceId)
{
    SwpInstanceState* state = FindSwpState(instanceId);
    return state && state->eredarTwins ? &*state->eredarTwins : nullptr;
}

}
