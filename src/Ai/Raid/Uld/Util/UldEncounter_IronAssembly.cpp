/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "UldEncounter_IronAssembly.h"

#include "Creature.h"
#include "Group.h"
#include "Map.h"
#include "ObjectAccessor.h"
#include "ObjectGuid.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "RaidEncounter.h"
#include "RaidInstanceState.h"
#include "EncounterHelpers.h"
#include "RtiTargetValue.h"
#include "Spell.h"
#include "SpellAuras.h"
#include "Timer.h"
#include "TotemsShamanStrategy.h"
#include "UldEncounterGate.h"
#include "UldHardMode.h"
#include "UldScripts.h"
#include "UldTargets.h"
#include "Unit.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <list>
#include <set>
#include <string>
#include <tuple>
#include <unordered_map>
#include <unordered_set>

using namespace EncounterHelpers;

const Position ULDUAR_IRON_ASSEMBLY_ANCHOR = Position(1587.18f, 121.02f, 427.27f);

namespace
{

// Spread slots are held, not re-derived: ranking by guid every tick lets one death renumber everyone
// behind the corpse and shuffle the ring mid-fight.
struct IronAssemblyEncounterState
{
    std::unordered_map<ObjectGuid, uint8> spreadSlots{};

    // Shift-ring heading the raid stepped onto, or -1 on the stack. Latched: Overload walks with Brundir,
    // and re-picking every tick flips the winner mid-cast.
    int8 stackShiftHeading = -1;

    // When the council was first seen engaged with all three up, 0 between pulls. Opens the burst window.
    uint32 engagedAtMs = 0;

    // Carriers that started the walk to the Meltdown point. Latched so one slowed or detoured mid-walk
    // doesn't turn "late" and stop halfway, which is the worst place to blow up.
    GuidUnorderedSet meltdownRunners;

    // The shaman dropping Nature Resistance Totem. Latched until it dies or leaves, so the duty never
    // trades back and forth between two spares.
    ObjectGuid natureTotemShaman;
    uint32 natureTotemElectMs = 0;
};

// Keyed by instance, because trigger, action and multiplier each hold their own helper instance and
// the state they must agree on has to live in one place.
RaidInstanceState<IronAssemblyEncounterState> ironAssemblyStates;

IronAssemblyEncounterState& IronAssemblyStateFor(Player* bot)
{
    return ironAssemblyStates.For(bot->GetInstanceId());
}

// True when atMs already holds this tick's answer; otherwise stamps it and the caller refills.
bool FreshThisTick(uint32& atMs)
{
    uint32 const now = getMSTime();
    if (atMs && atMs == now)
        return true;

    atMs = now;
    return false;
}

Position IronAssemblyPositionAt(float bearing, float radius)
{
    return Position(ULDUAR_IRON_ASSEMBLY_ANCHOR.GetPositionX() + std::cos(bearing) * radius,
                    ULDUAR_IRON_ASSEMBLY_ANCHOR.GetPositionY() + std::sin(bearing) * radius,
                    ULDUAR_IRON_ASSEMBLY_ANCHOR.GetPositionZ());
}

bool IronAssemblyHasEitherAura(Unit const* unit, uint32 first, uint32 second)
{
    return unit && (unit->HasAura(first) || unit->HasAura(second));
}

// Ranged and healers take spread slots and stack points; tanks hold their own boss and melee ride
// whatever the raid is killing.
bool IronAssemblyTakesRaidSpot(PlayerbotAI* botAI, Player* member)
{
    return member && botAI && PlayerbotAI::IsRanged(member) && !PlayerbotAI::IsTank(member);
}

bool IronAssemblyMemberCounts(Player* member, uint32 instanceId)
{
    return member && member->IsAlive() && member->GetMapId() == ULDUAR_MAP_ID &&
           member->GetInstanceId() == instanceId;
}

uint32 IronAssemblyOverwhelmingPowerLeftMs(Unit const* unit)
{
    if (!unit)
        return 0;

    for (uint32 spellId : {SPELL_OVERWHELMING_POWER_25_MAN, SPELL_OVERWHELMING_POWER_10_MAN})
        if (Aura const* aura = unit->GetAura(spellId))
            return static_cast<uint32>(std::max(aura->GetDuration(), 1));

    return 0;
}

// The carrier is still up but another grouped tank has Steelbreaker. Facts every bot reads the same way.
bool IronAssemblySwappedOut(Player* member, Unit* steelbreaker)
{
    if (!member || !steelbreaker || !IronAssemblyOverwhelmingPowerLeftMs(member))
        return false;

    Unit* const victim = steelbreaker->GetVictim();
    Player* const holder = victim ? victim->ToPlayer() : nullptr;
    return holder && holder != member && holder->GetGroup() == member->GetGroup() && PlayerbotAI::IsTank(holder);
}

// Brundir first, then the other two. His is the only spot that prevents damage, so he keeps a tank
// at every roster size; the rest are filled in whatever order is left.
void GatherIronAssemblyTankOrder(IronAssemblyTargets const& targets, std::vector<Unit*>& bosses)
{
    if (targets.brundir)
        bosses.push_back(targets.brundir);
    if (targets.steelbreaker)
        bosses.push_back(targets.steelbreaker);
    if (targets.molgeim)
        bosses.push_back(targets.molgeim);
}

// Bot tanks only, never GetGroupMainTank/GetGroupAssistTank: role flags count humans, and a slot bots
// can't fill leaves its boss loose. Guid rank renumbers on a tank death, exactly when it's wanted, and
// on a Meltdown swap: the carrier leaves, so the next bot tank becomes the new partner.
void GatherIronAssemblyTanks(PlayerbotAI* botAI, Player* bot, std::vector<Player*>& tanks)
{
    Group* group = bot->GetGroup();
    if (!group)
        return;

    uint32 const instanceId = bot->GetInstanceId();
    Unit* steelbreaker = GetIronAssemblyMember(botAI, NPC_STEELBREAKER);

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (IronAssemblyMemberCounts(member, instanceId) && GET_PLAYERBOT_AI(member) &&
            PlayerbotAI::IsTank(member) && !IronAssemblySwappedOut(member, steelbreaker))
            tanks.push_back(member);
    }

    std::sort(tanks.begin(), tanks.end(),
              [](Player* lhs, Player* rhs) { return lhs->GetGUID() < rhs->GetGUID(); });
}

// Also reports which point it picked, sparing the caller a second IronAssemblyBrundirIsLast sweep.
Position IronAssemblyStackPoint(PlayerbotAI* botAI, bool& brundirLast)
{
    // Brundir alone: the opening point would leave casters 38 yd off him. 3 yd past the anchor is 25 from
    // his spot, clear of Overload and in caster range.
    brundirLast = IronAssemblyBrundirIsLast(botAI);
    if (brundirLast)
        return IronAssemblyPositionAt(ULDUAR_IRON_ASSEMBLY_BRUNDIR_BEARING,
                                      ULDUAR_IRON_ASSEMBLY_BRUNDIR_LAST_STACK_RADIUS);

    return IronAssemblyPositionAt(ULDUAR_IRON_ASSEMBLY_STACK_BEARING,
                                  ULDUAR_IRON_ASSEMBLY_STACK_RADIUS);
}

