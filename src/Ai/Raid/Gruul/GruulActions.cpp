#include "GruulActions.h"
#include "GruulHelpers.h"
#include "CreatureAI.h"
#include "Playerbots.h"
#include "RaidBossHelpers.h"
#include "Unit.h"

using namespace GruulsLairHelpers;

// High King Maulgar Actions

// Main tank on Maulgar
bool HighKingMaulgarMainTankAttackMaulgarAction::Execute(Event /*event*/)
{
    Unit* maulgar = AI_VALUE2(Unit*, "find target", "high king maulgar");
    if (!maulgar)
        return false;

    MarkTargetWithSquare(bot, maulgar);
    SetRtiTarget(botAI, "square", maulgar);

    if (AI_VALUE(Unit*, "current target") != maulgar)
        return Attack(maulgar);

    if (maulgar->GetVictim() == bot)
    {
        const Position& position = MAULGAR_TANK_POSITION;
        const float distToPosition =
            bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());

        if (distToPosition > 3.0f)
        {
            const float dX = position.GetPositionX() - bot->GetPositionX();
            const float dY = position.GetPositionY() - bot->GetPositionY();
            const float moveDist = std::min(5.0f, distToPosition);
            const float moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
            const float moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

            return MoveTo(GRUULS_LAIR_MAP_ID, moveX, moveY, position.GetPositionZ(), false, false,
                          false, false, MovementPriority::MOVEMENT_COMBAT, true, true);
        }
    }

    return false;
}

// First offtank on Olm
bool HighKingMaulgarFirstAssistTankAttackOlmAction::Execute(Event /*event*/)
{
    Unit* olm = AI_VALUE2(Unit*, "find target", "olm the summoner");
    if (!olm)
        return false;

    MarkTargetWithCircle(bot, olm);
    SetRtiTarget(botAI, "circle", olm);

    if (AI_VALUE(Unit*, "current target") != olm)
        return Attack(olm);

    if (olm->GetVictim() == bot)
    {
        const Position& position = OLM_TANK_POSITION;
        const float distToPosition =
            bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());

        if (distToPosition > 3.0f)
        {
            const float dX = position.GetPositionX() - bot->GetPositionX();
            const float dY = position.GetPositionY() - bot->GetPositionY();
            const float moveDist = std::min(5.0f, distToPosition);
            const float moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
            const float moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

            return MoveTo(GRUULS_LAIR_MAP_ID, moveX, moveY, position.GetPositionZ(), false, false,
                          false, false, MovementPriority::MOVEMENT_COMBAT, true, true);
        }
    }

    return false;
}

// Second offtank on Blindeye
bool HighKingMaulgarSecondAssistTankAttackBlindeyeAction::Execute(Event /*event*/)
{
    Unit* blindeye = AI_VALUE2(Unit*, "find target", "blindeye the seer");
    if (!blindeye)
        return false;

    MarkTargetWithStar(bot, blindeye);
    SetRtiTarget(botAI, "star", blindeye);

    if (AI_VALUE(Unit*, "current target") != blindeye)
        return Attack(blindeye);

    if (blindeye->GetVictim() == bot)
    {
        const Position& position = BLINDEYE_TANK_POSITION;
        const float distToPosition =
            bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());

        if (distToPosition > 3.0f)
        {
            const float dX = position.GetPositionX() - bot->GetPositionX();
            const float dY = position.GetPositionY() - bot->GetPositionY();
            const float moveDist = std::min(5.0f, distToPosition);
            const float moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
            const float moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

            return MoveTo(GRUULS_LAIR_MAP_ID, moveX, moveY, position.GetPositionZ(), false, false,
                          false, false, MovementPriority::MOVEMENT_COMBAT, true, true);
        }
    }

    return false;
}

