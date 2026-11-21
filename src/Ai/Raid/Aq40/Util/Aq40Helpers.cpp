/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "Aq40Helpers.h"
#include "AiFactory.h"
#include "DBCStores.h"
#include "EncounterHelpers.h"
#include "InstanceScript.h"
#include "LastMovementValue.h"
#include "MoveSpline.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "PathGenerator.h"
#include "Playerbots.h"
#include "Spell.h"
#include "SpellAuras.h"
#include "SpellMgr.h"
#include "Timer.h"
#include "TotemsShamanStrategy.h"
#include <algorithm>
#include <cmath>
#include <iterator>
#include <limits>
#include <list>
#include <mutex>
#include <set>
#include <unordered_map>

namespace Aq40Helpers
{

namespace
{

std::mutex aq40StateMutex;
std::unordered_map<uint32, Aq40InstanceState> aq40States;

float AngleDelta(float to, float from) { return std::remainder(to - from, 2.0f * static_cast<float>(M_PI)); }

// Dark Glare hits anyone in front of the Eye less than 5 yd from its facing line (spell_cthun_dark_glare -> HasInLine).
bool InGlareBand(float bearing, float distance, float line, float halfWidth)
{
    float relative = AngleDelta(bearing, line);
    return std::abs(relative) < static_cast<float>(M_PI) / 2.0f && std::abs(std::sin(relative)) * distance < halfWidth;
}

// In the band now or within the next `ticks` ticks. Direction 0 means not known yet: both ways count.
bool InGlarePath(float bearing, float distance, float facing, float direction, float halfWidth, int ticks)
{
    for (int tick = 0; tick <= ticks; ++tick)
    {
        if (direction != 0.0f && InGlareBand(bearing, distance, facing + direction * GLARE_STEP * tick, halfWidth))
            return true;
        if (direction == 0.0f && (InGlareBand(bearing, distance, facing + GLARE_STEP * tick, halfWidth) ||
                                  InGlareBand(bearing, distance, facing - GLARE_STEP * tick, halfWidth)))
            return true;
    }
    return false;
}

// Bearing (around the Eye) to step to: ahead of the sweep for anyone it is coming toward, behind it for anyone it is
// leaving, and away from the line on the bot's own side while the direction is unknown.
float GlareEscapeBearing(float bearing, float distance, float facing, float direction, float halfWidth, int ticks)
{
    float relative = AngleDelta(bearing, facing);
    float side = relative >= 0.0f ? 1.0f : -1.0f;
    bool ahead = direction != 0.0f && relative * direction > 0.0f;
    if (ahead)
        side = direction;
    float clear = std::asin(std::min(1.0f, halfWidth / std::max(distance, halfWidth)));
    float lead = direction == 0.0f || ahead ? GLARE_STEP * float(ticks + 1) : GLARE_STEP;
    return facing + side * (clear + lead);
}

bool OnCthunFloor(WorldObject* probe, float x, float y)
{
    return probe->GetMapHeight(x, y, CTHUN_CENTER.GetPositionZ() + CTHUN_FLOOR_PROBE_HEIGHT) >
           CTHUN_CENTER.GetPositionZ() - CTHUN_FLOOR_TOLERANCE;
}

std::vector<uint32> EncounterEntries(Aq40Encounter encounter)
{
    switch (encounter)
    {
        case Aq40Encounter::Skeram:
            return {Id(Aq40Npcs::NPC_SKERAM)};
        case Aq40Encounter::Trio:
            return {Id(Aq40Npcs::NPC_KRI), Id(Aq40Npcs::NPC_YAUJ), Id(Aq40Npcs::NPC_VEM), Id(Aq40Npcs::NPC_YAUJ_BROOD),
                    Id(Aq40Npcs::NPC_POISON_CLOUD)};
        case Aq40Encounter::Sartura:
            return {Id(Aq40Npcs::NPC_SARTURA), Id(Aq40Npcs::NPC_SARTURA_ROYAL_GUARD)};
        case Aq40Encounter::Fankriss:
            return {Id(Aq40Npcs::NPC_FANKRISS), Id(Aq40Npcs::NPC_SPAWN_OF_FANKRISS),
                    Id(Aq40Npcs::NPC_VEKNISS_HATCHLING)};
        case Aq40Encounter::Viscidus:
            return {Id(Aq40Npcs::NPC_VISCIDUS), Id(Aq40Npcs::NPC_GLOB_OF_VISCIDUS), Id(Aq40Npcs::NPC_TOXIC_SLIME)};
        case Aq40Encounter::Huhuran:
            return {Id(Aq40Npcs::NPC_HUHURAN)};
        case Aq40Encounter::Twins:
            return {Id(Aq40Npcs::NPC_VEKNILASH), Id(Aq40Npcs::NPC_VEKLOR), Id(Aq40Npcs::NPC_QIRAJI_SCARAB),
                    Id(Aq40Npcs::NPC_QIRAJI_SCORPION)};
        case Aq40Encounter::Ouro:
            return {Id(Aq40Npcs::NPC_OURO), Id(Aq40Npcs::NPC_DIRT_MOUND), Id(Aq40Npcs::NPC_OURO_SCARAB)};
        case Aq40Encounter::Cthun:
            return {Id(Aq40Npcs::NPC_EYE_OF_CTHUN),       Id(Aq40Npcs::NPC_CTHUN),
                    Id(Aq40Npcs::NPC_EYE_TENTACLE),       Id(Aq40Npcs::NPC_CLAW_TENTACLE),
                    Id(Aq40Npcs::NPC_GIANT_EYE_TENTACLE), Id(Aq40Npcs::NPC_GIANT_CLAW_TENTACLE),
                    Id(Aq40Npcs::NPC_FLESH_TENTACLE),     Id(Aq40Npcs::NPC_EXIT_TRIGGER)};
        default:
            return {};
    }
}

uint32 ElapsedSinceEncounterStart(Player* bot)
{
    Aq40InstanceState const& state = Aq40State(bot->GetInstanceId());
    return state.encounterStart ? getMSTimeDiff(*state.encounterStart, getMSTime()) : 0;
}

bool IsPhysicalTwinDamage(Player* player) { return PlayerbotAI::IsMelee(player) || player->getClass() == CLASS_HUNTER; }

Unit* PickKillTarget(Player* bot, std::vector<uint32> const& priorities)
{
    for (uint32 entry : priorities)
    {
        bool bug = entry == Id(Aq40Npcs::NPC_QIRAJI_SCARAB) || entry == Id(Aq40Npcs::NPC_QIRAJI_SCORPION);
        std::vector<Creature*> candidates = GetUnits(bot, entry);
        candidates.erase(
            std::remove_if(candidates.begin(), candidates.end(), [&](Creature* unit)
                           { return !IsAllowedDamage(bot, unit) || (bug && unit->GetDistance(bot) > TWIN_BUG_REACH); }),
            candidates.end());
        if (candidates.empty())
            continue;
        // Bosses and other shared targets go by lowest GUID, so the raid agrees on one; local adds go by distance.
        Creature* selected = candidates.front();
        bool localTarget = entry == Id(Aq40Npcs::NPC_GLOB_OF_VISCIDUS) || entry == Id(Aq40Npcs::NPC_FLESH_TENTACLE) ||
                           entry == Id(Aq40Npcs::NPC_EYE_TENTACLE) || entry == Id(Aq40Npcs::NPC_CLAW_TENTACLE) || bug;
        if (localTarget)
            for (Creature* unit : candidates)
                if (unit->GetDistance(bot) < selected->GetDistance(bot))
                    selected = unit;
        bool needsTank = IsEncounterBoss(entry) && entry != Id(Aq40Npcs::NPC_EYE_OF_CTHUN) &&
                         entry != Id(Aq40Npcs::NPC_CTHUN) && entry != Id(Aq40Npcs::NPC_VISCIDUS);
        needsTank =
            needsTank || entry == Id(Aq40Npcs::NPC_SPAWN_OF_FANKRISS) || entry == Id(Aq40Npcs::NPC_GIANT_CLAW_TENTACLE);
        return needsTank && !IsHeld(bot, selected) ? nullptr : selected;
    }
    return nullptr;
}

Unit* GetOwnedTarget(Player* bot)
{
    Unit* selected = nullptr;
    float score = -1.0f;
    for (Creature* unit : GetUnits(bot))
    {
        if (GetTankFor(bot, unit) != bot || !IsAllowedDamage(bot, unit))
            continue;
        float current = unit->GetVictim() == bot ? 1.0f : 10.0f;
        if (Unit* victim = unit->GetVictim())
            if (victim->IsPlayer() && PlayerbotAI::IsHeal(victim->ToPlayer()))
                current += 100.0f;
        if (unit->GetEntry() == Id(Aq40Npcs::NPC_SPAWN_OF_FANKRISS))
            current += 20.0f;
        if (!selected || current > score)
        {
            selected = unit;
            score = current;
        }
    }
    return selected;
}

// An unassigned off-tank can help damage, but must never taunt another tank's boss.
Unit* GetTankOrKillTarget(Player* bot, std::vector<uint32> const& priorities)
{
    Unit* owned = PlayerbotAI::IsTank(bot) ? GetOwnedTarget(bot) : nullptr;
    return owned ? owned : PickKillTarget(bot, priorities);
}

Unit* GetSkeramTarget(Player* bot)
{
    if (PlayerbotAI::IsTank(bot))
        return GetOwnedTarget(bot);
    Aq40InstanceState& state = Aq40State(bot->GetInstanceId());
    auto side = state.skeramMemberPlatform.find(bot->GetGUID());
    if (side == state.skeramMemberPlatform.end())
        return nullptr;
    Creature* original = nullptr;
    for (Creature* unit : GetUnits(bot, Id(Aq40Npcs::NPC_SKERAM)))
    {
        if (!IsAllowedDamage(bot, unit))
            continue;
        if (!unit->IsSummon())
            original = unit;
        else if (auto body = state.skeramImagePlatform.find(unit->GetGUID());
                 body != state.skeramImagePlatform.end() && body->second == side->second && IsHeld(bot, unit))
            return unit;
    }
    // Before images spawn, and until the assist tank picks up our image, help the main tank.
    // A missing or moving tank on another platform must not block this target.
    return original && IsHeld(bot, original) ? original : nullptr;
}

Unit* GetBugTrioTarget(Player* bot)
{
    std::vector<uint32> priorities = {Id(Aq40Npcs::NPC_YAUJ_BROOD), Id(Aq40Npcs::NPC_KRI), Id(Aq40Npcs::NPC_YAUJ),
                                      Id(Aq40Npcs::NPC_VEM)};
    // A living bug marked skull explicitly overrides the default Kri/Yauj/Vem order.
    if (Group* group = bot->GetGroup())
        if (Creature* marked = bot->GetMap()->GetCreature(group->GetTargetIcon(SKULL_ICON)))
            if (marked->GetEntry() == Id(Aq40Npcs::NPC_KRI) || marked->GetEntry() == Id(Aq40Npcs::NPC_YAUJ) ||
                marked->GetEntry() == Id(Aq40Npcs::NPC_VEM))
                priorities.insert(priorities.begin() + 1, marked->GetEntry());
    return GetTankOrKillTarget(bot, priorities);
}

Unit* GetSarturaTarget(Player* bot)
{
    if (Unit* owned = PlayerbotAI::IsTank(bot) ? GetOwnedTarget(bot) : nullptr)
        return owned;
    // Every whirlwind wipes threat as it starts and ends, so a unit is rarely held after it. Only the opening waits
    // for the tanks; the taunt trigger takes a loose unit back. Melee skip a whirlwinding skull for the next unit.
    bool const opening = ElapsedSinceEncounterStart(bot) < SARTURA_OPENING_MS;
    for (Creature* unit : GetSarturaKillOrder(bot))
        if (IsAllowedDamage(bot, unit) && (!opening || IsHeld(bot, unit)))
            return unit;
    return nullptr;
}

Unit* GetViscidusTarget(Player* bot)
{
    return GetTankOrKillTarget(bot, {Id(Aq40Npcs::NPC_GLOB_OF_VISCIDUS), Id(Aq40Npcs::NPC_VISCIDUS)});
}

Unit* GetHuhuranTarget(Player* bot) { return GetTankOrKillTarget(bot, {Id(Aq40Npcs::NPC_HUHURAN)}); }

Unit* GetTwinEmperorsTarget(Player* bot)
{
    std::vector<uint32> const priorities = {
        Id(Aq40Npcs::NPC_QIRAJI_SCARAB), Id(Aq40Npcs::NPC_QIRAJI_SCORPION),
        IsPhysicalTwinDamage(bot) ? Id(Aq40Npcs::NPC_VEKNILASH) : Id(Aq40Npcs::NPC_VEKLOR)};
    if (IsTwinLock(bot, bot))
    {
        // Only the warlock on Vek'lor's side builds threat; the other one waits for his next phase.
        for (Creature* veklor : GetUnits(bot, Id(Aq40Npcs::NPC_VEKLOR)))
            if (GetTankFor(bot, veklor) == bot && IsAllowedDamage(bot, veklor))
                return veklor;
        return nullptr;
    }
    // Vek'nilash cannot be taunted: when his tank dies he goes to the next name on his threat list. A spare tank
    // keeps that name a tank instead of a healer.
    if (IsTwinSpareTank(bot, bot))
    {
        // Bugs come first for damage dealers, but a spare tank busy on a scorpion builds no threat on Vek'nilash,
        // who then goes to a damage dealer when his tank dies.
        for (Creature* nilash : GetUnits(bot, Id(Aq40Npcs::NPC_VEKNILASH)))
            if (IsAllowedDamage(bot, nilash) && IsHeld(bot, nilash))
                return nilash;
        return PickKillTarget(bot, priorities);
    }
    return PlayerbotAI::IsTank(bot) ? GetOwnedTarget(bot) : PickKillTarget(bot, priorities);
}

static bool IsNearDirtMound(Player* bot, Unit* unit)
{
    for (Creature* mound : GetUnits(bot, Id(Aq40Npcs::NPC_DIRT_MOUND)))
        if (mound->GetExactDist2d(unit) < DIRT_MOUND_RADIUS)
            return true;
    return false;
}

Unit* GetOuroTarget(Player* bot)
{
    if (PlayerbotAI::IsTank(bot))
    {
        if (Unit* owned = GetOwnedTarget(bot))
            return owned;
        // Backup tanks must keep building threat before the next Sand Blast.
        if (Creature* boss = GetBoss(bot); boss && IsAllowedDamage(bot, boss))
            return boss;
    }
    if (PlayerbotAI::IsMelee(bot))
    {
        // Scarabs spawn where the mounds stood, and a live mound's Quake would walk melee straight back out.
        // Melee take the scarabs outside every mound and leave the rest to ranged.
        for (Creature* scarab : GetUnits(bot, Id(Aq40Npcs::NPC_OURO_SCARAB)))
            if (IsAllowedDamage(bot, scarab) && !IsNearDirtMound(bot, scarab))
                return scarab;
        return PickKillTarget(bot, {Id(Aq40Npcs::NPC_OURO)});
    }
    return PickKillTarget(bot, {Id(Aq40Npcs::NPC_OURO_SCARAB), Id(Aq40Npcs::NPC_OURO)});
}

Unit* GetCthunStomachTarget(Player* bot)
{
    // No last-Flesh hold: waiting on the Giant tentacles outside wastes 36-94 s per window, while melee on
    // C'Thun and the Giant Eye response make the burn safe without it.
    return StomachExitNeeded(bot) ? nullptr : PickKillTarget(bot, {Id(Aq40Npcs::NPC_FLESH_TENTACLE)});
}

bool IsCthunEyeMelee(Player* bot)
{
    if (IsCthunPhase2(bot))
        return false;
    Aq40InstanceState const& state = Aq40State(bot->GetInstanceId());
    auto found = state.cthunSpots.find(bot->GetGUID());
    return found != state.cthunSpots.end() && CTHUN_SLOTS[found->second].kind == CthunSlotKind::EyeMelee;
}

Unit* GetCthunTarget(Player* bot)
{
    // Nobody chases a Giant Claw far from the raid: with no one in melee range it submerges, heals and re-emerges
    // under a random player (boss_cthun), back in the stack. Chasing it strands tanks out of healer range.
    Position spot;
    bool haveSpot = GetCthunSpot(bot, spot);
    auto nearRaid = [&](Unit* claw) { return haveSpot && claw->GetExactDist2d(&spot) <= CTHUN_FAR_CLAW; };
    bool tank = PlayerbotAI::IsTank(bot);
    // A tank takes its assigned Giant Claw when it is near the raid.
    if (tank)
        for (Creature* claw : GetUnits(bot, Id(Aq40Npcs::NPC_GIANT_CLAW_TENTACLE)))
            if (GetTankFor(bot, claw) == bot && IsAllowedDamage(bot, claw) && nearRaid(claw))
                return claw;
    // The tentacle killers take the far Eye Tentacles, even while C'Thun is Weakened. A Giant Eye still comes first.
    if (GetUnits(bot, Id(Aq40Npcs::NPC_GIANT_EYE_TENTACLE)).empty() && IsCthunTentacleKiller(bot))
    {
        Creature* best = nullptr;
        for (Creature* tentacle : GetUnits(bot, Id(Aq40Npcs::NPC_EYE_TENTACLE)))
            if (tentacle->GetExactDist2d(&CTHUN_STACK) > CTHUN_FAR_TENTACLE && IsAllowedDamage(bot, tentacle) &&
                (!best || bot->GetDistance(tentacle) < bot->GetDistance(best)))
                best = tentacle;
        if (best)
            return best;
    }
    bool melee = PlayerbotAI::IsMelee(bot) || tank;
    bool eyeMelee = IsCthunEyeMelee(bot);
    // While C'Thun is Weakened, a Giant Eye still dies first: left alone it kills more of the raid during the window
    // than the burn gains. A Giant Claw a tank holds is left to that tank (killing it first takes a large part of
    // the window); one nobody holds is killed first.
    bool weakened = IsCthunWeakened(bot);
    std::vector<uint32> priorities =
        weakened ? std::vector<uint32>{Id(Aq40Npcs::NPC_GIANT_EYE_TENTACLE), Id(Aq40Npcs::NPC_GIANT_CLAW_TENTACLE),
                                       Id(Aq40Npcs::NPC_CTHUN), Id(Aq40Npcs::NPC_EYE_TENTACLE),
                                       Id(Aq40Npcs::NPC_CLAW_TENTACLE)}
                 : std::vector<uint32>{Id(Aq40Npcs::NPC_GIANT_EYE_TENTACLE),  Id(Aq40Npcs::NPC_EYE_TENTACLE),
                                       Id(Aq40Npcs::NPC_GIANT_CLAW_TENTACLE), Id(Aq40Npcs::NPC_CLAW_TENTACLE),
                                       Id(Aq40Npcs::NPC_EYE_OF_CTHUN),        Id(Aq40Npcs::NPC_CTHUN)};
    bool phase2 = IsCthunPhase2(bot);
    for (uint32 entry : priorities)
    {
        Creature* best = nullptr;
        for (Creature* unit : GetUnits(bot, entry))
        {
            // Phase 2: a Giant Claw hits whoever it spawned under until its tank taunts it, so everyone near starts
            // on it at once instead of waiting for the tank.
            bool claw = entry == Id(Aq40Npcs::NPC_GIANT_CLAW_TENTACLE);
            if (!IsAllowedDamage(bot, unit) || (claw && !phase2 && !IsHeld(bot, unit)) ||
                (claw && phase2 && (!nearRaid(unit) || (weakened && IsHeld(bot, unit)))))
                continue;
            if (phase2)
            {
                if (entry == Id(Aq40Npcs::NPC_CTHUN))
                {
                    // Melee hit C'Thun only while he is Weakened; otherwise they leave him to the casters.
                    if (melee ? !unit->HasAura(Id(Aq40Spells::SPELL_PURPLE_COLORATION))
                              : bot->GetDistance(unit) > CTHUN_RANGED_REACH)
                        continue;
                }
                else if (melee)
                {
                    // From the stack, melee cover the near half of the Eye Tentacle ring (up to 14 alive at once).
                    if (!haveSpot || unit->GetExactDist2d(&spot) > CTHUN_STACK_MELEE_LEASH + unit->GetCombatReach())
                        continue;
                }
                else if (bot->GetDistance(unit) >
                         (entry == Id(Aq40Npcs::NPC_EYE_TENTACLE) || entry == Id(Aq40Npcs::NPC_GIANT_EYE_TENTACLE)
                              ? CTHUN_TENTACLE_CHASE
                              : CTHUN_RANGED_REACH))
                    continue;
            }
            else if (melee)
            {
                // Only the four Eye spots stand in the Eye's melee range; everyone else would stack on it.
                if (entry == Id(Aq40Npcs::NPC_EYE_OF_CTHUN) || entry == Id(Aq40Npcs::NPC_CTHUN))
                {
                    if (!eyeMelee)
                        continue;
                }
                else if (!haveSpot || unit->GetExactDist2d(&spot) > CTHUN_MELEE_LEASH + unit->GetCombatReach())
                    continue;
            }
            else if (bot->GetDistance(unit) > CTHUN_RANGED_REACH)
                continue;
            if (!best || bot->GetDistance(unit) < bot->GetDistance(best))
                best = unit;
        }
        if (best)
            return best;
    }
    return nullptr;
}

Position GlareDodge(PlayerbotAI* botAI, Unit* eye, float bearing, float distance)
{
    Player* bot = botAI->GetBot();
    Aq40BotCache& cache = BotCache(botAI);
    uint32 const now = getMSTime();
    if (cache.glareDodgeTime && getMSTimeDiff(cache.glareDodgeTime, now) < GLARE_DODGE_REFRESH_MS)
        return cache.glareDodge;
    float const direction = Aq40State(bot->GetInstanceId()).glareDirection;
    // Dodging at the bot's own distance carried outer-ring bots past the floor's edge, where they stayed stuck for
    // the rest of the fight. Step in until the dodge spot, a margin beyond it and the way there all have floor; the
    // escape bearing is recomputed for each radius, since the beam's clearance angle grows closer in.
    Position goal;
    float radius = std::max(GLARE_MIN_RADIUS, distance);
    while (true)
    {
        float escape =
            GlareEscapeBearing(bearing, radius, eye->GetOrientation(), direction, GLARE_HALF_WIDTH, GLARE_LOOKAHEAD);
        goal = Around(eye, radius, escape);
        Position beyond = Around(eye, radius + GLARE_FLOOR_MARGIN, escape);
        if (radius <= GLARE_MIN_RADIUS || (OnCthunFloor(bot, beyond.GetPositionX(), beyond.GetPositionY()) &&
                                           OnCthunFloor(bot, (bot->GetPositionX() + goal.GetPositionX()) / 2.0f,
                                                        (bot->GetPositionY() + goal.GetPositionY()) / 2.0f)))
            break;
        radius = std::max(GLARE_MIN_RADIUS, radius - GLARE_STEP_IN);
    }
    goal.m_positionZ = bot->GetPositionZ();
    cache.glareDodgeTime = now;
    cache.glareDodge = goal;
    return goal;
}

// A unit-centred hazard: step 5 yd straight away from the nearest matching unit inside the radius.
bool EscapeUnitHazard(PlayerbotAI* botAI, Position& goal, float radius, bool (*matches)(Unit*))
{
    Player* bot = botAI->GetBot();
    for (Creature* unit : GetUnits(bot))
        if (matches(unit) && bot->GetDistance2d(unit) < radius)
        {
            goal = Around(bot, 5.0f, unit->GetAngle(bot));
            return true;
        }
    return false;
}

enum class CthunGoalKind
{
    None,
    PullerHold,
    MeleeBurn,
    Spot,
    Stack
};

bool FormationPrelude(Player* bot, Aq40Encounter encounter)
{
    return IsInEncounter(bot, encounter) && GetBoss(bot) && !(encounter == Aq40Encounter::Cthun && InStomach(bot));
}

bool GetCthunGoal(PlayerbotAI* botAI, Position& goal, CthunGoalKind& kind)
{
    Player* bot = botAI->GetBot();
    kind = CthunGoalKind::None;
    if (!FormationPrelude(bot, Aq40Encounter::Cthun))
        return false;
    Aq40InstanceState const& state = Aq40State(bot->GetInstanceId());
    if (state.glareActive || !GetCthunSpot(bot, goal))
        return false;
    // The first three Eye Beams (3, 6 and 9 s) all go to the puller. He stays where he entered, alone, until the
    // raid has spread, or the opening beams chain through the raid still coming down the slope.
    if (bot->GetGUID() == state.cthunPuller && ElapsedSinceEncounterStart(bot) < CTHUN_PULLER_HOLD_MS)
    {
        // A Moon puller the Eye saw on his way in goes on to the pull spot while the opening beams land on him.
        if (Group* group = bot->GetGroup(); group && group->GetTargetIcon(CTHUN_PULLER_ICON) == bot->GetGUID())
        {
            kind = CthunGoalKind::PullerHold;
            goal = CTHUN_PULL_SPOT;
            return bot->GetExactDist2d(goal) > 3.0f;
        }
        return false;
    }
    bool phase2 = IsCthunPhase2(bot);
    // Melee may step off their spot to a tentacle next to it; they come back after.
    if (PlayerbotAI::IsTank(bot) || PlayerbotAI::IsMelee(bot))
    {
        if (Unit* target = GetAssignedTarget(botAI))
        {
            // Weakened burn: stand 8 yd from C'Thun's center on the bot's side of him.
            if (target->GetEntry() == Id(Aq40Npcs::NPC_CTHUN) && phase2)
            {
                kind = CthunGoalKind::MeleeBurn;
                goal = Around(target, CTHUN_BURN_MELEE_RADIUS, target->GetAngle(bot));
                return bot->GetExactDist2d(target) > CTHUN_BURN_MELEE_RADIUS + 1.0f;
            }
            if (target->GetEntry() != Id(Aq40Npcs::NPC_EYE_OF_CTHUN) && target->GetEntry() != Id(Aq40Npcs::NPC_CTHUN))
                return false;
        }
    }
    else if (phase2)
        if (Unit* target = GetAssignedTarget(botAI))
        {
            if (bot->GetDistance(target) > CTHUN_RANGED_REACH)
                return false;  // Walk into range of a far Eye Tentacle, then come back to the stack.
            // A tentacle killer stays where it can reach its far tentacle until it dies, instead of walking back to
            // the stack and out of range again.
            if (target->GetEntry() == Id(Aq40Npcs::NPC_EYE_TENTACLE) &&
                target->GetExactDist2d(&CTHUN_STACK) > CTHUN_FAR_TENTACLE && IsCthunTentacleKiller(bot))
                return false;
        }
    kind = phase2 ? CthunGoalKind::Stack : CthunGoalKind::Spot;
    return bot->GetExactDist2d(goal) > 3.0f;
}

bool GetCthunGoalOfKind(PlayerbotAI* botAI, Position& goal, CthunGoalKind wanted)
{
    CthunGoalKind kind;
    return GetCthunGoal(botAI, goal, kind) && kind == wanted;
}

// Twin Emperors roles: the platform tanks, the two warlocks, damage dealers (spare tanks included), healers.
bool IsTwinPlatformTankBot(Player* bot)
{
    Aq40InstanceState const& state = Aq40State(bot->GetInstanceId());
    return state.twinSides && (bot->GetGUID() == state.twinTanks[0] || bot->GetGUID() == state.twinTanks[1]);
}

bool IsTwinDamageRole(Player* bot)
{
    return !IsTwinPlatformTankBot(bot) && !IsTwinLock(bot, bot) && !PlayerbotAI::IsHeal(bot) &&
           (!PlayerbotAI::IsTank(bot) || IsTwinSpareTank(bot, bot));
}

}  // namespace

// Shared state

Aq40InstanceState& Aq40State(uint32 instanceId)
{
    std::lock_guard lock(aq40StateMutex);
    return aq40States[instanceId];
}

bool Aq40ResetInstance(uint32 instanceId)
{
    std::lock_guard lock(aq40StateMutex);
    return aq40States.erase(instanceId) > 0;
}

// General

Aq40Encounter GetCurrentEncounter(Player* bot)
{
    if (bot->GetMapId() != AQ40_MAP_ID)
        return Aq40Encounter::None;
    InstanceScript* instance = bot->GetInstanceScript();
    if (!instance)
        return Aq40Encounter::None;
    for (uint32 id = Id(Aq40Encounter::Skeram); id <= Id(Aq40Encounter::Cthun); ++id)
        if (instance->GetBossState(id) == IN_PROGRESS)
            return static_cast<Aq40Encounter>(id);
    return Aq40Encounter::None;
}

bool IsInEncounter(Player* bot, Aq40Encounter encounter) { return GetCurrentEncounter(bot) == encounter; }

bool InStomach(Player* player)
{
    return player->HasAura(Id(Aq40Spells::SPELL_DIGESTIVE_ACID)) || player->GetPositionZ() < CTHUN_STOMACH_Z;
}

bool Whirling(Unit* unit)
{
    return unit &&
           (unit->HasAura(Id(Aq40Spells::SPELL_WHIRLWIND)) || unit->HasAura(Id(Aq40Spells::SPELL_GUARD_WHIRLWIND)));
}

uint32 Stacks(Unit* unit, uint32 spell)
{
    Aura* aura = unit ? unit->GetAura(spell) : nullptr;
    return aura ? aura->GetStackAmount() : 0;
}

bool HasTrueFulfillment(Unit* unit) { return unit && unit->HasAura(Id(Aq40Spells::SPELL_TRUE_FULFILLMENT)); }

bool IsEncounterBoss(uint32 entry)
{
    switch (entry)
    {
        case Id(Aq40Npcs::NPC_SKERAM):
        case Id(Aq40Npcs::NPC_KRI):
        case Id(Aq40Npcs::NPC_YAUJ):
        case Id(Aq40Npcs::NPC_VEM):
        case Id(Aq40Npcs::NPC_SARTURA):
        case Id(Aq40Npcs::NPC_FANKRISS):
        case Id(Aq40Npcs::NPC_VISCIDUS):
        case Id(Aq40Npcs::NPC_HUHURAN):
        case Id(Aq40Npcs::NPC_VEKNILASH):
        case Id(Aq40Npcs::NPC_VEKLOR):
        case Id(Aq40Npcs::NPC_OURO):
        case Id(Aq40Npcs::NPC_EYE_OF_CTHUN):
        case Id(Aq40Npcs::NPC_CTHUN):
            return true;
        default:
            return false;
    }
}

bool IsTwinEmperor(Unit* unit)
{
    return unit && (unit->GetEntry() == Id(Aq40Npcs::NPC_VEKNILASH) || unit->GetEntry() == Id(Aq40Npcs::NPC_VEKLOR));
}

Position Around(WorldObject const* center, float radius, float angle)
{
    return Position(center->GetPositionX() + radius * std::cos(angle),
                    center->GetPositionY() + radius * std::sin(angle), center->GetPositionZ());
}

bool Attackable(Player* player, Unit* unit)
{
    return unit && unit->IsAlive() && unit->isTargetableForAttack() && player->IsValidAttackTarget(unit);
}

bool Passive(Unit* unit)
{
    Creature* creature = unit ? unit->ToCreature() : nullptr;
    return creature && creature->GetReactState() == REACT_PASSIVE;
}

void StopMoving(PlayerbotAI* botAI)
{
    botAI->GetBot()->GetMotionMaster()->Clear();
    botAI->GetBot()->StopMoving();
    botAI->GetAiObjectContext()->GetValue<LastMovement&>("last movement")->Get().clear();
}

void StopAttacking(Player* player)
{
    player->AttackStop();
    if (Pet* pet = player->GetPet())
    {
        pet->AttackStop();
        pet->GetMotionMaster()->MoveFollow(player, PET_FOLLOW_DIST, PET_FOLLOW_ANGLE);
    }
}

SpellInfo const* SpellFor(PlayerbotAI* botAI, std::string const& name)
{
    uint32 id = botAI->GetAiObjectContext()->GetValue<uint32>("spell id", name)->Get();
    return sSpellMgr->GetSpellInfo(id);
}

bool TryCast(PlayerbotAI* botAI, std::string const& name, Unit* target)
{
    Player* player = botAI->GetBot();
    SpellInfo const* spell = SpellFor(botAI, name);
    if (!spell || !target || player->HasUnitState(UNIT_STATE_CASTING) || !player->IsWithinLOSInMap(target) ||
        player->GetDistance(target) > spell->GetMaxRange(spell->IsPositive(), player) ||
        target->IsImmunedToSpell(spell))
        return false;
    if (player->isMoving() && spell->CalcCastTime())
        StopMoving(botAI);
    return botAI->CanCastSpell(spell->Id, target) && botAI->CastSpell(spell->Id, target);
}

std::string TauntSpell(Player* player)
{
    switch (player->getClass())
    {
        case CLASS_WARRIOR:
            return "taunt";
        case CLASS_DRUID:
            return "growl";
        case CLASS_PALADIN:
            return "hand of reckoning";
        case CLASS_DEATH_KNIGHT:
            return "dark command";
        default:
            return {};
    }
}

Aq40BotCache& BotCache(PlayerbotAI* botAI)
{
    return botAI->GetAiObjectContext()->GetValue<Aq40BotCache&>("aq40 bot cache")->Get();
}

std::vector<Player*> GetMembers(Player* bot, bool tanks)
{
    std::vector<Player*> result;
    if (Group* group = bot->GetGroup())
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
            if (Player* member = ref->GetSource())
                if (member->IsAlive() && member->IsInWorld() && member->GetMap() == bot->GetMap() &&
                    !HasTrueFulfillment(member) && (!tanks || PlayerbotAI::IsTank(member)))
                    result.push_back(member);
    std::sort(result.begin(), result.end(),
              [](Player* left, Player* right) { return left->GetGUID() < right->GetGUID(); });
    return result;
}

Player* GetMember(Player* bot, ObjectGuid guid)
{
    if (!guid)
        return nullptr;
    Player* player = ObjectAccessor::FindPlayer(guid);
    return player && player->IsAlive() && player->IsInWorld() && player->GetMap() == bot->GetMap() &&
                   player->GetGroup() == bot->GetGroup() && !HasTrueFulfillment(player)
               ? player
               : nullptr;
}

std::vector<Creature*> GetUnits(Player* bot, uint32 entry)
{
    std::vector<Creature*> result;
    for (ObjectGuid guid : Aq40State(bot->GetInstanceId()).units)
        if (Creature* unit = bot->GetMap()->GetCreature(guid))
            if (unit->IsAlive() && (!entry || unit->GetEntry() == entry))
                result.push_back(unit);
    return result;
}

Creature* GetBoss(Player* bot)
{
    std::vector<Creature*> units = GetUnits(bot);
    for (Creature* unit : units)
        if (IsEncounterBoss(unit->GetEntry()) && !unit->IsSummon())
            return unit;
    for (Creature* unit : units)
        if (IsEncounterBoss(unit->GetEntry()))
            return unit;
    return nullptr;
}

Creature* GetCthunBody(Player* bot)
{
    InstanceScript* instance = bot->GetInstanceScript();
    return instance ? instance->GetCreature(Id(Aq40Encounter::Cthun)) : nullptr;
}

Player* GetTankFor(Player* bot, Unit* unit)
{
    if (!unit)
        return nullptr;
    Aq40InstanceState const& state = Aq40State(bot->GetInstanceId());
    auto it = state.owners.find(unit->GetGUID());
    return it != state.owners.end() ? GetMember(bot, it->second) : nullptr;
}

bool IsHeld(Player* bot, Unit* unit)
{
    if (!unit || Passive(unit))
        return false;
    Player* owner = GetTankFor(bot, unit);
    // Any tank-role member already holding the unit counts (a human tank, a replacement after a blink, or the
    // previous tank awaiting a taunt), so the raid's damage never stalls on the assigned tank.
    if (Unit* victim = unit->GetVictim())
        if (Player* tank = victim->ToPlayer())
            if (GetMember(bot, tank->GetGUID()) && PlayerbotAI::IsTank(tank))
                owner = tank;
    if (!owner || unit->GetVictim() != owner || unit->GetThreatMgr().GetThreat(owner) <= 0.0f)
        return false;
    // Vek'lor keeps casting after knocking his tank out of melee. That is still a valid pickup; waiting for melee
    // contact here otherwise freezes every caster on its old platform.
    if (IsInEncounter(bot, Aq40Encounter::Twins) && unit->GetEntry() == Id(Aq40Npcs::NPC_VEKLOR))
        return owner == GetTankFor(bot, unit) && owner->GetDistance(unit) <= 45.0f;
    return owner->IsWithinMeleeRange(unit);
}

bool IsAllowedDamage(Player* bot, Unit* unit)
{
    if (!Attackable(bot, unit))
        return false;
    uint32 entry = unit->GetEntry();
    // Melee would stand in the whirlwind; ranged keep damaging it from outside.
    if (HasTrueFulfillment(unit) || (Whirling(unit) && !PlayerbotAI::IsRanged(bot)))
        return false;
    Aq40Encounter encounter = GetCurrentEncounter(bot);
    if (encounter == Aq40Encounter::Cthun)
    {
        bool inside = InStomach(bot);
        if ((entry == Id(Aq40Npcs::NPC_FLESH_TENTACLE)) != inside)
            return false;
        if (entry == Id(Aq40Npcs::NPC_CTHUN) && unit->HasAura(Id(Aq40Spells::SPELL_CARAPACE_CTHUN)))
            return false;
    }
    // The Eye is passive through Dark Glare but still takes damage, so players keep hitting it.
    if (IsEncounterBoss(entry) && Passive(unit) && entry != Id(Aq40Npcs::NPC_EYE_OF_CTHUN))
        return false;
    if (encounter == Aq40Encounter::Twins && !PlayerbotAI::IsTank(bot))
    {
        // Damage follows the emperor, not the platform: Vek'nilash is immune to magic and anyone in Vek'lor's melee
        // range triggers Arcane Burst. Melee and hunters take Vek'nilash, casters take Vek'lor, crossing on every swap.
        bool physical = IsPhysicalTwinDamage(bot);
        if ((entry == Id(Aq40Npcs::NPC_VEKLOR) && physical) || (entry == Id(Aq40Npcs::NPC_VEKNILASH) && !physical))
            return false;
    }
    if (encounter == Aq40Encounter::Twins &&
        (entry == Id(Aq40Npcs::NPC_QIRAJI_SCARAB) || entry == Id(Aq40Npcs::NPC_QIRAJI_SCORPION)))
    {
        if (unit->HasAura(Id(Aq40Spells::SPELL_EXPLODE_BUG)))
            return false;
        if (unit->HasAura(Id(Aq40Spells::SPELL_MUTATE_BUG)))
            return true;
        // Totems, Blizzard and other area damage also pull the room's bugs. Kill any bug already attacking the raid;
        // leave idle critters alone.
        Unit* victim = unit->GetVictim();
        Player* attacked = victim ? victim->GetCharmerOrOwnerPlayerOrPlayerItself() : nullptr;
        return attacked && GetMember(bot, attacked->GetGUID());
    }
    return entry != Id(Aq40Npcs::NPC_POISON_CLOUD) && entry != Id(Aq40Npcs::NPC_TOXIC_SLIME) &&
           entry != Id(Aq40Npcs::NPC_DIRT_MOUND) && entry != Id(Aq40Npcs::NPC_EXIT_TRIGGER);
}

Unit* ComputeAssignedTarget(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    if (HasTrueFulfillment(bot))
        return nullptr;
    switch (GetCurrentEncounter(bot))
    {
        case Aq40Encounter::Skeram:
            return GetSkeramTarget(bot);
        case Aq40Encounter::Trio:
            return GetBugTrioTarget(bot);
        case Aq40Encounter::Sartura:
            return GetSarturaTarget(bot);
        case Aq40Encounter::Viscidus:
            return GetViscidusTarget(bot);
        case Aq40Encounter::Huhuran:
            return GetHuhuranTarget(bot);
        case Aq40Encounter::Twins:
            return GetTwinEmperorsTarget(bot);
        case Aq40Encounter::Ouro:
            return GetOuroTarget(bot);
        case Aq40Encounter::Cthun:
            return InStomach(bot) ? GetCthunStomachTarget(bot) : GetCthunTarget(bot);
        default:
            return nullptr;
    }
}

Unit* GetAssignedTarget(PlayerbotAI* botAI)
{
    ObjectGuid guid = botAI->GetAiObjectContext()->GetValue<ObjectGuid>("aq40 assigned target")->Get();
    return guid ? botAI->GetUnit(guid) : nullptr;
}

// Mechanic tracker

bool IsActingTracker(Player* bot)
{
    Aq40InstanceState& state = Aq40State(bot->GetInstanceId());
    uint32 const now = getMSTime();
    if (state.tracker && state.tracker != bot->GetGUID() && getMSTimeDiff(state.trackerSeen, now) < TRACKER_INTERVAL_MS)
        return false;
    state.tracker = bot->GetGUID();
    state.trackerSeen = now;
    return true;
}

void TrackEncounter(Player* bot)
{
    InstanceScript* instance = bot->GetInstanceScript();
    Group* group = bot->GetGroup();
    if (!instance || !group)
        return;
    Aq40InstanceState& state = Aq40State(bot->GetInstanceId());
    Aq40Encounter current = GetCurrentEncounter(bot);
    if (current != state.encounter || state.group != group->GetGUID())
    {
        ObjectGuid const tracker = state.tracker;
        uint32 const trackerSeen = state.trackerSeen;
        state = Aq40InstanceState();
        state.tracker = tracker;
        state.trackerSeen = trackerSeen;
        state.encounter = current;
        state.group = group->GetGUID();
        state.encounterStart = getMSTime();
        state.twinLastSwap = state.encounterStart;
    }
    if (current == Aq40Encounter::None)
        return;
    // Instance boss GUID is the scan anchor, so a healer's visibility does not change assignments.
    WorldObject* anchor = bot;
    if (Creature* boss = instance->GetCreature(Id(current)))
        anchor = boss;
    else if (Creature* boss = GetBoss(bot))
        anchor = boss;
    state.units.clear();
    for (uint32 entry : EncounterEntries(current))
    {
        std::list<Creature*> found;
        anchor->GetCreatureListWithEntryInGrid(found, entry, ENCOUNTER_SCAN_DISTANCE);
        for (Creature* unit : found)
            if (unit->IsAlive())
                state.units.push_back(unit->GetGUID());
    }
    std::sort(state.units.begin(), state.units.end());
    std::vector<Player*> tanks = GetMembers(bot, true);
    if (!tanks.empty() && !GetMember(bot, state.mainTank))
    {
        auto main = std::find_if(tanks.begin(), tanks.end(), [](Player* p) { return PlayerbotAI::IsMainTank(p); });
        state.mainTank = (main == tanks.end() ? tanks.front() : *main)->GetGUID();
    }
}

void AssignAddTanks(Player* bot)
{
    Aq40Encounter encounter = GetCurrentEncounter(bot);
    if (encounter == Aq40Encounter::None || encounter == Aq40Encounter::Skeram || encounter == Aq40Encounter::Twins)
        return;
    std::vector<Player*> tanks = GetMembers(bot, true);
    if (tanks.empty())
        return;
    Aq40InstanceState& state = Aq40State(bot->GetInstanceId());
    std::map<ObjectGuid, uint32> loads;
    for (Player* tank : tanks)
        loads[tank->GetGUID()] = 0;
    for (auto it = state.owners.begin(); it != state.owners.end();)
        if (!GetMember(bot, it->second) || !std::binary_search(state.units.begin(), state.units.end(), it->first))
            it = state.owners.erase(it);
        else
        {
            ++loads[it->second];
            ++it;
        }
    for (Creature* unit : GetUnits(bot))
    {
        uint32 entry = unit->GetEntry();
        if (!Attackable(bot, unit) || entry == Id(Aq40Npcs::NPC_GLOB_OF_VISCIDUS) ||
            entry == Id(Aq40Npcs::NPC_FLESH_TENTACLE) || entry == Id(Aq40Npcs::NPC_EYE_TENTACLE) ||
            entry == Id(Aq40Npcs::NPC_GIANT_EYE_TENTACLE))
            continue;
        if (state.owners.count(unit->GetGUID()))
            continue;
        Player* owner = nullptr;
        bool primary = IsEncounterBoss(entry) && entry != Id(Aq40Npcs::NPC_YAUJ) && entry != Id(Aq40Npcs::NPC_VEM);
        if (primary)
            owner = GetMember(bot, state.mainTank);
        if (!owner)
            for (Player* tank : tanks)
            {
                if (tanks.size() > 1 && tank->GetGUID() == state.mainTank)
                    continue;
                if (!owner || loads[tank->GetGUID()] < loads[owner->GetGUID()] ||
                    (loads[tank->GetGUID()] == loads[owner->GetGUID()] &&
                     tank->GetDistance(unit) < owner->GetDistance(unit)))
                    owner = tank;
            }
        if (owner)
        {
            state.owners[unit->GetGUID()] = owner->GetGUID();
            ++loads[owner->GetGUID()];
        }
    }
}

bool SwapTanksOnStacks(Player* bot, uint32 aura, uint32 stacks)
{
    Creature* boss = GetBoss(bot);
    Player* owner = GetTankFor(bot, boss);
    if (!owner || Stacks(owner, aura) < stacks)
        return false;
    for (Player* tank : GetMembers(bot, true))
        if (tank != owner && Stacks(tank, aura) < stacks && tank->GetDistance(boss) < 30.0f)
        {
            Aq40InstanceState& state = Aq40State(bot->GetInstanceId());
            state.owners[boss->GetGUID()] = tank->GetGUID();
            state.mainTank = tank->GetGUID();
            // The new main tank hands its adds back; AssignAddTanks gives them to the off-tanks on its next pass.
            // Otherwise an owned worm outscores the boss in GetOwnedTarget and pulls the main tank off Fankriss.
            for (auto it = state.owners.begin(); it != state.owners.end();)
                if (it->second == tank->GetGUID() && it->first != boss->GetGUID())
                    it = state.owners.erase(it);
                else
                    ++it;
            return true;
        }
    return false;
}

void SwapFankrissTanks(Player* bot)
{
    SwapTanksOnStacks(bot, Id(Aq40Spells::SPELL_MORTAL_WOUND), FANKRISS_SWAP_STACKS);
}

bool UsesAssignedTarget(Player* bot) { return !IsInEncounter(bot, Aq40Encounter::Fankriss); }

// Fankriss the Unyielding

Creature* FindFankrissKillTarget(Player* bot)
{
    // A worm becomes the kill target once any tank has it, so damage never waits on the tank reaching melee.
    for (Creature* worm : GetUnits(bot, Id(Aq40Npcs::NPC_SPAWN_OF_FANKRISS)))
        if (Unit* victim = worm->GetVictim())
            if (Player* tank = victim->ToPlayer(); tank && GetMember(bot, tank->GetGUID()) && PlayerbotAI::IsTank(tank))
                return worm;
    std::vector<Creature*> const boss = GetUnits(bot, Id(Aq40Npcs::NPC_FANKRISS));
    return boss.empty() ? nullptr : boss.front();
}

Creature* FindFankrissOwnedSpawn(Player* bot)
{
    // Worms hit far harder than hatchlings, so an off-tank holding both keeps its worm.
    Creature* selected = nullptr;
    for (Creature* unit : GetUnits(bot))
    {
        uint32 const entry = unit->GetEntry();
        if ((entry != Id(Aq40Npcs::NPC_SPAWN_OF_FANKRISS) && entry != Id(Aq40Npcs::NPC_VEKNISS_HATCHLING)) ||
            GetTankFor(bot, unit) != bot || !Attackable(bot, unit))
            continue;
        if (entry == Id(Aq40Npcs::NPC_SPAWN_OF_FANKRISS))
            return unit;
        if (!selected)
            selected = unit;
    }
    return selected;
}

void SwapHuhuranTanks(Player* bot) { SwapTanksOnStacks(bot, Id(Aq40Spells::SPELL_ACID_SPIT), HUHURAN_SWAP_STACKS); }

// The Prophet Skeram

void AssignSkeramPlatforms(Player* bot)
{
    constexpr std::array<Aq40Spells, 3> blinks = {Aq40Spells::SPELL_SKERAM_BLINK_1, Aq40Spells::SPELL_SKERAM_BLINK_2,
                                                  Aq40Spells::SPELL_SKERAM_BLINK_3};
    Aq40InstanceState& state = Aq40State(bot->GetInstanceId());
    std::vector<Player*> tanks = GetMembers(bot, true);
    if (tanks.empty())
        return;
    if (!state.skeramPlatforms)
    {
        std::array<Position, 3> found;
        for (uint32 index = 0; index < blinks.size(); ++index)
        {
            SpellTargetPosition const* point = sSpellMgr->GetSpellTargetPosition(Id(blinks[index]), EFFECT_0);
            if (!point)
                return;
            found[index].Relocate(point->target_X, point->target_Y, point->target_Z);
        }
        state.skeramPlatforms = found;
        state.skeramPlatformTanks[SKERAM_CENTER_PLATFORM] = state.mainTank;
    }
    std::array<Position, 3> const& platforms = *state.skeramPlatforms;

    std::set<ObjectGuid> assigned;
    for (ObjectGuid& guid : state.skeramPlatformTanks)
        if (GetMember(bot, guid))
            assigned.insert(guid);
        else
            guid.Clear();
    for (uint32 index : {uint32(SKERAM_CENTER_PLATFORM), 0u, 2u})
    {
        if (!state.skeramPlatformTanks[index])
        {
            Player* nearest = nullptr;
            for (Player* tank : tanks)
                if (!assigned.count(tank->GetGUID()) &&
                    (!nearest || tank->GetExactDist(platforms[index]) < nearest->GetExactDist(platforms[index])))
                    nearest = tank;
            if (nearest)
            {
                state.skeramPlatformTanks[index] = nearest->GetGUID();
                assigned.insert(nearest->GetGUID());
            }
        }
        if (state.skeramPlatformTanks[index])
            state.skeramMemberPlatform[state.skeramPlatformTanks[index]] = index;
    }

    // Balance healers, mages and rogues separately, so every platform gets healing and interrupts.
    std::vector<Player*> members = GetMembers(bot);
    auto role = [](Player* player)
    {
        return PlayerbotAI::IsHeal(player)         ? 0u
               : player->getClass() == CLASS_MAGE  ? 1u
               : player->getClass() == CLASS_ROGUE ? 2u
                                                   : 3u;
    };
    std::array<std::array<uint32, 3>, 4> loads{};
    for (Player* member : members)
        if (!assigned.count(member->GetGUID()))
            if (auto it = state.skeramMemberPlatform.find(member->GetGUID()); it != state.skeramMemberPlatform.end())
                ++loads[role(member)][it->second];
    for (Player* member : members)
    {
        if (assigned.count(member->GetGUID()) || state.skeramMemberPlatform.count(member->GetGUID()))
            continue;
        auto& counts = loads[role(member)];
        uint32 side = SKERAM_CENTER_PLATFORM;
        for (uint32 index : {0u, 2u})
            if (counts[index] < counts[side])
                side = index;
        state.skeramMemberPlatform[member->GetGUID()] = side;
        ++counts[side];
    }

    state.owners.clear();
    for (Creature* unit : GetUnits(bot, Id(Aq40Npcs::NPC_SKERAM)))
    {
        ObjectGuid guid = unit->GetGUID();
        if (Passive(unit))
        {
            // Images are initially summoned on the original, then teleported to their own platforms.
            state.skeramImagePlatform.erase(guid);
            state.skeramImagePosition.erase(guid);
            continue;
        }
        auto previous = state.skeramImagePosition.find(guid);
        if (!state.skeramImagePlatform.count(guid) || previous == state.skeramImagePosition.end() ||
            unit->GetExactDist(previous->second) > SKERAM_IMAGE_TELEPORT_DISTANCE)
        {
            uint32 side = SKERAM_CENTER_PLATFORM;
            for (uint32 index : {0u, 2u})
                if (unit->GetExactDist(platforms[index]) < unit->GetExactDist(platforms[side]))
                    side = index;
            state.skeramImagePlatform[guid] = side;
        }
        // Chasing a player down the stairs must not reassign the body to a different platform's tank.
        state.skeramImagePosition[guid] = unit->GetPosition();
        state.owners[guid] = state.skeramPlatformTanks[state.skeramImagePlatform[guid]];
    }
}

bool GetSkeramPlatform(PlayerbotAI* botAI, Position& platform)
{
    Player* bot = botAI->GetBot();
    Aq40InstanceState const& state = Aq40State(bot->GetInstanceId());
    auto it = state.skeramMemberPlatform.find(bot->GetGUID());
    if (!IsInEncounter(bot, Aq40Encounter::Skeram) || !state.skeramPlatforms || it == state.skeramMemberPlatform.end())
        return false;
    uint32 side = it->second;
    if (!PlayerbotAI::IsHeal(bot))
    {
        bool tank = PlayerbotAI::IsTank(bot);
        Unit* target = GetAssignedTarget(botAI);
        if (!target && !tank)
        {
            // The boss can briefly leave melee reach while following its tank to center. Keep DPS moving to center
            // during that damage hold instead of sending it back upstairs.
            if (Creature* original = GetBoss(bot))
                if (!original->IsSummon() && IsAllowedDamage(bot, original))
                    if (Player* owner = GetTankFor(bot, original))
                        if (original->GetVictim() == owner && original->GetThreatMgr().GetThreat(owner) > 0.0f)
                            target = original;
        }
        if (target)
        {
            Player* owner = GetTankFor(bot, target);
            if (!target->IsSummon() && owner && (!tank || owner == bot))
            {
                bool acquired = target->GetVictim() == owner && target->GetThreatMgr().GetThreat(owner) > 0.0f;
                // Reach and pick up the real boss before bringing it back. Images stay on their platforms.
                if (tank || acquired)
                {
                    platform = acquired ? (*state.skeramPlatforms)[SKERAM_CENTER_PLATFORM] : target->GetPosition();
                    return true;
                }
            }
            // DPS joins an image's platform only after its assist tank has picked it up.
            if (!tank)
                if (auto body = state.skeramImagePlatform.find(target->GetGUID());
                    body != state.skeramImagePlatform.end())
                    side = body->second;
        }
    }
    platform = (*state.skeramPlatforms)[side];
    return true;
}

Player* GetMindControlledMember(Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        if (Player* member = ref->GetSource())
            if (member->IsAlive() && member->GetMap() == bot->GetMap() && HasTrueFulfillment(member) &&
                !member->HasAuraType(SPELL_AURA_MOD_CONFUSE) && !member->HasAuraType(SPELL_AURA_MOD_STUN))
                return member;
    return nullptr;
}

// Battleguard Sartura

Creature* GetWhirlwindInRange(Player* bot)
{
    if (!IsInEncounter(bot, Aq40Encounter::Sartura))
        return nullptr;
    for (Creature* unit : GetUnits(bot))
        if (Whirling(unit) && bot->GetDistance2d(unit) < WHIRLWIND_SAFE_DISTANCE)
            return unit;
    return nullptr;
}

std::vector<Creature*> GetSarturaKillOrder(Player* bot)
{
    // Guards by spawn id, so every bot and every pull agree on the order, then Sartura.
    std::vector<Creature*> order = GetUnits(bot, Id(Aq40Npcs::NPC_SARTURA_ROYAL_GUARD));
    std::sort(order.begin(), order.end(),
              [](Creature* left, Creature* right) { return left->GetSpawnId() < right->GetSpawnId(); });
    for (Creature* sartura : GetUnits(bot, Id(Aq40Npcs::NPC_SARTURA)))
        order.push_back(sartura);
    // Whoever set the skull, the marked unit dies first.
    if (Group* group = bot->GetGroup())
    {
        ObjectGuid const skull = group->GetTargetIcon(SKULL_ICON);
        auto marked =
            std::find_if(order.begin(), order.end(), [&](Creature* unit) { return unit->GetGUID() == skull; });
        if (marked != order.end())
            std::rotate(order.begin(), marked, marked + 1);
    }
    return order;
}

// Princess Huhuran

void AssignHuhuranSoakers(Player* bot)
{
    Aq40InstanceState& state = Aq40State(bot->GetInstanceId());
    state.huhuranSoakers.erase(std::remove_if(state.huhuranSoakers.begin(), state.huhuranSoakers.end(),
                                              [&](ObjectGuid guid) { return !GetMember(bot, guid); }),
                               state.huhuranSoakers.end());
    SpellInfo const* spell = sSpellMgr->GetSpellInfo(Id(Aq40Spells::SPELL_POISON_BOLT));
    uint32 count = spell ? spell->MaxAffectedTargets : 0;
    std::vector<Player*> members = GetMembers(bot);
    std::stable_sort(
        members.begin(), members.end(),
        [](Player* left, Player* right)
        {
            auto rank = [](Player* p) {
                return PlayerbotAI::IsTank(p) ? 0 : PlayerbotAI::IsHeal(p) ? 3 : PlayerbotAI::IsMelee(p) ? 1 : 2;
            };
            return rank(left) < rank(right);
        });
    for (Player* member : members)
        if (state.huhuranSoakers.size() < count && std::find(state.huhuranSoakers.begin(), state.huhuranSoakers.end(),
                                                             member->GetGUID()) == state.huhuranSoakers.end())
            state.huhuranSoakers.push_back(member->GetGUID());
}

bool IsHuhuranSoaker(Player* bot)
{
    GuidVector const& soakers = Aq40State(bot->GetInstanceId()).huhuranSoakers;
    return std::find(soakers.begin(), soakers.end(), bot->GetGUID()) != soakers.end();
}

Player* GetWyvernStungTank(Player* bot)
{
    for (Player* member : GetMembers(bot, true))
        if (member->HasAura(Id(Aq40Spells::SPELL_WYVERN_STING)) && member->GetHealth() > WYVERN_STING_SAFE_HEALTH)
            return member;
    return nullptr;
}

// Viscidus

namespace
{

// The spot on a circle around the boss nearest to the given angle that keeps clear of every Toxin cloud. Clouds land on
// the raid and never despawn, so the circle fills up; a spot inside one has the move-to-spot step and the cloud escape
// pull the bot back and forth all fight. When no spot is fully clear, take the one farthest from any cloud.
Position ViscidusClearSpot(Player* player, Creature* boss, float radius, float angle, bool avoidRamp)
{
    std::vector<Creature*> const clouds = GetUnits(player, Id(Aq40Npcs::NPC_TOXIC_SLIME));
    auto clearance = [&clouds](Position const& spot)
    {
        float nearest = std::numeric_limits<float>::max();
        for (Creature* cloud : clouds)
            nearest = std::min(nearest, cloud->GetExactDist2d(&spot));
        return nearest;
    };
    Position best = Around(boss, radius, angle);
    float bestClearance = -1.0f;
    for (float turn = 0.0f; turn <= float(M_PI); turn += VISCIDUS_SPOT_SEARCH_STEP)
    {
        for (float side : {1.0f, -1.0f})
        {
            Position candidate = Around(boss, radius, angle + side * turn);
            if (avoidRamp && candidate.GetPositionY() > VISCIDUS_RAMP_Y)
                continue;
            float candidateClearance = clearance(candidate);
            if (candidateClearance >= TOXIN_HAZARD_RADIUS)
                return candidate;
            if (candidateClearance > bestClearance)
            {
                best = candidate;
                bestClearance = candidateClearance;
            }
            if (turn == 0.0f)
                break;
        }
    }
    return best;
}

}  // namespace

Position ViscidusRing(Player* player, Creature* boss)
{
    float ring = boss->GetCombatReach() + VISCIDUS_RING_OFFSET;
    float angle = boss->GetAngle(player);
    // The boss is usually held near the entrance, so the slot behind a ramp-bound bot is up the ramp. Fan those bots
    // out on the room-center side of the boss instead.
    if (Around(boss, ring, angle).GetPositionY() > VISCIDUS_RAMP_Y || player->GetPositionZ() > VISCIDUS_RAMP_Z)
        angle = boss->GetAngle(&VISCIDUS_ROOM_CENTER) + (float(player->GetGUID().GetCounter() % 5) - 2.0f) * 0.35f;
    return ViscidusClearSpot(player, boss, ring, angle, true);
}

bool ViscidusShouldBeFrozen(Player* bot)
{
    if (!IsInEncounter(bot, Aq40Encounter::Viscidus) || PlayerbotAI::IsHeal(bot))
        return false;
    Creature* boss = GetBoss(bot);
    return boss && GetUnits(bot, Id(Aq40Npcs::NPC_GLOB_OF_VISCIDUS)).empty() &&
           !boss->HasAura(Id(Aq40Spells::SPELL_VISCIDUS_FREEZE)) && !boss->HasAura(Id(Aq40Spells::SPELL_INVIS_SELF));
}

bool IsNearToxin(Player* player, Position const& spot, float radius)
{
    for (Creature* cloud : GetUnits(player, Id(Aq40Npcs::NPC_TOXIC_SLIME)))
        if (cloud->GetExactDist2d(&spot) < radius)
            return true;
    return false;
}

float ToxinTankAvoidRadius(Creature* boss) { return TOXIN_HAZARD_RADIUS + boss->GetCombatReach(); }

namespace
{

float SegmentLengthInCircle(Position const& from, Position const& to, Position const& center, float radius)
{
    float const dx = to.GetPositionX() - from.GetPositionX();
    float const dy = to.GetPositionY() - from.GetPositionY();
    float const fx = from.GetPositionX() - center.GetPositionX();
    float const fy = from.GetPositionY() - center.GetPositionY();
    float const a = dx * dx + dy * dy;
    if (a < 0.0001f)
        return 0.0f;
    float const b = 2.0f * (fx * dx + fy * dy);
    float const c = fx * fx + fy * fy - radius * radius;
    float const discriminant = b * b - 4.0f * a * c;
    if (discriminant <= 0.0f)
        return 0.0f;
    float const root = std::sqrt(discriminant);
    float const t0 = std::clamp((-b - root) / (2.0f * a), 0.0f, 1.0f);
    float const t1 = std::clamp((-b + root) / (2.0f * a), 0.0f, 1.0f);
    return (t1 - t0) * std::sqrt(a);
}

}  // namespace

bool FindViscidusTankClearSpot(Player* bot, Creature* boss, Position& spot)
{
    std::vector<Creature*> const clouds = GetUnits(bot, Id(Aq40Npcs::NPC_TOXIC_SLIME));
    Position const from = bot->GetPosition();
    // Fully clear for the boss first; if the room is too full for that, at least clear for the tank.
    for (float clearRadius : {ToxinTankAvoidRadius(boss), TOXIN_HAZARD_RADIUS})
    {
        std::vector<std::pair<float, Position>> candidates;
        for (uint8 ring = 1; ring <= TOXIN_TANK_SEARCH_RINGS; ++ring)
        {
            float const radius = TOXIN_TANK_SEARCH_RING_SPACING * ring;
            for (uint8 i = 0; i < TOXIN_TANK_SEARCH_DIRECTIONS; ++i)
            {
                float const angle = 2.0f * static_cast<float>(M_PI) * i / TOXIN_TANK_SEARCH_DIRECTIONS;
                Position const candidate(from.GetPositionX() + std::cos(angle) * radius,
                                         from.GetPositionY() + std::sin(angle) * radius, from.GetPositionZ());
                if (candidate.GetPositionY() > VISCIDUS_RAMP_Y || IsNearToxin(bot, candidate, clearRadius))
                    continue;
                float inClouds = 0.0f;
                for (Creature* cloud : clouds)
                    inClouds += SegmentLengthInCircle(from, candidate, cloud->GetPosition(), TOXIN_RADIUS);
                candidates.emplace_back(radius + TOXIN_TANK_CLOUD_YARD_COST * inClouds, candidate);
            }
        }
        std::sort(candidates.begin(), candidates.end(), [](auto const& a, auto const& b) { return a.first < b.first; });
        uint8 checked = 0;
        for (auto const& [cost, candidate] : candidates)
        {
            if (++checked > TOXIN_TANK_PATH_CHECKS)
                break;
            PathGenerator path(bot);
            if (path.CalculatePath(candidate.GetPositionX(), candidate.GetPositionY(), candidate.GetPositionZ()) &&
                path.GetPathType() == PATHFIND_NORMAL)
            {
                spot = candidate;
                return true;
            }
        }
    }
    return false;
}

bool GetViscidusMeleeStepClearOfToxin(Player* bot, Unit* target, float& stepX, float& stepY, float& stepZ)
{
    if (!target || (bot->IsWithinMeleeRange(target) && !IsNearToxin(bot, bot->GetPosition(), TOXIN_HAZARD_RADIUS)))
        return false;
    float const meleeRange = bot->GetMeleeRange(target);
    float const ringRadius = meleeRange - TOXIN_MELEE_RANGE_INSET;
    std::vector<Creature*> nearby;
    for (Creature* cloud : GetUnits(bot, Id(Aq40Npcs::NPC_TOXIC_SLIME)))
        if (cloud->GetExactDist2d(target) < meleeRange + TOXIN_HAZARD_RADIUS)
            nearby.push_back(cloud);
    auto isClear = [&nearby](float x, float y)
    {
        return std::none_of(nearby.begin(), nearby.end(),
                            [x, y](Creature* cloud) { return cloud->GetExactDist2d(x, y) < TOXIN_HAZARD_RADIUS; });
    };
    float const botAngle = target->GetAngle(bot);
    constexpr uint8 samplesPerSide = 36;
    constexpr float sampleAngle = static_cast<float>(M_PI) / samplesPerSide;
    for (uint8 i = 0; i <= samplesPerSide; ++i)
    {
        for (int8 side = 1; side >= -1; side -= 2)
        {
            if ((i == 0 || i == samplesPerSide) && side < 0)
                continue;
            float const angle = botAngle + side * sampleAngle * i;
            float const x = target->GetPositionX() + std::cos(angle) * ringRadius;
            float const y = target->GetPositionY() + std::sin(angle) * ringRadius;
            if (!isClear(x, y))
                continue;
            constexpr float arrivalDistance = 0.5f;
            if (bot->GetExactDist2d(x, y) <= arrivalDistance)
                return false;
            if (EncounterHelpers::CanTakeStepTowards(bot, x, y, PATH_STEP_DISTANCE, stepX, stepY, stepZ))
                return true;
        }
    }
    return false;
}

bool GetStepOutOfNearestToxin(Player* bot, float& stepX, float& stepY, float& stepZ)
{
    std::vector<Creature*> const clouds = GetUnits(bot, Id(Aq40Npcs::NPC_TOXIC_SLIME));
    auto const nearest = std::min_element(clouds.begin(), clouds.end(), [bot](Creature* a, Creature* b)
                                          { return bot->GetExactDist2dSq(a) < bot->GetExactDist2dSq(b); });
    if (nearest == clouds.end())
        return false;
    Creature* cloud = *nearest;
    float const botAngle = bot->GetExactDist2d(cloud) > 0.1f ? cloud->GetAngle(bot) : bot->GetOrientation();
    constexpr uint8 samplesPerSide = 36;
    constexpr float sampleAngle = static_cast<float>(M_PI) / samplesPerSide;
    for (uint8 i = 0; i <= samplesPerSide; ++i)
    {
        for (int8 side = 1; side >= -1; side -= 2)
        {
            if ((i == 0 || i == samplesPerSide) && side < 0)
                continue;
            float const angle = botAngle + side * sampleAngle * i;
            float const x = cloud->GetPositionX() + std::cos(angle) * TOXIN_HAZARD_RADIUS;
            float const y = cloud->GetPositionY() + std::sin(angle) * TOXIN_HAZARD_RADIUS;
            if (EncounterHelpers::CanTakeStepTowards(bot, x, y, PATH_STEP_DISTANCE, stepX, stepY, stepZ))
                return true;
        }
    }
    return false;
}

bool HasFrostWeaponProc(Player* bot)
{
    for (uint8 slot : {EQUIPMENT_SLOT_MAINHAND, EQUIPMENT_SLOT_OFFHAND})
    {
        Item* weapon = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slot);
        if (!weapon)
            continue;
        for (EnchantmentSlot enchantSlot : {PERM_ENCHANTMENT_SLOT, TEMP_ENCHANTMENT_SLOT})
        {
            SpellItemEnchantmentEntry const* enchant =
                sSpellItemEnchantmentStore.LookupEntry(weapon->GetEnchantmentId(enchantSlot));
            if (!enchant)
                continue;
            for (uint8 i = 0; i < MAX_SPELL_ITEM_ENCHANTMENT_EFFECTS; ++i)
            {
                if (enchant->type[i] != ITEM_ENCHANTMENT_TYPE_COMBAT_SPELL)
                    continue;
                SpellInfo const* proc = sSpellMgr->GetSpellInfo(enchant->spellid[i]);
                if (proc && (proc->GetSchoolMask() & SPELL_SCHOOL_MASK_FROST))
                    return true;
            }
        }
    }
    return false;
}

