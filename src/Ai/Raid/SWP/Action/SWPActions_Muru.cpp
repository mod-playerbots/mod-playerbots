/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "SWPActions.h"
#include "EncounterHelpers.h"
#include "Playerbots.h"
#include "SWPEncounter_Muru.h"
#include "SWPShared.h"
#include "ThreatHelpers.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <iterator>
#include <utility>

using namespace SwpHelpers;
using namespace EncounterHelpers;

bool MuruMisdirectEnemyToTankAction::Execute(Event /*event*/)
{
    Unit* enemy = nullptr;
    Player* tank = nullptr;

    if (Unit* voidSentinel = FindThreatTarget(bot, "void sentinel");
        voidSentinel && voidSentinel->GetHealthPct() > MURU_MISDIRECT_MIN_TARGET_HP_PERCENT)
    {
        enemy = voidSentinel;
        tank = GetGroupAssistTank(bot, 0);
    }
    else if (Unit* entropius = FindThreatTarget(bot, "entropius");
        entropius && entropius->GetHealthPct() > MURU_MISDIRECT_MIN_TARGET_HP_PERCENT)
    {
        enemy = entropius;
        tank = GetGroupMainTank(bot);
    }

    if (!enemy || !tank || !tank->IsAlive())
        return false;

    return MisdirectTargetToTank(botAI, enemy, tank);
}

bool MuruMainTankPickUpEntropiusAction::Execute(Event /*event*/)
{
    Unit* entropius = FindThreatTarget(bot, "entropius");
    if (!entropius)
        return false;

    return AI_VALUE(Unit*, "current target") != entropius && Attack(entropius);
}