void EnsureIronAssemblySpreadSlot(PlayerbotAI* botAI, Player* bot)
{
    IronAssemblyEncounterState& state = IronAssemblyStateFor(bot);

    Group* group = bot->GetGroup();
    if (!group)
    {
        state.spreadSlots[bot->GetGUID()] = 0;
        return;
    }

    uint32 const instanceId = bot->GetInstanceId();

    std::unordered_set<ObjectGuid> present;
    ObjectGuid natureHunter;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!IronAssemblyMemberCounts(member, instanceId) || !IronAssemblyTakesRaidSpot(botAI, member))
            continue;

        present.insert(member->GetGUID());
        if (PlayerbotAI* memberAI = GET_PLAYERBOT_AI(member))
            if (IsIronAssemblyNatureHunter(memberAI))
                natureHunter = member->GetGUID();
    }

    for (auto itr = state.spreadSlots.begin(); itr != state.spreadSlots.end();)
        itr = present.count(itr->first) ? std::next(itr) : state.spreadSlots.erase(itr);

    if (state.spreadSlots.count(bot->GetGUID()))
        return;

    std::set<uint8> taken;
    for (auto const& assignment : state.spreadSlots)
        taken.insert(assignment.second);

    // The two nature resistance sources sit across the ring. Their slots stay free while they're here,
    // anyone takes them otherwise.
    Player* totemShaman = IronAssemblyNatureTotemShaman(botAI);
    ObjectGuid const natureTotem =
        totemShaman && present.count(totemShaman->GetGUID()) ? totemShaman->GetGUID() : ObjectGuid::Empty;

    auto const reservedFor = [&](uint8 slot)
    {
        if (slot == ULDUAR_IRON_ASSEMBLY_NATURE_HUNTER_SLOT)
            return natureHunter;

        return slot == ULDUAR_IRON_ASSEMBLY_NATURE_TOTEM_SLOT ? natureTotem : ObjectGuid::Empty;
    };

    for (uint8 slot : {ULDUAR_IRON_ASSEMBLY_NATURE_HUNTER_SLOT, ULDUAR_IRON_ASSEMBLY_NATURE_TOTEM_SLOT})
    {
        if (reservedFor(slot) == bot->GetGUID() && !taken.count(slot))
        {
            state.spreadSlots[bot->GetGUID()] = slot;
            return;
        }
    }

    for (uint8 slot = 0; slot < ULDUAR_IRON_ASSEMBLY_SPREAD_SLOTS; ++slot)
    {
        ObjectGuid const owner = reservedFor(slot);
        if (!taken.count(slot) && (owner.IsEmpty() || owner == bot->GetGUID()))
        {
            state.spreadSlots[bot->GetGUID()] = slot;
            return;
        }
    }

    // More ranged than the ring holds. Overflow keeps the stack point, which is no worse than where
    // they would stand with no spread at all.
    state.spreadSlots[bot->GetGUID()] = ULDUAR_IRON_ASSEMBLY_SPREAD_SLOTS;
}

// Inside the hall: two float compares against a fixed point, which is what keeps every bot elsewhere
// in Ulduar off the sweeps behind the formation test.
bool IronAssemblyBotInHall(Player* bot)
{
    if (!bot)
        return false;

    if (bot->GetExactDist2d(&ULDUAR_IRON_ASSEMBLY_ANCHOR) > ULDUAR_IRON_ASSEMBLY_ARENA_RADIUS)
        return false;

    return std::fabs(bot->GetPositionZ() - ULDUAR_IRON_ASSEMBLY_ANCHOR.GetPositionZ()) <=
           ULDUAR_IRON_ASSEMBLY_ARENA_HEIGHT;
}

bool IronAssemblyTargetsEngaged(IronAssemblyTargets const& targets)
{
    return (targets.steelbreaker && targets.steelbreaker->IsInCombat()) ||
           (targets.molgeim && targets.molgeim->IsInCombat()) ||
           (targets.brundir && targets.brundir->IsInCombat());
}

bool IronAssemblyCouncilAtFullHealth(IronAssemblyTargets const& targets)
{
    if (targets.AliveCount() < 3)
        return false;

    for (Unit* member : {targets.steelbreaker, targets.molgeim, targets.brundir})
        if (member->GetHealth() < member->GetMaxHealth())
            return false;

    return true;
}

// Stamped here rather than by the reset node, which only fires for a bot holding a spread slot, so a wipe
// before Steelbreaker's phase would leave the last pull's stamp and shut the next pull's window.
void UpdateIronAssemblyEngage(Player* bot, IronAssemblyTargets const& targets)
{
    IronAssemblyEncounterState& state = IronAssemblyStateFor(bot);

    if (!IronAssemblyTargetsEngaged(targets))
    {
        if (IronAssemblyCouncilAtFullHealth(targets))
        {
            state.engagedAtMs = 0;
            state.meltdownRunners.clear();
        }

        return;
    }

    if (!state.engagedAtMs && targets.AliveCount() == 3)
        state.engagedAtMs = std::max<uint32>(getMSTime(), 1);
}

}  // namespace

uint8 IronAssemblyTargets::AliveCount() const
{
    return (steelbreaker ? 1 : 0) + (molgeim ? 1 : 0) + (brundir ? 1 : 0);
}

void IronAssemblyScan::RefreshMembers()
{
    steelbreaker.Clear();
    molgeim.Clear();
    brundir.Clear();

    // One pass for all three: each read of the list is a fresh grid sweep.
    GuidVector const units =
        botAI->GetAiObjectContext()->GetValue<GuidVector>("possible targets no los")->Get();

    for (ObjectGuid const& guid : units)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive())
            continue;

        // First match per entry, which is the one a lookup for that entry would have stopped on.
        switch (unit->GetEntry())
        {
            case NPC_STEELBREAKER:
                if (!steelbreaker)
                    steelbreaker = guid;
                break;
            case NPC_MOLGEIM:
                if (!molgeim)
                    molgeim = guid;
                break;
            case NPC_BRUNDIR:
                if (!brundir)
                    brundir = guid;
                break;
            default:
                break;
        }
    }
}

Unit* IronAssemblyScan::Resolve(ObjectGuid const& guid) const
{
    if (!guid)
        return nullptr;

    Unit* unit = botAI->GetUnit(guid);
    return unit && unit->IsAlive() ? unit : nullptr;
}

void IronAssemblyScan::Members(IronAssemblyTargets& targets)
{
    bool const refreshed = !FreshThisTick(membersAtMs);
    if (refreshed)
        RefreshMembers();

    targets.steelbreaker = Resolve(steelbreaker);
    targets.molgeim = Resolve(molgeim);
    targets.brundir = Resolve(brundir);

    // Off the scan's once-a-tick refresh, not a node's predicate, so it doesn't matter which node asks
    // first or what its gates let through. Every lookup in the hall lands here.
    if (refreshed && IronAssemblyBotInHall(botAI->GetBot()))
        UpdateIronAssemblyEngage(botAI->GetBot(), targets);
}

Unit* IronAssemblyScan::Member(uint32 entry)
{
    IronAssemblyTargets targets;
    Members(targets);

    switch (entry)
    {
        case NPC_STEELBREAKER:
            return targets.steelbreaker;
        case NPC_MOLGEIM:
            return targets.molgeim;
        case NPC_BRUNDIR:
            return targets.brundir;
        default:
            return GetUldUnitByEntry(botAI, entry);
    }
}

std::vector<Position> IronAssemblyScan::RunesOfDeath()
{
    if (!FreshThisTick(runesAtMs))
    {
        runes.clear();
        GatherIronAssemblyRunesOfDeath(botAI->GetBot(), runes);
    }

    return runes;
}

std::vector<Position> IronAssemblyScan::RunesOfPower()
{
    if (!FreshThisTick(runesOfPowerAtMs))
    {
        runesOfPower.clear();

        std::list<Creature*> found;
        botAI->GetBot()->GetCreatureListWithEntryInGrid(found, NPC_IRON_ASSEMBLY_RUNE_OF_POWER,
                                                        ULDUAR_IRON_ASSEMBLY_RUNE_OF_POWER_SEARCH_RADIUS);
        for (Creature* rune : found)
            if (rune && rune->IsAlive() && rune->HasAura(SPELL_RUNE_OF_POWER_PULSE))
                runesOfPower.push_back(rune->GetPosition());
    }

    return runesOfPower;
}

IronAssemblyScan& GetIronAssemblyScan(PlayerbotAI* botAI)
{
    return *botAI->GetAiObjectContext()->GetValue<IronAssemblyScan*>("iron assembly scan")->Get();
}

Unit* GetIronAssemblyMember(PlayerbotAI* botAI, uint32 entry)
{
    return GetIronAssemblyScan(botAI).Member(entry);
}

void GatherIronAssemblyTargets(PlayerbotAI* botAI, IronAssemblyTargets& targets)
{
    GetIronAssemblyScan(botAI).Members(targets);
}

bool IronAssemblyEncounterActive(PlayerbotAI* botAI)
{
    IronAssemblyTargets targets;
    GatherIronAssemblyTargets(botAI, targets);
    return targets.AliveCount() > 0;
}

bool IronAssemblyFormationActive(PlayerbotAI* botAI)
{
    if (!botAI)
        return false;

    // Room test first: it is two float compares against a fixed point, and it keeps every bot outside
    // the hall off the grid sweep the target gather costs.
    if (!IronAssemblyBotInHall(botAI->GetBot()))
        return false;

    IronAssemblyTargets targets;
    GatherIronAssemblyTargets(botAI, targets);

    return IronAssemblyTargetsEngaged(targets);
}

