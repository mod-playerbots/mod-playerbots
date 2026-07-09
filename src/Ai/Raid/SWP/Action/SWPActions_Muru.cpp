/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include <algorithm>
#include <array>
#include <cmath>
#include <list>
#include <vector>

#include "SWPActions.h"
#include "SWPEncounter_Muru.h"
#include "CharmInfo.h"
#include "CreatureAI.h"
#include "Playerbots.h"
#include "RaidBossHelpers.h"
#include "TargetValue.h"

using namespace SunwellHelpers;

bool MuruMisdirectEnemiesToTanksAction::Execute(Event /*event*/)
{
    Unit* targetEnemy = nullptr;
    Unit* targetTank = nullptr;

    if (Unit* voidSentinel = AI_VALUE2(Unit*, "find target", "void sentinel"))
    {
        targetEnemy = voidSentinel;
        if (Player* firstAssistTank = GetGroupAssistTank(botAI, bot, 0))
            targetTank = firstAssistTank;
    }
    else if (Unit* entropius = AI_VALUE2(Unit*, "find target", "entropius"))
    {
        targetEnemy = entropius;
        if (Player* mainTank = GetGroupMainTank(botAI, bot))
            targetTank = mainTank;
    }

    if (!targetEnemy || !targetTank)
        return false;

    if (botAI->CanCastSpell("misdirection", targetTank))
        return botAI->CastSpell("misdirection", targetTank);

    if (bot->HasAura(static_cast<uint32>(SunwellSpells::SPELL_MISDIRECTION)) &&
        botAI->CanCastSpell("steady shot", targetEnemy))
    {
        return botAI->CastSpell("steady shot", targetEnemy);
    }

    return false;
}

bool MuruMainTankPickUpEntropiusAction::Execute(Event /*event*/)
{
    if (Unit* entropius = AI_VALUE2(Unit*, "find target", "entropius"))
        return AI_VALUE(Unit*, "current target") != entropius && Attack(entropius);

    return false;
}

bool MuruPositionRangedAction::Execute(Event /*event*/)
{
    Unit* muru = AI_VALUE2(Unit*, "find target", "m'uru");
    if (muru && muru->GetHealth() > 1)
    {
        _entropiusInitialRangedPositionReached = false;

        const Position& position = MURU_STACK_POSITION;
        constexpr float rangedGroupRadius = 2.0f;
        return MoveInside(
            SUNWELL_MAP_ID, position.GetPositionX(), position.GetPositionY(),
            position.GetPositionZ(), rangedGroupRadius, MovementPriority::MOVEMENT_COMBAT);
    }

    Unit* entropius = AI_VALUE2(Unit*, "find target", "entropius");
    MuruEncounterTargets targets;
    targets.muru = muru;
    targets.entropius = entropius;
    GatherMuruEncounterTargets(botAI, targets);

    const bool hasActiveAdds =
        !targets.voidSentinels.empty() ||
        !targets.furyMages.empty() || !targets.berserkers.empty();

    if (TryGetMuruDarknessActiveState(bot, muru))
        return false;

    if (!hasActiveAdds && !_entropiusInitialRangedPositionReached)
    {
        Position position;
        if (!TryGetEntropiusInitialRangedPosition(position))
            return false;

        constexpr float arrivalDistance = 2.0f;
        if (bot->GetExactDist2d(
                position.GetPositionX(), position.GetPositionY()) <= arrivalDistance)
        {
            _entropiusInitialRangedPositionReached = true;
            return false;
        }

        return MoveInside(
            SUNWELL_MAP_ID, position.GetPositionX(), position.GetPositionY(),
            position.GetPositionZ(), arrivalDistance, MovementPriority::MOVEMENT_COMBAT);
    }

    constexpr float safeDistFromPlayer = 4.0f;
    constexpr uint32 minInterval = 1000;
    if (Unit* nearestPlayer = GetNearestPlayerInRadius(bot, safeDistFromPlayer))
        return FleePosition(nearestPlayer->GetPosition(), safeDistFromPlayer, minInterval);

    return false;
}

