/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "KaraHelpers.h"
#include "CellImpl.h"
#include "DynamicObject.h"
#include "EncounterHelpers.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "Map.h"
#include "ObjectAccessor.h"
#include "PlayerbotTextMgr.h"
#include "Playerbots.h"
#include "Random.h"
#include "Spell.h"
#include "ThreatHelpers.h"
#include <algorithm>
#include <initializer_list>
#include <limits>
#include <list>
#include <map>
#include <mutex>
#include <unordered_map>
#include <utility>

using namespace EncounterHelpers;

namespace KaraHelpers
{

// Shared

namespace
{

std::mutex karaStateMutex;
std::unordered_map<uint32, KaraInstanceState> karaStates;
// Per instance: Mana Warp -> War Stomp claim time. Kept out of KaraInstanceState, which the reset
// erases during trash; claims expire on their own.
std::unordered_map<uint32, std::unordered_map<ObjectGuid, uint32>> manaWarpWarStompClaims;

float GetDistanceToSegment2d(Position const& point, Position const& start, Position const& end)
{
    float const segmentX = end.GetPositionX() - start.GetPositionX();
    float const segmentY = end.GetPositionY() - start.GetPositionY();
    float const lengthSquared = segmentX * segmentX + segmentY * segmentY;
    if (lengthSquared <= 0.0f)
        return point.GetExactDist2d(start);

    float const along =
        ((point.GetPositionX() - start.GetPositionX()) * segmentX +
         (point.GetPositionY() - start.GetPositionY()) * segmentY) / lengthSquared;
    float const fraction = std::clamp(along, 0.0f, 1.0f);
    return point.GetExactDist2d(
        start.GetPositionX() + segmentX * fraction, start.GetPositionY() + segmentY * fraction);
}

// The Opera Event consists of three plays with several actors each, and the Chess Event has no boss
// on a threat list, so neither has a name.
constexpr std::string_view BossName(KaraBosses boss)
{
    switch (boss)
    {
        case KaraBosses::DATA_ATTUMEN:
            return "attumen the huntsman";
        case KaraBosses::DATA_MOROES:
            return "moroes";
        case KaraBosses::DATA_MAIDEN:
            return "maiden of virtue";
        case KaraBosses::DATA_CURATOR:
            return "the curator";
        case KaraBosses::DATA_TERESTIAN:
            return "terestian illhoof";
        case KaraBosses::DATA_ARAN:
            return "shade of aran";
        case KaraBosses::DATA_NETHERSPITE:
            return "netherspite";
        case KaraBosses::DATA_MALCHEZAAR:
            return "prince malchezaar";
        case KaraBosses::DATA_NIGHTBANE:
            return "nightbane";
        default:
            return {};
    }
}

} // end anonymous namespace (Shared)

Unit* GetKaraBoss(Player* bot, KaraBosses boss)
{
    if (!IsBossInProgress(bot, KARA_MAP_ID, Id(boss)))
        return nullptr;

    return FindThreatTarget(bot, BossName(boss));
}

Unit* GetOperaActor(Player* bot, std::string_view name)
{
    if (!IsBossInProgress(bot, KARA_MAP_ID, Id(KaraBosses::DATA_OPERA_PERFORMANCE)))
        return nullptr;

    Unit* actor = FindThreatTarget(bot, name);
    return actor && !actor->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE) ? actor : nullptr;
}

bool IsKarazhanFearCasterActive(Player* bot)
{
    Unit* nightbane = GetKaraBoss(bot, KaraBosses::DATA_NIGHTBANE);
    if (nightbane && !IsNightbaneFlying(nightbane))
        return true;

    if (FindThreatTarget(bot, "spectral charger"))
        return true;

    return GetOperaActor(bot, "the big bad wolf") || GetOperaActor(bot, "dorothee") ||
        GetOperaActor(bot, "roar");
}

// Trash

namespace
{

bool NeedsComboPointsForManaWarpStun(Player* bot)
{
    return bot->getClass() == CLASS_ROGUE ||
        (bot->getClass() == CLASS_DRUID && bot->GetShapeshiftForm() == FORM_CAT);
}

bool IsManaWarpClaimedForWarStomp(Unit* warp)
{
    std::lock_guard lock(karaStateMutex);
    auto const instanceIt = manaWarpWarStompClaims.find(warp->GetInstanceId());
    if (instanceIt == manaWarpWarStompClaims.end())
        return false;

    auto const claimIt = instanceIt->second.find(warp->GetGUID());
    return claimIt != instanceIt->second.end() &&
        getMSTimeDiff(claimIt->second, getMSTime()) < WAR_STOMP_CLAIM_MS;
}

// First, attempt to stun between 10% and 15% HP because if the mob is stunned at 10%, it will not
// cast Warp Breach. If that fails, then there is a second opportunity to stun while it is casting.
bool IsManaWarpToStun(Player* bot, Unit* unit, float reach)
{
    if (!unit || unit->GetEntry() != Id(KaraNpcs::NPC_MANA_WARP) || !unit->IsAlive() ||
        bot->GetExactDist(unit) > reach || unit->HasUnitState(UNIT_STATE_STUNNED))
    {
        return false;
    }

    float const healthPct = unit->GetHealthPct();
    bool const isDue = unit->FindCurrentSpellBySpellId(Id(KaraSpells::SPELL_WARP_BREACH)) ||
        (healthPct > MANA_WARP_WARP_BREACH_HEALTH_PCT && healthPct <= MANA_WARP_STUN_HEALTH_PCT);

    return isDue && !IsManaWarpClaimedForWarStomp(unit);
}

} // end anonymous namespace (Trash)

bool IsNearManaWarpPacks(Player* bot)
{
    return bot->GetExactDist(MANA_WARP_PACKS_CENTER) <= MANA_WARP_PACKS_RADIUS;
}

float GetManaWarpStunReach(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    // A stun counts only if the bot knows it (some are talents) and it is off cooldown, so bots
    // never stand by with a stun they cannot cast.
    auto const isReady = [botAI, bot](char const* spell)
    {
        uint32 const spellId =
            botAI->GetAiObjectContext()->GetValue<uint32>("spell id", spell)->Get();
        return spellId && !bot->HasSpellCooldown(spellId);
    };

    float reach = 0.0f;
    switch (bot->getClass())
    {
        case CLASS_DRUID:
        {
            ShapeshiftForm const form = bot->GetShapeshiftForm();
            if (((form == FORM_BEAR || form == FORM_DIREBEAR) && isReady("bash")) ||
                (form == FORM_CAT && isReady("maim")))
            {
                reach = MELEE_STUN_REACH;
            }
            break;
        }
        case CLASS_PALADIN:
        {
            if (isReady("hammer of justice"))
                reach = HAMMER_OF_JUSTICE_REACH;
            break;
        }
        case CLASS_ROGUE:
        {
            if (isReady("kidney shot"))
                reach = MELEE_STUN_REACH;
            break;
        }
        case CLASS_WARRIOR:
        {
            if (isReady("concussion blow") || isReady("shockwave"))
                reach = MELEE_STUN_REACH;
            break;
        }
        case CLASS_WARLOCK:
        {
            if (isReady("shadowfury"))
                reach = SHADOWFURY_REACH;
            break;
        }
        default:
            break;
    }

    if (bot->getRace() == RACE_TAUREN && isReady("war stomp"))
        reach = std::max(reach, WAR_STOMP_RADIUS);

    return reach;
}

// Picks a Mana Warp for this bot to stun: its current target, else any other warp in reach. A warp
// qualifies at 10-15% health or while casting Warp Breach, unless it is stunned or a War Stomp on
// it is still casting. Rogues and cat druids stun only their current target, with combo points.
Unit* GetManaWarpToStun(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    if (!IsNearManaWarpPacks(bot))
        return nullptr;

    float const reach = GetManaWarpStunReach(botAI);
    if (reach <= 0.0f)
        return nullptr;

    AiObjectContext* context = botAI->GetAiObjectContext();
    Unit* target = context->GetValue<Unit*>("current target")->Get();
    bool const needsComboPoints = NeedsComboPointsForManaWarpStun(bot);
    if (IsManaWarpToStun(bot, target, reach) &&
        (!needsComboPoints || bot->GetComboPoints(target)))
    {
        return target;
    }

    if (!needsComboPoints)
    {
        for (auto const& attackers : context->GetValue<GuidVector>("attackers")->RefGet())
        {
            Unit* attacker = botAI->GetUnit(attackers);
            if (IsManaWarpToStun(bot, attacker, reach))
                return attacker;
        }
    }

    return nullptr;
}

