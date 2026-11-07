/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ULDENCOUNTERIRONASSEMBLY_H
#define PLAYERBOTS_ULDENCOUNTERIRONASSEMBLY_H

#include "ObjectGuid.h"
#include "Position.h"
#include "RaidResistAura.h"
#include "UldData.h"

#include <vector>

class Player;
class PlayerbotAI;
class Unit;

// Assembly of Iron. Killing one member restores the other two to full, so focus fire is the fight.

enum UlduarIronAssemblyIds
{
    // Unit::CastSpell difficulty-maps every id, so each pair is 10-man then 25-man and callers test both.
    SPELL_LIGHTNING_TENDRILS_10_MAN = 61887,
    SPELL_LIGHTNING_TENDRILS_25_MAN = 63486,
    SPELL_OVERLOAD_10_MAN = 61869,
    SPELL_OVERLOAD_25_MAN = 63481,
    SPELL_CHAIN_LIGHTNING_10_MAN = 61879,
    SPELL_CHAIN_LIGHTNING_25_MAN = 63479,
    SPELL_LIGHTNING_WHIRL_10_MAN = 61915,
    SPELL_LIGHTNING_WHIRL_25_MAN = 63483,
    SPELL_RUNE_OF_DEATH_10_MAN = 62269,
    SPELL_RUNE_OF_DEATH_25_MAN = 63490,
    SPELL_SHIELD_OF_RUNES_10_MAN = 62274,
    SPELL_SHIELD_OF_RUNES_25_MAN = 63489,
    SPELL_FUSION_PUNCH_10_MAN = 61903,
    SPELL_FUSION_PUNCH_25_MAN = 63493,
    SPELL_OVERWHELMING_POWER_10_MAN = 64637,
    SPELL_OVERWHELMING_POWER_25_MAN = 61888,
    // Rune of Power is a creature, not a DynamicObject: 63513 summons it at the target's feet. It carries
    // this aura for 60s, pulsing the +50% buff 64320 every 0.5s. A rune without the aura is spent.
    NPC_IRON_ASSEMBLY_RUNE_OF_POWER = 33705,
    SPELL_RUNE_OF_POWER_PULSE = 61974,
    // NPC_STEELBREAKER / NPC_MOLGEIM / NPC_BRUNDIR come from core ulduar.h via UldScripts.h
};

// Nothing anchor-centred sits past 30 yd, where the 45 and 135 degree diagonals lose the floor.

// Overload 61878 is 20,000 nature and a knockdown in 20 yd, Tendrils 61886/63485 3000/5000 a second in
// 18 (61884's 10 is a dummy). Clearance adds arrival slack: a bot stopping on the edge is still in.
inline constexpr float ULDUAR_IRON_ASSEMBLY_OVERLOAD_RADIUS = 20.0f;
inline constexpr float ULDUAR_IRON_ASSEMBLY_OVERLOAD_CLEARANCE = 25.0f;
inline constexpr float ULDUAR_IRON_ASSEMBLY_TENDRILS_RADIUS = 18.0f;
inline constexpr float ULDUAR_IRON_ASSEMBLY_TENDRILS_CLEARANCE = 28.0f;

// Rune of Death 62269/63490: a 13 yd persistent area aura ticking 2750 shadow every half second for
// 30s. Search wide enough to see one dropped anywhere in the formation.
inline constexpr float ULDUAR_IRON_ASSEMBLY_RUNE_OF_DEATH_RADIUS = 13.0f;

// DBC 13 yd, but the aura searcher adds object size at both ends (15.4 measured). Run inside DANGER,
// stand at CLEARANCE: with one radius the escape parks on the edge and the rune comes straight back.
inline constexpr float ULDUAR_IRON_ASSEMBLY_RUNE_OF_DEATH_DANGER_RADIUS = 16.0f;
inline constexpr float ULDUAR_IRON_ASSEMBLY_RUNE_OF_DEATH_CLEARANCE = 21.0f;
inline constexpr float ULDUAR_IRON_ASSEMBLY_RUNE_OF_DEATH_SEARCH_RADIUS = 40.0f;

