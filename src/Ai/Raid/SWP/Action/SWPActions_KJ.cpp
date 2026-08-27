/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "SWPActions.h"
#include "EncounterHelpers.h"
#include "Playerbots.h"
#include "PlayerbotTextMgr.h"
#include "SWPEncounter_KJ.h"
#include "SWPSharedConstants.h"
#include <algorithm>
#include <cmath>
#include <map>

using namespace SwpHelpers;
using namespace EncounterHelpers;

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
            {});
    }

    return botAI->SayToRaid(text);
}

bool KiljaedenAssignHandsOfTheDeceiverAction::Execute(Event /*event*/)
{
    std::vector<Unit*> hands;
    auto const& targets = AI_VALUE(GuidVector, "possible targets no los");

    for (ObjectGuid const& targetGuid : targets)
    {
        Unit* target = botAI->GetUnit(targetGuid);
        if (target && target->GetEntry() == Id(SwpNpcs::NPC_HAND_OF_THE_DECEIVER))
            hands.push_back(target);
    }

    if (hands.empty())
        return false;

    std::sort(hands.begin(), hands.end(),
        [](Unit* left, Unit* right) { return left->GetGUID() < right->GetGUID(); });

    if (IsMechanicTrackerBot(bot, SWP_MAP_ID) && MarkTargetWithSkull(bot, hands.front()))
        return true;

    if (!PlayerbotAI::IsTank(bot))
        return AI_VALUE(Unit*, "current target") != hands.front() && Attack(hands.front());

    Player* secondAssistTank = GetGroupAssistTank(bot, 1);
    if (!secondAssistTank)
        return false;

    Player* firstAssistTank = GetGroupAssistTank(bot, 0);
    if (!firstAssistTank)
        return false;

    Player* mainTank = GetGroupMainTank(bot);
    if (!mainTank)
        return false;

    if (mainTank != bot && firstAssistTank != bot && secondAssistTank != bot)
        return false;

    return ExecuteTankHandAssignment(hands, mainTank, firstAssistTank, secondAssistTank);
}

bool KiljaedenAssignHandsOfTheDeceiverAction::ExecuteTankHandAssignment(
    std::vector<Unit*> const& hands,
    Player* mainTank, Player* firstAssistTank, Player* secondAssistTank)
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
        bool stillPresent = false;
        for (Unit* hand : hands)
        {
            if (hand->GetGUID() == assignedGuid)
            {
                stillPresent = true;
                break;
            }
        }

        if (!stillPresent)
            assignedGuid = ObjectGuid::Empty;
    }

    if (assignedGuid.IsEmpty())
    {
        for (Unit* hand : hands)
        {
            ObjectGuid const handGuid = hand->GetGUID();

            bool claimedByOtherTank = false;
            for (size_t i = 0; i < assignments.size(); ++i)
            {
                if (i != myIndex && assignments[i] == handGuid)
                {
                    claimedByOtherTank = true;
                    break;
                }
            }

            if (!claimedByOtherTank)
            {
                assignedGuid = handGuid;
                break;
            }
        }
    }

    if (assignedGuid.IsEmpty())
        return false;

    Unit* assignedHand = botAI->GetUnit(assignedGuid);
    if (!assignedHand || !assignedHand->IsAlive())
        return false;

    if (AI_VALUE(Unit*, "current target") != assignedHand)
        return Attack(assignedHand);

    if (assignedHand->GetVictim() != bot || !bot->IsWithinMeleeRange(assignedHand) ||
        assignedHand->HasUnitState(UNIT_STATE_STUNNED))
    {
        return false;
    }

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
        if (distFromTank < HAND_TANK_SEPARATION)
            return MoveAway(otherTank, HAND_TANK_SEPARATION - distFromTank, true);
    }

    return false;
}