// Expired claims, in every instance, are purged here.
void ClaimManaWarpForWarStomp(Unit* warp)
{
    uint32 const now = getMSTime();
    std::lock_guard lock(karaStateMutex);
    std::erase_if(manaWarpWarStompClaims, [now](auto& instanceClaims)
    {
        std::erase_if(instanceClaims.second, [now](auto const& claim)
        {
            return getMSTimeDiff(claim.second, now) >= WAR_STOMP_CLAIM_MS;
        });
        return instanceClaims.second.empty();
    });

    manaWarpWarStompClaims[warp->GetInstanceId()][warp->GetGUID()] = now;
}

Unit* GetManaWarpToStunNext(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    if (!IsNearManaWarpPacks(bot) || PlayerbotAI::IsTank(bot) || PlayerbotAI::IsHeal(bot) ||
        GetManaWarpStunReach(botAI) <= 0.0f)
    {
        return nullptr;
    }

    Unit* target = botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Get();
    if (!target || target->GetEntry() != Id(KaraNpcs::NPC_MANA_WARP) || !target->IsAlive() ||
        target->GetHealthPct() > MANA_WARP_STAND_BY_HEALTH_PCT ||
        target->HasUnitState(UNIT_STATE_STUNNED))
    {
        return nullptr;
    }

    return !NeedsComboPointsForManaWarpStun(bot) || bot->GetComboPoints(target) ? target : nullptr;
}

// Attumen the Huntsman

// Midnight remains present but turns invisible when mounted Attumen is summoned. She stays on
// threat lists, and thus FindThreatTarget for Midnight will capture the entire encounter.
bool IsAttumenMounted(Unit* midnight)
{
    return midnight && !midnight->IsVisible();
}

// Both Attumens share a name, so the lookup can find either. This returns null unless it finds the
// mounted one, which can fail for up to 1s after mounting, until the unmounted one despawns.
Unit* GetMountedAttumen(Player* bot)
{
    Unit* attumen = GetKaraBoss(bot, KaraBosses::DATA_ATTUMEN);
    if (!attumen || attumen->GetEntry() != Id(KaraNpcs::NPC_ATTUMEN_THE_HUNTSMAN_MOUNTED))
        return nullptr;

    return attumen;
}

std::optional<Position> GetAttumenStackPosition(Player* bot, Unit* attumen)
{
    if (!attumen)
        return std::nullopt;

    float const distanceBehind = bot->getClass() == CLASS_HUNTER ?
        ATTUMEN_HUNTER_STACK_DISTANCE : ATTUMEN_STACK_DISTANCE;
    float const orientation = attumen->GetOrientation() + M_PI;
    Position const position(
        attumen->GetPositionX() + std::cos(orientation) * distanceBehind,
        attumen->GetPositionY() + std::sin(orientation) * distanceBehind,
        attumen->GetPositionZ());

    if (bot->GetExactDist2d(position) <= ATTUMEN_STACK_TOLERANCE)
        return std::nullopt;

    return position;
}

bool IsAttumenDpsHeld(Player* bot)
{
    Unit* midnight = FindThreatTarget(bot, "midnight");
    if (!midnight || !IsAttumenMounted(midnight))
        return false;

    KaraInstanceState const* state = FindKaraState(bot->GetInstanceId());
    if (state && state->attumenDpsHoldTimer &&
        getMSTimeDiff(*state->attumenDpsHoldTimer, getMSTime()) >= ATTUMEN_MOUNTED_DPS_HOLD_MS)
    {
        return false;
    }

    return !(PlayerbotAI::IsTank(bot) && PlayerbotAI::IsMainTank(bot));
}

// Moroes

GuidVector FindMoroesGuestGuids(Player* bot)
{
    return FindCreatureGuidsAround(
        bot, MOROES_SPAWN_POSITION, MOROES_GUEST_SEARCH_RADIUS, MOROES_GUESTS_IN_KILL_ORDER);
}

// Maiden of Virtue

Player* GetNearestRepentantHealer(Player* bot, Unit* maiden)
{
    if (!maiden)
        return nullptr;

    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    Player* nearest = nullptr;
    float nearestDist = std::numeric_limits<float>::max();
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() ||
            !member->HasAura(Id(KaraSpells::SPELL_REPENTANCE)) || !PlayerbotAI::IsHeal(member))
        {
            continue;
        }

        float const dist = maiden->GetExactDist2d(member);
        if (dist < nearestDist)
        {
            nearest = member;
            nearestDist = dist;
        }
    }

    return nearest;
}

// The Big Bad Wolf

namespace
{

void StepToward(Position& from, Position const& to, float distance)
{
    float const remaining = from.GetExactDist2d(to);
    if (remaining <= distance)
    {
        from.Relocate(to.GetPositionX(), to.GetPositionY());
        return;
    }

    float const ratio = distance / remaining;
    from.Relocate(
        from.GetPositionX() + (to.GetPositionX() - from.GetPositionX()) * ratio,
        from.GetPositionY() + (to.GetPositionY() - from.GetPositionY()) * ratio);
}

float GetSmallestBigBadWolfRunGap(
    Position runner, Position wolf, BigBadWolfRunPlan plan, float runnerStep, float wolfStep,
    float wolfStopDistance, uint32 steps)
{
    float smallestGap = std::numeric_limits<float>::max();
    for (uint32 step = 0; step < steps; ++step)
    {
        if (runner.GetExactDist2d(BIG_BAD_WOLF_RUN_POSITIONS[plan.index]) <
            BIG_BAD_WOLF_RUN_TURN_DISTANCE)
        {
            plan.index = GetNextBigBadWolfRunIndex(plan.index, plan.direction);
        }

        StepToward(runner, BIG_BAD_WOLF_RUN_POSITIONS[plan.index], runnerStep);
        if (wolf.GetExactDist2d(runner) > wolfStopDistance)
            StepToward(wolf, runner, wolfStep);

        smallestGap = std::min(smallestGap, wolf.GetExactDist2d(runner));
    }

    return smallestGap;
}

} // end anonymous namespace (The Big Bad Wolf)

bool IsLittleRedRidingHood(Player* bot)
{
    return bot->HasAura(Id(KaraSpells::SPELL_LITTLE_RED_RIDING_HOOD));
}

uint8 GetNextBigBadWolfRunIndex(uint8 index, int32 direction)
{
    int32 const count = static_cast<int32>(BIG_BAD_WOLF_RUN_POSITIONS.size());
    return static_cast<uint8>((index + direction + count) % count);
}

// BBW runs straight at the bot, cutting inside any route, so a poor start runs into him. This
// forecasts the chase for every point and direction and returns the one that keeps BBW furthest
// away from the bot at his closest approach.
BigBadWolfRunPlan GetBigBadWolfRunPlan(Player* bot, Unit& wolf, float seconds)
{
    constexpr float forecastStepSeconds = 0.1f;
    uint32 const steps = static_cast<uint32>(seconds / forecastStepSeconds);
    float const runnerStep = bot->GetSpeed(MOVE_RUN) * forecastStepSeconds;
    float const wolfStep = wolf.GetSpeed(MOVE_RUN) * forecastStepSeconds;
    float const wolfStopDistance =
        CONTACT_DISTANCE + wolf.GetCombatReach() + bot->GetCombatReach();

    BigBadWolfRunPlan best;
    float bestGap = -1.0f;
    for (uint8 index = 0; index < BIG_BAD_WOLF_RUN_POSITIONS.size(); ++index)
    {
        for (int32 const direction : { 1, -1 })
        {
            BigBadWolfRunPlan const plan{ index, direction };
            float const gap = GetSmallestBigBadWolfRunGap(
                bot->GetPosition(), wolf.GetPosition(), plan, runnerStep, wolfStep,
                wolfStopDistance, steps);
            if (gap > bestGap)
            {
                best = plan;
                bestGap = gap;
            }
        }
    }

    return best;
}

Player* GetBigBadWolfRedHood(Unit* wolf)
{
    if (!wolf)
        return nullptr;

    Unit* victim = wolf->GetVictim();
    Player* target = victim ? victim->ToPlayer() : nullptr;
    if (!target || !IsLittleRedRidingHood(target) ||
        target->IsImmunedToDamage(SPELL_SCHOOL_MASK_NORMAL))
    {
        return nullptr;
    }

    return target;
}

