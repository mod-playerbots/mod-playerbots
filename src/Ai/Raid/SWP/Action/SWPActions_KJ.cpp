/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "SWPActions.h"
#include "EncounterHelpers.h"
#include "Playerbots.h"
#include "RtiTargetValue.h"
#include "SWPEncounter_KJ.h"
#include "SWPShared.h"
#include <algorithm>
#include <cmath>
#include <limits>

using namespace SwpHelpers;
using namespace EncounterHelpers;

bool KiljaedenAnnounceDragonOrbUserAction::Execute(Event /*event*/)
{
    return IsMechanicTrackerBot(bot, SWP_MAP_ID) && UpdateKiljaedenDragonOrbUserAnnouncement(botAI);
}

bool KiljaedenMarkHandOfTheDeceiverAction::Execute(Event /*event*/)
{
    if (!IsMechanicTrackerBot(bot, SWP_MAP_ID))
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    std::vector<Unit*> const hands = GetKiljaedenHands(botAI);
    if (hands.empty())
        return false;

    ObjectGuid const markedGuid = group->GetTargetIcon(RtiTargetValue::skullIndex);
    if (std::any_of(hands.begin(), hands.end(),
            [&markedGuid](Unit* hand) { return hand->GetGUID() == markedGuid; }))
    {
        return false;
    }

    Unit* target = *std::min_element(hands.begin(), hands.end(),
        [](Unit* left, Unit* right) { return left->GetHealth() < right->GetHealth(); });

    return MarkTargetWithSkull(bot, target);
}

// Hammer of Justice!
bool KiljaedenMoveHolyPaladinIntoStunRangeAction::Execute(Event /*event*/)
{
    if (bot->getClass() != CLASS_PALADIN || !PlayerbotAI::IsHeal(bot))
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    Unit* hand = botAI->GetUnit(group->GetTargetIcon(RtiTargetValue::skullIndex));
    if (!hand || !hand->IsAlive() || hand->GetEntry() != Id(SwpNpcs::NPC_HAND_OF_THE_DECEIVER))
        return false;

    float moveX;
    float moveY;
    bool backwards;
    if (!GetStepToPosition(
            bot, hand->GetPosition(), HAND_HOLY_PALADIN_STANDOFF, nullptr, moveX, moveY,
            backwards))
    {
        return false;
    }

    return MoveTo(
        SWP_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_COMBAT, true, false);
}

bool KiljaedenControlHandsOfTheDeceiverAction::Execute(Event /*event*/)
{
    std::vector<Unit*> const hands = GetKiljaedenHands(botAI);

    for (Unit* target : hands)
    {
        if (target->GetHealthPct() <= HAND_CC_IMMUNE_HP_PERCENT)
            continue;

        if (target->HasUnitState(UNIT_STATE_STUNNED) || target->HasSilenceAura() ||
            IsKiljaedenHandControlClaimed(target))
        {
            continue;
        }

        if ((CastStunOnHand(target)) || CastSilenceOnHand(target))
        {
            ClaimKiljaedenHandControl(target);
            return true;
        }
    }

    return false;
}

bool KiljaedenControlHandsOfTheDeceiverAction::CastStunOnHand(Unit* hand)
{
    auto const castSpell = [&](auto const& spell)
    {
        return botAI->CanCastSpell(spell, hand) && botAI->CastSpell(spell, hand);
    };

    auto const castSelfAoe = [&](auto const& spell, float radius)
    {
        return bot->GetExactDist(hand) < radius && castSpell(spell);
    };

    switch (bot->getClass())
    {
        case CLASS_DRUID:
        {
            return (botAI->HasStrategy("bear", BOT_STATE_COMBAT) && castSpell("bash")) ||
                (botAI->HasStrategy("cat", BOT_STATE_COMBAT) &&
                 bot->GetComboPoints() >= 4 && castSpell("maim"));
        }
        case CLASS_MAGE:
            return castSpell(Id(SwpSpells::SPELL_DEEP_FREEZE));
        case CLASS_PALADIN:
            return castSpell("hammer of justice");
        case CLASS_ROGUE:
            return bot->GetComboPoints() >= 4 && castSpell("kidney shot");
        case CLASS_WARLOCK:
            return castSpell("shadowfury");
        case CLASS_WARRIOR:
        {
            return castSpell(Id(SwpSpells::SPELL_CONCUSSION_BLOW)) ||
                castSelfAoe("shockwave", SHOCKWAVE_RADIUS);
        }
        default:
        {
            return bot->getRace() == RACE_TAUREN &&
                castSelfAoe(Id(SwpSpells::SPELL_WAR_STOMP), SELF_AOE_RACIAL_RADIUS);
        }
    }
}