bool ViscidusMeleeWaitsForFreeze(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    if (!FormationPrelude(bot, Aq40Encounter::Viscidus) || PlayerbotAI::IsTank(bot) || PlayerbotAI::IsHeal(bot) ||
        PlayerbotAI::IsRanged(bot) || HasFrostWeaponProc(bot))
        return false;
    Creature* boss = GetBoss(bot);
    if (boss->HasAura(Id(Aq40Spells::SPELL_VISCIDUS_FREEZE)))
        return false;  // Frozen: everyone moves in to shatter.
    // Globs are fair game for everyone.
    Unit* target = GetAssignedTarget(botAI);
    return !target || target == boss;
}

bool IsViscidusRingMelee(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    if (!FormationPrelude(bot, Aq40Encounter::Viscidus) || PlayerbotAI::IsTank(bot) || PlayerbotAI::IsHeal(bot) ||
        PlayerbotAI::IsRanged(bot))
        return false;
    Creature* boss = GetBoss(bot);
    // Globs are fought with the normal melee movement.
    if (GetAssignedTarget(botAI) != boss)
        return false;
    return boss->HasAura(Id(Aq40Spells::SPELL_VISCIDUS_FREEZE)) || HasFrostWeaponProc(bot);
}

bool HasViscidusWaterTotem(Player* bot)
{
    constexpr float totemRange = 30.0f;
    Creature* totem = bot->GetMap()->GetCreature(bot->m_SummonSlot[SUMMON_SLOT_TOTEM_WATER]);
    if (!totem || totem->GetDistance(bot) > totemRange)
        return false;
    uint32 const spell = totem->GetUInt32Value(UNIT_CREATED_BY_SPELL);
    return spell == Id(Aq40Spells::SPELL_CLEANSING_TOTEM) ||
           std::find(std::begin(MANA_TIDE_TOTEM), std::end(MANA_TIDE_TOTEM), spell) != std::end(MANA_TIDE_TOTEM);
}

