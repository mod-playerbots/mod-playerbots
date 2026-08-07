/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "SWPActions.h"
#include "SWPEncounter_Muru.h"
#include "CharmInfo.h"
#include "CreatureAI.h"
#include "Playerbots.h"
#include "RaidBossHelpers.h"
#include "TargetValue.h"
#include <array>
#include <cmath>
#include <list>
#include <vector>

using namespace SwpHelpers;

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

    if (bot->HasAura(Id(SwpSpells::SPELL_MISDIRECTION)) &&
        botAI->CanCastSpell("steady shot", targetEnemy))
    {
        return botAI->CastSpell("steady shot", targetEnemy);
    }

    return false;
}

bool MuruMainTankPickUpEntropiusAction::Execute(Event /*event*/)
{
    Unit* entropius = AI_VALUE2(Unit*, "find target", "entropius");
    if (!entropius || AI_VALUE(Unit*, "current target") == entropius)
        return false;

    return Attack(entropius);
}

bool MuruPositionRangedAction::Execute(Event /*event*/)
{
    Unit* muru = AI_VALUE2(Unit*, "find target", "m'uru");
    if (muru && muru->GetHealth() > 1)
    {
        _entropiusRangedPositionReached = false;

        Position const& position = MURU_STACK_POSITION;
        constexpr float rangedGroupRadius = 2.0f;
        return MoveInside(
            SWP_MAP_ID, position.GetPositionX(), position.GetPositionY(),
            position.GetPositionZ(), rangedGroupRadius, MovementPriority::MOVEMENT_COMBAT);
    }

    Unit* entropius = AI_VALUE2(Unit*, "find target", "entropius");
    MuruEncounterTargets targets;
    targets.muru = muru;
    targets.entropius = entropius;
    GatherMuruEncounterTargets(botAI, targets);

    bool const hasActiveAdds =
        !targets.voidSentinels.empty() || !targets.furyMages.empty() || !targets.berserkers.empty();

    if (TryGetMuruDarknessActiveState(bot, muru))
        return false;

    if (!hasActiveAdds && !_entropiusRangedPositionReached)
    {
        Position position;
        if (!TryGetEntropiusInitialRangedPosition(position))
            return false;

        constexpr float arrivalDistance = 2.0f;
        if (bot->GetExactDist2d(position) <= arrivalDistance)
        {
            _entropiusRangedPositionReached = true;
            return false;
        }

        return MoveInside(
            SWP_MAP_ID, position.GetPositionX(), position.GetPositionY(),
            position.GetPositionZ(), arrivalDistance, MovementPriority::MOVEMENT_COMBAT);
    }

    constexpr float safeDistFromPlayer = 4.0f;
    if (Player* nearestPlayer = GetNearestPlayerInRadius(bot, safeDistFromPlayer))
        return FleePosition(nearestPlayer->GetPosition(), safeDistFromPlayer);

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
        if (!member || member->GetMapId() != SWP_MAP_ID)
            continue;

        if (GET_PLAYERBOT_AI(member) && PlayerbotAI::IsRanged(member))
            rangedMembers.push_back(member);
    }

    if (rangedMembers.empty())
        return false;

    auto const it = std::find(rangedMembers.begin(), rangedMembers.end(), bot);
    if (it == rangedMembers.end())
        return false;

    size_t const slotIndex = std::distance(rangedMembers.begin(), it);

    constexpr float spreadRadius = 25.0f;
    float const anchorAngle = std::atan2(
        MURU_STACK_POSITION.GetPositionY() - MURU_CENTER_POSITION.GetPositionY(),
        MURU_STACK_POSITION.GetPositionX() - MURU_CENTER_POSITION.GetPositionX());
    float const angleStep =
        2.0f * static_cast<float>(M_PI) / static_cast<float>(rangedMembers.size());
    float const angle = Position::NormalizeOrientation(anchorAngle + angleStep * slotIndex);

    float const destinationX = MURU_CENTER_POSITION.GetPositionX() + std::cos(angle) * spreadRadius;
    float const destinationY = MURU_CENTER_POSITION.GetPositionY() + std::sin(angle) * spreadRadius;

    position = Position{ destinationX, destinationY, MURU_CENTER_POSITION.GetPositionZ() };
    return true;
}