// Human non-tank Hoods only, near the end of Hood or below half health. Bots get the +70% rocket
// boots buff, and tanks have enough HP to survive, provided they are quickly healed.
Player* GetBigBadWolfTargetToProtect(Unit* wolf)
{
    if (!wolf)
        return nullptr;

    Player* target = GetBigBadWolfRedHood(wolf);
    if (!target)
        return nullptr;

    Aura const* hood = target->GetAura(Id(KaraSpells::SPELL_LITTLE_RED_RIDING_HOOD));
    if (!hood || hood->GetMaxDuration() - hood->GetDuration() < BIG_BAD_WOLF_PROTECT_REACTION_MS)
        return nullptr;

    if (hood->GetDuration() > HAND_OF_PROTECTION_DURATION_MS &&
        target->GetHealthPct() >= BIG_BAD_WOLF_PROTECT_HEALTH_PCT)
    {
        return nullptr;
    }

    if (!wolf->IsWithinMeleeRange(target) || GET_PLAYERBOT_AI(target) ||
        PlayerbotAI::IsTank(target))
    {
        return nullptr;
    }

    return target;
}

// Terestian Illhoof

// Prerequisite to searching for Demon Chains, which exist only while a group member is sacrificed.
bool IsTerestianSacrificeActive(Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->HasAura(Id(KaraSpells::SPELL_SACRIFICE)))
            return true;
    }

    return false;
}

// Shade of Aran

bool IsAranCastingArcaneExplosion(Unit* aran)
{
    return aran && aran->FindCurrentSpellBySpellId(Id(KaraSpells::SPELL_ARCANE_EXPLOSION));
}

std::optional<Position> GetBlizzardEscapePosition(Player* bot)
{
    std::list<WorldObject*> objects;
    Acore::AllWorldObjectsInRange check(bot, ARAN_ROOM_SEARCH_RADIUS);
    Acore::WorldObjectListSearcher<Acore::AllWorldObjectsInRange> searcher(
        bot, objects, check, GRID_MAP_TYPE_MASK_DYNAMICOBJECT);
    Cell::VisitObjects(bot, searcher, ARAN_ROOM_SEARCH_RADIUS);

    DynamicObject const* oldest = nullptr;
    for (WorldObject* object : objects)
    {
        DynamicObject const* patch = object->ToDynObject();
        if (patch && patch->GetSpellId() == Id(KaraSpells::SPELL_BLIZZARD) &&
            (!oldest || patch->GetDuration() < oldest->GetDuration()))
        {
            oldest = patch;
        }
    }

    if (!oldest)
        return std::nullopt;

    float const angle = ARAN_ROOM_CENTER.GetAngle(oldest) + ARAN_BLIZZARD_ESCAPE_ANGLE;
    return Position(
        ARAN_ROOM_CENTER.GetPositionX() + ARAN_BLIZZARD_ESCAPE_DISTANCE * std::cos(angle),
        ARAN_ROOM_CENTER.GetPositionY() + ARAN_BLIZZARD_ESCAPE_DISTANCE * std::sin(angle),
        bot->GetPositionZ());
}

bool IsFlameWreathInProgress(PlayerbotAI* botAI, Unit* aran)
{
    if (botAI->GetBot()->HasAura(Id(KaraSpells::SPELL_FLAME_WREATH_AURA)))
        return true;

    // A resurrected bot can't find Aran until zone combat refreshes, but it can see the rings.
    if (!aran)
    {
        return !botAI->GetAiObjectContext()
            ->GetValue<std::vector<Position>>("shade of aran flame wreath rings")->RefGet().empty();
    }

    return aran->FindCurrentSpellBySpellId(Id(KaraSpells::SPELL_FLAME_WREATH_CAST)) ||
        aran->GetDynObject(Id(KaraSpells::SPELL_FLAME_WREATH_AURA));
}

// The only movement allowed during Flame Wreath is a stuck bot moving to its target (for a healer,
// the party member to heal), and only if the movement does not come close to crossing any ring.
bool CanReachTargetDuringFlameWreath(PlayerbotAI* botAI, Unit* aran)
{
    Player* bot = botAI->GetBot();
    if (!aran || bot->HasAura(Id(KaraSpells::SPELL_FLAME_WREATH_AURA)))
        return false;

    AiObjectContext* context = botAI->GetAiObjectContext();
    Unit* target = context->GetValue<Unit*>(
        PlayerbotAI::IsHeal(bot) ? "party member to heal" : "current target")->Get();
    if (!target)
        return false;

    if (Spell* cast = aran->FindCurrentSpellBySpellId(Id(KaraSpells::SPELL_FLAME_WREATH_CAST)))
        return cast->GetCastTimeRemaining() > ARAN_FLAME_WREATH_STOP_LEAD_MS;

    DynamicObject* ring = aran->GetDynObject(Id(KaraSpells::SPELL_FLAME_WREATH_AURA));
    if (!ring)
        return true;

    if (ring->GetDuration() > ARAN_FLAME_WREATH_RING_DURATION_MS - ARAN_FLAME_WREATH_SETTLE_MS)
        return false;

    std::vector<Position> const& rings =
        context->GetValue<std::vector<Position>>("shade of aran flame wreath rings")->RefGet();
    if (rings.empty())
        return false;

    for (Position const& position : rings)
    {
        if (GetDistanceToSegment2d(position, *bot, *target) <= ARAN_FLAME_WREATH_REACH_CLEARANCE)
            return false;
    }

    return true;
}

// There is no raid action to banish the Elementals. This condition exists in case a player does it.
GuidVector FindUnbanishedConjuredElementals(Player* bot)
{
    std::list<Creature*> elementals;
    bot->GetCreatureListWithEntryInGrid(
        elementals, Id(KaraNpcs::NPC_CONJURED_ELEMENTAL), ARAN_ROOM_SEARCH_RADIUS);

    GuidVector guids;
    for (Creature* elemental : elementals)
    {
        if (elemental && elemental->IsAlive() &&
            !elemental->HasAura(Id(KaraSpells::SPELL_BANISH_RANK_1)) &&
            !elemental->HasAura(Id(KaraSpells::SPELL_BANISH_RANK_2)))
        {
            guids.push_back(elemental->GetGUID());
        }
    }

    return guids;
}

// Netherspite