// Mage with highest max HP on Krosh
bool HighKingMaulgarMageTankAttackKroshAction::Execute(Event /*event*/)
{
    Unit* krosh = AI_VALUE2(Unit*, "find target", "krosh firehand");
    if (!krosh)
        return false;

    MarkTargetWithTriangle(bot, krosh);
    SetRtiTarget(botAI, "triangle", krosh);

    if (krosh->HasAura(static_cast<uint32>(GruulsLairSpells::SPELL_SPELL_SHIELD)) &&
        botAI->CanCastSpell("spellsteal", krosh))
    {
        return botAI->CastSpell("spellsteal", krosh);
    }

    if (!bot->HasAura(static_cast<uint32>(GruulsLairSpells::SPELL_SPELL_SHIELD)) &&
        botAI->CanCastSpell("fire ward", bot))
    {
        return botAI->CastSpell("fire ward", bot);
    }

    if (AI_VALUE(Unit*, "current target") != krosh)
        return Attack(krosh);

    if (krosh->GetVictim() == bot)
    {
        const Position& position = KROSH_TANK_POSITION;
        const float distanceKroshToPosition =
            krosh->GetExactDist2d(position.GetPositionX(), position.GetPositionY());
        constexpr float minDistance = 17.0f;
        constexpr float maxDistance = 30.0f;

        if (distanceKroshToPosition > minDistance && distanceKroshToPosition < maxDistance &&
            bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY()) > 1.0f)
        {
            return MoveTo(GRUULS_LAIR_MAP_ID, position.GetPositionX(), position.GetPositionY(),
                          position.GetPositionZ(), false, false, false, false,
                          MovementPriority::MOVEMENT_COMBAT, true, false);
        }
        else
        {
            constexpr float safeDistance = 15.0f;
            const float currentDistance = bot->GetDistance2d(krosh);

            if (currentDistance < safeDistance)
            {
                botAI->InterruptSpell();
                return MoveAway(krosh, safeDistance - currentDistance);
            }
        }
    }

    return false;
}

// Moonkin with highest max HP on Kiggler
bool HighKingMaulgarMoonkinTankAttackKigglerAction::Execute(Event /*event*/)
{
    Unit* kiggler = AI_VALUE2(Unit*, "find target", "kiggler the crazed");
    if (!kiggler)
        return false;

    MarkTargetWithDiamond(bot, kiggler);
    SetRtiTarget(botAI, "diamond", kiggler);

    if (AI_VALUE(Unit*, "current target") != kiggler)
        return Attack(kiggler);

    if (kiggler->GetVictim() == bot)
    {
        constexpr float safeDistance = 28.5f;
        const float currentDistance = bot->GetDistance2d(kiggler);

        if (currentDistance < safeDistance)
        {
            botAI->InterruptSpell();
            return MoveAway(kiggler, safeDistance - currentDistance);
        }
    }

    return false;
}

bool HighKingMaulgarAssignDPSPriorityAction::Execute(Event /*event*/)
{
    // Target priority 1: Blindeye
    Unit* blindeye = AI_VALUE2(Unit*, "find target", "blindeye the seer");
    if (blindeye)
    {
        MarkTargetWithStar(bot, blindeye);
        SetRtiTarget(botAI, "star", blindeye);

        if (AI_VALUE(Unit*, "current target") != blindeye)
            return Attack(blindeye);

        return false;
    }

    // Target priority 2: Olm
    Unit* olm = AI_VALUE2(Unit*, "find target", "olm the summoner");
    if (olm)
    {
        MarkTargetWithCircle(bot, olm);
        SetRtiTarget(botAI, "circle", olm);

        if (AI_VALUE(Unit*, "current target") != olm)
            return Attack(olm);

        return false;
    }

    // Target priority 3a: Krosh (ranged only)
    Unit* krosh = AI_VALUE2(Unit*, "find target", "krosh firehand");
    if (krosh && botAI->IsRanged(bot))
    {
        MarkTargetWithTriangle(bot, krosh);
        SetRtiTarget(botAI, "triangle", krosh);

        if (AI_VALUE(Unit*, "current target") != krosh)
            return Attack(krosh);

        return false;
    }

    // Target priority 3b: Kiggler
    Unit* kiggler = AI_VALUE2(Unit*, "find target", "kiggler the crazed");
    if (kiggler)
    {
        MarkTargetWithDiamond(bot, kiggler);
        SetRtiTarget(botAI, "diamond", kiggler);

        if (AI_VALUE(Unit*, "current target") != kiggler)
            return Attack(kiggler);

        return false;
    }

    // Target priority 4: Maulgar
    Unit* maulgar = AI_VALUE2(Unit*, "find target", "high king maulgar");
    if (maulgar)
    {
        MarkTargetWithSquare(bot, maulgar);
        SetRtiTarget(botAI, "square", maulgar);

        if (AI_VALUE(Unit*, "current target") != maulgar)
            return Attack(maulgar);
    }

    return false;
}

