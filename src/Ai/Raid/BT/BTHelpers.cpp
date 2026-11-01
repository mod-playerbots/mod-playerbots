/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "BTHelpers.h"
#include "CellImpl.h"
#include "CreatureAI.h"
#include "DynamicObject.h"
#include "EncounterHelpers.h"
#include "GameObject.h"
#include "GameTime.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "PetDefines.h"
#include "Playerbots.h"
#include "RtiTargetValue.h"
#include "Spell.h"
#include "SpellAuras.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include "ThreatManager.h"
#include <algorithm>
#include <iterator>
#include <limits>
#include <list>
#include <mutex>
#include <tuple>

using namespace EncounterHelpers;

namespace BtHelpers
{

namespace
{

std::mutex btStateMutex;
std::unordered_map<uint32, BtInstanceState> btStates;

} // end anonymous namespace (Shared)

// Shared

Creature* GetBtBoss(Player* bot, BtBosses boss)
{
    return GetInstanceBoss(bot, BT_MAP_ID, Id(boss));
}

Creature* GetBtCreature(Player* bot, BtCreatureData data)
{
    return GetInstanceCreature(bot, BT_MAP_ID, Id(data));
}

bool MisdirectTargetToTank(PlayerbotAI* botAI, Unit* target, Player* tank)
{
    if (!target || !tank)
        return false;

    if (botAI->CanCastSpell(Id(BtSpells::SPELL_MISDIRECTION), tank))
        return botAI->CastSpell(Id(BtSpells::SPELL_MISDIRECTION), tank);

    if (!botAI->GetBot()->HasAura(Id(BtSpells::SPELL_MISDIRECTION_AURA)))
        return false;

    return botAI->CanCastSpell("steady shot", target) && botAI->CastSpell("steady shot", target);
}

bool CommandPetAttack(Creature* pet, Unit* target)
{
    if (!pet || !target)
        return false;

    pet->ClearUnitState(UNIT_STATE_FOLLOW);
    pet->AttackStop();
    pet->SetTarget(target->GetGUID());
    if (CharmInfo* charmInfo = pet->GetCharmInfo())
    {
        charmInfo->SetIsCommandAttack(true);
        charmInfo->SetIsAtStay(false);
        charmInfo->SetIsFollowing(false);
        charmInfo->SetIsCommandFollow(false);
        charmInfo->SetIsReturning(false);
    }

    pet->AI()->AttackStart(target);
    return true;
}

GuidVector FindAttackerGuidsWithEntry(PlayerbotAI* botAI, uint32 entry)
{
    GuidVector guids;
    auto const& attackers =
        botAI->GetAiObjectContext()->GetValue<GuidVector>("attackers")->RefGet();
    for (ObjectGuid const& guid : attackers)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (unit && unit->IsAlive() && unit->GetEntry() == entry)
            guids.push_back(guid);
    }

    return guids;
}

uint32 GetAuraElapsedMs(Aura const* aura)
{
    return aura ? static_cast<uint32>(aura->GetMaxDuration() - aura->GetDuration()) : 0;
}

// Turn the bot to the unit unless it is already in the bot's front 90 degrees
void FaceUnit(Player* bot, Unit* unit)
{
    if (unit && !bot->HasInArc(static_cast<float>(M_PI) / 2.0f, unit))
        bot->SetFacingTo(bot->GetAngle(unit));
}

// Trash

namespace
{

std::unordered_map<uint32, std::unordered_map<ObjectGuid, uint32>> reaverAbsorptionStarts;

bool IsMagicDamageSpell(SpellInfo const* spellInfo)
{
    return (spellInfo->GetSchoolMask() & SPELL_SCHOOL_MASK_MAGIC) &&
        (spellInfo->DmgClass == SPELL_DAMAGE_CLASS_MAGIC ||
         spellInfo->DmgClass == SPELL_DAMAGE_CLASS_NONE);
}

// Check a spell and the spells it triggers. Spell Absorption's charge proc ignores triggered spells
// without NOT_A_PROC and DoT ticks, but counts a dummy effect that can hit an enemy, because a
// script casts the real spell after it (e.g. Holy Shock, which heals or damages by target).
bool CanBuildChaoticCharge(SpellInfo const* spellInfo, bool triggered, uint8 depth, bool& area)
{
    constexpr uint8 maxDepth = 3;
    if (!spellInfo || depth > maxDepth)
        return false;

    bool const canProc = !triggered || spellInfo->HasAttribute(SPELL_ATTR3_NOT_A_PROC);
    bool builds = false;
    for (SpellEffectInfo const& effect : spellInfo->GetEffects())
    {
        uint32 triggerSpell = 0;
        switch (effect.Effect)
        {
            case SPELL_EFFECT_SCHOOL_DAMAGE:
            case SPELL_EFFECT_HEALTH_LEECH:
            case SPELL_EFFECT_POWER_BURN:
            {
                if (canProc && IsMagicDamageSpell(spellInfo))
                {
                    builds = true;
                    area |= effect.IsTargetingArea() || effect.ChainTarget > 1;
                }
                break;
            }
            // Wands: magic class, with the school taken from the wand.
            case SPELL_EFFECT_WEAPON_DAMAGE_NOSCHOOL:
            {
                if (canProc && spellInfo->DmgClass == SPELL_DAMAGE_CLASS_MAGIC)
                    builds = true;
                break;
            }
            case SPELL_EFFECT_DUMMY:
            {
                if (!IsMagicDamageSpell(spellInfo))
                    break;

                if (effect.IsTargetingArea())
                {
                    builds = true;
                    area = true;
                }
                else if (effect.TargetA.GetTarget() == TARGET_UNIT_TARGET_ENEMY ||
                    effect.TargetA.GetTarget() == TARGET_UNIT_TARGET_ANY)
                {
                    builds = true;
                }
                break;
            }
            case SPELL_EFFECT_TRIGGER_SPELL:
            case SPELL_EFFECT_TRIGGER_MISSILE:
            {
                triggerSpell = effect.TriggerSpell;
                break;
            }
            case SPELL_EFFECT_APPLY_AURA:
            case SPELL_EFFECT_PERSISTENT_AREA_AURA:
            {
                if (effect.ApplyAuraName == SPELL_AURA_PERIODIC_TRIGGER_SPELL ||
                    effect.ApplyAuraName == SPELL_AURA_PERIODIC_TRIGGER_SPELL_WITH_VALUE)
                {
                    triggerSpell = effect.TriggerSpell;
                }
                break;
            }
            default:
                break;
        }

        bool triggeredArea = false;
        if (triggerSpell && CanBuildChaoticCharge(
                sSpellMgr->GetSpellInfo(triggerSpell), true, depth + 1, triggeredArea))
        {
            builds = true;
            area |= triggeredArea;
        }
    }

    return builds;
}

} // end anonymous namespace (Trash)

// Grid search instead of attackers, which has an LoS check. Sisters are often up stairs or around
// corners, which could delay the skull's application through attackers.
GuidVector FindSisterOfPleasureGuids(PlayerbotAI* botAI)
{
    std::list<Creature*> sisters;
    botAI->GetBot()->GetCreatureListWithEntryInGrid(
        sisters, Id(BtNpcs::NPC_SISTER_OF_PLEASURE), SISTER_OF_PLEASURE_SEARCH_RADIUS);

    GuidVector guids;
    for (Creature* sister : sisters)
        guids.push_back(sister->GetGUID());

    return guids;
}

Unit* GetLinkedSisterOfPain(Unit* pleasure)
{
    if (!pleasure || pleasure->GetEntry() != Id(BtNpcs::NPC_SISTER_OF_PLEASURE) ||
        !pleasure->IsAlive())
    {
        return nullptr;
    }

    Aura* bonds = pleasure->GetAura(Id(BtSpells::SPELL_SHARED_BONDS));
    Unit* pain = bonds ? bonds->GetCaster() : nullptr;
    return pain && pain->IsAlive() ? pain : nullptr;
}

Unit* FindLinkedSisterOfPleasure(PlayerbotAI* botAI)
{
    auto const& sisters =
        botAI->GetAiObjectContext()->GetValue<GuidVector>("sisters of pleasure")->RefGet();
    for (ObjectGuid const& guid : sisters)
    {
        Unit* sister = botAI->GetUnit(guid);
        if (GetLinkedSisterOfPain(sister))
            return sister;
    }

    return nullptr;
}

Unit* GetSkulledSisterOfPain(Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    return GetLinkedSisterOfPain(
        ObjectAccessor::GetUnit(*bot, group->GetTargetIcon(RtiTargetValue::skullIndex)));
}

// While magic would risk giving a Reaver a Chaotic Charge: (1) during Spell Absorption, (2) before
// the first Spell Absorption is cast, and (3) in the margin before the earliest possible recast.
bool IsShadowmoonReaverUnsafeForMagic(Unit* unit)
{
    if (!unit || unit->GetEntry() != Id(BtNpcs::NPC_SHADOWMOON_REAVER) || !unit->IsAlive())
        return false;

    ObjectGuid const guid = unit->GetGUID();
    uint32 const instanceId = unit->GetInstanceId();
    std::lock_guard lock(btStateMutex);
    if (Aura* absorption = unit->GetAura(Id(BtSpells::SPELL_SPELL_ABSORPTION)))
    {
        reaverAbsorptionStarts[instanceId][guid] = getMSTime() - GetAuraElapsedMs(absorption);
        return true;
    }

    auto const instanceIt = reaverAbsorptionStarts.find(instanceId);
    if (instanceIt == reaverAbsorptionStarts.end())
        return true;

    auto const it = instanceIt->second.find(guid);
    if (it == instanceIt->second.end())
        return true;

    uint32 const sinceStart = GetMSTimeDiffToNow(it->second);
    return sinceStart < REAVER_ABSORPTION_DURATION_MS ||
        sinceStart >= REAVER_ABSORPTION_MIN_RECAST_MS - REAVER_MAGIC_MARGIN_MS;
}

bool IsAnyShadowmoonReaverUnsafeForMagic(PlayerbotAI* botAI)
{
    auto const& reavers =
        botAI->GetAiObjectContext()->GetValue<GuidVector>("shadowmoon reavers")->RefGet();
    for (ObjectGuid const& guid : reavers)
    {
        if (IsShadowmoonReaverUnsafeForMagic(botAI->GetUnit(guid)))
            return true;
    }

    return false;
}

// Erases the Spell Absorption timing of reavers that are dead or despawned.
bool PruneShadowmoonReaverAbsorptionStarts(Player* bot)
{
    std::lock_guard lock(btStateMutex);
    auto const reaversIt = reaverAbsorptionStarts.find(bot->GetInstanceId());
    if (reaversIt == reaverAbsorptionStarts.end())
        return false;

    bool const pruned = std::erase_if(
        reaversIt->second,
        [bot](auto const& entry)
        {
            Unit* reaver = ObjectAccessor::GetUnit(*bot, entry.first);
            return !reaver || !reaver->IsAlive();
        }) > 0;

    if (reaversIt->second.empty())
        reaverAbsorptionStarts.erase(reaversIt);

    return pruned;
}

ChaoticChargeReach GetChaoticChargeReach(SpellInfo const* spellInfo)
{
    bool area = false;
    if (!CanBuildChaoticCharge(spellInfo, false, 0, area))
        return ChaoticChargeReach::None;

    return area ? ChaoticChargeReach::Area : ChaoticChargeReach::Target;
}

// Imps, water elementals, succubi, and felhunters, whose attacks build Chaotic Charge.
bool IsChargeBuildingPet(Unit* unit)
{
    if (!unit)
        return false;

    switch (unit->GetEntry())
    {
        case NPC_IMP:
        case NPC_FELHUNTER:
        case NPC_SUCCUBUS:
        case NPC_WATER_ELEMENTAL_TEMP:
        case NPC_WATER_ELEMENTAL_PERM:
            return true;
        default:
            return false;
    }
}

Unit* FindPetTargetOtherThanReaver(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    auto const isAllowed = [bot](Unit* unit)
    {
        return unit && unit->IsAlive() && unit->GetEntry() != Id(BtNpcs::NPC_SHADOWMOON_REAVER) &&
            bot->IsValidAttackTarget(unit);
    };

    AiObjectContext* context = botAI->GetAiObjectContext();
    Unit* currentTarget = AI_VALUE(Unit*, "current target");
    if (isAllowed(currentTarget))
        return currentTarget;

    auto const& attackers = context->GetValue<GuidVector>("attackers")->RefGet();
    for (ObjectGuid const& guid : attackers)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (isAllowed(unit))
            return unit;
    }

    return nullptr;
}