bool ViscidusShamanNeedsCleansingTotem(Player* bot)
{
    uint32 const cleansing = Id(Aq40Spells::SPELL_CLEANSING_TOTEM);
    if (bot->getClass() != CLASS_SHAMAN || !bot->HasSpell(cleansing))
        return false;
    // Call of the Elements drops whatever the bar's water slot holds, so the bar must hold Cleansing too.
    ActionButton const* water = bot->GetActionButton(TOTEM_BAR_SLOT_WATER);
    if (!water || water->GetType() != ACTION_BUTTON_SPELL || water->GetAction() != cleansing)
        return true;
    return !HasViscidusWaterTotem(bot);
}

bool ViscidusMeleeNeedsFrostOil(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    return botAI->HasCheat(BotCheatMask::raid) && IsInEncounter(bot, Aq40Encounter::Viscidus) &&
           !PlayerbotAI::IsHeal(bot) && !PlayerbotAI::IsRanged(bot) &&
           bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND) && !HasFrostWeaponProc(bot);
}

// Twin Emperors

void AssignTwinRoles(Player* bot)
{
    Aq40InstanceState& state = Aq40State(bot->GetInstanceId());
    std::vector<Player*> tanks = GetMembers(bot, true);
    std::vector<Creature*> physical = GetUnits(bot, Id(Aq40Npcs::NPC_VEKNILASH));
    std::vector<Creature*> caster = GetUnits(bot, Id(Aq40Npcs::NPC_VEKLOR));
    if (!state.twinSides && !physical.empty() && !caster.empty())
    {
        // Side 0 is Vek'nilash's spawn side. Sides use the floor spots: the emperors move at engage (Vek'lor walks
        // toward targets beyond 45 yd), and the pedestal stairs break their pathing so they heal to full.
        std::array<Position, 2> spots = TWIN_FLOOR_SPOTS;
        if (physical.front()->GetHomePosition().GetExactDist2d(&spots[0]) >
            physical.front()->GetHomePosition().GetExactDist2d(&spots[1]))
            std::swap(spots[0], spots[1]);
        state.twinSides = spots;
        // The raid is stacked on its leader at the pull, so proximity puts nearly everyone on one emperor. Deal each
        // role out alternately instead, so both sides get healers, melee and casters.
        std::array<uint32, 3> dealt{};
        for (Player* member : GetMembers(bot))
        {
            if (PlayerbotAI::IsTank(member))
                continue;
            uint32 role = PlayerbotAI::IsHeal(member) ? 0 : PlayerbotAI::IsMelee(member) ? 1 : 2;
            state.twinMemberSide[member->GetGUID()] = dealt[role]++ % 2;
        }
    }
    if (!state.twinSides)
        return;
    std::array<Position, 2> const& sides = *state.twinSides;
    for (Player* member : GetMembers(bot))
        state.twinMemberSide.try_emplace(member->GetGUID(),
                                         member->GetExactDist2d(sides[0]) < member->GetExactDist2d(sides[1]) ? 0 : 1);
    // Vek'lor is not physically immune on this core, so one plate tank per platform holds whichever emperor lands
    // there and taunts on every swap. Warlocks are ordinary casters.
    auto assignSides = [&](std::array<ObjectGuid, 2>& owners, std::vector<Player*> const& candidates)
    {
        for (uint32 side = 0; side < 2; ++side)
        {
            if (GetMember(bot, owners[side]))
                continue;
            owners[side].Clear();
            Player* nearest = nullptr;
            for (Player* candidate : candidates)
                if (candidate->GetGUID() != owners[1 - side] &&
                    (!nearest || candidate->GetExactDist2d(sides[side]) < nearest->GetExactDist2d(sides[side])))
                    nearest = candidate;
            if (nearest)
                owners[side] = nearest->GetGUID();
        }
    };
    // Only bots take the platform roles. A human tank-spec player picked by proximity would own an emperor the bots
    // then never pick up.
    std::vector<Player*> botTanks;
    for (Player* tank : tanks)
        if (GET_PLAYERBOT_AI(tank))
            botTanks.push_back(tank);
    // Plate tanks with the most health take the platforms; other tank-spec bots (feral druid, arms/prot warrior)
    // only replace a dead platform tank, rather than whoever stood nearest at the first refresh.
    auto tankPoints = [](Player* tank)
    {
        uint8 tab = tank->getClass() == CLASS_WARRIOR ? 2 : tank->getClass() == CLASS_DEATH_KNIGHT ? 0 : 1;
        auto tabs = AiFactory::GetPlayerSpecTabs(tank);
        auto found = tabs.find(tab);
        return found == tabs.end() ? 0u : found->second;
    };
    std::stable_sort(botTanks.begin(), botTanks.end(),
                     [&](Player* left, Player* right)
                     {
                         bool leftPlate = left->getClass() != CLASS_DRUID;
                         bool rightPlate = right->getClass() != CLASS_DRUID;
                         if (leftPlate != rightPlate)
                             return leftPlate;
                         uint32 leftPoints = tankPoints(left), rightPoints = tankPoints(right);
                         if (leftPoints != rightPoints)
                             return leftPoints > rightPoints;
                         return left->GetMaxHealth() > right->GetMaxHealth();
                     });
    if (!GetMember(bot, state.twinTanks[0]) && !GetMember(bot, state.twinTanks[1]) && botTanks.size() >= 2)
    {
        Player* first = botTanks[0];
        Player* second = botTanks[1];
        float straight = first->GetExactDist2d(sides[0]) + second->GetExactDist2d(sides[1]);
        float crossed = first->GetExactDist2d(sides[1]) + second->GetExactDist2d(sides[0]);
        state.twinTanks = straight <= crossed ? std::array<ObjectGuid, 2>{first->GetGUID(), second->GetGUID()}
                                              : std::array<ObjectGuid, 2>{second->GetGUID(), first->GetGUID()};
    }
    // A spare tank Vek'nilash is hitting takes over a platform: he can't be taunted back, and mid-room the emperors
    // heal each other. A vacant side goes first, else the side not on Vek'lor.
    if (!physical.empty() && !caster.empty())
        if (Unit* victim = physical.front()->GetVictim())
            if (Player* held = victim->ToPlayer();
                held && std::find(botTanks.begin(), botTanks.end(), held) != botTanks.end() &&
                held->GetGUID() != state.twinTanks[0] && held->GetGUID() != state.twinTanks[1])
            {
                uint32 replace = !GetMember(bot, state.twinTanks[0]) ? 0 : !GetMember(bot, state.twinTanks[1]) ? 1 : 2;
                for (uint32 side = 0; side < 2 && replace == 2; ++side)
                    if (caster.front()->GetVictim() != GetMember(bot, state.twinTanks[side]))
                        replace = side;
                if (replace < 2)
                    state.twinTanks[replace] = held->GetGUID();
            }
    assignSides(state.twinTanks, botTanks);
    for (uint32 side = 0; side < 2; ++side)
        if (state.twinTanks[side])
            state.twinMemberSide[state.twinTanks[side]] = side;
    if (physical.empty() || caster.empty())
        return;
    // Match both emperors together. A displaced boss must not give one tank both owners and leave the opposite
    // platform without a pickup assignment.
    float original = physical.front()->GetExactDist2d(sides[0]) + caster.front()->GetExactDist2d(sides[1]);
    float swapped = physical.front()->GetExactDist2d(sides[1]) + caster.front()->GetExactDist2d(sides[0]);
    // Keep the matching stable while both emperors are off their platforms: flipping owners on small moves hands
    // both to one tank. Re-match after a teleport, for a new or dead owner, or if clearly better.
    uint32 physicalSide = original <= swapped ? 0 : 1;
    if (state.twinLast && physical.front()->GetExactDist2d((*state.twinLast)[0]) < TWIN_TELEPORT_JUMP &&
        caster.front()->GetExactDist2d((*state.twinLast)[1]) < TWIN_TELEPORT_JUMP &&
        std::abs(original - swapped) < 40.0f)
        for (uint32 side = 0; side < 2; ++side)
            if (state.twinTanks[side] && state.owners[physical.front()->GetGUID()] == state.twinTanks[side] &&
                state.owners[caster.front()->GetGUID()] == state.twinTanks[1 - side])
                physicalSide = side;
    // Vek'nilash cannot be taunted, so the platform tank he is actually hitting keeps him and brings him home; the
    // other tank takes Vek'lor, who can be taunted.
    if (Unit* victim = physical.front()->GetVictim())
        for (uint32 side = 0; side < 2; ++side)
            if (state.twinTanks[side] && victim->GetGUID() == state.twinTanks[side])
                physicalSide = side;
    if (state.twinLast && (physical.front()->GetExactDist2d((*state.twinLast)[0]) > TWIN_TELEPORT_JUMP ||
                           caster.front()->GetExactDist2d((*state.twinLast)[1]) > TWIN_TELEPORT_JUMP))
        state.twinLastSwap = getMSTime();
    state.twinLast = std::array<Position, 2>{physical.front()->GetPosition(), caster.front()->GetPosition()};
    state.owners[physical.front()->GetGUID()] = state.twinTanks[physicalSide];
    state.owners[caster.front()->GetGUID()] = state.twinTanks[1 - physicalSide];
    // At the pull the warriors have no threat on Vek'lor and healing pulls him across the room, so a raid-marked
    // warlock bot holds him with Searing Pain while the other platform tank waits beside him.
    state.twinWaiter = state.twinTanks[1 - physicalSide];
    // One raid-marked warlock bot per side tanks Vek'lor whenever he is on that side; the side's warrior only holds
    // him for a few seconds after a teleport, since his Shadow Bolts (~3.5k every 2.5 s) kill warriors.
    state.twinLocks = {};
    state.twinLock.Clear();
    if (Group* group = bot->GetGroup())
        for (auto [icon, side] : {std::pair<uint8, uint32>{TWIN_LOCK_ICON, 1}, {TWIN_SECOND_LOCK_ICON, 0}})
            if (Player* lock = GetMember(bot, group->GetTargetIcon(icon)))
                if (lock->getClass() == CLASS_WARLOCK && GET_PLAYERBOT_AI(lock) && !PlayerbotAI::IsTank(lock))
                    state.twinLocks[side] = lock->GetGUID();
    uint32 lorSide = TwinSideOf(state, caster.front()->GetPosition());
    if (Player* lock = GetMember(bot, state.twinLocks[lorSide]))
    {
        state.twinLock = lock->GetGUID();
        state.owners[caster.front()->GetGUID()] = state.twinLock;
    }
}