namespace
{

struct NetherspiteArc
{
    float start;
    float width;
};

struct NetherspiteRadiusBand
{
    float low;
    float high;
};

float GetBeamAngle(Unit& netherspite, KaraInstanceState const& state, NetherspiteBeam beam)
{
    Position const& portal = *state.netherspiteBeams[Id(beam)].portal;
    return netherspite.GetAngle(portal.GetPositionX(), portal.GetPositionY());
}

// The arc between the blue and green beams.
NetherspiteArc GetSideWithoutRed(Unit& netherspite, KaraInstanceState const& state)
{
    float const red = GetBeamAngle(netherspite, state, NetherspiteBeam::Red);
    float const green = GetBeamAngle(netherspite, state, NetherspiteBeam::Green);
    float const blue = GetBeamAngle(netherspite, state, NetherspiteBeam::Blue);

    float const greenToBlue = Position::NormalizeOrientation(blue - green);
    if (Position::NormalizeOrientation(red - green) < greenToBlue)
        return { blue, Position::NormalizeOrientation(green - blue) };

    return { green, greenToBlue };
}

// The arc between the blue and green beams, or its central 170° if it's wider. Under 180°, a
// straight move from inside it to the next blocker's position can't cross a beam.
NetherspiteArc GetNetherspiteStandingArc(Unit& netherspite, KaraInstanceState const& state)
{
    NetherspiteArc const side = GetSideWithoutRed(netherspite, state);
    float const width = std::min(side.width, NETHERSPITE_MAX_STANDING_ARC);
    return { side.start + (side.width - width) / 2.0f, width };
}

// How far from Netherspite's center bots stand between the blue and green beams. Melee stay inside
// his melee range (20.8 yd). Hunters stay outside it, or Auto Shot fails.
NetherspiteRadiusBand GetNetherspiteRadiusBand(Player* bot)
{
    if (PlayerbotAI::IsHeal(bot))
        return { 14.0f, 25.0f };
    if (bot->getClass() == CLASS_HUNTER)
        return { 25.0f, 32.0f };
    if (PlayerbotAI::IsMelee(bot))
        return { 14.0f, 20.0f };
    return { 18.0f, 25.0f };
}

bool CrossesBeam(Player* bot, Unit& netherspite, KaraInstanceState const& state, float x, float y)
{
    for (NetherspiteBeamState const& beam : state.netherspiteBeams)
    {
        if (!beam.portal)
            continue;

        NetherspiteBeamOffset const from = GetNetherspiteBeamOffset(
            netherspite, *beam.portal, bot->GetPositionX(), bot->GetPositionY());
        NetherspiteBeamOffset const to = GetNetherspiteBeamOffset(netherspite, *beam.portal, x, y);
        // A bot leaving a beam can move off of it toward either side.
        if (std::abs(from.off) < NETHERSPITE_BEAM_WIDTH || from.off * to.off >= 0.0f)
            continue;

        float const along =
            from.along + (to.along - from.along) * from.off / (from.off - to.off);
        if (along > 0.0f && along < netherspite.GetExactDist2d(*beam.portal))
            return true;
    }

    return false;
}

// True if a straight move to the position crosses a beam that will be up by the time the bot
// arrives. Beams start 10s into a portal phase so a move that ends before 10s never touches a beam.
bool CrossesBeamAfterStart(
    Player* bot, Unit& netherspite, KaraInstanceState const& state, float x, float y)
{
    uint32 const elapsed = state.netherspitePortalPhaseTimer ?
        getMSTimeDiff(*state.netherspitePortalPhaseTimer, getMSTime()) : NETHERSPITE_BEAMS_START_MS;
    float const travelMs = bot->GetExactDist2d(x, y) / NETHERSPITE_BOT_SPEED * IN_MILLISECONDS;
    if (elapsed + travelMs < NETHERSPITE_BEAMS_START_MS)
        return false;

    return CrossesBeam(bot, netherspite, state, x, y);
}

// The void zone on the path that is nearest the bot, so a step goes around that one first.
Position const* FindFirstVoidZoneOnPath(
    KaraInstanceState const& state, Position const& from, Position const& to)
{
    Position const* first = nullptr;
    float firstDistance = std::numeric_limits<float>::max();
    for (Position const& zone : state.netherspiteVoidZones)
    {
        float const fromZone = zone.GetExactDist2d(from);
        if (fromZone < NETHERSPITE_VZ_HAZARD_RADIUS || fromZone >= firstDistance)
            continue;

        float const clearance = fromZone < NETHERSPITE_VZ_CLEARANCE ?
            NETHERSPITE_VZ_HAZARD_RADIUS : NETHERSPITE_VZ_CLEARANCE;
        if (GetDistanceToSegment2d(zone, from, to) < clearance)
        {
            first = &zone;
            firstDistance = fromZone;
        }
    }

    return first;
}

// False if the floor ends 1 yd or more short of the point, on a straight line from the bot.
bool GetFloorAt(Player* bot, float x, float y, float& outZ)
{
    float clippedX = x;
    float clippedY = y;
    outZ = bot->GetPositionZ();
    if (!bot->GetMap()->CheckCollisionAndGetValidCoords(
            bot, bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(), clippedX, clippedY,
            outZ, false))
    {
        return false;
    }

    return std::abs(clippedX - x) < 1.0f && std::abs(clippedY - y) < 1.0f;
}

} // end anonymous namespace (Netherspite)

bool IsBanishPhase(Unit* netherspite)
{
    return netherspite && netherspite->HasAura(Id(KaraSpells::SPELL_NETHERSPITE_BANISHED));
}

// A druid's power type changes to reflect its form, so a cat shifting out to offheal would count as
// a mana user, when it should be treated as a non-mana user prioritized for the green beam. To
// avoid this, a role check is used to differentiate druids.
bool IsManaUser(Player* player)
{
    if (player->getClass() == CLASS_DRUID)
        return PlayerbotAI::IsRanged(player);

    return player->getPowerType() == POWER_MANA;
}

// Threat resets at the beginning of each portal phase.
bool IsNetherspiteDpsHeld(Player* bot, Unit* netherspite)
{
    if (!netherspite)
        return false;

    if (PlayerbotAI::IsTank(bot) ||
        netherspite->HasAura(Id(KaraSpells::SPELL_NETHERSPITE_BANISHED)))
    {
        return false;
    }

    KaraInstanceState const* state = FindKaraState(netherspite->GetInstanceId());
    if (!state || !state->netherspitePortalPhaseTimer)
        return true;

    uint32 const elapsed = getMSTimeDiff(*state->netherspitePortalPhaseTimer, getMSTime());
    if (elapsed < NETHERSPITE_DPS_HOLD_MS)
        return true;
    if (elapsed >= NETHERSPITE_BEAMS_START_MS)
        return false;

    Unit* victim = netherspite->GetVictim();
    Player* player = victim ? victim->ToPlayer() : nullptr;
    return !player || !PlayerbotAI::IsTank(player);
}

bool HasNetherspiteBeamStackLimit(Player* player, NetherspiteBeam beam)
{
    if (beam == NetherspiteBeam::Red)
        return false;

    // Non-mana users on the green beam have no stack limit because the debuff has no impact on
    // them (it reduces a blocker's maximum mana).
    return beam == NetherspiteBeam::Blue || IsManaUser(player);
}

uint8 GetNetherspiteBeamStacks(Player* player, NetherspiteBeamData const& data)
{
    Aura const* buff = player->GetAura(Id(data.buff));
    return buff ? buff->GetStackAmount() : 0;
}

bool AreNetherspitePortalsKnown(KaraInstanceState const& state)
{
    return std::all_of(
        state.netherspiteBeams.begin(), state.netherspiteBeams.end(),
        [](NetherspiteBeamState const& beam) { return beam.portal.has_value(); });
}

bool IsNetherspiteBlockerOrNext(KaraInstanceState const& state, ObjectGuid guid)
{
    return std::any_of(
        state.netherspiteBeams.begin(), state.netherspiteBeams.end(),
        [guid](NetherspiteBeamState const& beam)
        {
            return beam.blocker == guid || beam.next == guid;
        });
}

Position const* FindNetherspiteBlockerPosition(KaraInstanceState const& state, ObjectGuid guid)
{
    for (NetherspiteBeamState const& beam : state.netherspiteBeams)
    {
        if (beam.blocker == guid)
            return &beam.blockerPosition;
    }

    return nullptr;
}

Position const* FindNetherspiteNextPosition(KaraInstanceState const& state, ObjectGuid guid)
{
    for (NetherspiteBeamState const& beam : state.netherspiteBeams)
    {
        if (beam.next == guid)
            return &beam.nextPosition;
    }

    return nullptr;
}

NetherspiteBanishSlot const* FindNetherspiteBanishSlot(
    KaraInstanceState const& state, ObjectGuid guid)
{
    for (NetherspiteBanishSlot const& slot : state.netherspiteBanishSlots)
    {
        if (slot.guid == guid)
            return &slot;
    }

    return nullptr;
}

void YellNetherspiteBeamText(Player* player, std::string const& key, std::string const& text)
{
    std::map<std::string, std::string> placeholders{{"%player", player->GetName()}};
    player->Yell(
        PlayerbotTextMgr::instance().GetBotTextOrDefault(key, text, placeholders), LANG_UNIVERSAL);
}

void YellNetherspiteBeamHandover(Player* leaving, Player* blocking, NetherspiteBeam beam)
{
    if (leaving && leaving->IsAlive())
    {
        if (beam == NetherspiteBeam::Green)
        {
            YellNetherspiteBeamText(
                leaving, "netherspite_beam_leaving_green",
                "%player is leaving the green beam. Next blocker up!");
        }
        else if (beam == NetherspiteBeam::Blue)
        {
            YellNetherspiteBeamText(
                leaving, "netherspite_beam_leaving_blue",
                "%player is leaving the blue beam. Next blocker up!");
        }
    }

    if (!blocking)
        return;

    switch (beam)
    {
        case NetherspiteBeam::Red:
            YellNetherspiteBeamText(
                blocking, "netherspite_beam_blocking_red",
                "%player is moving to block the red beam!");
            break;
        case NetherspiteBeam::Green:
            YellNetherspiteBeamText(
                blocking, "netherspite_beam_blocking_green",
                "%player is moving to block the green beam!");
            break;
        case NetherspiteBeam::Blue:
            YellNetherspiteBeamText(
                blocking, "netherspite_beam_blocking_blue",
                "%player is moving to block the blue beam!");
            break;
        default:
            break;
    }
}

// The point's position relative to the beam. along: distance from Netherspite's center toward the
// portal (negative behind him). off: distance to the side of the line, positive to his left as he
// faces the portal.
NetherspiteBeamOffset GetNetherspiteBeamOffset(
    Unit& netherspite, Position const& portal, float x, float y)
{
    float const length = netherspite.GetExactDist2d(portal);
    if (length == 0.0f)
        return { 0.0f, 0.0f };

    float const dirX = (portal.GetPositionX() - netherspite.GetPositionX()) / length;
    float const dirY = (portal.GetPositionY() - netherspite.GetPositionY()) / length;
    float const dx = x - netherspite.GetPositionX();
    float const dy = y - netherspite.GetPositionY();
    return { dx * dirX + dy * dirY, dy * dirX - dx * dirY };
}

