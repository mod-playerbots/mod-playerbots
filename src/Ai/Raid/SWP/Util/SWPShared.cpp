/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "SWPShared.h"
#include "EncounterHelpers.h"
#include "PetDefines.h"
#include "Playerbots.h"
#include "SWPInstanceState.h"
#include "ThreatHelpers.h"
#include <array>
#include <mutex>
#include <string_view>
#include <unordered_map>
#include <utility>

using namespace EncounterHelpers;

namespace SwpHelpers
{

namespace
{

std::mutex swpStateMutex;
std::unordered_map<uint32, SwpInstanceState> swpStates;

constexpr std::string_view BossName(SwpBosses boss)
{
    switch (boss)
    {
        case SwpBosses::DATA_KALECGOS:
            return "kalecgos";
        case SwpBosses::DATA_BRUTALLUS:
            return "brutallus";
        case SwpBosses::DATA_FELMYST:
            return "felmyst";
        // Sacrolash is killed first, so Alythess stands for the encounter.
        case SwpBosses::DATA_EREDAR_TWINS:
            return "grand warlock alythess";
        case SwpBosses::DATA_MURU:
            return "m'uru";
        case SwpBosses::DATA_KILJAEDEN:
            return "kil'jaeden";
        default:
            return {};
    }
}

} // end anonymous namespace

Unit* GetSwpBoss(Player* bot, SwpBosses boss)
{
    if (!IsBossInProgress(bot, SWP_MAP_ID, Id(boss)))
        return nullptr;

    return FindThreatTarget(bot, BossName(boss));
}

float GetCenteredArcSlotAngleOffset(uint8 slotIndex, uint8 slotCount, float arcWidth)
{
    if (slotCount <= 1)
        return 0.0f;

    float const angleStep = arcWidth / static_cast<float>(slotCount - 1);
    if (slotCount % 2 == 1)
    {
        if (slotIndex == 0)
            return 0.0f;

        uint8 const stepIndex = (slotIndex + 1) / 2;
        float angleOffset = angleStep * stepIndex;
        if (slotIndex % 2 == 0)
            angleOffset = -angleOffset;

        return angleOffset;
    }

    float const halfStep = angleStep / 2.0f;
    uint8 const pairIndex = slotIndex / 2;
    float angleOffset = halfStep + angleStep * pairIndex;
    if (slotIndex % 2 == 1)
        angleOffset = -angleOffset;

    return angleOffset;
}

uint32 GetManualCastCooldown(uint32 spellId)
{
    constexpr uint32 minGlobalCooldown = 1000; // Spell.cpp MIN_GCD

    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
    if (!spellInfo)
        return minGlobalCooldown;

    uint32 cooldownMs = spellInfo->GetRecoveryTime();
    if (spellInfo->CategoryRecoveryTime > cooldownMs)
        cooldownMs = spellInfo->CategoryRecoveryTime;
    if (spellInfo->StartRecoveryTime > cooldownMs)
        cooldownMs = spellInfo->StartRecoveryTime;

    return cooldownMs ? cooldownMs : minGlobalCooldown;
}

uint32 GetManualCastGlobalCooldown(uint32 spellId)
{
    constexpr uint32 minGlobalCooldown = 1000; // Spell.cpp MIN_GCD

    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
    if (!spellInfo)
        return minGlobalCooldown;

    if (spellInfo->StartRecoveryTime)
        return spellInfo->StartRecoveryTime;

    // A charmed caster still gets MIN_GCD for a cooldownless spell.
    return spellInfo->RecoveryTime || spellInfo->CategoryRecoveryTime ? 0 : minGlobalCooldown;
}

bool MisdirectTargetToTank(PlayerbotAI* botAI, Unit* target, Player* tank)
{
    if (!target || !tank)
        return false;

    if (botAI->CanCastSpell(Id(SwpSpells::SPELL_MISDIRECTION), tank))
        return botAI->CastSpell(Id(SwpSpells::SPELL_MISDIRECTION), tank);

    if (!botAI->GetBot()->HasAura(Id(SwpSpells::SPELL_MISDIRECTION_AURA)))
        return false;

    return botAI->CanCastSpell("steady shot", target) && botAI->CastSpell("steady shot", target);
}

uint32 GetReadySpellLock(Creature* pet, Unit* target)
{
    if (!pet || !target || !pet->IsAlive() || pet->GetEntry() != NPC_FELHUNTER)
        return 0;

    if (!pet->IsWithinCombatRange(target, RANGED_ABILITY_REACH) || !pet->IsWithinLOSInMap(target))
        return 0;

    static constexpr std::array spellLockRanks = {
        Id(SwpSpells::SPELL_SPELL_LOCK_2),
        Id(SwpSpells::SPELL_SPELL_LOCK_1),
    };

    for (uint32 spellLock : spellLockRanks)
    {
        if (pet->HasSpell(spellLock))
            return pet->HasSpellCooldown(spellLock) ? 0 : spellLock;
    }

    return 0;
}

bool GetTankPathStep(
    Player* bot, Position const& position, float arrivalDist, Unit* tankedUnit, float& stepX,
    float& stepY, bool& backwards)
{
    backwards = false;
    if (bot->GetExactDist2d(position) <= arrivalDist)
        return false;

    // Whether the step is backwards is known only once the path is, so a tank takes the backward
    // length either way rather than paying for a second path.
    bool const tanking = tankedUnit && tankedUnit->GetVictim() == bot &&
        bot->IsWithinMeleeRange(tankedUnit);
    float const stepDistance = tanking ? PATH_BACKWARD_STEP_DISTANCE : PATH_STEP_DISTANCE;
    if (!GetPathStepTowardPoint(bot, position, arrivalDist, stepDistance, stepX, stepY))
        return false;

    float const botX = bot->GetPositionX();
    float const botY = bot->GetPositionY();
    backwards = tanking &&
        (stepX - botX) * (tankedUnit->GetPositionX() - botX) +
        (stepY - botY) * (tankedUnit->GetPositionY() - botY) < 0.0f;

    return true;
}

// Volatile Fiends

bool IsNearGauntlet(Player* bot)
{
    return bot->GetExactDist(GAUNTLET_CENTER) <= GAUNTLET_RADIUS;
}

// Gauntlet fiends: the nearest, engaged or not, since they explode right after engaging.
// Static fiends: only those engaged (on attackers) since they are harmless until they die.
ObjectGuid FindSwpVolatileFiendGuid(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    if (IsNearGauntlet(bot))
    {
        Creature* fiend = bot->FindNearestCreature(
            Id(SwpNpcs::NPC_VOLATILE_FIEND), VOLATILE_FIEND_SEARCH_RADIUS);

        return fiend ? fiend->GetGUID() : ObjectGuid::Empty;
    }

    ObjectGuid nearestGuid;
    float nearestDistance = VOLATILE_FIEND_SEARCH_RADIUS;
    AiObjectContext* context = botAI->GetAiObjectContext();
    for (ObjectGuid const& guid : context->GetValue<GuidVector>("attackers")->RefGet())
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || unit->GetEntry() != Id(SwpNpcs::NPC_VOLATILE_FIEND) || !unit->IsAlive())
            continue;

