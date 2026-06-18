#ifndef _PLAYERBOT_RAIDAQ40BOSSHELPER_H
#define _PLAYERBOT_RAIDAQ40BOSSHELPER_H

#include <algorithm>
#include <array>
#include <initializer_list>
#include <limits>
#include <unordered_map>
#include <vector>

#include "Group.h"
#include "ObjectGuid.h"
#include "Player.h"
#include "Playerbots.h"
#include "PlayerbotAI.h"
#include "SharedDefines.h"
#include "Spell.h"
#include "RaidAq40SpellIds.h"

namespace Aq40BossHelper
{
static constexpr uint32 MAP_ID = 531;

inline bool IsInAq40(Player const* player)
{
    return player && player->GetMapId() == MAP_ID;
}

    // Returns true if any group member near the caller (~100y, same instance) is currently in combat.
    // Scoped by proximity so that unrelated trash combat elsewhere in AQ40 does not prevent per-encounter state cleanup on wipe.
inline bool IsNearbyGroupMemberInCombat(Player const* player, float range = 100.0f)
{
    if (!player)
        return false;

    if (player->IsInCombat())
        return true;

    Group const* group = player->GetGroup();
    if (!group)
        return false;

    uint32 const instanceId = player->GetMap() ? player->GetMap()->GetInstanceId() : 0;
    for (GroupReference const* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player const* member = ref->GetSource();
        if (!member || !member->IsAlive() || member == player)
            continue;
        if (member->GetMapId() != player->GetMapId())
            continue;
        if (member->GetMap() && member->GetMap()->GetInstanceId() != instanceId)
            continue;
        if (member->IsInCombat() &&
            const_cast<Player*>(player)->GetDistance2d(const_cast<Player*>(member)) <= range)
            return true;
    }

    return false;
}

    // Returns true when an active encounter cluster exists near the caller.
    // Requires two or more in-combat group members within |range| of the caller in the same instance.
    // Caller-relative so that distant trash combat elsewhere in AQ40 does NOT preserve stale boss state after a wipe.
    // Does NOT short-circuit on the caller's own combat (a single bot on trash cannot keep the flag active by itself).
inline bool IsEncounterCombatActive(Player const* player, float range = 100.0f)
{
    if (!player)
        return false;

    Group const* group = player->GetGroup();
    if (!group)
        return false;

    uint32 const instanceId = player->GetMap() ? player->GetMap()->GetInstanceId() : 0;

    // Count in-combat members near the caller.
    uint32 nearbyCombatants = 0;
    for (GroupReference const* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player const* member = ref->GetSource();
        if (!member || !member->IsAlive())
            continue;
        if (member->GetMapId() != player->GetMapId())
            continue;
        if (member->GetMap() && member->GetMap()->GetInstanceId() != instanceId)
            continue;
        if (!member->IsInCombat())
            continue;
        if (const_cast<Player*>(player)->GetDistance2d(const_cast<Player*>(member)) > range)
            continue;

        if (++nearbyCombatants >= 2)
            return true;
    }

    return false;
}

    // Returns true when |member| is in the same instance copy as |reference|.
    // Used for globally-stable role selection (tanks, warlocks) where every member
    // in the instance must agree on the same assignment regardless of position.
inline bool IsSameInstance(Player const* reference, Player const* member)
{
    if (!reference || !member)
        return false;
    if (member == reference)
        return true;
    if (member->GetMapId() != reference->GetMapId())
        return false;
    if (reference->GetMap() && member->GetMap() &&
        reference->GetMap()->GetInstanceId() != member->GetMap()->GetInstanceId())
        return false;
    return true;
}

    // Returns true when |member| is near the same encounter as |reference|: same instance and within encounter range.
    // Used for spread/cohort assignment so that distant players elsewhere in the same instance copy
    // do not consume positioning slots.
inline bool IsNearEncounter(Player const* reference, Player const* member, float range = 100.0f)
{
    if (!IsSameInstance(reference, member))
        return false;
    if (member == reference)
        return true;
    return const_cast<Player*>(reference)->GetDistance2d(const_cast<Player*>(member)) <= range;
}