// High Warlord Naj'entus

namespace // High Warlord Naj'entus
{

bool IsValidSpineRemover(Player* bot, ObjectGuid remover)
{
    Player* player = ObjectAccessor::GetPlayer(*bot, remover);
    return player && player->IsAlive() && player->GetMapId() == BT_MAP_ID &&
        !IsNajentusImpaled(player);
}

bool CanThrowNajentusSpine(Player* bot)
{
    return bot && bot->IsAlive() && bot->GetMapId() == BT_MAP_ID && !IsNajentusImpaled(bot) &&
        bot->HasItemCount(Id(BtItems::ITEM_NAJENTUS_SPINE));
}

std::vector<NajentusSpineAssignment> const* FindNajentusSpineAssignments(uint32 instanceId)
{
    BtInstanceState const* state = FindBtState(instanceId);
    return state && state->najentusSpineAssignments ? &*state->najentusSpineAssignments : nullptr;
}

} // end anonymous namespace (High Warlord Naj'entus)

bool IsNajentusImpaled(Player* player)
{
    return player && player->IsAlive() && player->HasAura(Id(BtSpells::SPELL_IMPALING_SPINE));
}

Player* FindNajentusUnassignedImpaledPlayer(Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    auto const* assignments = FindNajentusSpineAssignments(bot->GetInstanceId());
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!IsNajentusImpaled(member))
            continue;

        bool assigned = false;
        if (assignments)
        {
            for (NajentusSpineAssignment const& assignment : *assignments)
            {
                if (assignment.impaled == member->GetGUID() &&
                    IsValidSpineRemover(bot, assignment.remover))
                {
                    assigned = true;
                    break;
                }
            }
        }

        if (!assigned)
            return member;
    }

    return nullptr;
}

Player* FindNajentusSpineRemover(Player* bot, Player* impaled)
{
    Group* group = bot->GetGroup();
    if (!group || !impaled)
        return nullptr;

    auto const* assignments = FindNajentusSpineAssignments(bot->GetInstanceId());

    Player* remover = nullptr;
    float closestDist = std::numeric_limits<float>::max();
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member == impaled || !member->IsAlive() || member->GetMapId() != BT_MAP_ID ||
            !GET_PLAYERBOT_AI(member) || PlayerbotAI::IsTank(member) || IsNajentusImpaled(member))
        {
            continue;
        }

        if (assignments &&
            std::any_of(assignments->begin(), assignments->end(),
                [member](NajentusSpineAssignment const& assignment)
                { return assignment.remover == member->GetGUID(); }))
        {
            continue;
        }

        float const dist = member->GetExactDist2d(impaled);
        if (dist < closestDist)
        {
            closestDist = dist;
            remover = member;
        }
    }

    return remover;
}

Player* GetNajentusImpaledPlayerToFree(Player* bot)
{
    auto const* assignments = FindNajentusSpineAssignments(bot->GetInstanceId());
    if (!assignments)
        return nullptr;

    for (NajentusSpineAssignment const& assignment : *assignments)
    {
        if (assignment.remover != bot->GetGUID())
            continue;

        Player* impaled = ObjectAccessor::GetPlayer(*bot, assignment.impaled);
        return IsNajentusImpaled(impaled) ? impaled : nullptr;
    }

    return nullptr;
}

bool IsNajentusSpineThrower(Player* bot)
{
    BtInstanceState const* state = FindBtState(bot->GetInstanceId());
    return state && state->najentusSpineThrower == bot->GetGUID();
}

Player* GetNajentusSpineThrower(Player* bot)
{
    BtInstanceState const* state = FindBtState(bot->GetInstanceId());
    if (!state || !state->najentusSpineThrower)
        return nullptr;

    Player* thrower = ObjectAccessor::GetPlayer(*bot, *state->najentusSpineThrower);
    return CanThrowNajentusSpine(thrower) ? thrower : nullptr;
}

Player* FindNajentusSpineThrower(Player* bot, Unit* najentus)
{
    Group* group = bot->GetGroup();
    if (!group || !najentus)
        return nullptr;

    Player* thrower = nullptr;
    float closestDist = std::numeric_limits<float>::max();
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !GET_PLAYERBOT_AI(member) || !CanThrowNajentusSpine(member))
            continue;

        float const dist = member->GetExactDist2d(najentus);
        if (dist < closestDist)
        {
            closestDist = dist;
            thrower = member;
        }
    }

    return thrower;
}

bool IsNajentusSpineInFlight(Player* bot)
{
    BtInstanceState const* state = FindBtState(bot->GetInstanceId());
    return state && state->najentusSpineThrowTime &&
        GetMSTimeDiffToNow(*state->najentusSpineThrowTime) < NAJENTUS_HURL_SPINE_FLIGHT_MS;
}

// Supremus

namespace
{

float GetSupremusHazardRadius(SupremusHazard const& hazard, SupremusHazardZone zone)
{
    return zone == SupremusHazardZone::Damage ? hazard.damageRadius : hazard.safeRadius;
}

} // end anonymous namespace (Supremus)

bool IsSupremusKitePhase(Unit* supremus)
{
    return supremus && supremus->HasAura(Id(BtSpells::SPELL_SNARE_SELF));
}

// Seconds left before Supremus fixates again, counted from the start of the kite phase. Nothing
// records the fixate itself, so the 10s schedule is measured from the phase switch that Snare Self
// marks. Its apply time is epoch seconds, so it is compared with GameTime::GetGameTime(), not
// getMSTime (counts from server start) or GetAuraElapsedMs (needs a duration, and Snare Self is
// permanent). The extra second covers the whole-second reading and errs toward caution.
float GetSupremusFixateTimeRemaining(Unit* supremus)
{
    if (!supremus)
        return 0.0f;

    Aura* snareSelf = supremus->GetAura(Id(BtSpells::SPELL_SNARE_SELF));
    if (!snareSelf)
        return 0.0f;

    int64 const elapsed =
        GameTime::GetGameTime().count() - static_cast<int64>(snareSelf->GetApplyTime());
    int64 const remaining = std::min(
        SUPREMUS_FIXATE_INTERVAL_SECONDS,
        SUPREMUS_FIXATE_INTERVAL_SECONDS + 1 - elapsed % SUPREMUS_FIXATE_INTERVAL_SECONDS);

    return static_cast<float>(remaining);
}

float GetSupremusCatchDistance(Player* bot, Unit* supremus)
{
    if (!supremus)
        return 0.0f;

    return supremus->GetMeleeRange(bot) + SUPREMUS_KITE_SAFETY_MARGIN;
}

bool CanSupremusCatchStandingBot(Player* bot, Unit* supremus)
{
    if (!supremus)
        return false;

    float const closing = supremus->GetSpeed(MOVE_RUN) * GetSupremusFixateTimeRemaining(supremus);
    return supremus->GetExactDist2d(bot) - closing < GetSupremusCatchDistance(bot, supremus);
}

// The volcano is visible for 30s, but its eruption deals damage for only the first 18s.
bool IsSupremusVolcanoErupting(Unit* volcano)
{
    return volcano && volcano->IsAlive() && volcano->HasAura(Id(BtSpells::SPELL_VOLCANIC_GEYSER));
}

// Hazards include Molten Flame only in the kite phase; in the tank phase, avoid aoe handles them.
std::vector<SupremusHazard> FindSupremusHazards(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    std::vector<SupremusHazard> hazards;

    std::list<Creature*> volcanoes;
    bot->GetCreatureListWithEntryInGrid(
        volcanoes, Id(BtNpcs::NPC_SUPREMUS_VOLCANO), SUPREMUS_HAZARD_SEARCH_RADIUS);
    for (Creature* volcano : volcanoes)
    {
        if (IsSupremusVolcanoErupting(volcano))
        {
            hazards.push_back({
                volcano->GetPosition(), SUPREMUS_VOLCANO_HAZARD_RADIUS,
                SUPREMUS_VOLCANO_SAFE_DISTANCE });
        }
    }

    Unit* supremus = GetBtBoss(bot, BtBosses::DATA_SUPREMUS);
    if (!supremus || !IsSupremusKitePhase(supremus))
        return hazards;

    for (Position const& patch : GetDynamicObjectPositions(
             bot, SUPREMUS_HAZARD_SEARCH_RADIUS, Id(BtSpells::SPELL_MOLTEN_FLAME_PATCH)))
    {
        hazards.push_back(
            { patch, SUPREMUS_MOLTEN_FLAME_HAZARD_RADIUS, SUPREMUS_MOLTEN_FLAME_SAFE_DISTANCE });
    }

    return hazards;
}

std::vector<SupremusHazard> const& GetSupremusHazards(PlayerbotAI* botAI)
{
    return botAI->GetAiObjectContext()
        ->GetValue<std::vector<SupremusHazard>>("supremus hazards")->RefGet();
}

bool IsInSupremusHazard(
    std::vector<SupremusHazard> const& hazards, float x, float y, SupremusHazardZone zone,
    float margin)
{
    for (SupremusHazard const& hazard : hazards)
    {
        if (hazard.position.GetExactDist2d(x, y) < GetSupremusHazardRadius(hazard, zone) + margin)
            return true;
    }

    return false;
}

float GetLineLengthInSupremusHazards(
    std::vector<SupremusHazard> const& hazards, Position const& from, Position const& to,
    SupremusHazardZone zone)
{
    float const length = from.GetExactDist2d(to);
    if (length <= 0.0f)
        return 0.0f;

    float const dx = (to.GetPositionX() - from.GetPositionX()) / length;
    float const dy = (to.GetPositionY() - from.GetPositionY()) / length;
    float total = 0.0f;
    for (SupremusHazard const& hazard : hazards)
    {
        float const radius = GetSupremusHazardRadius(hazard, zone);
        float const offsetX = from.GetPositionX() - hazard.position.GetPositionX();
        float const offsetY = from.GetPositionY() - hazard.position.GetPositionY();
        float const along = offsetX * dx + offsetY * dy;
        float const outside = offsetX * offsetX + offsetY * offsetY - radius * radius;
        float const discriminant = along * along - outside;
        if (discriminant <= 0.0f)
            continue;

        float const root = std::sqrt(discriminant);
        float const enter = std::max(0.0f, -along - root);
        float const exit = std::min(length, -along + root);
        if (exit > enter)
            total += exit - enter;
    }

    return total;
}

// With Divine Shield or Dispersion up, a bot may walk through fire but not stop in it.
bool CanWalkThroughSupremusFire(Player* bot)
{
    switch (bot->getClass())
    {
        case CLASS_PALADIN:
            return bot->HasAura(Id(BtSpells::SPELL_DIVINE_SHIELD));
        case CLASS_PRIEST:
            return bot->HasAura(Id(BtSpells::SPELL_DISPERSION));
        default:
            return false;
    }
}

bool IsSupremusReachBlockedByFire(PlayerbotAI* botAI, Unit* target, float range)
{
    Player* bot = botAI->GetBot();
    // Standard reach doesn't break channels, so take the same path with this custom reach method.
    if (CanWalkThroughSupremusFire(bot) || bot->GetCurrentSpell(CURRENT_CHANNELED_SPELL))
        return false;

    if (!target || !target->IsAlive() || bot->IsWithinCombatRange(target, range))
        return false;

    // Custom reach takes the bot to 1 yd inside range, on the straight line to the target.
    constexpr float rangeInset = 1.0f;
    float const stopDistance =
        range + bot->GetCombatReach() + target->GetCombatReach() - rangeInset;
    float const distance = bot->GetExactDist2d(target);
    if (distance <= stopDistance)
        return false;

    float const ratio = stopDistance / distance;
    Position const stop(
        target->GetPositionX() + (bot->GetPositionX() - target->GetPositionX()) * ratio,
        target->GetPositionY() + (bot->GetPositionY() - target->GetPositionY()) * ratio,
        bot->GetPositionZ());

    float const underFire = GetLineLengthInSupremusHazards(
        GetSupremusHazards(botAI), bot->GetPosition(), stop, SupremusHazardZone::Safe);
    return underFire > 0.0f;
}