// Ring and bearing steps of the escape's hazard sweep.
inline constexpr float ULDUAR_IRON_ASSEMBLY_ESCAPE_SWEEP_STEP = 2.0f;
inline constexpr float ULDUAR_IRON_ASSEMBLY_ESCAPE_SWEEP_ANGLE_STEP = static_cast<float>(M_PI) / 8.0f;

// Walks and goals inside ROUTE detour round the rune via fixed points on its CLEARANCE circle. Halfway,
// as a walk just clearing DANGER grazes it. Goals share it, or one between the radii never clears.
inline constexpr float ULDUAR_IRON_ASSEMBLY_RUNE_OF_DEATH_ROUTE_RADIUS = 18.5f;
inline constexpr float ULDUAR_IRON_ASSEMBLY_RUNE_DETOUR_STEP = 0.7854f;  // pi/4, chords stay 19.4 yd out

// Where the raid stands when a Rune of Death or Overload covers the stack point. A fixed set of anchor
// headings, since every bot picks on its own and they must agree.
inline constexpr uint8 ULDUAR_IRON_ASSEMBLY_STACK_SHIFT_HEADINGS = 8;
inline constexpr float ULDUAR_IRON_ASSEMBLY_STACK_SHIFT_RADIUS = 25.0f;

// Meltdown 61889: 29,250 nature to the carrier's allies within 15 yd, centre to centre, and an instakill on
// the carrier. The ranged ring is sized off it.
inline constexpr float ULDUAR_IRON_ASSEMBLY_MELTDOWN_RADIUS = 15.0f;

// The other tank taunts with this much Overwhelming Power left, so the carrier can walk out and blow up
// alone. The walk is 46 yd, about 6.6s, 2.6s on Nitro Boosts.
inline constexpr uint32 ULDUAR_IRON_ASSEMBLY_MELTDOWN_SWAP_LEAD_MS = 10000;
// Margin on top of the walk. With less left than walk plus this, the carrier stays put: dying halfway
// lands the blast on the ranged ring instead of the melee.
inline constexpr uint32 ULDUAR_IRON_ASSEMBLY_MELTDOWN_WALK_SLACK_MS = 1000;
// Where a carrier goes to die: anchor + 30 yd at 315 degrees, 46 yd from Steelbreaker's spot and 24 off the
// spread ring, empty once he's the last one up. navprobe: on the floor, straight path from his spot.
inline constexpr float ULDUAR_IRON_ASSEMBLY_MELTDOWN_POINT_BEARING = 5.4978f;  // 7*pi/4
inline constexpr float ULDUAR_IRON_ASSEMBLY_MELTDOWN_POINT_RADIUS = 30.0f;
inline constexpr float ULDUAR_IRON_ASSEMBLY_MELTDOWN_POINT_TOLERANCE = 2.0f;

// Phase 3 walks over this go on Nitro Boosts. Ranged spend them on the walk to their spread slot, tanks
// keep theirs for the Meltdown run, their one long walk in the phase.
inline constexpr float ULDUAR_IRON_ASSEMBLY_NITRO_MIN_WALK = 10.0f;

