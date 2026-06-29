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
#include "Timer.h"
#include "Aq40SpellIds.h"

namespace Aq40BossHelper
{
static constexpr uint32 MAP_ID = 531;

inline bool IsInAq40(Player const* player)
{
    return player && player->GetMapId() == MAP_ID;
}

namespace Detail
{
constexpr float kEncounterCacheDefaultRange = 100.0f;

struct EncounterMemberSnapshot
{
    uint64 botKey = 0;
    uint32 cachedAtMs = 0;
    uint32 mapId = 0;
    uint32 instanceId = 0;
    std::vector<Player*> sameInstanceMembers;
    Player* primaryTank = nullptr;
    std::array<Player*, 2> backupTanks = { nullptr, nullptr };
    bool nearbyGroupMemberInCombat = false;
    bool encounterCombatActive = false;
};

struct EncounterUnitSnapshot
{
    uint64 botKey = 0;
    uint32 cachedAtMs = 0;
    uint32 mapId = 0;
    uint32 instanceId = 0;
    GuidVector attackers;
    GuidVector possibleTargetsNoLos;
    GuidVector encounterUnits;
    GuidVector activeCombatUnits;
};

inline EncounterMemberSnapshot& GetEncounterMemberSnapshotStorage()
{
    thread_local EncounterMemberSnapshot snapshot;
    return snapshot;
}

inline EncounterUnitSnapshot& GetEncounterUnitSnapshotStorage()
{
    thread_local EncounterUnitSnapshot snapshot;
    return snapshot;
}

inline void ResetEncounterMemberSnapshot(EncounterMemberSnapshot& snapshot)
{
    snapshot.botKey = 0;
    snapshot.cachedAtMs = 0;
    snapshot.mapId = 0;
    snapshot.instanceId = 0;
    snapshot.sameInstanceMembers.clear();
    snapshot.primaryTank = nullptr;
    snapshot.backupTanks = { nullptr, nullptr };
    snapshot.nearbyGroupMemberInCombat = false;
    snapshot.encounterCombatActive = false;
}

inline void ResetEncounterUnitSnapshot(EncounterUnitSnapshot& snapshot)
{
    snapshot.botKey = 0;
    snapshot.cachedAtMs = 0;
    snapshot.mapId = 0;
    snapshot.instanceId = 0;
    snapshot.attackers.clear();
    snapshot.possibleTargetsNoLos.clear();
    snapshot.encounterUnits.clear();
    snapshot.activeCombatUnits.clear();
}

inline void RebuildEncounterMemberSnapshot(Player const* reference, EncounterMemberSnapshot& snapshot,
                                           float range = kEncounterCacheDefaultRange)
{
    ResetEncounterMemberSnapshot(snapshot);
    if (!reference)
        return;

    Player* player = const_cast<Player*>(reference);
    snapshot.botKey = player->GetGUID().GetRawValue();
    snapshot.cachedAtMs = getMSTime();
    snapshot.mapId = player->GetMapId();
    snapshot.instanceId = player->GetMap() ? player->GetMap()->GetInstanceId() : 0;

    snapshot.nearbyGroupMemberInCombat = player->IsInCombat();

    Group const* group = player->GetGroup();
    if (!group)
    {
        if (player->IsAlive() && player->IsInWorld())
        {
            snapshot.sameInstanceMembers.push_back(player);
            if (PlayerbotAI::IsTank(player))
                snapshot.primaryTank = player;
        }

        return;
    }

    uint32 nearbyCombatants = 0;
    for (GroupReference const* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || !member->IsInWorld())
            continue;
        if (member->GetMapId() != player->GetMapId())
            continue;
        if (member->GetMap() && member->GetMap()->GetInstanceId() != snapshot.instanceId)
            continue;

        snapshot.sameInstanceMembers.push_back(member);

        if (member->IsInCombat() && player->GetDistance2d(member) <= range)
        {
            if (member != player)
                snapshot.nearbyGroupMemberInCombat = true;

            ++nearbyCombatants;
        }
    }