bool MuruPositionRangedAction::TryGetEntropiusInitialRangedPosition(
    Position& position) const
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    std::vector<Player*> rangedMembers;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member->GetMapId() != SUNWELL_MAP_ID || !botAI->IsRanged(member))
            continue;

        rangedMembers.push_back(member);
    }

    if (rangedMembers.empty())
        return false;

    std::sort(rangedMembers.begin(), rangedMembers.end(),
        [](Player* left, Player* right) { return left->GetGUID() < right->GetGUID(); });

    size_t slotIndex = rangedMembers.size();
    for (size_t index = 0; index < rangedMembers.size(); ++index)
    {
        if (rangedMembers[index] == bot)
        {
            slotIndex = index;
            break;
        }
    }

    if (slotIndex >= rangedMembers.size())
        return false;

    constexpr float spreadRadius = 25.0f;
    const float anchorAngle = std::atan2(
        MURU_STACK_POSITION.GetPositionY() - MURU_CENTER_POSITION.GetPositionY(),
        MURU_STACK_POSITION.GetPositionX() - MURU_CENTER_POSITION.GetPositionX());
    const float angleStep =
        2.0f * static_cast<float>(M_PI) / static_cast<float>(rangedMembers.size());
    const float angle = Position::NormalizeOrientation(anchorAngle + angleStep * slotIndex);

    float destinationX = MURU_CENTER_POSITION.GetPositionX() + std::cos(angle) * spreadRadius;
    float destinationY = MURU_CENTER_POSITION.GetPositionY() + std::sin(angle) * spreadRadius;

    float destinationZ = bot->GetMapWaterOrGroundLevel(
        destinationX, destinationY, MURU_CENTER_POSITION.GetPositionZ());
    if (destinationZ <= INVALID_HEIGHT)
        destinationZ = MURU_CENTER_POSITION.GetPositionZ();

    bot->GetMap()->CheckCollisionAndGetValidCoords(
        bot, bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(),
        destinationX, destinationY, destinationZ, false);

    position = Position{ destinationX, destinationY, destinationZ };
    return true;
}

bool MuruSetDpsPriorityAction::Execute(Event /*event*/)
{
    Unit* currentTarget = context->GetValue<Unit*>("current target")->Get();
    Unit* target = ResolveMuruDpsTarget(currentTarget);

    if (target && target->GetEntry() ==
            static_cast<uint32>(SunwellNpcs::NPC_SHADOWSWORD_BERSERKER))
    {
        if (bot->getClass() == CLASS_ROGUE && !botAI->GetAura("dismantle", target) &&
            botAI->CanCastSpell("dismantle", target))
        {
            return botAI->CastSpell("dismantle", target);
        }

        if (bot->getClass() == CLASS_WARRIOR && !botAI->GetAura("disarm", target) &&
            botAI->CanCastSpell("disarm", target))
        {
            return botAI->CastSpell("disarm", target);
        }
    }

    if (target)
    {
        bool needsAttack = false;
        if (botAI->IsMelee(bot))
            needsAttack = currentTarget != target || !bot->HasUnitState(UNIT_STATE_MELEE_ATTACKING);
        else
            needsAttack = currentTarget != target;

        if (needsAttack)
            return Attack(target);
    }

    return false;
}

