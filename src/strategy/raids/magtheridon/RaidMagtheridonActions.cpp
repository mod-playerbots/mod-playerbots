#include "RaidMagtheridonActions.h"
#include "RaidMagtheridonHelpers.h"
#include "Creature.h"
#include "ObjectAccessor.h"
#include "ObjectGuid.h"
#include "Playerbots.h"

using namespace MagtheridonHelpers;

bool MagtheridonHellfireChannelerMainTankAction::Execute(Event event)
{
    // I'm adding this check at the top of each action because Magtheridon disappears upon a wipe
    // And thus the risk of null pointer dereference is too high if the check is only in the triggers
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    if (!magtheridon)
        return false;

    Group* group = bot->GetGroup();
    Creature* channelerSquare = GetChanneler(bot, SOUTH_CHANNELER);
    Creature* channelerStar   = GetChanneler(bot, WEST_CHANNELER);
    Creature* channelerCircle = GetChanneler(bot, EAST_CHANNELER);

    if (channelerSquare && channelerSquare->IsAlive())
    {
        ObjectGuid currentIconGuid = group->GetTargetIcon(squareIcon);
        if (currentIconGuid.IsEmpty() || currentIconGuid != channelerSquare->GetGUID())
            group->SetTargetIcon(squareIcon, bot->GetGUID(), channelerSquare->GetGUID());
    }
    if (channelerStar && channelerStar->IsAlive())
    {
        ObjectGuid currentIconGuid = group->GetTargetIcon(starIcon);
        if (currentIconGuid.IsEmpty() || currentIconGuid != channelerStar->GetGUID())
            group->SetTargetIcon(starIcon, bot->GetGUID(), channelerStar->GetGUID());
    }
    if (channelerCircle && channelerCircle->IsAlive())
    {
        ObjectGuid currentIconGuid = group->GetTargetIcon(circleIcon);
        if (currentIconGuid.IsEmpty() || currentIconGuid != channelerCircle->GetGUID())
            group->SetTargetIcon(circleIcon, bot->GetGUID(), channelerCircle->GetGUID());
    }

    // After first three channelers are dead, wait for Magtheridon to activate
    if ((!channelerSquare || !channelerSquare->IsAlive()) &&
        (!channelerStar   || !channelerStar->IsAlive()) &&
        (!channelerCircle || !channelerCircle->IsAlive()))
    {
        const Location& position = MagtheridonsLairLocations::WaitingForMagtheridonPosition;
        if (!bot->IsWithinDist2d(position.x, position.y, 2.0f))
            return MoveTo(bot->GetMapId(), position.x, position.y, position.z, false, false, false, false, 
                          MovementPriority::MOVEMENT_COMBAT, true, false);

        bot->SetFacingTo(position.orientation);
        return true;
    }

    Creature* currentTarget = nullptr;
    std::string rtiName;
    if (channelerSquare && channelerSquare->IsAlive())
    {
        currentTarget = channelerSquare;
        rtiName = "square";
    }
    else if (channelerStar && channelerStar->IsAlive())
    {
        currentTarget = channelerStar;
        rtiName = "star";
    }
    else if (channelerCircle && channelerCircle->IsAlive())
    {
        currentTarget = channelerCircle;
        rtiName = "circle";
    }

    if (currentTarget &&
        (botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Get() != rtiName ||
         botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Get() != currentTarget))
    {
        botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Set(rtiName);
        botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Set(currentTarget);
    }

    if (currentTarget && bot->GetVictim() != currentTarget)
    {
        Attack(currentTarget);

        if (!bot->IsWithinMeleeRange(currentTarget))
            return MoveTo(currentTarget->GetMapId(), currentTarget->GetPositionX(), 
                          currentTarget->GetPositionY(), currentTarget->GetPositionZ());
    }

    return false;
}

