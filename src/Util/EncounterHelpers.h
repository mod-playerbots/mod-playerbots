/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ENCOUNTERHELPERS_H
#define PLAYERBOTS_ENCOUNTERHELPERS_H

#include "Common.h"
#include "ObjectGuid.h"
#include "Position.h"
#include <initializer_list>
#include <optional>
#include <utility>
#include <vector>

class Action;
class Creature;
class Player;
class PlayerbotAI;
class Unit;
enum ReactStates : uint8;

namespace EncounterHelpers
{

// Constants

// Cheap, rough proxies for how far along an encounter is. 95% HP means the boss and raid are
// positioned, the tank has threat, and the fight proper has started, so it's time to use cooldowns.
// 10% means the boss is almost dead, so ignore adds and finish off the boss.
inline constexpr float BOSS_ENGAGED_HEALTH_PCT = 95.0f;
inline constexpr float BOSS_BURN_HEALTH_PCT = 10.0f;

// Instance Script Callers

// InstanceScript::IsEncounterInProgress is a cheap check to use as an initial gate for triggers and
// multipliers that should run only during a boss fight. This requires the boss to be scripted to
// report IN_PROGRESS (all TBC raid bosses do). It is possible for a boss script to set IN_PROGRESS
// upon an event other than the pull (e.g., Leotheras goes IN_PROGRESS when first hit, even while
// banished, or else when the last Spellbinder dies, not when his Spellbinders are pulled). In any
// case, the boss script must be consulted to determine viability.
bool IsEncounterInProgress(Player* bot, uint32 mapId);
// Encounters from the instance script. Boss ids follow DungeonEncounter.dbc's order, so they are
// right only if the instance script numbers its bosses in that same order. Each returns false or
// nullopt when the bot isn't on mapId.
// One boss's state, unlike IsEncounterInProgress, which is true with any boss in progress on mapId.
bool IsBossInProgress(Player* bot, uint32 mapId, uint32 bossId);
// The lowest boss id that is IN_PROGRESS, if any.
std::optional<uint32> GetBossInProgress(Player* bot, uint32 mapId);

// Class Roles
// A job given to one raid member of a class, such as a mage tank.

// FindClassRoleChoice's general priority, declared lowest first: (1) an assistant (human or bot),
// (2) a preferred-spec bot (if applicable), and (3) any other bot.
enum class ClassRoleChoiceReason : uint8
{
    None,
    Bot,
    PreferredSpecBot,
    Assistant,
};
struct ClassRoleChoice
{
    ObjectGuid guid;
    ClassRoleChoiceReason reason = ClassRoleChoiceReason::None;

