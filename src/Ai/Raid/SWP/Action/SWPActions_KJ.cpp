/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include <array>
#include <map>
#include <vector>

#include "SWPActions.h"
#include "SWPEncounter_KJ.h"
#include "Playerbots.h"
#include "PlayerbotTextMgr.h"
#include "RaidBossHelpers.h"
#include "RtiTargetValue.h"

using namespace SunwellHelpers;

bool KiljaedenAnnounceDragonOrbUserAction::Execute(Event /*event*/)
{
    const uint32 instanceId = bot->GetInstanceId();
    auto const stateItr = kiljaedenEncounterStates.find(instanceId);

    if (stateItr == kiljaedenEncounterStates.end() || !stateItr->second.dragonOrbAnnouncementMs)
    {
        kiljaedenEncounterStates[instanceId].dragonOrbAnnouncementMs = getMSTime();

        Player* orbUser = GetKiljaedenDragonOrbUser(bot);
        std::string text;

        if (orbUser)
        {
            std::map<std::string, std::string> placeholders = {
                { "%bot", orbUser->GetName() }
            };
            text = PlayerbotTextMgr::instance().GetBotTextOrDefault(
                "kiljaeden_designated_dragon_orb_user",
                "%bot is the first assistant and the designated dragon orb user!",
                placeholders);
        }
        else
        {
            text = PlayerbotTextMgr::instance().GetBotTextOrDefault(
                "kiljaeden_no_designated_dragon_orb_user",
                "No bot has been assigned as the designated dragon orb user, "
                "and therefore a player must control the dragons. "
                "If you would like a bot to use the dragon orbs, "
                "please set the assistant flag for a bot.",
                {}
            );
        }

        return botAI->SayToRaid(text);
    }

    return false;
}


bool KiljaedenMarkAndPrioritizeHandsOfTheDeceiverAction::Execute(Event /*event*/)
{
    Player* mainTank = GetGroupMainTank(botAI, bot);
    Player* firstAssistTank = GetGroupAssistTank(botAI, bot, 0);
    Player* secondAssistTank = GetGroupAssistTank(botAI, bot, 1);
    if (!mainTank || !firstAssistTank || !secondAssistTank)
        return false;

    if (botAI->IsTank(bot))
    {
        std::vector<Player*> const tanks = { mainTank, firstAssistTank, secondAssistTank };

        size_t myIndex = tanks.size();
        for (size_t i = 0; i < tanks.size(); ++i)
        {
            if (bot == tanks[i])
            {
                myIndex = i;
                break;
            }
        }

        if (myIndex >= tanks.size())
            return false;

        std::vector<Unit*> hands;
        auto const& attackers =
            botAI->GetAiObjectContext()->GetValue<GuidVector>("possible targets no los")->Get();

        for (ObjectGuid const guid : attackers)
        {
            Unit* unit = botAI->GetUnit(guid);
            if (unit && unit->IsAlive() &&
                unit->GetEntry() == static_cast<uint32>(SunwellNpcs::NPC_HAND_OF_THE_DECEIVER))
            {
                hands.push_back(unit);
            }
        }

        AssignHandsToTanks(hands, myIndex);

        ObjectGuid const assignedGuid =
            kiljaedenHandTankAssignments[bot->GetInstanceId()][myIndex];
        if (assignedGuid.IsEmpty())
            return false;

        Unit* assignedHand = botAI->GetUnit(assignedGuid);
        if (!assignedHand || !assignedHand->IsAlive())
            return false;

        if (AI_VALUE(Unit*, "current target") != assignedHand)
            return Attack(assignedHand);

        if (assignedHand->GetVictim() == bot && !assignedHand->HasUnitState(UNIT_STATE_STUNNED))
        {
            constexpr float minTankDistance = 15.0f;

            for (size_t i = 0; i < tanks.size(); ++i)
            {
                if (i == myIndex)
                    continue;

                Player* otherTank = tanks[i];
                if (!otherTank || !otherTank->IsAlive())
                    continue;

                ObjectGuid const otherGuid =
                    kiljaedenHandTankAssignments[bot->GetInstanceId()][i];
                if (otherGuid.IsEmpty())
                    continue;

                Unit* otherHand = botAI->GetUnit(otherGuid);
                if (!otherHand || !otherHand->IsAlive())
                    continue;

                float const distFromTank = bot->GetExactDist2d(otherTank);
                if (distFromTank < minTankDistance)
                    return MoveAway(otherTank, minTankDistance - distFromTank, true);
            }
        }

        return false;
    }

    if (botAI->IsDps(bot))
        return DpsAttackPriorityTargets();

    return false;
}