// The target of SupremusReachAroundFireAction, which is the "party member to heal" for a healer and
// the current target for other bots.
Unit* GetSupremusFireBlockedReachTarget(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    bool const isHealer = PlayerbotAI::IsHeal(bot);
    Unit* target = botAI->GetAiObjectContext()
        ->GetValue<Unit*>(isHealer ? "party member to heal" : "current target")->Get();
    float range;
    if (isHealer)
        range = botAI->GetRange("heal");
    else if (PlayerbotAI::IsRanged(bot))
        range = botAI->GetRange("spell");
    else
        range = sPlayerbotAIConfig.MeleeDistance;

    return IsSupremusReachBlockedByFire(botAI, target, range) ? target : nullptr;
}

// Shade of Akama

// Only the adds that must be prioritized to complete the encounter (Channelers and Sorcerers).
GuidVector FindShadeOfAkamaAddGuids(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    GuidVector adds;
    if (bot->GetPositionX() < SHADE_OF_AKAMA_BOUNDARY_MIN_X ||
        bot->GetPositionX() > SHADE_OF_AKAMA_BOUNDARY_MAX_X ||
        bot->GetPositionY() < SHADE_OF_AKAMA_BOUNDARY_MIN_Y ||
        bot->GetPositionY() > SHADE_OF_AKAMA_BOUNDARY_MAX_Y)
    {
        return adds;
    }

    for (BtNpcs const entry : { BtNpcs::NPC_ASHTONGUE_CHANNELER, BtNpcs::NPC_ASHTONGUE_SORCERER })
    {
        std::list<Creature*> creatures;
        bot->GetCreatureListWithEntryInGrid(creatures, Id(entry), SHADE_OF_AKAMA_ADD_SEARCH_RADIUS);

        std::vector<Creature*> living;
        for (Creature* creature : creatures)
        {
            if (creature && creature->IsAlive() && !creature->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE))
                living.push_back(creature);
        }

        std::sort(living.begin(), living.end(), [](Creature* first, Creature* second)
            { return first->GetGUID() < second->GetGUID(); });

        for (Creature* creature : living)
            adds.push_back(creature->GetGUID());
    }

    return adds;
}

Unit* GetShadeOfAkamaKillTarget(PlayerbotAI* botAI)
{
    auto const& adds =
        botAI->GetAiObjectContext()->GetValue<GuidVector>("shade of akama adds")->RefGet();
    for (ObjectGuid const& guid : adds)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (unit && unit->IsAlive())
            return unit;
    }

    return nullptr;
}

// Teron Gorefiend

// A creature casting from a script records no spell cooldown, and no global cooldown either if the
// cast is triggered (the third argument of CastSpell, passed as true below). Both are therefore
// applied here from the spell data. False if the cast fails.
bool CastVengefulSpiritSpell(Unit* spirit, Unit* target, uint32 spellId)
{
    if (!spirit || !target)
        return false;

    static constexpr std::array spiritSpells = {
        Id(BtSpells::SPELL_SPIRIT_STRIKE),
        Id(BtSpells::SPELL_SPIRIT_LANCE),
        Id(BtSpells::SPELL_SPIRIT_CHAINS),
        Id(BtSpells::SPELL_SPIRIT_VOLLEY),
        Id(BtSpells::SPELL_SPIRIT_SHIELD),
    };

    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
    if (!spellInfo || spirit->HasSpellCooldown(spellId) ||
        spirit->CastSpell(target, spellId, true) != SPELL_CAST_OK)
    {
        return false;
    }

    if (uint32 const cooldown = std::max(spellInfo->RecoveryTime, spellInfo->CategoryRecoveryTime))
        spirit->AddSpellCooldown(spellId, 0, cooldown);

    for (uint32 const spiritSpellId : spiritSpells)
    {
        if (!spirit->HasSpellCooldown(spiritSpellId))
            spirit->AddSpellCooldown(spiritSpellId, 0, spellInfo->StartRecoveryTime);
    }

    return true;
}

// Gurtogg Bloodboil

namespace
{

// The Fel Rage target is excluded because Gurtogg's rage ends 2s before the target's aura does, and
// until it does, the target is held at the tank position, too close to be one of the 5 farthest.
std::vector<std::vector<Player*>> GetGurtoggRangedRotationGroups(Player* bot)
{
    std::vector<std::vector<Player*>> groups(GURTOGG_ROTATION_GROUP_COUNT);
    Group* group = bot->GetGroup();
    if (!group)
        return groups;

    size_t count = 0;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member->GetMapId() != BT_MAP_ID || !member->IsAlive() ||
            !GET_PLAYERBOT_AI(member) || !PlayerbotAI::IsRanged(member) ||
            IsGurtoggFelRageTarget(member))
        {
            continue;
        }

        groups[count / GURTOGG_ROTATION_GROUP_SIZE].push_back(member);
        if (++count == GURTOGG_ROTATION_GROUP_COUNT * GURTOGG_ROTATION_GROUP_SIZE)
            break;
    }

    return groups;
}

// Next soakers: the group hit longest ago, determined by the lowest mean Bloodboil time left.
GuidVector FindGurtoggBloodboilSoakerGuids(Player* bot)
{
    std::vector<std::vector<Player*>> const groups = GetGurtoggRangedRotationGroups(bot);
    std::vector<Player*> const* soakers = nullptr;
    float lowestMeanRemaining = 0.0f;
    for (std::vector<Player*> const& group : groups)
    {
        if (group.empty())
            continue;

        int32 totalRemaining = 0;
        for (Player* member : group)
        {
            Aura* bloodboil = member->GetAura(Id(BtSpells::SPELL_BLOODBOIL));
            if (!bloodboil)
                continue;

            if (GetAuraElapsedMs(bloodboil) >= GURTOGG_ROTATION_DELAY_MS)
                totalRemaining += bloodboil->GetDuration();
        }

        float const meanRemaining = static_cast<float>(totalRemaining) / group.size();
        if (!soakers || meanRemaining < lowestMeanRemaining)
        {
            soakers = &group;
            lowestMeanRemaining = meanRemaining;
        }
    }

    GuidVector guids;
    if (soakers)
    {
        for (Player* member : *soakers)
            guids.push_back(member->GetGUID());
    }

    return guids;
}

} // end anonymous namespace (Gurtogg Bloodboil)

Position const& GetGurtoggBloodboilPosition(Player* bot)
{
    GuidVector const soakers = FindGurtoggBloodboilSoakerGuids(bot);
    bool const isSoaker =
        std::find(soakers.begin(), soakers.end(), bot->GetGUID()) != soakers.end();
    return isSoaker ? GURTOGG_SOAKER_POSITION : GURTOGG_RANGED_POSITION;
}

Player* GetGurtoggFelRageTarget(Unit* gurtogg)
{
    if (!gurtogg || !gurtogg->HasAura(Id(BtSpells::SPELL_BOSS_FEL_RAGE)))
        return nullptr;

    Player* victim = gurtogg->GetVictim() ? gurtogg->GetVictim()->ToPlayer() : nullptr;
    return victim && IsGurtoggFelRageTarget(victim) ? victim : nullptr;
}

bool IsGurtoggFelRageTarget(Player* player)
{
    return player->HasAura(Id(BtSpells::SPELL_PLAYER_FEL_RAGE));
}

Position GetGurtoggFelRageMeleePosition(
    Player* bot, Unit const& gurtogg, Unit const& felRageTarget)
{
    float const axisAngle = gurtogg.GetAngle(&felRageTarget);
    float relativeAngle = Position::NormalizeOrientation(gurtogg.GetAngle(bot) - axisAngle);
    if (relativeAngle > static_cast<float>(M_PI))
        relativeAngle -= 2.0f * static_cast<float>(M_PI);

    float angle = axisAngle + static_cast<float>(M_PI);
    if (std::fabs(relativeAngle) < static_cast<float>(M_PI_2))
    {
        float const side = relativeAngle >= 0.0f ? 1.0f : -1.0f;
        angle = axisAngle + side * GURTOGG_FEL_RAGE_MELEE_SIDE_ANGLE;
    }

    return Position(
        gurtogg.GetPositionX() + GURTOGG_FEL_RAGE_MELEE_DISTANCE * std::cos(angle),
        gurtogg.GetPositionY() + GURTOGG_FEL_RAGE_MELEE_DISTANCE * std::sin(angle),
        bot->GetPositionZ());
}

float FindGurtoggSecondTankThreat(PlayerbotAI* botAI)
{
    Unit* gurtogg = GetBtBoss(botAI->GetBot(), BtBosses::DATA_GURTOGG_BLOODBOIL);
    if (!gurtogg)
        return 0.0f;

    float highestThreat = 0.0f;
    float secondThreat = 0.0f;
    for (ThreatReference const* ref : gurtogg->GetThreatMgr().GetUnsortedThreatList())
    {
        if (!ref->IsAvailable())
            continue;

        Unit* victim = ref->GetVictim();
        Player* player = victim ? victim->ToPlayer() : nullptr;
        if (!player || !player->IsAlive() || !PlayerbotAI::IsTank(player))
            continue;

        float const threat = ref->GetThreat();
        if (threat > highestThreat)
        {
            secondThreat = highestThreat;
            highestThreat = threat;
        }
        else if (threat > secondThreat)
        {
            secondThreat = threat;
        }
    }

    return secondThreat;
}

// The rectangle is a space in which bots can be given a randomized (by GUID) position. There are
// two rectangles drawn, one for the 5 Bloodboil soakers and the other for other ranged bots.
bool IsInGurtoggRectangle(
    Position const& point, Position const& center, Position const& facing, float halfWidth,
    float halfDepth)
{
    float const angle = center.GetAngle(&facing);
    float const dX = point.GetPositionX() - center.GetPositionX();
    float const dY = point.GetPositionY() - center.GetPositionY();
    float const depth = dX * std::cos(angle) + dY * std::sin(angle);
    float const width = -dX * std::sin(angle) + dY * std::cos(angle);
    return std::fabs(depth) <= halfDepth && std::fabs(width) <= halfWidth;
}

// This function actually assigns the position. The rectangle approach is way overbaked for
// executing the strategy, but the result is intended to create realism by making bot positioning
// seem more random and natural (within required bounds).
Position GetBotPointInGurtoggRectangle(
    Player* bot, Position const& center, Position const& facing, float halfWidth, float halfDepth)
{
    constexpr float edgeInset = 0.5f;

    uint64 hash = bot->GetGUID().GetRawValue() + 0x9E3779B97F4A7C15ULL;
    hash = (hash ^ (hash >> 30)) * 0xBF58476D1CE4E5B9ULL;
    hash = (hash ^ (hash >> 27)) * 0x94D049BB133111EBULL;
    hash ^= hash >> 31;
    float const widthFraction = static_cast<float>(hash & 0xFFFF) / 0xFFFF * 2.0f - 1.0f;
    float const depthFraction = static_cast<float>((hash >> 32) & 0xFFFF) / 0xFFFF * 2.0f - 1.0f;

    float const angle = center.GetAngle(&facing);
    float const width = widthFraction * std::max(halfWidth - edgeInset, 0.0f);
    float const depth = depthFraction * std::max(halfDepth - edgeInset, 0.0f);
    float x = center.GetPositionX() + depth * std::cos(angle) - width * std::sin(angle);
    float y = center.GetPositionY() + depth * std::sin(angle) + width * std::cos(angle);
    float z = bot->GetPositionZ();

    if (!bot->GetMap()->CheckCollisionAndGetValidCoords(
            bot, center.GetPositionX(), center.GetPositionY(), bot->GetPositionZ(), x, y, z))
    {
        return Position(center.GetPositionX(), center.GetPositionY(), bot->GetPositionZ());
    }

    return Position(x, y, bot->GetPositionZ());
}

// Reliquary of Souls

bool IsSufferingFixateTank(Player* bot)
{
    return PlayerbotAI::IsTank(bot) && bot->GetHealthPct() > SUFFERING_TANK_MIN_HEALTH_PCT;
}

bool IsOutOfSufferingPosition(Player* bot, Unit* suffering)
{
    if (!suffering)
        return false;

    if (IsSufferingFixateTank(bot))
        return bot->GetExactDist2d(suffering) > SUFFERING_TANK_MAX_DISTANCE;

    if (PlayerbotAI::IsMelee(bot))
        return bot->GetExactDist2d(suffering) < SUFFERING_MELEE_MIN_DISTANCE;

    return PlayerbotAI::IsRanged(bot) && bot->GetExactDist2d(suffering) < SUFFERING_RANGED_DISTANCE;
}