bool IronAssemblyBrundirIsLast(PlayerbotAI* botAI)
{
    IronAssemblyTargets targets;
    GatherIronAssemblyTargets(botAI, targets);
    return targets.brundir && !targets.steelbreaker && !targets.molgeim;
}

static Unit* DeriveIronAssemblyFocusTarget(PlayerbotAI* botAI, Player* bot)
{
    // A skull outranks the built-in order: bots never set it here, so it came from a person. Only a
    // living council member counts, or a stale mark would strand the raid.
    if (Group* group = bot->GetGroup())
    {
        ObjectGuid const skull = group->GetTargetIcon(RtiTargetValue::skullIndex);
        if (skull)
        {
            Unit* marked = botAI->GetUnit(skull);
            if (marked && marked->IsAlive() &&
                (marked->GetEntry() == NPC_STEELBREAKER || marked->GetEntry() == NPC_MOLGEIM ||
                 marked->GetEntry() == NPC_BRUNDIR))
                return marked;
        }
    }

    return GetIronAssemblyNextKillTarget(botAI);
}

Unit* IronAssemblyFocusTarget(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return nullptr;

    return DeriveIronAssemblyFocusTarget(botAI, bot);
}

// A lone tank still takes Brundir: the unheld two drift onto him anyway, so the assignment decides
// where the council parks, and Brundir's spot keeps his Overload 38 yd off the stack.
static Unit* DeriveIronAssemblyAssignedBoss(PlayerbotAI* botAI, Player* bot)
{
    std::vector<Player*> tanks;
    GatherIronAssemblyTanks(botAI, bot, tanks);

    size_t rank = tanks.size();
    for (size_t i = 0; i < tanks.size(); ++i)
        if (tanks[i] == bot)
            rank = i;

    if (rank >= tanks.size())
        return nullptr;

    IronAssemblyTargets targets;
    GatherIronAssemblyTargets(botAI, targets);

    // Empowered Steelbreaker is the one member two tanks share, for the Overwhelming Power hand-off.
    // Ranked, not IsMainTank: group flags count humans, so the two would disagree on the partners.
    if (IsSteelbreakerEmpowered(botAI, targets.steelbreaker, targets.molgeim, targets.brundir) &&
        targets.steelbreaker)
    {
        if (rank < 2)
            return targets.steelbreaker;

        return nullptr;
    }

    std::vector<Unit*> bosses;
    GatherIronAssemblyTankOrder(targets, bosses);
    if (bosses.empty())
        return nullptr;

    // Two tanks, three members: rank 0 isolates Brundir, rank 1 holds the focus. The third stays on the
    // generic threat table, towing it anywhere would only put it back in the stack.
    if (tanks.size() == 2 && bosses.size() > 2 && rank == 1)
    {
        Unit* focus = IronAssemblyFocusTarget(botAI);
        if (focus && focus != targets.brundir)
            return focus;
    }

    if (rank >= bosses.size())
        return nullptr;

    return bosses[rank];
}