void KiljaedenMarkAndPrioritizeHandsOfTheDeceiverAction::AssignHandsToTanks(
    std::vector<Unit*> const& hands, size_t const myIndex)
{
    std::vector<uint8> const rtiIndices = {
        RtiTargetValue::starIndex,
        RtiTargetValue::circleIndex,
        RtiTargetValue::diamondIndex
    };
    std::vector<std::string> const rtiNames = { "star", "circle", "diamond" };

    auto& assignments = kiljaedenHandTankAssignments[bot->GetInstanceId()];
    ObjectGuid& assignedGuid = assignments[myIndex];

    if (!assignedGuid.IsEmpty())
    {
        for (Unit* hand : hands)
        {
            if (hand->GetGUID() == assignedGuid)
                return;
        }

        assignedGuid = ObjectGuid::Empty;
        return;
    }

    if (myIndex < hands.size())
    {
        Unit* hand = hands[myIndex];
        assignedGuid = hand->GetGUID();

        MarkTargetWithIcon(bot, hand, rtiIndices[myIndex]);
        SetRtiTarget(botAI, rtiNames[myIndex], hand);
    }
}

bool KiljaedenMarkAndPrioritizeHandsOfTheDeceiverAction::DpsAttackPriorityTargets()
{
    std::vector<std::string> const rtiNames = { "star", "circle", "diamond" };

    for (std::string const& rtiName : rtiNames)
    {
        Unit* hand = AI_VALUE2(Unit*, "rti target", rtiName);
        if (hand && AI_VALUE(Unit*, "current target") != hand)
            return Attack(hand);
    }

    return false;
}

bool KiljaedenStunHandsOfTheDeceiverAction::Execute(Event /*event*/)
{
    if (bot->getClass() == CLASS_SHAMAN || bot->getClass() == CLASS_MAGE)
        return false;

    auto const& attackers =
        botAI->GetAiObjectContext()->GetValue<GuidVector>("possible targets no los")->Get();

    for (ObjectGuid const guid : attackers)
    {
        Unit* hand = botAI->GetUnit(guid);
        if (!hand || !hand->IsAlive() || hand->GetHealthPct() <= 20.0f ||
            hand->GetEntry() != static_cast<uint32>(SunwellNpcs::NPC_HAND_OF_THE_DECEIVER))
        {
            continue;
        }

        if (hand->HasUnitState(UNIT_STATE_STUNNED) || hand->HasSilenceAura())
            continue;

        if (CastStunOnHand(hand))
            return true;

        if (CastSilenceOnHand(hand))
            return true;
    }

    return false;
}

bool KiljaedenStunHandsOfTheDeceiverAction::CastStunOnHand(Unit* hand)
{
    if (hand->GetHealthPct() > 80.0f)
        return false;

    auto const castSpell = [&](char const* spell)
    {
        return botAI->CanCastSpell(spell, hand) && botAI->CastSpell(spell, hand);
    };

    switch (bot->getClass())
    {
        case CLASS_DRUID:
            return castSpell("bash");

        case CLASS_PALADIN:
            return castSpell("hammer of justice");

        case CLASS_ROGUE:
            return castSpell("kidney shot");

        case CLASS_WARLOCK:
            return castSpell("shadowfury");

        case CLASS_WARRIOR:
            return castSpell("concussion blow") || castSpell("shockwave");

        default:
            if (bot->getRace() == RACE_TAUREN)
                return castSpell("war stomp");
            return false;
    }
}