// Mother Shahraz

// Melee dps hold at the ranged position while this is true.
bool IsShahrazBeingPositioned(Unit* shahraz)
{
    if (!shahraz || shahraz->GetHealthPct() < SHAHRAZ_MELEE_ENGAGE_HEALTH_PCT)
        return false;

    Unit* victim = shahraz->GetVictim();
    return !victim || victim->GetExactDist2d(SHAHRAZ_TANK_POSITION) > SHAHRAZ_POSITIONED_DISTANCE;
}

// Illidari Council

IllidariCouncilMembers GetIllidariCouncilMembers(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    auto const getMember = [bot](BtCreatureData data) -> Unit*
    {
        Creature* member = GetBtCreature(bot, data);
        return member && member->IsInCombat() ? member : nullptr;
    };

    return {
        getMember(BtCreatureData::DATA_GATHIOS_THE_SHATTERER),
        getMember(BtCreatureData::DATA_LADY_MALANDE),
        getMember(BtCreatureData::DATA_HIGH_NETHERMANCER_ZEREVOR),
        getMember(BtCreatureData::DATA_VERAS_DARKSHADOW),
    };
}

bool UpdateIllidariCouncilClassRoleAnnouncement(PlayerbotAI* botAI)
{
    BtInstanceState& state = BtState(botAI->GetBot()->GetInstanceId());
    return UpdateClassRoleAnnouncement(
        botAI, Id(BtNpcs::NPC_HIGH_NETHERMANCER_ZEREVOR), ZEREVOR_SPAWN_POSITION,
        COUNCIL_CLASS_ROLE_ANNOUNCE_DISTANCE, COUNCIL_CLASS_ROLE_REARM_DISTANCE, {
            { ZEREVOR_MAGE_TANK_ROLE, state.zerevorMageTank },
        });
}

Player* GetZerevorMageTank(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    BtInstanceState const* state = FindBtState(bot->GetInstanceId());
    return state ? GetClassRolePlayer(bot, state->zerevorMageTank) : nullptr;
}

bool IsZerevorMageTank(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    if (bot->getClass() != CLASS_MAGE)
        return false;

    BtInstanceState const* state = FindBtState(bot->GetInstanceId());
    return state && IsClassRolePlayer(bot, state->zerevorMageTank);
}

bool HasDangerousCouncilAura(Player* bot)
{
    static constexpr std::array dangerousAuras = {
        Id(BtSpells::SPELL_CONSECRATION),
        Id(BtSpells::SPELL_BLIZZARD),
        Id(BtSpells::SPELL_FLAMESTRIKE),
    };

    for (uint32 aura : dangerousAuras)
    {
        if (bot->HasAura(aura))
            return true;
    }

    return false;
}

bool IsVerasVanished(Unit* veras)
{
    return veras && veras->HasAura(Id(BtSpells::SPELL_VERAS_VANISH));
}

bool CanInterruptCircleOfHealing(Unit* malande)
{
    if (!malande)
        return false;

    Spell* circle = malande->FindCurrentSpellBySpellId(Id(BtSpells::SPELL_CIRCLE_OF_HEALING));
    return circle && circle->getState() == SPELL_STATE_PREPARING &&
        circle->GetCastTime() - circle->GetCastTimeRemaining() >= CIRCLE_OF_HEALING_REACTION_MS;
}

// Blizzard and Flamestrike are each on a 40s timer, 20s apart, and a patch lasts 12s, so only one
// of them is ever down at a time.
bool IsInZerevorPatch(Unit* zerevor, Position const& point, float margin)
{
    if (!zerevor)
        return false;

    static constexpr std::array patchSpells = {
        Id(BtSpells::SPELL_BLIZZARD),
        Id(BtSpells::SPELL_FLAMESTRIKE),
    };

    for (uint32 spellId : patchSpells)
    {
        DynamicObject* patch = zerevor->GetDynObject(spellId);
        if (patch && patch->GetExactDist2d(point) < patch->GetRadius() + margin)
            return true;
    }

    return false;
}

bool IsInCouncilPatch(Position const& point, Unit* gathios, Unit* zerevor, float margin)
{
    if (!gathios || !zerevor)
        return false;

    DynamicObject* consecration = gathios->GetDynObject(Id(BtSpells::SPELL_CONSECRATION));
    if (consecration && consecration->GetExactDist2d(point) < consecration->GetRadius() + margin)
        return true;

    return IsInZerevorPatch(zerevor, point, margin);
}

bool IsMalandeInCouncilPatch(Unit* malande, Unit* gathios, Unit* zerevor)
{
    return malande && gathios && zerevor && IsInCouncilPatch(*malande, gathios, zerevor);
}

// Ranged interrupt only when melee is unable to (when Malande is under Blessing of Protection
// or is standing in a hazard patch).
bool ShouldRangedInterruptCircleOfHealing(Unit* malande, Unit* gathios, Unit* zerevor)
{
    if (!malande || malande->HasAura(Id(BtSpells::SPELL_BLESSING_OF_SPELL_WARDING)))
        return false;

    return malande->HasAura(Id(BtSpells::SPELL_BLESSING_OF_PROTECTION)) ||
        IsMalandeInCouncilPatch(malande, gathios, zerevor);
}

bool IsZerevorOnMageTank(PlayerbotAI* botAI, Unit* zerevor)
{
    if (!zerevor)
        return false;

    Player* mageTank = GetZerevorMageTank(botAI);
    return mageTank && zerevor->GetVictim() == mageTank;
}

// Within heal range of the mage tank, clear of Arcane Explosion, and not standing in a patch.
bool IsMageTankHealerPositionSafe(Position const& point, Player* mageTank, Unit* zerevor)
{
    if (!mageTank || !zerevor)
        return false;

    float const toMageTank = mageTank->GetExactDist2d(point);
    if (toMageTank > MAGE_TANK_HEALER_MAX_DISTANCE || toMageTank < MAGE_TANK_HEALER_MIN_DISTANCE)
        return false;

    return zerevor->GetExactDist2d(point) >= ZEREVOR_ARCANE_EXPLOSION_SAFE_DISTANCE &&
        !IsInZerevorPatch(zerevor, point, COUNCIL_PATCH_MARGIN);
}

namespace
{

// A blocked ray stops at the obstacle and still counts as success, so a point that returns short
// means the move is blocked. Both callers compute the spot rather than using a fixed position (mage
// tank healer, Malande tank) and need it clear of the council patches, so a candidate that clamps
// short is rejected instead of handed to MoveTo, which would stop the bot inside a patch.
bool CanWalkStraightTo(Player* bot, Position const& point)
{
    constexpr float clampTolerance = 0.5f;
    float x = point.GetPositionX();
    float y = point.GetPositionY();
    float z = point.GetPositionZ();
    return bot->GetMap()->CheckCollisionAndGetValidCoords(
               bot, bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(), x, y, z) &&
        point.GetExactDist2d(x, y) <= clampTolerance;
}

} // end anonymous namespace (Illidari Council)

bool FindMageTankHealerPosition(
    Player* bot, Player* mageTank, Unit* zerevor, Position& destination)
{
    if (!mageTank || !zerevor)
        return false;

    static constexpr std::array ringRadii = { 16.0f, 24.0f };
    constexpr uint8 pointsPerRing = 16;

    std::vector<Position> candidates;
    for (float radius : ringRadii)
    {
        for (uint8 i = 0; i < pointsPerRing; ++i)
        {
            float const angle = 2.0f * static_cast<float>(M_PI) * i / pointsPerRing;
            Position const point(
                mageTank->GetPositionX() + radius * std::cos(angle),
                mageTank->GetPositionY() + radius * std::sin(angle), bot->GetPositionZ());
            if (IsMageTankHealerPositionSafe(point, mageTank, zerevor))
                candidates.push_back(point);
        }
    }

    std::sort(
        candidates.begin(), candidates.end(),
        [bot](Position const& a, Position const& b)
        {
            return bot->GetExactDist2d(a) < bot->GetExactDist2d(b);
        });

    for (Position const& candidate : candidates)
    {
        if (!CanWalkStraightTo(bot, candidate) ||
            !mageTank->IsWithinLOS(
                candidate.GetPositionX(), candidate.GetPositionY(), candidate.GetPositionZ()))
        {
            continue;
        }

        destination = candidate;
        return true;
    }

    return false;
}

bool FindMalandeTankPosition(
    Player* bot, Unit* malande, Unit* gathios, Unit* zerevor, Position& destination)
{
    if (!malande || !gathios || !zerevor)
        return false;

    static constexpr std::array ringFractions = { 0.5f, 0.85f };
    constexpr uint8 pointsPerRing = 16;
    float const meleeRange = bot->GetMeleeRange(malande);

    std::vector<Position> candidates;
    for (float fraction : ringFractions)
    {
        float const radius = meleeRange * fraction;
        for (uint8 i = 0; i < pointsPerRing; ++i)
        {
            float const angle = 2.0f * static_cast<float>(M_PI) * i / pointsPerRing;
            Position const point(
                malande->GetPositionX() + radius * std::cos(angle),
                malande->GetPositionY() + radius * std::sin(angle), bot->GetPositionZ());

            if (!IsInCouncilPatch(point, gathios, zerevor, COUNCIL_PATCH_MARGIN))
                candidates.push_back(point);
        }
    }

    std::sort(
        candidates.begin(), candidates.end(),
        [bot](Position const& a, Position const& b)
        {
            return bot->GetExactDist2d(a) < bot->GetExactDist2d(b);
        });

    for (Position const& candidate : candidates)
    {
        if (!CanWalkStraightTo(bot, candidate))
            continue;

        destination = candidate;
        return true;
    }

    return false;
}

bool IsAnotherCouncilMemberWithin(PlayerbotAI* botAI, float range)
{
    Player* bot = botAI->GetBot();
    AiObjectContext* context = botAI->GetAiObjectContext();
    Unit* ownTarget = AI_VALUE(Unit*, "current target");
    IllidariCouncilMembers const members = GetIllidariCouncilMembers(botAI);
    for (Unit* member : { members.gathios, members.malande, members.zerevor, members.veras })
    {
        if (member && member != ownTarget && bot->GetDistance(member) < range)
            return true;
    }

    return false;
}

// Felhunters stay on Malande for Spell Lock; other pets attack their master's target, or Gathios
// when the master has none.
Unit* GetCouncilPetTarget(PlayerbotAI* botAI, Creature* pet)
{
    if (!pet)
        return nullptr;

    IllidariCouncilMembers const members = GetIllidariCouncilMembers(botAI);
    if (!members.gathios)
        return nullptr;

    if (pet->GetEntry() == NPC_FELHUNTER && members.malande)
        return members.malande;

    AiObjectContext* context = botAI->GetAiObjectContext();
    Unit* target = AI_VALUE(Unit*, "current target");
    return target && target->IsAlive() ? target : members.gathios;
}

// CanCastSpell accepts any spell a pet knows without checking range or cooldown, so check manually.
uint32 GetReadySpellLock(Creature* pet, Unit* target)
{
    if (!pet || !target || !pet->IsAlive() || pet->GetEntry() != NPC_FELHUNTER)
        return 0;

    if (!pet->IsWithinCombatRange(target, SPELL_LOCK_RANGE) || !pet->IsWithinLOSInMap(target))
        return 0;

    static constexpr std::array spellLockRanks = {
        Id(BtSpells::SPELL_SPELL_LOCK_2),
        Id(BtSpells::SPELL_SPELL_LOCK_1),
    };

    for (uint32 spellLock : spellLockRanks)
    {
        if (pet->HasSpell(spellLock))
            return pet->HasSpellCooldown(spellLock) ? 0 : spellLock;
    }

    return 0;
}

// Illidan Stormrage <The Betrayer>