bool MuruSetDpsPriorityAction::Execute(Event /*event*/)
{
    Unit* currentTarget = context->GetValue<Unit*>("current target")->Get();
    Unit* target = ResolveMuruDpsTarget(currentTarget);
    if (!target)
        return false;

    if (target->GetEntry() == Id(SwpNpcs::NPC_SHADOWSWORD_BERSERKER))
    {
        if (bot->getClass() == CLASS_ROGUE &&
            botAI->CanCastSpell("dismantle", target) && botAI->CastSpell("dismantle", target))
        {
            return true;
        }

        if (bot->getClass() == CLASS_WARRIOR &&
            botAI->CanCastSpell("disarm", target) && botAI->CastSpell("disarm", target))
        {
            return true;
        }
    }

    bool needsAttack = false;
    if (PlayerbotAI::IsMelee(bot))
        needsAttack = currentTarget != target || !bot->HasUnitState(UNIT_STATE_MELEE_ATTACKING);
    else
        needsAttack = currentTarget != target;

    if (needsAttack)
        return Attack(target);

    return false;
}

Unit* MuruSetDpsPriorityAction::ResolveMuruDpsTarget(Unit*& currentTarget)
{
    bool const isShadowPriest =
        bot->getClass() == CLASS_PRIEST && botAI->HasStrategy("shadow", BOT_STATE_COMBAT);
    bool const isOtherRanged = PlayerbotAI::IsRanged(bot) && !isShadowPriest;

    MuruEncounterTargets targets;
    GatherMuruEncounterTargets(botAI, targets);
    Unit* muru = targets.muru;
    Unit* entropius = targets.entropius;

    if (!muru && !entropius)
        return nullptr;

    if (muru && currentTarget == muru)
        context->GetValue<bool>("neglect threat")->Set(true);

    bool const isMuruPhase = muru && muru->GetHealth() > 1;
    bool const darknessActive = isMuruPhase && TryGetMuruDarknessActiveState(bot, muru);

    Unit* voidSentinel = SelectMuruEncounterTarget(
        currentTarget, Id(SwpNpcs::NPC_VOID_SENTINEL), targets.voidSentinels);
    Unit* voidSpawn = SelectMuruEncounterTarget(
        currentTarget, Id(SwpNpcs::NPC_VOID_SPAWN), targets.voidSpawns);
    Unit* furyMage = SelectMuruEncounterTarget(
        currentTarget, Id(SwpNpcs::NPC_SHADOWSWORD_FURY_MAGE), targets.furyMages);
    Unit* berserker = SelectMuruEncounterTarget(
        currentTarget, Id(SwpNpcs::NPC_SHADOWSWORD_BERSERKER), targets.berserkers);

    Player* voidSentinelVictim = nullptr;
    if (voidSentinel && voidSentinel->IsAlive())
    {
        if (Unit* victim = voidSentinel->GetVictim())
            voidSentinelVictim = victim->ToPlayer();
    }

    bool const tankHasVoidSentinelAggro =
        voidSentinelVictim && PlayerbotAI::IsTank(voidSentinelVictim);

    auto const isAllowedPriorityTarget = [&](Unit* unit) -> bool
    {
        if (!unit || !unit->IsAlive())
            return false;

        switch (unit->GetEntry())
        {
            case Id(SwpNpcs::NPC_MURU):
                if (darknessActive && !isOtherRanged && !isShadowPriest)
                    return false;
                return unit->GetHealth() > 1;

            case Id(SwpNpcs::NPC_ENTROPIUS):
                return true;

            case Id(SwpNpcs::NPC_VOID_SENTINEL):
                return isOtherRanged && tankHasVoidSentinelAggro;

            case Id(SwpNpcs::NPC_VOID_SPAWN):
                return isOtherRanged;

            case Id(SwpNpcs::NPC_SHADOWSWORD_FURY_MAGE):
            case Id(SwpNpcs::NPC_SHADOWSWORD_BERSERKER):
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
        priorityTargets =
        {
            { Id(SwpNpcs::NPC_MURU), muru },
            { Id(SwpNpcs::NPC_ENTROPIUS), entropius }
        };
    }
    else if (isOtherRanged)
    {
        priorityTargets =
        {
            { Id(SwpNpcs::NPC_VOID_SENTINEL), voidSentinel },
            { Id(SwpNpcs::NPC_VOID_SPAWN), voidSpawn },
            { Id(SwpNpcs::NPC_SHADOWSWORD_FURY_MAGE), furyMage },
            { Id(SwpNpcs::NPC_SHADOWSWORD_BERSERKER), berserker },
            { Id(SwpNpcs::NPC_MURU), muru },
            { Id(SwpNpcs::NPC_ENTROPIUS), entropius }
        };
    }
    else
    {
        priorityTargets =
        {
            { Id(SwpNpcs::NPC_MURU), muru },
            { Id(SwpNpcs::NPC_SHADOWSWORD_FURY_MAGE), furyMage },
            { Id(SwpNpcs::NPC_SHADOWSWORD_BERSERKER), berserker },
            { Id(SwpNpcs::NPC_VOID_SPAWN), voidSpawn },
            { Id(SwpNpcs::NPC_ENTROPIUS), entropius },
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
        size_t const currentPriority = getPriorityIndex(stickyTarget);
        size_t const desiredPriority = getPriorityIndex(target);
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
        return unit->GetExactDist2d(MURU_STACK_POSITION);
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

        float const currentDistance = getDistanceFromStack(selected);
        float const candidateDistance = getDistanceFromStack(candidate);
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

    bool const isMuruPhase = muru && muru->GetHealth() > 1;

    Creature* darkFiendNearMuru = nullptr;
    constexpr float searchRadius = 50.0f;
    constexpr float massDispelRange = 15.0f;
    std::list<Creature*> darkFiends;
    bot->GetCreatureListWithEntryInGrid(darkFiends, Id(SwpNpcs::NPC_DARK_FIEND), searchRadius);

    if (isMuruPhase)
    {
        for (Creature* creature : darkFiends)
        {
            if (creature && creature->IsAlive() &&
                creature->GetExactDist2d(muru) <= massDispelRange)
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
            if (darkFiendNearMuru && botAI->CanCastSpell(Id(SwpSpells::SPELL_MASS_DISPEL), muru))
                return botAI->CastSpell(Id(SwpSpells::SPELL_MASS_DISPEL), muru);

            for (Creature* creature : darkFiends)
            {
                if (creature && botAI->CanCastSpell(Id(SwpSpells::SPELL_MASS_DISPEL), creature) &&
                    botAI->CastSpell(Id(SwpSpells::SPELL_MASS_DISPEL), creature))
                {
                    return true;
                }
            }
        }

        for (Creature* creature : darkFiends)
        {
            if (creature && botAI->CanCastSpell(Id(SwpSpells::SPELL_DISPEL_MAGIC_RANK_1), creature))
                return botAI->CastSpell(Id(SwpSpells::SPELL_DISPEL_MAGIC_RANK_1), creature);
        }
    }
    else
    {
        for (Creature* creature : darkFiends)
        {
            if (creature && botAI->CanCastSpell(Id(SwpSpells::SPELL_PURGE_RANK_1), creature))
                return botAI->CastSpell(Id(SwpSpells::SPELL_PURGE_RANK_1), creature);
        }
    }

    return false;
}

bool MuruDontTouchTheDarkFiendAction::Execute(Event /*event*/)
{
    Unit* hazard = nullptr;
    Unit* darkFiend = AI_VALUE2(Unit*, "find target", "dark fiend");
    constexpr float searchRadius = 20.0f;
    Creature* darkness = bot->FindNearestCreature(
        Id(SwpNpcs::NPC_DARKNESS), searchRadius, true);

    if (darkFiend)
        hazard = darkFiend;
    else if (darkness)
        hazard = darkness;
    else
        return false;

    constexpr float safeDistance = 15.0f;
    float const distFromHazard = bot->GetDistance2d(hazard);
    if (distFromHazard > safeDistance)
        return false;

    return MoveAway(hazard, safeDistance - distFromHazard);
}

bool MuruTanksMoveSentinelToSafePositionAction::Execute(Event /*event*/)
{
    Unit* voidSentinel = AI_VALUE2(Unit*, "find target", "void sentinel");
    Position const& waitPosition = MURU_STACK_POSITION;
    if (!voidSentinel && bot->GetExactDist2d(waitPosition) > 3.0f)
    {
        return MoveTo(
            SWP_MAP_ID, waitPosition.GetPositionX(), waitPosition.GetPositionY(),
            waitPosition.GetPositionZ(), false, false, false, false,
            MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    if (!voidSentinel)
        return false;

    if (PlayerbotAI::IsAssistTankOfIndex(bot, 0, true) &&
        AI_VALUE(Unit*, "current target") != voidSentinel)
    {
        return Attack(voidSentinel);
    }

    if (voidSentinel->GetVictim() != bot || !bot->IsWithinMeleeRange(voidSentinel))
        return false;

    Position const& tankPosition = GetAssignedVoidSentinelTankPosition(voidSentinel);
    float const distToPosition = bot->GetExactDist2d(tankPosition);

    if (distToPosition <= 2.0f)
        return false;

    float const posX = tankPosition.GetPositionX();
    float const posY = tankPosition.GetPositionY();
    float const botX = bot->GetPositionX();
    float const botY = bot->GetPositionY();

    float const toPosX = posX - botX;
    float const toPosY = posY - botY;
    float const toBossX = voidSentinel->GetPositionX() - botX;
    float const toBossY = voidSentinel->GetPositionY() - botY;
    bool const backwards = (toPosX * toBossX + toPosY * toBossY) < 0.0f;

    float const maxMoveDist = backwards ? 2.25f : 3.5f;
    float const moveDist = std::min(maxMoveDist, distToPosition);
    float const moveX = botX + (toPosX / distToPosition) * moveDist;
    float const moveY = botY + (toPosY / distToPosition) * moveDist;

    return MoveTo(
        SWP_MAP_ID, moveX, moveY, tankPosition.GetPositionZ(), false, false,
        false, false, MovementPriority::MOVEMENT_COMBAT, true, backwards);
}

Position const& MuruTanksMoveSentinelToSafePositionAction::GetAssignedVoidSentinelTankPosition(
    Unit* voidSentinel)
{
    ObjectGuid const sentinelGuid = voidSentinel->GetGUID();
    Position const& northPosition = MURU_VOID_SENTINEL_N_TANK_POSITION;
    Position const& eastPosition = MURU_VOID_SENTINEL_E_TANK_POSITION;

    auto& assignments = muruVoidSentinelTankAssignments[voidSentinel->GetInstanceId()];
    auto assignmentItr = assignments.find(sentinelGuid);
    if (assignmentItr == assignments.end())
    {
        float const northDistance = voidSentinel->GetExactDist2d(northPosition);
        float const eastDistance = voidSentinel->GetExactDist2d(eastPosition);

        uint8 const assignedIndex = northDistance <= eastDistance ? 0 : 1;
        assignmentItr = assignments.emplace(sentinelGuid, assignedIndex).first;
    }

    return assignmentItr->second == 0 ? northPosition : eastPosition;
}

bool MuruSecondAssistTankGuardRangedAction::Execute(Event /*event*/)
{
    Position const& position = MURU_ENTRANCE_POSITION;
    if (bot->GetExactDist2d(position) <= 1.0f)
        return false;

    return MoveTo(
        SWP_MAP_ID, position.GetPositionX(), position.GetPositionY(), position.GetPositionZ(),
        false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
}

bool MuruFleeTheDarknessAction::Execute(Event /*event*/)
{
    Unit* muru = AI_VALUE2(Unit*, "find target", "m'uru");
    if (!muru)
        return false;

    Position const& entrancePosition = MURU_ENTRANCE_POSITION;
    Position const& stackPosition = MURU_STACK_POSITION;

    constexpr float targetDistThreshold = 20.0f;
    Unit* currentTarget = AI_VALUE(Unit*, "current target");
    if (currentTarget && muru->GetExactDist2d(currentTarget) > targetDistThreshold)
    {
        Position const& refPosition = PlayerbotAI::IsAssistTankOfIndex(bot, 1, true) ?
            entrancePosition : stackPosition;
        if (currentTarget->GetExactDist2d(refPosition) <= targetDistThreshold)
            return false;
    }

    MuruEncounterTargets targets;
    GatherMuruEncounterTargets(botAI, targets);
    bool const isTankingVoidSentinel = std::any_of(
        targets.voidSentinels.begin(), targets.voidSentinels.end(),
        [this](Unit* voidSentinel) { return voidSentinel && voidSentinel->GetVictim() == bot; });

    if (PlayerbotAI::IsTank(bot))
    {
        if (isTankingVoidSentinel)
            return false;

        if (!PlayerbotAI::IsAssistTankOfIndex(bot, 0, true) &&
            TryGetMuruDarknessEarlyState(bot, muru))
        {
            Position const& holdingPosition = PlayerbotAI::IsAssistTankOfIndex(bot, 1, true) ?
                entrancePosition : stackPosition;
            constexpr float arrivalDistance = 1.0f;

            return MoveInside(
                SWP_MAP_ID, holdingPosition.GetPositionX(), holdingPosition.GetPositionY(),
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
            SWP_MAP_ID, stackPosition.GetPositionX(), stackPosition.GetPositionY(),
            stackPosition.GetPositionZ(), stackArrivalDistance, MovementPriority::MOVEMENT_FORCED);
    }
}

bool MuruFleeFromSingularityAction::Execute(Event /*event*/)
{
    Unit* entropius = AI_VALUE2(Unit*, "find target", "entropius");
    if (!entropius)
        return false;

    constexpr float searchRadius = 30.0f;
    Creature* singularity = bot->FindNearestCreature(
        Id(SwpNpcs::NPC_SINGULARITY), searchRadius, true);
    if (!singularity)
        return false;

    float const safeDistance = entropius->GetVictim() == bot ? 20.0f : 15.0f;
    float const currentDistance = bot->GetExactDist2d(singularity);
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
            return castStun("concussion blow") || castStun("revenge stun") || castStun("shockwave");

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
    return furyMage &&
        botAI->CanCastSpell(Id(SwpSpells::SPELL_SPELLSTEAL), furyMage) &&
        botAI->CastSpell(Id(SwpSpells::SPELL_SPELLSTEAL), furyMage);
}

bool MuruWarlockEnslaveVoidSpawnAction::Execute(Event /*event*/)
{
    if (bot->getClass() != CLASS_WARLOCK || bot->GetCharm())
        return false;

    Unit* muru = AI_VALUE2(Unit*, "find target", "m'uru");
    if (!muru)
        return false;

    Creature* voidSpawn = FindAvailableVoidSpawnForEnslave(bot);
    if (!voidSpawn)
        return false;

    return botAI->CanCastSpell("enslave demon", voidSpawn) &&
        botAI->CastSpell("enslave demon", voidSpawn);
}

Unit* MuruEnslavedVoidSpawnAttackAction::GetControlledVoidSpawn() const
{
    Unit* voidSpawn = bot->GetCharm();
    if (!voidSpawn || !voidSpawn->IsAlive() ||
        voidSpawn->GetEntry() != Id(SwpNpcs::NPC_VOID_SPAWN))
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

    bool const commandedAttack = CommandControlledCreatureToAttack(voidSpawn, target);

    if (voidSpawn->GetExactDist2d(target) > sPlayerbotAIConfig.spellDistance)
        return commandedAttack;

    constexpr uint32 volleySpellId = Id(SwpSpells::SPELL_SHADOW_BOLT_VOLLEY);
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

        float const currentDistance = bot->GetExactDist2d(current);
        float const candidateDistance = bot->GetExactDist2d(candidate);
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
        Id(SwpNpcs::NPC_SHADOWSWORD_FURY_MAGE), targets.furyMages);
    Unit* berserker = selectEncounterTarget(
        Id(SwpNpcs::NPC_SHADOWSWORD_BERSERKER), targets.berserkers);
    Unit* voidSentinel = selectEncounterTarget(
        Id(SwpNpcs::NPC_VOID_SENTINEL), targets.voidSentinels);

    Unit* validMuru = targets.muru;
    if (!validMuru || validMuru->GetHealth() <= 1 || TryGetMuruDarknessActiveState(bot, validMuru))
        validMuru = nullptr;

    std::array<Unit*, 5> priorities = {
        furyMage, berserker, voidSentinel, validMuru, targets.entropius };

    for (Unit* target : priorities)
    {
        if (target && target->IsAlive())
            return target;
    }

    return nullptr;
}
