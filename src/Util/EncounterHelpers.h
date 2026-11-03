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
class Player;
class PlayerbotAI;
class Unit;

namespace EncounterHelpers
{

// Cheap, rough proxies for how far along an encounter is. 95% HP means the boss and raid are
// positioned, the tank has threat, and the fight proper has started, so it's time to use cooldowns.
// 10% means the boss is almost dead, so ignore adds and finish off the boss.
inline constexpr float BOSS_ENGAGED_HEALTH_PCT = 95.0f;
inline constexpr float BOSS_BURN_HEALTH_PCT = 10.0f;

bool IsEncounterInProgress(Player* bot, uint32 mapId);
bool CanTakeStepTowards(
    Player* bot, float destinationX, float destinationY, float moveDist,
    float& stepX, float& stepY, float& stepZ);
bool GetStepToPosition(
    Player* bot, Position const& position, float arrivalDist, Unit* facing, float& stepX,
    float& stepY, bool& backwards);
bool MarkTargetWithIcon(Player* bot, Unit* target, uint8 iconId);
bool MarkTargetWithSkull(Player* bot, Unit* target);
bool MarkTargetWithSquare(Player* bot, Unit* target);
bool MarkTargetWithStar(Player* bot, Unit* target);
bool MarkTargetWithCircle(Player* bot, Unit* target);
bool MarkTargetWithDiamond(Player* bot, Unit* target);
bool MarkTargetWithTriangle(Player* bot, Unit* target);
bool MarkTargetWithCross(Player* bot, Unit* target);
bool MarkTargetWithMoon(Player* bot, Unit* target);
bool ClearTargetIcon(Player* bot, uint8 iconId);
bool ClearStrategyTargetIcon(Player* bot, uint8 iconId, std::initializer_list<uint32> entries);
void SetRtiTarget(PlayerbotAI* botAI, std::string const& rtiName);
bool IsMechanicTrackerBot(Player* bot, uint32 mapId);
Player* GetGroupMainTank(Player* bot);
Player* GetGroupAssistTank(Player* bot, uint8 index);
Player* GetNearestPlayerInRadius(Player* bot, float radius);
std::vector<Position> GetDynamicObjectPositions(Player* bot, float searchRadius, uint32 spellId);
uint32 GetSelfImmunitySpell(Player* bot);
bool IsDpsCooldownAction(Player* bot, Action* action);
bool IsTauntAction(Player* bot, Action* action);
bool IsAoeThreatAction(Player* bot, Action* action);

// Class roles: a job given to one raid member of a class, such as a mage tank.

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

}

#endif