Unit* IronAssemblyAssignedBoss(PlayerbotAI* botAI, Player* bot)
{
    if (!botAI || !bot || !PlayerbotAI::IsTank(bot))
        return nullptr;

    return DeriveIronAssemblyAssignedBoss(botAI, bot);

Unit* IronAssemblyUnheldMemberToTaunt(PlayerbotAI* botAI, Player* bot)
{
    if (!botAI || !bot || !PlayerbotAI::IsTank(bot) || !IronAssemblyFormationActive(botAI))
        return nullptr;

    std::vector<Player*> tanks;
    GatherIronAssemblyTanks(botAI, bot, tanks);
    if (tanks.size() != 2 || tanks.front() != bot)
        return nullptr;

    IronAssemblyTargets targets;
    GatherIronAssemblyTargets(botAI, targets);
    if (targets.AliveCount() != 3)
        return nullptr;

    Unit* const own = DeriveIronAssemblyAssignedBoss(botAI, tanks[0]);
    Unit* const partner = DeriveIronAssemblyAssignedBoss(botAI, tanks[1]);

    for (Unit* member : {targets.steelbreaker, targets.molgeim, targets.brundir})
    {
        if (member == own || member == partner)
            continue;

        // Off anyone in the group but a tank, pets included: a tank's pet holding it is still a loose boss.
        Unit* const victim = member->GetVictim();
        Player* const holder = victim ? victim->GetCharmerOrOwnerPlayerOrPlayerItself() : nullptr;
        if (!holder || holder == bot || holder->GetGroup() != bot->GetGroup())
            return nullptr;

        return victim == holder && PlayerbotAI::IsTank(holder) ? nullptr : member;
    }

    return nullptr;
}

// The designed spot and the bearing it sits on. False for anything but a council member.
static bool IronAssemblyDesignedTankSpot(Unit* boss, Position& spot, float& bearing)
{
    if (!boss)
        return false;

    float radius = 0.0f;
    switch (boss->GetEntry())
    {
        case NPC_BRUNDIR:
            bearing = ULDUAR_IRON_ASSEMBLY_BRUNDIR_BEARING;
            radius = ULDUAR_IRON_ASSEMBLY_BRUNDIR_RADIUS;
            break;
        case NPC_STEELBREAKER:
            bearing = ULDUAR_IRON_ASSEMBLY_STEELBREAKER_BEARING;
            radius = ULDUAR_IRON_ASSEMBLY_MELEE_BOSS_RADIUS;
            break;
        case NPC_MOLGEIM:
            bearing = ULDUAR_IRON_ASSEMBLY_MOLGEIM_BEARING;
            radius = ULDUAR_IRON_ASSEMBLY_MELEE_BOSS_RADIUS;
            break;
        default:
            return false;
    }

    spot = IronAssemblyPositionAt(bearing, radius);
    return true;
}

bool TryGetIronAssemblyBossRuneOfPower(PlayerbotAI* botAI, Unit* boss, Position& rune)
{
    Position designed;
    float bearing = 0.0f;
    if (!IronAssemblyDesignedTankSpot(boss, designed, bearing))
        return false;

    float best = ULDUAR_IRON_ASSEMBLY_RUNE_OF_POWER_OWNER_RADIUS;
    bool found = false;
    for (Position const& candidate : GetIronAssemblyScan(botAI).RunesOfPower())
    {
        float const distance = candidate.GetExactDist2d(designed.GetPositionX(), designed.GetPositionY());
        if (distance < best)
        {
            best = distance;
            rune = candidate;
            found = true;
        }
    }

    return found;
}

bool TryGetIronAssemblyBossTankSpot(PlayerbotAI* botAI, Unit* boss, Position& position)
{
    float bearing = 0.0f;
    if (!IronAssemblyDesignedTankSpot(boss, position, bearing))
        return false;

    // Only his own rune moves him. One anywhere else is the soakers' business.
    Position rune;
    if (!TryGetIronAssemblyBossRuneOfPower(botAI, boss, rune))
        return true;

    // Back away from the rune. He re-chases once the tank leaves his melee range and stops at both reaches
    // plus contact, so he ends OFFSET off the rune with it behind him, where melee can stand. Off the
    // rune, the designed spot and the stack point, all fixed, so the spot holds still while he walks.
    float const dx = position.GetPositionX() - rune.GetPositionX();
    float const dy = position.GetPositionY() - rune.GetPositionY();
    float const away = dx * dx + dy * dy < 0.25f ? bearing : std::atan2(dy, dx);
    float const reach = ULDUAR_IRON_ASSEMBLY_RUNE_OF_POWER_BOSS_OFFSET + boss->GetCombatReach() +
                        DEFAULT_COMBAT_REACH + CONTACT_DISTANCE;

    // Straight away can leave the casters behind: a rune on Brundir's own spot backed him out to 45 yd
    // from the stack. So turn, + first, to the first heading where both ends of where he can stop sit in
    // his window. A rune too far out or too near the stack for any: the heading that misses it least.
    bool brundirLast = false;
    Position const stack = IronAssemblyStackPoint(botAI, brundirLast);
    float const minStack = boss->GetEntry() == NPC_BRUNDIR ? ULDUAR_IRON_ASSEMBLY_RUNE_SHIFT_BRUNDIR_MIN_STACK
                                                           : ULDUAR_IRON_ASSEMBLY_RUNE_SHIFT_MELEE_MIN_STACK;
    float const nearStop = ULDUAR_IRON_ASSEMBLY_RUNE_OF_POWER_BOSS_OFFSET;
    float const farStop = nearStop + ULDUAR_IRON_ASSEMBLY_RUNE_OF_POWER_BOSS_OVERSHOOT;
    auto const miss = [&](float heading)
    {
        float yards = 0.0f;
        for (float off : {nearStop, farStop})
        {
            float const fromStack = stack.GetExactDist2d(rune.GetPositionX() + std::cos(heading) * off,
                                                         rune.GetPositionY() + std::sin(heading) * off);
            yards += std::max(minStack - fromStack, 0.0f) +
                     std::max(fromStack - ULDUAR_IRON_ASSEMBLY_RUNE_SHIFT_MAX_STACK, 0.0f);
        }

        return yards;
    };

    float heading = away;
    float best = miss(away);
    float const quarter = static_cast<float>(M_PI) / 4.0f;
    for (int8 turn : {1, -1, 2, -2, 3, -3, 4})
    {
        if (best <= 0.0f)
            break;

        float const candidate = away + quarter * turn;
        float const yards = miss(candidate);
        if (yards < best)
        {
            heading = candidate;
            best = yards;
        }
    }

    // navprobe: every tank spot and stop this yields, runes out to 14.5 yd round each designed spot on 8
    // bearings, is on the floor.
    position = Position(rune.GetPositionX() + std::cos(heading) * reach,
                        rune.GetPositionY() + std::sin(heading) * reach, position.GetPositionZ());
    return true;
}

bool TryGetIronAssemblyTankSpot(PlayerbotAI* botAI, Player* bot, Position& position)
{
    return TryGetIronAssemblyBossTankSpot(botAI, IronAssemblyAssignedBoss(botAI, bot), position);
}

// Somewhere the stack point may not sit, and how far away "clear" is. Overload and Rune of Death do
// not share a clearance, so each carries its own.
struct IronAssemblyStackHazard
{
    Position centre;
    float clearance;
};

static void GatherIronAssemblyStackHazards(PlayerbotAI* botAI, Position const& stack,
                                           std::vector<IronAssemblyStackHazard>& hazards)
{
    std::vector<Position> const swept = GetIronAssemblyScan(botAI).RunesOfDeath();

    // Filtered against the stack, not the caller, so bots standing apart weigh the same runes. Each bot
    // still sweeps from where it stands, fine while the raid is stacked round this point.
    for (Position const& rune : swept)
        if (rune.GetExactDist2d(stack.GetPositionX(), stack.GetPositionY()) <
            ULDUAR_IRON_ASSEMBLY_RUNE_OF_DEATH_SEARCH_RADIUS)
            hazards.push_back({rune, ULDUAR_IRON_ASSEMBLY_RUNE_OF_DEATH_CLEARANCE});

    // Overload rides Brundir, so an untanked or badly parked Brundir puts it on the stack.
    Unit* brundir = GetIronAssemblyMember(botAI, NPC_BRUNDIR);
    if (brundir && IronAssemblyOverloadActive(brundir))
        hazards.push_back({brundir->GetPosition(), ULDUAR_IRON_ASSEMBLY_OVERLOAD_CLEARANCE});
}

static bool IsIronAssemblyStackSpotClear(Position const& spot,
                                         std::vector<IronAssemblyStackHazard> const& hazards)
{
    for (IronAssemblyStackHazard const& hazard : hazards)
        if (hazard.centre.GetExactDist2d(spot.GetPositionX(), spot.GetPositionY()) < hazard.clearance)
            return false;

    return true;
}

// Without this, raid position and escape cancel every tick on a covered stack. Reads only the stack
// and hazards, never the caller: every bot derives it, and a caller-dependent answer scatters the raid.
static Position DisplaceIronAssemblyStackPoint(PlayerbotAI* botAI, Player* bot, Position const& stack)
{
    std::vector<IronAssemblyStackHazard> hazards;
    GatherIronAssemblyStackHazards(botAI, stack, hazards);

    IronAssemblyEncounterState& state = IronAssemblyStateFor(bot);

    if (IsIronAssemblyStackSpotClear(stack, hazards))
    {
        state.stackShiftHeading = -1;
        return stack;
    }

    // Keep the latched heading while it clears, or a walking Overload flips the winner mid-cast.
    if (state.stackShiftHeading >= 0 && state.stackShiftHeading < ULDUAR_IRON_ASSEMBLY_STACK_SHIFT_HEADINGS)
    {
        float const bearing = 2.0f * static_cast<float>(M_PI) *
                              static_cast<float>(state.stackShiftHeading) /
                              static_cast<float>(ULDUAR_IRON_ASSEMBLY_STACK_SHIFT_HEADINGS);
        Position const held = IronAssemblyPositionAt(bearing, ULDUAR_IRON_ASSEMBLY_STACK_SHIFT_RADIUS);
        if (IsIronAssemblyStackSpotClear(held, hazards))
            return held;
    }

    // Nearest clear heading, so the raid gives up as little range on the bosses as it can.
    Position best;
    float bestTravel = 0.0f;
    int8 bestHeading = -1;
    for (uint8 heading = 0; heading < ULDUAR_IRON_ASSEMBLY_STACK_SHIFT_HEADINGS; ++heading)
    {
        float const bearing = 2.0f * static_cast<float>(M_PI) * static_cast<float>(heading) /
                              static_cast<float>(ULDUAR_IRON_ASSEMBLY_STACK_SHIFT_HEADINGS);
        Position const candidate = IronAssemblyPositionAt(bearing, ULDUAR_IRON_ASSEMBLY_STACK_SHIFT_RADIUS);

        if (!IsIronAssemblyStackSpotClear(candidate, hazards))
            continue;

        float const travel = stack.GetExactDist2d(candidate.GetPositionX(), candidate.GetPositionY());
        if (bestHeading < 0 || travel < bestTravel)
        {
            best = candidate;
            bestTravel = travel;
            bestHeading = static_cast<int8>(heading);
        }
    }

    // Every heading covered: hold formation and let the escape action walk each bot out on its own.
    // Scattering the raid to chase a spot that does not exist costs more than the ticks do.
    if (bestHeading < 0)
    {
        state.stackShiftHeading = -1;
        return stack;
    }

    state.stackShiftHeading = bestHeading;
    return best;
}

// Squeezes every slot bearing into the arc the runes leave clear, keeping spacing even. Rotating only
// the covered ones piles a side onto one point, just what Static Disruption wants. False if all clear or covered.
static bool SqueezeIronAssemblyRingOffRunes(Position const& centre, float radius,
                                            std::vector<Position> const& runes, float& bearing)
{
    if (runes.empty())
        return false;

    constexpr uint8 samples = ULDUAR_IRON_ASSEMBLY_SPREAD_RING_SAMPLES;
    float const sampleStep = 2.0f * static_cast<float>(M_PI) / static_cast<float>(samples);

    std::array<bool, samples> clear{};
    uint8 clearCount = 0;
    for (uint8 i = 0; i < samples; ++i)
    {
        float const sampleBearing = sampleStep * static_cast<float>(i);
        Position const point(centre.GetPositionX() + std::cos(sampleBearing) * radius,
                             centre.GetPositionY() + std::sin(sampleBearing) * radius, centre.GetPositionZ());

        clear[i] = IsIronAssemblyPositionClearOfRunes(point, runes, ULDUAR_IRON_ASSEMBLY_RUNE_OF_DEATH_CLEARANCE);
        if (clear[i])
            ++clearCount;
    }

    if (clearCount == 0 || clearCount == samples)
        return false;

    // Longest covered run. Walked from just past a clear sample, so a run wrapping through 0 counts whole.
    uint8 origin = 0;
    while (!clear[origin])
        ++origin;

    uint8 coveredStart = 0;
    uint8 coveredLength = 0;
    uint8 runStart = 0;
    uint8 runLength = 0;
    for (uint8 k = 1; k <= samples; ++k)
    {
        uint8 const i = static_cast<uint8>((origin + k) % samples);
        if (clear[i])
        {
            runLength = 0;
            continue;
        }

        if (runLength == 0)
            runStart = i;

        if (++runLength > coveredLength)
        {
            coveredStart = runStart;
            coveredLength = runLength;
        }
    }

    // Cut mid covered run and stretch the rest over the clear arc, so the opposite slot barely moves. A
    // second rune can still cover a slot inside the arc, raid position holds that bot.
    float const cut = sampleStep * (static_cast<float>(coveredStart) + (coveredLength - 1) * 0.5f);
    float const arcStart = sampleStep * static_cast<float>(coveredStart + coveredLength);
    float const arcSpan = sampleStep * static_cast<float>(samples - coveredLength - 1);

    float const fromCut = Position::NormalizeOrientation(bearing - cut);
    bearing = arcStart + arcSpan * fromCut / (2.0f * static_cast<float>(M_PI));
    return true;
}

// A Rune of Power the ranged can stand in: within the travel cap of the opening stack point (so never
// Brundir's), clear of every Rune of Death at the stand radius, and outside a live Overload. Nearest to
// the stack wins. Reads no caller position, so every bot picks the same rune.
static bool TryGetIronAssemblySoakRune(PlayerbotAI* botAI, Position& rune)
{
    std::vector<Position> const runesOfPower = GetIronAssemblyScan(botAI).RunesOfPower();
    if (runesOfPower.empty())
        return false;

    Position const opening =
        IronAssemblyPositionAt(ULDUAR_IRON_ASSEMBLY_STACK_BEARING, ULDUAR_IRON_ASSEMBLY_STACK_RADIUS);
    std::vector<Position> const runesOfDeath = GetIronAssemblyScan(botAI).RunesOfDeath();

    Unit* brundir = GetIronAssemblyMember(botAI, NPC_BRUNDIR);
    bool const overload = brundir && IronAssemblyOverloadActive(brundir);

    float best = ULDUAR_IRON_ASSEMBLY_RUNE_OF_POWER_SOAK_MAX_TRAVEL;
    bool found = false;
    for (Position const& candidate : runesOfPower)
    {
        float const travel = candidate.GetExactDist2d(opening.GetPositionX(), opening.GetPositionY());
        if (travel >= best)
            continue;

        if (!IsIronAssemblyPositionClearOfRunes(candidate, runesOfDeath, ULDUAR_IRON_ASSEMBLY_RUNE_OF_DEATH_CLEARANCE))
            continue;

        if (overload && brundir->GetExactDist2d(candidate.GetPositionX(), candidate.GetPositionY()) <
                            ULDUAR_IRON_ASSEMBLY_OVERLOAD_CLEARANCE)
            continue;

        best = travel;
        rune = candidate;
        found = true;
    }

    return found;
}

bool IronAssemblyTakesRuneOfPower(Player* bot)
{
    return bot && !PlayerbotAI::IsHeal(bot) && bot->getClass() != CLASS_HUNTER;
}

bool IronAssemblyOnRuneOfPower(PlayerbotAI* botAI)
{
    Player* bot = botAI ? botAI->GetBot() : nullptr;
    if (!IronAssemblyTakesRuneOfPower(bot) || !IronAssemblyTakesRaidSpot(botAI, bot) ||
        !IronAssemblyFormationActive(botAI) || IsSteelbreakerEmpowered(botAI))
        return false;

    Position rune;
    return TryGetIronAssemblySoakRune(botAI, rune);
}

static bool DeriveIronAssemblyRaidSpot(PlayerbotAI* botAI, Player* bot, Position& position, float& tolerance)
{
    bool brundirLast = false;
    Position const opening = IronAssemblyStackPoint(botAI, brundirLast);
    Position const stack = DisplaceIronAssemblyStackPoint(botAI, bot, opening);

    IronAssemblyTargets targets;
    GatherIronAssemblyTargets(botAI, targets);

    // Spread only for Static Disruption, which needs Steelbreaker's phase 2. Otherwise stack, which keeps
    // healers in range and makes Rune of Power worth soaking.
    if (!IsSteelbreakerEmpowered(botAI, targets.steelbreaker, targets.molgeim, targets.brundir))
    {
        // The rune is the stack while it lasts, so no second mover has to trade the move with this one.
        // Not in the spread phase: one Static Disruption on a stack is +75% nature taken for 20s, and
        // High Voltage is nature.
        Position rune;
        if (IronAssemblyTakesRuneOfPower(bot) && TryGetIronAssemblySoakRune(botAI, rune))
        {
            position = rune;
            tolerance = ULDUAR_IRON_ASSEMBLY_RUNE_OF_POWER_SLOT_TOLERANCE;
            return true;
        }

        position = stack;
        return true;
    }

    EnsureIronAssemblySpreadSlot(botAI, bot);

    IronAssemblyEncounterState const& state = IronAssemblyStateFor(bot);
    auto const assignment = state.spreadSlots.find(bot->GetGUID());
    if (assignment == state.spreadSlots.end() || assignment->second >= ULDUAR_IRON_ASSEMBLY_SPREAD_SLOTS)
    {
        position = stack;
        return true;
    }

    float bearing = 2.0f * static_cast<float>(M_PI) * static_cast<float>(assignment->second) /
                    static_cast<float>(ULDUAR_IRON_ASSEMBLY_SPREAD_SLOTS);

    // On Steelbreaker's spot, since Meltdown goes off on his tank. Doesn't ride the stack displacement,
    // both bosses it dodges are dead by now. A rune outliving Molgeim squeezes the slots instead.
    Position centre = stack;
    float ringRadius = ULDUAR_IRON_ASSEMBLY_SPREAD_RING_RADIUS;
    Position steelbreakerSpot;
    if (TryGetIronAssemblyBossTankSpot(botAI, targets.steelbreaker, steelbreakerSpot))
    {
        centre = steelbreakerSpot;
        ringRadius = ULDUAR_IRON_ASSEMBLY_EMPOWERED_SPREAD_RING_RADIUS;
        SqueezeIronAssemblyRingOffRunes(centre, ringRadius, GetIronAssemblyScan(botAI).RunesOfDeath(), bearing);
    }

    position = Position(centre.GetPositionX() + std::cos(bearing) * ringRadius,
                        centre.GetPositionY() + std::sin(bearing) * ringRadius,
                        centre.GetPositionZ());
    return true;
}

bool TryGetIronAssemblyRaidSpot(PlayerbotAI* botAI, Player* bot, Position& position, float* tolerance)
{
    if (!IronAssemblyTakesRaidSpot(botAI, bot))
        return false;

    float spotTolerance = ULDUAR_IRON_ASSEMBLY_SLOT_TOLERANCE;
    bool const found = DeriveIronAssemblyRaidSpot(botAI, bot, position, spotTolerance);
    if (tolerance)
        *tolerance = spotTolerance;

    return found;
}

bool IronAssemblyOverloadActive(Unit* brundir)
{
    return IronAssemblyHasEitherAura(brundir, SPELL_OVERLOAD_10_MAN, SPELL_OVERLOAD_25_MAN);
}

bool IronAssemblyTendrilsActive(Unit* brundir)
{
    return IronAssemblyHasEitherAura(brundir, SPELL_LIGHTNING_TENDRILS_10_MAN,
                                     SPELL_LIGHTNING_TENDRILS_25_MAN);
}

bool IronAssemblyLightningWhirlActive(Unit* brundir)
{
    return IronAssemblyHasEitherAura(brundir, SPELL_LIGHTNING_WHIRL_10_MAN, SPELL_LIGHTNING_WHIRL_25_MAN);
}

bool IronAssemblyChainLightningCasting(Unit* brundir)
{
    if (!brundir || !brundir->HasUnitState(UNIT_STATE_CASTING))
        return false;

    return brundir->FindCurrentSpellBySpellId(SPELL_CHAIN_LIGHTNING_10_MAN) ||
           brundir->FindCurrentSpellBySpellId(SPELL_CHAIN_LIGHTNING_25_MAN);
}

void GatherIronAssemblyRunesOfDeath(Player* bot, std::vector<Position>& runes)
{
    if (!bot)
        return;

    // One visit for both ids rather than one each. The two are interleaved in the result instead of
    // grouped, and nothing reads the order: every caller asks whether a spot clears all of them.
    std::vector<Position> const found = GetDynamicObjectPositions(
        bot, ULDUAR_IRON_ASSEMBLY_RUNE_OF_DEATH_SEARCH_RADIUS,
        {SPELL_RUNE_OF_DEATH_10_MAN, SPELL_RUNE_OF_DEATH_25_MAN});

    runes.insert(runes.end(), found.begin(), found.end());
}

bool IsIronAssemblyPositionClearOfRunes(Position const& spot, std::vector<Position> const& runes,
                                        float radius)
{
    for (Position const& rune : runes)
        if (rune.GetExactDist2d(spot.GetPositionX(), spot.GetPositionY()) < radius)
            return false;

    return true;
}

// The next detour point round the first rune on the straight line from the bot to goal, the short
// way. False when the line stays out of ROUTE.
static bool TryGetIronAssemblyRuneDetour(Player* bot, Position const& goal, std::vector<Position> const& runes,
                                         Position& waypoint)
{
    float const fromX = bot->GetPositionX();
    float const fromY = bot->GetPositionY();
    float const dx = goal.GetPositionX() - fromX;
    float const dy = goal.GetPositionY() - fromY;
    float const lengthSq = dx * dx + dy * dy;

    Position const* blocking = nullptr;
    float blockingAlong = 0.0f;
    for (Position const& rune : runes)
    {
        float along = 0.0f;
        if (lengthSq > 0.0f)
            along = std::clamp(
                ((rune.GetPositionX() - fromX) * dx + (rune.GetPositionY() - fromY) * dy) / lengthSq, 0.0f, 1.0f);

        // Nearest point is the bot itself, so the walk only moves away. Counting it as blocked spirals a bot
        // already inside ROUTE inward.
        if (along <= 0.0f)
            continue;

        if (rune.GetExactDist2d(fromX + along * dx, fromY + along * dy) >=
            ULDUAR_IRON_ASSEMBLY_RUNE_OF_DEATH_ROUTE_RADIUS)
            continue;

        if (!blocking || along < blockingAlong)
        {
            blocking = &rune;
            blockingAlong = along;
        }
    }

    if (!blocking)
        return false;

    float const runeX = blocking->GetPositionX();
    float const runeY = blocking->GetPositionY();
    float const botBearing = std::atan2(fromY - runeY, fromX - runeX);
    float const goalBearing = std::atan2(goal.GetPositionY() - runeY, goal.GetPositionX() - runeX);

    // Short way round. Walking it moves the bot's bearing toward the goal's, so it never flips mid detour.
    float const step = ULDUAR_IRON_ASSEMBLY_RUNE_DETOUR_STEP;
    float const turn = Position::NormalizeOrientation(goalBearing - botBearing);
    float const direction = turn <= static_cast<float>(M_PI) ? 1.0f : -1.0f;

    // At least a quarter step ahead, or a bot that just reached a point picks the one it's standing on.
    float const ahead = botBearing + direction * step * 0.25f;
    float const next = direction > 0.0f ? (std::floor(ahead / step) + 1.0f) * step
                                        : (std::ceil(ahead / step) - 1.0f) * step;

    // An off-centre rune puts part of this circle off the floor. The check pulls the point back along the
    // bot's line, still outside DANGER. Static, since nothing collides a walking bot with a gameobject.
    waypoint = ValidateStaticFloorPoint(
        bot, Position(runeX + std::cos(next) * ULDUAR_IRON_ASSEMBLY_RUNE_OF_DEATH_CLEARANCE,
                      runeY + std::sin(next) * ULDUAR_IRON_ASSEMBLY_RUNE_OF_DEATH_CLEARANCE, bot->GetPositionZ()));
    return true;
}

bool TryGetIronAssemblyRouteStep(PlayerbotAI* botAI, Player* bot, Position const& goal, Position& step)
{
    std::vector<Position> const runes = GetIronAssemblyScan(botAI).RunesOfDeath();
    if (!IsIronAssemblyPositionClearOfRunes(goal, runes, ULDUAR_IRON_ASSEMBLY_RUNE_OF_DEATH_ROUTE_RADIUS))
        return false;

    Position waypoint;
    if (!TryGetIronAssemblyRuneDetour(bot, goal, runes, waypoint))
    {
        step = goal;
        return true;
    }

    // Only the first rune on the line is steered round, so with two down the point round one can sit
    // in the other. Holding outside both beats walking into it.
    if (!IsIronAssemblyPositionClearOfRunes(waypoint, runes, ULDUAR_IRON_ASSEMBLY_RUNE_OF_DEATH_ROUTE_RADIUS))
        return false;

    step = waypoint;
    return true;
}

static bool DeriveIronAssemblyMeleeRuneSpot(PlayerbotAI* botAI, Player* bot, Position& spot)
{
    Unit* focus = IronAssemblyFocusTarget(botAI);
    Position rune;
    if (!focus || !TryGetIronAssemblyBossRuneOfPower(botAI, focus, rune))
        return false;

    // Still on the rune's centre: his tank hasn't backed him off yet, so there's no side to stand on.
    float const dx = focus->GetPositionX() - rune.GetPositionX();
    float const dy = focus->GetPositionY() - rune.GetPositionY();
    float const offset = std::sqrt(dx * dx + dy * dy);
    if (offset < 0.5f)
        return false;

    // Deeper when he stopped further off, so a boss that overshot his tank stays in reach. Half a yard
    // inside the reach test below, or rounding fails a spot sitting right on it.
    float const reach = bot->GetMeleeRange(focus) - 1.0f;
    float const depth = std::clamp(offset - reach + 0.5f, ULDUAR_IRON_ASSEMBLY_RUNE_OF_POWER_MELEE_DEPTH,
                                   ULDUAR_IRON_ASSEMBLY_RUNE_OF_POWER_MELEE_DEPTH_MAX);
    spot = Position(rune.GetPositionX() + dx / offset * depth, rune.GetPositionY() + dy / offset * depth,
                    rune.GetPositionZ());

    if (focus->GetExactDist2d(spot.GetPositionX(), spot.GetPositionY()) > reach)
        return false;

    std::vector<Position> const runesOfDeath = GetIronAssemblyScan(botAI).RunesOfDeath();
    if (!IsIronAssemblyPositionClearOfRunes(spot, runesOfDeath, ULDUAR_IRON_ASSEMBLY_RUNE_OF_DEATH_CLEARANCE))
        return false;

    Unit* brundir = GetIronAssemblyMember(botAI, NPC_BRUNDIR);
    if (brundir && (IronAssemblyOverloadActive(brundir) || IronAssemblyTendrilsActive(brundir)))
        return false;

    float const fromRune = bot->GetExactDist2d(rune.GetPositionX(), rune.GetPositionY());
    if (fromRune <= ULDUAR_IRON_ASSEMBLY_RUNE_OF_POWER_MELEE_HOLD && bot->IsWithinMeleeRange(focus))
        return false;

    return true;
}

bool TryGetIronAssemblyMeleeRuneSpot(PlayerbotAI* botAI, Player* bot, Position& spot)
{
    if (!botAI || !bot || PlayerbotAI::IsRanged(bot) || PlayerbotAI::IsTank(bot))
        return false;

    return DeriveIronAssemblyMeleeRuneSpot(botAI, bot, spot);
}

Position IronAssemblyMeltdownPoint()
{
    return IronAssemblyPositionAt(ULDUAR_IRON_ASSEMBLY_MELTDOWN_POINT_BEARING,
                                  ULDUAR_IRON_ASSEMBLY_MELTDOWN_POINT_RADIUS);
}

static bool IronAssemblyWillBoost(Player* bot, float walk)
{
    return walk > ULDUAR_IRON_ASSEMBLY_NITRO_MIN_WALK && !bot->HasAura(NITRO_BOOSTS_SPEED_SPELL) &&
           NitroBoostsReady(bot);
}

bool IronAssemblyUseNitroBoosts(Player* bot, float walk)
{
    if (!bot || !IronAssemblyWillBoost(bot, walk))
        return false;

    // UseNitroBoosts refuses a bot mid-cast, and the walk breaks the cast anyway.
    bot->CastStop();
    return UseNitroBoosts(bot);
}

// Boots still to use run the first 5s at 2.5x. Once used, the bot's speed already carries them.
static uint32 IronAssemblyWalkMs(Player* bot, float walk)
{
    float const speed = std::max(bot->GetSpeed(MOVE_RUN), 1.0f);
    if (!IronAssemblyWillBoost(bot, walk))
        return static_cast<uint32>(walk / speed * IN_MILLISECONDS);

    float const fast = speed * NITRO_BOOSTS_SPEED_FACTOR;
    float const boosted = fast * NITRO_BOOSTS_DURATION_MS / IN_MILLISECONDS;
    if (walk <= boosted)
        return static_cast<uint32>(walk / fast * IN_MILLISECONDS);

    return NITRO_BOOSTS_DURATION_MS + static_cast<uint32>((walk - boosted) / speed * IN_MILLISECONDS);
}

// A runner keeps walking until the buff is gone, whatever the clock says by then.
static bool IronAssemblyMeltdownRunner(Player* member, Unit* steelbreaker)
{
    if (!member || !IronAssemblySwappedOut(member, steelbreaker))
        return false;

    IronAssemblyEncounterState const* state = ironAssemblyStates.Find(member->GetInstanceId());
    return state && state->meltdownRunners.count(member->GetGUID());
}

static IronAssemblyMeltdownRole DeriveIronAssemblyMeltdownRole(PlayerbotAI* botAI, Player* bot)
{
    IronAssemblyTargets targets;
    GatherIronAssemblyTargets(botAI, targets);

    Unit* steelbreaker = targets.steelbreaker;
    IronAssemblyEncounterState& state = IronAssemblyStateFor(bot);

    uint32 const left = IronAssemblyOverwhelmingPowerLeftMs(bot);
    if (!left)
        state.meltdownRunners.erase(bot->GetGUID());

    if (!IsSteelbreakerEmpowered(botAI, targets.steelbreaker, targets.molgeim, targets.brundir))
        return IronAssemblyMeltdownRole::None;

    if (left)
    {
        if (IronAssemblySwappedOut(bot, steelbreaker))
        {
            Position const point = IronAssemblyMeltdownPoint();
            float const distance = bot->GetExactDist2d(point.GetPositionX(), point.GetPositionY());

            if (!state.meltdownRunners.count(bot->GetGUID()))
            {
                uint32 const walkMs = IronAssemblyWalkMs(bot, distance) + ULDUAR_IRON_ASSEMBLY_MELTDOWN_WALK_SLACK_MS;
                if (walkMs > left)
                    return IronAssemblyMeltdownRole::Late;

                state.meltdownRunners.insert(bot->GetGUID());
            }

            return distance <= ULDUAR_IRON_ASSEMBLY_MELTDOWN_POINT_TOLERANCE ? IronAssemblyMeltdownRole::Parked
                                                                              : IronAssemblyMeltdownRole::Run;
        }

        if (left <= ULDUAR_IRON_ASSEMBLY_MELTDOWN_SWAP_LEAD_MS)
        {
            std::vector<Player*> tanks;
            GatherIronAssemblyTanks(botAI, bot, tanks);
            if (tanks.size() < 2)
                return IronAssemblyMeltdownRole::NoPartner;
        }

        return IronAssemblyMeltdownRole::Hold;
    }

    if (!steelbreaker || IronAssemblyAssignedBoss(botAI, bot) != steelbreaker)
        return IronAssemblyMeltdownRole::None;

    Unit* const victim = steelbreaker->GetVictim();
    Player* const holder = victim ? victim->ToPlayer() : nullptr;
    if (!holder || holder == bot || holder->GetGroup() != bot->GetGroup() || !PlayerbotAI::IsTank(holder))
        return IronAssemblyMeltdownRole::None;

    uint32 const holderLeft = IronAssemblyOverwhelmingPowerLeftMs(holder);
    return holderLeft && holderLeft <= ULDUAR_IRON_ASSEMBLY_MELTDOWN_SWAP_LEAD_MS ? IronAssemblyMeltdownRole::Taunt
                                                                                  : IronAssemblyMeltdownRole::None;
}

IronAssemblyMeltdownRole IronAssemblyMeltdownRoleFor(PlayerbotAI* botAI, Player* bot)
{
    if (!botAI || !bot || !PlayerbotAI::IsTank(bot) || !IronAssemblyFormationActive(botAI))
        return IronAssemblyMeltdownRole::None;

    return DeriveIronAssemblyMeltdownRole(botAI, bot);
}

bool IronAssemblyOpenerBurstWindowOpen(PlayerbotAI* botAI)
{
    IronAssemblyTargets targets;
    GatherIronAssemblyTargets(botAI, targets);
    if (targets.AliveCount() < 3)
        return false;

    IronAssemblyEncounterState const* state = ironAssemblyStates.Find(botAI->GetBot()->GetInstanceId());
    return state && state->engagedAtMs &&
           GetMSTimeDiffToNow(state->engagedAtMs) < ULDUAR_IRON_ASSEMBLY_OPENER_BURST_WINDOW_MS;
}

bool IronAssemblyMemberMustMove(PlayerbotAI* botAI, Player* member)
{
    if (!botAI || !member)
        return false;

    if (IronAssemblyMeltdownRunner(member, GetIronAssemblyMember(botAI, NPC_STEELBREAKER)))
        return true;

    if (Unit* brundir = GetIronAssemblyMember(botAI, NPC_BRUNDIR))
    {
        float const distance = member->GetDistance2d(brundir);

        if (IronAssemblyTendrilsActive(brundir) && distance < ULDUAR_IRON_ASSEMBLY_TENDRILS_CLEARANCE)
            return true;

        // Tanks hold through Overload on purpose, so they never count as committed to a walk.
        if (IronAssemblyOverloadActive(brundir) && !PlayerbotAI::IsTank(member) &&
            distance < ULDUAR_IRON_ASSEMBLY_OVERLOAD_CLEARANCE)
            return true;
    }

    // The cache belongs to the bot whose tick this is; anyone else is swept fresh, because their scan
    // was filled on their own tick and they have moved since.
    std::vector<Position> runes;
    if (member == botAI->GetBot())
        runes = GetIronAssemblyScan(botAI).RunesOfDeath();
    else
        GatherIronAssemblyRunesOfDeath(member, runes);

    return !IsIronAssemblyPositionClearOfRunes(member->GetPosition(), runes,
                                               ULDUAR_IRON_ASSEMBLY_RUNE_OF_DEATH_DANGER_RADIUS);
}

char const* IronAssemblyReadyInterrupt(Player* bot, Unit* target)
{
    if (!bot || !target)
        return nullptr;

    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
    if (!botAI)
        return nullptr;

    // No silences, Brundir is immune to MECHANIC_SILENCE. Stuns land, he's immune to neither STUN nor
    // INTERRUPT (the encounter's own achievement is built on stunning him).
    static char const* const interrupts[] = {"kick",         "pummel",
                                             "shield bash",  "counterspell",
                                             "wind shear",   "mind freeze",
                                             "hammer of justice", "bash",
                                             "kidney shot",  "shockwave"};

    for (char const* interrupt : interrupts)
        if (botAI->CanCastSpell(interrupt, target))
            return interrupt;

    return nullptr;
}

// Nothing stored: every bot reaches the same ranking from the same facts, so no two both stand down.
char const* IronAssemblyInterruptDuty(PlayerbotAI* botAI, Player* bot, Unit* brundir)
{
    if (!botAI || !bot || !brundir)
        return nullptr;

    bool const whirl = IronAssemblyLightningWhirlActive(brundir);
    bool const chainLightning = !whirl && IronAssemblyChainLightningCasting(brundir);
    if (!whirl && !chainLightning)
        return nullptr;

    if (!IronAssemblyReadyInterrupt(bot, brundir))
        return nullptr;

    // A bot already walking out of a hazard cannot cast, so it must not hold a duty either.
    if (IronAssemblyMemberMustMove(botAI, bot))
        return nullptr;

    // Past the rank that could hold this cast the answer is standby, so stop counting: each member costs
    // ten spell checks and, if one is ready, a rune sweep.
    uint8 const decided = whirl ? 1 : 2;

    uint8 rank = 0;
    if (Group* group = bot->GetGroup())
    {
        uint32 const instanceId = bot->GetInstanceId();

        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (member == bot || !IronAssemblyMemberCounts(member, instanceId))
                continue;

            if (member->GetGUID() < bot->GetGUID() && IronAssemblyReadyInterrupt(member, brundir) &&
                !IronAssemblyMemberMustMove(botAI, member))
            {
                if (++rank >= decided)
                    break;
            }
        }
    }

    // Whirl is 100 yd with no positional answer, so rank 0 saves its interrupt for it.
    if (whirl && rank == 0)
        return "whirl";

    if (chainLightning && rank == 1)
        return "chain";

    return nullptr;
}