bool IsTwinPlatformTank(Player* bot, Unit* unit)
{
    Aq40InstanceState const& state = Aq40State(bot->GetInstanceId());
    return unit && ((state.twinTanks[0] && unit->GetGUID() == state.twinTanks[0]) ||
                    (state.twinTanks[1] && unit->GetGUID() == state.twinTanks[1]));
}

bool IsTwinWaiter(Player* bot, Unit* unit)
{
    Aq40InstanceState const& state = Aq40State(bot->GetInstanceId());
    return unit && state.twinWaiter && unit->GetGUID() == state.twinWaiter;
}

bool IsTwinLock(Player* bot, Player* player)
{
    if (!player || !IsInEncounter(bot, Aq40Encounter::Twins))
        return false;
    Aq40InstanceState const& state = Aq40State(bot->GetInstanceId());
    return (state.twinLocks[0] && player->GetGUID() == state.twinLocks[0]) ||
           (state.twinLocks[1] && player->GetGUID() == state.twinLocks[1]);
}

// A tank-spec bot without a platform melees Vek'nilash as the second name on his threat list.
bool IsTwinSpareTank(Player* bot, Player* player)
{
    return IsInEncounter(bot, Aq40Encounter::Twins) && PlayerbotAI::IsTank(player) && !IsTwinPlatformTank(bot, player);
}

