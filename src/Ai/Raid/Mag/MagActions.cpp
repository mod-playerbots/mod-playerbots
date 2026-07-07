/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "MagActions.h"
#include "MagHelpers.h"
#include "Creature.h"
#include "ObjectAccessor.h"
#include "ObjectGuid.h"
#include "Playerbots.h"
#include "RaidBossHelpers.h"

using namespace MagtheridonHelpers;

bool MagtheridonMainTankAttackFirstThreeChannelersAction::Execute(Event /*event*/)
{
    Creature* channelerTarget = nullptr;
    if (Creature* channelerSquare = GetChanneler(bot, SOUTH_CHANNELER))
    {
        channelerTarget = channelerSquare;
        MarkTargetWithSquare(bot, channelerSquare);
        SetRtiTarget(botAI, "square", channelerSquare);
    }
    else if (Creature* channelerStar = GetChanneler(bot, WEST_CHANNELER))
    {
        channelerTarget = channelerStar;
        MarkTargetWithStar(bot, channelerStar);
        SetRtiTarget(botAI, "star", channelerStar);
    }
    else if (Creature* channelerCircle = GetChanneler(bot, EAST_CHANNELER))
    {
        channelerTarget = channelerCircle;
        MarkTargetWithCircle(bot, channelerCircle);
        SetRtiTarget(botAI, "circle", channelerCircle);
    }

    if (channelerTarget && AI_VALUE(Unit*, "current target") != channelerTarget)
        return Attack(channelerTarget);

    // After first three channelers are dead, wait for Magtheridon to activate
    if (!channelerTarget)
    {
        const Position& position = WAITING_FOR_MAGTHERIDON_POSITION;
        if (bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY()) > 2.0f)
        {
            return MoveTo(MAGTHERIDON_MAP_ID, position.GetPositionX(), position.GetPositionY(),
                          position.GetPositionZ(), false, false, false, false,
                          MovementPriority::MOVEMENT_FORCED, true, false);
        }

        bot->SetFacingTo(position.GetOrientation());
        return true;
    }

    return false;
}

bool MagtheridonFirstAssistTankAttackNWChannelerAction::Execute(Event /*event*/)
{
    Creature* channelerDiamond = GetChanneler(bot, NORTHWEST_CHANNELER);
    if (!channelerDiamond)
        return false;

    MarkTargetWithDiamond(bot, channelerDiamond);
    SetRtiTarget(botAI, "diamond", channelerDiamond);

    if (AI_VALUE(Unit*, "current target") != channelerDiamond)
        return Attack(channelerDiamond);

    if (channelerDiamond->GetVictim() == bot)
    {
        const Position& position = NW_CHANNELER_TANK_POSITION;
        const float distanceToPosition =
            bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());

        if (distanceToPosition > 3.0f)
        {
            const float dX = position.GetPositionX() - bot->GetPositionX();
            const float dY = position.GetPositionY() - bot->GetPositionY();
            const float moveDist = std::min(distanceToPosition, 3.5f);
            const float moveX = bot->GetPositionX() + (dX / distanceToPosition) * moveDist;
            const float moveY = bot->GetPositionY() + (dY / distanceToPosition) * moveDist;

            return MoveTo(MAGTHERIDON_MAP_ID, moveX, moveY, position.GetPositionZ(), false, false,
                          false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
        }
    }

    return false;
}

bool MagtheridonSecondAssistTankAttackNEChannelerAction::Execute(Event /*event*/)
{
    Creature* channelerTriangle = GetChanneler(bot, NORTHEAST_CHANNELER);
    if (!channelerTriangle)
        return false;

    MarkTargetWithTriangle(bot, channelerTriangle);
    SetRtiTarget(botAI, "triangle", channelerTriangle);

    if (AI_VALUE(Unit*, "current target") != channelerTriangle)
        return Attack(channelerTriangle);

    if (channelerTriangle->GetVictim() == bot)
    {
        const Position& position = NE_CHANNELER_TANK_POSITION;
        const float distanceToPosition =
            bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());

        if (distanceToPosition > 3.0f)
        {
            const float dX = position.GetPositionX() - bot->GetPositionX();
            const float dY = position.GetPositionY() - bot->GetPositionY();
            const float moveDist = std::min(distanceToPosition, 3.5f);
            const float moveX = bot->GetPositionX() + (dX / distanceToPosition) * moveDist;
            const float moveY = bot->GetPositionY() + (dY / distanceToPosition) * moveDist;

            return MoveTo(MAGTHERIDON_MAP_ID, moveX, moveY, position.GetPositionZ(), false, false,
                          false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
        }
    }

    return false;
}

