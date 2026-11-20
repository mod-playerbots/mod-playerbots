/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_SWPENCOUNTERKALEC_H
#define PLAYERBOTS_SWPENCOUNTERKALEC_H

#include "ObjectGuid.h"
#include "Position.h"
#include "SWPShared.h"
#include <array>
#include <limits>
#include <optional>
#include <unordered_map>

class Group;
class Player;
class PlayerbotAI;
class Unit;

namespace SwpHelpers
{

inline constexpr uint8 KALECGOS_INVALID_GROUP = std::numeric_limits<uint8>::max();
inline constexpr uint8 KALECGOS_TANK_COUNT = 3;
// Non-tank bots are sorted into this many groups, with dispellers and healers as evenly distributed
// as possible.
inline constexpr uint8 KALECGOS_GROUP_COUNT = 4;

// How long assist tanks hold off on attacking after the pull.
inline constexpr uint32 KALECGOS_PULL_THREAT_SUPPRESSION_MS = 5 * IN_MILLISECONDS;
// Curse of Boundless Agony doubles its tick damage every 5 ticks and, when removed by dispel or
// expiration, bounces to another player. Hold off on dispelling while damage is low.
inline constexpr int32 KALECGOS_DISPEL_REMAINING_MS = 15 * IN_MILLISECONDS;

// A rift lasts 15s after the Spectral Blast that opens it.
inline constexpr uint32 SPECTRAL_RIFT_ACTIVE_WINDOW_MS = 15 * IN_MILLISECONDS;
// Exhaustion must end this long before the rift does, for AI tick delay and latency.
inline constexpr int32 SPECTRAL_RIFT_ENTRY_MARGIN_MS = 200;
inline constexpr float SPECTRAL_RIFT_SEARCH_RADIUS = 75.0f;
inline constexpr uint32 SPECTRAL_RIFT_CACHE_INTERVAL_MS = 200;
// Approximate Z-coordinate of the Spectral Realm, used to force teleport bots down if they are
// struggling with gravity.
inline constexpr float SPECTRAL_REALM_Z = -74.5f;

// Spectral Blast hits every other player within 10 yds of the blasted player's center.
inline constexpr float KALECGOS_SPECTRAL_BLAST_HAZARD_RADIUS = 10.0f;
// ...But there isn't really enough room to spread out that far, so the spread is limited to 6 yds.
inline constexpr float KALECGOS_RANGED_SPREAD_CLEARANCE =
    KALECGOS_SPECTRAL_BLAST_HAZARD_RADIUS - 4.0f;

// The first boss under this health is called, then the other's health.
inline constexpr float KALECGOS_HEALTH_CALL_PCT = 15.0f;
// If the other boss is at this health or more, DPS on the first is held until the other is under
// KALECGOS_HEALTH_CALL_PCT, so neither boss reaches Crazed Rage at 10% while the other is high.
inline constexpr float KALECGOS_HEALTH_GAP_HOLD_PCT = 20.0f;
// Kalecgos's banish at 1% ends his Spectral Blasts, so nobody else can reach Sathrovarr. The
// surface holds DPS on Kalecgos under this health until Sathrovarr is banished.
inline constexpr float KALECGOS_END_HOLD_PCT = 5.0f;

inline Position const KALECGOS_TANK_POSITION =           { 1703.584f, 895.626f, 53.076f };
inline Position const KALECGOS_INITIAL_RANGED_POSITION = { 1704.634f, 938.080f, 53.076f };

enum class KalecgosBoss : uint8
{
    Kalecgos,
    Sathrovarr,
};

// The raid calls about the bosses' health, in order. Each call is made by a bot that can see the
// boss it names.
enum class KalecgosHealthCall : uint8
{
    None,
    FirstUnder, // one boss is called under 15%; the other's health is due
    GapHold,    // the other was at 20% or more, so DPS on the first is held
    Together,   // no hold, or the hold is over
};

// The call a bot's side of the realms has due now.
enum class KalecgosCall : uint8
{
    None,
    FirstUnder,
    OtherHealth,
    GapHoldRelease,
    EndHold,
    SathrovarrBanished,
};

enum class KalecgosDpsHold : uint8
{
    None,
    HealthGap, // the side of the first boss under 15%, until the other is under 15%
    End,       // the surface, from Kalecgos under 5% until Sathrovarr is banished
};

bool IsExhausted(Player* bot);
bool IsInSpectralRealm(Player* bot);
bool IsKalecgosDecurser(Player* bot);
// Returns true if the assignments were rebuilt.
bool EnsureKalecgosRaidAssignments(Player* player);
// The tracker's once-a-second upkeep: the start time, an expired rift, and the assignments. Returns
// true if anything changed.
bool UpdateKalecgosEncounterState(Player* player);
Player* FindKalecgosDesignatedTank(Player* player);
Player* GetKalecgosDesignatedTank(Player* player);
ObjectGuid FindKalecgosSpectralRiftGuid(Player* bot);
bool ShouldEnterKalecgosPortal(Player* bot);
void RecordSpectralBlastTarget(Player* player, PlayerbotAI* announcerAI);
void RecordSpectralRealmEnter(Player* player);
// Kalecgos for a surface bot, Sathrovarr for a bot in the Spectral Realm.
Unit* GetKalecgosSideBoss(Player* bot);
KalecgosCall GetDueKalecgosCall(Player* bot);
// The hold on the bot's side of the realms, whatever its role. For pets and cooldowns.
KalecgosDpsHold GetKalecgosDpsHoldWindow(Player* bot);
// The hold that applies to the bot itself. Tanks are never held.
KalecgosDpsHold GetKalecgosDpsHold(Player* bot);

// Shared encounter state

struct KalecgosEncounterState
{
    std::optional<uint32> encounterStartMs;
    uint32 activeRiftOpenedMs = 0;
    uint8 activeRiftGroup = KALECGOS_INVALID_GROUP;
    ObjectGuid blastedPlayerGuid = ObjectGuid::Empty;
    ObjectGuid currentTankGuid = ObjectGuid::Empty;
    ObjectGuid activeRiftOutgoingTankGuid = ObjectGuid::Empty;
    KalecgosHealthCall healthCall = KalecgosHealthCall::None;
    std::optional<KalecgosBoss> firstLowBoss;
    bool endHoldCalled = false;
    bool sathrovarrBanishCalled = false;
    std::array<ObjectGuid, KALECGOS_TANK_COUNT> tankAssignmentGuids = {
        ObjectGuid::Empty, ObjectGuid::Empty, ObjectGuid::Empty };
    std::array<ObjectGuid, KALECGOS_TANK_COUNT> tankPortalRotationGuids = {
        ObjectGuid::Empty, ObjectGuid::Empty, ObjectGuid::Empty };
    std::unordered_map<ObjectGuid, uint8> playerToGroup;
};

KalecgosEncounterState& KalecgosState(uint32 instanceId);
KalecgosEncounterState* FindKalecgosState(uint32 instanceId);

}

#endif