    bool operator==(ClassRoleChoice const&) const = default;
};
struct ClassRoleText
{
    char const* name = nullptr;
    char const* defaultText = nullptr;
};
struct ClassRole
{
    uint8 playerClass;
    std::optional<uint8> requiredSpec = std::nullopt;
    std::optional<uint8> preferredSpec = std::nullopt;
    ClassRoleText role;
    ClassRoleText candidate;
    ClassRoleText preferredCandidate = {};
};
// Not cleared by the raids' instance resets, since clearing announcedChoice would make the
// mechanic tracker repeat the announcement every second.
struct ClassRoleAssignment
{
    std::optional<ClassRoleChoice> choice;
    std::optional<ClassRoleChoice> announcedChoice;
};
struct ClassRoleSlot
{
    ClassRole const& role;
    ClassRoleAssignment& assignment;
};

// Priority from ClassRoleChoiceReason. Ties go to the highest gear score, then the lowest GUID.
ClassRoleChoice FindClassRoleChoice(PlayerbotAI* botAI, uint32 mapId, ClassRole const& role);
// Out of encounters, picks every class role player again. During one, picks a player only if the
// role is empty or its player has died or left, and announces the handover.
bool UpdateClassRoleAssignments(
    PlayerbotAI* botAI, uint32 mapId, std::initializer_list<ClassRoleSlot> slots);
// Announces any unannounced choice once the bot is within 2D announceDistance of the boss's spawn
// point, and forgets what was announced once it is beyond 2D rearmDistance. The announcement also
// requires that the boss be found alive within 3D rearmDistance and the blocker not be. The
// blocker is for a boss whose 2D area overlaps another's, such as Illidan above the Council.
bool UpdateClassRoleAnnouncement(
    PlayerbotAI* botAI, uint32 bossEntry, Position const& bossSpawn, float announceDistance,
    float rearmDistance, std::initializer_list<ClassRoleSlot> slots,
    std::optional<uint32> blockerEntry = std::nullopt);
// Null if nobody has the role, or its player is dead or not in the bot's instance.
Player* GetClassRolePlayer(Player* bot, ClassRoleAssignment const& assignment);
// Unlike GetClassRolePlayer, returns the GUID even while the player is dead or out of the instance.
ObjectGuid GetClassRolePlayerGuid(ClassRoleAssignment const& assignment);
bool IsClassRolePlayer(Player* bot, ClassRoleAssignment const& assignment);

// Locking Encounter States

// Sets the field if it is empty. Returns true if it did.
template <typename T, typename U>
bool EmplaceIfUnset(std::optional<T>& field, U&& value)
{
    if (field)
        return false;

    field.emplace(std::forward<U>(value));
    return true;
}
// Clears the field. Returns true if it had a value.
template <typename T>
bool ResetIfSet(std::optional<T>& field)
{
    bool const wasSet = field.has_value();
    field.reset();
    return wasSet;
}

// Raid Target Icons (RTI)

// These marking functions do not allow the player to change the icon during the encounter.
bool MarkTargetWithIcon(Player* bot, Unit* target, uint8 iconId);
bool MarkTargetWithSkull(Player* bot, Unit* target);
bool MarkTargetWithSquare(Player* bot, Unit* target);
bool MarkTargetWithStar(Player* bot, Unit* target);
bool MarkTargetWithCircle(Player* bot, Unit* target);
bool MarkTargetWithDiamond(Player* bot, Unit* target);
bool MarkTargetWithTriangle(Player* bot, Unit* target);
bool MarkTargetWithCross(Player* bot, Unit* target);
bool MarkTargetWithMoon(Player* bot, Unit* target);
// For bots to set their RTI attack target to the specified icon.
void SetRtiTarget(PlayerbotAI* botAI, std::string const& rtiName);
// For clearing marks outside of combat and/or encounters so bots don't Leeroy on sight.
bool ClearTargetIcon(Player* bot, uint8 iconId);
// Clears the icon only if it is on one of the given entries (i.e., one marked by the strategy).
// This leaves icons players have manually put on other mobs or players.
bool ClearStrategyTargetIcon(Player* bot, uint8 iconId, std::initializer_list<uint32> entries);

// Movement

// For validating ground and collision in connection with issuing incremental movement. The caller
// gives a destination and how far to travel towards it per tick. The helper projects that step,
// checks whether the bot can actually take it, and returns where it lands. The returned stepZ is
// snapped to the ground, so a MoveTo() using this helper should pass stepZ rather than the bot's Z.
bool CanTakeStepTowards(
    Player* bot, float destinationX, float destinationY, float moveDist, float& stepX, float& stepY,
    float& stepZ);
// One step along the bot's path to a point, stopping short of it by stopDistance. The step follows
// the path corner-by-corner, rather than aiming at the far end of it, so a bot can move around an
// obstacle between it and the point.
bool GetPathStepTowardPoint(
    Player* bot, Position const& destination, float stopDistance, float stepDistance, float& stepX,
    float& stepY);
// The same as GetPathStepTowardPoint, but toward a unit's position.
bool GetPathStepTowardUnit(
    Player* bot, Unit* target, float stopDistance, float stepDistance, float& stepX, float& stepY);
// Calculate incremental movement to a position. No ground or collision is validated, unlike
// CanTakeStepTowards(). The Z position passed for the MoveTo() action using this helper should
// use the bot's Z, not the position's. Returns false once the bot is within arrivalDist.
bool GetStepToPosition(
    Player* bot, Position const& position, float arrivalDist, Unit* facing, float& stepX,
    float& stepY, bool& backwards);

// Player Getters

// Requires the main tank to be alive.
Player* GetGroupMainTank(Player* bot);
// Returns the alive assist tank of the specified index (0 = first, 1 = second, etc.)
Player* GetGroupAssistTank(Player* bot, uint8 index);
// Return the nearest alive player (human or bot) within the specified radius. Distance is
// measured by GetExactDist2d(), which does not take into account either player's CombatReach
// (i.e., their hitboxes), which are 1.5y for all races (or 1.95y with Bloodlust/Heroism active).
Player* GetNearestPlayerInRadius(Player* bot, float radius);

// Action Class Checks for Multipliers

// Major dps cooldowns, including Bloodlust. Primarily for use when it is desirable for bots to save
// cooldowns for particular phases (or for the pull, e.g., until BOSS_ENGAGED_HEALTH_PCT).
bool IsDpsCooldownAction(Player* bot, Action* action);
// Includes both single-target and AoE taunts.
bool IsTauntAction(Player* bot, Action* action);
// These abilities (e.g., Avenger's Shield) can be problematic on the pull for a council-type boss.
// Covers tanks only.
bool IsAoeThreatAction(Player* bot, Action* action);
// Attacks and casts a dps hold covers: any AttackAction or CastSpellAction, except a non-tank's
// heals.
bool IsDpsHoldCandidate(Player* bot, Action* action);
// Healing, buffs etc. go through a dps hold. Totems don't because some are offensive.
float GetDpsHoldValue(Player* bot, Action* action);

// Stopping Attacks and Spells

// Except for SelectTarget, none of these helpers decides the bot's engine. In a fight, a bot leaves
// its combat engine through drop target, which runs when its current target is missing or invalid.
// - Hold and keep the target (StopAttackingAndChanneling or StopAttackingTarget): The hold lasts
//   one tick, so a multiplier is needed to prevent the bot from attacking again. If the multiplier
//   blocks every attack and spell, call StopAttackingAndChanneling. If it blocks attacks on only
//   one unit, call StopAttackingTarget on that unit.
// - Clear (StandDown): only if the bot has nothing to resume afterward. Drop target switches the
//   bot to its non-combat engine on the next tick unless a multiplier zeroes it. In either engine,
//   dps/tank assist will pick a target again unless a target exclusion or a multiplier prevents it.
// In any case, a spell in flight still lands, a filling cast bar can't be stopped by an action, and
// pets are not affected (CallBackPets would need to also be called).

// True if the bot's swing, auto shot, wand, or channel is aimed at the target. A channel on the
// ground at enemies counts as aimed at any enemy.
bool IsAttackingTarget(Player* bot, Unit* target);
// Stops the bot's swing, queued swing ability (e.g., Heroic Strike), auto shot or wand, and channel
// aimed at any enemy, regardless of which one, so channeled heals continue. Returns true if
// anything is stopped.
bool StopAttackingAndChanneling(Player* bot);
// Stops only what is aimed at the target: the bot's swing, auto shot or wand, and channel on the
// target enemy or on the ground at enemies, so channeled heals continue. Returns true if anything
// is stopped.
bool StopAttackingTarget(Player* bot, Unit* target);
// Calls StopAttackingAndChanneling and clears the current target.
void StandDown(PlayerbotAI* botAI);
// Like calling Attack, but without attacking or sending a pet. This switches the bot to its combat
// engine and makes the unit its current target. Also calls StopAttackingTarget on the old target.
// This can hold dps upon a pull, or switch targets without starting an attack. A multiplier is
// needed to keep the bot from attacking the new target right after selection. Returns false if the
// unit isn't a valid target, or is already the current target of a bot in its combat engine.
bool SelectTarget(PlayerbotAI* botAI, Unit* target);

// Commanding Pets

// Functionally, the pet Follow command a player gives (PetHandler.cpp, COMMAND_FOLLOW), for every
// unit with pet controls: standard pets, controllable guardians, and charmed creatures. This does
// not change a pet's stance, so a pet called back this way will attack again. To keep pets out of a
// fight, use SetMinionStancesForHold, which also calls this.
void CallBackPets(Player* bot);
// The pet Attack command a player gives (PetHandler.cpp, COMMAND_ATTACK), for a pet, guardian, or
// charmed creature. A cast in progress is not stopped, but unlike for players, CastStop can be
// called from an action to interrupt a pet's pending cast. Returns true if a command was issued.
bool CommandPetAttack(Creature* pet, Unit* target);
// A pet or guardian set passive by a hold, and the stance to give back. A pet comes back with a
// new GUID after it dies or is dismissed, so it is found by its pet number; a guardian has none
// (0) and is found by GUID.
struct SavedMinionStance
{
    ObjectGuid guid;
    uint32 petNumber;
    ReactStates reactState;
};

// True if the bot has a living pet or guardian with pet controls. Totems are in m_Controlled too,
// so this, not m_Controlled.empty(), is the cheap first check for a pet trigger.
bool HasControllableMinion(Player* bot);
// True if a pet or guardian with pet controls is passive outside a hold, or not passive in one.
bool IsMinionStanceWrongForHold(Player* bot, bool isHeld);
// Keeps pets and guardians with pet controls out of a fight while a hold lasts. When held, calls
// them back and sets them passive, saving each one's prior stance. When not held, restores the
// saved stances and drops them from the list, except a pet that isn't summoned, which keeps its
// entry until it returns. Returns true if a stance was set or restored. The raid action keeps
// savedStances as a member; its reset calls this with isHeld false, then clears the list.
bool SetMinionStancesForHold(
    Player* bot, bool isHeld, std::vector<SavedMinionStance>& savedStances);

// Miscellaneous

// True if this bot is the first living bot in the group on mapId, for giving one bot a job such as
// managing shared containers or marking targets.
bool IsMechanicTrackerBot(Player* bot, uint32 mapId);
// Grid search for dynamic objects for methods to avoid dynobj-based AoE hazards.
std::vector<Position> GetDynamicObjectPositions(Player* bot, float searchRadius, uint32 spellId);
// Ice Block, Divine Shield, and/or Cloak of Shadows can be used to nullify or ignore several
// dangerous boss mechanics.
uint32 GetSelfImmunitySpell(Player* bot);

}

#endif