    // Returns true when |member| is an active encounter participant: same
    // instance copy as the caller.  Used for role resolution (tanks, warlocks)
    // where every participant in the instance must agree on the same assignment regardless of position.
    // Instance-global so that an isolated tank cannot promote itself to "primary" in a local view, and so that a tank that
    // briefly moves far from the boss remains in the candidate set.
inline bool IsEncounterParticipant(Player const* reference, Player const* member)
{
    return IsSameInstance(reference, member);
}

inline Player* GetEncounterPrimaryTank(Player* player)
{
    if (!player)
        return nullptr;

    Group const* group = player->GetGroup();
    if (!group)
        return PlayerbotAI::IsTank(player) && player->IsAlive() ? player : nullptr;

    for (GroupReference const* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || !PlayerbotAI::IsTank(member))
            continue;
        if (!IsEncounterParticipant(player, member))
            continue;

        if (PlayerbotAI::IsMainTank(member))
            return member;
    }

    for (GroupReference const* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || !PlayerbotAI::IsTank(member))
            continue;
        if (!IsEncounterParticipant(player, member))
            continue;

        return member;
    }

    return nullptr;
}

inline Player* GetEncounterBackupTank(Player* player, uint8 index = 0)
{
    if (!player)
        return nullptr;

    Group const* group = player->GetGroup();
    if (!group)
        return nullptr;

    Player* primaryTank = GetEncounterPrimaryTank(player);
    std::vector<Player*> backups;
    for (GroupReference const* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || !PlayerbotAI::IsTank(member) || member == primaryTank)
            continue;
        if (!IsEncounterParticipant(player, member))
            continue;

        backups.push_back(member);
    }

    std::sort(backups.begin(), backups.end(), [](Player* left, Player* right)
    {
        auto getPriority = [](Player* member) -> uint32
        {
            if (PlayerbotAI::IsAssistTankOfIndex(member, 0, true))
                return 0;
            if (PlayerbotAI::IsAssistTankOfIndex(member, 1, true))
                return 1;
            if (PlayerbotAI::IsAssistTank(member))
                return 2;
            return 10;
        };

        uint32 const leftPriority = getPriority(left);
        uint32 const rightPriority = getPriority(right);
        if (leftPriority != rightPriority)
            return leftPriority < rightPriority;

        return left->GetGUID().GetRawValue() < right->GetGUID().GetRawValue();
    });

    return index < backups.size() ? backups[index] : nullptr;
}

inline bool IsEncounterPrimaryTank(Player* referencePlayer, Player* player)
{
    return player && player == GetEncounterPrimaryTank(referencePlayer);
}

inline bool IsEncounterBackupTank(Player* referencePlayer, Player* player, uint8 index = 0)
{
    return player && player == GetEncounterBackupTank(referencePlayer, index);
}

inline bool IsEncounterTank(Player* referencePlayer, Player* player)
{
    return IsEncounterPrimaryTank(referencePlayer, player) ||
           IsEncounterBackupTank(referencePlayer, player, 0) ||
           IsEncounterBackupTank(referencePlayer, player, 1);
}

inline bool IsUnitFocusedOnPlayer(Unit* unit, Player* player)
{
    return unit && player && (unit->GetVictim() == player || unit->GetTarget() == player->GetGUID());
}

inline bool IsUnitHeldByEncounterTank(Player* referencePlayer, Unit* unit, bool primaryOnly = false)
{
    if (!referencePlayer || !unit)
        return false;

    if (Player* primaryTank = GetEncounterPrimaryTank(referencePlayer))
    {
        if (IsUnitFocusedOnPlayer(unit, primaryTank))
            return true;
    }

    if (primaryOnly)
        return false;

    for (uint8 index = 0; index < 2; ++index)
    {
        if (Player* backupTank = GetEncounterBackupTank(referencePlayer, index))
        {
            if (IsUnitFocusedOnPlayer(unit, backupTank))
                return true;
        }
    }

    return false;
}