bool IronAssemblyShieldOfRunesUp(Unit* molgeim)
{
    return IronAssemblyHasEitherAura(molgeim, SPELL_SHIELD_OF_RUNES_10_MAN, SPELL_SHIELD_OF_RUNES_25_MAN);
}

bool IronAssemblyHasFusionPunch(Unit* unit)
{
    return IronAssemblyHasEitherAura(unit, SPELL_FUSION_PUNCH_10_MAN, SPELL_FUSION_PUNCH_25_MAN);
}

bool IronAssemblyHeldByOtherTank(Player* bot, Unit* boss)
{
    Unit* const victim = boss ? boss->GetVictim() : nullptr;
    Player* const holder = victim ? victim->ToPlayer() : nullptr;
    return holder && holder != bot && holder->GetGroup() == bot->GetGroup() && PlayerbotAI::IsTank(holder);
}

uint32 IronAssemblyFusionPunchIncomingMs(PlayerbotAI* botAI, Player* bot)
{
    if (!botAI || !bot || !PlayerbotAI::IsTank(bot) || !IronAssemblyFormationActive(botAI) ||
        !IsSteelbreakerEmpowered(botAI))
        return 0;

    Unit* steelbreaker = GetIronAssemblyMember(botAI, NPC_STEELBREAKER);
    Spell* punch = steelbreaker ? steelbreaker->GetCurrentSpell(CURRENT_GENERIC_SPELL) : nullptr;
    if (!punch || punch->m_targets.GetUnitTarget() != bot)
        return 0;

    uint32 const spellId = punch->GetSpellInfo()->Id;
    if (spellId != SPELL_FUSION_PUNCH_10_MAN && spellId != SPELL_FUSION_PUNCH_25_MAN)
        return 0;

    return static_cast<uint32>(std::max(punch->GetCastTimeRemaining(), 1));
}