bool KiljaedenStunHandsOfTheDeceiverAction::CastSilenceOnHand(Unit* hand)
{
    auto const castSpell = [&](char const* spell)
    {
        return botAI->CanCastSpell(spell, hand) && botAI->CastSpell(spell, hand);
    };

    switch (bot->getClass())
    {
        case CLASS_PRIEST:
            return castSpell("silence");

        case CLASS_DEATH_KNIGHT:
            return castSpell("strangulate");

        case CLASS_HUNTER:
            return castSpell("silencing shot");

        default:
            if (bot->getRace() == RACE_BLOODELF)
                return castSpell("arcane torrent");
            return false;
    }
}

bool KiljaedenPositionTanksAction::Execute(Event /*event*/)
{
    const Position& position = KILJAEDEN_TANK_POSITION;
    if (bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY()) > 2.0f)
    {
        return MoveTo(
            SUNWELL_MAP_ID, position.GetPositionX(), position.GetPositionY(),
            position.GetPositionZ(), false, false, false, false,
            MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    return false;
}

bool KiljaedenPositionMeleeAction::Execute(Event /*event*/)
{
    Position position;
    if (!TryGetPosition(position))
        return false;

    if (!TryAdjustForArmageddon(position))
        return false;

    if (bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY()) <= 2.0f)
        return false;

    return MoveTo(
        SUNWELL_MAP_ID, position.GetPositionX(), position.GetPositionY(),
        position.GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_COMBAT, true, false);
}

bool KiljaedenPositionMeleeAction::TryGetPosition(Position& position) const
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    size_t meleeIndex = 0;
    bool foundAssignment = false;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !botAI->IsMelee(member) || member->GetMapId() != SUNWELL_MAP_ID ||
            !GET_PLAYERBOT_AI(member) || botAI->IsTank(member))
        {
            continue;
        }

        if (member == bot)
        {
            foundAssignment = true;
            break;
        }

        ++meleeIndex;
    }

    if (!foundAssignment)
        return false;

    position = meleeIndex % 2 == 0 ? KILJAEDEN_S_MELEE_POSITION : KILJAEDEN_E_MELEE_POSITION;
    return true;
}

bool KiljaedenPositionMeleeAction::TryAdjustForArmageddon(Position& position)
{
    PruneExpiredKiljaedenArmageddons(bot->GetInstanceId());
    auto armageddonItr = kiljaedenEncounterStates.find(bot->GetInstanceId());
    if (armageddonItr == kiljaedenEncounterStates.end() || armageddonItr->second.armageddons.empty())
        return true;

    Unit* kiljaeden = AI_VALUE2(Unit*, "find target", "kil'jaeden");
    if (!kiljaeden || IsKiljaedenCastingDarknessOfAThousandSouls(kiljaeden) ||
        HasKiljaedenDragonAura(bot))
    {
        return true;
    }

    Position const& assignedPosition = position;
    bool const isSouthPosition =
        assignedPosition.GetExactDist2d(KILJAEDEN_S_MELEE_POSITION) < 1.0f;
    Position const& swapPosition =
        isSouthPosition ? KILJAEDEN_E_MELEE_POSITION : KILJAEDEN_S_MELEE_POSITION;

    auto const isSafePosition = [&](Position const& pos)
    {
        for (KiljaedenArmageddon const& armageddon : armageddonItr->second.armageddons)
        {
            if (pos.GetExactDist2d(
                    armageddon.destination.GetPositionX(),
                    armageddon.destination.GetPositionY()) <
                armageddon.safeDistance)
            {
                return false;
            }
        }

        return true;
    };

    if (isSafePosition(assignedPosition))
        return true;

    if (isSafePosition(swapPosition))
    {
        position = swapPosition;
        return true;
    }

    KiljaedenArmageddon armageddon;
    if (!TryGetKiljaedenNearestArmageddon(bot, armageddon))
        return false;

    position = BestPositionForMeleeToFlee(armageddon.destination, armageddon.safeDistance);
    return position != Position();
}