inline bool HasAnyNamedUnitHeldByEncounterTank(PlayerbotAI* botAI, Player* referencePlayer, GuidVector const& units,
                                               std::initializer_list<char const*> names, bool primaryOnly = false)
{
    if (!botAI || !referencePlayer)
        return false;

    for (ObjectGuid const guid : units)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit)
            continue;

        bool matches = false;
        for (char const* name : names)
        {
            if (botAI->EqualLowercaseName(unit->GetName(), name))
            {
                matches = true;
                break;
            }
        }
        if (!matches)
            continue;

        if (IsUnitHeldByEncounterTank(referencePlayer, unit, primaryOnly))
            return true;
    }

    return false;
}

inline bool ShouldWaitForEncounterTankAggro(Player* referencePlayer, Player* player, Unit* unit, bool primaryOnly = false)
{
    if (!referencePlayer || !player || !unit)
        return false;

    if (IsEncounterTank(referencePlayer, player))
        return false;

    if (IsUnitFocusedOnPlayer(unit, player))
        return false;

    bool hasAssignedTank = GetEncounterPrimaryTank(referencePlayer) != nullptr;
    if (!primaryOnly)
        hasAssignedTank = hasAssignedTank || GetEncounterBackupTank(referencePlayer, 0) != nullptr ||
                          GetEncounterBackupTank(referencePlayer, 1) != nullptr;

    if (!hasAssignedTank)
        return false;

    return !IsUnitHeldByEncounterTank(referencePlayer, unit, primaryOnly);
}

inline bool IsUnitNamedAny(PlayerbotAI* botAI, Unit* unit, std::initializer_list<char const*> names)
{
    if (!botAI || !unit)
        return false;

    for (char const* name : names)
    {
        if (botAI->EqualLowercaseName(unit->GetName(), name))
            return true;
    }

    return false;
}

inline Unit* FindUnitByAnyName(PlayerbotAI* botAI, GuidVector const& units, std::initializer_list<char const*> names)
{
    if (!botAI)
        return nullptr;

    Player* bot = botAI->GetBot();
    if (!bot)
        return nullptr;

    for (ObjectGuid const guid : units)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsInWorld() || !unit->IsAlive() || unit->GetMapId() != bot->GetMapId() || unit->IsFriendlyTo(bot))
            continue;

        if (IsUnitNamedAny(botAI, unit, names))
            return unit;
    }

    return nullptr;
}

inline std::vector<Unit*> FindUnitsByAnyName(PlayerbotAI* botAI, GuidVector const& units,
                                             std::initializer_list<char const*> names)
{
    std::vector<Unit*> found;
    if (!botAI)
        return found;

    Player* bot = botAI->GetBot();
    if (!bot)
        return found;

    for (ObjectGuid const guid : units)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsInWorld() || !unit->IsAlive() || unit->GetMapId() != bot->GetMapId() || unit->IsFriendlyTo(bot))
            continue;

        if (IsUnitNamedAny(botAI, unit, names))
            found.push_back(unit);
    }

    return found;
}

inline bool IsNearbyEncounterUnit(Player* bot, PlayerbotAI* botAI, Unit* candidate, GuidVector const& attackers)
{
    if (!bot || !botAI || !candidate || !candidate->IsInWorld() || !candidate->IsAlive() ||
        candidate->GetMapId() != bot->GetMapId() || candidate->IsFriendlyTo(bot))
        return false;

    float const encounterRange = sPlayerbotAIConfig.sightDistance;

    if (candidate->GetDistance2d(bot) <= encounterRange)
        return true;

    for (ObjectGuid const attackerGuid : attackers)
    {
        Unit* attacker = botAI->GetUnit(attackerGuid);
        if (!attacker || !attacker->IsInWorld() || attacker->GetMapId() != bot->GetMapId())
            continue;

        if (candidate->GetDistance2d(attacker) <= encounterRange)
            return true;
    }

    return false;
}