bool MagtheridonHellfireChannelerNWChannelerTankAction::Execute(Event event)
{
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    if (!magtheridon)
        return false;

    Group* group = bot->GetGroup();
    Creature* channelerDiamond = GetChanneler(bot, NORTHWEST_CHANNELER);

    ObjectGuid currentIconGuid = group->GetTargetIcon(diamondIcon);
    if (currentIconGuid.IsEmpty() || currentIconGuid != channelerDiamond->GetGUID())
        group->SetTargetIcon(diamondIcon, bot->GetGUID(), channelerDiamond->GetGUID());

    if (botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Get() != "diamond" ||
        botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Get() != channelerDiamond)
    {
        botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Set("diamond");
        botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Set(channelerDiamond);
    }

    if (bot->GetVictim() != channelerDiamond)
        Attack(channelerDiamond);

    if (channelerDiamond->GetVictim() == bot)
    {
        const Location& position = MagtheridonsLairLocations::NWChannelerTankPosition;
        const float maxDistance = 3.0f;

        float distanceToPosition = bot->GetExactDist2d(position.x, position.y);

        if (distanceToPosition > maxDistance)
        {
            float dX = position.x - bot->GetPositionX();
            float dY = position.y - bot->GetPositionY();
            float dist = sqrt(dX * dX + dY * dY);
            float moveX = bot->GetPositionX() + (dX / dist) * maxDistance;
            float moveY = bot->GetPositionY() + (dY / dist) * maxDistance;

            return MoveTo(bot->GetMapId(), moveX, moveY, position.z, false, false, false, false, 
                          MovementPriority::MOVEMENT_COMBAT, true, false);
        }
        else if (!bot->IsWithinMeleeRange(channelerDiamond))
            return MoveTo(channelerDiamond->GetMapId(), channelerDiamond->GetPositionX(),
                          channelerDiamond->GetPositionY(), channelerDiamond->GetPositionZ());
    }

    return false;
}

bool MagtheridonHellfireChannelerNEChannelerTankAction::Execute(Event event)
{
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    if (!magtheridon)
        return false;
    
    Group* group = bot->GetGroup();
    Creature* channelerTriangle = GetChanneler(bot, NORTHEAST_CHANNELER);

    ObjectGuid currentIconGuid = group->GetTargetIcon(triangleIcon);
    if (currentIconGuid.IsEmpty() || currentIconGuid != channelerTriangle->GetGUID())
        group->SetTargetIcon(triangleIcon, bot->GetGUID(), channelerTriangle->GetGUID());

    if (botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Get() != "triangle" ||
        botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Get() != channelerTriangle)
    {
        botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Set("triangle");
        botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Set(channelerTriangle);
    }

    if (bot->GetVictim() != channelerTriangle)
        Attack(channelerTriangle);

    if (channelerTriangle->GetVictim() == bot)
    {
        const Location& position = MagtheridonsLairLocations::NEChannelerTankPosition;
        const float maxDistance = 3.0f;

        float distanceToPosition = bot->GetExactDist2d(position.x, position.y);

        if (distanceToPosition > maxDistance)
        {
            float dX = position.x - bot->GetPositionX();
            float dY = position.y - bot->GetPositionY();
            float dist = sqrt(dX * dX + dY * dY);
            float moveX = bot->GetPositionX() + (dX / dist) * maxDistance;
            float moveY = bot->GetPositionY() + (dY / dist) * maxDistance;

            return MoveTo(bot->GetMapId(), moveX, moveY, position.z, false, false, false, false, 
                          MovementPriority::MOVEMENT_COMBAT, true, false);
        }
        else if (!bot->IsWithinMeleeRange(channelerTriangle))
            return MoveTo(channelerTriangle->GetMapId(), channelerTriangle->GetPositionX(),
                          channelerTriangle->GetPositionY(), channelerTriangle->GetPositionZ());
    }

    return false;
}