bool KiljaedenPositionRangedAction::Execute(Event /*event*/)
{
    Position position = KILJAEDEN_TANK_POSITION;
    if (!TryGetPosition(position))
        return false;

    if (!TryAdjustForArmageddon(position))
        return false;

    if (bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY()) <= 2.0f)
        return false;

    return MoveTo(
        SUNWELL_MAP_ID, position.GetPositionX(), position.GetPositionY(),
        position.GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_COMBAT, true, false);
}

bool KiljaedenPositionRangedAction::TryGetPosition(Position& position) const
{
    EnsureKiljaedenRangedAssignments(botAI, bot);

    auto const instanceItr = kiljaedenEncounterStates.find(bot->GetInstanceId());
    if (instanceItr == kiljaedenEncounterStates.end())
        return false;

    auto const assignmentItr = instanceItr->second.rangedAssignments.find(bot->GetGUID());
    if (assignmentItr == instanceItr->second.rangedAssignments.end())
        return false;

    return TryGetKiljaedenRangedSlotPosition(assignmentItr->second, position);
}

bool KiljaedenPositionRangedAction::TryAdjustForArmageddon(Position& position)
{
    EnsureKiljaedenRangedArmageddonAssignments(botAI, bot);
    auto const armageddonAssignmentItr =
        kiljaedenEncounterStates.find(bot->GetInstanceId());

    if (armageddonAssignmentItr == kiljaedenEncounterStates.end())
        return true;

    auto const tempAssignmentItr = armageddonAssignmentItr->second.rangedArmageddonAssignments.find(bot->GetGUID());
    if (tempAssignmentItr == armageddonAssignmentItr->second.rangedArmageddonAssignments.end())
        return true;

    return TryGetKiljaedenRangedSlotPosition(tempAssignmentItr->second, position);
}

bool KiljaedenRemoveFireBloomAction::Execute(Event /*event*/)
{
    switch (bot->getClass())
    {
        case CLASS_MAGE:
            return botAI->CanCastSpell("ice block", bot) &&
                botAI->CastSpell("ice block", bot);

        case CLASS_PALADIN:
            return botAI->CanCastSpell("divine shield", bot) &&
                botAI->CastSpell("divine shield", bot);

        case CLASS_ROGUE:
            return botAI->CanCastSpell("cloak of shadows", bot) &&
                botAI->CastSpell("cloak of shadows", bot);

        default:
            return false;
    }
}

bool KiljaedenStackForShieldOfTheBlueAction::Execute(Event /*event*/)
{
    const Position& position = KILJAEDEN_DARKNESS_POSITION;
    if (bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY()) > 2.0f)
    {
        return MoveTo(
            SUNWELL_MAP_ID, position.GetPositionX(), position.GetPositionY(),
            position.GetPositionZ(), false, false, false, false,
            MovementPriority::MOVEMENT_FORCED, true, false);
    }

    return false;
}