inline GuidVector GetEncounterUnits(PlayerbotAI* botAI, GuidVector const& attackers)
{
    GuidVector units;
    if (!botAI)
        return units;

    Player* bot = botAI->GetBot();
    if (!bot)
        return units;

    for (ObjectGuid const guid : attackers)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsInWorld() || !unit->IsAlive() || unit->GetMapId() != bot->GetMapId() || unit->IsFriendlyTo(bot))
            continue;

        units.push_back(guid);
    }

    // Do not pull in no-LOS AQ40 units unless there is an actual local
    // encounter in progress. Otherwise nearby dead-room bosses/trash can
    // suppress normal follow/formation behavior just by being visible.
    if (attackers.empty() && !IsEncounterCombatActive(bot))
        return units;

    GuidVector const& possibleTargetsNoLos =
        botAI->GetAiObjectContext()->GetValue<GuidVector>("possible targets no los")->Get();
    for (ObjectGuid const guid : possibleTargetsNoLos)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!IsNearbyEncounterUnit(bot, botAI, unit, attackers))
            continue;

        if (std::find(units.begin(), units.end(), guid) == units.end())
            units.push_back(guid);
    }

    return units;
}

inline GuidVector GetActiveCombatUnits(PlayerbotAI* botAI, GuidVector const& attackers)
{
    GuidVector units;
    if (!botAI)
        return units;

    Player* bot = botAI->GetBot();
    if (!bot)
        return units;

    for (ObjectGuid const guid : attackers)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsInWorld() || !unit->IsAlive() || unit->GetMapId() != bot->GetMapId() ||
            unit->IsFriendlyTo(bot))
            continue;

        units.push_back(guid);
    }

    GuidVector const& possibleTargetsNoLos =
        botAI->GetAiObjectContext()->GetValue<GuidVector>("possible targets no los")->Get();
    for (ObjectGuid const guid : possibleTargetsNoLos)
    {
        if (std::find(units.begin(), units.end(), guid) != units.end())
            continue;

        Unit* unit = botAI->GetUnit(guid);
        if (!IsNearbyEncounterUnit(bot, botAI, unit, attackers))
            continue;

        // Mind-controlled raid members can become hostile, but they are still
        // players. Querying a player's ThreatManager current victim here can
        // drive the core down creature-only AI update paths and trip
        // ASSERT_NOTNULL(_owner->ToCreature()) in ThreatManager.
        bool const hasThreatVictim = unit && unit->IsCreature() && unit->GetThreatMgr().GetCurrentVictim();
        bool const isCombatRelevant =
            unit->IsInCombat() || unit->GetVictim() || unit->GetTarget() || hasThreatVictim;
        if (!isCombatRelevant)
            continue;

        units.push_back(guid);
    }

    return units;
}

inline bool HasAnyNamedUnit(PlayerbotAI* botAI, GuidVector const& units, std::initializer_list<char const*> names)
{
    return FindUnitByAnyName(botAI, units, names) != nullptr;
}

inline Unit* FindLowestHealthUnitByAnyName(PlayerbotAI* botAI, GuidVector const& units,
                                           std::initializer_list<char const*> names)
{
    std::vector<Unit*> matches = FindUnitsByAnyName(botAI, units, names);
    Unit* chosen = nullptr;
    for (Unit* unit : matches)
    {
        if (!unit)
            continue;

        if (!chosen || unit->GetHealthPct() < chosen->GetHealthPct())
            chosen = unit;
    }

    return chosen;
}