namespace // Illidan Stormrage
{

// A point `distance` yd from Illidan and `angle` off the line from him through the grate center,
// positive toward the warlock tank's side.
Position GetDemonFormPoint(Position const& illidan, float distance, float angle)
{
    float const bearing = illidan.GetAngle(&ILLIDAN_GRATE_CENTER) - angle;
    return Position(
        illidan.GetPositionX() + distance * std::cos(bearing),
        illidan.GetPositionY() + distance * std::sin(bearing), illidan.GetPositionZ());
}

// One row of Demon Form positions 6 yd apart, from highAngle down, each with its angle. All are
// clear of Shadow Blast targeted at the warlock position and are within 36 yd of the grate center.
std::vector<std::pair<float, Position>> GetDemonFormRow(
    Position const& illidan, Position const& warlockTankPosition, float distance, float highAngle,
    float lowAngle, bool halfStepIn)
{
    constexpr float slotSpacing = 6.0f;
    constexpr float maxCenterDistance = 36.0f;

    float const step = slotSpacing / distance;
    float const firstAngle = halfStepIn ? highAngle - step / 2.0f : highAngle;
    int32 const count = static_cast<int32>(std::floor((firstAngle - lowAngle) / step)) + 1;

    std::vector<std::pair<float, Position>> row;
    for (int32 i = 0; i < count; ++i)
    {
        float const angle = firstAngle - i * step;
        Position const point = GetDemonFormPoint(illidan, distance, angle);
        if (point.GetExactDist2d(warlockTankPosition) >= ILLIDAN_SHADOW_BLAST_CLEARANCE &&
            ILLIDAN_GRATE_CENTER.GetExactDist2d(point) <= maxCenterDistance)
        {
            row.emplace_back(angle, point);
        }
    }

    return row;
}

int32 GetIllidanRosterIndex(
    std::vector<ObjectGuid> first, std::vector<ObjectGuid> second, ObjectGuid guid,
    size_t slotCount)
{
    std::sort(first.begin(), first.end());
    std::sort(second.begin(), second.end());
    first.insert(first.end(), second.begin(), second.end());

    auto const it = std::find(first.begin(), first.end(), guid);
    if (it == first.end())
        return -1;

    size_t const index = static_cast<size_t>(std::distance(first.begin(), it));
    return static_cast<int32>(index % slotCount);
}

Player* FindFlameOfAzzinothTank(Player* bot, bool isEast)
{
    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member &&
            (isEast ? IsEastFlameOfAzzinothTank(member) : IsWestFlameOfAzzinothTank(member)))
        {
            return member;
        }
    }

    return nullptr;
}

} // end anonymous namespace (Illidan Stormrage)

std::unordered_map<ObjectGuid, IllidanInfection> const* FindIllidanInfections(uint32 instanceId)
{
    BtInstanceState const* state = FindBtState(instanceId);
    return state && state->illidanInfections ? &*state->illidanInfections : nullptr;
}

IllidanPhase GetIllidanPhase(Unit* illidan)
{
    if (!illidan)
        return IllidanPhase::None;

    if (illidan->GetHealth() == 1)
        return IllidanPhase::Dying;

    float const healthPct = illidan->GetHealthPct();
    if (illidan->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE))
    {
        if (healthPct <= ILLIDAN_PHASE_5_HEALTH_PCT)
            return IllidanPhase::None;

        constexpr float landingPointTolerance = 0.2f;
        float x;
        float y;
        float z;
        bool const isLanding =
            illidan->GetExactDist2d(ILLIDAN_LANDING_POSITION) < landingPointTolerance ||
            (illidan->GetMotionMaster()->GetDestination(x, y, z) &&
             ILLIDAN_LANDING_POSITION.GetExactDist2d(x, y) < landingPointTolerance);

        return isLanding ? IllidanPhase::Landing : IllidanPhase::Flying;
    }

    if (healthPct > ILLIDAN_PHASE_2_HEALTH_PCT)
    {
        // Above 65% HP is phase 1 only if the tracker has recorded nothing but phase 1. Draw Soul
        // can be cast at liftoff, healing Illidan to over 65% for the duration of phase 2 and
        // causing him to enter phase 3 above 65% HP.
        BtInstanceState const* state = FindBtState(illidan->GetInstanceId());
        bool const isPastInitial = state && state->illidanLastPhase &&
            *state->illidanLastPhase != IllidanPhase::Initial;
        if (!isPastInitial)
            return IllidanPhase::Initial;
    }

    if (illidan->HasAura(Id(BtSpells::SPELL_DEMON_FORM)) ||
        illidan->HasAura(Id(BtSpells::SPELL_DEMON_TRANSFORM_1)) ||
        illidan->HasAura(Id(BtSpells::SPELL_DEMON_TRANSFORM_2)) ||
        illidan->HasAura(Id(BtSpells::SPELL_DEMON_TRANSFORM_3)))
    {
        return IllidanPhase::Demon;
    }

    return healthPct > ILLIDAN_PHASE_5_HEALTH_PCT ? IllidanPhase::Grounded : IllidanPhase::Maiev;
}

bool IsIllidanSpeechInProgress(Player* bot)
{
    if (IsEncounterInProgress(bot, BT_MAP_ID) ||
        bot->GetExactDist(ILLIDAN_SPEECH_POSITION) > ILLIDAN_SPEECH_SEARCH_RADIUS)
    {
        return false;
    }

    Creature* illidan =
        GetInstanceCreature(bot, BT_MAP_ID, Id(BtBosses::DATA_ILLIDAN_STORMRAGE));

    return illidan && illidan->getStandState() == UNIT_STAND_STATE_STAND &&
        illidan->HasUnitFlag(UNIT_FLAG_IMMUNE_TO_PC) && !illidan->IsInCombat();
}

// Written by the mechanic tracker, or at the Demon Form transform by a spell listener. True if the
// phase changed.
bool RecordIllidanPhase(uint32 instanceId, IllidanPhase phase)
{
    BtInstanceState& state = BtState(instanceId);
    if (state.illidanLastPhase == phase)
        return false;

    state.illidanPreviousPhase = state.illidanLastPhase.value_or(IllidanPhase::None);
    state.illidanLastPhase = phase;
    if (phase == IllidanPhase::Initial || phase == IllidanPhase::Grounded ||
        phase == IllidanPhase::Demon || phase == IllidanPhase::Maiev)
    {
        state.illidanPhaseStartTime = getMSTime();
    }
    else if (phase == IllidanPhase::Flying)
    {
        ResetIfSet(state.illidanPhaseStartTime);
    }

    if (phase != IllidanPhase::Flying)
        ResetIfSet(state.illidanFlamePhaseStartTime);

    ResetIfSet(state.illidanTankAggroPhase);
    return true;
}

bool IsIllidanPhaseStartWithin(Unit const& illidan, IllidanPhase phase, uint32 durationMs)
{
    BtInstanceState const* state = FindBtState(illidan.GetInstanceId());
    return !state || state->illidanLastPhase != phase || !state->illidanPhaseStartTime ||
        GetMSTimeDiffToNow(*state->illidanPhaseStartTime) < durationMs;
}

IllidanPhase GetIllidanPhaseBefore(Unit const& illidan, IllidanPhase phase)
{
    BtInstanceState const* state = FindBtState(illidan.GetInstanceId());
    if (!state || !state->illidanLastPhase)
        return IllidanPhase::None;

    if (*state->illidanLastPhase != phase)
        return *state->illidanLastPhase;

    return state->illidanPreviousPhase.value_or(IllidanPhase::None);
}

bool IsIllidanDemonFormEnding(Unit const& illidan, uint32 durationMs)
{
    BtInstanceState const* state = FindBtState(illidan.GetInstanceId());
    if (!state || state->illidanLastPhase != IllidanPhase::Demon)
        return false;

    return state->illidanPhaseStartTime && GetMSTimeDiffToNow(*state->illidanPhaseStartTime) >=
        ILLIDAN_DEMON_FORM_DURATION_MS - durationMs;
}

IllidanDpsHold GetIllidanHoldWindow(Unit* illidan)
{
    if (!illidan)
        return IllidanDpsHold::None;

    IllidanPhase const phase = GetIllidanPhase(illidan);
    if (phase == IllidanPhase::Demon)
    {
        // A phase 5 form is already under 30%, so it gets the usual windows.
        float const healthPct = illidan->GetHealthPct();
        if (healthPct > ILLIDAN_PHASE_5_HEALTH_PCT &&
            healthPct <= ILLIDAN_DEMON_FORM_NO_PUSH_HEALTH_PCT)
        {
            return IllidanDpsHold::DemonFormNoPush;
        }

        if (IsIllidanDemonFormEnding(*illidan, ILLIDAN_DEMON_FORM_END_HOLD_MS))
            return IllidanDpsHold::DemonFormEnd;

        return IsIllidanPhaseStartWithin(*illidan, phase, ILLIDAN_DEMON_FORM_DPS_WAIT_MS) ?
            IllidanDpsHold::DemonFormStart : IllidanDpsHold::None;
    }

    bool const isAfterReset = phase == IllidanPhase::Initial || phase == IllidanPhase::Grounded ||
        phase == IllidanPhase::Maiev;

    return isAfterReset && IsIllidanPhaseStartWithin(*illidan, phase, ILLIDAN_MAIN_TANK_PICKUP_MS) ?
        IllidanDpsHold::AfterReset : IllidanDpsHold::None;
}

IllidanDpsHold GetIllidanDpsHold(PlayerbotAI* botAI, Unit* illidan)
{
    if (!illidan)
        return IllidanDpsHold::None;

    IllidanDpsHold const window = GetIllidanHoldWindow(illidan);
    Player* bot = botAI->GetBot();
    if (window == IllidanDpsHold::DemonFormStart || window == IllidanDpsHold::DemonFormNoPush)
    {
        return IsIllidanWarlockTank(botAI) ? IllidanDpsHold::None : window;
    }
    else if (window == IllidanDpsHold::AfterReset)
    {
        return PlayerbotAI::IsTank(bot) && PlayerbotAI::IsMainTank(bot) ?
            IllidanDpsHold::None : window;
    }

    return window;
}

bool IsFlameOfAzzinothDpsHeld(Unit const& illidan)
{
    BtInstanceState const* state = FindBtState(illidan.GetInstanceId());
    return state && state->illidanFlamePhaseStartTime &&
        GetMSTimeDiffToNow(*state->illidanFlamePhaseStartTime) < ILLIDAN_FLAME_DPS_WAIT_MS;
}

bool IsOnlineOnIllidanThreatList(Unit const& illidan, Player* bot)
{
    for (ThreatReference const* ref : illidan.GetThreatMgr().GetUnsortedThreatList())
    {
        if (ref->GetVictim() == bot)
            return ref->GetOnlineState() == ThreatReference::ONLINE_STATE_ONLINE;
    }

    return false;
}

// True from each phase change until the tracker sees the tanks have aggro, and through the landing
// and Maiev's arrival, but never at the pull or in the death scene.
bool IsIllidanTankAggroPending(Unit const& illidan, IllidanPhase phase)
{
    if (phase == IllidanPhase::Landing || phase == IllidanPhase::None)
        return true;

    if (phase != IllidanPhase::Flying && phase != IllidanPhase::Grounded &&
        phase != IllidanPhase::Demon && phase != IllidanPhase::Maiev)
    {
        return false;
    }

    BtInstanceState const* state = FindBtState(illidan.GetInstanceId());
    return !state || state->illidanTankAggroPhase != phase;
}

// Each tank that should hold a creature must be its victim with ILLIDAN_TANK_AGGRO_THREAT.
// Illidan: the main tank, or the warlock tank in Demon Form. While he flies, each living Flame: its
// own tank, or the other Flame tank while its own is dead. A dead tank counts as holding it.
bool DoIllidanTanksHaveAggro(PlayerbotAI* botAI, Unit const& illidan, IllidanPhase phase)
{
    auto const hasAggro = [](Unit const& creature, Player* tank)
    {
        ThreatManager const& threatManager = creature.GetThreatMgr();
        return threatManager.GetLastVictim() == tank &&
            threatManager.GetThreat(tank) >= ILLIDAN_TANK_AGGRO_THREAT;
    };

    Player* bot = botAI->GetBot();
    switch (phase)
    {
        case IllidanPhase::Flying:
        {
            BtInstanceState const* state = FindBtState(illidan.GetInstanceId());
            if (!state || !state->eastFlameGuid || !state->westFlameGuid)
                return false;

            for (bool const isEast : { true, false })
            {
                Creature* flame = illidan.GetMap()->GetCreature(
                    isEast ? *state->eastFlameGuid : *state->westFlameGuid);
                if (!flame || !flame->IsAlive())
                    continue;

                Player* tank = FindFlameOfAzzinothTank(bot, isEast);
                if (!tank || !tank->IsAlive())
                    tank = FindFlameOfAzzinothTank(bot, !isEast);

                if (tank && tank->IsAlive() && !hasAggro(*flame, tank))
                    return false;
            }

            return true;
        }
        case IllidanPhase::Initial:
        case IllidanPhase::Grounded:
        case IllidanPhase::Maiev:
        {
            Player* mainTank = GetGroupMainTank(bot);
            return !mainTank || hasAggro(illidan, mainTank);
        }
        case IllidanPhase::Demon:
        {
            Player* warlockTank = GetIllidanWarlockTank(botAI);
            return !warlockTank || hasAggro(illidan, warlockTank);
        }
        default:
            return false;
    }
}

