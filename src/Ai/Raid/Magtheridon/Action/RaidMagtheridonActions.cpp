#include "RaidMagtheridonActions.h"
#include "RaidMagtheridonHelpers.h"
#include "Creature.h"
#include "ObjectAccessor.h"
#include "ObjectGuid.h"
#include "Playerbots.h"
#include "RaidBossHelpers.h"

using namespace MagtheridonHelpers;

bool MagtheridonMainTankAttackFirstThreeChannelersAction::Execute(Event event)
{
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    if (!magtheridon)
        return false;

    Creature* channelerSquare = GetChanneler(bot, SOUTH_CHANNELER);
    if (channelerSquare)
        MarkTargetWithSquare(bot, channelerSquare);

    Creature* channelerStar   = GetChanneler(bot, WEST_CHANNELER);
    if (channelerStar)
        MarkTargetWithStar(bot, channelerStar);

    Creature* channelerCircle = GetChanneler(bot, EAST_CHANNELER);
    if (channelerCircle)
        MarkTargetWithCircle(bot, channelerCircle);

    // After first three channelers are dead, wait for Magtheridon to activate
    if (!channelerSquare && !channelerStar && !channelerCircle)
    {
        const Position& position = WAITING_FOR_MAGTHERIDON_POSITION;
        if (!bot->IsWithinDist2d(position.GetPositionX(), position.GetPositionY(), 2.0f))
        {
            return MoveTo(MAGTHERIDON_MAP_ID, position.GetPositionX(), position.GetPositionY(),
                          position.GetPositionZ(), false, false, false, false,
                          MovementPriority::MOVEMENT_COMBAT, true, false);
        }

        bot->SetFacingTo(position.GetOrientation());
        return true;
    }

    Creature* currentTarget = nullptr;
    std::string rtiName;
    if (channelerSquare)
    {
        currentTarget = channelerSquare;
        rtiName = "square";
    }
    else if (channelerStar)
    {
        currentTarget = channelerStar;
        rtiName = "star";
    }
    else if (channelerCircle)
    {
        currentTarget = channelerCircle;
        rtiName = "circle";
    }

    SetRtiTarget(botAI, rtiName, currentTarget);

    if (currentTarget && bot->GetVictim() != currentTarget)
        return Attack(currentTarget);

    return false;
}

bool MagtheridonFirstAssistTankAttackNWChannelerAction::Execute(Event event)
{
    Creature* channelerDiamond = GetChanneler(bot, NORTHWEST_CHANNELER);
    if (!channelerDiamond)
        return false;

    MarkTargetWithDiamond(bot, channelerDiamond);
    SetRtiTarget(botAI, "diamond", channelerDiamond);

    if (bot->GetVictim() != channelerDiamond)
        return Attack(channelerDiamond);

    if (channelerDiamond->GetVictim() == bot)
    {
        const Position& position = NW_CHANNELER_TANK_POSITION;
        const float maxDistance = 3.0f;
        float distanceToPosition = bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());

        if (distanceToPosition > maxDistance)
        {
            float dX = position.GetPositionX() - bot->GetPositionX();
            float dY = position.GetPositionY() - bot->GetPositionY();
            float moveX = bot->GetPositionX() + (dX / distanceToPosition) * maxDistance;
            float moveY = bot->GetPositionY() + (dY / distanceToPosition) * maxDistance;

            return MoveTo(MAGTHERIDON_MAP_ID, moveX, moveY, position.GetPositionZ(), false, false, false, false,
                          MovementPriority::MOVEMENT_COMBAT, true, false);
        }
    }

    return false;
}

bool MagtheridonSecondAssistTankAttackNEChannelerAction::Execute(Event event)
{
    Creature* channelerTriangle = GetChanneler(bot, NORTHEAST_CHANNELER);
    if (!channelerTriangle)
        return false;

    MarkTargetWithTriangle(bot, channelerTriangle);
    SetRtiTarget(botAI, "triangle", channelerTriangle);

    if (bot->GetVictim() != channelerTriangle)
        return Attack(channelerTriangle);

    if (channelerTriangle->GetVictim() == bot)
    {
        const Position& position = NE_CHANNELER_TANK_POSITION;
        const float maxDistance = 3.0f;
        float distanceToPosition = bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());

        if (distanceToPosition > maxDistance)
        {
            float dX = position.GetPositionX() - bot->GetPositionX();
            float dY = position.GetPositionY() - bot->GetPositionY();
            float moveX = bot->GetPositionX() + (dX / distanceToPosition) * maxDistance;
            float moveY = bot->GetPositionY() + (dY / distanceToPosition) * maxDistance;

            return MoveTo(MAGTHERIDON_MAP_ID, moveX, moveY, position.GetPositionZ(), false, false, false, false,
                          MovementPriority::MOVEMENT_COMBAT, true, false);
        }
    }

    return false;
}