bool IsOnNetherspiteBeam(Unit* netherspite, Position const& portal, Unit* unit)
{
    if (!netherspite || !unit)
        return false;

    NetherspiteBeamOffset const offset = GetNetherspiteBeamOffset(
        *netherspite, portal, unit->GetPositionX(), unit->GetPositionY());
    return offset.along > NETHERSPITE_BEAM_START &&
        offset.along < netherspite->GetExactDist2d(portal) &&
        std::abs(offset.off) < NETHERSPITE_BEAM_WIDTH;
}

Position GetPositionOnNetherspiteBeam(Unit& netherspite, Position const& portal, float distance)
{
    float const length = netherspite.GetExactDist2d(portal);
    return Position(
        netherspite.GetPositionX() +
            (portal.GetPositionX() - netherspite.GetPositionX()) / length * distance,
        netherspite.GetPositionY() +
            (portal.GetPositionY() - netherspite.GetPositionY()) / length * distance,
        netherspite.GetPositionZ());
}

// At least 3 yd off every beam along its whole length. Netherspite's AC script does not allow
// players to interact with beams that are closer than 16.5 yd from him, but I believe this is a bug
// (the beam visual continues to his center, and I think it should be blockable for the whole
// length). The strategy has bots avoid the beam all the way to his center so it should continue to
// work even if the AC script is fixed.
bool IsClearOfNetherspiteBeams(Unit& netherspite, KaraInstanceState const& state, float x, float y)
{
    for (NetherspiteBeamState const& beam : state.netherspiteBeams)
    {
        if (!beam.portal)
            continue;

        NetherspiteBeamOffset const offset =
            GetNetherspiteBeamOffset(netherspite, *beam.portal, x, y);
        if (offset.along > 0.0f && offset.along < netherspite.GetExactDist2d(*beam.portal) &&
            std::abs(offset.off) < NETHERSPITE_BEAM_CLEARANCE)
        {
            return false;
        }
    }

    return true;
}

bool IsClearOfVoidZones(KaraInstanceState const& state, float x, float y, float clearance)
{
    return std::none_of(
        state.netherspiteVoidZones.begin(), state.netherspiteVoidZones.end(),
        [x, y, clearance](Position const& zone) { return zone.GetExactDist2d(x, y) < clearance; });
}

// True if the straight line passes within 3 yd of a void zone. The actual hazard radius is 2 yd,
// and a bot already between 2 and 3 yd of a void zone may pass it but not enter it.
bool CrossesVoidZone(KaraInstanceState const& state, Position const& from, Position const& to)
{
    return FindFirstVoidZoneOnPath(state, from, to);
}

// How far from Netherspite's center the blocker stands on the line: the point closest to 18 yd
// (hunters 25) that is 2.5 yd clear of void zones. If every point searched is in a zone, it's 18
// (25) anyway (possible only when the portal is too close to Netherspite).
float FindNetherspiteBeamPositionDistance(
    Unit& netherspite, Position const& portal, KaraInstanceState const& state, Player* blocker)
{
    bool const isHunter = blocker->getClass() == CLASS_HUNTER;
    float const preferred =
        isHunter ? NETHERSPITE_HUNTER_BLOCKER_DISTANCE : NETHERSPITE_BLOCKER_DISTANCE;
    float const low =
        isHunter ? NETHERSPITE_HUNTER_BLOCKER_DISTANCE : NETHERSPITE_BLOCKER_MIN_DISTANCE;
    float const length = netherspite.GetExactDist2d(portal);
    float const dirX = (portal.GetPositionX() - netherspite.GetPositionX()) / length;
    float const dirY = (portal.GetPositionY() - netherspite.GetPositionY()) / length;

    // Past the portal, a bot is off the beam.
    float const high = length - 1.0f;
    constexpr float searchStep = 0.5f;
    float best = preferred;
    float bestGap = std::numeric_limits<float>::max();
    auto const search = [&](float from)
    {
        for (uint32 i = 0; from + i * searchStep <= high; ++i)
        {
            float const distance = from + i * searchStep;
            float const x = netherspite.GetPositionX() + dirX * distance;
            float const y = netherspite.GetPositionY() + dirY * distance;
            float const gap = std::abs(distance - preferred);
            if (gap < bestGap &&
                IsClearOfVoidZones(state, x, y, VOID_ZONE_BEAM_POSITION_CLEARANCE))
            {
                best = distance;
                bestGap = gap;
            }
        }
    };

    search(low);
    if (isHunter && bestGap == std::numeric_limits<float>::max())
        search(NETHERSPITE_BLOCKER_MIN_DISTANCE);

    return best;
}

// The side of the blue or green beam that faces the other one: +1 if that's Netherspite's left as
// he faces this beam's portal, where off is positive, and -1 if it's his right.
float GetNetherspiteInnerSide(
    Unit& netherspite, KaraInstanceState const& state, NetherspiteBeam beam)
{
    float const toOwn = Position::NormalizeOrientation(
        GetBeamAngle(netherspite, state, beam) - GetSideWithoutRed(netherspite, state).start);
    return std::min(toOwn, 2.0f * static_cast<float>(M_PI) - toOwn) < 0.001f ? 1.0f : -1.0f;
}

// Where a bot that is neither blocking nor next to block may stand: in the standing arc, at its
// role's distance, and clear of void zones and beams. Healers also stay within 38 yd of the red
// beam tank. Melee, including tanks other than the red beam tank, stand behind Netherspite.
bool IsBetweenNetherspiteBeams(
    Player* bot, Unit& netherspite, KaraInstanceState const& state, float x, float y)
{
    if (!IsClearOfVoidZones(state, x, y, NETHERSPITE_VZ_CLEARANCE) ||
        !IsClearOfNetherspiteBeams(netherspite, state, x, y))
    {
        return false;
    }

    NetherspiteRadiusBand const band = GetNetherspiteRadiusBand(bot);
    float const radius = netherspite.GetExactDist2d(x, y);
    if (radius < band.low || radius > band.high)
        return false;

    NetherspiteArc const arc = GetNetherspiteStandingArc(netherspite, state);
    float const angle = netherspite.GetAngle(x, y);
    if (Position::NormalizeOrientation(angle - arc.start) > arc.width)
        return false;

    if (PlayerbotAI::IsHeal(bot))
    {
        NetherspiteBeamState const& red = state.netherspiteBeams[Id(NetherspiteBeam::Red)];
        return red.blocker.IsEmpty() ||
            red.blockerPosition.GetExactDist2d(x, y) <= NETHERSPITE_HEALER_TANK_RANGE;
    }

    if (bot->getClass() == CLASS_HUNTER || !PlayerbotAI::IsMelee(bot))
        return true;

    // To keep melee behind Netherspite while respecting beams and void zones, set behind is
    // disabled by multiplier, and this test is used to reject melee positions in front of him.
    float const fromRed = Position::NormalizeOrientation(
        angle - GetBeamAngle(netherspite, state, NetherspiteBeam::Red));
    return fromRed > M_PI / 2.0f && fromRed < 3.0f * M_PI / 2.0f;
}

// True if the position passes IsBetweenNetherspiteBeams and a straight move there from the bot
// crosses no void zone, and no beam that will be up by the time it arrives (i.e., the bot will not
// cross an active beam on its way to its destination).
bool CanMoveBetweenNetherspiteBeams(
    Player* bot, Unit* netherspite, KaraInstanceState const& state, float x, float y)
{
    return netherspite && IsBetweenNetherspiteBeams(bot, *netherspite, state, x, y) &&
        !CrossesVoidZone(state, bot->GetPosition(), Position(x, y)) &&
        !CrossesBeamAfterStart(bot, *netherspite, state, x, y);
}