bool KiljaedenControlHandsOfTheDeceiverAction::CastSilenceOnHand(Unit* hand)
{
    auto const castSpell = [&](auto const& spell)
    {
        return botAI->CanCastSpell(spell, hand) && botAI->CastSpell(spell, hand);
    };

    switch (bot->getClass())
    {
        case CLASS_HUNTER:
            return castSpell(Id(SwpSpells::SPELL_SILENCING_SHOT));
        case CLASS_PRIEST:
            return castSpell(Id(SwpSpells::SPELL_SILENCE));
        case CLASS_DEATH_KNIGHT:
            return castSpell(Id(SwpSpells::SPELL_STRANGULATE));
        default:
        {
            return bot->getRace() == RACE_BLOODELF &&
                bot->GetExactDist(hand) < SELF_AOE_RACIAL_RADIUS && castSpell("arcane torrent");
        }
    }
}

bool KiljaedenStartDpsHoldTimerAction::Execute(Event /*event*/)
{
    return EmplaceIfUnset(KiljaedenState(bot->GetInstanceId()).dpsHoldStartMs, getMSTime());
}

bool KiljaedenSelectTargetForHoldAction::isUseful()
{
    return botAI->GetState() == BOT_STATE_NON_COMBAT;
}

bool KiljaedenSelectTargetForHoldAction::Execute(Event /*event*/)
{
    if (!PlayerbotAI::IsTank(bot))
        return SelectTarget(botAI, AI_VALUE(Unit*, "dps target"));

    return SelectTarget(botAI, AI_VALUE(Unit*, "tank target"));
}

bool KiljaedenPositionAndMoveTanksAction::Execute(Event /*event*/)
{
    if (!PlayerbotAI::IsMainTank(bot))
    {
        // This grid search captures the 3s after spawn, during which Reflections are passive and
        // neither "find target" nor standard target acquisition through "attackers" can locate it.
        if (Creature* reflection = GetNearestSinisterReflection(bot))
        {
            // Once Reflections are aggressive, tank assist can take over.
            if (reflection->GetReactState() == REACT_PASSIVE &&
                PickUpSinisterReflections(reflection))
            {
                return true;
            }

            return MoveAroundKiljaedenToReflection();
        }
    }

    constexpr float arrivalDist = 2.0f;
    float moveX;
    float moveY;
    if (!GetKiljaedenStepToPosition(bot, KILJAEDEN_TANK_POSITION, arrivalDist, moveX, moveY))
        return false;

    return MoveTo(
        SWP_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_COMBAT, true, false);
}

// When Reflections activate after 3s, they begin attack with SMART_ACTION_ATTACK_START, which sets
// a random victim. Thus, the first hit after activation immediately grabs aggro.
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

    // A charge goes straight to the Reflection, into KJ's Knockback if he is in the way.
    bool const canCharge = !IsLineThroughKiljaedenKnockback(*bot, *reflection);

    switch (bot->getClass())
    {
        case CLASS_DEATH_KNIGHT:
        {
            return castSpell("death and decay", RANGED_ABILITY_REACH) ||
                castSpell("icy touch", ICY_TOUCH_REACH);
        }
        case CLASS_DRUID:
        {
            return (canCharge && castSpell("feral charge - bear", CHARGE_REACH)) ||
                castSpell("challenging roar", TAUNT_SHOUT_RADIUS);
        }
        case CLASS_PALADIN:
        {
            return castSpell("avenger's shield", RANGED_ABILITY_REACH) ||
                castSpell("consecration", CONSECRATION_RADIUS);
        }
        case CLASS_WARRIOR:
        {
            return (canCharge && castSpell("charge", CHARGE_REACH)) ||
                castSpell("challenging shout", TAUNT_SHOUT_RADIUS);
        }
        default:
            return false;
    }
}