// Misdirect West & East Channelers to Main Tank
bool MagtheridonHellfireChannelerMisdirectionAction::Execute(Event event)
{
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    if (!magtheridon)
        return false;
    
    Group* group = bot->GetGroup();
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
            if (mainTank && channelerStar && channelerStar->IsAlive() && 
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
            if (mainTank && channelerCircle && channelerCircle->IsAlive() && 
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

bool MagtheridonHellfireChannelerDPSPriorityAction::Execute(Event event)
{
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    if (!magtheridon)
        return false;
    
    // Listed in order of priority
    Creature* channelerSquare   = GetChanneler(bot, SOUTH_CHANNELER);
    Creature* channelerStar = GetChanneler(bot, WEST_CHANNELER);
    Creature* channelerCircle = GetChanneler(bot, EAST_CHANNELER);
    Creature* channelerDiamond  = GetChanneler(bot, NORTHWEST_CHANNELER);
    Creature* channelerTriangle = GetChanneler(bot, NORTHEAST_CHANNELER);

    if (channelerSquare && channelerSquare->IsAlive())
    {
        Unit* rtiTarget = botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Get();
        std::string rtiValue = botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Get();
        
        if (rtiValue != "square" || rtiTarget != channelerSquare)
        {
            botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Set("square");
            botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Set(channelerSquare);
        }

        if (bot->GetVictim() != channelerSquare)
            Attack(channelerSquare);

        return false;
    }

    if (channelerStar && channelerStar->IsAlive())
    {
        Unit* rtiTarget = botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Get();
        std::string rtiValue = botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Get();
        
        if (rtiValue != "star" || rtiTarget != channelerStar)
        {
            botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Set("star");
            botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Set(channelerStar);
        }

        if (bot->GetVictim() != channelerStar)
            Attack(channelerStar);

        return false;
    }

    if (channelerCircle && channelerCircle->IsAlive())
    {
        Unit* rtiTarget = botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Get();
        std::string rtiValue = botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Get();

        if (rtiValue != "circle" || rtiTarget != channelerCircle)
        {
            botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Set("circle");
            botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Set(channelerCircle);
        }

        if (bot->GetVictim() != channelerCircle)
            Attack(channelerCircle);

        return false;
    }

    if (channelerDiamond && channelerDiamond->IsAlive())
    {
        Unit* rtiTarget = botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Get();
        std::string rtiValue = botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Get();

        if (rtiValue != "diamond" || rtiTarget != channelerDiamond)
        {
            botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Set("diamond");
            botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Set(channelerDiamond);
        }

        if (bot->GetVictim() != channelerDiamond)
            Attack(channelerDiamond);

        return false;
    }

    if (channelerTriangle && channelerTriangle->IsAlive())
    {
        Unit* rtiTarget = botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Get();
        std::string rtiValue = botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Get();

        if (rtiValue != "triangle" || rtiTarget != channelerTriangle)
        {
            botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Set("triangle");
            botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Set(channelerTriangle);
        }

        if (bot->GetVictim() != channelerTriangle)
            Attack(channelerTriangle);

        return false;
    }

    if (magtheridon && magtheridon->IsAlive() && 
        !magtheridon->HasAura(SPELL_SHADOW_CAGE) &&
        (!channelerSquare || !channelerSquare->IsAlive()) &&
        (!channelerStar || !channelerStar->IsAlive()) &&
        (!channelerCircle || !channelerCircle->IsAlive()) &&
        (!channelerDiamond || !channelerDiamond->IsAlive()) &&
        (!channelerTriangle || !channelerTriangle->IsAlive()))
    {
        Unit* rtiTarget = botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Get();
        std::string rtiValue = botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Get();

        if (rtiValue != "cross" || rtiTarget != magtheridon)
        {
            botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Set("cross");
            botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Set(magtheridon);
        }

        if (bot->GetVictim() != magtheridon)
            Attack(magtheridon);
    }

    return false;
}

// Assign Burning Abyssals to Warlocks to Banish
// Burning Abyssals in excess of Warlocks in party will be Feared
bool MagtheridonBurningAbyssalWarlockCCAction::Execute(Event event)
{
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    if (!magtheridon)
        return false;

    const GuidVector& npcs = AI_VALUE(GuidVector, "nearest hostile npcs");

    std::vector<Unit*> abyssals;
    for (const auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (unit && unit->GetEntry() == NPC_BURNING_ABYSSAL && unit->IsAlive())
            abyssals.push_back(unit);
    }

    std::vector<Player*> warlocks;
    Group* group = bot->GetGroup();
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
        if (!assignedAbyssal->HasAura(SPELL_BANISH) && botAI->CanCastSpell(SPELL_BANISH, assignedAbyssal, true))
            return botAI->CastSpell("banish", assignedAbyssal);
    }
    
    for (size_t i = warlocks.size(); i < abyssals.size(); ++i)
    {
        Unit* excessAbyssal = abyssals[i];
        if (!excessAbyssal->HasAura(SPELL_BANISH) && !excessAbyssal->HasAura(SPELL_FEAR) && 
            botAI->CanCastSpell(SPELL_FEAR, excessAbyssal, true))
            return botAI->CastSpell("fear", excessAbyssal);
    }

    return false;
}

// MT will back up to the Northern point of the room
bool MagtheridonPositionBossAction::Execute(Event event)
{
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    if (!magtheridon)
        return false;

    Group* group = bot->GetGroup();
    ObjectGuid currentIconGuid = group->GetTargetIcon(crossIcon);
    if (currentIconGuid.IsEmpty() || currentIconGuid != magtheridon->GetGUID())
        group->SetTargetIcon(crossIcon, bot->GetGUID(), magtheridon->GetGUID());

    Unit* rtiTarget = botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Get();
    std::string rtiValue = botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Get();
    if (rtiValue != "cross" || rtiTarget != magtheridon)
    {
        botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Set("cross");
        botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Set(magtheridon);
    }

    if (bot->GetVictim() != magtheridon)
        Attack(magtheridon);

    if (magtheridon->GetVictim() == bot)
    {
        const Location& position = MagtheridonsLairLocations::MagtheridonTankPosition;
        const float maxDistance = 3.0f;

        float distanceToPosition = bot->GetExactDist2d(position.x, position.y);

        if (distanceToPosition > maxDistance)
        {
            float dX = position.x - bot->GetPositionX();
            float dY = position.y - bot->GetPositionY();
            float dist = sqrt(dX * dX + dY * dY);
            float moveX = bot->GetPositionX() + (dX / dist) * maxDistance;
            float moveY = bot->GetPositionY() + (dY / dist) * maxDistance;
            
            return MoveTo(bot->GetMapId(), moveX, moveY, position.z, false, false, false, false, 
                          MovementPriority::MOVEMENT_COMBAT, true, true);
        }
        else if (!bot->IsWithinMeleeRange(magtheridon))
            return MoveTo(magtheridon->GetMapId(), magtheridon->GetPositionX(),
                          magtheridon->GetPositionY(), magtheridon->GetPositionZ());
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
    {
        return false;
    }

    const int spreadWaitSeconds = 8;
    auto it = magtheridonSpreadWaitTimer.find(bot->GetMapId());
    if (it != magtheridonSpreadWaitTimer.end())
    {
        time_t since = time(nullptr) - it->second;
        if (since < spreadWaitSeconds)
            return false;
    }

    auto cubeIt = botToCubeAssignment.find(bot->GetGUID());
    if (cubeIt != botToCubeAssignment.end())
    {
        time_t now = time(nullptr);
        auto timerIt = magtheridonBlastNovaTimer.find(bot->GetMapId());
        if (timerIt != magtheridonBlastNovaTimer.end())
        {
            time_t lastBlastNova = timerIt->second;
            if (now - lastBlastNova >= 49)
                return false;
        }
    }

    Group* group = bot->GetGroup();
    std::vector<Player*> members;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive())
            members.push_back(member);
    }

    bool isHealer = botAI->IsHeal(bot);
    const Location& center = isHealer
        ? MagtheridonsLairLocations::HealerSpreadPosition
        : MagtheridonsLairLocations::RangedSpreadPosition;
    float maxSpreadRadius = isHealer ? 15.0f : 25.0f;
    float centerX = center.x;
    float centerY = center.y;
    float centerZ = bot->GetPositionZ();
    const float radiusBuffer = 3.0f;

    if (!initialPositions.count(bot->GetGUID()))
    {
        auto it = std::find(members.begin(), members.end(), bot);
        uint32 botIndex = (it != members.end()) ? std::distance(members.begin(), it) : 0;
        uint32 count = members.size();

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
            return MoveTo(bot->GetMapId(), destX, destY, destZ);
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
            return MoveTo(bot->GetMapId(), targetX, targetY, centerZ);
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

    // 4. Let go of cubes between 0.2 to 3 seconds (randomized) after Blast Nova is interrupted
    if (bot->HasAura(SPELL_SHADOW_GRASP) &&
        !(magtheridon->HasUnitState(UNIT_STATE_CASTING) && 
          magtheridon->FindCurrentSpellBySpellId(SPELL_BLAST_NOVA)))
    {
            uint32 delay = urand(200, 3000);
            botAI->AddTimedEvent(
                [this, cube]
                {
                    botAI->Reset();
                },
                delay);
            botAI->SetNextCheckDelay(delay + 50);
            return true;
    }

    // 1. Ordinary bot behavior for the first 49s after Magtheridon actives or Blast Nova starts casting
    time_t now = time(nullptr);
    time_t lastBlastNova = magtheridonBlastNovaTimer[bot->GetMapId()];
    bool blastNovaActive = magtheridon->HasUnitState(UNIT_STATE_CASTING) &&
         magtheridon->FindCurrentSpellBySpellId(SPELL_BLAST_NOVA);

    if (now - lastBlastNova < 49)
        return false;

    // 2. After 49s, move to a safe distance from the assigned cube and wait
    if (!blastNovaActive) 
    {
        const float safeWaitDistance = 8.0f;
        float cubeDist = bot->GetExactDist2d(cubeInfo.x, cubeInfo.y);

        if (fabs(cubeDist - safeWaitDistance) > 0.5f) 
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
                    bot->InterruptNonMeleeSpells(false);
                    return MoveTo(bot->GetMapId(), targetX, targetY, targetZ, false, false, false, false, 
                                  MovementPriority::MOVEMENT_COMBAT, true, false);
                }
            }
            float angle = static_cast<float>(rand()) / RAND_MAX * 2.0f * M_PI;
            float fallbackX = cubeInfo.x + cos(angle) * safeWaitDistance;
            float fallbackY = cubeInfo.y + sin(angle) * safeWaitDistance;
            float fallbackZ = bot->GetPositionZ();

            return MoveTo(bot->GetMapId(), fallbackX, fallbackY, fallbackZ, false, false, false, false, 
                          MovementPriority::MOVEMENT_COMBAT, true, false);
        }
        else
            return true;
    }

    const float interactDistance = 1.0f;
    const float interactDistanceBuffer = 1.0f;
    float cubeDist = bot->GetExactDist2d(cubeInfo.x, cubeInfo.y);

    // 3. When Blast Nova starts casting, move to and click the assigned cube
    // Randomized delay between 0.2 to 1.5 seconds to click
    if (cubeDist > interactDistance) 
    {
        if (cubeDist <= interactDistance + interactDistanceBuffer) 
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
        bot->InterruptNonMeleeSpells(false);
        return MoveTo(bot->GetMapId(), targetX, targetY, targetZ, false, false, false, false, 
                      MovementPriority::MOVEMENT_FORCED, true, false);
    }

    return false;
}