// Rune of Power buffs any unit whose centre is within 5 yd of the rune's: 2D, no combat reach on either
// side. Ranged soak a rune within the travel cap of the opening stack point, which takes every rune on
// Steelbreaker or Molgeim and none on Brundir, 38 yd off and inside Overload range.
inline constexpr float ULDUAR_IRON_ASSEMBLY_RUNE_OF_POWER_RADIUS = 5.0f;
inline constexpr float ULDUAR_IRON_ASSEMBLY_RUNE_OF_POWER_SOAK_MAX_TRAVEL = 25.0f;
inline constexpr float ULDUAR_IRON_ASSEMBLY_RUNE_OF_POWER_SEARCH_RADIUS = 50.0f;
// A rune within this of a boss's designed tank spot is his. The spots are at least 22.6 yd apart.
inline constexpr float ULDUAR_IRON_ASSEMBLY_RUNE_OF_POWER_OWNER_RADIUS = 15.0f;
// Boss centre to rune centre once his tank has backed him off it, when he stops at both reaches plus
// contact. Chasing a tank that left in one long step he can stop up to OVERSHOOT further out (8.4 traced).
inline constexpr float ULDUAR_IRON_ASSEMBLY_RUNE_OF_POWER_BOSS_OFFSET = 5.5f;
inline constexpr float ULDUAR_IRON_ASSEMBLY_RUNE_OF_POWER_BOSS_OVERSHOOT = 2.0f;
// Where a backed-off boss may end, by distance from the stack point. Brundir outside his Overload
// clearance and inside caster reach (casters at the stack chased him at 39, not at 36.8), the other two
// no nearer the stack than their designed spots.
inline constexpr float ULDUAR_IRON_ASSEMBLY_RUNE_SHIFT_BRUNDIR_MIN_STACK = 27.0f;
inline constexpr float ULDUAR_IRON_ASSEMBLY_RUNE_SHIFT_MELEE_MIN_STACK = 11.0f;
inline constexpr float ULDUAR_IRON_ASSEMBLY_RUNE_SHIFT_MAX_STACK = 36.0f;
// Melee stand this far into the rune toward the boss, deeper up to MAX when he parked further out, so he
// stays in reach and they stay 1 yd inside the edge.
inline constexpr float ULDUAR_IRON_ASSEMBLY_RUNE_OF_POWER_MELEE_DEPTH = 3.0f;
inline constexpr float ULDUAR_IRON_ASSEMBLY_RUNE_OF_POWER_MELEE_DEPTH_MAX = 4.0f;
inline constexpr float ULDUAR_IRON_ASSEMBLY_RUNE_OF_POWER_MELEE_HOLD = 4.5f;
// Raid spot deadband inside the rune. The default 3 lets a bot re-walk only past 6, outside it.
inline constexpr float ULDUAR_IRON_ASSEMBLY_RUNE_OF_POWER_SLOT_TOLERANCE = 2.0f;

// Hard mode holds burst for Steelbreaker's last phase, bar one round at the pull: this window, and only
// cooldowns of 3 min or less, which are back early in his phase (he's usually alone ~170s in).
inline constexpr uint32 ULDUAR_IRON_ASSEMBLY_OPENER_BURST_WINDOW_MS = 20000;
inline constexpr int32 ULDUAR_IRON_ASSEMBLY_OPENER_BURST_MAX_COOLDOWN_MS = 180000;

// Brundir parks at 28, not the 25 his Overload needs, so the stack 38 yd off never reacts to it and
// stays parked, free to kick Lightning Whirl.
inline constexpr float ULDUAR_IRON_ASSEMBLY_BRUNDIR_BEARING = 0.0f;
inline constexpr float ULDUAR_IRON_ASSEMBLY_BRUNDIR_RADIUS = 28.0f;
inline constexpr float ULDUAR_IRON_ASSEMBLY_STEELBREAKER_BEARING = 2.3562f;  // 3*pi/4
inline constexpr float ULDUAR_IRON_ASSEMBLY_MOLGEIM_BEARING = 3.9270f;       // 5*pi/4
inline constexpr float ULDUAR_IRON_ASSEMBLY_MELEE_BOSS_RADIUS = 16.0f;
inline constexpr float ULDUAR_IRON_ASSEMBLY_STACK_BEARING = 3.1416f;         // pi
inline constexpr float ULDUAR_IRON_ASSEMBLY_STACK_RADIUS = 10.0f;
// Once Brundir is the last one up his isolation protects nothing, so the raid closes to a second
// point 25 yd short of him: outside Overload, inside caster range of him.
inline constexpr float ULDUAR_IRON_ASSEMBLY_BRUNDIR_LAST_STACK_RADIUS = 3.0f;
inline constexpr float ULDUAR_IRON_ASSEMBLY_SLOT_TOLERANCE = 3.0f;

// Stops short of the hall doors, 84 and 86 yd from the anchor. Sight range is 100 yd with no LOS check,
// so without it the council drives bots from out in the corridor.
inline constexpr float ULDUAR_IRON_ASSEMBLY_ARENA_RADIUS = 78.0f;
inline constexpr float ULDUAR_IRON_ASSEMBLY_ARENA_HEIGHT = 10.0f;
// A tank that has drifted this far off its spot is walking, not parked.
inline constexpr float ULDUAR_IRON_ASSEMBLY_TANK_SPOT_TOLERANCE = 4.0f;