uint32 TwinSideOf(Aq40InstanceState const& state, Position const& pos)
{
    std::array<Position, 2> const& sides = *state.twinSides;
    return pos.GetExactDist2d(&sides[0]) < pos.GetExactDist2d(&sides[1]) ? 0 : 1;
}

Position TwinLockSpot(Aq40InstanceState const& state, uint32 side)
{
    std::array<Position, 2> const& sides = *state.twinSides;
    return TWIN_LOCK_SPOTS[0].GetExactDist2d(&sides[side]) < TWIN_LOCK_SPOTS[1].GetExactDist2d(&sides[side])
               ? TWIN_LOCK_SPOTS[0]
               : TWIN_LOCK_SPOTS[1];
}

bool TwinSwapWindow(Player* bot)
{
    Aq40InstanceState const& state = Aq40State(bot->GetInstanceId());
    return IsInEncounter(bot, Aq40Encounter::Twins) && state.twinLastSwap &&
           getMSTimeDiff(*state.twinLastSwap, getMSTime()) >= TWIN_SWAP_WINDOW_MS;
}

bool TwinOpeningHold(Player* bot, Unit* target)
{
    return IsInEncounter(bot, Aq40Encounter::Twins) && target &&
           target->GetHealthPct() >= TWIN_OPENING_HOLD_HEALTH_PCT &&
           ElapsedSinceEncounterStart(bot) < TWIN_OPENING_HOLD_MS;
}

Position TwinUnstuckSpot(Player* bot, Unit* emperor)
{
    // The platform tops are where the emperors spawn, so their pathing is known good. The stairs down from them are
    // not: Vek'nilash can lose his path to a tank standing right next to him there.
    Aq40InstanceState const& state = Aq40State(bot->GetInstanceId());
    if (!state.twinSides)
        return emperor->GetPosition();
    std::array<Position, 2> const& sides = *state.twinSides;
    uint32 side = TwinSideOf(state, emperor->GetPosition());
    float inward = sides[side].GetAngle(&sides[1 - side]);
    float offset = emperor->GetEntry() == Id(Aq40Npcs::NPC_VEKLOR)
                       ? emperor->GetCombatReach() + TWIN_BURST_CLEARANCE + 2.0f
                       : TWIN_TANK_OFFSET;
    Position spot = sides[side];
    spot.m_positionX += offset * std::cos(inward);
    spot.m_positionY += offset * std::sin(inward);
    return spot;
}

Position TwinWaitSpot(PlayerbotAI* botAI, Unit* veklor)
{
    Player* bot = botAI->GetBot();
    float wait = veklor->GetCombatReach() + bot->GetCombatReach() + TWIN_BURST_CLEARANCE;
    float base = veklor->GetAngle(bot);
    auto spot = [&](float angle)
    {
        Position goal = Around(veklor, wait, angle);
        bot->UpdateAllowedPositionZ(goal.GetPositionX(), goal.GetPositionY(), goal.m_positionZ);
        return goal;
    };
    // On the ring already: hold the current bearing, no path search needed.
    if (std::abs(bot->GetExactDist2d(veklor) - wait) <= 1.0f)
        return spot(base);
    Aq40BotCache& cache = BotCache(botAI);
    uint32 const now = getMSTime();
    if (cache.waitAngleTime && getMSTimeDiff(cache.waitAngleTime, now) < IN_MILLISECONDS)
        return spot(base + cache.waitAngle);
    float chosen = 0.0f;
    for (float offset : {0.0f, 0.6f, -0.6f, 1.2f, -1.2f, 1.9f, -1.9f, 2.6f, -2.6f, static_cast<float>(M_PI)})
    {
        Position goal = spot(base + offset);
        PathGenerator route(bot);
        if (route.CalculatePath(goal.GetPositionX(), goal.GetPositionY(), goal.GetPositionZ()) &&
            route.GetPathType() == PATHFIND_NORMAL && !route.GetPath().empty())
        {
            auto const& end = route.GetPath().back();
            if (goal.GetExactDist(end.x, end.y, end.z) <= 3.0f)
            {
                chosen = offset;
                break;
            }
        }
    }
    cache.waitAngle = chosen;
    cache.waitAngleTime = now;
    return spot(base + chosen);
}

bool IsHoldingVeknilash(Player* bot)
{
    for (Creature* nilash : GetUnits(bot, Id(Aq40Npcs::NPC_VEKNILASH)))
        if (nilash->GetVictim() == bot)
            return true;
    return false;
}

Creature* GetEmperorThatCannotReach(Player* bot)
{
    for (Creature* emperor : GetUnits(bot))
        if (IsTwinEmperor(emperor) && emperor->GetVictim() == bot && emperor->CanNotReachTarget())
            return emperor;
    return nullptr;
}

Player* GetTwinHealFocus(Player* bot)
{
    // Unbalancing Strike + Uppercut + melee can remove a tank in about two seconds, so tank healers keep whoever the
    // emperors hit topped up. After a swap Vek'nilash is passive for 2 s; top up the tank he is about to hit.
    Player* focus = nullptr;
    for (Creature* emperor : GetUnits(bot))
    {
        if (!IsTwinEmperor(emperor))
            continue;
        Unit* victim = emperor->GetVictim();
        if (!victim && emperor->GetEntry() == Id(Aq40Npcs::NPC_VEKNILASH))
            victim = GetTankFor(bot, emperor);
        if (Player* hit = victim ? victim->ToPlayer() : nullptr)
            if (GetMember(bot, hit->GetGUID()) && hit->GetHealthPct() < 90.0f && bot->GetDistance(hit) < 38.0f &&
                bot->IsWithinLOSInMap(hit) && (!focus || hit->GetHealthPct() < focus->GetHealthPct()))
                focus = hit;
    }
    return focus && !TwinOpeningHold(bot, focus) ? focus : nullptr;
}

// C'Thun

