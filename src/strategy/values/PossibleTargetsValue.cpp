/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "PossibleTargetsValue.h"

#include "AttackersValue.h"
#include "CellImpl.h"
#include "DBCStructure.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "Playerbots.h"
#include "SharedDefines.h"
#include "SpellAuraDefines.h"
#include "SpellAuraEffects.h"
#include "SpellMgr.h"
#include "Unit.h"
#include "AreaDefines.h"
#include <unordered_map>

// Level difference thresholds for attack probability
constexpr int32 EXTREME_LEVEL_DIFF = 5;  // Don't attack if enemy is this much higher
constexpr int32 HIGH_LEVEL_DIFF = 4;     // 25% chance at +/- this difference
constexpr int32 MID_LEVEL_DIFF = 3;      // 50% chance at +/- this difference
constexpr int32 LOW_LEVEL_DIFF = 2;      // 75% chance at +/- this difference

// Cache duration before reconsidering attack decision
constexpr uint32 ATTACK_DECISION_CACHE_DURATION = 2 * MINUTE;

// Custom hash function for (botGUID, targetGUID) pairs
struct PairGuidHash
{
    std::size_t operator()(const std::pair<ObjectGuid, ObjectGuid>& pair) const
    {
        return std::hash<uint64>()(pair.first.GetRawValue()) ^
               (std::hash<uint64>()(pair.second.GetRawValue()) << 1);
    }
};

// Cache for probability-based attack decisions (Per-bot: non-global)
// Map: (botGUID, targetGUID) -> (should attack decision, timestamp)
static std::unordered_map<std::pair<ObjectGuid, ObjectGuid>, std::pair<bool, time_t>, PairGuidHash> attackDecisionCache;

void PossibleTargetsValue::FindUnits(std::list<Unit*>& targets)
{
    Acore::AnyUnfriendlyUnitInObjectRangeCheck u_check(bot, bot, range);
    Acore::UnitListSearcher<Acore::AnyUnfriendlyUnitInObjectRangeCheck> searcher(bot, targets, u_check);
    Cell::VisitObjects(bot, searcher, range);
}

bool PossibleTargetsValue::AcceptUnit(Unit* unit)
{
    if (!AttackersValue::IsPossibleTarget(unit, bot, range))
        return false;

    // Level-based PvP restrictions
    if (unit->IsPlayer())
    {
        // Self-defense - always allow fighting back
        if (bot->IsInCombat() && bot->GetVictim() == unit)
            return true;  // Already fighting

        Unit* botAttacker = bot->getAttackerForHelper();
        if (botAttacker)
        {
            if (botAttacker == unit)
                return true;  // Enemy attacking

            if (botAttacker->IsPet())
            {
                Unit* petOwner = botAttacker->GetOwner();
                if (petOwner && petOwner == unit)
                    return true;  // Enemy's pet attacking
            }
        }

        // Skip restrictions in BG/Arena
        if (bot->InBattleground() || bot->InArena())
            return true;

        // Skip restrictions if in duel with this player
        if (bot->duel && bot->duel->Opponent == unit)
            return true;

        // Capital cities - no restrictions
        uint32 zoneId = bot->GetZoneId();
        bool inCapitalCity = (zoneId == AREA_STORMWIND_CITY ||
                            zoneId == AREA_IRONFORGE        ||
                            zoneId == AREA_DARNASSUS        ||
                            zoneId == AREA_THE_EXODAR       ||
                            zoneId == AREA_ORGRIMMAR        ||
                            zoneId == AREA_THUNDER_BLUFF    ||
                            zoneId == AREA_UNDERCITY        ||
                            zoneId == AREA_SILVERMOON_CITY);

        if (inCapitalCity)
            return true;

        // Level difference check
        int32 levelDifference = unit->GetLevel() - bot->GetLevel();
        int32 absLevelDifference = std::abs(levelDifference);

        // Extreme difference - do not attack
        if (levelDifference >= EXTREME_LEVEL_DIFF)
            return false;

        // Calculate attack chance based on level difference
        uint32 attackChance = 100; // Default 100%: Bot and target's levels are very close

        // There's a chance a bot might gank on an extremly low target
        if ((absLevelDifference < EXTREME_LEVEL_DIFF && absLevelDifference >= HIGH_LEVEL_DIFF) ||
            levelDifference <= -EXTREME_LEVEL_DIFF)
            attackChance = 25;

        else if (absLevelDifference < HIGH_LEVEL_DIFF && absLevelDifference >= MID_LEVEL_DIFF)
            attackChance = 50;

        else if (absLevelDifference < MID_LEVEL_DIFF && absLevelDifference >= LOW_LEVEL_DIFF)
            attackChance = 75;

        // If probability check needed, use cache
        if (attackChance < 100)
        {
            std::pair<ObjectGuid, ObjectGuid> cacheKey = std::make_pair(bot->GetGUID(), unit->GetGUID());
            time_t currentTime = time(nullptr);

            auto it = attackDecisionCache.find(cacheKey);
            if (it != attackDecisionCache.end())
            {
                if (currentTime - it->second.second < ATTACK_DECISION_CACHE_DURATION)
                    return it->second.first;
            }

            bool shouldAttack = (urand(1, 100) <= attackChance);
            attackDecisionCache[cacheKey] = std::make_pair(shouldAttack, currentTime);
            return shouldAttack;
        }
    }

    return true;
}

void PossibleTriggersValue::FindUnits(std::list<Unit*>& targets)
{
    Acore::AnyUnfriendlyUnitInObjectRangeCheck u_check(bot, bot, range);
    Acore::UnitListSearcher<Acore::AnyUnfriendlyUnitInObjectRangeCheck> searcher(bot, targets, u_check);
    Cell::VisitObjects(bot, searcher, range);
}

bool PossibleTriggersValue::AcceptUnit(Unit* unit)
{
    if (!unit->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE))
        return false;

    Unit::AuraEffectList const& aurasPeriodicTriggerSpell =
        unit->GetAuraEffectsByType(SPELL_AURA_PERIODIC_TRIGGER_SPELL);
    Unit::AuraEffectList const& aurasPeriodicTriggerWithValueSpell =
        unit->GetAuraEffectsByType(SPELL_AURA_PERIODIC_TRIGGER_SPELL_WITH_VALUE);
    for (const Unit::AuraEffectList& list : {aurasPeriodicTriggerSpell, aurasPeriodicTriggerWithValueSpell})
    {
        for (auto i = list.begin(); i != list.end(); ++i)
        {
            AuraEffect* aurEff = *i;
            const SpellInfo* spellInfo = aurEff->GetSpellInfo();
            if (!spellInfo)
                continue;
            const SpellInfo* triggerSpellInfo =
                sSpellMgr->GetSpellInfo(spellInfo->Effects[aurEff->GetEffIndex()].TriggerSpell);
            if (!triggerSpellInfo)
                continue;
            for (int j = 0; j < MAX_SPELL_EFFECTS; j++)
            {
                if (triggerSpellInfo->Effects[j].Effect == SPELL_EFFECT_SCHOOL_DAMAGE)
                    return true;
            }
        }
    }
    return false;
}