bool IsIllidanPickupPending(PlayerbotAI* botAI, Unit* illidan)
{
    if (!illidan)
        return false;

    // Illidan lands 3.665s after reaching the landing point, and threat is reset 2.43s later.
    // Spells are not held until the landing, so it is possible for a heal to start right as he
    // lands and resolve right after the reset. I don't believe this is likely to be a problem (by
    // the time Illidan lands, the party should be topped up or close to it such that only fast
    // heals are being cast), but it is worth noting in case that comes up. HoTs are potentially
    // also an issue. If need be, bots can be made to "right-click" them off with aura removal.
    IllidanPhase const phase = GetIllidanPhase(illidan);
    if (phase == IllidanPhase::Landing)
        return !illidan->IsLevitating();

    if (phase != IllidanPhase::Grounded && phase != IllidanPhase::Maiev)
        return false;

    IllidanPhase const before = GetIllidanPhaseBefore(*illidan, phase);
    if ((before != IllidanPhase::Landing && before != IllidanPhase::Demon) ||
        !IsIllidanPhaseStartWithin(*illidan, phase, ILLIDAN_MAIN_TANK_PICKUP_MS))
    {
        return false;
    }

    return !DoIllidanTanksHaveAggro(botAI, *illidan, phase);
}

// Single-target damage on a Shadowfiend or a Shadow Demon is permitted at all times.
bool IsIllidanPickupExemptTarget(Unit* target, bool isAoe)
{
    if (!target || isAoe)
        return false;

    uint32 const entry = target->GetEntry();
    return entry == Id(BtNpcs::NPC_PARASITIC_SHADOWFIEND) || entry == Id(BtNpcs::NPC_SHADOW_DEMON);
}

bool ShouldIllidanPetsBePassive(PlayerbotAI* botAI, Unit* illidan)
{
    IllidanPhase const phase = GetIllidanPhase(illidan);
    return phase == IllidanPhase::Flying || phase == IllidanPhase::Demon ||
        IsIllidanPickupPending(botAI, illidan);
}

// After a threat reset, a channel cast before the hold continues to build threat. This often
// results in Illidan immediately killing the channeler before the main tank can pick him up. The
// main culprit is Evocation. So everybody other than the main tank should cancel a channel (other
// than single-target damage on an add).
bool IsIllidanPickupChannelHeld(PlayerbotAI* botAI, Unit* illidan, Spell const& channel)
{
    if (!IsIllidanPickupPending(botAI, illidan))
        return false;

    Player* bot = botAI->GetBot();
    if (PlayerbotAI::IsTank(bot) && PlayerbotAI::IsMainTank(bot))
        return false;

    return !IsIllidanPickupExemptTarget(
        channel.m_targets.GetUnitTarget(), channel.GetSpellInfo()->IsAffectingArea());
}

// The point's angle from the grate center: 0 north, increasing counterclockwise.
float GetGrateAngle(Position const& position)
{
    return ILLIDAN_GRATE_CENTER.GetAngle(&position);
}

Position GetRingPoint(float angle, float radius)
{
    return Position(
        ILLIDAN_GRATE_CENTER.GetPositionX() + radius * std::cos(angle),
        ILLIDAN_GRATE_CENTER.GetPositionY() + radius * std::sin(angle),
        ILLIDAN_GRATE_CENTER.GetPositionZ());
}

Position GetIllidanSettlePosition(Unit const& illidan)
{
    float const radius = ILLIDAN_GRATE_CENTER.GetExactDist2d(&illidan) + ILLIDAN_TANK_OFFSET;
    return GetRingPoint(GetGrateAngle(illidan), radius);
}

Position GetGrateSlot(size_t index)
{
    constexpr float fullCircle = 2.0f * static_cast<float>(M_PI);

    size_t const slot = index % ILLIDAN_RANGED_SLOT_COUNT;
    if (slot == 0)
        return ILLIDAN_GRATE_CENTER;

    if (slot <= ILLIDAN_GRATE_INNER_SLOT_COUNT)
    {
        float const angle =
            fullCircle * static_cast<float>(slot - 1) / ILLIDAN_GRATE_INNER_SLOT_COUNT;
        return GetRingPoint(angle, ILLIDAN_GRATE_INNER_RADIUS);
    }

    float const angle = fullCircle *
        (static_cast<float>(slot - 1 - ILLIDAN_GRATE_INNER_SLOT_COUNT) + 0.5f) /
        ILLIDAN_GRATE_OUTER_SLOT_COUNT;
    return GetRingPoint(angle, ILLIDAN_GRATE_OUTER_RADIUS);
}

DemonFormLayout GetDemonFormLayout(Unit const& illidan)
{
    constexpr float degree = static_cast<float>(M_PI) / 180.0f;
    // Float values for the arrays are exact 2D distances from Illidan.
    constexpr std::array rangedRows = { 25.2f, 31.0f };
    constexpr std::array healerRows = { 36.8f, 42.6f };
    static_assert(rangedRows.size() == healerRows.size());
    constexpr std::array meleeRows = { 48.4f, 54.2f, 60.0f };
    constexpr float rangedHighAngle = 40.0f * degree;
    constexpr float healerHighAngle = 60.0f * degree;
    constexpr float lowAngle = -90.0f * degree;
    constexpr float meleeHalfArc = 40.0f * degree;

    DemonFormLayout layout;
    layout.warlockTankPosition = GetDemonFormPoint(
        illidan, ILLIDAN_WARLOCK_POSITION_DISTANCE, ILLIDAN_WARLOCK_POSITION_ANGLE);
    layout.mainTankWaitPosition = GetDemonFormPoint(
        illidan, ILLIDAN_MAIN_TANK_WAIT_DISTANCE, ILLIDAN_MAIN_TANK_WAIT_ANGLE);
    layout.mainTankPickupPosition = GetDemonFormPoint(
        illidan, ILLIDAN_PICKUP_POSITION_DISTANCE, ILLIDAN_PICKUP_POSITION_ANGLE);
    layout.mainTankStepInPosition = GetDemonFormPoint(
        illidan, ILLIDAN_STEP_IN_POSITION_DISTANCE, ILLIDAN_PICKUP_POSITION_ANGLE);

    auto const fromWarlockSide =
        [](std::pair<float, Position> const& a, std::pair<float, Position> const& b)
    {
        return a.first > b.first;
    };

    std::vector<std::pair<float, Position>> ranged;
    std::vector<std::pair<float, Position>> healers;
    for (size_t i = 0; i < rangedRows.size(); ++i)
    {
        std::vector<std::pair<float, Position>> const rangedRow = GetDemonFormRow(
            illidan, layout.warlockTankPosition, rangedRows[i], rangedHighAngle, lowAngle,
            i % 2 == 1);
        ranged.insert(ranged.end(), rangedRow.begin(), rangedRow.end());

        std::vector<std::pair<float, Position>> const healerRow = GetDemonFormRow(
            illidan, layout.warlockTankPosition, healerRows[i], healerHighAngle, lowAngle,
            i % 2 == 1);
        healers.insert(healers.end(), healerRow.begin(), healerRow.end());
    }

    if (ranged.empty())
        return layout;

    std::stable_sort(ranged.begin(), ranged.end(), fromWarlockSide);
    if (ranged.size() > ILLIDAN_RANGED_SLOT_COUNT)
        ranged.resize(ILLIDAN_RANGED_SLOT_COUNT);

    for (std::pair<float, Position> const& slot : ranged)
        layout.rangedSlots.push_back(slot.second);

    std::stable_sort(healers.begin(), healers.end(), fromWarlockSide);
    if (healers.size() > ILLIDAN_HEALER_SLOT_COUNT)
        healers.resize(ILLIDAN_HEALER_SLOT_COUNT);

    for (std::pair<float, Position> const& slot : healers)
        layout.healerSlots.push_back(slot.second);

    float const meleeCenterAngle = (ILLIDAN_WARLOCK_POSITION_ANGLE + ranged.back().first) / 2.0f;
    for (size_t i = 0; i < meleeRows.size(); ++i)
    {
        std::vector<std::pair<float, Position>> row = GetDemonFormRow(
            illidan, layout.warlockTankPosition, meleeRows[i], meleeCenterAngle + meleeHalfArc,
            meleeCenterAngle - meleeHalfArc, i % 2 == 1);
        std::stable_sort(row.begin(), row.end(), fromWarlockSide);

        for (std::pair<float, Position> const& slot : row)
        {
            if (layout.meleeSlots.size() < ILLIDAN_MELEE_SLOT_COUNT)
                layout.meleeSlots.push_back(slot.second);
        }
    }

    return layout;
}

Position GetIllidanInfectionPosition(Unit const& illidan)
{
    return GetRingPoint(
        GetGrateAngle(illidan) + ILLIDAN_INFECTION_POSITION_ANGLE,
        ILLIDAN_INFECTION_POSITION_DISTANCE);
}

// Every recorded infection shares one position, fixed when the first was recorded. If none are
// recorded, this returns the position an infection would be recorded at now.
Position GetIllidanRecordedInfectionPosition(PlayerbotAI* botAI, Unit const& illidan)
{
    auto const* infections = FindIllidanInfections(botAI->GetBot()->GetInstanceId());
    if (infections && !infections->empty())
        return infections->begin()->second.infectionPosition;

    return GetIllidanInfectionPosition(illidan);
}

std::pair<Unit*, Unit*> GetFlamesOfAzzinoth(Player* bot)
{
    BtInstanceState const* state = FindBtState(bot->GetInstanceId());
    if (!state)
        return { nullptr, nullptr };

    auto const findLivingFlame = [bot](std::optional<ObjectGuid> const& guid) -> Unit*
    {
        Unit* flame = guid ? ObjectAccessor::GetUnit(*bot, *guid) : nullptr;
        return flame && flame->IsAlive() ? flame : nullptr;
    };

    return { findLivingFlame(state->eastFlameGuid), findLivingFlame(state->westFlameGuid) };
}

bool IsWestFlameOfAzzinothTank(Player* bot)
{
    return PlayerbotAI::IsAssistTankOfIndex(bot, ILLIDAN_WEST_FLAME_TANK_INDEX);
}

bool IsEastFlameOfAzzinothTank(Player* bot)
{
    return PlayerbotAI::IsAssistTankOfIndex(bot, ILLIDAN_EAST_FLAME_TANK_INDEX);
}

bool IsFlameOfAzzinothTank(Player* bot)
{
    return IsEastFlameOfAzzinothTank(bot) || IsWestFlameOfAzzinothTank(bot);
}

Unit* GetAssignedFlameOfAzzinoth(Player* bot)
{
    bool const isEastTank = IsEastFlameOfAzzinothTank(bot);
    if (!isEastTank && !IsWestFlameOfAzzinothTank(bot))
        return nullptr;

    auto const [eastFlame, westFlame] = GetFlamesOfAzzinoth(bot);
    Unit* ownFlame = isEastTank ? eastFlame : westFlame;
    if (ownFlame)
        return ownFlame;

    BtInstanceState const* state = FindBtState(bot->GetInstanceId());
    bool const isOwnFlameRecorded =
        state && (isEastTank ? state->eastFlameGuid : state->westFlameGuid).has_value();
    Unit* otherFlame = isEastTank ? westFlame : eastFlame;
    if (!otherFlame || !isOwnFlameRecorded)
        return nullptr;

    Player* otherTank = FindFlameOfAzzinothTank(bot, !isEastTank);
    return !otherTank || !otherTank->IsAlive() ? otherFlame : nullptr;
}