// Misdirect West & East Channelers to Main Tank
bool MagtheridonMisdirectHellfireChannelers::Execute(Event event)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    std::vector<Player*> hunters;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive() && member->getClass() == CLASS_HUNTER && GET_PLAYERBOT_AI(member))
            hunters.push_back(member);
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

    Player* mainTank = nullptr;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive() && botAI->IsMainTank(member))
        {
            mainTank = member;
            break;
        }
    }

    Creature* channelerStar = GetChanneler(bot, WEST_CHANNELER);
    Creature* channelerCircle = GetChanneler(bot, EAST_CHANNELER);

    switch (hunterIndex)
    {
    case 0:
        if (mainTank && channelerStar &&
            channelerStar->GetVictim() != mainTank)
        {
            if (botAI->CanCastSpell("misdirection", mainTank))
                return botAI->CastSpell("misdirection", mainTank);

            if (!bot->HasAura(SPELL_MISDIRECTION))
                return false;

            if (botAI->CanCastSpell("steady shot", channelerStar))
                return botAI->CastSpell("steady shot", channelerStar);
        }
        break;

    case 1:
        if (mainTank && channelerCircle &&
            channelerCircle->GetVictim() != mainTank)
        {
            if (botAI->CanCastSpell("misdirection", mainTank))
                return botAI->CastSpell("misdirection", mainTank);

            if (!bot->HasAura(SPELL_MISDIRECTION))
                return false;

            if (botAI->CanCastSpell("steady shot", channelerCircle))
                return botAI->CastSpell("steady shot", channelerCircle);
        }
        break;

    default:
        break;
    }

    return false;
}

bool MagtheridonAssignDPSPriorityAction::Execute(Event event)
{
    // Listed in order of priority
    Creature* channelerSquare   = GetChanneler(bot, SOUTH_CHANNELER);
    if (channelerSquare)
    {
        SetRtiTarget(botAI, "square", channelerSquare);

        if (bot->GetTarget() != channelerSquare->GetGUID())
            return Attack(channelerSquare);

        return false;
    }

    Creature* channelerStar = GetChanneler(bot, WEST_CHANNELER);
    if (channelerStar)
    {
        SetRtiTarget(botAI, "star", channelerStar);

        if (bot->GetTarget() != channelerStar->GetGUID())
            return Attack(channelerStar);

        return false;
    }

    Creature* channelerCircle = GetChanneler(bot, EAST_CHANNELER);
    if (channelerCircle)
    {
        SetRtiTarget(botAI, "circle", channelerCircle);

        if (bot->GetTarget() != channelerCircle->GetGUID())
            return Attack(channelerCircle);

        return false;
    }

    Creature* channelerDiamond  = GetChanneler(bot, NORTHWEST_CHANNELER);
    if (channelerDiamond)
    {
        SetRtiTarget(botAI, "diamond", channelerDiamond);

        if (bot->GetTarget() != channelerDiamond->GetGUID())
            return Attack(channelerDiamond);

        return false;
    }

    Creature* channelerTriangle = GetChanneler(bot, NORTHEAST_CHANNELER);
    if (channelerTriangle)
    {
        SetRtiTarget(botAI, "triangle", channelerTriangle);

        if (bot->GetTarget() != channelerTriangle->GetGUID())
            return Attack(channelerTriangle);

        return false;
    }

    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    if (magtheridon && !magtheridon->HasAura(SPELL_SHADOW_CAGE) &&
        !channelerSquare && !channelerStar && !channelerCircle &&
        !channelerDiamond && !channelerTriangle)
    {
        SetRtiTarget(botAI, "cross", magtheridon);

        if (bot->GetTarget() != magtheridon->GetGUID())
            return Attack(magtheridon);
    }

    return false;
}