void AssignCthunSpots(Player* bot)
{
    constexpr uint32 count = sizeof(CTHUN_SLOTS) / sizeof(CTHUN_SLOTS[0]);
    Aq40InstanceState& state = Aq40State(bot->GetInstanceId());
    std::vector<Player*> members = GetMembers(bot);

    // The puller entered alone and is the member nearest the Eye when the fight starts; the first Eye Beam (3 s)
    // confirms it. The Eye's victim is not reliable: a healer may hold its threat.
    for (Creature* eye : GetUnits(bot, Id(Aq40Npcs::NPC_EYE_OF_CTHUN)))
    {
        if (!state.cthunPuller)
            for (Player* member : members)
                if (Group* group = member->GetGroup();
                    group && group->GetTargetIcon(CTHUN_PULLER_ICON) == member->GetGUID())
                    state.cthunPuller = member->GetGUID();
        if (!state.cthunPuller)
        {
            Player* nearest = nullptr;
            for (Player* member : members)
                if (!nearest || member->GetExactDist2d(eye) < nearest->GetExactDist2d(eye))
                    nearest = member;
            if (nearest)
                state.cthunPuller = nearest->GetGUID();
        }
        if (ElapsedSinceEncounterStart(bot) < CTHUN_PULLER_BEAM_MS)
            if (Spell* beam = eye->GetCurrentSpell(CURRENT_GENERIC_SPELL))
                if (beam->m_spellInfo->Id == Id(Aq40Spells::SPELL_EYE_BEAM))
                    if (Unit* target = beam->m_targets.GetUnitTarget())
                        if (target->IsPlayer())
                            state.cthunPuller = target->GetGUID();
    }

    auto role = [](Player* member)
    {
        if (PlayerbotAI::IsHeal(member))
            return CthunSlotKind::Outer;
        if (PlayerbotAI::IsMelee(member) || PlayerbotAI::IsTank(member))
            return CthunSlotKind::EyeMelee;
        return CthunSlotKind::Middle;
    };
    auto taken = [&state](uint32 index)
    {
        for (auto const& [guid, slot] : state.cthunSpots)
            if (slot == index)
                return true;
        return false;
    };
    auto claim = [&](ObjectGuid guid, std::initializer_list<CthunSlotKind> kinds)
    {
        for (CthunSlotKind kind : kinds)
            for (uint32 index = 0; index < count; ++index)
                if (CTHUN_SLOTS[index].kind == kind && !taken(index))
                {
                    state.cthunSpots[guid] = index;
                    return true;
                }
        return false;
    };
    // After Dark Glare the raid has moved around ahead of the beam; re-deal spots so each bot takes the nearest free
    // one of its kind instead of walking back past the others, where Eye Beam chains them.
    if (state.cthunRedeal && !state.cthunSpots.empty() && !IsCthunPhase2(bot))
    {
        state.cthunRedeal = false;
        std::map<ObjectGuid, uint32> old;
        old.swap(state.cthunSpots);
        for (Player* member : members)
        {
            auto previous = old.find(member->GetGUID());
            if (previous == old.end())
                continue;
            CthunSlotKind kind = CTHUN_SLOTS[previous->second].kind;
            float best = std::numeric_limits<float>::max();
            uint32 bestIndex = previous->second;
            for (uint32 index = 0; index < count; ++index)
            {
                if (CTHUN_SLOTS[index].kind != kind || taken(index))
                    continue;
                Position spot(CTHUN_CENTER.GetPositionX() + CTHUN_SLOTS[index].x,
                              CTHUN_CENTER.GetPositionY() + CTHUN_SLOTS[index].y, CTHUN_CENTER.GetPositionZ());
                float distance = member->GetExactDist2d(&spot);
                if (distance < best)
                {
                    best = distance;
                    bestIndex = index;
                }
            }
            state.cthunSpots[member->GetGUID()] = bestIndex;
        }
        // Anyone not re-dealt (dead, in the stomach) keeps a free spot of its old kind if one is left.
        for (auto const& [guid, index] : old)
            if (!state.cthunSpots.count(guid) && !taken(index))
                state.cthunSpots[guid] = index;
    }
    if (state.cthunSpots.empty())
    {
        // Deal the spots once per pull, by GUID: damage-dealing melee on the Eye first (tanks only if short of
        // melee), healers outside, ranged damage and the remaining melee in the middle ring.
        for (bool tanks : {false, true})
            for (Player* member : members)
                if (role(member) == CthunSlotKind::EyeMelee && PlayerbotAI::IsTank(member) == tanks)
                    claim(member->GetGUID(), {CthunSlotKind::EyeMelee});
        for (Player* member : members)
            if (role(member) == CthunSlotKind::Outer)
                claim(member->GetGUID(), {CthunSlotKind::Outer, CthunSlotKind::Middle});
        for (Player* member : members)
            if (!state.cthunSpots.count(member->GetGUID()))
                claim(member->GetGUID(), {CthunSlotKind::Middle, CthunSlotKind::Outer, CthunSlotKind::EyeMelee});
    }
    // Late arrivals (a revived or returning member) take a free spot of their kind.
    for (Player* member : members)
        if (!state.cthunSpots.count(member->GetGUID()))
        {
            CthunSlotKind kind = role(member) == CthunSlotKind::EyeMelee ? CthunSlotKind::Middle : role(member);
            claim(member->GetGUID(), {kind, CthunSlotKind::Middle, CthunSlotKind::Outer});
        }
}

void TrackDarkGlare(Player* bot)
{
    Aq40InstanceState& state = Aq40State(bot->GetInstanceId());
    for (Creature* eye : GetUnits(bot, Id(Aq40Npcs::NPC_EYE_OF_CTHUN)))
    {
        bool glare = eye->HasAura(Id(Aq40Spells::SPELL_RED_COLORATION));
        float angle = eye->GetOrientation();
        if (glare && state.glareActive)
        {
            float delta = AngleDelta(angle, state.lastGlare);
            if (std::abs(delta) > 0.01f && std::abs(delta) < 0.5f)
                state.glareDirection = delta > 0.0f ? 1.0f : -1.0f;
        }
        else
            state.glareDirection = 0.0f;
        if (state.glareActive && !glare)
            state.cthunRedeal = true;
        state.glareActive = glare;
        state.lastGlare = angle;
    }
}

void AssignCthunTentacleKillers(Player* bot)
{
    Aq40InstanceState& state = Aq40State(bot->GetInstanceId());
    state.cthunTentacleKillers.clear();
    if (!IsCthunPhase2(bot))
        return;
    // Ranged damage bots only (never a human player): hunters first (longest reach), then mages, then the rest.
    std::vector<Player*> killers;
    for (Player* member : GetMembers(bot))
        if (GET_PLAYERBOT_AI(member) && !InStomach(member) && !PlayerbotAI::IsHeal(member) &&
            !PlayerbotAI::IsMelee(member) && !PlayerbotAI::IsTank(member))
            killers.push_back(member);
    auto rank = [](Player* member) {
        return member->getClass() == CLASS_HUNTER ? 0u : member->getClass() == CLASS_MAGE ? 1u : 2u;
    };
    std::stable_sort(killers.begin(), killers.end(),
                     [&](Player* left, Player* right) { return rank(left) < rank(right); });
    for (uint32 index = 0; index < killers.size() && index < CTHUN_TENTACLE_KILLERS; ++index)
        state.cthunTentacleKillers.push_back(killers[index]->GetGUID());
}

void AssignCthunClawTanks(Player* bot)
{
    // The nearest tank takes each Giant Claw (the least-loaded one could be across the room while the claw kills
    // whoever it spawned under).
    Aq40InstanceState& state = Aq40State(bot->GetInstanceId());
    std::vector<Player*> tanks = GetMembers(bot, true);
    for (Creature* claw : GetUnits(bot, Id(Aq40Npcs::NPC_GIANT_CLAW_TENTACLE)))
    {
        Player* nearest = nullptr;
        for (Player* tank : tanks)
            if (!InStomach(tank) && (!nearest || tank->GetDistance(claw) < nearest->GetDistance(claw)))
                nearest = tank;
        if (nearest)
            state.owners[claw->GetGUID()] = nearest->GetGUID();
    }
}

void AcceptOuroTank(Player* bot)
{
    // Sand Blast erases the hit tank's threat. Accept the next tank instead of taunting it back.
    Creature* boss = GetBoss(bot);
    Unit* victim = boss ? boss->GetVictim() : nullptr;
    if (!victim || !victim->IsPlayer() || !PlayerbotAI::IsTank(victim->ToPlayer()) ||
        !GetMember(bot, victim->GetGUID()))
        return;
    Aq40InstanceState& state = Aq40State(bot->GetInstanceId());
    state.owners[boss->GetGUID()] = victim->GetGUID();
    state.mainTank = victim->GetGUID();
}

bool StomachExitNeeded(Player* bot)
{
    if (!IsInEncounter(bot, Aq40Encounter::Cthun) || !InStomach(bot))
        return false;
    // A swallowed tank leaves at once: Giant Claws need it up top.
    if (PlayerbotAI::IsTank(bot))
        return true;
    bool healerInside = false;
    for (Player* member : GetMembers(bot))
        healerInside = healerInside || (member != bot && PlayerbotAI::IsHeal(member) && InStomach(member));
    uint32 const leaveStacks = healerInside ? CTHUN_STOMACH_LEAVE_STACKS_WITH_HEALER : CTHUN_STOMACH_LEAVE_STACKS;
    return Stacks(bot, Id(Aq40Spells::SPELL_DIGESTIVE_ACID)) >= leaveStacks ||
           bot->GetHealthPct() < CTHUN_STOMACH_LEAVE_HEALTH_PCT ||
           GetUnits(bot, Id(Aq40Npcs::NPC_FLESH_TENTACLE)).empty();
}

bool GetCthunSpot(Player* bot, Position& spot)
{
    Aq40InstanceState const& state = Aq40State(bot->GetInstanceId());
    auto found = state.cthunSpots.find(bot->GetGUID());
    if (found == state.cthunSpots.end())
        return false;
    // Phase 2 stacks instead of keeping the phase-1 spread, which cost far more deaths and less damage on C'Thun.
    if (IsCthunPhase2(bot))
    {
        // A Giant Eye away from the stack is killed where it stands: the stack moves to it so melee can interrupt
        // it and casters see it. Left alone, one behind C'Thun takes no damage while its chaining beam kills the
        // stack.
        Position center = CTHUN_STACK;
        for (Creature* giantEye : GetUnits(bot, Id(Aq40Npcs::NPC_GIANT_EYE_TENTACLE)))
            if (giantEye->GetExactDist2d(&CTHUN_STACK) > CTHUN_GIANT_EYE_STACK_DISTANCE)
            {
                float angle = giantEye->GetAngle(&CTHUN_STACK);
                center.Relocate(giantEye->GetPositionX() + 8.0f * std::cos(angle),
                                giantEye->GetPositionY() + 8.0f * std::sin(angle), giantEye->GetPositionZ());
                break;
            }
        // A small spiral around the stack point, so bots don't all walk to one pixel.
        float index = float(found->second);
        float radius = CTHUN_STACK_RADIUS * std::sqrt(std::fmod(index, 20.0f) / 20.0f);
        spot.Relocate(center.GetPositionX() + radius * std::cos(index * 2.4f),
                      center.GetPositionY() + radius * std::sin(index * 2.4f), center.GetPositionZ());
        return true;
    }
    CthunSlotSpot const& slot = CTHUN_SLOTS[found->second];
    spot.Relocate(CTHUN_CENTER.GetPositionX() + slot.x, CTHUN_CENTER.GetPositionY() + slot.y,
                  CTHUN_CENTER.GetPositionZ());
    return true;
}

bool IsCthunPhase2(Player* bot)
{
    // The Eye fakes its death (0 health, still "alive") when phase 2 starts.
    for (Creature* eye : GetUnits(bot, Id(Aq40Npcs::NPC_EYE_OF_CTHUN)))
        if (eye->GetHealth() > 0)
            return false;
    return true;
}

bool IsCthunWeakened(Player* bot)
{
    Creature* body = GetCthunBody(bot);
    return body && body->HasAura(Id(Aq40Spells::SPELL_PURPLE_COLORATION));
}

bool IsCthunTentacleKiller(Player* bot)
{
    GuidVector const& killers = Aq40State(bot->GetInstanceId()).cthunTentacleKillers;
    return !InStomach(bot) && std::find(killers.begin(), killers.end(), bot->GetGUID()) != killers.end();
}

bool CthunMeleeBurn(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    if (!IsInEncounter(bot, Aq40Encounter::Cthun) || InStomach(bot) ||
        !(PlayerbotAI::IsMelee(bot) || PlayerbotAI::IsTank(bot)))
        return false;
    Unit* target = GetAssignedTarget(botAI);
    return target && target->GetEntry() == Id(Aq40Npcs::NPC_CTHUN) &&
           target->HasAura(Id(Aq40Spells::SPELL_PURPLE_COLORATION));
}

void ClearStaleLastMove(PlayerbotAI* botAI)
{
    if (botAI->GetBot()->movespline->Finalized())
        botAI->GetAiObjectContext()->GetValue<LastMovement&>("last movement")->Get().clear();
}

bool GetRouteStep(Player* bot, Position goal, bool allowPartial, Position& next)
{
    bot->UpdateAllowedPositionZ(goal.GetPositionX(), goal.GetPositionY(), goal.m_positionZ);
    PathGenerator route(bot);
    if (!route.CalculatePath(goal.GetPositionX(), goal.GetPositionY(), goal.GetPositionZ()) || route.GetPath().empty())
        return false;
    bool const partial = allowPartial && route.GetPathType() == PATHFIND_INCOMPLETE;
    if (route.GetPathType() != PATHFIND_NORMAL && !partial)
        return false;
    auto const& end = route.GetPath().back();
    if (!partial && goal.GetExactDist(end.x, end.y, end.z) > 3.0f)
        return false;
    // Twins transfers run continuously across the room: stopping every few yards adds an AI reaction delay per
    // segment and can miss the next teleport.
    float remaining = 250.0f;
    next = bot->GetPosition();
    for (auto const& point : route.GetPath())
    {
        float segment = next.GetExactDist(point.x, point.y, point.z);
        if (segment > remaining)
        {
            float fraction = remaining / segment;
            next.Relocate(next.GetPositionX() + (point.x - next.GetPositionX()) * fraction,
                          next.GetPositionY() + (point.y - next.GetPositionY()) * fraction,
                          next.GetPositionZ() + (point.z - next.GetPositionZ()) * fraction);
            break;
        }
        next.Relocate(point.x, point.y, point.z);
        remaining -= segment;
    }
    return true;
}

bool GetEscapeStep(Player* bot, Position const& goal, std::vector<Creature*> const& avoid, Position& step)
{
    float const distance = bot->GetExactDist2d(goal);
    if (distance < 1.5f)
        return false;
    float const angle = bot->GetAngle(&goal);
    float const length = std::min(distance, 7.0f);
    bool found = false;
    float bestClearance = -1.0f;
    for (float turn : {0.0f, 0.9f, -0.9f, 1.8f, -1.8f, static_cast<float>(M_PI)})
    {
        float x = bot->GetPositionX() + length * std::cos(angle + turn);
        float y = bot->GetPositionY() + length * std::sin(angle + turn);
        float z = bot->GetPositionZ();
        if (!bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bot->GetPositionX(), bot->GetPositionY(),
                                                            bot->GetPositionZ(), x, y, z))
            continue;
        PathGenerator path(bot);
        if (!path.CalculatePath(x, y, z) || path.GetPathType() != PATHFIND_NORMAL)
            continue;
        float pathLength = 0.0f;
        Position previous = bot->GetPosition();
        for (auto const& point : path.GetPath())
        {
            Position corner(point.x, point.y, point.z);
            pathLength += previous.GetExactDist(corner);
            previous = corner;
        }
        if (pathLength > 12.0f)
            continue;
        float clearance = std::numeric_limits<float>::max();
        for (Creature* unit : avoid)
            clearance = std::min(clearance, unit->GetExactDist2d(x, y));
        if (found && clearance <= bestClearance)
            continue;
        found = true;
        step.Relocate(x, y, z);
        bestClearance = clearance;
        if (avoid.empty())
            break;
    }
    return found;
}

// Hazard escapes

bool GetStomachExitGoal(PlayerbotAI* botAI, Position& goal)
{
    if (!StomachExitNeeded(botAI->GetBot()))
        return false;
    AreaTrigger const* exit = sObjectMgr->GetAreaTrigger(CTHUN_STOMACH_EXIT_TRIGGER);
    if (!exit)
        return false;
    goal.Relocate(exit->x, exit->y, exit->z);
    return true;
}

bool GetToxinEscapeGoal(PlayerbotAI* botAI, Position& goal)
{
    Player* bot = botAI->GetBot();
    if (!IsInEncounter(bot, Aq40Encounter::Viscidus))
        return false;
    // Melee on the boss move around clouds with their own action, and his tank drags him clear.
    if (IsViscidusRingMelee(botAI))
        return false;
    if (Creature* boss = GetBoss(bot); boss && boss->GetVictim() == bot)
        return false;
    // Clouds never despawn and stack up wherever the raid stands. Escape along the sum of every nearby cloud's push,
    // leaning toward the bot's ring slot, instead of stepping straight off the nearest one.
    constexpr float hazard = TOXIN_HAZARD_RADIUS;
    constexpr float awareness = hazard + 4.0f;
    float pushX = 0.0f;
    float pushY = 0.0f;
    bool inside = false;
    for (Creature* cloud : GetUnits(bot, Id(Aq40Npcs::NPC_TOXIC_SLIME)))
    {
        float distance = bot->GetExactDist2d(cloud);
        if (distance > awareness)
            continue;
        inside = inside || distance < hazard;
        float weight = (awareness - distance) / std::max(distance, 0.5f);
        pushX += (bot->GetPositionX() - cloud->GetPositionX()) * weight;
        pushY += (bot->GetPositionY() - cloud->GetPositionY()) * weight;
    }
    if (!inside)
        return false;
    float push = std::sqrt(pushX * pushX + pushY * pushY);
    if (push > 0.01f)
    {
        pushX /= push;
        pushY /= push;
    }
    if (Creature* boss = GetBoss(bot))
    {
        Position slot = ViscidusRing(bot, boss);
        float dx = slot.GetPositionX() - bot->GetPositionX();
        float dy = slot.GetPositionY() - bot->GetPositionY();
        float length = std::sqrt(dx * dx + dy * dy);
        if (length > 1.0f)
        {
            pushX += 0.5f * dx / length;
            pushY += 0.5f * dy / length;
        }
    }
    float length = std::sqrt(pushX * pushX + pushY * pushY);
    if (length < 0.01f)
    {
        pushX = 1.0f;
        pushY = 0.0f;
        length = 1.0f;
    }
    goal.Relocate(bot->GetPositionX() + hazard * pushX / length, bot->GetPositionY() + hazard * pushY / length,
                  bot->GetPositionZ());
    return true;
}

bool GetPoisonCloudEscapeGoal(PlayerbotAI* botAI, Position& goal)
{
    return IsInEncounter(botAI->GetBot(), Aq40Encounter::Trio) &&
           EscapeUnitHazard(botAI, goal, POISON_CLOUD_RADIUS,
                            [](Unit* unit) { return unit->GetEntry() == Id(Aq40Npcs::NPC_POISON_CLOUD); });
}

// A mound chases its target at run speed, so its target can never step clear. It leads the mound away from the
// raid instead, and holds once nobody else is inside the Quake.
static bool GetDirtMoundLeadGoal(Player* bot, Creature* mound, Position& goal)
{
    float pushX = 0.0f;
    float pushY = 0.0f;
    bool crowded = false;
    for (Player* member : GetMembers(bot))
    {
        float distance = bot->GetExactDist2d(member);
        if (member == bot || distance >= DIRT_MOUND_RADIUS)
            continue;
        crowded = true;
        if (distance > 0.1f)
        {
            pushX += (bot->GetPositionX() - member->GetPositionX()) / distance;
            pushY += (bot->GetPositionY() - member->GetPositionY()) / distance;
        }
    }
    if (!crowded)
        return false;
    float angle = std::sqrt(pushX * pushX + pushY * pushY) > 0.01f ? std::atan2(pushY, pushX) : mound->GetAngle(bot);
    goal = Around(bot, 5.0f, angle);
    return true;
}