bool KiljaedenStunHandsOfTheDeceiverAction::Execute(Event /*event*/)
{
    if (bot->getClass() == CLASS_SHAMAN)
        return false;

    auto const& targets = AI_VALUE(GuidVector, "possible targets no los");

    for (ObjectGuid const& targetGuid : targets)
    {
        Unit* target = botAI->GetUnit(targetGuid);
        if (!target || target->GetHealthPct() <= HAND_STUN_IMMUNE_HP_PERCENT ||
            target->GetEntry() != Id(SwpNpcs::NPC_HAND_OF_THE_DECEIVER))
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
    if (hand->GetHealthPct() > HAND_STUN_MAX_HP_PERCENT)
        return false;

    auto const castSpell = [&](char const* spell)
    {
        return botAI->CanCastSpell(spell, hand) && botAI->CastSpell(spell, hand);
    };

    auto const castSelfAoe = [&](char const* spell, float radius)
    {
        return bot->GetExactDist(hand) < radius && castSpell(spell);
    };

    switch (bot->getClass())
    {
        case CLASS_DRUID:
            return (botAI->HasStrategy("bear", BOT_STATE_COMBAT) && castSpell("bash")) ||
                (botAI->HasStrategy("cat", BOT_STATE_COMBAT) && castSpell("maim"));

        case CLASS_MAGE:
            return castSpell("deep freeze");

        case CLASS_PALADIN:
            return castSpell("hammer of justice");

        case CLASS_ROGUE:
            return castSpell("kidney shot");

        case CLASS_WARLOCK:
            return castSpell("shadowfury");

        case CLASS_WARRIOR:
            return castSpell("concussion blow") ||
                castSelfAoe("shockwave", KILJAEDEN_SHOCKWAVE_RADIUS);

        default:
            return bot->getRace() == RACE_TAUREN &&
                castSelfAoe("war stomp", KILJAEDEN_SELF_AOE_RACIAL_RADIUS);
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
        case CLASS_HUNTER:
            return castSpell("silencing shot");

        case CLASS_PRIEST:
            return castSpell("silence");

        case CLASS_DEATH_KNIGHT:
            return castSpell("strangulate");

        default:
            // Arcane Torrent is centred on the caster too, so it needs the same guard
            return bot->getRace() == RACE_BLOODELF &&
                bot->GetExactDist(hand) < KILJAEDEN_SELF_AOE_RACIAL_RADIUS &&
                castSpell("arcane torrent");
    }
}

bool KiljaedenPositionAndMoveTanksAction::Execute(Event /*event*/)
{
    if (!PlayerbotAI::IsMainTank(bot))
    {
        // This grid search exists only to bridge the three seconds after spawn, during which a
        // Reflection is passive and on nobody's threat list, meaning neither FindTargetValue nor
        // standard bot target acquisition through "attackers" can locate it
        if (Creature* reflection = bot->FindNearestCreature(
                Id(SwpNpcs::NPC_SINISTER_REFLECTION), KILJAEDEN_REFLECTION_SEARCH_RADIUS))
        {
            // Once aggressive it is on a threat list and therefore in "attackers,"" so failing
            // here is intended to allow TankAssistAction to take over.
            return reflection->GetReactState() == REACT_PASSIVE &&
                PickUpSinisterReflections(reflection);
        }
    }

    Position const& position = KILJAEDEN_TANK_POSITION;
    if (bot->GetExactDist2d(position) <= 2.0f)
        return false;

    return MoveTo(
        SWP_MAP_ID, position.GetPositionX(), position.GetPositionY(), position.GetPositionZ(),
        false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
}

// When Reflections activate after 3s, they begin attack with SMART_ACTION_ATTACK_START, which sets
// a random victim. Thus, the first landed hit after activation should immediately grab aggro, and
// we want that to be a tank, so hopefully they can get in range to start attacking before 3s pass.
bool KiljaedenPositionAndMoveTanksAction::PickUpSinisterReflections(Creature* reflection)
{
    if (AI_VALUE(Unit*, "current target") != reflection)
        return Attack(reflection);

    float const distance = bot->GetExactDist(reflection);
    auto const castSpell = [&](char const* spell, float reach)
    {
        return distance < reach && botAI->CanCastSpell(spell, reflection) &&
            botAI->CastSpell(spell, reflection);
    };

    switch (bot->getClass())
    {
        case CLASS_DEATH_KNIGHT:
            return castSpell("death and decay", KILJAEDEN_REFLECTION_RANGED_REACH) ||
                castSpell("icy touch", KILJAEDEN_REFLECTION_ICY_TOUCH_REACH);

        case CLASS_DRUID:
            return castSpell("feral charge - bear", KILJAEDEN_REFLECTION_CHARGE_REACH) ||
                castSpell("challenging roar", KILJAEDEN_REFLECTION_SHOUT_REACH);

        case CLASS_PALADIN:
            return castSpell("avenger's shield", KILJAEDEN_REFLECTION_RANGED_REACH) ||
                castSpell("consecration", KILJAEDEN_REFLECTION_CONSECRATION_REACH);

        case CLASS_WARRIOR:
            return castSpell("charge", KILJAEDEN_REFLECTION_CHARGE_REACH) ||
                castSpell("challenging shout", KILJAEDEN_REFLECTION_SHOUT_REACH);

        default:
            return false;
    }
}

bool KiljaedenPositionMeleeAction::Execute(Event /*event*/)
{
    Position position;
    if (!TryGetPosition(position))
        return false;

    if (!TryAdjustForArmageddon(position))
        return false;

    if (bot->GetExactDist2d(position) <= 2.0f)
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
        if (!member || member->GetMapId() != SWP_MAP_ID || !GET_PLAYERBOT_AI(member) ||
            !PlayerbotAI::IsMelee(member) || PlayerbotAI::IsTank(member))
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
                    armageddon.destination.GetPositionY()) < armageddon.safeDistance)
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

bool KiljaedenPositionRangedAndAvoidArmageddonsAction::Execute(Event /*event*/)
{
    Position position;
    if (!TryGetPosition(position))
        return false;

    if (!TryAdjustForArmageddon(position))
        return false;

    if (bot->GetExactDist2d(position) <= 2.0f)
        return false;

    return MoveTo(
        SWP_MAP_ID, position.GetPositionX(), position.GetPositionY(), position.GetPositionZ(),
        false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
}

bool KiljaedenPositionRangedAndAvoidArmageddonsAction::TryGetPosition(Position& position) const
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

bool KiljaedenPositionRangedAndAvoidArmageddonsAction::TryAdjustForArmageddon(Position& position)
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
            return botAI->CanCastSpell(Id(SwpSpells::SPELL_ICE_BLOCK), bot) &&
                botAI->CastSpell(Id(SwpSpells::SPELL_ICE_BLOCK), bot);

        case CLASS_PALADIN:
            return botAI->CanCastSpell(Id(SwpSpells::SPELL_DIVINE_SHIELD), bot) &&
                botAI->CastSpell(Id(SwpSpells::SPELL_DIVINE_SHIELD), bot);

        case CLASS_ROGUE:
            return botAI->CanCastSpell(Id(SwpSpells::SPELL_CLOAK_OF_SHADOWS), bot) &&
                botAI->CastSpell(Id(SwpSpells::SPELL_CLOAK_OF_SHADOWS), bot);

        default:
            return false;
    }
}