// Assign Burning Abyssals to Warlocks to Banish
// Burning Abyssals in excess of Warlocks in party will be Feared
bool MagtheridonWarlockCCBurningAbyssalAction::Execute(Event event)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    const GuidVector& npcs = AI_VALUE(GuidVector, "nearest hostile npcs");

    std::vector<Unit*> abyssals;
    for (auto const& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (unit && unit->GetEntry() == NPC_BURNING_ABYSSAL && unit->IsAlive())
            abyssals.push_back(unit);
    }

    std::vector<Player*> warlocks;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive() && member->getClass() == CLASS_WARLOCK && GET_PLAYERBOT_AI(member))
            warlocks.push_back(member);
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
        if (!botAI->HasAura("banish", assignedAbyssal) && botAI->CanCastSpell("banish", assignedAbyssal))
            return botAI->CastSpell("banish", assignedAbyssal);
    }

    for (size_t i = warlocks.size(); i < abyssals.size(); ++i)
    {
        Unit* excessAbyssal = abyssals[i];
        if (!botAI->HasAura("banish", excessAbyssal) && !botAI->HasAura("fear", excessAbyssal) &&
            botAI->CanCastSpell("fear", excessAbyssal))
            return botAI->CastSpell("fear", excessAbyssal);
    }

    return false;
}

// Main tank will back up to the Northern point of the room
bool MagtheridonMainTankPositionBossAction::Execute(Event event)
{
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    if (!magtheridon)
        return false;

    MarkTargetWithCross(bot, magtheridon);
    SetRtiTarget(botAI, "cross", magtheridon);

    if (bot->GetVictim() != magtheridon)
        return Attack(magtheridon);

    if (magtheridon->GetVictim() == bot)
    {
        const Position& position = MAGTHERIDON_TANK_POSITION;
        const float maxDistance = 2.0f;
        float distanceToPosition = bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());

        if (distanceToPosition > maxDistance)
        {
            float dX = position.GetPositionX() - bot->GetPositionX();
            float dY = position.GetPositionY() - bot->GetPositionY();
            float moveX = bot->GetPositionX() + (dX / distanceToPosition) * maxDistance;
            float moveY = bot->GetPositionY() + (dY / distanceToPosition) * maxDistance;

            return MoveTo(MAGTHERIDON_MAP_ID, moveX, moveY, position.GetPositionZ(), false, false, false, false,
                          MovementPriority::MOVEMENT_COMBAT, true, true);
        }
    }

    return false;
}

// Ranged DPS will remain within 25 yards of the center of the room
// Healers will remain within 15 yards of a position that is between ranged DPS and the boss
std::unordered_map<ObjectGuid, Position> MagtheridonSpreadRangedAction::initialPositions;
std::unordered_map<ObjectGuid, bool> MagtheridonSpreadRangedAction::hasReachedInitialPosition;