bool GetDirtMoundEscapeGoal(PlayerbotAI* botAI, Position& goal)
{
    Player* bot = botAI->GetBot();
    if (!IsInEncounter(bot, Aq40Encounter::Ouro))
        return false;
    // A tank holding Ouro or scarabs stays put; dragging them through the raid costs more than the Quake.
    if (PlayerbotAI::IsTank(bot))
        for (Creature* unit : GetUnits(bot))
            if (unit->GetEntry() != Id(Aq40Npcs::NPC_DIRT_MOUND) && unit->GetVictim() == bot)
                return false;
    std::vector<Creature*> mounds = GetUnits(bot, Id(Aq40Npcs::NPC_DIRT_MOUND));
    for (Creature* mound : mounds)
        if (mound->GetVictim() == bot)
            return GetDirtMoundLeadGoal(bot, mound, goal);
    for (Creature* mound : mounds)
        if (bot->GetDistance2d(mound) < DIRT_MOUND_RADIUS)
        {
            goal = Around(bot, 5.0f, mound->GetAngle(bot));
            return true;
        }
    return false;
}

bool GetExplodingBugEscapeGoal(PlayerbotAI* botAI, Position& goal)
{
    return IsInEncounter(botAI->GetBot(), Aq40Encounter::Twins) &&
           EscapeUnitHazard(botAI, goal, EXPLODING_BUG_RADIUS,
                            [](Unit* unit) { return unit->HasAura(Id(Aq40Spells::SPELL_EXPLODE_BUG)); });
}

bool GetDarkGlareEscapeGoal(PlayerbotAI* botAI, Position& goal)
{
    Player* bot = botAI->GetBot();
    if (!IsInEncounter(bot, Aq40Encounter::Cthun) || InStomach(bot))
        return false;
    float const direction = Aq40State(bot->GetInstanceId()).glareDirection;
    for (Creature* eye : GetUnits(bot, Id(Aq40Npcs::NPC_EYE_OF_CTHUN)))
    {
        if (!eye->HasAura(Id(Aq40Spells::SPELL_RED_COLORATION)))
            continue;
        // Only the real band moves anyone: a fixed angle would flag ~26 yd either side at 40 yd.
        float bearing = eye->GetAngle(bot);
        float distance = eye->GetExactDist2d(bot);
        if (InGlarePath(bearing, distance, eye->GetOrientation(), direction, GLARE_HALF_WIDTH, GLARE_LOOKAHEAD))
        {
            goal = GlareDodge(botAI, eye, bearing, distance);
            return true;
        }
    }
    return false;
}

bool GetOuroConeEscapeGoal(PlayerbotAI* botAI, Position& goal)
{
    Player* bot = botAI->GetBot();
    if (!IsInEncounter(bot, Aq40Encounter::Ouro))
        return false;
    for (Creature* ouro : GetUnits(bot, Id(Aq40Npcs::NPC_OURO)))
        if (ouro->GetVictim() != bot && bot->GetDistance2d(ouro) < OURO_CONE_RANGE &&
            std::abs(AngleDelta(ouro->GetAngle(bot), ouro->GetOrientation())) < static_cast<float>(M_PI) / 3.0f)
        {
            float angle = ouro->GetAngle(bot);
            float side = AngleDelta(angle, ouro->GetOrientation()) >= 0.0f ? 1.0f : -1.0f;
            goal = Around(ouro, std::max(8.0f, ouro->GetExactDist2d(bot)), angle + side * 0.3f);
            return true;
        }
    return false;
}

bool GetVeknilashLandingEscapeGoal(PlayerbotAI* botAI, Position& goal)
{
    Player* bot = botAI->GetBot();
    if (!IsInEncounter(bot, Aq40Encounter::Twins) || PlayerbotAI::IsTank(bot) || PlayerbotAI::IsHeal(bot))
        return false;
    // After a swap Vek'nilash is passive for 2 s, then gives the nearest player 2000 threat; he can't be taunted, so
    // damage dealers clear the landing spot until he picks. Healers already hold outside the bystander ring.
    for (Creature* nilash : GetUnits(bot, Id(Aq40Npcs::NPC_VEKNILASH)))
        if (nilash->IsInCombat() && nilash->HasReactState(REACT_PASSIVE) && GetTankFor(bot, nilash) != bot &&
            bot->GetExactDist2d(nilash) < TWIN_BYSTANDER_DISTANCE + 6.0f)
        {
            goal = Around(nilash, TWIN_BYSTANDER_DISTANCE + 10.0f, nilash->GetAngle(bot));
            bot->UpdateAllowedPositionZ(goal.GetPositionX(), goal.GetPositionY(), goal.m_positionZ);
            return true;
        }
    return false;
}

bool GetVeklorEscapeGoal(PlayerbotAI* botAI, Position& goal)
{
    Player* bot = botAI->GetBot();
    if (!IsInEncounter(bot, Aq40Encounter::Twins))
        return false;
    Aq40InstanceState const& state = Aq40State(bot->GetInstanceId());
    for (Creature* veklor : GetUnits(bot, Id(Aq40Npcs::NPC_VEKLOR)))
    {
        // Arcane Burst fires on anyone in Vek'lor's melee range: ~4.5k damage, a ~33-yard knockback and a 70% slow.
        // His own tank steps just outside it and stays the nearest player to him.
        bool plate = GetTankFor(bot, veklor) == bot || (PlayerbotAI::IsTank(bot) && !IsTwinSpareTank(bot, bot));
        if (plate)
        {
            if (bot->GetDistance2d(veklor) >= TWIN_BURST_CLEARANCE - 1.0f)
                continue;
            // Swap ownership can lag the teleport by one refresh; a plate tank still only steps out.
            goal = TwinWaitSpot(botAI, veklor);
            return true;
        }
        // After a teleport he lands on the melee stack with a 2.3-second grace, so the escape goal is deep enough
        // that one forced move clears the radius.
        if (bot->GetExactDist2d(veklor) >= TWIN_BYSTANDER_DISTANCE - 2.0f)
            continue;
        float escape = veklor->GetAngle(bot);
        if (state.twinSides)
        {
            std::array<Position, 2> const& sides = *state.twinSides;
            uint32 side = TwinSideOf(state, veklor->GetPosition());
            float inward = sides[side].GetAngle(&sides[1 - side]);
            float offset = AngleDelta(escape, inward);
            // A bot knocked behind Vek'lor must leave around his side. Retreating straight outward makes its
            // cross-room route pass through him and repeat the escape forever.
            escape = std::cos(offset) < 0.0f ? inward + (offset >= 0.0f ? static_cast<float>(M_PI) / 2.0f
                                                                        : -static_cast<float>(M_PI) / 2.0f)
                                             : inward;
        }
        // Fan the escapes out: everyone leaving along one bearing ends in a stack that a single Blizzard can kill.
        escape += 0.3f * (float(bot->GetGUID().GetCounter() % 7) - 3.0f);
        goal = Around(veklor, TWIN_BYSTANDER_DISTANCE + 2.0f, escape);
        return true;
    }
    return false;
}

bool GetBlizzardEscapeGoal(PlayerbotAI* botAI, Position& goal)
{
    Player* bot = botAI->GetBot();
    if (!IsInEncounter(bot, Aq40Encounter::Twins))
        return false;
    std::vector<Position> const& blizzards =
        botAI->GetAiObjectContext()->GetValue<std::vector<Position>>("aq40 blizzard positions")->RefGet();
    for (Position const& blizzard : blizzards)
        if (bot->GetExactDist2d(blizzard) < BLIZZARD_HAZARD_RADIUS)
        {
            // Blizzard ticks ~1.4k every 2 s in a 10-yard circle. Leave it in one move instead of 5-yard steps that
            // keep a clustered caster group inside for another tick.
            float away = bot->GetExactDist2d(blizzard) > 0.5f ? blizzard.GetAngle(bot) : bot->GetOrientation();
            goal.Relocate(blizzard.GetPositionX() + BLIZZARD_ESCAPE_DISTANCE * std::cos(away),
                          blizzard.GetPositionY() + BLIZZARD_ESCAPE_DISTANCE * std::sin(away), bot->GetPositionZ());
            bot->UpdateAllowedPositionZ(goal.GetPositionX(), goal.GetPositionY(), goal.m_positionZ);
            return true;
        }
    return false;
}

// Encounter positions

bool GetSkeramPlatformGoal(PlayerbotAI* botAI, Position& goal)
{
    Player* bot = botAI->GetBot();
    if (!FormationPrelude(bot, Aq40Encounter::Skeram) || !GetSkeramPlatform(botAI, goal))
        return false;
    Aq40InstanceState const& state = Aq40State(bot->GetInstanceId());
    Position const tankPlatform = goal;
    uint32 side = state.skeramMemberPlatform.at(bot->GetGUID());
    bool healer = PlayerbotAI::IsHeal(bot);
    float radius = PlayerbotAI::IsTank(bot) ? 4.0f : 8.0f;
    // In-game healer positions cover the center and their raised platform without standing on the boss's blink
    // destination. Tank ownership and pickup bounds keep using that destination.
    if (healer && side != SKERAM_CENTER_PLATFORM)
    {
        goal = side == 0 ? SKERAM_LOW_Y_HEALER_SPOT : SKERAM_HIGH_Y_HEALER_SPOT;
        radius = 3.0f;
    }
    if (bot->GetExactDist2d(goal) > radius ||
        std::abs(bot->GetPositionZ() - goal.GetPositionZ()) > SKERAM_PLATFORM_HEIGHT_TOLERANCE)
        return true;
    if (healer)
    {
        // Healing target selection excludes units outside LOS. Recover sight of our platform's tank independently,
        // so a null healing target cannot strand the healer behind geometry.
        Player* tank = GetMember(bot, state.skeramPlatformTanks[side]);
        if (tank && tank->GetExactDist2d(tankPlatform) <= SKERAM_TANK_PLATFORM_RADIUS &&
            std::abs(tank->GetPositionZ() - tankPlatform.GetPositionZ()) <= SKERAM_PLATFORM_HEIGHT_TOLERANCE &&
            !bot->IsWithinLOSInMap(tank))
        {
            goal = tank->GetPosition();
            return true;
        }
    }
    return false;
}

bool GetTwinTankGoal(PlayerbotAI* botAI, Position& goal)
{
    Player* bot = botAI->GetBot();
    if (!FormationPrelude(bot, Aq40Encounter::Twins) || !IsTwinPlatformTankBot(bot))
        return false;
    Aq40InstanceState const& state = Aq40State(bot->GetInstanceId());
    std::array<Position, 2> const& sides = *state.twinSides;
    uint32 side = bot->GetGUID() == state.twinTanks[0] ? 0 : 1;
    // Hold a world-space anchor, not an offset from a boss that follows us. Chasing that moving offset lets
    // knockbacks and teleports ratchet both emperors across the room.
    float tankTolerance = 2.0f;
    // Vek'nilash follows the tank he is hitting while Vek'lor, held by taunt threat, stays put, so Vek'nilash's tank
    // does the separating: it takes him to whichever platform is clearly farther from Vek'lor.
    uint32 home = side;
    for (Creature* nilash : GetUnits(bot, Id(Aq40Npcs::NPC_VEKNILASH)))
        if (GetTankFor(bot, nilash) == bot)
            for (Creature* lor : GetUnits(bot, Id(Aq40Npcs::NPC_VEKLOR)))
                if (lor->GetExactDist2d(sides[side]) + 20.0f < lor->GetExactDist2d(sides[1 - side]))
                    home = 1 - side;
    float away = sides[1 - home].GetAngle(&sides[home]);
    goal = sides[home];
    goal.m_positionX += TWIN_TANK_OFFSET * std::cos(away);
    goal.m_positionY += TWIN_TANK_OFFSET * std::sin(away);
    for (Creature* incoming : GetUnits(bot))
    {
        bool waiter =
            incoming->GetEntry() == Id(Aq40Npcs::NPC_VEKLOR) && state.twinLock && state.twinWaiter == bot->GetGUID();
        if (GetTankFor(bot, incoming) != bot && !waiter)
            continue;
        if (incoming->GetEntry() == Id(Aq40Npcs::NPC_VEKLOR))
        {
            // Vek'nilash can't be taunted and after a swap attacks whoever is nearest Vek'lor's old spot, so wait
            // just outside Arcane Burst. Vek'lor casts at victims within 45 yd; Taunt reclaims him.
            goal = TwinWaitSpot(botAI, incoming);
            tankTolerance = 1.0f;  // A narrow band between Arcane Burst and Demoralizing Shout range.
            break;
        }
        if (incoming->GetEntry() == Id(Aq40Npcs::NPC_VEKNILASH) && incoming->GetVictim() != bot)
        {
            // Reach Vek'nilash at his actual position. A fixed pickup radius can strand the tank outside attack
            // range; once acquired, the anchor above brings him home.
            goal = Around(incoming, TWIN_TANK_OFFSET, away);
            bot->UpdateAllowedPositionZ(goal.GetPositionX(), goal.GetPositionY(), goal.m_positionZ);
            break;
        }
    }
    return bot->GetExactDist2d(goal) > tankTolerance || std::abs(bot->GetPositionZ() - goal.GetPositionZ()) > 3.0f;
}

bool GetTwinWarlockGoal(PlayerbotAI* botAI, Position& goal)
{
    Player* bot = botAI->GetBot();
    if (!FormationPrelude(bot, Aq40Encounter::Twins) || IsTwinPlatformTankBot(bot) || !IsTwinLock(bot, bot))
        return false;
    Aq40InstanceState const& state = Aq40State(bot->GetInstanceId());
    if (!state.twinSides)
        return false;
    uint32 mySide = state.twinLocks[0] == bot->GetGUID() ? 0 : 1;
    Position spot = TwinLockSpot(state, mySide);
    for (Creature* veklor : GetUnits(bot, Id(Aq40Npcs::NPC_VEKLOR)))
        if (GetTankFor(bot, veklor) == bot)
        {
            float distance = bot->GetExactDist2d(veklor);
            // Cast from anywhere inside Searing Pain range and outside the bystander ring. Vek'lor often walks after
            // the pull; chasing an exact spot would keep the warlock from casting.
            if (distance >= TWIN_BYSTANDER_DISTANCE - 2.0f && distance <= TWIN_LOCK_MAX_DISTANCE &&
                bot->IsWithinLOSInMap(veklor))
                return false;
            goal = Around(veklor, TWIN_LOCK_DISTANCE, veklor->GetAngle(&spot));
            bot->UpdateAllowedPositionZ(goal.GetPositionX(), goal.GetPositionY(), goal.m_positionZ);
            return true;
        }
    // Not our phase: wait at our spot beside our side's healers, clear of both landing spots.
    goal = spot;
    bot->UpdateAllowedPositionZ(goal.GetPositionX(), goal.GetPositionY(), goal.m_positionZ);
    return bot->GetExactDist2d(goal) > 4.0f;
}

bool GetTwinMeleeSwapGoal(PlayerbotAI* botAI, Position& goal)
{
    Player* bot = botAI->GetBot();
    if (!FormationPrelude(bot, Aq40Encounter::Twins) || !IsTwinDamageRole(bot) || !PlayerbotAI::IsMelee(bot) ||
        !TwinSwapWindow(bot))
        return false;
    Aq40InstanceState const& state = Aq40State(bot->GetInstanceId());
    if (!state.twinSides || GetUnits(bot, Id(Aq40Npcs::NPC_VEKNILASH)).empty())
        return false;
    std::array<Position, 2> const& sides = *state.twinSides;
    // Before the teleport window opens, melee leave Vek'nilash for the middle of the path between the spots:
    // Vek'lor lands where Vek'nilash stood and Arcane Bursts everyone in melee range.
    goal.Relocate((sides[0].GetPositionX() + sides[1].GetPositionX()) / 2.0f,
                  (sides[0].GetPositionY() + sides[1].GetPositionY()) / 2.0f, sides[0].GetPositionZ());
    bot->UpdateAllowedPositionZ(goal.GetPositionX(), goal.GetPositionY(), goal.m_positionZ);
    return bot->GetExactDist2d(goal) > 6.0f;
}

bool GetTwinDpsGoal(PlayerbotAI* botAI, Position& goal)
{
    Player* bot = botAI->GetBot();
    if (!FormationPrelude(bot, Aq40Encounter::Twins) || !IsTwinDamageRole(bot))
        return false;
    bool melee = PlayerbotAI::IsMelee(bot);
    if (melee && TwinSwapWindow(bot))
        return false;  // The melee swap goal owns this window.
    Aq40InstanceState const& state = Aq40State(bot->GetInstanceId());
    if (!state.twinSides)
        return false;
    std::array<Position, 2> const& sides = *state.twinSides;
    bool physical = IsPhysicalTwinDamage(bot);
    std::vector<Creature*> emperors = GetUnits(bot, physical ? Id(Aq40Npcs::NPC_VEKNILASH) : Id(Aq40Npcs::NPC_VEKLOR));
    if (emperors.empty())
        return false;
    Creature* emperor = emperors.front();
    uint32 side = TwinSideOf(state, emperor->GetPosition());
    // Travel follows the emperor even while damage waits for a tank pickup. Otherwise the held check and the
    // old-platform fallback prevent a bot from ever acquiring a distant target.
    constexpr float rangedDistance = TWIN_BYSTANDER_DISTANCE + 6.0f;
    constexpr float pickupDistance = 20.0f;
    float inward = sides[side].GetAngle(&sides[1 - side]);
    if (melee && IsHeld(bot, emperor))
    {
        float const minDistance = bot->GetCombatReach() + emperor->GetCombatReach() + 0.3f;
        float const maxDistance = bot->GetMeleeRange(emperor) - 0.3f;
        float const current = bot->GetExactDist2d(emperor);
        if (current >= minDistance && current <= maxDistance && !emperor->HasInArc(static_cast<float>(M_PI), bot) &&
            std::abs(bot->GetPositionZ() - emperor->GetPositionZ()) <= 3.0f)
            return false;
        float const behind = emperor->GetOrientation() + static_cast<float>(M_PI);
        float const bearing = std::cos(AngleDelta(inward, behind)) > 0.0f ? inward : behind;
        goal = Around(emperor, (minDistance + maxDistance) / 2.0f, bearing);
        bot->UpdateAllowedPositionZ(goal.GetPositionX(), goal.GetPositionY(), goal.m_positionZ);
        return true;
    }
    float distance = melee ? pickupDistance : rangedDistance;
    if (!melee)
    {
        uint32 slot = 0;
        uint32 count = 0;
        for (Player* member : GetMembers(bot))
            if (!PlayerbotAI::IsTank(member) && !PlayerbotAI::IsHeal(member) && !PlayerbotAI::IsMelee(member) &&
                (member->getClass() == CLASS_HUNTER) == physical)
            {
                if (member->GetGUID() < bot->GetGUID())
                    ++slot;
                ++count;
            }
        // Spread ranged over a wide arc and two depths: a 10-yard Blizzard on a tight arc can kill six casters.
        inward += 2.4f * ((float(slot) + 0.5f) / float(std::max(count, 1u)) - 0.5f);
        if (slot % 2)
            distance += 5.0f;
    }
    goal = Around(emperor, distance, inward);
    bot->UpdateAllowedPositionZ(goal.GetPositionX(), goal.GetPositionY(), goal.m_positionZ);
    return bot->GetExactDist2d(goal) > (melee ? 3.0f : 5.0f) ||
           std::abs(bot->GetPositionZ() - goal.GetPositionZ()) > 3.0f;
}