    snapshot.encounterCombatActive = nearbyCombatants >= 2;

    for (Player* member : snapshot.sameInstanceMembers)
    {
        if (PlayerbotAI::IsExplicitMainTank(member))
        {
            snapshot.primaryTank = member;
            break;
        }
    }

    if (!snapshot.primaryTank)
    {
        for (Player* member : snapshot.sameInstanceMembers)
        {
            if (!PlayerbotAI::IsTank(member))
                continue;

            if (PlayerbotAI::IsMainTank(member))
            {
                snapshot.primaryTank = member;
                break;
            }
        }
    }

    if (!snapshot.primaryTank)
    {
        for (Player* member : snapshot.sameInstanceMembers)
        {
            if (!PlayerbotAI::IsTank(member))
                continue;

            snapshot.primaryTank = member;
            break;
        }
    }

    std::vector<Player*> backups;
    backups.reserve(snapshot.sameInstanceMembers.size());
    for (Player* member : snapshot.sameInstanceMembers)
    {
        if (!PlayerbotAI::IsTank(member) || member == snapshot.primaryTank)
            continue;

        backups.push_back(member);
    }

    std::stable_sort(backups.begin(), backups.end(), [group](Player* left, Player* right)
    {
        auto getPriority = [group](Player* member) -> uint32
        {
            if (PlayerbotAI::IsAssistTankOfIndex(member, 0, true))
                return 0;
            if (PlayerbotAI::IsAssistTankOfIndex(member, 1, true))
                return 1;
            if (PlayerbotAI::IsAssistTank(member))
                return 2;
            if (group && group->IsAssistant(member->GetGUID()))
                return 3;
            return 10;
        };

        uint32 const leftPriority = getPriority(left);
        uint32 const rightPriority = getPriority(right);
        if (leftPriority != rightPriority)
            return leftPriority < rightPriority;

        return left->GetGUID().GetRawValue() < right->GetGUID().GetRawValue();
    });

    for (size_t index = 0; index < snapshot.backupTanks.size() && index < backups.size(); ++index)
        snapshot.backupTanks[index] = backups[index];
}

inline EncounterMemberSnapshot const& GetEncounterMemberSnapshot(Player const* reference,
                                                                 float range = kEncounterCacheDefaultRange)
{
    EncounterMemberSnapshot& snapshot = GetEncounterMemberSnapshotStorage();
    if (!reference)
    {
        ResetEncounterMemberSnapshot(snapshot);
        return snapshot;
    }

    Player const* player = reference;
    uint32 const nowMs = getMSTime();
    uint64 const botKey = player->GetGUID().GetRawValue();
    uint32 const mapId = player->GetMapId();
    uint32 const instanceId = player->GetMap() ? player->GetMap()->GetInstanceId() : 0;
    bool const needsRefresh = range != kEncounterCacheDefaultRange ||
                              snapshot.cachedAtMs != nowMs ||
                              snapshot.botKey != botKey ||
                              snapshot.mapId != mapId ||
                              snapshot.instanceId != instanceId;
    if (needsRefresh)
        RebuildEncounterMemberSnapshot(player, snapshot, range);

    return snapshot;
}
}    // namespace Detail

