/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "SWPEncounter_Muru.h"
#include "AiObjectContext.h"
#include "DynamicObject.h"
#include "EncounterHelpers.h"
#include "Playerbots.h"
#include "SWPInstanceState.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include <utility>

using namespace EncounterHelpers;

namespace SwpHelpers
{

namespace
{

Unit* ResolveLivingUnit(PlayerbotAI* botAI, ObjectGuid const& guid)
{
    Unit* unit = botAI->GetUnit(guid);
    return unit && unit->IsAlive() ? unit : nullptr;
}

// An encounter unit that "possible targets" would list: not M'uru once he turns unselectable for
// Entropius, and not a Void Spawn a warlock has enslaved, which is friendly.
Unit* ResolveMuruEncounterTarget(PlayerbotAI* botAI, ObjectGuid const& guid)
{
    Unit* unit = ResolveLivingUnit(botAI, guid);
    return unit && !unit->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE) &&
        !unit->IsFriendlyTo(botAI->GetBot()) ? unit : nullptr;
}

float GetBerserkerStunReach(Player* bot)
{
    switch (bot->getClass())
    {
        case CLASS_DRUID:
        case CLASS_ROGUE:
        case CLASS_WARRIOR:
            return MELEE_ABILITY_REACH;
        case CLASS_PALADIN:
            return HAMMER_OF_JUSTICE_REACH;
        case CLASS_WARLOCK:
            return RANGED_ABILITY_REACH;
        default:
            return bot->getRace() == RACE_TAUREN ? SELF_AOE_RACIAL_RADIUS : 0.0f;
    }
}

float GetFuryMageInterruptReach(Player* bot)
{
    switch (bot->getClass())
    {
        case CLASS_ROGUE:
        case CLASS_WARRIOR:
            return MELEE_ABILITY_REACH;
        case CLASS_SHAMAN:
            return WIND_SHEAR_REACH;
        case CLASS_DEATH_KNIGHT:
        case CLASS_MAGE:
        case CLASS_PRIEST:
        case CLASS_WARLOCK:
            return RANGED_ABILITY_REACH;
        case CLASS_HUNTER:
            return SILENCING_SHOT_REACH;
        default:
            return 0.0f;
    }
}

bool IsFlurriedBerserker(Unit* berserker)
{
    return berserker && berserker->HasAura(Id(SwpSpells::SPELL_FLURRY)) &&
        !berserker->HasUnitState(UNIT_STATE_STUNNED);
}

bool IsCastingFelFireball(Unit* furyMage)
{
    return furyMage && furyMage->FindCurrentSpellBySpellId(Id(SwpSpells::SPELL_FEL_FIREBALL));
}

bool IsSpellFuryBuffedFuryMage(Unit* furyMage)
{
    return furyMage && furyMage->HasAura(Id(SwpSpells::SPELL_SPELL_FURY));
}

Unit* SelectNearestQualifying(
    PlayerbotAI* botAI, SwpNpcs npc, float reach, bool (*qualifies)(Unit*))
{
    Player* bot = botAI->GetBot();
    Unit* currentTarget = botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Get();

    Unit* best = nullptr;
    float bestDistance = std::numeric_limits<float>::max();

    for (ObjectGuid const& guid : GetMuruUnits(bot))
    {
        if (guid.GetEntry() != Id(npc))
            continue;

        Unit* candidate = ResolveMuruEncounterTarget(botAI, guid);
        if (!candidate || !qualifies(candidate))
            continue;

        float const distance = bot->GetExactDist(candidate);
        if (distance > reach)
            continue;

        if (candidate == currentTarget)
            return candidate;

        if (distance < bestDistance)
        {
            best = candidate;
            bestDistance = distance;
        }
    }

    return best;
}

} // end anonymous namespace

bool IsMuruPhaseActive(Unit* muru)
{
    // DamageTaken caps M'uru at exactly 1 health when phase 2 activates, and from then on M'uru
    // is invisible but remains on players' threat lists.
    return muru && muru->GetHealth() > 1;
}

