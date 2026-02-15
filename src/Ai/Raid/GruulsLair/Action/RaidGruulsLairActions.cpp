#include "RaidGruulsLairActions.h"
#include "RaidGruulsLairHelpers.h"
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

    if (bot->GetVictim() != maulgar)
        return Attack(maulgar);

    if (maulgar->GetVictim() == bot)
    {
        const Position& position = MAULGAR_TANK_POSITION;
        const float maxDistance = 3.0f;

        float distanceToPosition = bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());

        if (distanceToPosition > maxDistance)
        {
            float dX = position.GetPositionX() - bot->GetPositionX();
            float dY = position.GetPositionY() - bot->GetPositionY();
            float moveX = bot->GetPositionX() + (dX / distanceToPosition) * maxDistance;
            float moveY = bot->GetPositionY() + (dY / distanceToPosition) * maxDistance;
            return MoveTo(GRUULS_LAIR_MAP_ID, moveX, moveY, position.GetPositionZ(), false, false, false, false,
                          MovementPriority::MOVEMENT_COMBAT, true, true);
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

    if (bot->GetVictim() != olm)
        return Attack(olm);

    if (olm->GetVictim() == bot)
    {
        const Position& position = OLM_TANK_POSITION;
        const float maxDistance = 3.0f;
        const float olmTankLeeway = 30.0f;

        float distanceOlmToPosition = olm->GetExactDist2d(position.GetPositionX(), position.GetPositionY());

        if (distanceOlmToPosition > olmTankLeeway)
        {
            float dX = position.GetPositionX() - bot->GetPositionX();
            float dY = position.GetPositionY() - bot->GetPositionY();
            float moveX = bot->GetPositionX() + (dX / distanceOlmToPosition) * maxDistance;
            float moveY = bot->GetPositionY() + (dY / distanceOlmToPosition) * maxDistance;
            return MoveTo(GRUULS_LAIR_MAP_ID, moveX, moveY, position.GetPositionZ(), false, false, false, false,
                          MovementPriority::MOVEMENT_COMBAT, true, false);
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

    if (bot->GetVictim() != blindeye)
        return Attack(blindeye);

    if (blindeye->GetVictim() == bot)
    {
        const Position& position = BLINDEYE_TANK_POSITION;
        const float maxDistance = 3.0f;

        float distanceToPosition = bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());

        if (distanceToPosition > maxDistance)
        {
            float dX = position.GetPositionX() - bot->GetPositionX();
            float dY = position.GetPositionY() - bot->GetPositionY();
            float moveX = bot->GetPositionX() + (dX / distanceToPosition) * maxDistance;
            float moveY = bot->GetPositionY() + (dY / distanceToPosition) * maxDistance;
            return MoveTo(GRUULS_LAIR_MAP_ID, moveX, moveY, position.GetPositionZ(), false, false, false, false,
                          MovementPriority::MOVEMENT_COMBAT, true, false);
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

    if (krosh->HasAura(SPELL_SPELL_SHIELD) && botAI->CanCastSpell("spellsteal", krosh))
        return botAI->CastSpell("spellsteal", krosh);

    if (!bot->HasAura(SPELL_SPELL_SHIELD) && botAI->CanCastSpell("fire ward", bot))
        return botAI->CastSpell("fire ward", bot);

    if (bot->GetTarget() != krosh->GetGUID())
        return Attack(krosh);

    if (krosh->GetVictim() == bot)
    {
        const Position& position = KROSH_TANK_POSITION;
        float distanceToKrosh = krosh->GetExactDist2d(position.GetPositionX(), position.GetPositionY());
        const float minDistance = 16.0f;
        const float maxDistance = 29.0f;
        const float tankPositionLeeway = 1.0f;

        if (distanceToKrosh > minDistance && distanceToKrosh < maxDistance)
        {
            if (!bot->IsWithinDist2d(position.GetPositionX(), position.GetPositionY(), tankPositionLeeway))
            {
                return MoveTo(GRUULS_LAIR_MAP_ID, position.GetPositionX(), position.GetPositionY(), position.GetPositionZ(),
                              false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
            }

            float orientation = atan2(krosh->GetPositionY() - bot->GetPositionY(),
                                      krosh->GetPositionX() - bot->GetPositionX());
            bot->SetFacingTo(orientation);
        }
        else
        {
            Position safePos;
            if (TryGetNewSafePosition(botAI, bot, safePos))
            {
                return MoveTo(GRUULS_LAIR_MAP_ID, safePos.GetPositionX(), safePos.GetPositionY(), safePos.GetPositionZ(),
                              false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
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

    if (bot->GetTarget() != kiggler->GetGUID())
        return Attack(kiggler);

    Position safePos;
    if (TryGetNewSafePosition(botAI, bot, safePos))
    {
        return MoveTo(GRUULS_LAIR_MAP_ID, safePos.GetPositionX(), safePos.GetPositionY(), safePos.GetPositionZ(),
                      false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    return false;
}

bool HighKingMaulgarAssignDPSPriorityAction::Execute(Event /*event*/)
{
    // Target priority 1: Blindeye
    Unit* blindeye = AI_VALUE2(Unit*, "find target", "blindeye the seer");
    if (blindeye)
    {
        Position safePos;
        if (TryGetNewSafePosition(botAI, bot, safePos))
        {
            bot->AttackStop();
            bot->InterruptNonMeleeSpells(false);
            return MoveTo(GRUULS_LAIR_MAP_ID, safePos.GetPositionX(), safePos.GetPositionY(), safePos.GetPositionZ(),
                          false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
        }

        SetRtiTarget(botAI, "star", blindeye);

        if (bot->GetTarget() != blindeye->GetGUID())
            return Attack(blindeye);

        return false;
    }

    // Target priority 2: Olm
    Unit* olm = AI_VALUE2(Unit*, "find target", "olm the summoner");
    if (olm)
    {
        Position safePos;
        if (TryGetNewSafePosition(botAI, bot, safePos))
        {
            bot->AttackStop();
            bot->InterruptNonMeleeSpells(false);
            return MoveTo(GRUULS_LAIR_MAP_ID, safePos.GetPositionX(), safePos.GetPositionY(), safePos.GetPositionZ(),
                          false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
        }

        SetRtiTarget(botAI, "circle", olm);

        if (bot->GetTarget() != olm->GetGUID())
            return Attack(olm);

        return false;
    }

    // Target priority 3a: Krosh (ranged only)
    Unit* krosh = AI_VALUE2(Unit*, "find target", "krosh firehand");
    if (krosh && botAI->IsRanged(bot))
    {
        Position safePos;
        if (TryGetNewSafePosition(botAI, bot, safePos))
        {
            bot->AttackStop();
            bot->InterruptNonMeleeSpells(false);
            return MoveTo(GRUULS_LAIR_MAP_ID, safePos.GetPositionX(), safePos.GetPositionY(), safePos.GetPositionZ(),
                          false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
        }

        SetRtiTarget(botAI, "triangle", krosh);

        if (bot->GetTarget() != krosh->GetGUID())
            return Attack(krosh);

        return false;
    }

    // Target priority 3b: Kiggler
    Unit* kiggler = AI_VALUE2(Unit*, "find target", "kiggler the crazed");
    if (kiggler)
    {
        Position safePos;
        if (TryGetNewSafePosition(botAI, bot, safePos))
        {
            bot->AttackStop();
            bot->InterruptNonMeleeSpells(false);
            return MoveTo(GRUULS_LAIR_MAP_ID, safePos.GetPositionX(), safePos.GetPositionY(), safePos.GetPositionZ(),
                          false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
        }

        SetRtiTarget(botAI, "diamond", kiggler);

        if (bot->GetTarget() != kiggler->GetGUID())
            return Attack(kiggler);

        return false;
    }

    // Target priority 4: Maulgar
    Unit* maulgar = AI_VALUE2(Unit*, "find target", "high king maulgar");
    if (maulgar)
    {
        Position safePos;
        if (TryGetNewSafePosition(botAI, bot, safePos))
        {
            bot->AttackStop();
            bot->InterruptNonMeleeSpells(false);
            return MoveTo(GRUULS_LAIR_MAP_ID, safePos.GetPositionX(), safePos.GetPositionY(), safePos.GetPositionZ(),
                          false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
        }

        SetRtiTarget(botAI, "square", maulgar);

        if (bot->GetTarget() != maulgar->GetGUID())
            return Attack(maulgar);
    }

    return false;
}

// Avoid Whirlwind and Blast Wave and generally try to stay near the center of the room
bool HighKingMaulgarHealerFindSafePositionAction::Execute(Event /*event*/)
{
    const Position& center = MAULGAR_ROOM_CENTER;
    const float maxDistanceFromCenter = 50.0f;
    float distToCenter = bot->GetExactDist2d(center.GetPositionX(), center.GetPositionY());

    if (distToCenter > maxDistanceFromCenter)
    {
        float angle = atan2(bot->GetPositionY() - center.GetPositionY(), bot->GetPositionX() - center.GetPositionX());
        float destX = center.GetPositionX() + 40.0f * cos(angle);
        float destY = center.GetPositionY() + 40.0f * sin(angle);
        float destZ = center.GetPositionZ();

        if (!bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bot->GetPositionX(), bot->GetPositionY(),
            bot->GetPositionZ(), destX, destY, destZ))
            return false;

        return MoveTo(GRUULS_LAIR_MAP_ID, destX, destY, destZ, false, false, false, false,
                      MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    Position safePos;
    if (TryGetNewSafePosition(botAI, bot, safePos))
    {
        bot->AttackStop();
        bot->InterruptNonMeleeSpells(false);
        return MoveTo(GRUULS_LAIR_MAP_ID, safePos.GetPositionX(), safePos.GetPositionY(), safePos.GetPositionZ(),
                      false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    return false;
}

// Run away from Maulgar during Whirlwind (logic for after all other ogres are dead)
bool HighKingMaulgarRunAwayFromWhirlwindAction::Execute(Event /*event*/)
{
    Unit* maulgar = AI_VALUE2(Unit*, "find target", "high king maulgar");
    if (!maulgar)
        return false;

    const float safeDistance = 10.0f;
    float distance = bot->GetExactDist2d(maulgar);

    if (distance < safeDistance)
    {
        float angle = atan2(bot->GetPositionY() - maulgar->GetPositionY(),
                            bot->GetPositionX() - maulgar->GetPositionX());
        float destX = maulgar->GetPositionX() + safeDistance * cos(angle);
        float destY = maulgar->GetPositionY() + safeDistance * sin(angle);
        float destZ = bot->GetPositionZ();

        if (!bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bot->GetPositionX(), bot->GetPositionY(),
            bot->GetPositionZ(), destX, destY, destZ))
            return false;

        float destDist = maulgar->GetExactDist2d(destX, destY);

        if (destDist >= safeDistance - 0.1f)
        {
            bot->AttackStop();
            bot->InterruptNonMeleeSpells(true);
            return MoveTo(GRUULS_LAIR_MAP_ID, destX, destY, destZ, false, false, false, false,
                          MovementPriority::MOVEMENT_COMBAT, true, false);
        }
    }

    return false;
}

bool HighKingMaulgarBanishFelstalkerAction::Execute(Event /*event*/)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    const GuidVector& npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    std::vector<Unit*> felStalkers;
    for (auto const& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (unit && unit->GetEntry() == NPC_WILD_FEL_STALKER && unit->IsAlive())
            felStalkers.push_back(unit);
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

    if (warlockIndex >= 0 && warlockIndex < felStalkers.size())
    {
        Unit* assignedFelStalker = felStalkers[warlockIndex];
        if (!botAI->HasAura("banish", assignedFelStalker) && botAI->CanCastSpell("banish", assignedFelStalker))
            return botAI->CastSpell("banish", assignedFelStalker);
    }

    return false;
}

// Hunter 1: Misdirect Olm to first offtank and have pet attack Blindeye
// Hunter 2: Misdirect Blindeye to second offtank
bool HighKingMaulgarMisdirectOlmAndBlindeyeAction::Execute(Event /*event*/)
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

    Unit* olm = AI_VALUE2(Unit*, "find target", "olm the summoner");
    Unit* blindeye = AI_VALUE2(Unit*, "find target", "blindeye the seer");
    Player* olmTank = nullptr;
    Player* blindeyeTank = nullptr;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive())
            continue;
        else if (botAI->IsAssistTankOfIndex(member, 0)) olmTank = member;
        else if (botAI->IsAssistTankOfIndex(member, 1)) blindeyeTank = member;
    }

    switch (hunterIndex)
    {
    case 0:
        botAI->CastSpell("misdirection", olmTank);
        if (bot->HasAura(SPELL_MISDIRECTION))
        {
            Pet* pet = bot->GetPet();
            if (pet && pet->IsAlive() && pet->GetVictim() != blindeye)
            {
                pet->ClearUnitState(UNIT_STATE_FOLLOW);
                pet->AttackStop();
                pet->SetTarget(blindeye->GetGUID());
                if (pet->GetCharmInfo())
                {
                    pet->GetCharmInfo()->SetIsCommandAttack(true);
                    pet->GetCharmInfo()->SetIsAtStay(false);
                    pet->GetCharmInfo()->SetIsFollowing(false);
                    pet->GetCharmInfo()->SetIsCommandFollow(false);
                    pet->GetCharmInfo()->SetIsReturning(false);
                }
                pet->ToCreature()->AI()->AttackStart(blindeye);
            }
            return botAI->CastSpell("steady shot", olm);
        }
        break;

    case 1:
        botAI->CastSpell("misdirection", blindeyeTank);
        if (bot->HasAura(SPELL_MISDIRECTION))
            return botAI->CastSpell("steady shot", blindeye);
        break;

    default:
        break;
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

    if (bot->GetVictim() != gruul)
        return Attack(gruul);

    if (gruul->GetVictim() == bot)
    {
        const Position& position = GRUUL_TANK_POSITION;
        const float maxDistance = 5.0f;

        float dX = position.GetPositionX() - bot->GetPositionX();
        float dY = position.GetPositionY() - bot->GetPositionY();
        float distanceToTankPosition = bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());

        if (distanceToTankPosition > maxDistance)
        {
            float step = std::min(maxDistance, distanceToTankPosition);
            float moveX = bot->GetPositionX() + (dX / distanceToTankPosition) * maxDistance;
            float moveY = bot->GetPositionY() + (dY / distanceToTankPosition) * maxDistance;
            const float moveZ = position.GetPositionZ();
            return MoveTo(GRUULS_LAIR_MAP_ID, moveX, moveY, moveZ, false, false, false, false,
                          MovementPriority::MOVEMENT_COMBAT, true, false);
        }
    }

    return false;
}

// Ranged will take initial positions around the middle of the room, 25-40 yards from center
// Ranged should spread out 10 yards from each other
bool GruulTheDragonkillerSpreadRangedAction::Execute(Event /*event*/)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    static std::unordered_map<ObjectGuid, Position> initialPositions;
    static std::unordered_map<ObjectGuid, bool> hasReachedInitialPosition;

    Unit* gruul = AI_VALUE2(Unit*, "find target", "gruul the dragonkiller");
    if (gruul && gruul->GetHealth() == gruul->GetMaxHealth())
    {
        initialPositions.erase(bot->GetGUID());
        hasReachedInitialPosition.erase(bot->GetGUID());
    }

    const Position& position = GRUUL_TANK_POSITION;
    const float centerX = position.GetPositionX();
    const float centerY = position.GetPositionY();
    const float centerZ = position.GetPositionZ();
    const float minRadius = 25.0f;
    const float maxRadius = 40.0f;

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

    if (!initialPositions.count(bot->GetGUID()))
    {
        auto it = std::find(members.begin(), members.end(), bot);
        uint8 botIndex = (it != members.end()) ? std::distance(members.begin(), it) : 0;
        uint8 count = members.size();

        float angle = 2 * M_PI * botIndex / count;
        float radius = minRadius + static_cast<float>(rand()) /
                       static_cast<float>(RAND_MAX) * (maxRadius - minRadius);
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

            return MoveTo(GRUULS_LAIR_MAP_ID, destX, destY, destZ, false, false, false, false,
                          MovementPriority::MOVEMENT_COMBAT, true, false);
        }

        hasReachedInitialPosition[bot->GetGUID()] = true;
    }

    const float minSpreadDistance = 10.0f;
    const float movementThreshold = 2.0f;

    if (closestMember && closestDist < minSpreadDistance - movementThreshold)
    {
        return FleePosition(Position(closestMember->GetPositionX(), closestMember->GetPositionY(),
                            closestMember->GetPositionZ()), minSpreadDistance, 0);
    }

    return false;
}

// Try to get away from other group members when Ground Slam is cast
bool GruulTheDragonkillerShatterSpreadAction::Execute(Event /*event*/)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    GuidVector members = AI_VALUE(GuidVector, "group members");
    Unit* closestMember = nullptr;
    float closestDist = std::numeric_limits<float>::max();

    for (auto& member : members)
    {
        Unit* unit = botAI->GetUnit(member);
        if (!unit || bot->GetGUID() == member)
            continue;

        const float dist = bot->GetExactDist2d(unit);
        if (dist < closestDist)
        {
            closestDist = dist;
            closestMember = unit;
        }
    }

    if (closestMember)
    {
        return FleePosition(Position(closestMember->GetPositionX(), closestMember->GetPositionY(),
                            closestMember->GetPositionZ()), 6.0f, 0);
    }

    return false;
}
