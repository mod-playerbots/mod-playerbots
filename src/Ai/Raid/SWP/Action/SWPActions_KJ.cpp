/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "SWPActions.h"
#include "SWPEncounter_KJ.h"
#include "Playerbots.h"
#include "PlayerbotTextMgr.h"
#include "RaidBossHelpers.h"
#include <map>
#include <vector>

using namespace SwpHelpers;

bool KiljaedenAnnounceDragonOrbUserAction::Execute(Event /*event*/)
{
    uint32 const instanceId = bot->GetInstanceId();
    auto const stateItr = kiljaedenEncounterStates.find(instanceId);

    if (stateItr != kiljaedenEncounterStates.end() && stateItr->second.dragonOrbAnnouncementMs)
        return false;

    kiljaedenEncounterStates[instanceId].dragonOrbAnnouncementMs = getMSTime();

    Player* orbUser = GetKiljaedenDragonOrbUser(bot);
    std::string text;

    if (orbUser)
    {
        std::map<std::string, std::string> placeholders = {{"%bot", orbUser->GetName()}};
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

bool KiljaedenMarkAndPrioritizeHandsOfTheDeceiverAction::Execute(Event /*event*/)
{
    Player* mainTank = GetGroupMainTank(botAI, bot);
    Player* firstAssistTank = GetGroupAssistTank(botAI, bot, 0);
    Player* secondAssistTank = GetGroupAssistTank(botAI, bot, 1);
    if (!mainTank || !GET_PLAYERBOT_AI(mainTank) ||
        !firstAssistTank || !GET_PLAYERBOT_AI(firstAssistTank) ||
        !secondAssistTank || !GET_PLAYERBOT_AI(secondAssistTank))
    {
        return false;
    }

    std::vector<Unit*> hands;
    auto const& targets =
        botAI->GetAiObjectContext()->GetValue<GuidVector>("possible targets no los")->Get();

    for (ObjectGuid const targetGuid : targets)
    {
        Unit* target = botAI->GetUnit(targetGuid);
        if (target && target->GetEntry() == static_cast<uint32>(SwpNpcs::NPC_HAND_OF_THE_DECEIVER))
            hands.push_back(target);
    }

    if (hands.empty())
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

        auto& assignments = kiljaedenHandTankAssignments[bot->GetInstanceId()];
        ObjectGuid& assignedGuid = assignments[myIndex];

        if (!assignedGuid.IsEmpty())
        {
            bool alive = false;
            for (Unit* hand : hands)
            {
                if (hand->GetGUID() == assignedGuid)
                {
                    alive = true;
                    break;
                }
            }
            if (!alive)
                assignedGuid = ObjectGuid::Empty;
        }

        if (assignedGuid.IsEmpty() && myIndex < hands.size())
            assignedGuid = hands[myIndex]->GetGUID();

        if (assignedGuid.IsEmpty())
            return false;

        Unit* assignedHand = botAI->GetUnit(assignedGuid);
        if (!assignedHand || !assignedHand->IsAlive())
            return false;

        if (AI_VALUE(Unit*, "current target") != assignedHand)
            return Attack(assignedHand);

        if (assignedHand->GetVictim() == bot && bot->IsWithinMeleeRange(assignedHand) &&
            !assignedHand->HasUnitState(UNIT_STATE_STUNNED))
        {
            constexpr float minTankDistance = 15.0f;

            for (size_t i = 0; i < tanks.size(); ++i)
            {
                if (i == myIndex)
                    continue;

                Player* otherTank = tanks[i];
                if (!otherTank || !otherTank->IsAlive())
                    continue;

                ObjectGuid const otherGuid = assignments[i];
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
    }
    else
    {
        Unit* focusHand = hands[0];
        for (Unit* hand : hands)
        {
            if (hand->GetGUID() < focusHand->GetGUID())
                focusHand = hand;
        }

        if (IsMechanicTrackerBot(bot, SWP_MAP_ID) && MarkTargetWithSkull(bot, focusHand))
            return true;

        if (AI_VALUE(Unit*, "current target") != focusHand)
            return Attack(focusHand);
    }

    return false;
}

bool KiljaedenStunHandsOfTheDeceiverAction::Execute(Event /*event*/)
{
    if (bot->getClass() == CLASS_SHAMAN || bot->getClass() == CLASS_MAGE)
        return false;

    auto const& targets =
        botAI->GetAiObjectContext()->GetValue<GuidVector>("possible targets no los")->Get();

    for (ObjectGuid const targetGuid : targets)
    {
        Unit* target = botAI->GetUnit(targetGuid);
        if (!target || target->GetHealthPct() <= 20.0f ||
            target->GetEntry() != static_cast<uint32>(SwpNpcs::NPC_HAND_OF_THE_DECEIVER))
        {
            continue;
        }

        if (target->HasUnitState(UNIT_STATE_STUNNED) || target->HasSilenceAura())
            continue;

        if (CastStunOnHand(target))
            return true;

        if (CastSilenceOnHand(target))
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
            return castSpell("bash") || castSpell("maim");

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
    Position const& position = KILJAEDEN_TANK_POSITION;
    if (bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY()) <= 2.0f)
        return false;

    return MoveTo(
        SWP_MAP_ID, position.GetPositionX(), position.GetPositionY(), position.GetPositionZ(),
        false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
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
        SWP_MAP_ID, position.GetPositionX(), position.GetPositionY(), position.GetPositionZ(),
        false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
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
        if (!member || !botAI->IsMelee(member) || member->GetMapId() != SWP_MAP_ID ||
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
    if (armageddonItr == kiljaedenEncounterStates.end() ||
        armageddonItr->second.armageddons.empty())
    {
        return true;
    }

    Unit* kiljaeden = AI_VALUE2(Unit*, "find target", "kil'jaeden");
    if (!kiljaeden || IsKiljaedenCastingDarknessOfAThousandSouls(kiljaeden) ||
        HasKiljaedenDragonAura(bot))
    {
        return true;
    }

    Position const assignedPosition = position;
    bool const isSouthPosition =
        assignedPosition.GetExactDist2d(KILJAEDEN_S_MELEE_POSITION) < 1.0f;
    Position const swapPosition =
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
        SWP_MAP_ID, position.GetPositionX(), position.GetPositionY(), position.GetPositionZ(),
        false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
}

bool KiljaedenPositionRangedAction::TryGetPosition(Position& position) const
{
    EnsureKiljaedenRangedAssignments(bot);

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
    EnsureKiljaedenRangedArmageddonAssignments(bot);
    auto const armageddonAssignmentItr =
        kiljaedenEncounterStates.find(bot->GetInstanceId());

    if (armageddonAssignmentItr == kiljaedenEncounterStates.end())
        return true;

    auto const tempAssignmentItr =
        armageddonAssignmentItr->second.rangedArmageddonAssignments.find(bot->GetGUID());
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
    Position const& position = KILJAEDEN_DARKNESS_POSITION;
    if (bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY()) <= 2.0f)
        return false;

    botAI->InterruptSpell();
    return MoveTo(
        SWP_MAP_ID, position.GetPositionX(), position.GetPositionY(), position.GetPositionZ(),
        false, false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
}

bool KiljaedenUseDragonOrbAction::Execute(Event /*event*/)
{
    GameObject* closestOrb = nullptr;
    GameObject* closestInUseOrb = nullptr;
    float closestDistance = 0.0f;
    float closestInUseOrbDistance = 0.0f;
    bool orbInUse = false;

    constexpr float orbInUsePendingDistance = 15.0f;

    for (uint32 const orbEntry : KILJAEDEN_DRAGON_ORB_ENTRIES)
    {
        GameObject* orb = bot->FindNearestGameObject(orbEntry, 200.0f, true);
        if (!orb)
            continue;

        float const distance = bot->GetExactDist2d(orb);
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
                SWP_MAP_ID, closestInUseOrb->GetPositionX(), closestInUseOrb->GetPositionY(),
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
        SWP_MAP_ID, destX, destY, closestOrb->GetPositionZ(), false, false,
        false, false, MovementPriority::MOVEMENT_FORCED, true, false);
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
        kiljaeden->FindCurrentSpellBySpellId(static_cast<uint32>(SwpSpells::SPELL_SHADOW_SPIKE)))
    {
        if (HasKiljaedenDragonAura(bot))
        {
            bot->RemoveAura(
                static_cast<uint32>(SwpSpells::SPELL_VENGEANCE_OF_THE_BLUE_FLIGHT));
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
        static_cast<uint32>(SwpSpells::SPELL_DARKNESS_OF_A_THOUSAND_SOULS));
    if (!darknessSpell)
        return false;

    constexpr float desiredDistanceFromStack = 2.0f;
    constexpr float castReadyDistanceFromStack = 3.0f;
    Position const stackPosition = KILJAEDEN_DARKNESS_POSITION;
    float const distanceToStack = dragon->GetExactDist2d(
        stackPosition.GetPositionX(), stackPosition.GetPositionY());
    if (distanceToStack > castReadyDistanceFromStack)
    {
        if (dragon->GetMotionMaster()->GetCurrentMovementGeneratorType() == POINT_MOTION_TYPE &&
            dragon->isMoving())
        {
            return true;
        }

        float const deltaX = stackPosition.GetPositionX() - dragon->GetPositionX();
        float const deltaY = stackPosition.GetPositionY() - dragon->GetPositionY();
        float const moveRatio = (distanceToStack - desiredDistanceFromStack) / distanceToStack;
        float const moveX = dragon->GetPositionX() + deltaX * moveRatio;
        float const moveY = dragon->GetPositionY() + deltaY * moveRatio;

        dragon->GetMotionMaster()->MovePoint(0, moveX, moveY, stackPosition.GetPositionZ());
        return true;
    }

    if (dragon->GetCurrentSpell(CURRENT_GENERIC_SPELL) ||
        dragon->GetCurrentSpell(CURRENT_CHANNELED_SPELL))
    {
        return false;
    }

    if (darknessSpell->GetCastTimeRemaining() < 4500)
    {
        return CastKiljaedenDragonSpell(
            dragon, static_cast<uint32>(SwpSpells::SPELL_SHIELD_OF_THE_BLUE));
    }
    else if (CastKiljaedenDragonSpell(
        dragon, static_cast<uint32>(SwpSpells::SPELL_DRAGON_BREATH_HASTE)))
    {
        return true;
    }
    else if (CastKiljaedenDragonSpell(
        dragon, static_cast<uint32>(SwpSpells::SPELL_DRAGON_BREATH_REVITALIZE)))
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
        static_cast<uint32>(SwpSpells::SPELL_DRAGON_BREATH_HASTE);
    constexpr uint32 revitalizeSpellId =
        static_cast<uint32>(SwpSpells::SPELL_DRAGON_BREATH_REVITALIZE);

    if (!dragon->HasSpellCooldown(hasteSpellId))
    {
        target = FindBestKiljaedenDragonClusterTarget(bot, dragon, hasteSpellId);
        if (!target)
            target = FindClosestKiljaedenDragonTarget(bot, dragon, hasteSpellId);
        if (target)
            spellId = hasteSpellId;
    }

    if (!target && !dragon->HasSpellCooldown(revitalizeSpellId))
    {
        target = FindBestKiljaedenDragonClusterTarget(bot, dragon, revitalizeSpellId);
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
    float const distanceToTarget = dragon->GetExactDist2d(target);
    if (distanceToTarget > desiredDistance + distanceTolerance ||
        (distanceToTarget > std::numeric_limits<float>::min() &&
         distanceToTarget < desiredDistance - distanceTolerance))
    {
        float const deltaX = target->GetPositionX() - dragon->GetPositionX();
        float const deltaY = target->GetPositionY() - dragon->GetPositionY();
        float const moveRatio = (distanceToTarget - desiredDistance) / distanceToTarget;
        float const moveX = dragon->GetPositionX() + deltaX * moveRatio;
        float const moveY = dragon->GetPositionY() + deltaY * moveRatio;

        dragon->GetMotionMaster()->MovePoint(0, moveX, moveY, target->GetPositionZ());
        return true;
    }

    dragon->SetFacingToObject(target);

    return CastKiljaedenDragonSpell(dragon, spellId);
}