bool MagtheridonSpreadRangedAction::Execute(Event event)
{
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    if (!magtheridon)
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    const uint32 instanceId = magtheridon->GetMap()->GetInstanceId();

    // Wait for 6 seconds after Magtheridon activates to spread
    const uint8 spreadWaitSeconds = 6;
    auto it = spreadWaitTimer.find(instanceId);
    if (it == spreadWaitTimer.end() ||
        (time(nullptr) - it->second) < spreadWaitSeconds)
        return false;

    auto cubeIt = botToCubeAssignment.find(bot->GetGUID());
    if (cubeIt != botToCubeAssignment.end())
    {
        time_t now = time(nullptr);
        auto timerIt = blastNovaTimer.find(instanceId);
        if (timerIt != blastNovaTimer.end())
        {
            time_t lastBlastNova = timerIt->second;
            if (now - lastBlastNova >= 49)
                return false;
        }
    }

    std::vector<Player*> members;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive())
            members.push_back(member);
    }

    bool isHealer = botAI->IsHeal(bot);
    const Position& center = isHealer
        ? HEALER_SPREAD_POSITION
        : RANGED_SPREAD_POSITION;
    float maxSpreadRadius = isHealer ? 15.0f : 20.0f;
    float centerX = center.GetPositionX();
    float centerY = center.GetPositionY();
    float centerZ = center.GetPositionZ();
    const float radiusBuffer = 3.0f;

    if (!initialPositions.count(bot->GetGUID()))
    {
        auto it = std::find(members.begin(), members.end(), bot);
        uint8 botIndex = (it != members.end()) ? std::distance(members.begin(), it) : 0;
        uint8 count = members.size();

        float angle = 2 * M_PI * botIndex / count;
        float radius = static_cast<float>(rand()) / RAND_MAX * maxSpreadRadius;
        float targetX = centerX + radius * cos(angle);
        float targetY = centerY + radius * sin(angle);

        initialPositions[bot->GetGUID()] = Position(targetX, targetY, centerZ);
        hasReachedInitialPosition[bot->GetGUID()] = false;
    }

    Position targetPosition = initialPositions[bot->GetGUID()];
    if (!hasReachedInitialPosition[bot->GetGUID()])
    {
        if (!bot->IsWithinDist2d(targetPosition.GetPositionX(), targetPosition.GetPositionY(), 2.0f))
        {
            float destX = targetPosition.GetPositionX();
            float destY = targetPosition.GetPositionY();
            float destZ = targetPosition.GetPositionZ();

            if (!bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bot->GetPositionX(),
                bot->GetPositionY(), bot->GetPositionZ(), destX, destY, destZ))
                return false;

            bot->AttackStop();
            bot->InterruptNonMeleeSpells(false);
            return MoveTo(MAGTHERIDON_MAP_ID, destX, destY, destZ, false, false, false, false,
                          MovementPriority::MOVEMENT_COMBAT, true, false);
        }
        hasReachedInitialPosition[bot->GetGUID()] = true;
    }

    float distToCenter = bot->GetExactDist2d(centerX, centerY);

    if (distToCenter > maxSpreadRadius + radiusBuffer)
    {
        float angle = static_cast<float>(rand()) / RAND_MAX * 2.0f * M_PI;
        float radius = static_cast<float>(rand()) / RAND_MAX * maxSpreadRadius;
        float targetX = centerX + radius * cos(angle);
        float targetY = centerY + radius * sin(angle);

        if (bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bot->GetPositionX(), bot->GetPositionY(),
            bot->GetPositionZ(), targetX, targetY, centerZ))
        {
            bot->AttackStop();
            bot->InterruptNonMeleeSpells(false);
            return MoveTo(MAGTHERIDON_MAP_ID, targetX, targetY, centerZ, false, false, false, false,
                          MovementPriority::MOVEMENT_COMBAT, true, false);
        }
    }

    return false;
}

// For bots that are assigned to click cubes
// Magtheridon casts Blast Nova every 54.35 to 55.40s, with a 2s cast time
bool MagtheridonUseManticronCubeAction::Execute(Event event)
{
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    if (!magtheridon)
        return false;

    auto it = botToCubeAssignment.find(bot->GetGUID());
    const CubeInfo& cubeInfo = it->second;
    GameObject* cube = botAI->GetGameObject(cubeInfo.guid);
    if (!cube)
        return false;

    // Release cubes after Blast Nova is interrupted
    if (HandleCubeRelease(magtheridon, cube))
        return true;

    // Check if cube logic should be active (49+ second rule)
    if (!ShouldActivateCubeLogic(magtheridon))
        return false;

    // Handle active cube logic based on Blast Nova casting state
    bool blastNovaActive = magtheridon->HasUnitState(UNIT_STATE_CASTING) &&
                           magtheridon->FindCurrentSpellBySpellId(SPELL_BLAST_NOVA);

    if (!blastNovaActive)
        // After 49 seconds, wait at safe distance from cube
        return HandleWaitingPhase(cubeInfo);
    else
        // Blast Nova is casting - move to and click cube
        return HandleCubeInteraction(cubeInfo, cube);

    return false;
}