bool HighKingMaulgarRunAwayFromWhirlwindAction::Execute(Event /*event*/)
{
    Unit* maulgar = AI_VALUE2(Unit*, "find target", "high king maulgar");
    if (!maulgar)
        return false;

    constexpr float safeDistance = 8.0f;
    const float currentDistance = bot->GetDistance2d(maulgar);

    if (currentDistance < safeDistance)
    {
        botAI->InterruptSpell();
        return MoveAway(maulgar, safeDistance - currentDistance);
    }

    return false;
}

bool HighKingMaulgarMoveAwayFromBlastNovaDangerAction::Execute(Event /*event*/)
{
    Unit* krosh = AI_VALUE2(Unit*, "find target", "krosh firehand");
    if (!krosh)
        return false;

    constexpr float safeDistance = 20.0f;
    constexpr uint32 minInterval = 1000;
    const float currentDistance = bot->GetDistance2d(krosh);

    if (currentDistance < safeDistance)
    {
        botAI->InterruptSpell();
        return FleePosition(krosh->GetPosition(), safeDistance, minInterval);
    }

    return false;
}

bool HighKingMaulgarBanishFelStalkerAction::Execute(Event /*event*/)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    std::vector<Unit*> felStalkers;
    std::list<Creature*> creatureList;
    constexpr float searchRadius = 50.0f;
    bot->GetCreatureListWithEntryInGrid(
        creatureList, static_cast<uint32>(GruulsLairNpcs::NPC_WILD_FEL_STALKER), searchRadius);

    for (Creature* creature : creatureList)
    {
        if (creature && creature->IsAlive())
            felStalkers.push_back(creature);
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

    const uint8 felStalkersSize = felStalkers.size();

    if (warlockIndex >= 0 && warlockIndex < felStalkersSize)
    {
        Unit* assignedFelStalker = felStalkers[warlockIndex];
        if (!botAI->HasAura("banish", assignedFelStalker) &&
            botAI->CanCastSpell("banish", assignedFelStalker))
        {
            return botAI->CastSpell("banish", assignedFelStalker);
        }
    }

    return false;
}

// Hunter 1: Misdirect Blindeye to second offtank
// Hunter 2: Misdirect Olm to first offtank
// Hunter 3: Misdirect Kiggler to Moonkin tank
// Hunter 4: Misdirect Krosh to Mage tank
bool HighKingMaulgarMisdirectOgresToTanksAction::Execute(Event /*event*/)
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

        if (hunters.size() >= 4)
            break;
    }

    int8 hunterIndex = -1;
    for (size_t i = 0; i < hunters.size(); ++i)
    {
        if (hunters[i] == bot)
        {
            hunterIndex = static_cast<int8>(i);
            break;
        }
    }
    if (hunterIndex == -1)
        return false;

    Unit* ogreTarget = nullptr;
    Player* tankTarget = nullptr;
    if (hunterIndex == 0)
    {
        ogreTarget = AI_VALUE2(Unit*, "find target", "blindeye the seer");
        tankTarget = GetGroupAssistTank(botAI, bot, 1);
    }
    else if (hunterIndex == 1)
    {
        ogreTarget = AI_VALUE2(Unit*, "find target", "olm the summoner");
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            if (Player* member = GetGroupAssistTank(botAI, bot, 0))
            {
                tankTarget = member;
                break;
            }
        }
    }
    else if (hunterIndex == 2)
    {
        ogreTarget = AI_VALUE2(Unit*, "find target", "kiggler the crazed");
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            if (Player* member = GetKigglerMoonkinTank(bot))
            {
                tankTarget = member;
                break;
            }
        }
    }
    else if (hunterIndex == 3)
    {
        ogreTarget = AI_VALUE2(Unit*, "find target", "krosh firehand");
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            if (Player* member = GetKroshMageTank(bot))
            {
                tankTarget = member;
                break;
            }
        }
    }

    if (!ogreTarget || !tankTarget || !tankTarget->IsAlive())
        return false;

    if (botAI->CanCastSpell("misdirection", tankTarget))
        return botAI->CastSpell("misdirection", tankTarget);

    if (bot->HasAura(static_cast<uint32>(GruulsLairSpells::SPELL_MISDIRECTION)) &&
        botAI->CanCastSpell("steady shot", ogreTarget))
    {
        return botAI->CastSpell("steady shot", ogreTarget);
    }

    return false;
}