// Misdirect West & East Channelers to Main Tank
bool MagtheridonMisdirectHellfireChannelersToMainTankAction::Execute(Event /*event*/)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    std::vector<Player*> hunters;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive() && member->getClass() == CLASS_HUNTER &&
            GET_PLAYERBOT_AI(member))
        {
            hunters.push_back(member);
        }
    }

    int hunterIndex = -1;
    for (size_t i = 0; i < hunters.size(); ++i)
    {
        if (hunters[i] == bot)
        {
            hunterIndex = static_cast<int>(i);
            break;
        }
    }

    Player* mainTank = GetGroupMainTank(botAI, bot);

    Creature* targetChanneler = nullptr;
    if (hunterIndex == 0)
        targetChanneler = GetChanneler(bot, WEST_CHANNELER);
    else if (hunterIndex == 1)
        targetChanneler = GetChanneler(bot, EAST_CHANNELER);

    if (mainTank && targetChanneler)
    {
        if (botAI->CanCastSpell("misdirection", mainTank))
            return botAI->CastSpell("misdirection", mainTank);

        if (!bot->HasAura(static_cast<uint32>(MagtheridonSpells::SPELL_MISDIRECTION)))
            return false;

        if (botAI->CanCastSpell("steady shot", targetChanneler))
            return botAI->CastSpell("steady shot", targetChanneler);
    }

    return false;
}

bool MagtheridonAssignDpsPriorityAction::Execute(Event /*event*/)
{
    // Listed in order of priority
    if (Creature* channelerSquare   = GetChanneler(bot, SOUTH_CHANNELER))
    {
        MarkTargetWithSquare(bot, channelerSquare);
        SetRtiTarget(botAI, "square", channelerSquare);

        if (AI_VALUE(Unit*, "current target") != channelerSquare)
            return Attack(channelerSquare);
    }
    else if (Creature* channelerStar = GetChanneler(bot, WEST_CHANNELER))
    {
        MarkTargetWithStar(bot, channelerStar);
        SetRtiTarget(botAI, "star", channelerStar);

        if (AI_VALUE(Unit*, "current target") != channelerStar)
            return Attack(channelerStar);
    }
    else if (Creature* channelerCircle = GetChanneler(bot, EAST_CHANNELER))
    {
        MarkTargetWithCircle(bot, channelerCircle);
        SetRtiTarget(botAI, "circle", channelerCircle);

        if (AI_VALUE(Unit*, "current target") != channelerCircle)
            return Attack(channelerCircle);
    }
    else if (Creature* channelerDiamond = GetChanneler(bot, NORTHWEST_CHANNELER))
    {
        MarkTargetWithDiamond(bot, channelerDiamond);
        SetRtiTarget(botAI, "diamond", channelerDiamond);

        if (AI_VALUE(Unit*, "current target") != channelerDiamond)
            return Attack(channelerDiamond);
    }
    else if (Creature* channelerTriangle = GetChanneler(bot, NORTHEAST_CHANNELER))
    {
        MarkTargetWithTriangle(bot, channelerTriangle);
        SetRtiTarget(botAI, "triangle", channelerTriangle);

        if (AI_VALUE(Unit*, "current target") != channelerTriangle)
            return Attack(channelerTriangle);
    }
    else if (Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon"))
    {
        SetRtiTarget(botAI, "cross", magtheridon);

        if (AI_VALUE(Unit*, "current target") != magtheridon)
            return Attack(magtheridon);
    }

    return false;
}

// Assign Burning Abyssals to Warlocks to Banish
// Burning Abyssals in excess of Warlocks in party will be Feared
bool MagtheridonWarlockCcBurningAbyssalAction::Execute(Event /*event*/)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    std::vector<Unit*> abyssals;
    std::list<Creature*> creatureList;
    constexpr float searchRadius = 100.0f;

    bot->GetCreatureListWithEntryInGrid(
        creatureList, static_cast<uint32>(MagtheridonNpcs::NPC_BURNING_ABYSSAL), searchRadius);

    for (Creature* creature : creatureList)
    {
        if (creature && creature->IsAlive())
            abyssals.push_back(creature);
    }

    std::vector<Player*> warlocks;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive() && member->getClass() == CLASS_WARLOCK &&
            GET_PLAYERBOT_AI(member))
        {
            warlocks.push_back(member);
        }
    }

    int warlockIndex = -1;
    for (size_t i = 0; i < warlocks.size(); ++i)
    {
        if (warlocks[i] == bot)
        {
            warlockIndex = static_cast<int>(i);
            break;
        }
    }

    if (warlockIndex >= 0 && warlockIndex < abyssals.size())
    {
        Unit* assignedAbyssal = abyssals[warlockIndex];
        if (!botAI->HasAura("banish", assignedAbyssal) &&
            botAI->CanCastSpell("banish", assignedAbyssal))
        {
            return botAI->CastSpell("banish", assignedAbyssal);
        }
    }

    for (size_t i = warlocks.size(); i < abyssals.size(); ++i)
    {
        Unit* excessAbyssal = abyssals[i];
        if (!botAI->HasAura("banish", excessAbyssal) &&
            !botAI->HasAura("fear", excessAbyssal) &&
            botAI->CanCastSpell("fear", excessAbyssal))
        {
            return botAI->CastSpell("fear", excessAbyssal);
        }
    }

    return false;
}