bool UpdateIllidanClassRoleAnnouncement(PlayerbotAI* botAI)
{
    BtInstanceState& state = BtState(botAI->GetBot()->GetInstanceId());
    // Illidan spawns right above the Illidari Council, so no announcement is made while Zerevor is
    // still alive (could be any council member used for the check).
    return UpdateClassRoleAnnouncement(
        botAI, Id(BtNpcs::NPC_ILLIDAN_STORMRAGE), ILLIDAN_SPEECH_POSITION,
        ILLIDAN_CLASS_ROLE_ANNOUNCE_DISTANCE, ILLIDAN_CLASS_ROLE_REARM_DISTANCE, {
            { ILLIDAN_WARLOCK_TANK_ROLE, state.illidanWarlockTank },
        }, Id(BtNpcs::NPC_HIGH_NETHERMANCER_ZEREVOR));
}

ObjectGuid GetIllidanWarlockTankGuid(Player* bot)
{
    BtInstanceState const* state = FindBtState(bot->GetInstanceId());
    return state ? GetClassRolePlayerGuid(state->illidanWarlockTank) : ObjectGuid::Empty;
}

Player* GetIllidanWarlockTank(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    BtInstanceState const* state = FindBtState(bot->GetInstanceId());
    return state ? GetClassRolePlayer(bot, state->illidanWarlockTank) : nullptr;
}

bool IsIllidanWarlockTank(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    if (bot->getClass() != CLASS_WARLOCK)
        return false;

    BtInstanceState const* state = FindBtState(bot->GetInstanceId());
    return state && IsClassRolePlayer(bot, state->illidanWarlockTank);
}

bool CastIllidanLifeTapIfLow(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    if (bot->GetPowerPct(POWER_MANA) >= ILLIDAN_LIFE_TAP_MANA_PCT ||
        bot->GetHealthPct() <= ILLIDAN_LIFE_TAP_HEALTH_PCT)
    {
        return false;
    }

    return botAI->CanCastSpell("life tap", bot) && botAI->CastSpell("life tap", bot);
}

GuidVector FindIllidanTrapHunterGuids(Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group)
        return {};

    GuidVector trapHunters;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->GetMapId() == BT_MAP_ID && member->getClass() == CLASS_HUNTER &&
            GET_PLAYERBOT_AI(member))
        {
            trapHunters.push_back(member->GetGUID());
        }
    }

    std::sort(trapHunters.begin(), trapHunters.end());
    return trapHunters;
}

bool IsIllidanTrapHunterReady(Player* hunter)
{
    return hunter && hunter->IsAlive() &&
        !hunter->HasSpellCooldown(Id(BtSpells::SPELL_FROST_TRAP)) &&
        !HasParasiticShadowfiend(hunter);
}

bool IsIllidanTrapHunterOnDuty(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    if (bot->getClass() != CLASS_HUNTER || bot->HasSpellCooldown(Id(BtSpells::SPELL_FROST_TRAP)) ||
        HasParasiticShadowfiend(bot))
    {
        return false;
    }

    auto const* infections = FindIllidanInfections(bot->GetInstanceId());
    if (!infections)
        return false;

    for (auto const& [infected, infection] : *infections)
    {
        if (infection.hunter == bot->GetGUID())
            return true;
    }

    return false;
}

// The bots that never run to the infection position (so Shadowfiends spawn where they stand):
// Illidan's victim, the main tank, and the warlock tank in Demon Form.
bool IsExemptFromIllidanInfectionRun(PlayerbotAI* botAI, Unit* illidan, Player* player)
{
    if (!illidan || !player)
        return false;

    if (illidan->GetVictim() == player ||
        (PlayerbotAI::IsTank(player) && PlayerbotAI::IsMainTank(player)))
    {
        return true;
    }

    return GetIllidanPhase(illidan) == IllidanPhase::Demon &&
        GetIllidanWarlockTank(botAI) == player;
}

// A bot infected in phase 1 that still has the debuff when phase 2 starts will stay at the
// infection position until its Shadowfiends spawn, and the trap hunter will still drop the trap.
// Exception: The main tank will wait out the spawn at its position at takeoff since it never
// would have gone to the infection position in the first place.
bool IsIllidanInfectionPositionKeptInFlight(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    auto const* infections = FindIllidanInfections(bot->GetInstanceId());
    if (!infections)
        return false;

    if (HasParasiticShadowfiend(bot) && infections->contains(bot->GetGUID()))
        return true;

    return IsIllidanTrapHunterOnDuty(botAI);
}

bool IsFrostTrapAt(Player* bot, Position const& infectionPosition, int32 minDurationMs)
{
    std::list<GameObject*> traps;
    bot->GetGameObjectListWithEntryInGrid(
        traps, Id(BtObjects::GO_FROST_TRAP), ILLIDAN_ADD_SEARCH_RADIUS);
    for (GameObject* trap : traps)
    {
        if (trap->isSpawned() &&
            trap->GetExactDist2d(infectionPosition) <= ILLIDAN_FROST_TRAP_DISTANCE)
        {
            return true;
        }
    }

    std::list<WorldObject*> objects;
    Acore::AllWorldObjectsInRange check(bot, ILLIDAN_ADD_SEARCH_RADIUS);
    Acore::WorldObjectListSearcher<Acore::AllWorldObjectsInRange> searcher(
        bot, objects, check, GRID_MAP_TYPE_MASK_DYNAMICOBJECT);
    Cell::VisitObjects(bot, searcher, ILLIDAN_ADD_SEARCH_RADIUS);

    for (WorldObject* object : objects)
    {
        DynamicObject* patch = object->ToDynObject();
        if (patch && patch->GetSpellId() == Id(BtSpells::SPELL_FROST_TRAP_AURA) &&
            patch->GetExactDist2d(infectionPosition) < patch->GetRadius() &&
            patch->GetDuration() >= minDurationMs)
        {
            return true;
        }
    }

    return false;
}

// The index wraps round slotCount, so in the unlikely event that there are more bots than available
// slots, they start stacking up.
int32 GetIllidanRangedIndex(PlayerbotAI* botAI, size_t slotCount, bool inDemonForm)
{
    Player* bot = botAI->GetBot();
    Group* group = bot->GetGroup();
    if (!group || slotCount == 0)
        return -1;

    ObjectGuid const warlockTankGuid =
        inDemonForm ? GetIllidanWarlockTankGuid(bot) : ObjectGuid::Empty;

    std::vector<ObjectGuid> healers;
    std::vector<ObjectGuid> rangedDps;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member->GetMapId() != BT_MAP_ID || !GET_PLAYERBOT_AI(member))
            continue;

        ObjectGuid const guid = member->GetGUID();
        if (guid == warlockTankGuid)
            continue;

        if (PlayerbotAI::IsHeal(member))
        {
            if (!inDemonForm)
                healers.push_back(guid);
        }
        else if (PlayerbotAI::IsRanged(member))
        {
            rangedDps.push_back(guid);
        }
    }

    return GetIllidanRosterIndex(healers, rangedDps, bot->GetGUID(), slotCount);
}

int32 GetIllidanHealerIndex(PlayerbotAI* botAI, size_t slotCount)
{
    Player* bot = botAI->GetBot();
    Group* group = bot->GetGroup();
    if (!group || slotCount == 0)
        return -1;

    std::vector<ObjectGuid> healers;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->GetMapId() == BT_MAP_ID && GET_PLAYERBOT_AI(member) &&
            PlayerbotAI::IsHeal(member))
        {
            healers.push_back(member->GetGUID());
        }
    }

    return GetIllidanRosterIndex(healers, {}, bot->GetGUID(), slotCount);
}

int32 GetIllidanMeleeIndex(PlayerbotAI* botAI, size_t slotCount)
{
    Player* bot = botAI->GetBot();
    Group* group = bot->GetGroup();
    if (!group || slotCount == 0)
        return -1;

    ObjectGuid const mainTank = PlayerbotAI::GetMainTankGuid(group);

    std::vector<ObjectGuid> meleeDps;
    std::vector<ObjectGuid> tanks;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member->GetMapId() != BT_MAP_ID || !GET_PLAYERBOT_AI(member))
            continue;

        ObjectGuid const guid = member->GetGUID();
        if (guid == mainTank)
            continue;

        if (PlayerbotAI::IsTank(member))
            tanks.push_back(guid);
        else if (PlayerbotAI::IsMelee(member))
            meleeDps.push_back(guid);
    }

    return GetIllidanRosterIndex(meleeDps, tanks, bot->GetGUID(), slotCount);
}

// Phases 3 and 5: the grate slot by roster, except the bot indexed to the slot that is closest to
// the warlock tank's Demon Form position (measured as of the start of the phase) swaps slots with
// the warlock tank to facilitate a shorter move by the warlock tank to its phase 4 position.
int32 GetIllidanGrateIndex(PlayerbotAI* botAI, Unit const& illidan)
{
    constexpr bool inDemonForm = false;
    int32 const index = GetIllidanRangedIndex(botAI, ILLIDAN_RANGED_SLOT_COUNT, inDemonForm);
    if (index < 0)
        return index;

    Player* warlockTank = GetIllidanWarlockTank(botAI);
    PlayerbotAI* warlockTankAI = warlockTank ? GET_PLAYERBOT_AI(warlockTank) : nullptr;
    if (!warlockTankAI)
        return index;

    Position const warlockTankPosition = GetDemonFormPoint(
        illidan, ILLIDAN_WARLOCK_POSITION_DISTANCE, ILLIDAN_WARLOCK_POSITION_ANGLE);
    int32 warlockSlot = 0;
    float nearestDistance = std::numeric_limits<float>::max();
    for (size_t i = 0; i < ILLIDAN_RANGED_SLOT_COUNT; ++i)
    {
        float const distance = GetGrateSlot(i).GetExactDist2d(warlockTankPosition);
        if (distance < nearestDistance)
        {
            nearestDistance = distance;
            warlockSlot = static_cast<int32>(i);
        }
    }

    if (warlockTank == botAI->GetBot())
        return warlockSlot;

    if (index != warlockSlot)
        return index;

    int32 const warlockIndex =
        GetIllidanRangedIndex(warlockTankAI, ILLIDAN_RANGED_SLOT_COUNT, inDemonForm);
    return warlockIndex < 0 ? index : warlockIndex;
}

int32 GetIllidanGrateStackIndex(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    Group* group = bot->GetGroup();
    if (!group)
        return -1;

    std::vector<ObjectGuid> bots;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->GetMapId() == BT_MAP_ID && GET_PLAYERBOT_AI(member) &&
            !IsFlameOfAzzinothTank(member))
        {
            bots.push_back(member->GetGUID());
        }
    }

    return GetIllidanRosterIndex(bots, {}, bot->GetGUID(), ILLIDAN_GRATE_POSITIONS.size());
}

Aura* GetParasiticShadowfiendAura(Player* player)
{
    if (!player)
        return nullptr;

    if (Aura* aura = player->GetAura(Id(BtSpells::SPELL_PARASITIC_SHADOWFIEND_PRI)))
        return aura;

    return player->GetAura(Id(BtSpells::SPELL_PARASITIC_SHADOWFIEND_SEC));
}

bool HasParasiticShadowfiend(Player* player)
{
    return GetParasiticShadowfiendAura(player);
}

// Shadowfiends spawn defensive and turn aggressive and join zone combat 2.4s later. Until then,
// they attack only a unit that attacks them.
GuidVector FindParasiticShadowfiendGuids(PlayerbotAI* botAI)
{
    std::list<Creature*> creatures;
    botAI->GetBot()->GetCreatureListWithEntryInGrid(
        creatures, Id(BtNpcs::NPC_PARASITIC_SHADOWFIEND), ILLIDAN_ADD_SEARCH_RADIUS);

    GuidVector shadowfiends;
    for (Creature* creature : creatures)
    {
        if (creature->IsAlive())
            shadowfiends.push_back(creature->GetGUID());
    }

    return shadowfiends;
}