bool MuruPositionRangedByPhaseAction::Execute(Event /*event*/)
{
    Unit* muru = GetSwpBoss(bot, SwpBosses::DATA_MURU);
    if (IsMuruPhaseActive(muru))
    {
        _entropiusRangedPositionReached = false;

        float moveX;
        float moveY;
        bool backwards;
        if (!GetStepToPosition(
                bot, MURU_STACK_POSITION, MURU_RANGED_STACK_ARRIVAL_DISTANCE, nullptr, moveX, moveY,
                backwards))
        {
            return false;
        }

        return MoveTo(
            SWP_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
            MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    if (IsMuruDarknessActive(muru))
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

        if (bot->GetExactDist2d(position) <= ENTROPIUS_RANGED_ARRIVAL_DISTANCE)
        {
            _entropiusRangedPositionReached = true;
            return false;
        }

        float moveX;
        float moveY;
        bool backwards;
        if (!GetStepToPosition(
                bot, position, ENTROPIUS_RANGED_ARRIVAL_DISTANCE, nullptr, moveX, moveY, backwards))
        {
            return false;
        }

        return MoveTo(
            SWP_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
            MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    constexpr float safeDistFromPlayer = 4.0f;
    Player* nearestPlayer = GetNearestPlayerInRadius(bot, safeDistFromPlayer);
    return nearestPlayer && FleePosition(nearestPlayer->GetPosition(), safeDistFromPlayer);
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

    // Select Entropius without attacking him while the DPS hold is active.
    if (target->GetEntry() == Id(SwpNpcs::NPC_ENTROPIUS) && IsEntropiusDpsHoldActive(bot))
    {
        return IsAttackingTarget(bot, target) ? StopAttackingTarget(bot, target) :
            SelectTarget(botAI, target);
    }

    return currentTarget != target && Attack(target);
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

    // Start the DPS hold's clock.
    if (entropius)
        EmplaceIfUnset(MuruState(bot->GetInstanceId()).entropiusFirstSeenMs, getMSTime());

    if (muru && currentTarget == muru)
        context->GetValue<bool>("neglect threat")->Set(true);

    bool const isMuruPhase = IsMuruPhaseActive(muru);
    bool const darknessActive = IsMuruDarknessActive(muru);

    bool const isMelee = PlayerbotAI::IsMelee(bot);
    Unit* addStickyTarget = currentTarget;
    if (isMelee && (darknessActive || !targets.voidSentinels.empty()))
    {
        auto const isUnsafe = [&](Unit* add)
        {
            return IsMuruAddInVoidSentinelPulse(add, targets.voidSentinels) ||
                (darknessActive && IsMeleeTargetInMuruDarkness(muru, add));
        };

        targets.furyMages.erase(
            std::remove_if(targets.furyMages.begin(), targets.furyMages.end(), isUnsafe),
            targets.furyMages.end());
        targets.berserkers.erase(
            std::remove_if(targets.berserkers.begin(), targets.berserkers.end(), isUnsafe),
            targets.berserkers.end());

        if (isUnsafe(addStickyTarget))
            addStickyTarget = nullptr;
    }

    Position const& origin = MURU_STACK_POSITION;
    Unit* voidSentinel = SelectNearestMuruTargetByEntry(
        currentTarget, Id(SwpNpcs::NPC_VOID_SENTINEL), targets.voidSentinels, origin);
    Unit* voidSpawn = SelectNearestMuruTargetByEntry(
        currentTarget, Id(SwpNpcs::NPC_VOID_SPAWN), targets.voidSpawns, origin);
    Unit* furyMage = SelectNearestMuruTargetByEntry(
        addStickyTarget, Id(SwpNpcs::NPC_SHADOWSWORD_FURY_MAGE), targets.furyMages, origin);
    Unit* berserker = SelectNearestMuruTargetByEntry(
        addStickyTarget, Id(SwpNpcs::NPC_SHADOWSWORD_BERSERKER), targets.berserkers, origin);

    Player* voidSentinelVictim = nullptr;
    if (voidSentinel)
    {
        if (Unit* victim = voidSentinel->GetVictim())
            voidSentinelVictim = victim->ToPlayer();
    }

    // Void Sentinel: Attack only if a tank has aggro or if it is below 10% health.
    bool const isVoidSentinelAllowed =
        ((voidSentinel && voidSentinel->GetHealthPct() < 10.0f) ||
         (voidSentinelVictim && PlayerbotAI::IsTank(voidSentinelVictim)));

    auto const isAllowedPriorityTarget = [&](Unit* unit) -> bool
    {
        if (!unit || !unit->IsAlive())
            return false;

        // Melee don't attack anything Darkness covers (M'uru himself is handled below).
        if (isMelee && darknessActive && unit->GetEntry() != Id(SwpNpcs::NPC_MURU) &&
            IsMeleeTargetInMuruDarkness(muru, unit))
        {
            return false;
        }

        switch (unit->GetEntry())
        {
            // Melee will stay on M'uru as long as Darkness is not active.
            // Shadow Priests stay on M'uru through all of phase 1.
            case Id(SwpNpcs::NPC_MURU):
            {
                if (!isMuruPhase)
                    return false;
                return isOtherRanged || isShadowPriest || !darknessActive;
            }
            case Id(SwpNpcs::NPC_ENTROPIUS):
                return true;
            case Id(SwpNpcs::NPC_VOID_SENTINEL):
            {
                if (isShadowPriest)
                    return !isMuruPhase && isVoidSentinelAllowed;
                if (isOtherRanged)
                    return isVoidSentinelAllowed;
                return false;
            }
            case Id(SwpNpcs::NPC_VOID_SPAWN):
                return isOtherRanged;
            case Id(SwpNpcs::NPC_SHADOWSWORD_FURY_MAGE):
            case Id(SwpNpcs::NPC_SHADOWSWORD_BERSERKER):
            {
                if (isMelee && IsMuruAddInVoidSentinelPulse(unit, targets.voidSentinels))
                    return false;
                if (isShadowPriest)
                    return !isMuruPhase;
                if (isOtherRanged)
                    return true;
                return darknessActive || !isMuruPhase;
            }
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
            { Id(SwpNpcs::NPC_VOID_SENTINEL), voidSentinel },
            { Id(SwpNpcs::NPC_SHADOWSWORD_FURY_MAGE), furyMage },
            { Id(SwpNpcs::NPC_SHADOWSWORD_BERSERKER), berserker },
            { Id(SwpNpcs::NPC_ENTROPIUS), entropius },
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
            { Id(SwpNpcs::NPC_ENTROPIUS), entropius },
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
        target = isMelee ? (isMuruPhase ? muru : nullptr) : AI_VALUE(Unit*, "dps target");

    return target;
}

bool MuruKillDarkFiendsWithDispelAction::Execute(Event /*event*/)
{
    Unit* muru = GetSwpBoss(bot, SwpBosses::DATA_MURU);
    Unit* entropius = FindThreatTarget(bot, "entropius");
    if (!muru && !entropius)
        return false;

    std::vector<Creature*> darkFiends = GetMuruDarkFiends(botAI);

    // Mass Dispel on M'uru reaches 15 yd round him, whatever the bot's distance to the fiends.
    constexpr float massDispelRadius = 15.0f;
    bool const isFiendNearMuru = IsMuruPhaseActive(muru) &&
        std::ranges::any_of(darkFiends, [&](Creature* darkFiend)
            { return darkFiend && darkFiend->GetExactDist2d(muru) < massDispelRadius; });

    // Dispel Magic, Mass Dispel, and Purge all reach 30 yd. A failed cast out of range would still
    // turn the bot, so only fiends in range are tried, nearest first.
    std::erase_if(darkFiends, [this](Creature* darkFiend)
        { return !darkFiend || !darkFiend->IsAlive() ||
            !bot->IsWithinCombatRange(darkFiend, RANGED_ABILITY_REACH); });
    std::ranges::sort(darkFiends, {}, [this](Creature* darkFiend)
        { return bot->GetExactDist(darkFiend); });

    auto const castOnFiend = [&](uint32 spellId)
    {
        return std::ranges::any_of(darkFiends, [&](Creature* darkFiend)
            { return botAI->CanCastSpell(spellId, darkFiend) &&
                botAI->CastSpell(spellId, darkFiend); });
    };

    if (bot->getClass() != CLASS_PRIEST)
        return castOnFiend(Id(SwpSpells::SPELL_PURGE_RANK_1));

    if (IsMuruPhaseActive(muru))
    {
        uint32 const massDispel = Id(SwpSpells::SPELL_MASS_DISPEL);
        if (isFiendNearMuru && bot->IsWithinCombatRange(muru, MASS_DISPEL_REACH) &&
            botAI->CanCastSpell(massDispel, muru) && botAI->CastSpell(massDispel, muru))
        {
            return true;
        }

        if (castOnFiend(massDispel))
            return true;
    }

    return castOnFiend(Id(SwpSpells::SPELL_DISPEL_MAGIC_RANK_1));
}

bool MuruTanksMoveSentinelToSafePositionAction::Execute(Event /*event*/)
{
    Unit* voidSentinel = FindThreatTarget(bot, "void sentinel");
    if (!voidSentinel)
    {
        float moveX;
        float moveY;
        bool backwards;
        if (!GetStepToPosition(
                bot, MURU_STACK_POSITION, MURU_TANK_STACK_ARRIVAL_DISTANCE, nullptr, moveX, moveY,
                backwards))
        {
            return false;
        }

        return MoveTo(
            SWP_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
            MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    if (PlayerbotAI::IsAssistTankOfIndex(bot, 0, true) &&
        AI_VALUE(Unit*, "current target") != voidSentinel)
    {
        return Attack(voidSentinel);
    }

    if (voidSentinel->GetVictim() != bot || !bot->IsWithinMeleeRange(voidSentinel))
        return false;

    float moveX;
    float moveY;
    bool backwards;
    if (!GetStepToPosition(
            bot, GetAssignedVoidSentinelTankPosition(voidSentinel),
            MURU_VOID_SENTINEL_TANK_ARRIVAL_DISTANCE, voidSentinel, moveX, moveY, backwards))
    {
        return false;
    }

    return MoveTo(
        SWP_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_COMBAT, true, backwards);
}

bool MuruSecondAssistTankGuardRangedAction::Execute(Event /*event*/)
{
    float moveX;
    float moveY;
    bool backwards;
    if (!GetStepToPosition(
            bot, MURU_ENTRANCE_POSITION, MURU_ENTRANCE_ARRIVAL_DISTANCE, nullptr, moveX, moveY,
            backwards))
    {
        return false;
    }

    return MoveTo(
        SWP_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_COMBAT, true, false);
}

bool MuruMeleeFleeTheDarknessAction::Execute(Event /*event*/)
{
    Unit* muru = GetSwpBoss(bot, SwpBosses::DATA_MURU);
    if (!muru)
        return false;

    Position const& entrancePosition = MURU_ENTRANCE_POSITION;
    Position const& stackPosition = MURU_STACK_POSITION;

    Unit* currentTarget = AI_VALUE(Unit*, "current target");
    if (currentTarget && !IsMeleeTargetInMuruDarkness(muru, currentTarget))
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

        if (!PlayerbotAI::IsAssistTankOfIndex(bot, 0, true) && IsMuruDarknessEarly(muru))
        {
            Position const& holdingPosition = PlayerbotAI::IsAssistTankOfIndex(bot, 1, true) ?
                entrancePosition : stackPosition;
            return StepToPosition(holdingPosition, MURU_TANK_HOLDING_ARRIVAL_DISTANCE);
        }

        constexpr uint32 minInterval = 0;
        if (bot->GetExactDist2d(muru) > MURU_DARKNESS_CLEARANCE)
            return false;

        return FleePosition(muru->GetPosition(), MURU_DARKNESS_CLEARANCE, minInterval);
    }

    return StepToPosition(stackPosition, MURU_MELEE_STACK_ARRIVAL_DISTANCE);
}

bool MuruMeleeFleeTheDarknessAction::StepToPosition(Position const& position, float arrivalDistance)
{
    float moveX;
    float moveY;
    bool backwards;
    if (!GetStepToPosition(bot, position, arrivalDistance, nullptr, moveX, moveY, backwards))
        return false;

    return MoveTo(
        SWP_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_FORCED, true, false);
}

bool MuruCastStunOnBerserkerAction::Execute(Event /*event*/)
{
    Unit* berserker = FindMuruBerserkerToStun(botAI);
    if (!berserker)
        return false;

    auto const castStun = [&](auto const& spell)
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
            return castStun(Id(SwpSpells::SPELL_CONCUSSION_BLOW)) || castStun("shockwave");
        default:
            return bot->getRace() == RACE_TAUREN && castStun(Id(SwpSpells::SPELL_WAR_STOMP));
    }
}

bool MuruInterruptFelFireballAction::Execute(Event /*event*/)
{
    Unit* furyMage = FindMuruFuryMageToInterrupt(botAI);
    if (!furyMage)
        return false;

    auto const castInterrupt = [&](auto const& spell)
    {
        return botAI->CanCastSpell(spell, furyMage) && botAI->CastSpell(spell, furyMage);
    };

    switch (bot->getClass())
    {
        case CLASS_DEATH_KNIGHT:
        {
            return (bot->IsWithinMeleeRange(furyMage) &&
                    castInterrupt(Id(SwpSpells::SPELL_MIND_FREEZE))) ||
                castInterrupt(Id(SwpSpells::SPELL_STRANGULATE));
        }
        case CLASS_HUNTER:
            return castInterrupt(Id(SwpSpells::SPELL_SILENCING_SHOT));
        case CLASS_MAGE:
            return castInterrupt(Id(SwpSpells::SPELL_COUNTERSPELL));
        case CLASS_PRIEST:
            return castInterrupt(Id(SwpSpells::SPELL_SILENCE));
        case CLASS_ROGUE:
            return castInterrupt(Id(SwpSpells::SPELL_KICK));
        case CLASS_SHAMAN:
            return castInterrupt(Id(SwpSpells::SPELL_WIND_SHEAR));
        case CLASS_WARLOCK:
        {
            Pet* pet = bot->GetPet();
            uint32 const spellLock = GetReadySpellLock(pet, furyMage);
            return spellLock && pet->CastSpell(furyMage, spellLock, false) == SPELL_CAST_OK;
        }
        case CLASS_WARRIOR:
            return CastWarriorInterrupt(furyMage);
        default:
            return false;
    }
}

// Pummel requires Berserker Stance. This function makes Arms Warriors stance change, which keeps
// 10 rage from Stance Mastery, Pummel's cost, allowing Pummel to be cast on the next tick.
bool MuruInterruptFelFireballAction::CastWarriorInterrupt(Unit* furyMage)
{
    if (bot->GetShapeshiftForm() == FORM_BERSERKERSTANCE)
    {
        return botAI->CanCastSpell(Id(SwpSpells::SPELL_PUMMEL), furyMage) &&
            botAI->CastSpell(Id(SwpSpells::SPELL_PUMMEL), furyMage);
    }

    if (botAI->CanCastSpell(Id(SwpSpells::SPELL_SHIELD_BASH), furyMage) &&
        botAI->CastSpell(Id(SwpSpells::SPELL_SHIELD_BASH), furyMage))
    {
        return true;
    }

    if (PlayerbotAI::IsTank(bot))
        return false;

    constexpr uint32 pummelRageCost = 100; // 10 rage
    uint32 const pummel = Id(SwpSpells::SPELL_PUMMEL);
    if (!bot->HasSpell(pummel) || bot->HasSpellCooldown(pummel) ||
        bot->GetPower(POWER_RAGE) < pummelRageCost)
    {
        return false;
    }

    return botAI->CanCastSpell(Id(SwpSpells::SPELL_BERSERKER_STANCE), bot) &&
        botAI->CastSpell(Id(SwpSpells::SPELL_BERSERKER_STANCE), bot);
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

bool MuruVoidSpawnCastShadowBoltVolleyAction::Execute(Event /*event*/)
{
    Unit* voidSpawn = GetControlledVoidSpawn();
    if (!voidSpawn)
        return false;

    Unit* target = GetVoidSpawnVolleyPriorityTarget(voidSpawn);
    if (!target)
        return false;

    bool const commandedAttack = CommandPetAttack(voidSpawn->ToCreature(), target);

    if (voidSpawn->GetExactDist2d(target) >
        MURU_SHADOW_BOLT_VOLLEY_RADIUS + target->GetCombatReach())
    {
        return commandedAttack;
    }

    constexpr uint32 volleySpellId = Id(SwpSpells::SPELL_SHADOW_BOLT_VOLLEY);
    if (voidSpawn->HasSpellCooldown(volleySpellId))
        return commandedAttack;

    voidSpawn->CastSpell(target, volleySpellId, true);
    voidSpawn->AddSpellCooldown(volleySpellId, 0, GetManualCastCooldown(volleySpellId));
    return true;
}

Unit* MuruVoidSpawnCastShadowBoltVolleyAction::GetControlledVoidSpawn() const
{
    Unit* voidSpawn = bot->GetCharm();
    if (!voidSpawn || !voidSpawn->IsAlive() ||
        voidSpawn->GetEntry() != Id(SwpNpcs::NPC_VOID_SPAWN))
    {
        return nullptr;
    }

    return voidSpawn;
}

Unit* MuruVoidSpawnCastShadowBoltVolleyAction::GetVoidSpawnVolleyPriorityTarget(
    Unit* voidSpawn) const
{
    MuruEncounterTargets targets;
    GatherMuruEncounterTargets(botAI, targets);

    Position const& origin = voidSpawn->GetPosition();
    Unit* currentTarget = AI_VALUE(Unit*, "current target");

    Unit* furyMage = SelectNearestMuruTargetByEntry(
        currentTarget, Id(SwpNpcs::NPC_SHADOWSWORD_FURY_MAGE), targets.furyMages, origin);
    Unit* berserker = SelectNearestMuruTargetByEntry(
        currentTarget, Id(SwpNpcs::NPC_SHADOWSWORD_BERSERKER), targets.berserkers, origin);
    Unit* voidSentinel = SelectNearestMuruTargetByEntry(
        currentTarget, Id(SwpNpcs::NPC_VOID_SENTINEL), targets.voidSentinels, origin);

    Unit* validMuru = targets.muru;
    if (!IsMuruPhaseActive(validMuru) || IsMuruDarknessActive(validMuru))
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

bool MuruMoveClearOfVoidZonesAndFiendsAction::Execute(Event /*event*/)
{
    Position destination;
    if (!TryGetMuruClearPosition(botAI, destination))
        return false;

    float moveX;
    float moveY;
    bool backwards;
    if (!GetStepToPosition(
            bot, destination, MURU_CLEAR_POSITION_ARRIVAL_DISTANCE, nullptr, moveX, moveY,
            backwards))
    {
        return false;
    }

    bot->CastStop();
    return MoveTo(
        SWP_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_FORCED, true, false);
}

bool MuruEscapeTheSingularityAction::Execute(Event /*event*/)
{
    Unit* entropius = FindThreatTarget(bot, "entropius");
    if (!entropius)
        return false;

    Creature* singularity = GetMuruSingularity(bot);
    if (!singularity || !singularity->IsAlive())
        return false;

    float const clearance = GetSingularityClearance(bot, entropius);
    if (bot->GetExactDist(singularity) >= clearance)
        return false;

    return FleePosition(singularity->GetPosition(), clearance);
}

// Pets are held through Darkness so that their owners' attacks don't send them into it. A held pet
// attacks only the targets it is commanded onto, and those are outside Darkness.
bool MuruSetPetStanceAction::Execute(Event /*event*/)
{
    Unit* muru = GetSwpBoss(bot, SwpBosses::DATA_MURU);
    return SetMinionStancesForHold(bot, IsMuruPetHoldActive(muru), _previousReactStates);
}

bool MuruSetPetStanceAction::ResetPetStance()
{
    bool const restored = SetMinionStancesForHold(bot, false, _previousReactStates);
    _previousReactStates.clear();
    return restored;
}

bool MuruCommandPetTargetAction::Execute(Event /*event*/)
{
    Pet* pet = bot->GetPet();
    if (!pet || !pet->IsAlive())
        return false;

    Unit* muru = GetSwpBoss(bot, SwpBosses::DATA_MURU);
    if (Unit* target = GetMuruPetTarget(botAI, pet, muru))
        return CommandPetAttack(pet, target);

    // A held pet's commanded target can move into Darkness.
    if (!IsMuruPetHoldActive(muru) || !IsMeleeTargetInMuruDarkness(muru, pet->GetVictim()))
        return false;

    CallBackPets(bot);
    return true;
}