// Main tank will back up to the Eastern point of the room
bool MagtheridonMainTankPositionBossAction::Execute(Event /*event*/)
{
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    if (!magtheridon)
        return false;

    MarkTargetWithCross(bot, magtheridon);
    SetRtiTarget(botAI, "cross", magtheridon);

    if (AI_VALUE(Unit*, "current target") != magtheridon)
        return Attack(magtheridon);

    if (magtheridon->GetVictim() == bot && bot->GetHealthPct() > 50.0f)
    {
        const Position& position = MAGTHERIDON_TANK_POSITION;
        const float distanceToPosition =
            bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());

        if (distanceToPosition > 3.0f)
        {
            const float dX = position.GetPositionX() - bot->GetPositionX();
            const float dY = position.GetPositionY() - bot->GetPositionY();
            const float moveDist = std::min(2.25f, distanceToPosition);
            const float moveX = bot->GetPositionX() + (dX / distanceToPosition) * moveDist;
            const float moveY = bot->GetPositionY() + (dY / distanceToPosition) * moveDist;

            return MoveTo(MAGTHERIDON_MAP_ID, moveX, moveY, position.GetPositionZ(), false, false,
                          false, false, MovementPriority::MOVEMENT_COMBAT, true, true);
        }
    }

    return false;
}

// Ranged stay away from Magtheridon and other players
// Magtheridon's CombatReach is 12 yards and BoundingRadius is 4 yards
bool MagtheridonSpreadRangedAction::Execute(Event /*event*/)
{
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    if (!magtheridon)
        return false;

    if (IsCubeClicker(bot))
    {
        auto timerIt = blastNovaTimer.find(magtheridon->GetMap()->GetInstanceId());
        if (timerIt != blastNovaTimer.end() &&
            time(nullptr) - timerIt->second >= BLAST_NOVA_INTERIM_SECONDS)
        {
            return false;
        }
    }

    constexpr float safeDistFromBoss = 10.0f;
    const float currentDistance = bot->GetDistance2d(magtheridon);
    if (currentDistance < safeDistFromBoss)
        return MoveAway(magtheridon, safeDistFromBoss - currentDistance);

    constexpr float safeDistFromPlayer = 6.0f;
    constexpr uint32 minInterval = 1000;
    if (Unit* nearestPlayer = GetNearestPlayerInRadius(bot, safeDistFromPlayer))
        return FleePosition(nearestPlayer->GetPosition(), safeDistFromPlayer, minInterval);

    return false;
}