// The Blast Nova timer resets when Magtheridon stops casting it, which is needed to ensure that bots use cubes.
// However, Magtheridon's Blast Nova cooldown actually runs from when he starts casting it. This means that if a Blast Nova 
// is not interrupted or takes too long to interrupt, the timer will be thrown off for the rest of the encounter.
// Correcting this issue is complicated and probably needs a second timer and reworked logic--I have not done so and 
// and view the current solution as sufficient since in TBC a missed Blast Nova would be a guaranteed wipe anyway.
bool MagtheridonResetTimersAndAssignmentsAction::Execute(Event event)
{
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    if (magtheridon)
    {
        bool blastNovaActive = magtheridon->HasUnitState(UNIT_STATE_CASTING) &&
                               magtheridon->FindCurrentSpellBySpellId(SPELL_BLAST_NOVA);
        bool lastBlastNova = lastBlastNovaState[magtheridon->GetMapId()];
        if (lastBlastNova && !blastNovaActive)
        {
            magtheridonBlastNovaTimer[magtheridon->GetMapId()] = time(nullptr);
        }
        lastBlastNovaState[magtheridon->GetMapId()] = blastNovaActive;

        bool lastShadowCage = lastShadowCageState[magtheridon->GetMapId()];
        bool shadowCageActive = magtheridon->HasAura(SPELL_SHADOW_CAGE);

        if (lastShadowCage && !shadowCageActive)
        {
            magtheridonSpreadWaitTimer[magtheridon->GetMapId()] = time(nullptr);
            magtheridonBlastNovaTimer[magtheridon->GetMapId()] = time(nullptr);
            magtheridonAggroWaitTimer[magtheridon->GetMapId()] = time(nullptr);
        }
        lastShadowCageState[magtheridon->GetMapId()] = shadowCageActive;
        
        if (shadowCageActive)
        {
            MagtheridonSpreadRangedAction::initialPositions.clear();
            MagtheridonSpreadRangedAction::hasReachedInitialPosition.clear();
            botToCubeAssignment.clear();
        }
        
    }

    return false;
}