uint8 IronAssemblyResistWantedSchools(PlayerbotAI* botAI, uint8 /*previous*/)
{
    // The boss state, not this bot's view of the hall: one read stands for the whole instance, and a bot
    // running back from the graveyard must not take the hunter off it for everyone.
    return BossStateGateLive(botAI, ULD_BOSS_ASSEMBLY) ? RAID_RESIST_NATURE : 0;
}

bool IsIronAssemblyNatureHunter(PlayerbotAI* botAI)
{
    return botAI && IsRaidResistHunterHolding(botAI, ULD_IRON_ASSEMBLY_RESIST_AURA);
}

uint32 IronAssemblyNatureTotemSpell(Player* bot)
{
    if (!bot)
        return 0;

    for (uint32 spellId : NATURE_RESISTANCE_TOTEM)
        if (bot->HasSpell(spellId))
            return spellId;

    return 0;
}

// The air totem strategy a shaman bot runs, empty for none. Siblings, so a bot runs one at most.
static std::string IronAssemblyAirTotemStrategy(PlayerbotAI* memberAI)
{
    static char const* const strategies[] = {"nature resistance", "wrath of air", "windfury", "grounding"};
    for (char const* strategy : strategies)
        if (memberAI->HasStrategy(strategy, BOT_STATE_COMBAT))
            return strategy;

    return "";
}