// For bots that are assigned to click cubes
// Magtheridon casts Blast Nova every 54.35 to 55.40s, with a 2s cast time
bool MagtheridonUseManticronCubeAction::Execute(Event /*event*/)
{
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    if (!magtheridon)
        return false;

    CubeInfo const* cubeInfo = GetAssignedCube();
    if (!cubeInfo)
        return false;

    GameObject* cube = botAI->GetGameObject(cubeInfo->guid);
    if (!cube)
        return false;

    // Release cubes after Blast Nova is interrupted
    if (HandleCubeRelease(magtheridon))
        return true;

    // Handle active cube logic based on Blast Nova casting state
    const bool blastNovaActive =
        magtheridon->HasUnitState(UNIT_STATE_CASTING) &&
        magtheridon->FindCurrentSpellBySpellId(
            static_cast<uint32>(MagtheridonSpells::SPELL_BLAST_NOVA));

    // If Blast Nova is actively casting, always try to click the cube
    if (blastNovaActive)
        return HandleCubeInteraction(*cubeInfo, cube);

    // Otherwise, if Blast Nova is coming soon, move to and wait near the cube
    return HandleWaitingPhase(*cubeInfo);
}

CubeInfo const* MagtheridonUseManticronCubeAction::GetAssignedCube()
{
    auto mapIt = botToCubeAssignments.find(bot->GetMap()->GetInstanceId());
    if (mapIt == botToCubeAssignments.end())
        return nullptr;

    auto it = mapIt->second.find(bot->GetGUID());
    return it != mapIt->second.end() ? &it->second : nullptr;
}

bool MagtheridonUseManticronCubeAction::HandleCubeRelease(Unit* magtheridon)
{
    if (bot->HasAura(static_cast<uint32>(MagtheridonSpells::SPELL_SHADOW_GRASP)) &&
        !(magtheridon->HasUnitState(UNIT_STATE_CASTING) &&
          magtheridon->FindCurrentSpellBySpellId(
            static_cast<uint32>(MagtheridonSpells::SPELL_BLAST_NOVA))))
    {
        uint32 delay = urand(200, 3000);
        botAI->AddTimedEvent(
            [this]
            {
                botAI->Reset();
            },
            delay);
        botAI->SetNextCheckDelay(delay + 50);
        return true;
    }

    return false;
}

bool MagtheridonUseManticronCubeAction::HandleWaitingPhase(const CubeInfo& cubeInfo)
{
    auto timerIt = blastNovaTimer.find(bot->GetMap()->GetInstanceId());
    if (timerIt == blastNovaTimer.end() ||
        time(nullptr) - timerIt->second < BLAST_NOVA_INTERIM_SECONDS)
    {
        return false;
    }

    constexpr float safeWaitDistance = 8.0f;

    if (fabs(bot->GetDistance2d(cubeInfo.x, cubeInfo.y) - safeWaitDistance) <= 1.0f)
        return true;

    if (Position safePos; FindSafePositionNearCube(cubeInfo, safeWaitDistance, safePos))
    {
        botAI->InterruptSpell();
        return MoveTo(MAGTHERIDON_MAP_ID, safePos.GetPositionX(), safePos.GetPositionY(),
                      bot->GetPositionZ(), false, false, false, false,
                      MovementPriority::MOVEMENT_FORCED, true, false);
    }

    return false;
}

bool MagtheridonUseManticronCubeAction::FindSafePositionNearCube(
    const CubeInfo& cubeInfo, float preferredDistance, Position& outPos)
{
    constexpr float angleStep = M_PI / 8.0f;

    float minMoveDistance = std::numeric_limits<float>::max();
    bool foundSafe = false;

    for (float angle = 0.0f; angle < 2.0f * M_PI; angle += angleStep)
    {
        const float x = cubeInfo.x + std::cos(angle) * preferredDistance;
        const float y = cubeInfo.y + std::sin(angle) * preferredDistance;

        if (IsPositionInActiveDebris(bot->GetMap()->GetInstanceId(), x, y))
            continue;

        if (IsPositionInActiveConflagration(botAI, bot, x, y))
            continue;

        const float moveDistance = bot->GetExactDist2d(x, y);
        if (moveDistance < minMoveDistance)
        {
            outPos = Position(x, y, bot->GetPositionZ());
            minMoveDistance = moveDistance;
            foundSafe = true;
        }
    }

    return foundSafe;
}