// The stock move to a target goes straight, so an off-tank whose Reflection is across Kil'jaeden
// goes round him on the ring until the straight line clears, then the stock move takes over.
bool KiljaedenPositionAndMoveTanksAction::MoveAroundKiljaedenToReflection()
{
    Unit* target = AI_VALUE(Unit*, "current target");
    if (!target || target->GetEntry() != Id(SwpNpcs::NPC_SINISTER_REFLECTION) ||
        !IsLineThroughKiljaedenKnockback(*bot, *target))
    {
        return false;
    }

    constexpr float arrivalDist = 0.0f;
    float moveX;
    float moveY;
    if (!GetKiljaedenStepToPosition(bot, *target, arrivalDist, moveX, moveY))
        return false;

    return MoveTo(
        SWP_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_COMBAT, true, false);
}

bool KiljaedenPositionMeleeAndAvoidArmageddonsAction::Execute(Event /*event*/)
{
    PruneExpiredKiljaedenArmageddons(bot->GetInstanceId());

    Position position;
    if (!TryGetMeleePosition(position))
        return false;

    if (!TryAdjustMeleeForArmageddon(position))
        return false;

    constexpr float arrivalDist = 2.0f;
    float moveX;
    float moveY;
    if (!GetKiljaedenStepToPosition(bot, position, arrivalDist, moveX, moveY))
        return false;

    return MoveTo(
        SWP_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_COMBAT, true, false);
}

bool KiljaedenPositionMeleeAndAvoidArmageddonsAction::TryGetMeleePosition(Position& position) const
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    if (bot->HasAura(Id(SwpSpells::SPELL_FIRE_BLOOM)) &&
        !CanClearKiljaedenFireBloomWithImmunity(
            bot, GetSwpBoss(bot, SwpBosses::DATA_KILJAEDEN)) &&
        IsOutsideKiljaedenArmageddons(bot->GetInstanceId(), KILJAEDEN_MELEE_BACK_POSITION))
    {
        position = KILJAEDEN_MELEE_BACK_POSITION;
        return true;
    }

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

bool KiljaedenPositionMeleeAndAvoidArmageddonsAction::TryAdjustMeleeForArmageddon(
    Position& position)
{
    KiljaedenEncounterState const* state = FindKiljaedenState(bot->GetInstanceId());
    if (!state || state->armageddons.empty())
        return true;

    Unit* kiljaeden = GetSwpBoss(bot, SwpBosses::DATA_KILJAEDEN);
    if (!kiljaeden || IsKiljaedenCastingDarknessOfAThousandSouls(kiljaeden) ||
        HasKiljaedenDragonAura(bot))
    {
        return true;
    }

    uint32 const instanceId = bot->GetInstanceId();
    if (IsOutsideKiljaedenArmageddons(instanceId, position))
        return true;

    bool const isSouthPosition = position.GetExactDist2d(KILJAEDEN_S_MELEE_POSITION) < 1.0f;
    Position const& swapPosition =
        isSouthPosition ? KILJAEDEN_E_MELEE_POSITION : KILJAEDEN_S_MELEE_POSITION;
    if (IsOutsideKiljaedenArmageddons(instanceId, swapPosition))
    {
        position = swapPosition;
        return true;
    }

    if (!IsOutsideKiljaedenArmageddons(instanceId, KILJAEDEN_MELEE_BACK_POSITION))
        return false;

    position = KILJAEDEN_MELEE_BACK_POSITION;
    return true;
}

bool KiljaedenPositionRangedAndAvoidArmageddonsAction::Execute(Event /*event*/)
{
    Position position;
    if (!TryGetRangedPosition(position))
        return false;

    if (!TryAdjustRangedForArmageddon(position))
        return false;

    constexpr float arrivalDist = 2.0f;
    float moveX;
    float moveY;
    if (!GetKiljaedenStepToPosition(bot, position, arrivalDist, moveX, moveY))
        return false;

    return MoveTo(
        SWP_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_COMBAT, true, false);
}

bool KiljaedenPositionRangedAndAvoidArmageddonsAction::TryGetRangedPosition(
    Position& position) const
{
    EnsureKiljaedenRangedAssignments(bot);

    KiljaedenEncounterState const* state = FindKiljaedenState(bot->GetInstanceId());
    if (!state)
        return false;

    auto const assignmentItr = state->rangedAssignments.find(bot->GetGUID());
    if (assignmentItr == state->rangedAssignments.end())
        return false;

    return TryGetKiljaedenRangedSlotPosition(assignmentItr->second, position);
}