// Static Disruption targets someone beyond 10 yd and only exists from Steelbreaker's phase 2, hence a
// ranged, hard-mode-only spread. Fallback radius, for when he has no tank spot.
inline constexpr float ULDUAR_IRON_ASSEMBLY_SPREAD_RING_RADIUS = 18.0f;
inline constexpr uint8 ULDUAR_IRON_ASSEMBLY_SPREAD_SLOTS = 16;

// Centred on Steelbreaker's tank spot, so every slot clears Meltdown by 7 yd. Don't go past 23: at 24
// the 135 degree slot falls into a hole in the navmesh.
inline constexpr float ULDUAR_IRON_ASSEMBLY_EMPOWERED_SPREAD_RING_RADIUS = 22.0f;

// Molgeim's last rune outlives him on the stack point, inside this ring, and blocks about 140 degrees
// of it at CLEARANCE. The clear arc is found by sampling this many bearings.
inline constexpr uint8 ULDUAR_IRON_ASSEMBLY_SPREAD_RING_SAMPLES = 64;

// Ring slots of the two nature resistance sources. Across the ring from each other, their 30 yd auras
// reach every slot: the farthest is 31 yd off one, 44 * sin 45.
inline constexpr uint8 ULDUAR_IRON_ASSEMBLY_NATURE_HUNTER_SLOT = 0;
inline constexpr uint8 ULDUAR_IRON_ASSEMBLY_NATURE_TOTEM_SLOT = ULDUAR_IRON_ASSEMBLY_SPREAD_SLOTS / 2;
// The totem shaman re-drops once it stands this far from its totem, so the totem follows it to the stack,
// the rune or its ring slot rather than staying where the last spot was.
inline constexpr float ULDUAR_IRON_ASSEMBLY_NATURE_TOTEM_REDROP = 10.0f;

// Room centre, read off Brundir's out-of-combat wander in boss_assembly_of_iron.cpp. On mesh, floor 427.267.
extern const Position ULDUAR_IRON_ASSEMBLY_ANCHOR;

struct IronAssemblyTargets
{
    Unit* steelbreaker = nullptr;
    Unit* molgeim = nullptr;
    Unit* brundir = nullptr;

    uint8 AliveCount() const;
};

// By entry, never "find target": that walks only the bot's threat list by localized name, and Brundir
// wipes his threat table on every Tendrils landing, so it goes quietly inert.
Unit* GetIronAssemblyMember(PlayerbotAI* botAI, uint32 entry);
void GatherIronAssemblyTargets(PlayerbotAI* botAI, IronAssemblyTargets& targets);
bool IronAssemblyEncounterActive(PlayerbotAI* botAI);

// Per-bot cache for one tick: the value behind it is a 100 yd grid sweep and a vector copy per read.
// Keyed on getMSTime() as a bot never ticks twice in one ms. Holds guids, so a dead member reads null.
class IronAssemblyScan
{
public:
    explicit IronAssemblyScan(PlayerbotAI* botAI) : botAI(botAI) {}

    // The three members, alive, exactly as three GetUldUnitByEntry calls return them.
    void Members(IronAssemblyTargets& targets);
    Unit* Member(uint32 entry);

    // Rune of Death positions around this bot, off one grid visit. By value: a caller can hold the
    // list across another scan read, and that read refills this one the moment the clock moves on.
    std::vector<Position> RunesOfDeath();

    // Live Runes of Power around this bot, off one grid visit, same rules as above.
    std::vector<Position> RunesOfPower();

private:
    void RefreshMembers();
    Unit* Resolve(ObjectGuid const& guid) const;

    PlayerbotAI* botAI;

    uint32 membersAtMs = 0;
    ObjectGuid steelbreaker;
    ObjectGuid molgeim;
    ObjectGuid brundir;

    uint32 runesAtMs = 0;
    std::vector<Position> runes;

    uint32 runesOfPowerAtMs = 0;
    std::vector<Position> runesOfPower;
};

// This bot's IronAssemblyScan, held by the "iron assembly scan" value.
IronAssemblyScan& GetIronAssemblyScan(PlayerbotAI* botAI);

// Gate for positioning and targeting. The strategy runs out of combat too and the council is visible
// from the corridor, so a presence gate walks bots through the doors and opens the pull early.
bool IronAssemblyFormationActive(PlayerbotAI* botAI);