// From the pre-effect's start to the zone's end.
bool IsMuruDarknessActive(Unit* muru)
{
    return muru && (muru->HasAura(Id(SwpSpells::SPELL_DARKNESS_PRE_EFFECT)) ||
        muru->GetDynObject(Id(SwpSpells::SPELL_DARKNESS)));
}

// The first MURU_DARKNESS_EARLY_WINDOW_MS from the pre-effect's start.
bool IsMuruDarknessEarly(Unit* muru)
{
    if (!muru)
        return false;

    if (muru->HasAura(Id(SwpSpells::SPELL_DARKNESS_PRE_EFFECT)))
        return true;

    DynamicObject* darkness = muru->GetDynObject(Id(SwpSpells::SPELL_DARKNESS));
    if (!darkness)
        return false;

    int32 const remainingMs =
        std::clamp(darkness->GetDuration(), 0, static_cast<int32>(MURU_DARKNESS_DURATION_MS));
    uint32 const elapsedMs =
        MURU_DARKNESS_PRE_EFFECT_MS + MURU_DARKNESS_DURATION_MS - static_cast<uint32>(remainingMs);

    return elapsedMs < MURU_DARKNESS_EARLY_WINDOW_MS;
}

bool IsMeleeTargetInMuruDarkness(Unit* muru, Unit* target)
{
    return muru && target &&
        muru->GetExactDist2d(target) <= MURU_DARKNESS_MELEE_TARGET_DISTANCE;
}

bool IsMuruPetHoldActive(Unit* muru)
{
    return IsMuruPhaseActive(muru) && IsMuruDarknessActive(muru);
}

// Damage can't kill a Dark Fiend, so a pet is never sent onto one, even when its owner targets it.
Unit* GetMuruPetTarget(PlayerbotAI* botAI, Creature* pet, Unit* muru)
{
    if (!pet)
        return nullptr;

    Unit* target = botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Get();
    if (!target || !target->IsAlive() || target->GetEntry() == Id(SwpNpcs::NPC_DARK_FIEND))
        return nullptr;

    if (IsMuruPetHoldActive(muru) && IsMeleeTargetInMuruDarkness(muru, target))
        return nullptr;

    return pet->IsValidAttackTarget(target) ? target : nullptr;
}

bool IsSpellFuryReservedForMages(Unit* furyMage)
{
    if (!furyMage)
        return false;

    Aura const* spellFury = furyMage->GetAura(Id(SwpSpells::SPELL_SPELL_FURY));
    if (!spellFury)
        return false;

    int32 const elapsedMs = spellFury->GetMaxDuration() - spellFury->GetDuration();
    return elapsedMs < static_cast<int32>(SPELL_FURY_MAGE_PRIORITY_MS);
}

// Before any bot has seen Entropius, the hold counts as active.
bool IsEntropiusDpsHoldActive(Player* bot)
{
    MuruEncounterState const* state = FindMuruState(bot->GetInstanceId());
    if (!state || !state->entropiusFirstSeenMs)
        return true;

    return getMSTimeDiff(*state->entropiusFirstSeenMs, getMSTime()) < ENTROPIUS_DPS_HOLD_MS;
}

GuidVector FindMuruUnitGuids(Player* bot)
{
    return FindCreatureGuidsAround(
        bot, MURU_CENTER_POSITION, MURU_ROOM_SEARCH_RADIUS, {
            Id(SwpNpcs::NPC_MURU),
            Id(SwpNpcs::NPC_ENTROPIUS),
            Id(SwpNpcs::NPC_VOID_SENTINEL),
            Id(SwpNpcs::NPC_VOID_SPAWN),
            Id(SwpNpcs::NPC_SHADOWSWORD_FURY_MAGE),
            Id(SwpNpcs::NPC_SHADOWSWORD_BERSERKER),
            Id(SwpNpcs::NPC_DARKNESS),
            Id(SwpNpcs::NPC_DARK_FIEND),
            Id(SwpNpcs::NPC_SINGULARITY),
        });
}