inline bool IsNearbyGroupMemberInCombat(Player const* player, float range = 100.0f)
{
    if (!player)
        return false;

    if (range == Detail::kEncounterCacheDefaultRange)
        return Detail::GetEncounterMemberSnapshot(player).nearbyGroupMemberInCombat;

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
inline bool IsEncounterCombatActive(Player const* player, float range = 100.0f)
{
    if (!player)
        return false;

    if (range == Detail::kEncounterCacheDefaultRange)
        return Detail::GetEncounterMemberSnapshot(player).encounterCombatActive;

    Group const* group = player->GetGroup();
    if (!group)
        return false;

    uint32 const instanceId = player->GetMap() ? player->GetMap()->GetInstanceId() : 0;

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
inline bool IsNearEncounter(Player const* reference, Player const* member, float range = 100.0f)
{
    if (!IsSameInstance(reference, member))
        return false;
    if (member == reference)
        return true;
    return const_cast<Player*>(reference)->GetDistance2d(const_cast<Player*>(member)) <= range;
}

inline bool IsEncounterParticipant(Player const* reference, Player const* member)
{
    return IsSameInstance(reference, member);
}

inline Player* GetEncounterPrimaryTank(Player* player)
{
    if (!player)
        return nullptr;

    return Detail::GetEncounterMemberSnapshot(player).primaryTank;
}

inline Player* GetEncounterBackupTank(Player* player, uint8 index = 0)
{
    if (!player)
        return nullptr;

    Detail::EncounterMemberSnapshot const& snapshot = Detail::GetEncounterMemberSnapshot(player);
    return index < snapshot.backupTanks.size() ? snapshot.backupTanks[index] : nullptr;
}

inline std::vector<Player*> GetSameInstanceGroupMembers(Player const* reference)
{
    return Detail::GetEncounterMemberSnapshot(reference).sameInstanceMembers;
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
    Detail::EncounterUnitSnapshot& snapshot = Detail::GetEncounterUnitSnapshotStorage();
    if (!botAI)
    {
        Detail::ResetEncounterUnitSnapshot(snapshot);
        return GuidVector();
    }

    Player* bot = botAI->GetBot();
    if (!bot)
    {
        Detail::ResetEncounterUnitSnapshot(snapshot);
        return GuidVector();
    }

    GuidVector const& possibleTargetsNoLos =
        botAI->GetAiObjectContext()->GetValue<GuidVector>("possible targets no los")->Get();
    uint32 const nowMs = getMSTime();
    uint64 const botKey = bot->GetGUID().GetRawValue();
    uint32 const mapId = bot->GetMapId();
    uint32 const instanceId = bot->GetMap() ? bot->GetMap()->GetInstanceId() : 0;
    bool const needsRefresh = snapshot.cachedAtMs != nowMs ||
                              snapshot.botKey != botKey ||
                              snapshot.mapId != mapId ||
                              snapshot.instanceId != instanceId ||
                              snapshot.attackers != attackers ||
                              snapshot.possibleTargetsNoLos != possibleTargetsNoLos;

    if (needsRefresh)
    {
        Detail::ResetEncounterUnitSnapshot(snapshot);
        snapshot.botKey = botKey;
        snapshot.cachedAtMs = nowMs;
        snapshot.mapId = mapId;
        snapshot.instanceId = instanceId;
        snapshot.attackers = attackers;
        snapshot.possibleTargetsNoLos = possibleTargetsNoLos;

        auto const appendUnique = [](GuidVector& units, ObjectGuid guid)
        {
            if (std::find(units.begin(), units.end(), guid) == units.end())
                units.push_back(guid);
        };

        for (ObjectGuid const guid : attackers)
        {
            Unit* unit = botAI->GetUnit(guid);
            if (!unit || !unit->IsInWorld() || !unit->IsAlive() || unit->GetMapId() != bot->GetMapId() ||
                unit->IsFriendlyTo(bot))
            {
                continue;
            }

            snapshot.encounterUnits.push_back(guid);
            snapshot.activeCombatUnits.push_back(guid);
        }

        bool const includeEncounterNoLos = !attackers.empty() || IsEncounterCombatActive(bot);
        for (ObjectGuid const guid : possibleTargetsNoLos)
        {
            Unit* unit = botAI->GetUnit(guid);
            if (!IsNearbyEncounterUnit(bot, botAI, unit, attackers))
                continue;

            if (includeEncounterNoLos)
                appendUnique(snapshot.encounterUnits, guid);

            bool const hasThreatVictim = unit && unit->IsCreature() && unit->GetThreatMgr().GetCurrentVictim();
            bool const isCombatRelevant =
                unit && (unit->IsInCombat() || unit->GetVictim() || unit->GetTarget() || hasThreatVictim);
            if (isCombatRelevant)
                appendUnique(snapshot.activeCombatUnits, guid);
        }
    }

    return snapshot.encounterUnits;
}

inline GuidVector GetActiveCombatUnits(PlayerbotAI* botAI, GuidVector const& attackers)
{
    GetEncounterUnits(botAI, attackers);
    return Detail::GetEncounterUnitSnapshotStorage().activeCombatUnits;
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

namespace Twin
{
struct TankPairAssignments
{
    std::array<ObjectGuid, 2> warlockTanks = { ObjectGuid::Empty, ObjectGuid::Empty };
    std::array<ObjectGuid, 2> meleeTanks = { ObjectGuid::Empty, ObjectGuid::Empty };

    bool IsComplete() const
    {
        return !warlockTanks[0].IsEmpty() && !warlockTanks[1].IsEmpty() &&
               !meleeTanks[0].IsEmpty() && !meleeTanks[1].IsEmpty();
    }

    bool IsWarlockTank(Player const* player) const
    {
        if (!player)
            return false;

        ObjectGuid const guid = player->GetGUID();
        return warlockTanks[0] == guid || warlockTanks[1] == guid;
    }

    bool IsMeleeTank(Player const* player) const
    {
        if (!player)
            return false;

        ObjectGuid const guid = player->GetGUID();
        return meleeTanks[0] == guid || meleeTanks[1] == guid;
    }

    bool IsTankPairMember(Player const* player) const
    {
        return IsWarlockTank(player) || IsMeleeTank(player);
    }

    int8 GetPairIndex(Player const* player) const
    {
        if (!player)
            return -1;

        ObjectGuid const guid = player->GetGUID();
        for (uint8 index = 0; index < 2; ++index)
        {
            if (warlockTanks[index] == guid || meleeTanks[index] == guid)
                return static_cast<int8>(index);
        }

        return -1;
    }
};

inline GuidVector GetEncounterUnits(PlayerbotAI* botAI)
{
    if (!botAI || !botAI->GetAiObjectContext())
        return GuidVector();

    return Aq40BossHelper::GetEncounterUnits(
        botAI, botAI->GetAiObjectContext()->GetValue<GuidVector>("attackers")->Get());
}

inline GuidVector GetActiveCombatUnits(PlayerbotAI* botAI)
{
    if (!botAI || !botAI->GetAiObjectContext())
        return GuidVector();

    return Aq40BossHelper::GetActiveCombatUnits(
        botAI, botAI->GetAiObjectContext()->GetValue<GuidVector>("attackers")->Get());
}

inline Unit* FindUnitByEntry(PlayerbotAI* botAI, GuidVector const& units, uint32 entry)
{
    if (!botAI)
        return nullptr;

    Player* bot = botAI->GetBot();
    if (!bot)
        return nullptr;

    for (ObjectGuid const guid : units)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive() || !unit->IsInWorld() || unit->IsFriendlyTo(bot) ||
            unit->GetMapId() != bot->GetMapId())
        {
            continue;
        }

        if (unit->GetEntry() == entry)
            return unit;
    }

    return nullptr;
}

inline Unit* FindVeklor(PlayerbotAI* botAI, GuidVector const& units)
{
    return FindUnitByEntry(botAI, units, Aq40SpellIds::TwinVeklorNpcEntry);
}

inline Unit* FindVeknilash(PlayerbotAI* botAI, GuidVector const& units)
{
    return FindUnitByEntry(botAI, units, Aq40SpellIds::TwinVeknilashNpcEntry);
}

inline bool AppendUniquePlayer(std::vector<Player*>& players, Player* candidate)
{
    if (!candidate)
        return false;

    if (std::find(players.begin(), players.end(), candidate) != players.end())
        return false;

    players.push_back(candidate);
    return true;
}

inline TankPairAssignments GetTankPairAssignments(Player* referencePlayer)
{
    TankPairAssignments assignments;
    if (!referencePlayer)
        return assignments;

    Group const* group = referencePlayer->GetGroup();
    std::vector<Player*> assistantWarlocks;
    std::vector<Player*> fallbackWarlocks;
    for (Player* member : Aq40BossHelper::GetSameInstanceGroupMembers(referencePlayer))
    {
        if (!member || !member->IsAlive() || member->getClass() != CLASS_WARLOCK)
            continue;

        if (group && group->IsAssistant(member->GetGUID()))
            AppendUniquePlayer(assistantWarlocks, member);
        else if (GET_PLAYERBOT_AI(member))
            AppendUniquePlayer(fallbackWarlocks, member);
    }

    auto sortByGuid = [](Player* left, Player* right)
    {
        if (!left || !right)
            return left != nullptr;
        return left->GetGUID().GetRawValue() < right->GetGUID().GetRawValue();
    };
    std::stable_sort(assistantWarlocks.begin(), assistantWarlocks.end(), sortByGuid);
    std::stable_sort(fallbackWarlocks.begin(), fallbackWarlocks.end(), sortByGuid);

    std::vector<Player*> selectedWarlocks;
    selectedWarlocks.reserve(2);
    for (Player* warlock : assistantWarlocks)
    {
        if (selectedWarlocks.size() >= 2)
            break;
        AppendUniquePlayer(selectedWarlocks, warlock);
    }

    for (Player* warlock : fallbackWarlocks)
    {
        if (selectedWarlocks.size() >= 2)
            break;
        AppendUniquePlayer(selectedWarlocks, warlock);
    }

    for (uint8 index = 0; index < selectedWarlocks.size() && index < assignments.warlockTanks.size(); ++index)
        assignments.warlockTanks[index] = selectedWarlocks[index]->GetGUID();

    std::vector<Player*> selectedMeleeTanks;
    selectedMeleeTanks.reserve(2);
    auto appendMeleeTank = [&selectedMeleeTanks, referencePlayer](Player* candidate)
    {
        if (!candidate || !candidate->IsAlive() || candidate->getClass() == CLASS_WARLOCK ||
            !Aq40BossHelper::IsSameInstance(referencePlayer, candidate))
        {
            return;
        }

        AppendUniquePlayer(selectedMeleeTanks, candidate);
    };

    appendMeleeTank(Aq40BossHelper::GetEncounterPrimaryTank(referencePlayer));
    appendMeleeTank(Aq40BossHelper::GetEncounterBackupTank(referencePlayer, 0));
    appendMeleeTank(Aq40BossHelper::GetEncounterBackupTank(referencePlayer, 1));

    for (uint8 index = 0; index < selectedMeleeTanks.size() && index < assignments.meleeTanks.size(); ++index)
        assignments.meleeTanks[index] = selectedMeleeTanks[index]->GetGUID();

    return assignments;
}

inline bool IsTankPairMember(Player* bot)
{
    return bot && GetTankPairAssignments(bot).IsTankPairMember(bot);
}

inline bool IsSelectedWarlockTank(Player* bot)
{
    return bot && GetTankPairAssignments(bot).IsWarlockTank(bot);
}

inline bool IsSelectedMeleeTank(Player* bot)
{
    return bot && GetTankPairAssignments(bot).IsMeleeTank(bot);
}

inline bool IsExplodeBugCast(Unit* unit)
{
    if (!unit)
        return false;

    Spell* spell = unit->GetCurrentSpell(CURRENT_GENERIC_SPELL);
    return spell && Aq40SpellIds::MatchesAnySpellId(spell->GetSpellInfo(), { Aq40SpellIds::TwinExplodeBug });
}

inline Unit* FindNearestBug(Player* bot, PlayerbotAI* botAI, GuidVector const& units, float maxDistance,
                            bool explodingOnly = false)
{
    if (!bot || !botAI)
        return nullptr;

    Unit* nearestBug = nullptr;
    float nearestDistance = std::numeric_limits<float>::max();
    for (ObjectGuid const guid : units)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive() || !Aq40SpellIds::IsTwinBugEntry(unit->GetEntry()))
            continue;

        if (explodingOnly && !IsExplodeBugCast(unit))
            continue;

        float const distance = bot->GetDistance2d(unit);
        if (distance > maxDistance || distance >= nearestDistance)
            continue;

        nearestBug = unit;
        nearestDistance = distance;
    }

    return nearestBug;
}

