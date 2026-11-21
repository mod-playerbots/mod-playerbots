/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ULDENCOUNTERAURIAYA_H
#define PLAYERBOTS_ULDENCOUNTERAURIAYA_H

#include "Position.h"
#include "UldData.h"

#include <vector>

class Player;
class PlayerbotAI;
class Unit;
class WorldObject;

enum UlduarAuriayaIds
{
    NPC_AURIAYA_SANCTUM_SENTRY = 34014,
    NPC_AURIAYA_FERAL_DEFENDER = 34035,
    NPC_AURIAYA_SEEPING_FERAL_ESSENCE = 34098,
};

inline constexpr float ULDUAR_AURIAYA_AXIS_Z_PATHING_ISSUE_DETECT = 410.0f;

// Centre distances, never IsWithinDist: that adds both units' sizes, up to 4.5 yd here. The tick
// (64459, 25N 64675) hits at exactly 5 yd from the stalker, so 7 leaves 2 yd for the 1s tick and lag.
inline constexpr float ULDUAR_AURIAYA_SEEPING_ESSENCE_RADIUS = 7.0f;
// A yard past the trigger, so a bot parked on a spot it picked can't drift back into a dodge.
inline constexpr float ULDUAR_AURIAYA_ESSENCE_CLEARANCE = 8.0f;

// Sonic Screech is a 120 degree cone reaching 80 yd, so any spot in front of her shares the hit and
// there's no need for one exact range. A bot inside the stay band holds. One outside walks into the
// narrower target band, so a small turn of hers moves nobody.
inline constexpr float ULDUAR_AURIAYA_SOAK_STAY_HALF_ANGLE = static_cast<float>(M_PI) / 4.0f;
inline constexpr float ULDUAR_AURIAYA_SOAK_STAY_MIN = 8.0f;
inline constexpr float ULDUAR_AURIAYA_SOAK_STAY_MAX = 28.0f;
inline constexpr float ULDUAR_AURIAYA_SOAK_TARGET_HALF_ANGLE = static_cast<float>(M_PI) / 6.0f;
inline constexpr float ULDUAR_AURIAYA_SOAK_TARGET_MIN = 12.0f;
inline constexpr float ULDUAR_AURIAYA_SOAK_TARGET_MAX = 24.0f;
inline constexpr float ULDUAR_AURIAYA_SOAK_SPOT_TOLERANCE = 1.0f;
inline constexpr float ULDUAR_AURIAYA_MAINTANK_SPOT_TOLERANCE = 3.0f;

// Pools never despawn before she dies, so the tank steps west through fixed stations as they pile up.
inline constexpr int ULDUAR_AURIAYA_STATION_COUNT = 3;
// Pool radius plus the tank's tolerance, so a tank anywhere on a clean spot is out of the pool.
inline constexpr float ULDUAR_AURIAYA_STATION_FOUL_RADIUS = 10.0f;
inline constexpr float ULDUAR_AURIAYA_ROOM_SEARCH_RADIUS = 100.0f;

// How far a pool dodge may take the main tank from its spot, or melee from the boss.
inline constexpr float ULDUAR_AURIAYA_ESSENCE_LEASH = 12.0f;
// Melee only take the Defender this close to her. It re-rolls aggro, so they'd chase it all fight.
inline constexpr float ULDUAR_AURIAYA_MELEE_DEFENDER_RANGE = 15.0f;

extern const Position ULDUAR_AURIAYA_MAINTANK_SPOTS[];

// Terrifying Screech fears every 35s from the pull, so the whole fight is the fear window.
bool AuriayaFearWindowActive(PlayerbotAI* botAI);

// By entry, not "find target": that only walks the bot's threat list, so a bot on an add loses her.
Unit* GetAuriaya(PlayerbotAI* botAI);
bool AuriayaEncounterActive(PlayerbotAI* botAI);

// Gate for anything that pulls. A Sentry's JustEngagedWith calls SetInCombatWithZone on her, so
// her combat flag covers however the raid opens.
bool IsAuriayaEngaged(PlayerbotAI* botAI);

// Sentries first: they stay dead and stop buffing her, the Defender only feigns and comes back.
Unit* GetAuriayaFocusTarget(PlayerbotAI* botAI);

// A Sentry not already on the off-tank. Two spawn, so "first alive sentry" leaves the second loose.
Unit* GetAuriayaLooseSentry(PlayerbotAI* botAI, Player* tank);

// Grid search, not "possible targets": the stalkers aren't selectable. Runs per bot per tick on the
// dodge path, so pass the smallest radius that answers the question. The radius counts both units'
// sizes, so it over-collects; test GetAuriayaPoolClearance for an exact answer.
std::vector<Unit*> CollectAuriayaEssencePools(WorldObject* from, float radius);

// Every pool in the room, scanned off the boss once per bot per tick. Empty while she's out of
// sight. Only good for the current tick.
std::vector<Unit*> GetAuriayaRoomPools(PlayerbotAI* botAI);

// Centre distance to the nearest pool, or FLT_MAX with none.
float GetAuriayaPoolClearance(std::vector<Unit*> const& pools, float x, float y);

// Within ULDUAR_AURIAYA_SEEPING_ESSENCE_RADIUS of a pool's centre.
bool IsInAuriayaPool(Player* bot);

// Lowest station whose spot is clear of every pool by the foul radius. A fouled spot never turns clean
// again, so this only ever moves west. All fouled: the spot furthest from its nearest pool.
// Never score off her live position: she follows the tank, so it'd oscillate.
int GetAuriayaStationIndex(std::vector<Unit*> const& roomPools);

// Ranged and healers: the bot's own spot while it still soaks clear of pools, or while she walks,
// else the nearest clear spot in her target band it can walk to. False with no floor in front of her.
bool GetAuriayaSoakSpot(PlayerbotAI* botAI, Player* bot, Unit* boss, std::vector<Unit*> const& pools,
                        Position& out);

// Same target band for a bot that fell through the floor. Never holds, and checks the floor from her.
bool PickAuriayaSoakSpotFromBoss(Unit* boss, std::vector<Unit*> const& pools, Position& out);

// Main tank gets its station spot, ranged and healers their soak spot, melee and the off-tank get
// false. Trigger and action both call this so they can't disagree.
bool GetAuriayaAnchor(PlayerbotAI* botAI, Player* bot, Unit* boss, std::vector<Unit*> const& roomPools,
                      Position& out, float& tolerance);

#endif
