/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "SWPActions.h"
#include "CharmInfo.h"
#include "CreatureAI.h"
#include "EncounterHelpers.h"
#include "Playerbots.h"
#include "SWPEncounter_Muru.h"
#include "SWPSharedConstants.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <iterator>
#include <list>
#include <utility>

using namespace SwpHelpers;
using namespace EncounterHelpers;

namespace
{

Unit* SelectNearestByEntry(
    Unit* currentTarget, uint32 entry, std::vector<Unit*> const& candidates, Position const& origin)
{
    Unit* selected = nullptr;
    if (currentTarget && currentTarget->IsAlive() && currentTarget->GetEntry() == entry)
        selected = currentTarget;

    for (Unit* candidate : candidates)
    {
        if (!candidate || selected == candidate)
            continue;

        if (!selected)
        {
            selected = candidate;
            continue;
        }

        if (candidate->GetExactDist2d(origin) + MURU_TARGET_SWITCH_MARGIN <
            selected->GetExactDist2d(origin))
        {
            selected = candidate;
        }
    }

    return selected;
}

} // end anonymous namespace

bool MuruMisdirectEnemiesToTanksAction::Execute(Event /*event*/)
{
    Unit* enemy = nullptr;
    Unit* tank = nullptr;

    Unit* voidSentinel = AI_VALUE2(Unit*, "find target", "void sentinel");
    Unit* entropius = AI_VALUE2(Unit*, "find target", "entropius");

    if (voidSentinel && voidSentinel->GetHealthPct() > MURU_MISDIRECT_MIN_TARGET_HP_PERCENT)
    {
        enemy = voidSentinel;
        tank = GetGroupAssistTank(bot, 0);
    }
    else if (entropius && entropius->GetHealthPct() > MURU_MISDIRECT_MIN_TARGET_HP_PERCENT)
    {
        enemy = entropius;
        tank = GetGroupMainTank(bot);
    }

    if (!enemy || !tank || !tank->IsAlive())
        return false;

    if (botAI->CanCastSpell("misdirection", tank))
        return botAI->CastSpell("misdirection", tank);

    if (!bot->HasAura(Id(SwpSpells::SPELL_MISDIRECTION)))
        return false;

    return botAI->CanCastSpell("steady shot", enemy) && botAI->CastSpell("steady shot", enemy);
}

bool MuruMainTankPickUpEntropiusAction::Execute(Event /*event*/)
{
    Unit* entropius = AI_VALUE2(Unit*, "find target", "entropius");
    if (!entropius)
        return false;

    return AI_VALUE(Unit*, "current target") != entropius && Attack(entropius);
}

bool MuruPositionRangedByPhaseAction::Execute(Event /*event*/)
{
    Unit* muru = AI_VALUE2(Unit*, "find target", "m'uru");
    if (IsMuruPhaseActive(muru))
    {
        _entropiusRangedPositionReached = false;

        Position const& position = MURU_STACK_POSITION;
        constexpr float rangedGroupRadius = 2.0f;
        return MoveInside(
            SWP_MAP_ID, position.GetPositionX(), position.GetPositionY(),
            position.GetPositionZ(), rangedGroupRadius, MovementPriority::MOVEMENT_COMBAT);
    }

    if (TryGetMuruDarknessActiveState(bot, muru))
        return false;

    MuruEncounterTargets targets;
    GatherMuruEncounterTargets(botAI, targets);

    bool const hasActiveAdds =
        !targets.voidSentinels.empty() || !targets.furyMages.empty() || !targets.berserkers.empty();

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

bool MuruPositionRangedByPhaseAction::TryGetEntropiusInitialRangedPosition(
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

bool MuruAssignDpsPriorityAction::Execute(Event /*event*/)
{
    Unit* currentTarget = AI_VALUE(Unit*, "current target");
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

    return needsAttack && Attack(target);
}

Unit* MuruAssignDpsPriorityAction::ResolveMuruDpsTarget(Unit* currentTarget)
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

    bool const isMuruPhase = IsMuruPhaseActive(muru);
    bool const darknessActive = isMuruPhase && TryGetMuruDarknessActiveState(bot, muru);

    Position const& origin = MURU_STACK_POSITION;
    Unit* voidSentinel = SelectNearestByEntry(
        currentTarget, Id(SwpNpcs::NPC_VOID_SENTINEL), targets.voidSentinels, origin);
    Unit* voidSpawn = SelectNearestByEntry(
        currentTarget, Id(SwpNpcs::NPC_VOID_SPAWN), targets.voidSpawns, origin);
    Unit* furyMage = SelectNearestByEntry(
        currentTarget, Id(SwpNpcs::NPC_SHADOWSWORD_FURY_MAGE), targets.furyMages, origin);
    Unit* berserker = SelectNearestByEntry(
        currentTarget, Id(SwpNpcs::NPC_SHADOWSWORD_BERSERKER), targets.berserkers, origin);

    Player* voidSentinelVictim = nullptr;
    if (voidSentinel)
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
                if (!isMuruPhase)
                    return false;

                // Shadow Priests stay on M'uru through all of phase 1
                return isOtherRanged || isShadowPriest || !darknessActive;

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
        if (currentPriority < priorityTargets.size() && currentPriority <= desiredPriority)
            target = stickyTarget;
    }

    if (!target)
        target = AI_VALUE(Unit*, "dps target");

    return target;
}