// True once Brundir is the only member left. His isolation protects nothing at that point, so the
// raid closes onto a second stack point instead of holding the opening one.
bool IronAssemblyBrundirIsLast(PlayerbotAI* botAI);

// What every non-tank hits: a human's skull on a living member, else the configured kill order. Never
// null while a member lives, which is what makes the unconditional targeting suppression safe.
Unit* IronAssemblyFocusTarget(PlayerbotAI* botAI);

// Which member this bot tanks, or nullptr. Bot tanks rank by guid and claim from the front of Brundir,
// Steelbreaker, Molgeim. Surplus ones get nullptr and fall through to the focus target.
Unit* IronAssemblyAssignedBoss(PlayerbotAI* botAI, Player* bot);

// Where that tank stands, so the boss follows him instead of being dragged through the raid. Backed off
// his boss's Rune of Power so the boss ends just outside it with the rune behind him.
bool TryGetIronAssemblyTankSpot(PlayerbotAI* botAI, Player* bot, Position& position);

// The same spot for a named boss rather than the caller's own assignment, so the ranged ring can be
// built on Steelbreaker's without a tank having to exist.
bool TryGetIronAssemblyBossTankSpot(PlayerbotAI* botAI, Unit* boss, Position& position);

// The live Rune of Power nearest this boss's designed tank spot, within OWNER_RADIUS. Keyed on the
// spot, not his buff: the buff drops the moment he steps out, and keying on it walks him back in.
bool TryGetIronAssemblyBossRuneOfPower(PlayerbotAI* botAI, Unit* boss, Position& rune);

// Where ranged and healers stand: stacked, ranged dps in a soakable Rune of Power instead; on the
// hard-mode spread ring once Steelbreaker is empowered. tolerance is the arrival deadband the spot wants.
bool TryGetIronAssemblyRaidSpot(PlayerbotAI* botAI, Player* bot, Position& position, float* tolerance = nullptr);

// Ranged dps but hunters. The buff is damage only, so healers gain nothing, and a rune next to Molgeim or
// Steelbreaker is inside a hunter's 5 yd minimum shot range.
bool IronAssemblyTakesRuneOfPower(Player* bot);
// This bot's raid spot is a Rune of Power right now.
bool IronAssemblyOnRuneOfPower(PlayerbotAI* botAI);

// Where a melee bot stands to hit the focus from inside his Rune of Power. False once it's in, or when
// there's nothing to walk to.
bool TryGetIronAssemblyMeleeRuneSpot(PlayerbotAI* botAI, Player* bot, Position& spot);

bool IronAssemblyOverloadActive(Unit* brundir);
bool IronAssemblyTendrilsActive(Unit* brundir);

// Rune of Death is a persistent area aura, so it is a DynamicObject and AvoidAoeAction cannot see
// it. Both difficulty ids are swept.
void GatherIronAssemblyRunesOfDeath(Player* bot, std::vector<Position>& runes);

// DANGER asks whether a bot has to move, CLEARANCE whether a spot is somewhere to stand. One radius for
// both parks bots on the rune's edge.
bool IsIronAssemblyPositionClearOfRunes(Position const& spot, std::vector<Position> const& runes,
                                        float radius);

// Next step toward goal: goal itself, or a point round a Rune of Death the straight line would cross.
// False means hold, goal or detour point is covered. Walking through a rune hands the bot to the escape.
bool TryGetIronAssemblyRouteStep(PlayerbotAI* botAI, Player* bot, Position const& goal, Position& step);

// A bot walking out of a hazard can't cast, so it can't be the interrupter. From the hazards, not
// isMoving(), so every bot reaches the same answer about every other bot.
bool IronAssemblyMemberMustMove(PlayerbotAI* botAI, Player* member);

// The interrupt this bot could land on Brundir now, or nullptr. Stuns yes, silences no: his
// creature_immunities mask 0x24CB375F has SILENCE but neither STUN nor INTERRUPT.
char const* IronAssemblyReadyInterrupt(Player* bot, Unit* target);