GuidVector const& GetMuruUnits(Player* bot)
{
    return GetInstanceCreatureList(
        SwpState(bot->GetInstanceId()).muruUnits, MURU_UNIT_CACHE_INTERVAL_MS, bot,
        FindMuruUnitGuids);
}

void GatherMuruEncounterTargets(PlayerbotAI* botAI, MuruEncounterTargets& targets)
{
    for (ObjectGuid const& guid : GetMuruUnits(botAI->GetBot()))
    {
        uint32 const entry = guid.GetEntry();
        if (entry == Id(SwpNpcs::NPC_DARKNESS) || entry == Id(SwpNpcs::NPC_DARK_FIEND) ||
            entry == Id(SwpNpcs::NPC_SINGULARITY))
        {
            continue;
        }

        Unit* unit = ResolveMuruEncounterTarget(botAI, guid);
        if (!unit)
            continue;

        switch (entry)
        {
            case Id(SwpNpcs::NPC_MURU):
                targets.muru = unit;
                break;
            case Id(SwpNpcs::NPC_ENTROPIUS):
                targets.entropius = unit;
                break;
            case Id(SwpNpcs::NPC_VOID_SENTINEL):
                targets.voidSentinels.push_back(unit);
                break;
            case Id(SwpNpcs::NPC_VOID_SPAWN):
                targets.voidSpawns.push_back(unit);
                break;
            case Id(SwpNpcs::NPC_SHADOWSWORD_FURY_MAGE):
                targets.furyMages.push_back(unit);
                break;
            case Id(SwpNpcs::NPC_SHADOWSWORD_BERSERKER):
                targets.berserkers.push_back(unit);
                break;
            default:
                break;
        }
    }
}