Unit* MuruSetDpsPriorityAction::ResolveMuruDpsTarget(Unit*& currentTarget)
{
    const bool isShadowPriest =
        bot->getClass() == CLASS_PRIEST && botAI->HasStrategy("shadow", BOT_STATE_COMBAT);
    const bool isOtherRanged = botAI->IsRanged(bot) && !isShadowPriest;

    MuruEncounterTargets targets;
    GatherMuruEncounterTargets(botAI, targets);
    Unit* muru = targets.muru;
    Unit* entropius = targets.entropius;

    if (!muru && !entropius)
        return nullptr;

    const bool isMuruPhase = muru && muru->GetHealth() > 1;
    const bool darknessActive = isMuruPhase && TryGetMuruDarknessActiveState(bot, muru);

    Unit* voidSentinel = SelectMuruEncounterTarget(currentTarget, static_cast<uint32>(
        SunwellNpcs::NPC_VOID_SENTINEL), targets.voidSentinels);
    Unit* voidSpawn = SelectMuruEncounterTarget(currentTarget, static_cast<uint32>(
        SunwellNpcs::NPC_VOID_SPAWN), targets.voidSpawns);
    Unit* furyMage = SelectMuruEncounterTarget(currentTarget, static_cast<uint32>(
        SunwellNpcs::NPC_SHADOWSWORD_FURY_MAGE), targets.furyMages);
    Unit* berserker = SelectMuruEncounterTarget(currentTarget, static_cast<uint32>(
        SunwellNpcs::NPC_SHADOWSWORD_BERSERKER), targets.berserkers);

    Player* voidSentinelVictim = nullptr;
    if (voidSentinel && voidSentinel->IsAlive())
    {
        Unit* victim = voidSentinel->GetVictim();
        if (victim)
            voidSentinelVictim = victim->ToPlayer();
    }

    const bool voidSentinelHasTankAggro = voidSentinelVictim && botAI->IsTank(voidSentinelVictim);

    auto const isAllowedPriorityTarget = [&](Unit* unit) -> bool
    {
        if (!unit || !unit->IsAlive())
            return false;

        /* constexpr float rangedInitialPhaseTargetDistance = 30.0f;
        if (isOtherRanged && isMuruPhase &&
            bot->GetExactDist2d(unit) > rangedInitialPhaseTargetDistance)
        {
            return false;
        } */

        switch (unit->GetEntry())
        {
            case static_cast<uint32>(SunwellNpcs::NPC_MURU):
                if (darknessActive && !isOtherRanged && !isShadowPriest)
                    return false;
                return unit->GetHealth() > 1;

            case static_cast<uint32>(SunwellNpcs::NPC_ENTROPIUS):
                return true;

            case static_cast<uint32>(SunwellNpcs::NPC_VOID_SENTINEL):
                return isOtherRanged && voidSentinelHasTankAggro;

            case static_cast<uint32>(SunwellNpcs::NPC_VOID_SPAWN):
                return isOtherRanged;

            case static_cast<uint32>(SunwellNpcs::NPC_SHADOWSWORD_FURY_MAGE):
            case static_cast<uint32>(SunwellNpcs::NPC_SHADOWSWORD_BERSERKER):
                if (isShadowPriest)
                    return false;
                if (isOtherRanged)
                    return true;
                return darknessActive || !isMuruPhase;

            default:
                return false;
        }
    };

    std::vector<std::pair<uint32, Unit*>> priorityTargets;
    if (isShadowPriest)
    {
        priorityTargets = {
            { static_cast<uint32>(SunwellNpcs::NPC_MURU), muru },
            { static_cast<uint32>(SunwellNpcs::NPC_ENTROPIUS), entropius }
        };
    }
    else if (isOtherRanged)
    {
        priorityTargets = {
            { static_cast<uint32>(SunwellNpcs::NPC_VOID_SENTINEL), voidSentinel },
            { static_cast<uint32>(SunwellNpcs::NPC_VOID_SPAWN), voidSpawn },
            { static_cast<uint32>(SunwellNpcs::NPC_SHADOWSWORD_FURY_MAGE), furyMage },
            { static_cast<uint32>(SunwellNpcs::NPC_SHADOWSWORD_BERSERKER), berserker },
            { static_cast<uint32>(SunwellNpcs::NPC_MURU), muru },
            { static_cast<uint32>(SunwellNpcs::NPC_ENTROPIUS), entropius }
        };
    }
    else
    {
        priorityTargets = {
            { static_cast<uint32>(SunwellNpcs::NPC_MURU), muru },
            { static_cast<uint32>(SunwellNpcs::NPC_SHADOWSWORD_FURY_MAGE), furyMage },
            { static_cast<uint32>(SunwellNpcs::NPC_SHADOWSWORD_BERSERKER), berserker },
            { static_cast<uint32>(SunwellNpcs::NPC_VOID_SPAWN), voidSpawn },
            { static_cast<uint32>(SunwellNpcs::NPC_ENTROPIUS), entropius },
        };
    }

    Unit* target = nullptr;
    for (auto const& candidate : priorityTargets)
    {
        if (isAllowedPriorityTarget(candidate.second))
        {
            target = candidate.second;
            break;
        }
    }

    Unit* stickyTarget = currentTarget;

    auto const getPriorityIndex = [&](Unit* unit) -> size_t
    {
        if (!isAllowedPriorityTarget(unit))
            return priorityTargets.size();

        for (size_t index = 0; index < priorityTargets.size(); ++index)
        {
            if (priorityTargets[index].first == unit->GetEntry())
                return index;
        }

        return priorityTargets.size();
    };

    if (stickyTarget)
    {
        const size_t currentPriority = getPriorityIndex(stickyTarget);
        const size_t desiredPriority = getPriorityIndex(target);
        if (currentPriority <= desiredPriority)
            target = stickyTarget;
    }

    if (!target)
        target = AI_VALUE(Unit*, "dps target");

    return target;
}