// A Shadowfiend has a 1 yd combat reach.
Unit* GetNearestParasiticShadowfiend(PlayerbotAI* botAI, Unit const& unit)
{
    auto const& shadowfiends = botAI->GetAiObjectContext()
        ->GetValue<GuidVector>("illidan stormrage parasitic shadowfiends")->RefGet();

    Unit* nearest = nullptr;
    float nearestDistance = std::numeric_limits<float>::max();
    for (ObjectGuid const& guid : shadowfiends)
    {
        Unit* shadowfiend = botAI->GetUnit(guid);
        if (!shadowfiend || !shadowfiend->IsAlive())
            continue;

        float const distance = unit.GetExactDist2d(shadowfiend);
        if (distance < nearestDistance)
        {
            nearestDistance = distance;
            nearest = shadowfiend;
        }
    }

    return nearest;
}

bool IsNearNewParasiticShadowfiend(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    if (HasParasiticShadowfiend(bot))
        return false;

    auto const& shadowfiends = botAI->GetAiObjectContext()
        ->GetValue<GuidVector>("illidan stormrage parasitic shadowfiends")->RefGet();
    for (ObjectGuid const& guid : shadowfiends)
    {
        Unit* shadowfiend = botAI->GetUnit(guid);
        if (shadowfiend && shadowfiend->IsAlive() &&
            bot->GetExactDist2d(shadowfiend) <= ILLIDAN_SHADOWFIEND_BIRTHER_DISTANCE &&
            shadowfiend->GetHealthPct() >= ILLIDAN_SHADOWFIEND_FINISH_HEALTH_PCT)
        {
            return true;
        }
    }

    return false;
}

// Living group members with either Shadowfiend debuff, humans included. Thus, trap hunters will
// trap for humans also--make sure to follow the hunter to the active infection position!
GuidVector FindPlayersWithParasiticShadowfiendGuids(Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group)
        return {};

    GuidVector infected;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->GetMapId() == BT_MAP_ID && member->IsAlive() &&
            HasParasiticShadowfiend(member))
        {
            infected.push_back(member->GetGUID());
        }
    }

    return infected;
}

// Each Shadow Demon's only target is the player it has paralyzed.
std::vector<ShadowDemon> FindShadowDemons(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    std::list<Creature*> creatures;
    bot->GetCreatureListWithEntryInGrid(
        creatures, Id(BtNpcs::NPC_SHADOW_DEMON), ILLIDAN_ADD_SEARCH_RADIUS);

    std::vector<ShadowDemon> shadowDemons;
    for (Creature* creature : creatures)
    {
        if (creature->IsAlive())
            shadowDemons.push_back({ creature->GetGUID(), ObjectGuid::Empty });
    }

    Group* group = bot->GetGroup();
    if (shadowDemons.empty() || !group)
        return shadowDemons;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member->GetMapId() != BT_MAP_ID || !member->IsAlive())
            continue;

        Aura const* paralyze = member->GetAura(Id(BtSpells::SPELL_PARALYZE));
        if (!paralyze)
            continue;

        for (ShadowDemon& shadowDemon : shadowDemons)
        {
            if (shadowDemon.demon == paralyze->GetCasterGUID())
                shadowDemon.target = member->GetGUID();
        }
    }

    return shadowDemons;
}

// Ranged prioritize a Shadow Demon fixated on the warlock tank, then the moving Shadow Demon that
// is closest to its target, and finally any that are idle.
Unit* GetShadowDemonForRangedToAttack(PlayerbotAI* botAI, Unit const& from)
{
    // A Demon moves 4 yd/s, so 1 yd is a quarter second of movement.
    constexpr float targetDistanceStep = 1.0f;
    constexpr uint32 walkingToWarlock = 0;
    constexpr uint32 walking = 1;
    constexpr uint32 idle = 2;

    auto const& shadowDemons = botAI->GetAiObjectContext()
        ->GetValue<std::vector<ShadowDemon>>("illidan stormrage shadow demons")->RefGet();
    ObjectGuid const warlockTank = GetIllidanWarlockTankGuid(botAI->GetBot());

    Unit* best = nullptr;
    std::tuple<uint32, uint32, float> bestKey = { std::numeric_limits<uint32>::max(), 0, 0.0f };
    for (ShadowDemon const& shadowDemon : shadowDemons)
    {
        Unit* demon = botAI->GetUnit(shadowDemon.demon);
        if (!demon || !demon->IsAlive())
            continue;

        Unit* target =
            shadowDemon.target.IsEmpty() ? nullptr : botAI->GetUnit(shadowDemon.target);
        uint32 rank = idle;
        uint32 step = 0;
        if (target && target->IsAlive())
        {
            rank = shadowDemon.target == warlockTank ? walkingToWarlock : walking;
            step = static_cast<uint32>(demon->GetExactDist2d(target) / targetDistanceStep);
        }

        std::tuple<uint32, uint32, float> const key = { rank, step, from.GetExactDist2d(demon) };
        if (key < bestKey)
        {
            bestKey = key;
            best = demon;
        }
    }

    return best;
}

Unit* GetShadowDemonMovingToWarlockTank(PlayerbotAI* botAI)
{
    ObjectGuid const warlockTank = GetIllidanWarlockTankGuid(botAI->GetBot());
    if (warlockTank.IsEmpty())
        return nullptr;

    auto const& shadowDemons = botAI->GetAiObjectContext()
        ->GetValue<std::vector<ShadowDemon>>("illidan stormrage shadow demons")->RefGet();
    for (ShadowDemon const& shadowDemon : shadowDemons)
    {
        if (shadowDemon.target != warlockTank)
            continue;

        Unit* demon = botAI->GetUnit(shadowDemon.demon);
        Unit* target = botAI->GetUnit(shadowDemon.target);
        if (demon && demon->IsAlive() && target && target->IsAlive())
            return demon;
    }

    return nullptr;
}

// Melee attack the closest Shadow Demon that is moving to a paralyzed target, but never one on the
// warlock tank (it is impossible for melee to attack it without being in Shadow Blast's radius).
Unit* GetShadowDemonForMeleeToAttack(PlayerbotAI* botAI, Unit const& from)
{
    // Demons spawn 5 yd from Illidan, but the melee safe positions are far away enough from Illidan
    // that melee will reach a Demon outside Aura of Dread's 15 yd range even if they move in to
    // attack as soon as the Demons spawn.
    ObjectGuid const warlockTank = GetIllidanWarlockTankGuid(botAI->GetBot());
    Unit* nearest = nullptr;
    std::pair<bool, float> nearestKey = { true, std::numeric_limits<float>::max() };
    auto const& shadowDemons = botAI->GetAiObjectContext()
        ->GetValue<std::vector<ShadowDemon>>("illidan stormrage shadow demons")->RefGet();
    for (ShadowDemon const& shadowDemon : shadowDemons)
    {
        Unit* demon = botAI->GetUnit(shadowDemon.demon);
        if (!demon || !demon->IsAlive() ||
            (!warlockTank.IsEmpty() && shadowDemon.target == warlockTank))
        {
            continue;
        }

        Unit* target =
            shadowDemon.target.IsEmpty() ? nullptr : botAI->GetUnit(shadowDemon.target);
        bool const isIdle = !target || !target->IsAlive();
        std::pair<bool, float> const key = { isIdle, from.GetExactDist2d(demon) };
        if (key < nearestKey)
        {
            nearestKey = key;
            nearest = demon;
        }
    }

    return nearest;
}

Unit* GetShadowDemonForPetToAttack(PlayerbotAI* botAI, Pet const& pet)
{
    uint32 const entry = pet.GetEntry();
    if (entry == NPC_IMP || entry == NPC_WATER_ELEMENTAL_TEMP ||
        entry == NPC_WATER_ELEMENTAL_PERM)
    {
        return GetShadowDemonForRangedToAttack(botAI, pet);
    }

    return GetShadowDemonForMeleeToAttack(botAI, pet);
}

bool IsTargetingShadowDemon(PlayerbotAI* botAI)
{
    Unit* target = botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Get();
    return target && target->GetEntry() == Id(BtNpcs::NPC_SHADOW_DEMON);
}

Position GetClosestPointOnSegment(
    Position const& start, Position const& end, Position const& point)
{
    float const dx = end.GetPositionX() - start.GetPositionX();
    float const dy = end.GetPositionY() - start.GetPositionY();
    float const lengthSq = dx * dx + dy * dy;
    if (lengthSq <= 0.0f)
        return start;

    float const along = std::clamp(
        ((point.GetPositionX() - start.GetPositionX()) * dx +
         (point.GetPositionY() - start.GetPositionY()) * dy) / lengthSq,
        0.0f, 1.0f);
    return Position(
        start.GetPositionX() + along * dx, start.GetPositionY() + along * dy,
        start.GetPositionZ());
}

EyeBlastDangerArea GetEyeBlastDangerArea(Player* bot)
{
    std::list<Creature*> creatureList;
    bot->GetCreatureListWithEntryInGrid(
        creatureList, Id(BtNpcs::NPC_ILLIDAN_DB_TARGET), ILLIDAN_ADD_SEARCH_RADIUS);

    Creature* eyeBlastTrigger = nullptr;
    for (Creature* creature : creatureList)
    {
        if (creature && creature->IsAlive())
        {
            eyeBlastTrigger = creature;
            break;
        }
    }

    if (!eyeBlastTrigger)
        return {};

    Position const startPos = eyeBlastTrigger->GetPosition();

    float destX, destY, destZ;
    if (!eyeBlastTrigger->GetMotionMaster()->GetDestination(destX, destY, destZ))
        return {};

    Position const endPos(destX, destY, destZ);

    if (startPos.GetExactDist2d(endPos) < ILLIDAN_MIN_SEGMENT_LENGTH)
        return {};

    return { startPos, endPos, ILLIDAN_EYE_BLAST_CLEARANCE };
}

bool IsPositionInEyeBlastDangerArea(Position const& pos, EyeBlastDangerArea const& area)
{
    if (area.start.GetExactDist2d(area.end) < ILLIDAN_MIN_SEGMENT_LENGTH)
        return false;

    Position const closest = GetClosestPointOnSegment(area.start, area.end, pos);
    return pos.GetExactDist2d(closest) < area.width;
}

bool IsPathInEyeBlastDangerArea(
    Position const& from, Position const& to, EyeBlastDangerArea const& area)
{
    constexpr float sampleSpacing = 1.0f;
    float const length = from.GetExactDist2d(to);
    int32 const samples = static_cast<int32>(length / sampleSpacing) + 1;
    for (int32 i = 0; i <= samples; ++i)
    {
        float const fraction = static_cast<float>(i) / samples;
        Position const point(
            from.GetPositionX() + (to.GetPositionX() - from.GetPositionX()) * fraction,
            from.GetPositionY() + (to.GetPositionY() - from.GetPositionY()) * fraction,
            from.GetPositionZ());
        if (IsPositionInEyeBlastDangerArea(point, area))
            return true;
    }

    return false;
}

// Shared encounter state

bool UpdateBtClassRoles(PlayerbotAI* botAI)
{
    BtInstanceState& state = BtState(botAI->GetBot()->GetInstanceId());
    return UpdateClassRoleAssignments(botAI, BT_MAP_ID, {
        { ZEREVOR_MAGE_TANK_ROLE, state.zerevorMageTank },
        { ILLIDAN_WARLOCK_TANK_ROLE, state.illidanWarlockTank },
    });
}

BtInstanceState& BtState(uint32 instanceId)
{
    std::lock_guard lock(btStateMutex);
    return btStates[instanceId];
}

BtInstanceState* FindBtState(uint32 instanceId)
{
    std::lock_guard lock(btStateMutex);
    auto it = btStates.find(instanceId);
    return it != btStates.end() ? &it->second : nullptr;
}

bool BtResetInstance(uint32 instanceId)
{
    std::lock_guard lock(btStateMutex);
    auto it = btStates.find(instanceId);
    if (it == btStates.end())
        return false;

    BtInstanceState& state = it->second;
    bool const wasSet = state.najentusSpineAssignments || state.najentusSpineThrower ||
        state.najentusSpineThrowTime || state.councilDpsWaitTimer || state.illidanLastPhase ||
        state.illidanPreviousPhase || state.illidanPhaseStartTime ||
        state.illidanFlamePhaseStartTime || state.eastFlameGuid || state.westFlameGuid ||
        state.illidanTankAggroPhase || state.illidanInfections;

    // Not erased: the class roles outlive encounters, see ClassRoleAssignment.
    BtInstanceState kept;
    kept.zerevorMageTank = state.zerevorMageTank;
    kept.illidanWarlockTank = state.illidanWarlockTank;
    state = std::move(kept);
    return wasSet;
}

}