inline bool IsTwinKillBug(PlayerbotAI* botAI, Unit* unit)
{
    if (!botAI || !unit || !unit->IsAlive() || !unit->IsInWorld() ||
        !Aq40SpellIds::IsTwinBugEntry(unit->GetEntry()))
    {
        return false;
    }

    Player* bot = botAI->GetBot();
    if (bot && (unit->IsFriendlyTo(bot) || unit->GetMapId() != bot->GetMapId()))
        return false;

    if (IsExplodeBugCast(unit) ||
        Aq40SpellIds::HasAnyAura(botAI, unit, { Aq40SpellIds::TwinExplodeBug }))
    {
        return false;
    }

    if (Aq40SpellIds::HasAnyAura(botAI, unit,
            { Aq40SpellIds::TwinMutateBug, Aq40SpellIds::TwinVirulentPoisonProc }))
    {
        return true;
    }

    bool const hasThreatVictim = unit->IsCreature() && unit->GetThreatMgr().GetCurrentVictim();
    return unit->IsInCombat() || unit->GetVictim() || unit->GetTarget() || hasThreatVictim;
}

inline bool IsTwinPriorityKillBug(PlayerbotAI* botAI, Unit* unit)
{
    return IsTwinKillBug(botAI, unit) &&
           Aq40SpellIds::HasAnyAura(botAI, unit,
               { Aq40SpellIds::TwinMutateBug, Aq40SpellIds::TwinVirulentPoisonProc });
}