// A random position the bot may stand in, or, failing that, the nearest safe position if the bot is
// in danger. None if it's safe where the bot is.
std::optional<Position> FindPositionBetweenNetherspiteBeams(
    Player* bot, Unit* netherspite, KaraInstanceState const& state)
{
    if (!netherspite)
        return std::nullopt;

    float z = bot->GetPositionZ();
    NetherspiteArc const arc = GetNetherspiteStandingArc(*netherspite, state);
    NetherspiteRadiusBand const band = GetNetherspiteRadiusBand(bot);
    for (uint8 i = 0; i < NETHERSPITE_POSITION_TRIES; ++i)
    {
        float const angle = arc.start + frand(0.0f, arc.width);
        float const radius = frand(band.low, band.high);
        float const x = netherspite->GetPositionX() + std::cos(angle) * radius;
        float const y = netherspite->GetPositionY() + std::sin(angle) * radius;
        if (CanMoveBetweenNetherspiteBeams(bot, netherspite, state, x, y) &&
            GetFloorAt(bot, x, y, z))
        {
            return Position(x, y, z);
        }
    }

    if (IsClearOfVoidZones(
            state, bot->GetPositionX(), bot->GetPositionY(), NETHERSPITE_VZ_CLEARANCE) &&
        IsClearOfNetherspiteBeams(
            *netherspite, state, bot->GetPositionX(), bot->GetPositionY()))
    {
        return std::nullopt;
    }

    constexpr float minDistance = 1.0f;
    constexpr float maxDistance = 20.0f;
    constexpr float distanceStep = 0.5f;
    constexpr uint8 numAngles = 24;
    for (uint8 step = 0; minDistance + step * distanceStep <= maxDistance; ++step)
    {
        float const distance = minDistance + step * distanceStep;
        for (uint8 i = 0; i < numAngles; ++i)
        {
            float const angle = i * 2.0f * M_PI / numAngles;
            float const x = bot->GetPositionX() + std::cos(angle) * distance;
            float const y = bot->GetPositionY() + std::sin(angle) * distance;

            if (IsClearOfVoidZones(state, x, y, NETHERSPITE_VZ_CLEARANCE) &&
                IsClearOfNetherspiteBeams(*netherspite, state, x, y) &&
                !CrossesVoidZone(state, bot->GetPosition(), Position(x, y)) &&
                !CrossesBeamAfterStart(bot, *netherspite, state, x, y) &&
                GetFloorAt(bot, x, y, z))
            {
                return Position(x, y, z);
            }
        }
    }

    return std::nullopt;
}

// The destination if the straight way there is clear of void zones; otherwise a position beside
// the nearest zone in the way, 3.5 yd from its center (5.5 if neither side works at 3.5). None if
// nothing works.
std::optional<Position> GetStepAroundVoidZones(
    Player* bot, Unit* netherspite, KaraInstanceState const& state, Position const& destination)
{
    if (!netherspite)
        return std::nullopt;

    Position const from = bot->GetPosition();
    Position const* zone = FindFirstVoidZoneOnPath(state, from, destination);
    float const length = from.GetExactDist2d(destination);
    if (!zone || length == 0.0f)
        return destination;

    float const normalX = (from.GetPositionY() - destination.GetPositionY()) / length;
    float const normalY = (destination.GetPositionX() - from.GetPositionX()) / length;
    float const zoneSide = (zone->GetPositionX() - from.GetPositionX()) * normalX +
        (zone->GetPositionY() - from.GetPositionY()) * normalY;
    float const away = zoneSide > 0.0f ? -1.0f : 1.0f;

    for (float const distance :
         { VOID_ZONE_STEP_AROUND_DISTANCE, VOID_ZONE_WIDE_STEP_AROUND_DISTANCE })
    {
        for (float const side : { away, -away })
        {
            float const x = zone->GetPositionX() + normalX * side * distance;
            float const y = zone->GetPositionY() + normalY * side * distance;
            float z = bot->GetPositionZ();

            if (IsClearOfVoidZones(state, x, y, NETHERSPITE_VZ_CLEARANCE) &&
                IsClearOfNetherspiteBeams(*netherspite, state, x, y) &&
                !CrossesVoidZone(state, from, Position(x, y)) &&
                !CrossesBeamAfterStart(bot, *netherspite, state, x, y) &&
                GetFloorAt(bot, x, y, z))
            {
                return Position(x, y, z);
            }
        }
    }

    return std::nullopt;
}

// Prince Malchezaar

// Enfeeble lasts a little bit longer than the next Shadow Nova cast, and there is only one cast
// per Enfeeble.
bool CanShadowNovaHitEnfeebled(Unit* malchezaar, Aura const& enfeeble)
{
    return enfeeble.GetDuration() >= MALC_SHADOW_NOVA_CAST_TIME_MS ||
        malchezaar->FindCurrentSpellBySpellId(Id(KaraSpells::SPELL_SHADOW_NOVA));
}

GuidVector FindInfernalGuids(Player* bot)
{
    return FindCreatureGuidsAround(
        bot, MALCHEZAAR_SPAWN_POSITION, INFERNAL_SEARCH_RADIUS,
        { Id(KaraNpcs::NPC_NETHERSPITE_INFERNAL) });
}

std::vector<Position> GetLandedInfernals(Player* bot)
{
    GuidVector const& guids = GetInstanceCreatureList(
        KaraState(bot->GetInstanceId()).malchezaarInfernals, INFERNALS_CACHE_INTERVAL_MS, bot,
        FindInfernalGuids);

    std::vector<Position> infernals;
    for (ObjectGuid const& guid : guids)
    {
        Creature* infernal = ObjectAccessor::GetCreature(*bot, guid);
        if (infernal && infernal->IsAlive() &&
            infernal->GetDisplayId() != INFERNAL_MODEL_INVISIBLE)
        {
            infernals.push_back(infernal->GetPosition());
        }
    }

    return infernals;
}

bool IsClearOfInfernals(
    Position const& position, std::vector<Position> const& infernals, float clearance)
{
    return std::none_of(infernals.begin(), infernals.end(), [&](Position const& infernal)
    {
        return infernal.GetExactDist2d(position) < clearance;
    });
}

bool IsLineClearOfInfernals(
    Position const& from, Position const& to, std::vector<Position> const& infernals)
{
    return std::none_of(infernals.begin(), infernals.end(), [&](Position const& infernal)
    {
        return infernal.GetExactDist2d(from) >= MALC_HELLFIRE_HAZARD_RADIUS &&
            GetDistanceToSegment2d(infernal, from, to) < MALC_HELLFIRE_HAZARD_RADIUS;
    });
}

namespace
{

bool CanWalkStraightTo(Player* bot, Position const& destination)
{
    float x = destination.GetPositionX();
    float y = destination.GetPositionY();
    float z = bot->GetMapWaterOrGroundLevel(x, y, bot->GetPositionZ());
    if (z <= INVALID_HEIGHT)
        z = bot->GetPositionZ();

    if (!bot->GetMap()->CanReachPositionAndGetValidCoords(bot, x, y, z, true, false))
        return false;

    // An incomplete path can cut the point short without counting as a collision. A point the move
    // stops further short of than the movers' arrival distance is rejected, not clamped.
    return std::hypot(x - destination.GetPositionX(), y - destination.GetPositionY()) <=
        MALCHEZAAR_ARRIVAL_DISTANCE;
}

// The nearest position clear of Hellfire from which the bot can keep fighting or healing. Ranged:
// just out of Shadow Nova from Malchezaar, and in range of their target. Melee: just inside melee
// range of their target. A line that passes no infernal comes first, then any straight line.
std::optional<Position> FindPositionInRangeOfTarget(
    PlayerbotAI* botAI, Unit* malchezaar, std::vector<Position> const& infernals)
{
    if (!malchezaar)
        return std::nullopt;

    Player* bot = botAI->GetBot();
    bool const isRanged = PlayerbotAI::IsRanged(bot);
    bool const isHealer = PlayerbotAI::IsHeal(bot);
    Unit* target = botAI->GetAiObjectContext()
        ->GetValue<Unit*>(isHealer ? "party member to heal" : "current target")->Get();
    if (!target)
        target = malchezaar;

    // How far from the target's center the bot can stand and still reach it. Spell range is edge to
    // edge, so both reaches are added.
    float reach;
    if (isRanged)
    {
        float const spellRange = botAI->GetRange(isHealer ? "heal" : "spell");
        reach = spellRange + bot->GetCombatReach() + target->GetCombatReach();
    }
    else
        reach = bot->GetMeleeRange(target);

    constexpr float rangeInset = 1.0f;
    reach -= rangeInset;

    // Candidates lie on a ring: around Malchezaar just out of Shadow Nova for ranged, around the
    // target at reach for melee.
    Unit* ringCenter = isRanged ? malchezaar : target;
    float const ringRadius = isRanged ? MALC_SHADOW_NOVA_CLEARANCE : reach;
    Position const origin = bot->GetPosition();

    constexpr uint8 angleCount = 72;
    std::vector<std::pair<float, Position>> candidates;
    for (uint8 i = 0; i < angleCount; ++i)
    {
        float const angle = 2.0f * static_cast<float>(M_PI) * i / angleCount;
        Position const candidate(
            ringCenter->GetPositionX() + ringRadius * std::cos(angle),
            ringCenter->GetPositionY() + ringRadius * std::sin(angle), origin.GetPositionZ());

        bool const isInReach = ringCenter == target || target->GetExactDist2d(candidate) <= reach;
        if (isInReach && IsClearOfInfernals(candidate, infernals, MALC_HELLFIRE_CLEARANCE))
            candidates.emplace_back(origin.GetExactDist2d(candidate), candidate);
    }

    std::stable_sort(candidates.begin(), candidates.end(), [](auto const& a, auto const& b)
    {
        return a.first < b.first;
    });

    uint8 walkChecks = 0;
    for (bool const clearLine : { true, false })
    {
        for (auto const& [distance, candidate] : candidates)
        {
            if (IsLineClearOfInfernals(origin, candidate, infernals) != clearLine)
                continue;

            if (walkChecks >= MALCHEZAAR_MAX_WALK_CHECKS)
                return std::nullopt;

            ++walkChecks;
            if (CanWalkStraightTo(bot, candidate))
                return candidate;
        }
    }

    return std::nullopt;
}

} // end anonymous namespace (Prince Malchezaar)