bool MagtheridonUseManticronCubeAction::HandleCubeInteraction(
    const CubeInfo& cubeInfo, GameObject* cube)
{
    constexpr float interactDistance = 1.0f;
    const float cubeDist = bot->GetDistance2d(cubeInfo.x, cubeInfo.y);

    if (cubeDist < interactDistance + 1.0f)
    {
        uint32 delay = urand(200, 1500);
        botAI->AddTimedEvent(
            [this, cube]
            {
                bot->StopMoving();
                cube->Use(bot);
            },
            delay);
        botAI->SetNextCheckDelay(delay + 50);
        return true;
    }

    botAI->InterruptSpell();
    return MoveTo(cube, interactDistance, MovementPriority::MOVEMENT_FORCED);
}

bool MagtheridonMoveOutOfDebrisAction::Execute(Event /*event*/)
{
    if (Position safePos; FindSafePosition(safePos))
    {
        botAI->InterruptSpell();
        return MoveTo(MAGTHERIDON_MAP_ID, safePos.GetPositionX(), safePos.GetPositionY(),
                      bot->GetPositionZ(), false, false, false, false,
                      MovementPriority::MOVEMENT_FORCED, true, false);
    }

    return false;
}

bool MagtheridonMoveOutOfDebrisAction::FindSafePosition(Position& outPos)
{
    constexpr float maxSearchRadius = 20.0f;
    constexpr float distanceStep = 1.0f;
    constexpr float angleStep = M_PI / 12.0f;

    float minMoveDistance = std::numeric_limits<float>::max();
    bool foundSafe = false;
    uint32 const instanceId = bot->GetMap()->GetInstanceId();

    for (float distance = 2.0f; distance <= maxSearchRadius; distance += distanceStep)
    {
        for (float angle = 0.0f; angle < 2.0f * M_PI; angle += angleStep)
        {
            float const x = bot->GetPositionX() + distance * std::cos(angle);
            float const y = bot->GetPositionY() + distance * std::sin(angle);

            if (IsPositionInActiveDebris(instanceId, x, y))
                continue;

            if (IsPositionInActiveConflagration(botAI, bot, x, y))
                continue;

            float const moveDistance = bot->GetExactDist2d(x, y);

            if (!foundSafe || moveDistance < minMoveDistance)
            {
                outPos = Position(x, y, bot->GetPositionZ());
                minMoveDistance = moveDistance;
                foundSafe = true;
            }
        }
    }

    return foundSafe;
}

bool MagtheridonManageTimersAndAssignmentsAction::Execute(Event /*event*/)
{
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    if (!magtheridon)
        return false;

    const uint32 instanceId = magtheridon->GetMap()->GetInstanceId();
    const time_t now = time(nullptr);

    const bool blastNovaActive =
        magtheridon->HasUnitState(UNIT_STATE_CASTING) &&
        magtheridon->FindCurrentSpellBySpellId(
            static_cast<uint32>(MagtheridonSpells::SPELL_BLAST_NOVA));

    if (!lastBlastNovaState[instanceId] && blastNovaActive)
        blastNovaTimer[instanceId] = now;

    lastBlastNovaState[instanceId] = blastNovaActive;

    bool updated = false;

    if (IsMagtheridonActive(magtheridon))
    {
        if (blastNovaTimer.try_emplace(instanceId, now).second)
            updated = true;

        if (dpsWaitTimer.try_emplace(instanceId, now).second)
            updated = true;

        if (magtheridon->GetHealthPct() < 30.0f && !ceilingCollapseApplied[instanceId])
        {
            blastNovaTimer[instanceId] += 18;
            ceilingCollapseApplied[instanceId] = true;
            updated = true;
        }

        if (NeedsCubeReassignment(instanceId) && AssignCubeClickers())
            updated = true;
    }
    else
    {
        if (blastNovaTimer.erase(instanceId) > 0)
            updated = true;

        if (dpsWaitTimer.erase(instanceId) > 0)
            updated = true;

        if (botToCubeAssignments.erase(instanceId) > 0)
            updated = true;

        if (ceilingCollapseApplied.erase(instanceId) > 0)
            updated = true;

        if (lastBlastNovaState.erase(instanceId) > 0)
            updated = true;
    }

    return updated;
}