inline Unit* FindNearestKillBug(Player* bot, PlayerbotAI* botAI, GuidVector const& units, float maxDistance)
{
    if (!bot || !botAI)
        return nullptr;

    Unit* nearestPriorityBug = nullptr;
    float nearestPriorityDistance = std::numeric_limits<float>::max();
    Unit* nearestBug = nullptr;
    float nearestDistance = std::numeric_limits<float>::max();
    for (ObjectGuid const guid : units)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!IsTwinKillBug(botAI, unit))
            continue;

        float const distance = bot->GetDistance2d(unit);
        if (distance > maxDistance)
            continue;

        if (IsTwinPriorityKillBug(botAI, unit))
        {
            if (distance < nearestPriorityDistance)
            {
                nearestPriorityBug = unit;
                nearestPriorityDistance = distance;
            }
        }
        else
        {
            if (distance < nearestDistance)
            {
                nearestBug = unit;
                nearestDistance = distance;
            }
        }
    }

    return nearestPriorityBug ? nearestPriorityBug : nearestBug;
}

inline bool IsWarlockTankProfile(Player* bot, PlayerbotAI* botAI)
{
    return bot && botAI && bot->getClass() == CLASS_WARLOCK && !botAI->IsHeal(bot) &&
           IsSelectedWarlockTank(bot);
}

inline bool IsMeleeOrHunterProfile(Player* bot, PlayerbotAI* botAI, bool includeEncounterTank = true)
{
    if (!bot || !botAI)
        return false;

    if (includeEncounterTank && Aq40BossHelper::IsEncounterTank(bot, bot))
        return true;

    return bot->getClass() == CLASS_HUNTER || (!PlayerbotAI::IsRanged(bot) && !botAI->IsHeal(bot));
}

inline bool IsTrueCasterProfile(Player* bot, PlayerbotAI* botAI)
{
    return bot && botAI && !botAI->IsHeal(bot) && bot->getClass() != CLASS_HUNTER && PlayerbotAI::IsRanged(bot);
}
}    // namespace Twin

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

    static constexpr char const* ccSpells[] = {
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