// The nearest position to origin, searching outward up to maxDistance, that is infernalClearance
// from every landed infernal, out of Shadow Nova from novaSource if one is given, and reachable in
// a straight line. Lines that pass no infernal come first; with mayCross, any line after that.
std::optional<Position> FindNearestMalchezaarSafePosition(
    Player* bot, Position const& origin, std::vector<Position> const& infernals,
    float maxDistance, bool mayCross, float infernalClearance, Unit* novaSource)
{
    constexpr float ringStep = 2.0f;
    constexpr uint8 angleCount = 32;

    int const ringCount = static_cast<int>(maxDistance / ringStep);
    uint8 walkChecks = 0;
    for (bool const clearLine : { true, false })
    {
        if (!clearLine && !mayCross)
            break;

        for (int ring = 1; ring <= ringCount; ++ring)
        {
            float const distance = ring * ringStep;
            for (uint8 i = 0; i < angleCount; ++i)
            {
                float const angle = 2.0f * static_cast<float>(M_PI) * i / angleCount;
                Position const candidate(
                    origin.GetPositionX() + distance * std::cos(angle),
                    origin.GetPositionY() + distance * std::sin(angle), origin.GetPositionZ());

                bool const isGood = IsClearOfInfernals(candidate, infernals, infernalClearance) &&
                    (!novaSource ||
                     novaSource->GetExactDist2d(candidate) >= MALC_SHADOW_NOVA_CLEARANCE);

                if (!isGood || (clearLine && !IsLineClearOfInfernals(origin, candidate, infernals)))
                    continue;

                if (walkChecks >= MALCHEZAAR_MAX_WALK_CHECKS)
                    return std::nullopt;

                ++walkChecks;
                if (CanWalkStraightTo(bot, candidate))
                    return candidate;
            }
        }
    }

    return std::nullopt;
}

std::optional<Position> FindMalchezaarClearPosition(
    PlayerbotAI* botAI, Unit* malchezaar, std::vector<Position> const& infernals)
{
    if (malchezaar)
    {
        std::optional<Position> const position =
            FindPositionInRangeOfTarget(botAI, malchezaar, infernals);
        if (position)
            return position;
    }

    Player* bot = botAI->GetBot();
    bool const isRanged = PlayerbotAI::IsRanged(bot);
    return FindNearestMalchezaarSafePosition(
        bot, bot->GetPosition(), infernals, MALCHEZAAR_SEARCH_DISTANCE, true,
        MALC_HELLFIRE_CLEARANCE, isRanged ? malchezaar : nullptr);
}

bool IsClearOfMalchezaarHazards(
    Player* bot, Unit* malchezaar, std::vector<Position> const& infernals, Position const& position)
{
    return IsClearOfInfernals(position, infernals, MALC_HELLFIRE_CLEARANCE) &&
        (!malchezaar || !PlayerbotAI::IsRanged(bot) ||
         malchezaar->GetExactDist2d(position) >= MALC_SHADOW_NOVA_CLEARANCE);
}

// Where the tank should take Malchezaar, within 60 yd: (a) clear of Infernals by enough that, with
// him trailing behind, nobody in his melee range is in Hellfire; (b) failing that, enough for
// his melee range alone; and (c) failing that, enough for the tank. Sets clearance to the distance
// the position met, so the position can later be rechecked against new Infernals.
std::optional<Position> FindMalchezaarTankPosition(
    Player* bot, std::vector<Position> const& infernals, float& clearance)
{
    constexpr float clearances[] =
    {
        MALCHEZAAR_MELEE_CLEARANCE + MALCHEZAAR_TRAIL_DISTANCE,
        MALCHEZAAR_MELEE_CLEARANCE,
        MALC_HELLFIRE_CLEARANCE + 1.0f,
    };

    for (float const candidateClearance : clearances)
    {
        std::optional<Position> const position = FindNearestMalchezaarSafePosition(
            bot, bot->GetPosition(), infernals, MALCHEZAAR_SEARCH_DISTANCE, true,
            candidateClearance, nullptr);

        if (position)
        {
            clearance = candidateClearance;
            return position;
        }
    }

    return std::nullopt;
}

// Nightbane

float GetTerraceAngle(Position const& position)
{
    float const angle = std::atan2(
        position.GetPositionY() - TERRACE_DOME_CENTER.GetPositionY(),
        position.GetPositionX() - TERRACE_DOME_CENTER.GetPositionX());

    return angle < TERRACE_ARC_START ? angle + 2.0f * static_cast<float>(M_PI) : angle;
}

// The destination, unless the straight line to it would pass through the dome, in which case this
// is a step on an arc that moves to the destination.
Position GetTerraceWalkPosition(Position const& from, Position const& destination)
{
    if (GetDistanceToSegment2d(TERRACE_DOME_CENTER, from, destination) >= TERRACE_INNER_RADIUS)
        return destination;

    constexpr float stepDist = 3.5f;
    float const radius =
        std::max(from.GetExactDist2d(TERRACE_DOME_CENTER), TERRACE_INNER_LANE_DIST);
    float const fromAngle = GetTerraceAngle(from);
    float const angleLeft = GetTerraceAngle(destination) - fromAngle;
    float const maxStepAngle = stepDist / radius;
    float const angle = fromAngle + std::clamp(angleLeft, -maxStepAngle, maxStepAngle);

    return Position(
        TERRACE_DOME_CENTER.GetPositionX() + radius * std::cos(angle),
        TERRACE_DOME_CENTER.GetPositionY() + radius * std::sin(angle),
        from.GetPositionZ());
}

// Searches the terrace's inner lane outward from startAngle in 2.5° steps, alternating sides,
// clockwise first. Returns the nearest flight stack position clear of Charred Earth and outside
// any Rain of Bones.
std::optional<Position> FindNightbaneStackPosition(
    float startAngle, std::vector<Position> const& charredEarths, Position const* rainCenter)
{
    constexpr float angleStep = 0.0436f; // 2.5°, or 1.7 yd

    for (uint8 i = 0;
         i * angleStep <= NIGHTBANE_FLIGHT_STACK_MAX_ANGLE - NIGHTBANE_FLIGHT_STACK_MIN_ANGLE; ++i)
    {
        for (float const direction : { -1.0f, 1.0f })
        {
            float const angle = startAngle + direction * i * angleStep;
            if (angle < NIGHTBANE_FLIGHT_STACK_MIN_ANGLE ||
                angle > NIGHTBANE_FLIGHT_STACK_MAX_ANGLE)
            {
                continue;
            }

            Position const position(
                TERRACE_DOME_CENTER.GetPositionX() + TERRACE_INNER_LANE_DIST * std::cos(angle),
                TERRACE_DOME_CENTER.GetPositionY() + TERRACE_INNER_LANE_DIST * std::sin(angle),
                NIGHTBANE_GROUND_Z);
            if (IsClearOfCharredEarth(position, charredEarths) &&
                (!rainCenter ||
                 position.GetExactDist2d(*rainCenter) >= RAIN_OF_BONES_CLEARANCE))
            {
                return position;
            }
        }
    }

    return std::nullopt;
}