        float const distance = bot->GetExactDist(unit);
        if (distance <= nearestDistance)
        {
            nearestDistance = distance;
            nearestGuid = guid;
        }
    }

    return nearestGuid;
}

bool IsVolatileFiendInterceptor(Player* bot)
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    if (Player* assistTank = GetGroupAssistTank(bot, 0))
        return assistTank == bot;

    return GetGroupMainTank(bot) == bot;
}

// Apocalypse Guards

Unit* GetInfernalDefenseGuard(PlayerbotAI* botAI)
{
    Unit* guard = nullptr;
    AiObjectContext* context = botAI->GetAiObjectContext();
    for (ObjectGuid const& guid : context->GetValue<GuidVector>("attackers")->RefGet())
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || unit->GetEntry() != Id(SwpNpcs::NPC_APOCALYPSE_GUARD) || !unit->IsAlive() ||
            !unit->HasAura(Id(SwpSpells::SPELL_INFERNAL_DEFENSE)))
        {
            continue;
        }

        if (!guard || guid < guard->GetGUID())
            guard = unit;
    }

    return guard;
}

// Shared encounter state

SwpInstanceState& SwpState(uint32 instanceId)
{
    std::lock_guard lock(swpStateMutex);
    return swpStates[instanceId];
}

SwpInstanceState* FindSwpState(uint32 instanceId)
{
    std::lock_guard lock(swpStateMutex);
    auto it = swpStates.find(instanceId);
    return it != swpStates.end() ? &it->second : nullptr;
}

bool SwpResetInstance(uint32 instanceId)
{
    std::lock_guard lock(swpStateMutex);
    auto it = swpStates.find(instanceId);
    if (it == swpStates.end())
        return false;

    SwpInstanceState& state = it->second;
    bool const wasSet = state.kalecgos || state.brutallus || state.felmyst ||
        state.eredarTwins || state.muru || state.kiljaeden;
    SwpInstanceState kept;
    kept.alythessPaladinTank = state.alythessPaladinTank;
    kept.kiljaedenDragonOrbUser = state.kiljaedenDragonOrbUser;
    state = std::move(kept);
    return wasSet;
}

bool UpdateSwpClassRoles(PlayerbotAI* botAI)
{
    SwpInstanceState& state = SwpState(botAI->GetBot()->GetInstanceId());
    bool const classRolesChanged = UpdateClassRoleAssignments(botAI, SWP_MAP_ID, {
        { ALYTHESS_PALADIN_TANK_ROLE, state.alythessPaladinTank },
    });

    return UpdateKiljaedenDragonOrbUser(botAI) || classRolesChanged;
}

}