// Bots only: a human shaman's totems are its own business, so one never counts as the spare's twin.
static Player* ElectIronAssemblyNatureTotemShaman(Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    struct Shaman
    {
        Player* player;
        std::string air;
    };

    std::vector<Shaman> shamans;
    uint32 const instanceId = bot->GetInstanceId();
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        PlayerbotAI* memberAI = member ? GET_PLAYERBOT_AI(member) : nullptr;
        if (memberAI && member->getClass() == CLASS_SHAMAN && IronAssemblyMemberCounts(member, instanceId) &&
            IronAssemblyNatureTotemSpell(member))
            shamans.push_back({member, IronAssemblyAirTotemStrategy(memberAI)});
    }

    // Already on it, then a ring slot holder, then guid.
    auto const rank = [](Shaman const& shaman)
    {
        return std::make_tuple(shaman.air == "nature resistance" ? 0 : 1,
                               PlayerbotAI::IsRanged(shaman.player) ? 0 : 1, shaman.player->GetGUID());
    };

    Shaman const* best = nullptr;
    for (Shaman const& shaman : shamans)
    {
        bool spare = shaman.air.empty() || shaman.air == "nature resistance";
        for (Shaman const& other : shamans)
            if (&other != &shaman && other.air == shaman.air)
                spare = true;

        if (spare && (!best || rank(shaman) < rank(*best)))
            best = &shaman;
    }

    return best ? best->player : nullptr;
}