bool KiljaedenPositionRangedAndAvoidArmageddonsAction::TryAdjustRangedForArmageddon(
    Position& position)
{
    EnsureKiljaedenRangedArmageddonAssignments(bot);
    KiljaedenEncounterState const* state = FindKiljaedenState(bot->GetInstanceId());
    if (!state)
        return true;

    auto const tempAssignmentItr = state->rangedArmageddonAssignments.find(bot->GetGUID());
    if (tempAssignmentItr == state->rangedArmageddonAssignments.end())
        return true;

    return TryGetKiljaedenRangedSlotPosition(tempAssignmentItr->second, position);
}

bool KiljaedenStackForShieldOfTheBlueAction::Execute(Event /*event*/)
{
    Position destination = KILJAEDEN_TANK_POSITION;
    if (bot->HasAura(Id(SwpSpells::SPELL_FIRE_BLOOM)))
    {
        Unit* kiljaeden = GetSwpBoss(bot, SwpBosses::DATA_KILJAEDEN);
        if (!kiljaeden)
            return false;

        if (!CanClearKiljaedenFireBloomWithImmunity(bot, kiljaeden) &&
            GetKiljaedenDarknessTimeLeftMs(kiljaeden) >= SHIELD_OF_THE_BLUE_CAST_WINDOW_MS)
        {
            destination = GetKiljaedenFireBloomWaitPosition(bot);
        }
    }

    constexpr float arrivalDist = 1.0f;
    float moveX;
    float moveY;
    if (!GetKiljaedenStepToPosition(bot, destination, arrivalDist, moveX, moveY))
        return false;

    bot->CastStop();
    bool const isMoving = MoveTo(
        SWP_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_FORCED, true, false);

    // A step is refused while the last one's wait runs, and the wait is set at the speed the step
    // started with. A snare that ends mid-step (Flame Dart) leaves the bot standing for the rest of
    // the wait, so the action keeps the tick: otherwise a cast can start then and hold the bot.
    return isMoving || botAI->CanMove();
}

bool KiljaedenUseDragonOrbAction::Execute(Event /*event*/)
{
    GameObject* closestOrb = nullptr;
    GameObject* closestInUseOrb = nullptr;
    float closestDistance = std::numeric_limits<float>::max();
    float closestInUseOrbDistance = std::numeric_limits<float>::max();
    bool orbInUse = false;

    for (auto const& orbGuid : context->GetValue<GuidVector>("kiljaeden dragon orbs")->RefGet())
    {
        GameObject* orb = botAI->GetGameObject(orbGuid);
        if (!orb)
            continue;

        float const distance = bot->GetExactDist2d(orb);
        if (orb->HasGameObjectFlag(GO_FLAG_IN_USE))
        {
            orbInUse = true;
            if (distance < closestInUseOrbDistance)
            {
                closestInUseOrb = orb;
                closestInUseOrbDistance = distance;
            }

            continue;
        }

        if (orb->HasGameObjectFlag(GO_FLAG_NOT_SELECTABLE))
            continue;

        if (distance < closestDistance)
        {
            closestOrb = orb;
            closestDistance = distance;
        }
    }

    float moveX;
    float moveY;

    // Failsafe to keep the orb user from leaving early
    if (orbInUse)
    {
        if (!closestInUseOrb)
            return false;

        if (closestInUseOrbDistance <= DRAGON_ORB_IN_USE_HOLD_DISTANCE)
            return true;

        if (!GetKiljaedenStepToPosition(
                bot, closestInUseOrb->GetPosition(), DRAGON_ORB_IN_USE_HOLD_DISTANCE, moveX,
                moveY))
        {
            return false;
        }

        return MoveTo(
            SWP_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
            MovementPriority::MOVEMENT_FORCED, true, false);
    }

    if (!closestOrb)
        return false;

    if (closestOrb->IsAtInteractDistance(*bot, closestOrb->GetInteractionDistance()))
    {
        closestOrb->Use(bot);
        KiljaedenState(bot->GetInstanceId()).dragonOrbUsers.insert(bot->GetGUID());
        return true;
    }

    float const targetDist = closestOrb->GetInteractionDistance() - 0.5f;
    float const angle = closestOrb->GetAngle(bot);
    Position const approach(
        closestOrb->GetPositionX() + std::cos(angle) * targetDist,
        closestOrb->GetPositionY() + std::sin(angle) * targetDist, bot->GetPositionZ());

    // The interaction test above is the arrival test; the step lands on the approach point.
    if (!GetKiljaedenStepToPosition(bot, approach, 0.0f, moveX, moveY))
        return false;

    return MoveTo(
        SWP_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_FORCED, true, false);
}