bool KiljaedenStackForShieldOfTheBlueAction::Execute(Event /*event*/)
{
    Position const& darknessPosition = KILJAEDEN_DARKNESS_POSITION;
    float destX = darknessPosition.GetPositionX();
    float destY = darknessPosition.GetPositionY();

    if (bot->HasAura(Id(SwpSpells::SPELL_FIRE_BLOOM)))
    {
        Unit* kiljaeden = AI_VALUE2(Unit*, "find target", "kil'jaeden");
        if (!kiljaeden)
            return false;

        Spell* darknessSpell = kiljaeden->FindCurrentSpellBySpellId(
            Id(SwpSpells::SPELL_DARKNESS_OF_A_THOUSAND_SOULS));
        if (darknessSpell &&
            darknessSpell->GetCastTimeRemaining() >= SHIELD_OF_THE_BLUE_CAST_WINDOW_MS)
        {
            float const angle = darknessPosition.GetAngle(bot);
            destX = darknessPosition.GetPositionX() + std::cos(angle) * FIRE_BLOOM_STANDOFF;
            destY = darknessPosition.GetPositionY() + std::sin(angle) * FIRE_BLOOM_STANDOFF;
        }
    }

    if (bot->GetExactDist2d(destX, destY) <= 1.0f)
        return false;

    bot->CastStop();
    return MoveTo(
        SWP_MAP_ID, destX, destY, bot->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_FORCED, true, false);
}