Player* IronAssemblyNatureTotemShaman(PlayerbotAI* botAI)
{
    Player* bot = botAI ? botAI->GetBot() : nullptr;
    if (!bot)
        return nullptr;

    IronAssemblyEncounterState& state = IronAssemblyStateFor(bot);
    ObjectGuid const held = state.natureTotemShaman;
    Player* holder = held ? ObjectAccessor::FindPlayer(held) : nullptr;
    if (holder && holder->GetGroup() == bot->GetGroup() && IronAssemblyMemberCounts(holder, bot->GetInstanceId()))
        return holder;

    // One election per instance per ms: every shaman asks each tick, and the hold rule per action.
    if (FreshThisTick(state.natureTotemElectMs))
        return nullptr;

    holder = ElectIronAssemblyNatureTotemShaman(bot);
    state.natureTotemShaman = holder ? holder->GetGUID() : ObjectGuid::Empty;
    return holder;
}

bool IsIronAssemblyNatureTotemShaman(PlayerbotAI* botAI)
{
    // Class first: the hold rule asks this of every bot.
    Player* bot = botAI ? botAI->GetBot() : nullptr;
    return bot && bot->getClass() == CLASS_SHAMAN && IronAssemblyFormationActive(botAI) &&
           IronAssemblyNatureTotemShaman(botAI) == bot;
}

IronAssemblyNatureTotemStep IronAssemblyNatureTotemNext(PlayerbotAI* botAI)
{
    if (!IsIronAssemblyNatureTotemShaman(botAI))
        return IronAssemblyNatureTotemStep::None;

    Player* bot = botAI->GetBot();
    uint32 const spellId = IronAssemblyNatureTotemSpell(bot);
    if (!spellId)
        return IronAssemblyNatureTotemStep::None;

    ActionButton const* button = bot->GetActionButton(TOTEM_BAR_SLOT_AIR);
    if (!button || button->GetType() != ACTION_BUTTON_SPELL || button->GetAction() != spellId)
        return IronAssemblyNatureTotemStep::SetButton;

    // Dropped where it stops, not on the way there.
    if (bot->isMoving())
        return IronAssemblyNatureTotemStep::None;

    Creature* totem = bot->GetMap()->GetCreature(bot->m_SummonSlot[SUMMON_SLOT_TOTEM_AIR]);
    bool const placed = totem && totem->GetUInt32Value(UNIT_CREATED_BY_SPELL) == spellId &&
                        totem->GetDistance(bot) <= ULDUAR_IRON_ASSEMBLY_NATURE_TOTEM_REDROP;
    return placed ? IronAssemblyNatureTotemStep::None : IronAssemblyNatureTotemStep::Drop;
}

bool IronAssemblyEncounterStateIsStale(PlayerbotAI* botAI)
{
    IronAssemblyTargets targets;
    GatherIronAssemblyTargets(botAI, targets);

    // All three up and untouched is the pull. Testing combat would wipe the state every Lightning
    // Tendrils, when Brundir spends 16s with no victim by design.
    return IronAssemblyCouncilAtFullHealth(targets);
}

bool IronAssemblyBotHasEncounterState(Player* bot)
{
    if (!bot)
        return false;

    IronAssemblyEncounterState const* state = ironAssemblyStates.Find(bot->GetInstanceId());
    return state && state->spreadSlots.count(bot->GetGUID()) > 0;
}

void ResetIronAssemblyEncounterState(Player* bot, bool clearInstance)
{
    if (!bot)
        return;

    if (clearInstance)
    {
        ironAssemblyStates.Reset(bot->GetInstanceId());
        return;
    }

    IronAssemblyEncounterState* state = ironAssemblyStates.Find(bot->GetInstanceId());
    if (!state)
        return;

    state->spreadSlots.erase(bot->GetGUID());
}