bool MuruKillDarkFiendsWithDispelAction::Execute(Event /*event*/)
{
    Unit* muru = AI_VALUE2(Unit*, "find target", "m'uru");
    Unit* entropius = AI_VALUE2(Unit*, "find target", "entropius");
    if (!muru && !entropius)
        return false;

    bool const isMuruPhase = IsMuruPhaseActive(muru);

    Creature* darkFiendNearMuru = nullptr;
    constexpr float massDispelRange = 15.0f;
    std::list<Creature*> darkFiends;
    bot->GetCreatureListWithEntryInGrid(
        darkFiends, Id(SwpNpcs::NPC_DARK_FIEND), DARK_FIEND_DISPEL_SEARCH_RADIUS);

    if (isMuruPhase)
    {
        for (Creature* creature : darkFiends)
        {
            if (creature && creature->IsAlive() &&
                creature->GetExactDist2d(muru) < massDispelRange)
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

bool MuruTanksMoveSentinelToSafePositionAction::Execute(Event /*event*/)
{
    Unit* voidSentinel = AI_VALUE2(Unit*, "find target", "void sentinel");
    if (!voidSentinel)
    {
        Position const& waitPosition = MURU_STACK_POSITION;
        constexpr float arrivalDistance = 3.0f;
        if (bot->GetExactDist2d(waitPosition) <= arrivalDistance)
            return false;

        return MoveTo(
            SWP_MAP_ID, waitPosition.GetPositionX(), waitPosition.GetPositionY(),
            waitPosition.GetPositionZ(), false, false, false, false,
            MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    if (PlayerbotAI::IsAssistTankOfIndex(bot, 0, true) &&
        AI_VALUE(Unit*, "current target") != voidSentinel)
    {
        return Attack(voidSentinel);
    }

    if (voidSentinel->GetVictim() != bot || !bot->IsWithinMeleeRange(voidSentinel))
        return false;

    constexpr float arrivalDist = 2.0f;
    float moveX;
    float moveY;
    bool backwards;
    if (!GetTankPositionStep(
            bot, GetAssignedVoidSentinelTankPosition(voidSentinel), arrivalDist, voidSentinel,
            moveX, moveY, backwards))
    {
        return false;
    }

    return MoveTo(
        SWP_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_COMBAT, true, backwards);
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

bool MuruMeleeFleeTheDarknessAction::Execute(Event /*event*/)
{
    Unit* muru = AI_VALUE2(Unit*, "find target", "m'uru");
    if (!muru)
        return false;

    Position const& entrancePosition = MURU_ENTRANCE_POSITION;
    Position const& stackPosition = MURU_STACK_POSITION;

    Unit* currentTarget = AI_VALUE(Unit*, "current target");
    if (currentTarget && muru->GetExactDist2d(currentTarget) > DARKNESS_SAFE_DISTANCE)
    {
        Position const& refPosition = PlayerbotAI::IsAssistTankOfIndex(bot, 1, true) ?
            entrancePosition : stackPosition;
        if (currentTarget->GetExactDist2d(refPosition) <= MURU_HOLDING_POSITION_RADIUS)
            return false;
    }

    if (PlayerbotAI::IsTank(bot))
    {
        if (IsTankingMuruVoidSentinel(botAI))
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

        constexpr uint32 minInterval = 0;
        if (bot->GetExactDist2d(muru) > DARKNESS_SAFE_DISTANCE)
            return false;

        return FleePosition(muru->GetPosition(), DARKNESS_SAFE_DISTANCE, minInterval);
    }

    constexpr float stackArrivalDistance = 3.0f;
    return MoveInside(
        SWP_MAP_ID, stackPosition.GetPositionX(), stackPosition.GetPositionY(),
        stackPosition.GetPositionZ(), stackArrivalDistance, MovementPriority::MOVEMENT_FORCED);
}

bool MuruCastStunOnBerserkerAction::Execute(Event /*event*/)
{
    Unit* berserker = FindMuruBerserkerToStun(botAI);
    if (!berserker)
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
            return castStun("concussion blow") || castStun("shockwave");

        default:
            // Tauren
            return castStun("war stomp");
    }
}

bool MuruInterruptFelFireballAction::Execute(Event /*event*/)
{
    Unit* furyMage = FindMuruFuryMageToInterrupt(botAI);
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

        case CLASS_PALADIN:
            return castInterrupt("avenger's shield");

        case CLASS_PRIEST:
            return castInterrupt("silence");

        case CLASS_ROGUE:
            return castInterrupt("kick");

        case CLASS_SHAMAN:
            return castInterrupt("wind shear");

        case CLASS_WARLOCK:
            return castInterrupt("spell lock");

        case CLASS_WARRIOR:
            return castInterrupt("pummel") || castInterrupt("shield bash");

        default:
            return false;
    }
}

bool MuruCastSpellStealOnSpellFuryAction::Execute(Event /*event*/)
{
    Unit* furyMage = FindMuruFuryMageToSpellsteal(botAI);
    return furyMage &&
        botAI->CanCastSpell(Id(SwpSpells::SPELL_SPELLSTEAL), furyMage) &&
        botAI->CastSpell(Id(SwpSpells::SPELL_SPELLSTEAL), furyMage);
}

bool MuruWarlockEnslaveVoidSpawnAction::Execute(Event /*event*/)
{
    Creature* voidSpawn = FindAvailableVoidSpawnForEnslave(botAI);
    return voidSpawn &&
        botAI->CanCastSpell("enslave demon", voidSpawn) &&
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

bool MuruVoidSpawnCastShadowBoltVolleyAction::Execute(Event /*event*/)
{
    Unit* voidSpawn = GetControlledVoidSpawn();
    if (!voidSpawn)
        return false;

    Unit* target = GetVoidSpawnVolleyPriorityTarget(voidSpawn);
    if (!target)
        return false;

    bool const commandedAttack = CommandControlledCreatureToAttack(voidSpawn, target);

    if (voidSpawn->GetDistance(target) > MURU_SHADOW_BOLT_VOLLEY_RADIUS)
        return commandedAttack;

    constexpr uint32 volleySpellId = Id(SwpSpells::SPELL_SHADOW_BOLT_VOLLEY);
    if (voidSpawn->HasSpellCooldown(volleySpellId))
        return commandedAttack;

    constexpr uint32 globalCooldown = 1000;
    voidSpawn->CastSpell(target, volleySpellId, true);
    voidSpawn->AddSpellCooldown(volleySpellId, 0, globalCooldown);
    return true;
}

Unit* MuruEnslavedVoidSpawnAttackAction::GetVoidSpawnVolleyPriorityTarget(Unit* voidSpawn) const
{
    MuruEncounterTargets targets;
    GatherMuruEncounterTargets(botAI, targets);

    Position const& origin = voidSpawn->GetPosition();
    Unit* currentTarget = AI_VALUE(Unit*, "current target");

    Unit* furyMage = SelectNearestByEntry(
        currentTarget, Id(SwpNpcs::NPC_SHADOWSWORD_FURY_MAGE), targets.furyMages, origin);
    Unit* berserker = SelectNearestByEntry(
        currentTarget, Id(SwpNpcs::NPC_SHADOWSWORD_BERSERKER), targets.berserkers, origin);
    Unit* voidSentinel = SelectNearestByEntry(
        currentTarget, Id(SwpNpcs::NPC_VOID_SENTINEL), targets.voidSentinels, origin);

    Unit* validMuru = targets.muru;
    if (!IsMuruPhaseActive(validMuru) || TryGetMuruDarknessActiveState(bot, validMuru))
        validMuru = nullptr;

    std::array<Unit*, 5> priorities = {
        furyMage, berserker, voidSentinel, validMuru, targets.entropius };

    for (Unit* target : priorities)
    {
        if (target)
            return target;
    }

    return nullptr;
}

bool MuruKeepDistanceFromDarkFiendsAction::Execute(Event /*event*/)
{
    bot->CastStop();

    if (Creature* voidZone = FindMuruVoidZoneToAvoid(botAI))
    {
        float const distFromVoidZone = bot->GetDistance2d(voidZone);
        return MoveAway(voidZone, VOID_ZONE_SAFE_DISTANCE - distFromVoidZone);
    }

    Creature* darkFiend =
        bot->FindNearestCreature(Id(SwpNpcs::NPC_DARK_FIEND), DARK_FIEND_AVOID_SEARCH_RADIUS);
    if (!darkFiend)
        return false;

    float const distFromFiend = bot->GetDistance2d(darkFiend);
    if (distFromFiend > DARK_FIEND_SAFE_DISTANCE)
        return false;

    return MoveAway(darkFiend, DARK_FIEND_SAFE_DISTANCE - distFromFiend);
}

bool MuruEscapeTheSingularityAction::Execute(Event /*event*/)
{
    Unit* entropius = AI_VALUE2(Unit*, "find target", "entropius");
    if (!entropius)
        return false;

    Creature* singularity = botAI->GetCreature(AI_VALUE(ObjectGuid, "muru singularity"));
    if (!singularity || !singularity->IsAlive())
        return false;

    float const safeDistance = entropius->GetVictim() == bot ? 20.0f : 15.0f;
    float const currentDistance = bot->GetExactDist2d(singularity);
    if (currentDistance >= safeDistance)
        return false;

    return FleePosition(singularity->GetPosition(), safeDistance);
}