bool KiljaedenUseDragonOrbAction::Execute(Event /*event*/)
{
    GameObject* closestOrb = nullptr;
    GameObject* closestInUseOrb = nullptr;
    float closestDistance = 0.0f;
    float closestInUseOrbDistance = 0.0f;
    bool orbInUse = false;

    for (ObjectGuid const& orbGuid : AI_VALUE(GuidVector, "kiljaeden dragon orbs"))
    {
        GameObject* orb = botAI->GetGameObject(orbGuid);
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

    // Failsafe to keep the orb user from leaving early
    if (orbInUse)
    {
        if (!closestInUseOrb)
            return false;

        if (closestInUseOrbDistance <= DRAGON_ORB_IN_USE_HOLD_DISTANCE)
            return true;

        return MoveTo(
            SWP_MAP_ID, closestInUseOrb->GetPositionX(), closestInUseOrb->GetPositionY(),
            closestInUseOrb->GetPositionZ(), false, false, false, false,
            MovementPriority::MOVEMENT_FORCED, true, false);
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
        SWP_MAP_ID, destX, destY, closestOrb->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_FORCED, true, false);
}

// There is an issue (maybe with the root packets) that causes bots to get stuck with the root
// movement flag after using a dragon orb; this action is a workaround to remove the stale flag.
bool KiljaedenReleaseStaleRootAction::Execute(Event /*event*/)
{
    bot->m_movementInfo.RemoveMovementFlag(MOVEMENTFLAG_ROOT);
    bot->SendMovementFlagUpdate();
    return true;
}

bool KiljaedenDragonBuffAndProtectRaidAction::Execute(Event /*event*/)
{
    Unit* kiljaeden = AI_VALUE2(Unit*, "find target", "kil'jaeden");
    if (!kiljaeden)
        return false;

    // End remaining bot drake control after phase changes, which may not be ideal but
    // is the safer approach without knowing the player's composition or raid knowledge.
    if (kiljaeden->HasUnitState(UNIT_STATE_CASTING) &&
        kiljaeden->FindCurrentSpellBySpellId(Id(SwpSpells::SPELL_SHADOW_SPIKE)))
    {
        if (!HasKiljaedenDragonAura(bot))
            return false;

        bot->RemoveAura(Id(SwpSpells::SPELL_VENGEANCE_OF_THE_BLUE_FLIGHT));
        return true;
    }

    Unit* dragon = GetKiljaedenControlledDragon(bot);
    if (!dragon)
        return false;

    if (IsKiljaedenCastingDarknessOfAThousandSouls(kiljaeden))
        return ExecuteDuringDarknessOfAThousandSouls(kiljaeden, dragon);

    return ExecuteOutsideDarknessOfAThousandSouls(dragon);
}

bool KiljaedenDragonBuffAndProtectRaidAction::ExecuteDuringDarknessOfAThousandSouls(
    Unit* kiljaeden, Unit* dragon)
{
    Spell* darknessSpell = kiljaeden->FindCurrentSpellBySpellId(
        Id(SwpSpells::SPELL_DARKNESS_OF_A_THOUSAND_SOULS));
    if (!darknessSpell)
        return false;

    constexpr float castReadyDistanceFromStack = 3.0f;
    Position const& stackPosition = KILJAEDEN_DARKNESS_POSITION;
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
        constexpr float desiredDistanceFromStack = 2.0f;
        float const moveRatio = (distanceToStack - desiredDistanceFromStack) / distanceToStack;
        float const moveX = dragon->GetPositionX() + deltaX * moveRatio;
        float const moveY = dragon->GetPositionY() + deltaY * moveRatio;

        dragon->GetMotionMaster()->MovePoint(0, moveX, moveY, stackPosition.GetPositionZ());
        return true;
    }

    if (dragon->IsNonMeleeSpellCast(false))
        return false;

    if (darknessSpell->GetCastTimeRemaining() < SHIELD_OF_THE_BLUE_CAST_WINDOW_MS)
        return CastKiljaedenDragonSpell(dragon, Id(SwpSpells::SPELL_SHIELD_OF_THE_BLUE));
    else if (CastKiljaedenDragonSpell(dragon, Id(SwpSpells::SPELL_DRAGON_BREATH_HASTE)))
        return true;
    else if (CastKiljaedenDragonSpell(dragon, Id(SwpSpells::SPELL_DRAGON_BREATH_REVITALIZE)))
        return true;

    return false;
}

bool KiljaedenDragonBuffAndProtectRaidAction::ExecuteOutsideDarknessOfAThousandSouls(Unit* dragon)
{
    if (dragon->IsNonMeleeSpellCast(false))
        return false;

    uint32 spellId = 0;
    Player* target = nullptr;

    constexpr uint32 hasteSpellId = Id(SwpSpells::SPELL_DRAGON_BREATH_HASTE);
    constexpr uint32 revitalizeSpellId = Id(SwpSpells::SPELL_DRAGON_BREATH_REVITALIZE);

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

    float const distanceToTarget = dragon->GetExactDist2d(target);

    if (distanceToTarget > DRAGON_BREATH_STANDOFF + DRAGON_STANDOFF_TOLERANCE ||
        (distanceToTarget > std::numeric_limits<float>::min() &&
         distanceToTarget < DRAGON_BREATH_STANDOFF - DRAGON_STANDOFF_TOLERANCE))
    {
        float const deltaX = target->GetPositionX() - dragon->GetPositionX();
        float const deltaY = target->GetPositionY() - dragon->GetPositionY();
        float const moveRatio =
            (distanceToTarget - DRAGON_BREATH_STANDOFF) / distanceToTarget;
        float const moveX = dragon->GetPositionX() + deltaX * moveRatio;
        float const moveY = dragon->GetPositionY() + deltaY * moveRatio;

        dragon->GetMotionMaster()->MovePoint(0, moveX, moveY, target->GetPositionZ());
        return true;
    }

    dragon->SetFacingToObject(target);

    return CastKiljaedenDragonSpell(dragon, spellId);
}