bool MagtheridonUseManticronCubeAction::HandleCubeRelease(Unit* magtheridon, GameObject* cube)
{
    if (bot->HasAura(SPELL_SHADOW_GRASP) &&
        !(magtheridon->HasUnitState(UNIT_STATE_CASTING) &&
          magtheridon->FindCurrentSpellBySpellId(SPELL_BLAST_NOVA)))
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

bool MagtheridonUseManticronCubeAction::ShouldActivateCubeLogic(Unit* magtheridon)
{
    auto timerIt = blastNovaTimer.find(magtheridon->GetMap()->GetInstanceId());
    if (timerIt == blastNovaTimer.end())
        return false;

    time_t now = time(nullptr);
    time_t lastBlastNova = timerIt->second;

    return (now - lastBlastNova >= 49);
}

bool MagtheridonUseManticronCubeAction::HandleWaitingPhase(const CubeInfo& cubeInfo)
{
    const float safeWaitDistance = 8.0f;
    float cubeDist = bot->GetExactDist2d(cubeInfo.x, cubeInfo.y);

    if (fabs(cubeDist - safeWaitDistance) > 1.0f)
    {
        for (int i = 0; i < 12; ++i)
        {
            float angle = i * M_PI / 6.0f;
            float targetX = cubeInfo.x + cos(angle) * safeWaitDistance;
            float targetY = cubeInfo.y + sin(angle) * safeWaitDistance;
            float targetZ = bot->GetPositionZ();

            if (IsSafeFromMagtheridonHazards(botAI, bot, targetX, targetY, targetZ))
            {
                bot->AttackStop();
                bot->InterruptNonMeleeSpells(true);
                return MoveTo(MAGTHERIDON_MAP_ID, targetX, targetY, targetZ, false, false, false, false,
                              MovementPriority::MOVEMENT_COMBAT, true, false);
            }
        }

        float angle = static_cast<float>(rand()) / RAND_MAX * 2.0f * M_PI;
        float fallbackX = cubeInfo.x + cos(angle) * safeWaitDistance;
        float fallbackY = cubeInfo.y + sin(angle) * safeWaitDistance;
        float fallbackZ = bot->GetPositionZ();

        return MoveTo(MAGTHERIDON_MAP_ID, fallbackX, fallbackY, fallbackZ, false, false, false, false,
                      MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    return true;
}

bool MagtheridonUseManticronCubeAction::HandleCubeInteraction(const CubeInfo& cubeInfo, GameObject* cube)
{
    const float interactDistance = 1.0f;
    float cubeDist = bot->GetExactDist2d(cubeInfo.x, cubeInfo.y);

    if (cubeDist > interactDistance)
    {
        if (cubeDist <= interactDistance + 1.0f)
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

        float angle = atan2(cubeInfo.y - bot->GetPositionY(), cubeInfo.x - bot->GetPositionX());
        float targetX = cubeInfo.x - cos(angle) * interactDistance;
        float targetY = cubeInfo.y - sin(angle) * interactDistance;
        float targetZ = bot->GetPositionZ();

        bot->AttackStop();
        bot->InterruptNonMeleeSpells(true);
        return MoveTo(MAGTHERIDON_MAP_ID, targetX, targetY, targetZ, false, false, false, false,
                      MovementPriority::MOVEMENT_FORCED, true, false);
    }

    return false;
}

// The Blast Nova timer resets when Magtheridon stops casting it, which is needed to ensure that bots use cubes.
// However, Magtheridon's Blast Nova cooldown actually runs from when he starts casting it. This means that if a Blast Nova
// is not interrupted or takes too long to interrupt, the timer will be thrown off for the rest of the encounter.
// Correcting this issue is complicated and probably would need some rewriting--I have not done so and
// and view the current solution as sufficient since in TBC a missed Blast Nova would be a guaranteed wipe anyway.
bool MagtheridonManageTimersAndAssignmentsAction::Execute(Event event)
{
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    if (!magtheridon)
        return false;

    const uint32 instanceId = magtheridon->GetMap()->GetInstanceId();
    const time_t now = time(nullptr);

    bool blastNovaActive = magtheridon->HasUnitState(UNIT_STATE_CASTING) &&
                           magtheridon->FindCurrentSpellBySpellId(SPELL_BLAST_NOVA);
    bool lastBlastNova = lastBlastNovaState[instanceId];

    if (lastBlastNova && !blastNovaActive)
        blastNovaTimer[instanceId] = now;

    lastBlastNovaState[instanceId] = blastNovaActive;

    if (!magtheridon->HasAura(SPELL_SHADOW_CAGE))
    {
        if (IsMechanicTrackerBot(botAI, bot, MAGTHERIDON_MAP_ID, nullptr))
        {
            spreadWaitTimer.try_emplace(instanceId, now);
            blastNovaTimer.try_emplace(instanceId, now);
            dpsWaitTimer.try_emplace(instanceId, now);
        }
    }
    else
    {
        ObjectGuid guid = bot->GetGUID();
        MagtheridonSpreadRangedAction::initialPositions.erase(guid);
        MagtheridonSpreadRangedAction::hasReachedInitialPosition.erase(guid);
        botToCubeAssignment.erase(guid);

        if (IsMechanicTrackerBot(botAI, bot, MAGTHERIDON_MAP_ID, nullptr))
        {
            spreadWaitTimer.erase(instanceId);
            blastNovaTimer.erase(instanceId);
            dpsWaitTimer.erase(instanceId);
        }
    }

    return false;
}