bool MagtheridonManageTimersAndAssignmentsAction::AssignCubeClickers()
{
    const uint32 instanceId = bot->GetMap()->GetInstanceId();
    std::vector<CubeInfo> cubes = GetAllCubeInfosByDbGuids(bot->GetMap(), MANTICRON_CUBE_DB_GUIDS);

    auto& assignment = botToCubeAssignments[instanceId];
    Group* group = bot->GetGroup();
    if (!group || cubes.empty())
    {
        assignment.clear();
        return true;
    }

    // Prune dead or absent players from the existing assignment
    for (auto it = assignment.begin(); it != assignment.end(); )
    {
        Player* player = ObjectAccessor::FindPlayer(it->first);
        if (!player || !player->IsAlive() || player->GetMapId() != MAGTHERIDON_MAP_ID)
            it = assignment.erase(it);
        else
            ++it;
    }

    // Fill unassigned cubes
    for (CubeInfo const& cube : cubes)
    {
        bool alreadyAssigned = false;
        for (auto const& pair : assignment)
        {
            if (pair.second.guid == cube.guid)
            {
                alreadyAssigned = true;
                break;
            }
        }
        if (alreadyAssigned)
            continue;

        Player* candidate = nullptr;

        // Pass 1: ranged DPS excluding warlocks
        for (GroupReference* ref = group->GetFirstMember();
             ref && !candidate; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive() || !botAI->IsRangedDps(member) ||
                !GET_PLAYERBOT_AI(member) || member->getClass() == CLASS_WARLOCK)
                continue;

            if (assignment.find(member->GetGUID()) != assignment.end())
                continue;

            candidate = member;
        }

        // Pass 2: any non-tank bot
        if (!candidate)
        {
            for (GroupReference* ref = group->GetFirstMember();
                 ref && !candidate; ref = ref->next())
            {
                Player* member = ref->GetSource();
                if (!member || !member->IsAlive() || botAI->IsTank(member) ||
                    !GET_PLAYERBOT_AI(member))
                    continue;

                if (assignment.find(member->GetGUID()) != assignment.end())
                    continue;

                candidate = member;
            }
        }

        if (candidate)
            assignment[candidate->GetGUID()] = cube;
    }

    return true;
}

bool MagtheridonManageTimersAndAssignmentsAction::NeedsCubeReassignment(const uint32 instanceId)
{
    auto mapIt = botToCubeAssignments.find(instanceId);
    if (mapIt == botToCubeAssignments.end() || mapIt->second.empty())
        return true;

    for (auto const& pair : mapIt->second)
    {
        Player* assigned = ObjectAccessor::FindPlayer(pair.first);
        if (!assigned || !assigned->IsAlive())
            return true;
    }

    return false;
}

bool MagtheridonEraseTimersAndTrackersAction::Execute(Event /*event*/)
{
    const uint32 instanceId = bot->GetMap()->GetInstanceId();
    bool erased = false;

    if (blastNovaTimer.erase(instanceId) > 0)
        erased = true;
    if (dpsWaitTimer.erase(instanceId) > 0)
        erased = true;
    if (ceilingCollapseApplied.erase(instanceId) > 0)
        erased = true;
    if (lastBlastNovaState.erase(instanceId) > 0)
        erased = true;
    if (botToCubeAssignments.erase(instanceId) > 0)
        erased = true;
    if (activeDebrisPositions.erase(instanceId) > 0)
        erased = true;

    return erased;
}