bool GetTwinHealerGoal(PlayerbotAI* botAI, Position& goal)
{
    Player* bot = botAI->GetBot();
    if (!FormationPrelude(bot, Aq40Encounter::Twins) || !PlayerbotAI::IsHeal(bot) || IsTwinPlatformTankBot(bot) ||
        IsTwinLock(bot, bot))
        return false;
    Aq40InstanceState const& state = Aq40State(bot->GetInstanceId());
    if (!state.twinSides)
        return false;
    std::array<Position, 2> const& sides = *state.twinSides;
    // Only healers consume healing slots. Counting the whole raid pushes the later healers beyond healing range,
    // especially after their tank is knocked back.
    auto sideIt = state.twinMemberSide.find(bot->GetGUID());
    uint32 side = sideIt != state.twinMemberSide.end() ? sideIt->second : 0;
    Player* tank = GetMember(bot, state.twinTanks[side]);
    // Vek'lor's Shadow Bolts land on his warlock, not on the waiting warrior. The healers of the side Vek'lor is on
    // look after the warlock while he holds him.
    if (Player* lock = GetMember(bot, state.twinLock))
        for (Creature* veklor : GetUnits(bot, Id(Aq40Npcs::NPC_VEKLOR)))
            if (GetTankFor(bot, veklor) == lock && tank &&
                veklor->GetExactDist2d(sides[side]) < veklor->GetExactDist2d(sides[1 - side]))
                tank = lock;
    std::vector<Creature*> veklors = GetUnits(bot, Id(Aq40Npcs::NPC_VEKLOR));
    auto clearOfVeklor = [&](Position const& spot)
    {
        for (Creature* veklor : veklors)
            if (veklor->GetExactDist2d(spot) < TWIN_BYSTANDER_DISTANCE)
                return false;
        return true;
    };
    if (tank && bot->GetExactDist(tank) >= 18.0f && bot->GetExactDist(tank) <= 30.0f && bot->IsWithinLOSInMap(tank) &&
        clearOfVeklor(bot->GetPosition()))
        return false;  // Keep healing instead of repositioning for every small tank movement.
    float axis = sides[side].GetAngle(&sides[1 - side]);
    uint32 slot = 0;
    uint32 count = 0;
    for (Player* member : GetMembers(bot))
    {
        auto memberSide = state.twinMemberSide.find(member->GetGUID());
        if (PlayerbotAI::IsHeal(member) && !PlayerbotAI::IsTank(member) && memberSide != state.twinMemberSide.end() &&
            memberSide->second == side)
        {
            if (member->GetGUID() < bot->GetGUID())
                ++slot;
            ++count;
        }
    }
    // Follow the assigned tank on this platform. Keep room for a knockback within normal healing range, and stay
    // outside Vek'lor's Arcane Burst radius.
    float spread = (float(slot) + 0.5f) / float(std::max(count, 1u)) - 0.5f;
    Position anchor = tank ? tank->GetPosition() : sides[side];
    // Healers stand between their tank and their side's warlock spot, toward the entrance.
    Position lockSpot = TwinLockSpot(state, side);
    float angle = (anchor.GetExactDist2d(&lockSpot) > 3.0f ? anchor.GetAngle(&lockSpot)
                                                           : axis + static_cast<float>(M_PI) / 2.0f) +
                  spread;
    // Healers stay farther from Vek'lor than his waiting tank, or Vek'nilash lands on them.
    bool found = false;
    Position fallback;
    for (float turn : {0.0f, 0.8f, -0.8f, 1.6f, -1.6f, 2.4f, -2.4f})
    {
        for (float radius : {24.0f, 20.0f, 16.0f})
        {
            goal.Relocate(anchor.GetPositionX() + radius * std::cos(angle + turn),
                          anchor.GetPositionY() + radius * std::sin(angle + turn), anchor.GetPositionZ());
            bot->UpdateAllowedPositionZ(goal.GetPositionX(), goal.GetPositionY(), goal.m_positionZ);
            if (turn == 0.0f && radius == 24.0f)
                fallback = goal;
            if ((!tank || tank->IsWithinLOS(goal.GetPositionX(), goal.GetPositionY(), goal.GetPositionZ())) &&
                clearOfVeklor(goal))
            {
                found = true;
                break;
            }
        }
        if (found)
            break;
    }
    if (!found)
        goal = fallback;
    return bot->GetExactDist2d(goal) > 5.0f || std::abs(bot->GetPositionZ() - goal.GetPositionZ()) > 3.0f;
}

bool GetCthunPositionGoal(PlayerbotAI* botAI, Position& goal)
{
    CthunGoalKind kind;
    return GetCthunGoal(botAI, goal, kind);
}

bool GetCthunPullerHoldGoal(PlayerbotAI* botAI, Position& goal)
{
    return GetCthunGoalOfKind(botAI, goal, CthunGoalKind::PullerHold);
}

bool GetCthunMeleeBurnGoal(PlayerbotAI* botAI, Position& goal)
{
    return GetCthunGoalOfKind(botAI, goal, CthunGoalKind::MeleeBurn);
}

bool GetCthunSpotGoal(PlayerbotAI* botAI, Position& goal)
{
    return GetCthunGoalOfKind(botAI, goal, CthunGoalKind::Spot);
}

bool GetCthunStackGoal(PlayerbotAI* botAI, Position& goal)
{
    return GetCthunGoalOfKind(botAI, goal, CthunGoalKind::Stack);
}

bool GetTrioTankGoal(PlayerbotAI* botAI, Position& goal)
{
    Player* bot = botAI->GetBot();
    if (!FormationPrelude(bot, Aq40Encounter::Trio) || !PlayerbotAI::IsTank(bot))
        return false;
    // Keep the bosses separated without pulling them away from their own healers.
    Unit* target = GetAssignedTarget(botAI);
    if (!target || GetTankFor(bot, target) != bot || target->GetVictim() != bot)
        return false;
    for (Creature* other : GetUnits(bot))
        if (other != target && IsEncounterBoss(other->GetEntry()) && other->GetDistance(target) < TRIO_BOSS_SEPARATION)
        {
            goal = Around(bot, 4.0f, other->GetAngle(bot));
            return true;
        }
    return false;
}

bool GetHuhuranSoakGoal(PlayerbotAI* botAI, Position& goal)
{
    Player* bot = botAI->GetBot();
    if (!FormationPrelude(bot, Aq40Encounter::Huhuran) || PlayerbotAI::IsTank(bot) || !IsHuhuranSoaker(bot))
        return false;
    Creature* boss = GetBoss(bot);
    float desired = PlayerbotAI::IsMelee(bot) ? 1.0f : 8.0f;
    goal = Around(boss, desired + boss->GetCombatReach(), boss->GetAngle(bot));
    return std::abs(bot->GetExactDist2d(boss) - desired - boss->GetCombatReach()) > 4.0f;
}

bool GetHuhuranSpreadGoal(PlayerbotAI* botAI, Position& goal)
{
    Player* bot = botAI->GetBot();
    // Melee stay on the boss whether or not the soak list exists.
    if (!FormationPrelude(bot, Aq40Encounter::Huhuran) || PlayerbotAI::IsTank(bot) || PlayerbotAI::IsMelee(bot) ||
        IsHuhuranSoaker(bot))
        return false;
    // Noxious Poison silences a 15-yd circle around a random raider. Everyone outside the soak group gets a slot
    // ~15 yd apart on two staggered rings: healers on the tank's side, ranged damage on the far side.
    Creature* boss = GetBoss(bot);
    bool healer = PlayerbotAI::IsHeal(bot);
    std::vector<Player*> peers;
    for (Player* member : GetMembers(bot))
        if (PlayerbotAI::IsHeal(member) == healer && !PlayerbotAI::IsTank(member) && !PlayerbotAI::IsMelee(member) &&
            !IsHuhuranSoaker(member))
            peers.push_back(member);
    auto self = std::find(peers.begin(), peers.end(), bot);
    if (self == peers.end())
        return false;
    uint32 index = uint32(self - peers.begin());
    uint32 ring = index % 2;
    uint32 slot = index / 2;
    uint32 slots = (uint32(peers.size()) + 1 - ring) / 2;
    Player* tank = GetTankFor(bot, boss);
    Unit* anchor = tank ? tank : boss->GetVictim();
    float center = anchor ? boss->GetAngle(anchor) : boss->GetOrientation();
    float span = healer ? 1.4f : 1.7f;
    if (!healer)
        center += static_cast<float>(M_PI);
    float angle = center - span + span * 2.0f * (float(slot) + 0.5f) / float(std::max(slots, 1u));
    float radius = boss->GetCombatReach() + (ring == 0 ? 20.0f : 27.0f);
    // Tanked near the cave edge, part of the ring lands on the wall: bots climbed it chasing a slot at the boss's
    // height. Take the nearest variant whose floor is at the boss's level and that sees her; else hold position.
    bool found = false;
    for (float pull : {0.0f, 4.0f, 8.0f, 12.0f})
    {
        for (float turn : {0.0f, 0.2f, -0.2f, 0.4f, -0.4f})
        {
            goal = Around(boss, radius - pull, angle + turn);
            bot->UpdateAllowedPositionZ(goal.GetPositionX(), goal.GetPositionY(), goal.m_positionZ);
            if (std::abs(goal.GetPositionZ() - boss->GetPositionZ()) < HUHURAN_SPREAD_MAX_RISE &&
                boss->IsWithinLOS(goal.GetPositionX(), goal.GetPositionY(), goal.GetPositionZ()))
            {
                found = true;
                break;
            }
        }
        if (found)
            break;
    }
    if (!found)
        return false;
    return bot->GetExactDist2d(goal) > 5.0f ||
           std::abs(bot->GetPositionZ() - goal.GetPositionZ()) > HUHURAN_SPREAD_MAX_RISE;
}

bool GetViscidusRingGoal(PlayerbotAI* botAI, Position& goal)
{
    Player* bot = botAI->GetBot();
    if (!FormationPrelude(bot, Aq40Encounter::Viscidus) || PlayerbotAI::IsTank(bot) ||
        !(PlayerbotAI::IsHeal(bot) || PlayerbotAI::IsRanged(bot) || ViscidusMeleeWaitsForFreeze(botAI)))
        return false;
    Creature* boss = GetBoss(bot);
    if (!PlayerbotAI::IsHeal(bot))
        if (Unit* target = GetAssignedTarget(botAI); target && target != boss)
            return false;
    // The generic ring measures from the boss's center but aims at 28 yards plus his 10-yard reach, so nobody ever
    // settles. Measure and aim with the same reach, and keep the slot off the ramp.
    float ring = boss->GetCombatReach() + VISCIDUS_RING_OFFSET;
    if (bot->GetPositionZ() <= VISCIDUS_RAMP_Z && std::abs(bot->GetExactDist2d(boss) - ring) <= VISCIDUS_RING_TOLERANCE)
        return false;
    goal = ViscidusRing(bot, boss);
    return true;
}

bool GetBossRangeGoal(PlayerbotAI* botAI, Position& goal)
{
    Player* bot = botAI->GetBot();
    Aq40Encounter encounter = GetCurrentEncounter(bot);
    if (encounter != Aq40Encounter::Trio && encounter != Aq40Encounter::Sartura &&
        encounter != Aq40Encounter::Fankriss && encounter != Aq40Encounter::Ouro)
        return false;
    if (!FormationPrelude(bot, encounter) || PlayerbotAI::IsTank(bot) ||
        !(PlayerbotAI::IsHeal(bot) || PlayerbotAI::IsRanged(bot)))
        return false;
    Creature* boss = GetBoss(bot);
    if (!PlayerbotAI::IsHeal(bot))
        if (Unit* target = GetAssignedTarget(botAI); target && target != boss)
            return false;
    float distance = bot->GetDistance2d(boss);
    if (distance >= 22.0f && distance <= 36.0f)
        return false;
    goal = Around(boss, 28.0f + boss->GetCombatReach(), boss->GetAngle(bot));
    return true;
}

// Before the pull

bool GetTwinPrepullSpot(PlayerbotAI* botAI, Position& spot)
{
    Player* bot = botAI->GetBot();
    InstanceScript* instance = bot->GetInstanceScript();
    Group* group = bot->GetGroup();
    if (bot->GetMapId() != AQ40_MAP_ID || !instance || !group || bot->IsInCombat() || !bot->IsAlive() ||
        bot->getClass() != CLASS_WARLOCK || PlayerbotAI::IsTank(bot))
        return false;
    EncounterState state = instance->GetBossState(Id(Aq40Encounter::Twins));
    if (state == IN_PROGRESS || state == DONE)
        return false;
    // Only inside the Twin Emperors room (and its entrance), never elsewhere in the temple.
    if (bot->GetExactDist2d(TWIN_ROOM_CENTER) > TWIN_ROOM_RADIUS || bot->GetPositionZ() < TWIN_ROOM_MIN_Z ||
        bot->GetPositionZ() > TWIN_ROOM_MAX_Z)
        return false;
    if (group->GetTargetIcon(TWIN_LOCK_ICON) == bot->GetGUID())
        spot = TWIN_MOON_PREPULL_SPOT;
    else if (group->GetTargetIcon(TWIN_SECOND_LOCK_ICON) == bot->GetGUID())
        spot = TWIN_LOCK_SPOTS[0];  // south warlock's waiting spot
    else
        return false;
    bot->UpdateAllowedPositionZ(spot.GetPositionX(), spot.GetPositionY(), spot.m_positionZ);
    return true;
}

namespace
{

// Out of combat, the tank marked with Moon walks alone from the entrance ramps to C'Thun's pull spot; the Eye pulls
// when it sees him. Only with the Twin Emperors dead (the Eye evades until then) and near the room.
bool CanPrepareCthunPull(Player* bot)
{
    InstanceScript* instance = bot->GetInstanceScript();
    if (bot->GetMapId() != AQ40_MAP_ID || !instance || !bot->GetGroup() || bot->IsInCombat() || !bot->IsAlive())
        return false;
    EncounterState state = instance->GetBossState(Id(Aq40Encounter::Cthun));
    return instance->GetBossState(Id(Aq40Encounter::Twins)) == DONE && state != IN_PROGRESS && state != DONE;
}

bool NearCthunRoom(Player* player)
{
    return player->GetExactDist2d(&CTHUN_CENTER) <= CTHUN_PREPULL_REACH &&
           player->GetPositionZ() >= CTHUN_PREPULL_MIN_Z;
}

}  // namespace

bool GetCthunPullerPrepullSpot(PlayerbotAI* botAI, Position& spot)
{
    Player* bot = botAI->GetBot();
    if (!CanPrepareCthunPull(bot) || !NearCthunRoom(bot) ||
        bot->GetGroup()->GetTargetIcon(CTHUN_PULLER_ICON) != bot->GetGUID() || !PlayerbotAI::IsTank(bot))
        return false;
    spot = CTHUN_PULL_SPOT;
    return true;
}

// The rest of the raid follows the puller down the slope, CTHUN_TRAIL_DISTANCE behind, so it is that much closer to
// its spots when the random Eye Beams start at about 17 s. Any closer and it reaches him during the opening beams.
bool GetCthunTrailSpot(PlayerbotAI* botAI, Position& spot)
{
    Player* bot = botAI->GetBot();
    if (!CanPrepareCthunPull(bot) || !NearCthunRoom(bot))
        return false;
    Group* group = bot->GetGroup();
    ObjectGuid pullerGuid = group->GetTargetIcon(CTHUN_PULLER_ICON);
    if (pullerGuid == bot->GetGUID())
        return false;
    // Only while he is walking in, so an idle Moon tank never holds the raid in place.
    Player* puller = ObjectAccessor::GetPlayer(*bot, pullerGuid);
    if (!puller || puller->GetGroup() != group || !puller->IsAlive() || puller->IsInCombat() || !puller->isMoving() ||
        !NearCthunRoom(puller) || !PlayerbotAI::IsTank(puller))
        return false;
    float distance = bot->GetExactDist2d(puller);
    float pullerRange = puller->GetExactDist2d(&CTHUN_CENTER);
    spot = bot->GetPosition();
    // Never walk to where the Eye could see a trailing bot before him.
    if (distance <= CTHUN_TRAIL_DISTANCE || bot->GetExactDist2d(&CTHUN_CENTER) < pullerRange + CTHUN_TRAIL_MARGIN)
        return true;
    float angle = puller->GetAngle(bot);
    float share = CTHUN_TRAIL_DISTANCE / distance;
    spot.Relocate(puller->GetPositionX() + CTHUN_TRAIL_DISTANCE * std::cos(angle),
                  puller->GetPositionY() + CTHUN_TRAIL_DISTANCE * std::sin(angle),
                  puller->GetPositionZ() + (bot->GetPositionZ() - puller->GetPositionZ()) * share);
    if (spot.GetExactDist2d(&CTHUN_CENTER) < pullerRange + CTHUN_TRAIL_MARGIN)
        spot = bot->GetPosition();
    return true;
}

}  // namespace Aq40Helpers