Unit* SelectNearestMuruTargetByEntry(
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

bool IsMuruAddInVoidSentinelPulse(Unit* add, std::vector<Unit*> const& voidSentinels)
{
    if (!add)
        return false;

    if (add->GetEntry() != Id(SwpNpcs::NPC_SHADOWSWORD_FURY_MAGE) &&
        add->GetEntry() != Id(SwpNpcs::NPC_SHADOWSWORD_BERSERKER) &&
        add->GetEntry() != Id(SwpNpcs::NPC_VOID_SPAWN))
    {
        return false;
    }

    for (Unit* voidSentinel : voidSentinels)
    {
        if (voidSentinel &&
            add->GetExactDist2d(voidSentinel) < MURU_MELEE_ADD_MIN_DIST_FROM_SENTINEL)
        {
            return true;
        }
    }

    return false;
}

Unit* FindMuruBerserkerToStun(PlayerbotAI* botAI)
{
    float const reach = GetBerserkerStunReach(botAI->GetBot());
    if (reach <= 0.0f)
        return nullptr;

    return SelectNearestQualifying(
        botAI, SwpNpcs::NPC_SHADOWSWORD_BERSERKER, reach, &IsFlurriedBerserker);
}

Unit* FindMuruFuryMageToInterrupt(PlayerbotAI* botAI)
{
    float const reach = GetFuryMageInterruptReach(botAI->GetBot());
    if (reach <= 0.0f)
        return nullptr;

    return SelectNearestQualifying(
        botAI, SwpNpcs::NPC_SHADOWSWORD_FURY_MAGE, reach, &IsCastingFelFireball);
}

Unit* FindMuruFuryMageToSpellsteal(PlayerbotAI* botAI)
{
    if (botAI->GetBot()->getClass() != CLASS_MAGE)
        return nullptr;

    return SelectNearestQualifying(
        botAI, SwpNpcs::NPC_SHADOWSWORD_FURY_MAGE, RANGED_ABILITY_REACH,
        &IsSpellFuryBuffedFuryMage);
}

Position const& GetAssignedVoidSentinelTankPosition(Unit* voidSentinel)
{
    ObjectGuid const sentinelGuid = voidSentinel->GetGUID();
    Position const& northPosition = MURU_VOID_SENTINEL_N_TANK_POSITION;
    Position const& eastPosition = MURU_VOID_SENTINEL_E_TANK_POSITION;

    auto& assignments = MuruState(voidSentinel->GetInstanceId()).voidSentinelTankAssignments;
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

bool IsTankingMuruVoidSentinel(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    for (ObjectGuid const& guid : GetMuruUnits(bot))
    {
        if (guid.GetEntry() != Id(SwpNpcs::NPC_VOID_SENTINEL))
            continue;

        Unit* voidSentinel = ResolveMuruEncounterTarget(botAI, guid);
        if (voidSentinel && voidSentinel->GetVictim() == bot)
            return true;
    }

    return false;
}

Creature* GetMuruSingularity(Player* bot)
{
    Creature* nearest = nullptr;
    float nearestDistance = SINGULARITY_SEARCH_RADIUS;
    for (ObjectGuid const& guid : GetMuruUnits(bot))
    {
        if (guid.GetEntry() != Id(SwpNpcs::NPC_SINGULARITY))
            continue;

        Creature* singularity = ObjectAccessor::GetCreature(*bot, guid);
        if (!singularity || !singularity->IsAlive())
            continue;

        float const distance = bot->GetDistance(singularity);
        if (distance <= nearestDistance)
        {
            nearestDistance = distance;
            nearest = singularity;
        }
    }

    return nearest;
}

std::vector<Creature*> GetMuruDarkFiends(PlayerbotAI* botAI)
{
    std::vector<Creature*> darkFiends;
    for (ObjectGuid const& guid : GetMuruUnits(botAI->GetBot()))
    {
        if (guid.GetEntry() != Id(SwpNpcs::NPC_DARK_FIEND))
            continue;

        Unit* unit = ResolveLivingUnit(botAI, guid);
        if (Creature* darkFiend = unit ? unit->ToCreature() : nullptr)
            darkFiends.push_back(darkFiend);
    }

    return darkFiends;
}

Creature* GetNearestMuruDarkFiend(PlayerbotAI* botAI, float radius)
{
    Player* bot = botAI->GetBot();
    Creature* nearest = nullptr;
    float nearestDistance = radius;

    for (Creature* darkFiend : GetMuruDarkFiends(botAI))
    {
        float const distance = bot->GetDistance(darkFiend);
        if (distance > nearestDistance)
            continue;

        nearest = darkFiend;
        nearestDistance = distance;
    }

    return nearest;
}

// A new zone is about to summon a Dark Fiend at its center.
float GetMuruVoidZoneClearance(Unit* voidZone)
{
    if (!voidZone)
        return 0.0f;

    return voidZone->HasAura(Id(SwpSpells::SPELL_VOID_ZONE_VISUAL)) ?
        DARK_FIEND_CLEARANCE : MURU_VOID_ZONE_CLEARANCE;
}

Creature* FindMuruVoidZoneToAvoid(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    Creature* nearest = nullptr;
    float nearestDistance = std::numeric_limits<float>::max();

    for (ObjectGuid const& guid : GetMuruUnits(bot))
    {
        if (guid.GetEntry() != Id(SwpNpcs::NPC_DARKNESS))
            continue;

        Unit* unit = ResolveLivingUnit(botAI, guid);
        Creature* voidZone = unit ? unit->ToCreature() : nullptr;
        if (!voidZone)
            continue;

        float const distance = bot->GetExactDist(voidZone);
        if (distance >= GetMuruVoidZoneClearance(voidZone) || distance >= nearestDistance)
            continue;

        nearest = voidZone;
        nearestDistance = distance;
    }

    return nearest;
}

Creature* FindMuruDarkFiendToAvoid(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    Creature* nearest = nullptr;
    float nearestDistance = DARK_FIEND_CLEARANCE;

    for (Creature* darkFiend : GetMuruDarkFiends(botAI))
    {
        float const distance = bot->GetExactDist(darkFiend);
        if (distance >= nearestDistance)
            continue;

        nearest = darkFiend;
        nearestDistance = distance;
    }

    return nearest;
}

// The nearest position clear of every void zone and Dark Fiend, in 16 directions out to
// MURU_CLEAR_POSITION_SEARCH_RADIUS, that the bot can reach in a straight line.
bool TryGetMuruClearPosition(PlayerbotAI* botAI, Position& destination)
{
    Player* bot = botAI->GetBot();

    std::vector<std::pair<Position, float>> hazards;
    for (ObjectGuid const& guid : GetMuruUnits(bot))
    {
        if (guid.GetEntry() != Id(SwpNpcs::NPC_DARKNESS))
            continue;

        if (Unit* voidZone = ResolveLivingUnit(botAI, guid))
            hazards.emplace_back(voidZone->GetPosition(), GetMuruVoidZoneClearance(voidZone));
    }

    for (Creature* darkFiend : GetMuruDarkFiends(botAI))
        hazards.emplace_back(darkFiend->GetPosition(), DARK_FIEND_CLEARANCE);

    auto const isClear = [&hazards](float x, float y)
    {
        return std::ranges::none_of(hazards, [x, y](std::pair<Position, float> const& hazard)
            { return hazard.first.GetExactDist2d(x, y) < hazard.second; });
    };

    constexpr uint8 directionCount = 16;
    constexpr float directionStep = 2.0f * static_cast<float>(M_PI) / directionCount;
    constexpr float ringStep = 1.0f;
    constexpr uint8 ringCount = static_cast<uint8>(MURU_CLEAR_POSITION_SEARCH_RADIUS / ringStep);

    float const botX = bot->GetPositionX();
    float const botY = bot->GetPositionY();

    for (uint8 ring = 1; ring <= ringCount; ++ring)
    {
        float const distance = ring * ringStep;
        for (uint8 direction = 0; direction < directionCount; ++direction)
        {
            float const angle = direction * directionStep;
            float const x = botX + std::cos(angle) * distance;
            float const y = botY + std::sin(angle) * distance;
            if (!isClear(x, y))
                continue;

            float stepX = 0.0f;
            float stepY = 0.0f;
            float stepZ = 0.0f;
            if (!CanTakeStepTowards(bot, x, y, distance, stepX, stepY, stepZ) ||
                !isClear(stepX, stepY))
            {
                continue;
            }

            destination = Position(stepX, stepY, bot->GetPositionZ());
            return true;
        }
    }

    return false;
}

float GetSingularityClearance(Player* bot, Unit* entropius)
{
    return entropius && entropius->GetVictim() == bot ?
        SINGULARITY_TANK_CLEARANCE : SINGULARITY_CLEARANCE;
}

Creature* FindAvailableVoidSpawnForEnslave(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();

    Creature* bestSpawn = nullptr;
    float closestDistance = std::numeric_limits<float>::max();

    for (ObjectGuid const& guid : GetMuruUnits(bot))
    {
        if (guid.GetEntry() != Id(SwpNpcs::NPC_VOID_SPAWN))
            continue;

        Unit* unit = ResolveMuruEncounterTarget(botAI, guid);
        if (!unit || unit->IsCharmed() || unit->GetCharmer() ||
            !bot->IsWithinCombatRange(unit, RANGED_ABILITY_REACH))
        {
            continue;
        }

        Creature* creature = unit->ToCreature();
        if (!creature)
            continue;

        float const distance = bot->GetExactDist2d(unit);
        if (distance >= closestDistance)
            continue;

        bestSpawn = creature;
        closestDistance = distance;
    }

    return bestSpawn;
}

// Shared encounter state

MuruEncounterState& MuruState(uint32 instanceId)
{
    std::optional<MuruEncounterState>& state = SwpState(instanceId).muru;
    if (!state)
        state.emplace();

    return *state;
}

MuruEncounterState* FindMuruState(uint32 instanceId)
{
    SwpInstanceState* state = FindSwpState(instanceId);
    return state && state->muru ? &*state->muru : nullptr;
}

}
