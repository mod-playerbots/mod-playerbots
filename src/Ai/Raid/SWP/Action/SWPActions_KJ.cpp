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

bool KiljaedenMoveAwayFromFelfirePortalAction::Execute(Event /*event*/)
{
    constexpr float searchRadius = 20.0f;
    Unit* felfirePortal = bot->FindNearestCreature(
        static_cast<uint32>(SunwellNpcs::NPC_FELFIRE_PORTAL), searchRadius, true);
    if (!felfirePortal)
        return false;

    constexpr float safeDistance = 15.0f;
    const float currentDistance = bot->GetDistance2d(felfirePortal);
    if (currentDistance >= safeDistance)
        return false;

    if (botAI->IsTank(bot))
        return MoveAway(felfirePortal, safeDistance - currentDistance, true);
    else
        return FleePosition(felfirePortal->GetPosition(), safeDistance);

    return false;
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
    Group* group = bot->GetGroup();
    if (!group || !botAI->IsRanged(bot))
        return false;

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

    if (closestDistance < 3.0f)
    {
        closestOrb->Use(bot);
        kiljaedenDragonOrbUseTimes[bot->GetGUID().GetCounter()] = getMSTime();
        return true;
    }

    return MoveTo(
        SUNWELL_MAP_ID, closestOrb->GetPositionX(), closestOrb->GetPositionY(),
        closestOrb->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_FORCED, true, false);
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
        _inDarkness = false;
        _shieldCastThisDarkness = false;
        _darknessStartMs = 0;
        _lastDarknessCastMsLeft = 0;

        if (HasKiljaedenDragonAura(bot))
        {
            bot->RemoveAura(
                static_cast<uint32>(SunwellSpells::SPELL_VENGEANCE_OF_THE_BLUE_FLIGHT));
            return true;
        }

        return false;
    }

    if (IsKiljaedenCastingDarknessOfAThousandSouls(kiljaeden))
    {
        if (!_inDarkness)
        {
            _inDarkness = true;
            _shieldCastThisDarkness = false;
            _darknessStartMs = getMSTime();
        }

        return ExecuteDuringDarknessOfAThousandSouls(kiljaeden);
    }

    _inDarkness = false;
    _shieldCastThisDarkness = false;
    _darknessStartMs = 0;
    _lastDarknessCastMsLeft = 0;

    return ExecuteOutsideDarknessOfAThousandSouls();
}

bool KiljaedenControlDragonAction::ExecuteDuringDarknessOfAThousandSouls(Unit* kiljaeden)
{
    Unit* dragon = GetKiljaedenControlledDragon(bot);
    if (!dragon)
        return false;

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

    const uint32 darknessCastTimeLeft = darknessSpell->GetCastTimeRemaining();
    bool const darknessCastReset = _lastDarknessCastMsLeft > 0 &&
        darknessCastTimeLeft > _lastDarknessCastMsLeft + 250;

    if (!_inDarkness || darknessCastReset)
    {
        _inDarkness = true;
        _shieldCastThisDarkness = false;
        _darknessStartMs = getMSTime();
    }

    if (darknessCastTimeLeft > 3000)
    {
        if (CastKiljaedenDragonSpell(
                dragon, static_cast<uint32>(SunwellSpells::SPELL_DRAGON_BREATH_HASTE)))
        {
            _lastDarknessCastMsLeft = darknessCastTimeLeft;
            return true;
        }

        if (CastKiljaedenDragonSpell(
                dragon, static_cast<uint32>(SunwellSpells::SPELL_DRAGON_BREATH_REVITALIZE)))
        {
            _lastDarknessCastMsLeft = darknessCastTimeLeft;
            return true;
        }
    }

    if (!_shieldCastThisDarkness && darknessCastTimeLeft < 4500)
    {
        bool const castedShield = CastKiljaedenDragonSpell(
            dragon, static_cast<uint32>(SunwellSpells::SPELL_SHIELD_OF_THE_BLUE));

        if (castedShield)
            _shieldCastThisDarkness = true;

        _lastDarknessCastMsLeft = darknessCastTimeLeft;
        return castedShield;
    }

    _lastDarknessCastMsLeft = darknessCastTimeLeft;

    return false;
}

bool KiljaedenControlDragonAction::ExecuteOutsideDarknessOfAThousandSouls()
{
    Unit* dragon = GetKiljaedenControlledDragon(bot);
    if (!dragon)
        return false;

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