bool KiljaedenUseDragonOrbAction::Execute(Event /*event*/)
{
    GameObject* closestOrb = nullptr;
    GameObject* closestInUseOrb = nullptr;
    float closestDistance = 0.0f;
    float closestInUseOrbDistance = 0.0f;
    bool orbInUse = false;

    constexpr float orbInUsePendingDistance = 15.0f;

    for (const uint32 orbEntry : KILJAEDEN_DRAGON_ORB_ENTRIES)
    {
        GameObject* orb = bot->FindNearestGameObject(orbEntry, 200.0f, true);
        if (!orb)
            continue;

        const float distance = bot->GetExactDist2d(orb);
        if (orb->HasGameObjectFlag(GO_FLAG_IN_USE))
        {
            orbInUse = true;
            if (!closestInUseOrb || distance < closestInUseOrbDistance)
            {
                closestInUseOrb = orb;
                closestInUseOrbDistance = distance;
            }

            continue;
        }

        if (orb->HasGameObjectFlag(GO_FLAG_NOT_SELECTABLE))
            continue;

        if (!closestOrb || distance < closestDistance)
        {
            closestOrb = orb;
            closestDistance = distance;
        }
    }

    if (orbInUse)
    {
        if (closestInUseOrb)
        {
            if (closestInUseOrbDistance <= orbInUsePendingDistance)
                return true;

            return MoveTo(
                SUNWELL_MAP_ID, closestInUseOrb->GetPositionX(), closestInUseOrb->GetPositionY(),
                closestInUseOrb->GetPositionZ(), false, false, false, false,
                MovementPriority::MOVEMENT_FORCED, true, false);
        }

        return false;
    }

    if (!closestOrb)
        return false;

    if (closestOrb->IsAtInteractDistance(*bot, closestOrb->GetInteractionDistance()))
    {
        closestOrb->Use(bot);
        kiljaedenDragonOrbUseTimes[bot->GetGUID().GetCounter()] = getMSTime();
        return true;
    }

    float const targetDist = closestOrb->GetInteractionDistance() - 0.5f;
    float const angle = closestOrb->GetAngle(bot);
    float const destX = closestOrb->GetPositionX() + std::cos(angle) * targetDist;
    float const destY = closestOrb->GetPositionY() + std::sin(angle) * targetDist;

    return MoveTo(
        SUNWELL_MAP_ID, destX, destY, closestOrb->GetPositionZ(),
        false, false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
}

// There is an issue with the root packets that causes bots to get stuck with
// the root movement flag after using a dragon orb; this action is a workaround
// to remove the stale root flag in those cases
bool KiljaedenReleaseStaleRootAction::Execute(Event /*event*/)
{
    bot->m_movementInfo.RemoveMovementFlag(MOVEMENTFLAG_ROOT);
    bot->SendMovementFlagUpdate();
    return true;
}

bool KiljaedenControlDragonAction::Execute(Event /*event*/)
{
    Unit* kiljaeden = AI_VALUE2(Unit*, "find target", "kil'jaeden");
    if (!kiljaeden)
        return false;

    // Design choice: End drake control after phase changes
    if (kiljaeden->HasUnitState(UNIT_STATE_CASTING) &&
        kiljaeden->FindCurrentSpellBySpellId(
            static_cast<uint32>(SunwellSpells::SPELL_SHADOW_SPIKE)))
    {
        if (HasKiljaedenDragonAura(bot))
        {
            bot->RemoveAura(
                static_cast<uint32>(SunwellSpells::SPELL_VENGEANCE_OF_THE_BLUE_FLIGHT));
            return true;
        }

        return false;
    }

    Unit* dragon = GetKiljaedenControlledDragon(bot);
    if (!dragon)
        return false;

    if (IsKiljaedenCastingDarknessOfAThousandSouls(kiljaeden))
        return ExecuteDuringDarknessOfAThousandSouls(kiljaeden, dragon);

    return ExecuteOutsideDarknessOfAThousandSouls(dragon);
}

bool KiljaedenControlDragonAction::ExecuteDuringDarknessOfAThousandSouls(
    Unit* kiljaeden, Unit* dragon)
{
    Spell* darknessSpell = kiljaeden->FindCurrentSpellBySpellId(
        static_cast<uint32>(SunwellSpells::SPELL_DARKNESS_OF_A_THOUSAND_SOULS));
    if (!darknessSpell)
        return false;

    constexpr float desiredDistanceFromStack = 2.0f;
    constexpr float castReadyDistanceFromStack = 3.0f;
    const Position& stackPosition = KILJAEDEN_DARKNESS_POSITION;
    const float distanceToStack = dragon->GetExactDist2d(
        stackPosition.GetPositionX(), stackPosition.GetPositionY());
    if (distanceToStack > castReadyDistanceFromStack)
    {
        if (dragon->GetMotionMaster()->GetCurrentMovementGeneratorType() == POINT_MOTION_TYPE &&
            dragon->isMoving())
        {
            return true;
        }

        const float deltaX = stackPosition.GetPositionX() - dragon->GetPositionX();
        const float deltaY = stackPosition.GetPositionY() - dragon->GetPositionY();
        const float moveRatio = (distanceToStack - desiredDistanceFromStack) / distanceToStack;
        const float moveX = dragon->GetPositionX() + deltaX * moveRatio;
        const float moveY = dragon->GetPositionY() + deltaY * moveRatio;

        dragon->GetMotionMaster()->MovePoint(0, moveX, moveY, stackPosition.GetPositionZ());
        return true;
    }

    if (dragon->GetCurrentSpell(CURRENT_GENERIC_SPELL) ||
        dragon->GetCurrentSpell(CURRENT_CHANNELED_SPELL))
    {
        return false;
    }

    if (darknessSpell->GetCastTimeRemaining() < 4000) // Cast Shield of the Blue at 4s remaining
    {
        return CastKiljaedenDragonSpell(
            dragon, static_cast<uint32>(SunwellSpells::SPELL_SHIELD_OF_THE_BLUE));
    }
    else if (CastKiljaedenDragonSpell(
                 dragon, static_cast<uint32>(SunwellSpells::SPELL_DRAGON_BREATH_HASTE)) ||
             CastKiljaedenDragonSpell(
                 dragon, static_cast<uint32>(SunwellSpells::SPELL_DRAGON_BREATH_REVITALIZE)))
    {
        return true;
    }

    return false;
}

bool KiljaedenControlDragonAction::ExecuteOutsideDarknessOfAThousandSouls(Unit* dragon)
{
    if (dragon->GetCurrentSpell(CURRENT_GENERIC_SPELL) ||
        dragon->GetCurrentSpell(CURRENT_CHANNELED_SPELL))
    {
        return false;
    }

    uint32 spellId = 0;
    Player* target = nullptr;

    constexpr uint32 hasteSpellId =
        static_cast<uint32>(SunwellSpells::SPELL_DRAGON_BREATH_HASTE);
    constexpr uint32 revitalizeSpellId =
        static_cast<uint32>(SunwellSpells::SPELL_DRAGON_BREATH_REVITALIZE);

    if (!dragon->HasSpellCooldown(hasteSpellId))
    {
        target = FindBestKiljaedenDragonClusterTarget(botAI, bot, dragon, hasteSpellId);
        if (!target)
            target = FindClosestKiljaedenDragonTarget(bot, dragon, hasteSpellId);
        if (target)
            spellId = hasteSpellId;
    }

    if (!target && !dragon->HasSpellCooldown(revitalizeSpellId))
    {
        target = FindBestKiljaedenDragonClusterTarget(botAI, bot, dragon, revitalizeSpellId);
        if (!target)
            target = FindClosestKiljaedenDragonTarget(bot, dragon, revitalizeSpellId);
        if (target)
            spellId = revitalizeSpellId;
    }

    if (!target)
    {
        target = FindClosestKiljaedenDragonTarget(bot, dragon);
        if (!target)
            return false;

        if (!dragon->HasSpellCooldown(hasteSpellId))
            spellId = hasteSpellId;
        else if (!dragon->HasSpellCooldown(revitalizeSpellId))
            spellId = revitalizeSpellId;
        else
            return false;
    }

    if (!spellId)
        return false;

    constexpr float desiredDistance = 6.0f;
    constexpr float distanceTolerance = 1.0f;
    const float distanceToTarget = dragon->GetExactDist2d(target);
    if (distanceToTarget > desiredDistance + distanceTolerance ||
        (distanceToTarget > std::numeric_limits<float>::min() &&
         distanceToTarget < desiredDistance - distanceTolerance))
    {
        const float deltaX = target->GetPositionX() - dragon->GetPositionX();
        const float deltaY = target->GetPositionY() - dragon->GetPositionY();
        const float moveRatio = (distanceToTarget - desiredDistance) / distanceToTarget;
        const float moveX = dragon->GetPositionX() + deltaX * moveRatio;
        const float moveY = dragon->GetPositionY() + deltaY * moveRatio;

        dragon->GetMotionMaster()->MovePoint(0, moveX, moveY, target->GetPositionZ());
        return true;
    }

    dragon->SetFacingToObject(target);

    return CastKiljaedenDragonSpell(dragon, spellId);
}