bool KiljaedenDragonBuffAndProtectRaidAction::Execute(Event /*event*/)
{
    Unit* kiljaeden = GetSwpBoss(bot, SwpBosses::DATA_KILJAEDEN);
    if (!kiljaeden)
        return false;

    // End remaining bot drake control after phase changes, which may not be ideal but
    // is the safer approach without knowing the player's composition or raid knowledge.
    if (kiljaeden->FindCurrentSpellBySpellId(Id(SwpSpells::SPELL_SHADOW_SPIKE)))
    {
        if (!HasKiljaedenDragonAura(bot))
            return false;

        bot->RemoveOwnedAura(
            Id(SwpSpells::SPELL_VENGEANCE_OF_THE_BLUE_FLIGHT),
            ObjectGuid::Empty, 0, AURA_REMOVE_BY_CANCEL);

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
    int32 const darknessTimeLeftMs = GetKiljaedenDarknessTimeLeftMs(kiljaeden);
    if (darknessTimeLeftMs < 0)
        return false;

    constexpr float castReadyDistanceFromStack = 3.0f;
    Position const& stackPosition = KILJAEDEN_TANK_POSITION;
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

    if (darknessTimeLeftMs < SHIELD_OF_THE_BLUE_CAST_WINDOW_MS)
    {
        // The Shield is centered on the drake, so it is cast from the stack position itself.
        // The breaths are cast from short of it so that their cones cover the stacked raid.
        constexpr float shieldDistanceFromStack = 1.0f;
        if (distanceToStack <= shieldDistanceFromStack)
            return CastKiljaedenDragonSpell(dragon, Id(SwpSpells::SPELL_SHIELD_OF_THE_BLUE));

        if (dragon->GetMotionMaster()->GetCurrentMovementGeneratorType() != POINT_MOTION_TYPE ||
            !dragon->isMoving())
        {
            dragon->GetMotionMaster()->MovePoint(
                0, stackPosition.GetPositionX(), stackPosition.GetPositionY(),
                stackPosition.GetPositionZ());
        }

        return true;
    }

    return CastKiljaedenDragonSpell(dragon, Id(SwpSpells::SPELL_DRAGON_BREATH_HASTE)) ||
        CastKiljaedenDragonSpell(dragon, Id(SwpSpells::SPELL_DRAGON_BREATH_REVITALIZE));
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
    if (distanceToTarget > KILJAEDEN_DRAGON_BREATH_STANDOFF + KILJAEDEN_DRAGON_STANDOFF_TOLERANCE ||
        (distanceToTarget > std::numeric_limits<float>::min() &&
         distanceToTarget < KILJAEDEN_DRAGON_BREATH_STANDOFF - KILJAEDEN_DRAGON_STANDOFF_TOLERANCE))
    {
        float const deltaX = target->GetPositionX() - dragon->GetPositionX();
        float const deltaY = target->GetPositionY() - dragon->GetPositionY();
        float const moveRatio =
            (distanceToTarget - KILJAEDEN_DRAGON_BREATH_STANDOFF) / distanceToTarget;
        float const moveX = dragon->GetPositionX() + deltaX * moveRatio;
        float const moveY = dragon->GetPositionY() + deltaY * moveRatio;

        dragon->GetMotionMaster()->MovePoint(0, moveX, moveY, target->GetPositionZ());
        return true;
    }

    dragon->SetFacingToObject(target);
    return CastKiljaedenDragonSpell(dragon, spellId);
}

// See KiljaedenStaleRootAfterDragonTrigger for the mechanism. This covers a drake lost
// mid-encounter; the same release runs again from the Sunwell reset once the encounter is over.
bool KiljaedenReleaseStaleRootAction::Execute(Event /*event*/)
{
    return ReleaseStaleRootFlag(bot);
}

// Melee pets attack from behind Kil'jaeden, far from the Shield of the Blue stack, so every pet is
// held for the Darkness of a Thousand Souls cast and channel, and follows its owner into the stack.
bool KiljaedenSetPetStanceAction::Execute(Event /*event*/)
{
    Unit* kiljaeden = GetSwpBoss(bot, SwpBosses::DATA_KILJAEDEN);
    return SetMinionStancesForHold(
        bot, IsKiljaedenCastingDarknessOfAThousandSouls(kiljaeden), _previousReactStates);
}

bool KiljaedenSetPetStanceAction::ResetPetStance()
{
    bool const restored = SetMinionStancesForHold(bot, false, _previousReactStates);
    _previousReactStates.clear();
    return restored;
}