Unit* MuruSetDpsPriorityAction::SelectMuruEncounterTarget(
    Unit* currentTarget, uint32 entry, std::vector<Unit*> const& candidates) const
{
    Unit* selected = nullptr;
    if (currentTarget && currentTarget->IsAlive() && currentTarget->GetEntry() == entry)
        selected = currentTarget;

    constexpr float targetSwitchDistance = 10.0f;
    auto const getDistanceFromStack = [](Unit* unit)
    {
        return unit->GetExactDist2d(
            MURU_STACK_POSITION.GetPositionX(), MURU_STACK_POSITION.GetPositionY());
    };

    for (Unit* candidate : candidates)
    {
        if (!candidate)
            continue;

        if (!selected)
        {
            selected = candidate;
            continue;
        }

        if (selected == candidate)
            continue;

        const float currentDistance = getDistanceFromStack(selected);
        const float candidateDistance = getDistanceFromStack(candidate);
        if (candidateDistance + targetSwitchDistance < currentDistance)
            selected = candidate;
    }

    return selected;
}

bool MuruKillDarkFiendsWithDispelAction::Execute(Event /*event*/)
{
    Unit* muru = AI_VALUE2(Unit*, "find target", "m'uru");
    Unit* entropius = AI_VALUE2(Unit*, "find target", "entropius");
    if (!muru && !entropius)
        return false;

    const bool isMuruPhase = muru && muru->GetHealth() > 1;

    Creature* darkFiendNearMuru = nullptr;
    constexpr float searchRadius = 50.0f;
    std::list<Creature*> darkFiends;
    bot->GetCreatureListWithEntryInGrid(
        darkFiends, static_cast<uint32>(SunwellNpcs::NPC_DARK_FIEND), searchRadius);

    if (isMuruPhase)
    {
        for (Creature* creature : darkFiends)
        {
            if (creature && creature->IsAlive() && creature->GetExactDist2d(muru) < 15.0f)
            {
                darkFiendNearMuru = creature;
                break;
            }
        }
    }

    if (bot->getClass() == CLASS_PRIEST)
    {
        if (isMuruPhase)
        {
            if (darkFiendNearMuru && botAI->CanCastSpell("mass dispel", muru))
                return botAI->CastSpell("mass dispel", muru);

            for (Creature* creature : darkFiends)
            {
                if (creature && botAI->CanCastSpell("mass dispel", creature) &&
                    botAI->CastSpell("mass dispel", creature))
                {
                    return true;
                }
            }
        }

        for (Creature* creature : darkFiends)
        {
            if (creature && botAI->CanCastSpell("dispel magic", creature))
                return botAI->CastSpell("dispel magic", creature);
        }
    }
    else
    {
        for (Creature* creature : darkFiends)
        {
            if (creature && botAI->CanCastSpell("purge", creature))
                return botAI->CastSpell("purge", creature);
        }
    }

    return false;
}