// Gruul the Dragonkiller Actions

// Position in center of the room
bool GruulTheDragonkillerTanksPositionBossAction::Execute(Event /*event*/)
{
    Unit* gruul = AI_VALUE2(Unit*, "find target", "gruul the dragonkiller");
    if (!gruul)
        return false;

    if (AI_VALUE(Unit*, "current target") != gruul)
        return Attack(gruul);

    if (gruul->GetVictim() == bot)
    {
        const Position& position = GRUUL_TANK_POSITION;
        const float distToPosition =
            bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());

        if (distToPosition > 3.0f)
        {
            const float dX = position.GetPositionX() - bot->GetPositionX();
            const float dY = position.GetPositionY() - bot->GetPositionY();
            const float moveDist = std::min(5.0f, distToPosition);
            const float moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
            const float moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

            return MoveTo(GRUULS_LAIR_MAP_ID, moveX, moveY, position.GetPositionZ(), false, false,
                          false, false, MovementPriority::MOVEMENT_COMBAT, true, true);
        }
    }

    return false;
}

// Ranged will take initial positions around the middle of the room, 25-40 yards from center
// Ranged should spread out 10 yards from each other
bool GruulTheDragonkillerSpreadRangedAction::Execute(Event /*event*/)
{
    Unit* gruul = AI_VALUE2(Unit*, "find target", "gruul the dragonkiller");
    if (!gruul)
        return false;

    if (gruul->GetHealth() == gruul->GetMaxHealth())
        _hasReachedInitialPosition = false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    std::vector<Player*> members;
    Player* closestMember = nullptr;
    float closestDist = std::numeric_limits<float>::max();
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive())
            continue;

        members.push_back(member);

        if (member != bot)
        {
            float dist = bot->GetExactDist2d(member);
            if (dist < closestDist)
            {
                closestDist = dist;
                closestMember = member;
            }
        }
    }

    const Position& position = GRUUL_TANK_POSITION;

    if (_initialPosition.GetPositionX() == 0.0f && _initialPosition.GetPositionY() == 0.0f)
    {
        auto it = std::find(members.begin(), members.end(), bot);
        uint8 botIndex = (it != members.end()) ? std::distance(members.begin(), it) : 0;
        uint8 count = members.size();

        constexpr float minRadius = 25.0f;
        constexpr float maxRadius = 40.0f;
        float angle = 2 * M_PI * botIndex / count;
        float radius = minRadius + static_cast<float>(rand()) /
                       static_cast<float>(RAND_MAX) * (maxRadius - minRadius);
        float targetX = position.GetPositionX() + radius * cos(angle);
        float targetY = position.GetPositionY() + radius * sin(angle);

        _initialPosition = Position(targetX, targetY, position.GetPositionZ());
    }

    if (!_hasReachedInitialPosition)
    {
        const float distanceToTarget =
            bot->GetExactDist2d(_initialPosition.GetPositionX(), _initialPosition.GetPositionY());
        if (distanceToTarget > 2.0f)
        {
            float destX = _initialPosition.GetPositionX();
            float destY = _initialPosition.GetPositionY();
            float destZ = _initialPosition.GetPositionZ();

            if (!bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bot->GetPositionX(),
                bot->GetPositionY(), bot->GetPositionZ(), destX, destY, destZ))
            {
                return false;
            }

            return MoveTo(GRUULS_LAIR_MAP_ID, destX, destY, destZ, false, false, false,
                          false, MovementPriority::MOVEMENT_COMBAT, true, false);
        }

        _hasReachedInitialPosition = true;
    }
    else
    {
        constexpr float minSpreadDistance = 10.0f;
        constexpr uint32 minInterval = 1000;
        if (closestMember && closestDist < minSpreadDistance)
            return FleePosition(closestMember->GetPosition(), minSpreadDistance, minInterval);
    }

    return false;
}

// Try to get away from other group members when Ground Slam is cast
bool GruulTheDragonkillerShatterSpreadAction::Execute(Event /*event*/)
{
    constexpr float safeDistance = 10.0f;
    constexpr uint32 minInterval = 0;
    if (Unit* nearestPlayer = GetNearestPlayerInRadius(bot, safeDistance))
        return FleePosition(nearestPlayer->GetPosition(), safeDistance, minInterval);

    return false;
}