std::vector<Position> const& GetCharredEarths(PlayerbotAI* botAI)
{
    return botAI->GetAiObjectContext()
        ->GetValue<std::vector<Position>>("nightbane charred earths")->RefGet();
}

bool IsClearOfCharredEarth(Position const& position, std::vector<Position> const& charredEarths)
{
    for (Position const& charredEarth : charredEarths)
    {
        if (charredEarth.GetExactDist2d(position) < CHARRED_EARTH_CLEARANCE)
            return false;
    }

    return true;
}

bool IsOnNightbaneRangedSide(Position const& position, Unit& nightbane)
{
    return GetTerraceAngle(position) < GetTerraceAngle(nightbane);
}

bool IsInNightbaneRangedSight(Player& bot, Position const& from, Position const& to)
{
    float const eyeHeight = bot.GetCollisionHeight();
    return bot.GetMap()->isInLineOfSight(
        from.GetPositionX(), from.GetPositionY(), from.GetPositionZ() + eyeHeight,
        to.GetPositionX(), to.GetPositionY(), to.GetPositionZ() + eyeHeight,
        bot.GetPhaseMask(), LINEOFSIGHT_ALL_CHECKS, VMAP::ModelIgnoreFlags::Nothing);
}

std::optional<Position> FindNightbaneRangedPosition(
    Player& bot, Unit& nightbane, Position const& from, std::vector<Position> const& charredEarths,
    NightbaneRangedTier tier)
{
    constexpr float angleStep = 0.0436f; // 2.5°
    constexpr float maxAngleOffset = 1.0f; // 57°, beyond the ranged band on every ring
    constexpr uint8 ringCount = 9;
    constexpr float ringStep =
        (TERRACE_FLOOR_OUTER_RADIUS - TERRACE_FLOOR_INNER_RADIUS) / (ringCount - 1);

    float const minBossDist = tier == NightbaneRangedTier::Forward ?
        nightbane.GetMeleeRange(&bot) : NIGHTBANE_RANGED_MIN_DIST;
    uint8 const firstRing = tier == NightbaneRangedTier::Parapet ? ringCount - 1 : 0;

    float const bossAngle = GetTerraceAngle(nightbane);
    std::vector<std::pair<float, Position>> candidates;
    for (uint8 i = 1; i * angleStep <= maxAngleOffset; ++i)
    {
        float const angle = bossAngle - i * angleStep;
        if (angle < TERRACE_ARC_START)
            break;

        for (uint8 ring = firstRing; ring < ringCount; ++ring)
        {
            float const radius = TERRACE_FLOOR_INNER_RADIUS + ring * ringStep;
            Position const candidate(
                TERRACE_DOME_CENTER.GetPositionX() + radius * std::cos(angle),
                TERRACE_DOME_CENTER.GetPositionY() + radius * std::sin(angle),
                NIGHTBANE_GROUND_Z);
            float const bossDist = nightbane.GetExactDist2d(candidate);
            if (bossDist >= minBossDist && bossDist <= NIGHTBANE_RANGED_MAX_DIST &&
                IsClearOfCharredEarth(candidate, charredEarths))
            {
                candidates.emplace_back(from.GetExactDist2d(candidate), candidate);
            }
        }
    }

    std::sort(
        candidates.begin(), candidates.end(),
        [](auto const& a, auto const& b) { return a.first < b.first; });

    Position const farFlank =
        GetNightbaneMeleePosition(nightbane, NightbaneMeleePosition::CounterclockwiseFlank);
    for (auto const& candidate : candidates)
    {
        if (IsInNightbaneRangedSight(bot, candidate.second, nightbane.GetPosition()) &&
            (tier == NightbaneRangedTier::Forward ||
             IsInNightbaneRangedSight(bot, candidate.second, farFlank)))
        {
            return candidate.second;
        }
    }

    return std::nullopt;
}

Position GetNightbaneMeleePosition(Unit& nightbane, NightbaneMeleePosition position)
{
    // Angles from Nightbane's facing. Negative is clockwise.
    float angle = NIGHTBANE_MELEE_HOLD_ANGLE;
    float dist = NIGHTBANE_MELEE_HOLD_DIST;
    if (position != NightbaneMeleePosition::Hold)
    {
        angle = position == NightbaneMeleePosition::ClockwiseFlank ?
            -NIGHTBANE_MELEE_FLANK_ANGLE : NIGHTBANE_MELEE_FLANK_ANGLE;
        dist = NIGHTBANE_MELEE_FLANK_DIST;
    }

    float const facing = nightbane.GetOrientation() + angle;
    return Position(
        nightbane.GetPositionX() + dist * std::cos(facing),
        nightbane.GetPositionY() + dist * std::sin(facing), nightbane.GetPositionZ());
}

bool HoldsNightbaneGroundPosition(PlayerbotAI* botAI, Unit& nightbane)
{
    Player* bot = botAI->GetBot();
    if (nightbane.GetVictim() == bot || (PlayerbotAI::IsTank(bot) && PlayerbotAI::IsMainTank(bot)))
        return false;

    return PlayerbotAI::IsRanged(bot) ||
        botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Get() == &nightbane;
}

bool IsOutsideNightbaneTerrace(Unit* unit)
{
    if (!unit)
        return false;

    constexpr float belowTerraceLeeway = 5.0f;
    if (unit->GetPositionZ() < NIGHTBANE_GROUND_Z - belowTerraceLeeway)
        return true;

    return unit->GetExactDist2d(TERRACE_DOME_CENTER) > TERRACE_OUTER_RADIUS &&
        GetTerraceAngle(*unit) <= TERRACE_ARC_END;
}

bool IsNightbaneFlying(Unit* nightbane)
{
    return nightbane && nightbane->GetPositionZ() > NIGHTBANE_FLIGHT_Z;
}

bool IsNightbaneFlightPhaseActive(Unit* nightbane)
{
    if (!IsNightbaneFlying(nightbane))
        return false;

    KaraInstanceState const* state = FindKaraState(nightbane->GetInstanceId());
    return !state || state->nightbaneFlightStage != NightbaneFlightStage::Landing;
}

bool IsNightbaneDpsHeld(Player* bot, Unit* nightbane)
{
    if (!nightbane)
        return false;

    if (IsNightbaneFlying(nightbane))
        return true;

    KaraInstanceState const* state = FindKaraState(nightbane->GetInstanceId());
    if (state && state->nightbaneDpsHoldTimer &&
        getMSTimeDiff(*state->nightbaneDpsHoldTimer, getMSTime()) >= NIGHTBANE_DPS_HOLD_MS)
    {
        return false;
    }

    return !PlayerbotAI::IsMainTank(bot);
}

// Where the current Rain of Bones channel is aimed, read after RAIN_OF_BONES_REACTION_MS has passed
// (or bots unrealistically react before the visual lands). Rain of Bones doesn't follow its target.
Position const* GetRainOfBonesCenter(Unit* nightbane)
{
    if (!nightbane)
        return nullptr;

    Spell* channel = nightbane->GetCurrentSpell(CURRENT_CHANNELED_SPELL);
    if (!channel || channel->m_spellInfo->Id != Id(KaraSpells::SPELL_RAIN_OF_BONES) ||
        channel->m_spellInfo->GetDuration() - channel->GetCastTimeRemaining() <
            RAIN_OF_BONES_REACTION_MS)
    {
        return nullptr;
    }

    return channel->m_targets.GetDstPos();
}

// Shared encounter state

KaraInstanceState& KaraState(uint32 instanceId)
{
    std::lock_guard lock(karaStateMutex);
    return karaStates[instanceId];
}

KaraInstanceState const* FindKaraState(uint32 instanceId)
{
    std::lock_guard lock(karaStateMutex);
    auto it = karaStates.find(instanceId);
    return it != karaStates.end() ? &it->second : nullptr;
}

bool KaraResetInstance(uint32 instanceId)
{
    std::lock_guard lock(karaStateMutex);
    auto it = karaStates.find(instanceId);
    if (it == karaStates.end())
        return false;

    KaraInstanceState const& state = it->second;
    bool const wasSet = state.attumenDpsHoldTimer || state.netherspitePortalPhaseTimer ||
        AreNetherspitePortalsKnown(state) || !state.netherspiteVoidZones.empty() ||
        !state.netherspiteBanishSlots.empty() || state.nightbaneDpsHoldTimer ||
        state.nightbaneFlightStage || state.nightbaneFlightStackPosition ||
        state.nightbaneRainOfBonesCenter || state.nightbaneMeleePosition ||
        state.nightbaneRangedPosition;

    karaStates.erase(it);
    return wasSet;
}

}