bool MuruDontTouchTheDarkFiendAction::Execute(Event /*event*/)
{
    Unit* hazard = nullptr;
    Unit* darkFiend = AI_VALUE2(Unit*, "find target", "dark fiend");
    constexpr float searchDistance = 15.0f;
    Creature* darkness = bot->FindNearestCreature(
        static_cast<uint32>(SunwellNpcs::NPC_DARKNESS), searchDistance, true);

    if (darkFiend)
        hazard = darkFiend;
    else if (darkness)
        hazard = darkness;
    else
        return false;

    constexpr float safeDistance = 10.0f;
    const float distFromHazard = bot->GetDistance2d(hazard);
    if (distFromHazard < safeDistance && MoveAway(hazard, safeDistance - distFromHazard))
        return true;

    const float randomAngle = static_cast<float>(urand(0, 7)) * ANGLE_45_DEG;
    return Move(randomAngle, safeDistance - distFromHazard);
}

bool MuruTanksMoveSentinelToSafePositionAction::Execute(Event /*event*/)
{
    Unit* voidSentinel = AI_VALUE2(Unit*, "find target", "void sentinel");
    const Position& waitPosition = MURU_STACK_POSITION;
    if (!voidSentinel &&
        bot->GetExactDist2d(waitPosition.GetPositionX(), waitPosition.GetPositionY()) > 3.0f)
    {
        return MoveTo(
            SUNWELL_MAP_ID, waitPosition.GetPositionX(), waitPosition.GetPositionY(),
            waitPosition.GetPositionZ(), false, false, false, false,
            MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    if (!voidSentinel)
        return false;

    const Position* tankPosition = GetAssignedVoidSentinelTankPosition(voidSentinel);
    if (!tankPosition)
        return false;

    if (voidSentinel->GetVictim() == bot)
    {
        const float distToPosition = bot->GetExactDist2d(
            tankPosition->GetPositionX(), tankPosition->GetPositionY());

        if (distToPosition > 2.0f)
        {
            const float dX = tankPosition->GetPositionX() - bot->GetPositionX();
            const float dY = tankPosition->GetPositionY() - bot->GetPositionY();
            const float moveDist = std::min(2.25f, distToPosition);
            const float moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
            const float moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

            return MoveTo(
                SUNWELL_MAP_ID, moveX, moveY, tankPosition->GetPositionZ(), false, false,
                false, false, MovementPriority::MOVEMENT_COMBAT, true, true);
        }
    }

    if (botAI->IsAssistTankOfIndex(bot, 0, true) &&
        AI_VALUE(Unit*, "current target") != voidSentinel)
    {
        return Attack(voidSentinel);
    }

    return false;
}

const Position* MuruTanksMoveSentinelToSafePositionAction::GetAssignedVoidSentinelTankPosition(
    Unit* voidSentinel) const
{
    if (!voidSentinel)
        return nullptr;

    auto& assignments = muruVoidSentinelTankAssignments[voidSentinel->GetInstanceId()];
    auto assignmentItr = assignments.find(voidSentinel->GetGUID());
    if (assignmentItr == assignments.end())
    {
        const float northDistance = voidSentinel->GetExactDist2d(
            MURU_VOID_SENTINEL_N_TANK_POSITION.GetPositionX(),
            MURU_VOID_SENTINEL_N_TANK_POSITION.GetPositionY());

        const float eastDistance = voidSentinel->GetExactDist2d(
            MURU_VOID_SENTINEL_E_TANK_POSITION.GetPositionX(),
            MURU_VOID_SENTINEL_E_TANK_POSITION.GetPositionY());

        const uint8 assignedIndex = northDistance <= eastDistance ? 0 : 1;
        assignmentItr = assignments.emplace(voidSentinel->GetGUID(), assignedIndex).first;
    }

    const Position& north = MURU_VOID_SENTINEL_N_TANK_POSITION;
    const Position& east = MURU_VOID_SENTINEL_E_TANK_POSITION;
    return assignmentItr->second == 0 ? &north : &east;
}

bool MuruSecondAssistTankGuardRangedAction::Execute(Event /*event*/)
{
    const Position& position = MURU_ENTRANCE_POSITION;
    if (bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY()) > 1.0f)
    {
        return MoveTo(
            SUNWELL_MAP_ID, position.GetPositionX(), position.GetPositionY(),
            position.GetPositionZ(), false, false, false, false,
            MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    return false;
}

bool MuruFleeTheDarknessAction::Execute(Event /*event*/)
{
    Unit* muru = AI_VALUE2(Unit*, "find target", "m'uru");
    if (!muru)
        return false;

    constexpr uint32 targetDistThreshold = 20.0f;
    Unit* currentTarget = AI_VALUE(Unit*, "current target");
    if (currentTarget && muru->GetExactDist2d(currentTarget) > targetDistThreshold)
    {
        const Position& refPosition = botAI->IsAssistTankOfIndex(bot, 1, true) ?
            MURU_ENTRANCE_POSITION : MURU_STACK_POSITION;
        if (currentTarget->GetExactDist2d(
                refPosition.GetPositionX(), refPosition.GetPositionY()) < targetDistThreshold)
        {
            return false;
        }
    }

    MuruEncounterTargets targets;
    GatherMuruEncounterTargets(botAI, targets);
    const bool isTankingVoidSentinel = std::any_of(
        targets.voidSentinels.begin(), targets.voidSentinels.end(),
        [this](Unit* voidSentinel) {
            return voidSentinel && voidSentinel->GetVictim() == bot;
        });

    if (botAI->IsTank(bot))
    {
        if (isTankingVoidSentinel)
            return false;

        if (!botAI->IsAssistTankOfIndex(bot, 0, true) &&
            TryGetMuruDarknessEarlyState(bot, muru))
        {
            const Position& holdingPosition = botAI->IsAssistTankOfIndex(bot, 1, true) ?
                MURU_ENTRANCE_POSITION : MURU_STACK_POSITION;
            constexpr float arrivalDistance = 1.0f;

            return MoveInside(
                SUNWELL_MAP_ID, holdingPosition.GetPositionX(), holdingPosition.GetPositionY(),
                holdingPosition.GetPositionZ(), arrivalDistance, MovementPriority::MOVEMENT_FORCED);
        }

        constexpr float safeDistanceFromMuru = 20.0f;
        constexpr uint32 minInterval = 0;
        if (bot->GetExactDist2d(muru) > safeDistanceFromMuru)
            return false;

        return FleePosition(muru->GetPosition(), safeDistanceFromMuru, minInterval);
    }
    else
    {
        constexpr float stackArrivalDistance = 3.0f;
        return MoveInside(
            SUNWELL_MAP_ID, MURU_STACK_POSITION.GetPositionX(), MURU_STACK_POSITION.GetPositionY(),
            MURU_STACK_POSITION.GetPositionZ(), stackArrivalDistance,
            MovementPriority::MOVEMENT_FORCED);
    }
}

bool MuruFleeFromSingularityAction::Execute(Event /*event*/)
{
    Unit* entropius = AI_VALUE2(Unit*, "find target", "entropius");
    if (!entropius)
        return false;

    Creature* singularity = GetNearestMuruSingularity(bot);
    if (!singularity)
        return false;

    const float safeDistance = entropius->GetVictim() == bot ? 15.0f : 10.0f;
    const float currentDistance = bot->GetExactDist2d(singularity);
    if (currentDistance >= safeDistance)
        return false;

    return FleePosition(singularity->GetPosition(), safeDistance);
}

bool MuruCastStunOnShadowswordBerserkerAction::Execute(Event /*event*/)
{
    Unit* berserker = AI_VALUE2(Unit*, "find target", "shadowsword berserker");
    if (!berserker || berserker->HasUnitState(UNIT_STATE_STUNNED))
        return false;

    auto const castStun = [&](const char* spell)
    {
        return botAI->CanCastSpell(spell, berserker) && botAI->CastSpell(spell, berserker);
    };

    switch (bot->getClass())
    {
        case CLASS_DRUID:
            return castStun("bash") || castStun("maim");

        case CLASS_PALADIN:
            return castStun("hammer of justice");

        case CLASS_ROGUE:
            return castStun("kidney shot");

        case CLASS_WARLOCK:
            return castStun("shadowfury");

        case CLASS_WARRIOR:
            return castStun("concussion blow") || castStun("revenge stun") ||
                castStun("shockwave");

        default:
            return bot->getRace() == RACE_TAUREN && castStun("war stomp");
    }
}

bool MuruInterruptFelFireballAction::Execute(Event /*event*/)
{
    Unit* furyMage = AI_VALUE2(Unit*, "find target", "shadowsword fury mage");
    if (!furyMage)
        return false;

    auto const castInterrupt = [&](const char* spell)
    {
        return botAI->CanCastSpell(spell, furyMage) && botAI->CastSpell(spell, furyMage);
    };

    switch (bot->getClass())
    {
        case CLASS_DEATH_KNIGHT:
            return castInterrupt("mind freeze") || castInterrupt("strangulate");

        case CLASS_HUNTER:
            return castInterrupt("silencing shot");

        case CLASS_MAGE:
            return castInterrupt("counterspell");

        case CLASS_ROGUE:
            return castInterrupt("kick");

        case CLASS_SHAMAN:
            return castInterrupt("wind shear");

        case CLASS_WARRIOR:
            return castInterrupt("pummel") || castInterrupt("shield bash");

        default:
            return bot->getRace() == RACE_BLOODELF && castInterrupt("arcane torrent");
    }
}

bool MuruCastSpellStealOnSpellFuryAction::Execute(Event /*event*/)
{
    Unit* furyMage = AI_VALUE2(Unit*, "find target", "shadowsword fury mage");
    return furyMage && botAI->CanCastSpell("spellsteal", furyMage) &&
        botAI->CastSpell("spellsteal", furyMage);
}

bool MuruWarlockEnslaveVoidSpawnAction::Execute(Event /*event*/)
{
    if (bot->getClass() != CLASS_WARLOCK || bot->GetCharm())
        return false;

    Unit* muru = AI_VALUE2(Unit*, "find target", "m'uru");
    if (!muru)
        return false;

    Creature* voidSpawn = FindAvailableVoidSpawnForEnslave(botAI, bot);
    if (!voidSpawn)
        return false;

    if (botAI->CanCastSpell("enslave demon", voidSpawn))
        return botAI->CastSpell("enslave demon", voidSpawn);

    return false;
}

Unit* MuruEnslavedVoidSpawnAttackAction::GetControlledVoidSpawn() const
{
    Unit* voidSpawn = bot->GetCharm();
    if (!voidSpawn || !voidSpawn->IsAlive() ||
        voidSpawn->GetEntry() != static_cast<uint32>(SunwellNpcs::NPC_VOID_SPAWN))
    {
        return nullptr;
    }

    return voidSpawn;
}

bool MuruEnslavedVoidSpawnAttackAction::CommandControlledCreatureToAttack(
    Unit* controlled, Unit* target) const
{
    if (!controlled || !controlled->IsAlive() || !target || controlled->GetVictim() == target)
        return false;

    controlled->ClearUnitState(UNIT_STATE_FOLLOW);
    controlled->AttackStop();
    controlled->SetTarget(target->GetGUID());

    if (CharmInfo* charmInfo = controlled->GetCharmInfo())
    {
        charmInfo->SetIsCommandAttack(true);
        charmInfo->SetIsAtStay(false);
        charmInfo->SetIsFollowing(false);
        charmInfo->SetIsCommandFollow(false);
        charmInfo->SetIsReturning(false);
    }

    if (!controlled->IsPlayer() && controlled->IsCreature() &&
        controlled->ToCreature()->IsAIEnabled)
    {
        controlled->ToCreature()->AI()->AttackStart(target);
    }
    else
    {
        controlled->Attack(target, true);
    }

    return true;
}

bool MuruEnslavedVoidSpawnCastShadowBoltVolleyAction::Execute(Event /*event*/)
{
    if (bot->getClass() != CLASS_WARLOCK)
        return false;

    Unit* voidSpawn = GetControlledVoidSpawn();
    if (!voidSpawn)
        return false;

    Unit* target = GetVoidSpawnVolleyPriorityTarget();
    if (!target)
        return false;

    const bool commandedAttack = CommandControlledCreatureToAttack(voidSpawn, target);

    if (voidSpawn->GetExactDist2d(target) > sPlayerbotAIConfig.spellDistance)
        return commandedAttack;

    constexpr uint32 volleySpellId = static_cast<uint32>(SunwellSpells::SPELL_SHADOW_BOLT_VOLLEY);
    if (voidSpawn->HasSpellCooldown(volleySpellId))
        return commandedAttack;

    constexpr uint32 globalCooldown = 1000;
    voidSpawn->CastSpell(target, volleySpellId, true);
    voidSpawn->AddSpellCooldown(volleySpellId, 0, globalCooldown);
    return true;
}

Unit* MuruEnslavedVoidSpawnAttackAction::GetVoidSpawnVolleyPriorityTarget() const
{
    MuruEncounterTargets targets;
    GatherMuruEncounterTargets(botAI, targets);

    Unit* currentTarget = context->GetValue<Unit*>("current target")->Get();
    constexpr float targetSwitchDistance = 10.0f;
    auto const chooseNearestTarget = [&](Unit*& current, Unit* candidate)
    {
        if (!candidate)
            return;

        if (!current)
        {
            current = candidate;
            return;
        }

        if (current == candidate)
            return;

        const float currentDistance = bot->GetExactDist2d(current);
        const float candidateDistance = bot->GetExactDist2d(candidate);
        if (candidateDistance + targetSwitchDistance < currentDistance)
            current = candidate;
    };

    auto const selectEncounterTarget = [&](uint32 entry, std::vector<Unit*> const& candidates)
    {
        Unit* selected = nullptr;
        if (currentTarget && currentTarget->IsAlive() && currentTarget->GetEntry() == entry)
            selected = currentTarget;

        for (Unit* candidate : candidates)
            chooseNearestTarget(selected, candidate);

        return selected;
    };

    Unit* furyMage = selectEncounterTarget(
        static_cast<uint32>(SunwellNpcs::NPC_SHADOWSWORD_FURY_MAGE), targets.furyMages);
    Unit* berserker = selectEncounterTarget(
        static_cast<uint32>(SunwellNpcs::NPC_SHADOWSWORD_BERSERKER), targets.berserkers);
    Unit* voidSentinel = selectEncounterTarget(
        static_cast<uint32>(SunwellNpcs::NPC_VOID_SENTINEL), targets.voidSentinels);

    Unit* validMuru = targets.muru;
    if (!validMuru || validMuru->GetHealth() <= 1 || TryGetMuruDarknessActiveState(bot, validMuru))
        validMuru = nullptr;

    std::array<Unit*, 5> priorities = {
        furyMage, berserker, voidSentinel, validMuru, targets.entropius
    };

    for (Unit* target : priorities)
    {
        if (target && target->IsAlive())
            return target;
    }

    return nullptr;
}
