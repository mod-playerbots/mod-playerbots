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
#include <string>
#include <utility>
#include <vector>

class Action;
class Creature;
class Player;
class PlayerbotAI;
class Unit;

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

// Find bosses and their encounters from the instance script rather than the threat list or a grid
// search. Boss ids follow DungeonEncounter.dbc's order, so they are right only if the instance
// script numbers its bosses in that same order. A creature is found only if the instance script
// registers it in its ObjectData under the id passed. The creature getters return a creature only
// while it and the bot are alive and the bot is taking part in its encounter. Each returns false,
// nullopt, or nullptr when the bot isn't on mapId.
//
// CreatureAI::DoZoneInCombat's default range. A bot this close is zoned into the fight.
inline constexpr float ENCOUNTER_PARTICIPATION_DISTANCE = 250.0f;
// One boss's state, unlike IsEncounterInProgress, which is true with any boss in progress on mapId.
bool IsBossInProgress(Player* bot, uint32 mapId, uint32 bossId);
// The lowest boss id that is IN_PROGRESS, if any.
std::optional<uint32> GetBossInProgress(Player* bot, uint32 mapId);
// Checks no boss state, so it also finds a creature before its fight starts (e.g. during a speech).
Creature* GetInstanceCreature(Player* bot, uint32 mapId, uint32 dataId);
// IsBossInProgress, then GetInstanceCreature with bossId as the creature key (each boss's
// ObjectData key is its boss id).
Creature* GetInstanceBoss(Player* bot, uint32 mapId, uint32 bossId);

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

// Primarily for when it is desirable for bots to save cooldowns for particular phases (or for the
// pull, often until BOSS_ENGAGED_HEALTH_PCT).
bool IsDpsCooldownAction(Player* bot, Action* action);
// Includes both single-target and AoE taunts.
bool IsTauntAction(Player* bot, Action* action);
// These abilities (e.g., Avenger's Shield) can be problematic on the pull for a council-type boss.
// Covers tanks only.
bool IsAoeThreatAction(Player* bot, Action* action);

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