// "whirl", "chain" or nullptr. Bots with an interrupt ready rank by guid: 0 takes Lightning Whirl, 1
// Chain Lightning, so with one interrupt up Chain Lightning goes through to save it for the next Whirl.
char const* IronAssemblyInterruptDuty(PlayerbotAI* botAI, Player* bot, Unit* brundir);

bool IronAssemblyLightningWhirlActive(Unit* brundir);
bool IronAssemblyChainLightningCasting(Unit* brundir);

// Shield of Runes absorbs 20,000 and pays Molgeim +50% damage for 15s if it is drained rather than
// removed, so stripping it early is worth more than the dispel costs.
bool IronAssemblyShieldOfRunesUp(Unit* molgeim);
bool IronAssemblyHasFusionPunch(Unit* unit);

// Another grouped tank, human or bot, has boss on him.
bool IronAssemblyHeldByOtherTank(Player* bot, Unit* boss);

// With two bot tanks on three members, one member has no tank and rides the threat table. Brundir's tank
// gets it back from anyone who isn't a tank: this returns it while that is due, else nullptr.
Unit* IronAssemblyUnheldMemberToTaunt(PlayerbotAI* botAI, Player* bot);

// How long until an empowered Steelbreaker's Fusion Punch lands on this tank, 0 when none is coming. A 3s
// cast, 35,000 nature plus a DoT in 25-man, and each Electrical Charge adds 25%: 67k raw traced at two.
uint32 IronAssemblyFusionPunchIncomingMs(PlayerbotAI* botAI, Player* bot);

// Nature resistance: everything this fight hits with is nature. One hunter holds Aspect of the Wild while
// the encounter is in progress, both modes.
uint8 IronAssemblyResistWantedSchools(PlayerbotAI* botAI, uint8 previous);
inline constexpr RaidResistAuraConfig ULD_IRON_ASSEMBLY_RESIST_AURA{ULDUAR_MAP_ID, &IronAssemblyResistWantedSchools,
                                                                   nullptr};
bool IsIronAssemblyNatureHunter(PlayerbotAI* botAI);

// Nature Resistance Totem is the same aura as Aspect of the Wild, so it only covers what the hunter's 30 yd
// misses. A spare shaman drops it: one whose air totem another shaman bot already drops, or who drops none,
// so the swap costs nothing while they stand together.
Player* IronAssemblyNatureTotemShaman(PlayerbotAI* botAI);
bool IsIronAssemblyNatureTotemShaman(PlayerbotAI* botAI);

enum class IronAssemblyNatureTotemStep : uint8
{
    None,
    SetButton,  // the multi-cast bar's air button, which Call of the Elements drops
    Drop,
};

IronAssemblyNatureTotemStep IronAssemblyNatureTotemNext(PlayerbotAI* botAI);
// Highest Nature Resistance Totem rank the bot knows, 0 for none.
uint32 IronAssemblyNatureTotemSpell(Player* bot);

// What this bot does about Overwhelming Power. Carrier: hold, run, parked, late (swap came too late to
// walk out), nopartner. The other assigned tank: taunt once the carrier is inside SWAP_LEAD.
enum class IronAssemblyMeltdownRole : uint8
{
    None,
    Hold,
    NoPartner,
    Taunt,
    Run,
    Parked,
    Late,
};

IronAssemblyMeltdownRole IronAssemblyMeltdownRoleFor(PlayerbotAI* botAI, Player* bot);
Position IronAssemblyMeltdownPoint();

// Uses the bot's Nitro Boosts for a walk of this length, if it's long enough and they're ready.
bool IronAssemblyUseNitroBoosts(Player* bot, float walk);

// One round of burst at the pull is allowed in hard mode: all three up and the pull under 20s old.
bool IronAssemblyOpenerBurstWindowOpen(PlayerbotAI* botAI);

// Reset once the council is back at full health, never on "nobody in combat": Brundir spends 16s of
// every Lightning Tendrils out of combat by design.
void ResetIronAssemblyEncounterState(Player* bot, bool clearInstance);
bool IronAssemblyEncounterStateIsStale(PlayerbotAI* botAI);
// Nothing to reset means nothing to do, which keeps the reset node from swallowing every tick
// before the pull, when all three are alive at full health by definition.
bool IronAssemblyBotHasEncounterState(Player* bot);

#endif