inline bool IsBossEncounterActive(PlayerbotAI* botAI, GuidVector const& attackers)
{
    return HasAnyNamedUnit(botAI, attackers,
                           { "the prophet skeram", "battleguard sartura", "sartura's royal guard",
                             "lord kri", "princess yauj", "vem", "yauj brood",
                             "fankriss the unyielding", "spawn of fankriss", "princess huhuran",
                             "emperor vek'nilash", "emperor vek'lor", "ouro", "dirt mound",
                             "viscidus", "glob of viscidus", "toxic slime", "c'thun",
                             "eye of c'thun", "eye tentacle", "claw tentacle",
                             "giant eye tentacle", "giant claw tentacle", "flesh tentacle" });
}

inline bool IsTrashEncounterActive(PlayerbotAI* botAI, GuidVector const& attackers)
{
    return HasAnyNamedUnit(botAI, attackers, { "anubisath defender" });
}

// -----------------------------------------------------------------------
// Shared encounter helpers — centralised to avoid duplicate definitions
// across Actions, Triggers, and Multipliers.
// -----------------------------------------------------------------------

inline bool IsSarturaMob(PlayerbotAI* botAI, Unit* unit)
{
    return unit && (botAI->EqualLowercaseName(unit->GetName(), "battleguard sartura") ||
                    botAI->EqualLowercaseName(unit->GetName(), "sartura's royal guard"));
}

inline bool IsSarturaSpinning(PlayerbotAI* botAI, Unit* unit)
{
    if (!IsSarturaMob(botAI, unit))
        return false;

    Spell* spell = unit->GetCurrentSpell(CURRENT_GENERIC_SPELL);
    return (spell && Aq40SpellIds::MatchesAnySpellId(spell->GetSpellInfo(),
                { Aq40SpellIds::SarturaWhirlwind, Aq40SpellIds::SarturaGuardWhirlwind })) ||
           Aq40SpellIds::HasAnyAura(botAI, unit,
               { Aq40SpellIds::SarturaWhirlwind, Aq40SpellIds::SarturaGuardWhirlwind }) ||
           botAI->HasAura("whirlwind", unit);
}

inline Unit* FindBurrowedOuro(PlayerbotAI* botAI, GuidVector const& attackers)
{
    Unit* ouro = FindUnitByAnyName(botAI, attackers, { "ouro" });
    if (!ouro || (ouro->GetUnitFlags() & UNIT_FLAG_NOT_SELECTABLE) != UNIT_FLAG_NOT_SELECTABLE)
        return nullptr;

    return ouro;
}

inline Unit* FindLowestHealthUnit(std::vector<Unit*> const& units)
{
    Unit* chosen = nullptr;
    for (Unit* unit : units)
    {
        if (!unit)
            continue;

        if (!chosen || unit->GetHealthPct() < chosen->GetHealthPct())
            chosen = unit;
    }

    return chosen;
}

// Shared mind-control CC logic used by both Skeram and Trash encounters.
// Returns true if a CC spell was successfully cast on a charmed player.
inline bool TryCrowdControlCharmedPlayer(Player* bot, PlayerbotAI* botAI, GuidVector const& encounterUnits)
{
    if (!bot || !botAI)
        return false;

    Unit* mcTarget = nullptr;
    float closestDist = std::numeric_limits<float>::max();
    for (ObjectGuid const guid : encounterUnits)
    {
        Unit* unit = botAI->GetUnit(guid);
        Player* player = unit ? unit->ToPlayer() : nullptr;
        if (!player || !player->IsAlive() || player == bot)
            continue;

        if (!player->IsCharmed() || player->IsPolymorphed())
            continue;

        float const dist = bot->GetDistance2d(player);
        if (dist < closestDist)
        {
            closestDist = dist;
            mcTarget = player;
        }
    }

    if (!mcTarget)
        return false;

    static std::initializer_list<char const*> ccSpells = {
        "polymorph", "fear", "hibernate", "freezing trap", "repentance", "blind"
    };
    for (char const* spell : ccSpells)
    {
        if (botAI->CanCastSpell(spell, mcTarget) && botAI->CastSpell(spell, mcTarget))
            return true;
    }

    return false;
}
}    // namespace Aq40BossHelper

#endif
