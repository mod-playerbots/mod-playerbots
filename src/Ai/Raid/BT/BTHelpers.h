/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_BTHELPERS_H
#define PLAYERBOTS_BTHELPERS_H

#include "Common.h"
#include "EncounterHelpers.h"
#include "ObjectGuid.h"
#include "PlayerbotAI.h"
#include "Position.h"
#include "SharedDefines.h"
#include <array>
#include <cmath>
#include <optional>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

class Aura;
class Creature;
class GameObject;
class Pet;
class Player;
class Spell;
class SpellInfo;
class Unit;

namespace BtHelpers
{

template <typename T, std::enable_if_t<std::is_enum_v<T>, int> = 0>
constexpr uint32 Id(T value)
{
    return static_cast<uint32>(value);
}

enum class BtSpells : uint32
{
    // Trash
    SPELL_SHARED_BONDS              = 41363,
    SPELL_SPELL_ABSORPTION          = 41034,

    // High Warlord Naj'entus
    SPELL_IMPALING_SPINE            = 39837,
    SPELL_TIDAL_SHIELD              = 39872,

    // Supremus
    SPELL_SNARE_SELF                = 41922,
    SPELL_VOLCANIC_ERUPTION         = 40276,
    SPELL_VOLCANIC_GEYSER           = 42055,
    SPELL_MOLTEN_FLAME_PATCH        = 40253,

    // Teron Gorefiend
    SPELL_SHADOW_OF_DEATH           = 40251,
    SPELL_SPIRITUAL_VENGEANCE       = 40268,

    SPELL_SPIRIT_LANCE              = 40157,
    SPELL_SPIRIT_CHAINS             = 40175,
    SPELL_SPIRIT_VOLLEY             = 40314,
    SPELL_SPIRIT_STRIKE             = 40325,
    SPELL_SPIRIT_SHIELD             = 40322,

    // Gurtogg Bloodboil
    SPELL_BOSS_FEL_RAGE             = 40594, // Dropped for the final 2s of phase 2
    SPELL_PLAYER_FEL_RAGE           = 40604, // Lasts the entirety of phase 2 (30s)
    SPELL_INSIGNIFICANCE            = 40618,
    SPELL_BLOODBOIL                 = 42005,

    // Reliquary of Souls
    SPELL_DEADEN                    = 41410,
    SPELL_RUNE_SHIELD               = 41431,

    // Mother Shahraz
    SPELL_FATAL_ATTRACTION          = 41001,

    // Gathios the Shatterer
    SPELL_BLESSING_OF_PROTECTION    = 41450,
    SPELL_BLESSING_OF_SPELL_WARDING = 41451,
    SPELL_JUDGEMENT                 = 41467,
    SPELL_SEAL_OF_COMMAND           = 41469,
    SPELL_CONSECRATION              = 41541,

    // Lady Malande
    SPELL_CIRCLE_OF_HEALING         = 41455,

    // Veras Darkshadow
    SPELL_VERAS_VANISH              = 41476,

    // High Nethermancer Zerevor
    SPELL_DAMPEN_MAGIC              = 41478,
    SPELL_FLAMESTRIKE               = 41481,
    SPELL_BLIZZARD                  = 41482,

    // Illidan Stormrage <The Betrayer>
    SPELL_DEMON_TRANSFORM_1         = 40511,
    SPELL_DEMON_TRANSFORM_2         = 40398,
    SPELL_DEMON_TRANSFORM_3         = 40510,
    SPELL_DEMON_FORM                = 40506,
    SPELL_DARK_BARRAGE              = 40585,
    SPELL_AGONIZING_FLAMES          = 40932,
    SPELL_PARASITIC_SHADOWFIEND_PRI = 41917, // primary infection, cast by Illidan
    SPELL_PARASITIC_SHADOWFIEND_SEC = 41914, // secondary infection, from a Shadowfiend
    SPELL_PARALYZE                  = 41083,
    SPELL_SHADOW_PRISON             = 40647,
    SPELL_SHADOW_BLAST              = 41078,

    // Death Knight
    SPELL_MIND_FREEZE               = 47528,

    // Druid
    SPELL_TREE_OF_LIFE              = 33891,

    // Hunter
    SPELL_FEIGN_DEATH               =  5384,
    SPELL_FROST_TRAP                = 13809,
    SPELL_FROST_TRAP_AURA           = 13810,
    SPELL_MISDIRECTION              = 34477,
    SPELL_MISDIRECTION_AURA         = 35079, // the aura on the hunter

    // Mage
    SPELL_COUNTERSPELL              =  2139,
    SPELL_SPELLSTEAL                = 30449,

    // Paladin
    SPELL_DIVINE_SHIELD             =   642,

    // Priest
    SPELL_DISPERSION                = 47585,

    // Rogue
    SPELL_KICK                      =  1766,

    // Shaman
    SPELL_TOTEMIC_RECALL            = 36936,
    SPELL_WIND_SHEAR                = 57994,

    // Warlock (Felhunter)
    SPELL_SPELL_LOCK_1              = 19244,
    SPELL_SPELL_LOCK_2              = 19647,

    // Warrior
    SPELL_SHIELD_BASH               =    72,
    SPELL_BERSERKER_STANCE          =  2458,
    SPELL_PUMMEL                    =  6552,
    SPELL_SPELL_REFLECTION          = 23920,
    SPELL_VIGILANCE                 = 50720,
};

enum class BtNpcs : uint32
{
    // Trash
    NPC_SISTER_OF_PAIN              = 22956,
    NPC_SISTER_OF_PLEASURE          = 22964,
    NPC_SHADOWMOON_REAVER           = 22879,

    // High Warlord Naj'entus
    NPC_HIGH_WARLORD_NAJENTUS       = 22887,

    // Supremus
    NPC_SUPREMUS                    = 22898,
    NPC_SUPREMUS_VOLCANO            = 23085,

    // Shade of Akama
    NPC_SHADE_OF_AKAMA              = 22841,
    NPC_ASHTONGUE_CHANNELER         = 23421,
    NPC_ASHTONGUE_SORCERER          = 23215,

    // Teron Gorefiend
    NPC_TERON_GOREFIEND             = 22871,
    NPC_SHADOWY_CONSTRUCT           = 23111,

    // Gurtogg Bloodboil
    NPC_GURTOGG_BLOODBOIL           = 22948,

    // Reliquary of Souls
    NPC_RELIQUARY_OF_THE_LOST       = 22856,
    NPC_ESSENCE_OF_SUFFERING        = 23418,
    NPC_ESSENCE_OF_DESIRE           = 23419,
    NPC_ESSENCE_OF_ANGER            = 23420,

    // Mother Shahraz
    NPC_MOTHER_SHAHRAZ              = 22947,

    // Illidari Council
    NPC_GATHIOS_THE_SHATTERER       = 22949,
    NPC_HIGH_NETHERMANCER_ZEREVOR   = 22950,
    NPC_LADY_MALANDE                = 22951,
    NPC_VERAS_DARKSHADOW            = 22952,

    // Illidan Stormrage <The Betrayer>
    NPC_ILLIDAN_STORMRAGE           = 22917,
    NPC_FLAME_OF_AZZINOTH           = 22997,
    NPC_DEMON_FIRE                  = 23069,
    NPC_ILLIDAN_DB_TARGET           = 23070,
    NPC_BLAZE                       = 23259,
    NPC_FLAME_CRASH                 = 23336,
    NPC_SHADOW_DEMON                = 23375,
    NPC_PARASITIC_SHADOWFIEND       = 23498,
};

enum class BtItems : uint32
{
    // High Warlord Naj'entus
    ITEM_NAJENTUS_SPINE             = 32408,
};

enum class BtObjects : uint32
{
    // High Warlord Naj'entus
    GO_NAJENTUS_SPINE               = 185584,

    // Hunter
    GO_FROST_TRAP                   = 164639, // dropped by 13809, despawns once sprung
};

inline constexpr uint32 BT_MAP_ID = 564;

// DungeonEncounter.dbc encounterIndex, map 564; also each boss's ObjectData key
enum class BtBosses : uint32
{
    DATA_HIGH_WARLORD_NAJENTUS      = 0,
    DATA_SUPREMUS                   = 1,
    DATA_SHADE_OF_AKAMA             = 2,
    DATA_TERON_GOREFIEND            = 3,
    DATA_GURTOGG_BLOODBOIL          = 4,
    DATA_RELIQUARY_OF_SOULS         = 5,
    DATA_MOTHER_SHAHRAZ             = 6,
    DATA_ILLIDARI_COUNCIL           = 7,
    DATA_ILLIDAN_STORMRAGE          = 8,
};

// ObjectData keys of the Council members, from black_temple.h; not in the DBC
enum class BtCreatureData : uint32
{
    DATA_GATHIOS_THE_SHATTERER      = 12,
    DATA_HIGH_NETHERMANCER_ZEREVOR  = 13,
    DATA_LADY_MALANDE               = 14,
    DATA_VERAS_DARKSHADOW           = 15,
};

Creature* GetBtBoss(Player* bot, BtBosses boss);
Creature* GetBtCreature(Player* bot, BtCreatureData data);

inline constexpr float PATH_STEP_DISTANCE = 3.5f;
// How close to its assigned spot a bot has to stand to count as arrived.
inline constexpr float BT_ARRIVAL_DISTANCE = 2.0f;

bool MisdirectTargetToTank(PlayerbotAI* botAI, Unit* target, Player* tank);
bool CommandPetAttack(Creature* pet, Unit* target);
GuidVector FindAttackerGuidsWithEntry(PlayerbotAI* botAI, uint32 entry);
uint32 GetAuraElapsedMs(Aura const* aura);
void FaceUnit(Player* bot, Unit* unit);

// Trash

inline constexpr float SISTER_OF_PLEASURE_SEARCH_RADIUS = 100.0f;

Unit* GetLinkedSisterOfPain(Unit* pleasure);
Unit* FindLinkedSisterOfPleasure(PlayerbotAI* botAI);
Unit* GetSkulledSisterOfPain(Player* bot);

enum class ChaoticChargeReach : uint8
{
    None,
    Target,
    Area,
};

// Spell Absorption lasts 15s and is recast 30 to 40s after the last one began. Magic is held
// starting 5s before the earliest possible recast to avoid pending casts and channels from landing.
inline constexpr uint32 REAVER_ABSORPTION_DURATION_MS = 15 * IN_MILLISECONDS;
inline constexpr uint32 REAVER_ABSORPTION_MIN_RECAST_MS = 30 * IN_MILLISECONDS;
inline constexpr uint32 REAVER_MAGIC_MARGIN_MS = 5 * IN_MILLISECONDS;
inline constexpr uint32 SHADOWMOON_REAVER_CACHE_INTERVAL_MS = 1000;

bool IsShadowmoonReaverUnsafeForMagic(Unit* unit);
bool IsAnyShadowmoonReaverUnsafeForMagic(PlayerbotAI* botAI);
bool PruneShadowmoonReaverAbsorptionStarts(Player* bot);
ChaoticChargeReach GetChaoticChargeReach(SpellInfo const* spellInfo);
bool IsChargeBuildingPet(Unit* unit);
Unit* FindPetTargetOtherThanReaver(PlayerbotAI* botAI);

// High Warlord Naj'entus

// Impales can overlap (every 20s, 30s stun), so use one entry per impaled player.
struct NajentusSpineAssignment
{
    ObjectGuid impaled;
    ObjectGuid remover;
};

// Needle Spine Explosion hits allies within 6 yd of each player struck.
inline constexpr float NAJENTUS_NEEDLE_SPINE_SPLASH_RANGE = 6.0f;
inline constexpr float NAJENTUS_RANGED_SPREAD_DISTANCE = NAJENTUS_NEEDLE_SPINE_SPLASH_RANGE + 1.0f;
// Hurl Spine (39948) counts both combat reaches.
inline constexpr float NAJENTUS_HURL_SPINE_RANGE = 25.0f;
// How far inside Hurl Spine range a thrower moves to before throwing.
inline constexpr float NAJENTUS_HURL_SPINE_APPROACH_MARGIN = 2.0f;
// A spine flies up to 1.25s and breaks the shield on impact, and the throw is a queued packet,
// so no spine is thrown within 2s after the last one to avoid wasting them.
inline constexpr uint32 NAJENTUS_HURL_SPINE_FLIGHT_MS = 2 * IN_MILLISECONDS;

inline Position const NAJENTUS_TANK_POSITION = { 438.515f, 772.436f, 11.931f };

bool IsNajentusImpaled(Player* player);
Player* FindNajentusUnassignedImpaledPlayer(Player* bot);
Player* FindNajentusSpineRemover(Player* bot, Player* impaled);
Player* GetNajentusImpaledPlayerToFree(Player* bot);
bool IsNajentusSpineThrower(Player* bot);
Player* GetNajentusSpineThrower(Player* bot);
Player* FindNajentusSpineThrower(Player* bot, Unit* najentus);
bool IsNajentusSpineInFlight(Player* bot);

// Supremus

// Ground fire: an erupting volcano or a Molten Flame trail.
struct SupremusHazard
{
    Position position;
    float damageRadius;
    float safeRadius;
};

enum class SupremusHazardZone
{
    Damage,
    Safe,
};

inline constexpr float SUPREMUS_HAZARD_SEARCH_RADIUS = 60.0f;
inline constexpr uint32 SUPREMUS_HAZARD_CACHE_INTERVAL_MS = 200;
// Tank phase only, so a Molten Flame trail chasing one ranged bot doesn't run through the rest.
inline constexpr float SUPREMUS_RANGED_SPREAD_DISTANCE = 6.0f;
inline constexpr float SUPREMUS_VOLCANO_HAZARD_RADIUS = 15.0f;
inline constexpr float SUPREMUS_VOLCANO_SAFE_DISTANCE = SUPREMUS_VOLCANO_HAZARD_RADIUS + 3.0f;
inline constexpr float SUPREMUS_MOLTEN_FLAME_HAZARD_RADIUS = 6.5f; // 5 yd + victim's combat reach
inline constexpr float SUPREMUS_MOLTEN_FLAME_SAFE_DISTANCE =
    SUPREMUS_MOLTEN_FLAME_HAZARD_RADIUS + 1.0f;
// Supremus fixates on a new target every 10s from the start of the kite phase.
inline constexpr int64 SUPREMUS_FIXATE_INTERVAL_SECONDS = 10;
// The safety margin is added to Supremus's melee range, which is ~27 yd (combat reach of 24 yd).
inline constexpr float SUPREMUS_KITE_SAFETY_MARGIN = 3.0f;
inline constexpr uint8 SUPREMUS_KITE_HEADINGS = 16;
// The Y boundaries run from the top of the ramp leading up into the temple to just before reaching
// the vestibule inside the courtyard gate. The X boundaries are pretty close to spanning the entire
// width of the courtyard.
inline constexpr float SUPREMUS_BOUNDARY_MIN_X = 587.0f;
inline constexpr float SUPREMUS_BOUNDARY_MAX_X = 820.0f;
inline constexpr float SUPREMUS_BOUNDARY_MIN_Y = 590.0f;
inline constexpr float SUPREMUS_BOUNDARY_MAX_Y = 970.0f;

inline Position const SUPREMUS_INITIAL_TANK_POSITION = { 700.678f, 698.270f, 71.514 };

bool IsSupremusKitePhase(Unit* supremus);
float GetSupremusFixateTimeRemaining(Unit* supremus);
float GetSupremusCatchDistance(Player* bot, Unit* supremus);
bool CanSupremusCatchStandingBot(Player* bot, Unit* supremus);
bool IsSupremusVolcanoErupting(Unit* volcano);
std::vector<SupremusHazard> FindSupremusHazards(PlayerbotAI* botAI);
std::vector<SupremusHazard> const& GetSupremusHazards(PlayerbotAI* botAI);
bool IsInSupremusHazard(
    std::vector<SupremusHazard> const& hazards, float x, float y, SupremusHazardZone zone,
    float margin = 0.0f);
float GetLineLengthInSupremusHazards(
    std::vector<SupremusHazard> const& hazards, Position const& from, Position const& to,
    SupremusHazardZone zone);
bool CanWalkThroughSupremusFire(Player* bot);
bool IsSupremusReachBlockedByFire(PlayerbotAI* botAI, Unit* target, float range);
Unit* GetSupremusFireBlockedReachTarget(PlayerbotAI* botAI);

// Shade of Akama

// These are the approximate boundaries of Shade of Akama's room.
inline constexpr float SHADE_OF_AKAMA_BOUNDARY_MIN_X = 406.8f;
inline constexpr float SHADE_OF_AKAMA_BOUNDARY_MAX_X = 564.0f;
inline constexpr float SHADE_OF_AKAMA_BOUNDARY_MIN_Y = 327.9f;
inline constexpr float SHADE_OF_AKAMA_BOUNDARY_MAX_Y = 473.5f;
// 150 yd reaches every channeler from anywhere in the room, so every bot gets the same kill order.
inline constexpr float SHADE_OF_AKAMA_ADD_SEARCH_RADIUS = 150.0f;
inline constexpr uint32 SHADE_OF_AKAMA_ADD_CACHE_INTERVAL_MS = 1000;

GuidVector FindShadeOfAkamaAddGuids(PlayerbotAI* botAI);
Unit* GetShadeOfAkamaKillTarget(PlayerbotAI* botAI);

// Teron Gorefiend

// The run from the balcony to a far corner takes about 12.5s.
inline constexpr int32 GOREFIEND_SHADOW_OF_DEATH_MOVE_MS = 15 * IN_MILLISECONDS;
// Spirit Chains and Spirit Volley hit within 12 yd of the spirit. The spirit moves to this close
// before casting (the buffer is for reaction delay and movement of constructs).
inline constexpr float GOREFIEND_SPIRIT_AOE_DISTANCE = 10.0f;
// Spirit Lance's lowest damage. Constructs within one Lance of the highest health count as even.
inline constexpr uint32 GOREFIEND_SPIRIT_LANCE_MIN_DAMAGE = 6175;
inline constexpr uint32 GOREFIEND_CONSTRUCT_CACHE_INTERVAL_MS = 1000;
// From Gorefiend's tank position, about 90 yd reaches the far corners.
inline constexpr float GOREFIEND_CONSTRUCT_SEARCH_RADIUS = 100.0f;

inline Position const GOREFIEND_TANK_POSITION  = { 597.653f, 402.284f, 187.090f };
inline Position const GOREFIEND_DEATH_POSITION = { 525.709f, 377.177f, 193.203f };

bool CastVengefulSpiritSpell(Unit* spirit, Unit* target, uint32 spellId);

// Gurtogg Bloodboil

// Bloodboil hits the 5 farthest players, and it stacks, so 3 groups of 5 ranged rotate to soak it.
// The timing allows each group's debuff to lapse by the time its turn comes again.
inline constexpr size_t GURTOGG_ROTATION_GROUP_COUNT = 3;
inline constexpr size_t GURTOGG_ROTATION_GROUP_SIZE = 5;
// Each ranged group stands in a rectangle, with its depth axis pointing toward the tank spot.
// Width is across that axis and, obviously, depth along it.
inline constexpr float GURTOGG_RANGED_HALF_WIDTH = 5.0f;
inline constexpr float GURTOGG_RANGED_HALF_DEPTH = 2.0f;
// Rotation groups wait until this long after a Bloodboil lands to swap. Mostly for realism.
inline constexpr uint32 GURTOGG_ROTATION_DELAY_MS = 2 * IN_MILLISECONDS;
// During Fel Rage, Arcing Smash is a cone about 11.5 yd deep in front of Gurtogg (10 yd radius plus
// the victim's reach). He faces the Fel Rage target and chases it to 14 yd, so the cone covers the
// ground between them and reaches at most about 9 yd from that player. Everyone else in front of
// him steps off his line to that player to keep this far away (a generous distance).
inline constexpr float GURTOGG_FEL_RAGE_AVOID_DISTANCE = 15.0f;
// The step goes 1 yd farther than the distance the trigger fires at, so small rounding and
// imprecision don't leave the bot just inside it, firing the trigger again.
inline constexpr float GURTOGG_FEL_RAGE_AVOID_MARGIN = 1.0f;
// Melee stand this far behind Gurtogg during Fel Rage, opposite the Fel Rage target. A bot in front
// first goes to a point this far around to his side, so it doesn't cut across his front.
inline constexpr float GURTOGG_FEL_RAGE_MELEE_DISTANCE = 5.0f;
inline constexpr float GURTOGG_FEL_RAGE_MELEE_TOLERANCE = 2.0f;
inline constexpr float GURTOGG_FEL_RAGE_MELEE_SIDE_ANGLE = 2.0f * static_cast<float>(M_PI) / 3.0f;
// For this long into Fel Rage, bots behind him keep away too, to allow the Fel Rage target to
// reach the tank position.
inline constexpr uint32 GURTOGG_FEL_RAGE_EARLY_AVOID_MS = 5 * IN_MILLISECONDS;
// Bewildering Strike confuses his victim for 8s and switches his target to second on his threat
// list (threat is not dropped). Everyone stays below 80% of the second highest tank's threat to
// ensure that a tank is targeted upon Bewildering Strike.
inline constexpr float GURTOGG_THREAT_HOLD_RATIO = 0.8f;
inline constexpr uint32 GURTOGG_TANK_THREAT_CACHE_INTERVAL_MS = 1000;

inline Position const GURTOGG_TANK_POSITION   = { 735.987f, 272.451f, 63.554f };
inline Position const GURTOGG_RANGED_POSITION = { 762.265f, 277.183f, 63.781f };
inline Position const GURTOGG_SOAKER_POSITION = { 769.348f, 280.116f, 63.780f };

Position const& GetGurtoggBloodboilPosition(Player* bot);
Player* GetGurtoggFelRageTarget(Unit* gurtogg);
bool IsGurtoggFelRageTarget(Player* player);
Position GetGurtoggFelRageMeleePosition(
    Player* bot, Unit const& gurtogg, Unit const& felRageTarget);
float FindGurtoggSecondTankThreat(PlayerbotAI* botAI);
bool IsInGurtoggRectangle(
    Position const& point, Position const& center, Position const& facing, float halfWidth,
    float halfDepth);
Position GetBotPointInGurtoggRectangle(
    Player* bot, Position const& center, Position const& facing, float halfWidth, float halfDepth);

// Reliquary of Souls

// Essence of Suffering fixates the nearest enemy every 5s, so tanks above 25% HP stand closer to it
// than other melee and pets (about 10 yd). Suffering backs away from a victim within about 1.4 yd
// and chases one beyond about 10 yd, so the tank distance stays between the two.
inline constexpr float SUFFERING_TANK_MIN_HEALTH_PCT = 25.0f;
inline constexpr float SUFFERING_TANK_DISTANCE = 6.0f;
inline constexpr float SUFFERING_TANK_MAX_DISTANCE = 7.0f;
inline constexpr float SUFFERING_MELEE_DISTANCE = 10.0f;
inline constexpr float SUFFERING_MELEE_MIN_DISTANCE = 9.0f;
inline constexpr float SUFFERING_RANGED_DISTANCE = 15.0f;
// Non-mage dispellers wait this long into each Rune Shield to dispel to leave a window for mages
// to Spellsteal.
inline constexpr uint32 RUNE_SHIELD_MAGE_PRIORITY_MS = 2 * IN_MILLISECONDS;
inline constexpr float SPELLSTEAL_RANGE = 30.0f;

bool IsSufferingFixateTank(Player* bot);
bool IsOutOfSufferingPosition(Player* bot, Unit* suffering);

// Mother Shahraz

inline constexpr float SHAHRAZ_TANK_POSITION_TOLERANCE = 0.5f;
// How far her victim may stand from the tank spot and still count as her being positioned for
// purposes of melee DPS joining the fight.
inline constexpr float SHAHRAZ_POSITIONED_DISTANCE = 3.0f;
inline constexpr float SHAHRAZ_OFF_TANK_DISTANCE = 2.0f;
inline constexpr float SHAHRAZ_RANGED_POSITION_TOLERANCE = 1.0f;
// Below this HP percent, Shahraz is considered positioned regardless of whether she is at the tank
// position. This exists so that a human tank who does not take her to the designated spot does not
// cause melee DPS to wait out of range the whole fight.
inline constexpr float SHAHRAZ_MELEE_ENGAGE_HEALTH_PCT = 90.0f;
inline constexpr float SHAHRAZ_FATAL_ATTRACTION_STEP_DISTANCE = 5.0f;

inline Position const SHAHRAZ_TANK_POSITION       = { 960.438f, 178.989f, 192.826f };
inline Position const SHAHRAZ_TRANSITION_POSITION = { 951.327f, 179.550f, 192.550f };
inline Position const SHAHRAZ_RANGED_POSITION     = { 935.267f, 175.459f, 192.821f };

bool IsShahrazBeingPositioned(Unit* shahraz);

// Illidari Council

inline constexpr EncounterHelpers::ClassRole ZEREVOR_MAGE_TANK_ROLE = {
    .playerClass = CLASS_MAGE,
    .role = { "class_role_zerevor_mage_tank", "Zerevor mage tank" },
    .candidate = { "class_role_class_mage", "Mage" },
};
// 125 yd extends to the outer entrance to the Council's room.
inline constexpr float COUNCIL_CLASS_ROLE_ANNOUNCE_DISTANCE = 125.0f;
// 175 yd extends to the top of the steps leading to the courtyard outside the Council's room.
inline constexpr float COUNCIL_CLASS_ROLE_REARM_DISTANCE = 175.0f;

inline constexpr uint32 COUNCIL_DPS_WAIT_MS = 5 * IN_MILLISECONDS; // on pull
inline constexpr float COUNCIL_AOE_THREAT_CLEARANCE = 15.0f;
// A minor spread is maintained by ranged when clear of hazards to avoid the next one hitting too
// many bots at once.
inline constexpr float COUNCIL_RANGED_SPREAD_DISTANCE = 4.0f;
// HealDistance is 38.5 yd by default. This is the mage tank healer's manually imposed version.
inline constexpr float MAGE_TANK_HEALER_MAX_DISTANCE = 38.0f;
// Far enough that a patch centered on the mage tank can't reach the healer. Patches reach 11.5 yd
// (10 yd radius plus a player's combat reach), and this leaves another yard.
inline constexpr float MAGE_TANK_HEALER_MIN_DISTANCE = 12.5f;
// Zerevor casts Arcane Explosion when anyone is within 10 yd, plus both combat reaches (14.65 yd).
inline constexpr float ZEREVOR_ARCANE_EXPLOSION_SAFE_DISTANCE = 15.0f;
// A patch hits a player whose center is within its radius plus the player's 1.5 yd combat reach,
// and we add another yard for good measure.
inline constexpr float COUNCIL_PATCH_MARGIN = 2.5f;
// Manual delay to interrupt a Circle of Healing cast in progress.
inline constexpr int32 CIRCLE_OF_HEALING_REACTION_MS = 200;
// A ranged interrupter casting at the same time that Malande is casting Circle of Healing will stop
// its cast if it would end with less than this time left on Circle of Healing's cast.
inline constexpr int32 CIRCLE_OF_HEALING_CASTER_MARGIN_MS = 500;
inline constexpr float COUNTERSPELL_RANGE = 30.0f;
inline constexpr float WIND_SHEAR_RANGE = 25.0f;
inline constexpr float SPELL_LOCK_RANGE = 30.0f;

inline std::array const GATHIOS_TANK_POSITIONS = {
    Position{ 662.977f, 296.246f, 271.688f },
    Position{ 636.238f, 283.719f, 271.629f },
    Position{ 655.571f, 261.377f, 271.687f },
    Position{ 673.789f, 274.139f, 271.689f },
};
inline Position const ZEREVOR_SPAWN_POSITION = { 700.792f, 307.192f, 277.526f };
inline std::array const ZEREVOR_TANK_POSITIONS = {
    Position{ 654.854f, 352.502f, 271.689f },
    Position{ 644.597f, 338.875f, 271.688f },
};

struct IllidariCouncilMembers
{
    Unit* gathios = nullptr;
    Unit* malande = nullptr;
    Unit* zerevor = nullptr;
    Unit* veras = nullptr;
};

IllidariCouncilMembers GetIllidariCouncilMembers(PlayerbotAI* botAI);
bool UpdateIllidariCouncilClassRoleAnnouncement(PlayerbotAI* botAI);
Player* GetZerevorMageTank(PlayerbotAI* botAI);
bool IsZerevorMageTank(PlayerbotAI* botAI);
bool HasDangerousCouncilAura(Player* bot);
bool IsVerasVanished(Unit* veras);
bool CanInterruptCircleOfHealing(Unit* malande);
bool IsInZerevorPatch(Unit* zerevor, Position const& point, float margin = 0.0f);
bool IsInCouncilPatch(Position const& point, Unit* gathios, Unit* zerevor, float margin = 0.0f);
bool IsMalandeInCouncilPatch(Unit* malande, Unit* gathios, Unit* zerevor);
bool ShouldRangedInterruptCircleOfHealing(Unit* malande, Unit* gathios, Unit* zerevor);
bool IsZerevorOnMageTank(PlayerbotAI* botAI, Unit* zerevor);
bool IsMageTankHealerPositionSafe(Position const& point, Player* mageTank, Unit* zerevor);
bool FindMageTankHealerPosition(
    Player* bot, Player* mageTank, Unit* zerevor, Position& destination);
bool FindMalandeTankPosition(
    Player* bot, Unit* malande, Unit* gathios, Unit* zerevor, Position& destination);
bool IsAnotherCouncilMemberWithin(PlayerbotAI* botAI, float range);
Unit* GetCouncilPetTarget(PlayerbotAI* botAI, Creature* pet);
uint32 GetReadySpellLock(Creature* pet, Unit* target);

// Illidan Stormrage <The Betrayer>

enum class IllidanPhase : uint8
{
    None, // also Maiev's arrival scene
    Initial, // phase 1
    Flying, // phase 2
    Landing, // transition from Illidan flying to his landing point until he is again selectable
    Grounded, // phase 3
    Demon, // phase 4
    Maiev, // phase 5
    Dying, // the 1 HP death scene
};

// Damage on Illidan is held after each threat reset to facilitate clean pickups.
enum class IllidanDpsHold : uint8
{
    None,
    AfterReset,      // phase 1, 3, and 5 starts: everyone but the main tank
    DemonFormStart,  // the transformation into Demon Form: everyone but the warlock tank
    DemonFormEnd,    // the last 3s before Demon Form ends: everyone
    DemonFormNoPush, // 30% to 33% HP in Demon Form: everyone but the warlock tank
};

// Where the main tank is dragging Illidan to in Phases 1, 3, and 5.
enum class IllidanTankLead : uint8
{
    None,
    Pickup,    // straight out from near the grate center
    Opening,   // once at the pull, straight out and then clockwise 62°
    Step,      // on a Flame Crash
    StepAgain, // a second time on a Flame Crash if Illidan stops short (casting Draw Soul)
};

struct EyeBlastDangerArea
{
    Position start;
    Position end;
    float width;
};

// Phase 4 positions, calculated from where Demon Form roots Illidan and the line from him through
// the grate center. Ranged DPS slots are in spell reach of him and healer slots behind them. All
// rows fill from the warlock tank's side first, to keep the raid close together and concentrate DPS
// on Shadow Demons.
struct DemonFormLayout
{
    Position warlockTankPosition;
    Position mainTankWaitPosition;
    Position mainTankPickupPosition;
    Position mainTankStepInPosition;
    std::vector<Position> rangedSlots;
    std::vector<Position> healerSlots;
    std::vector<Position> meleeSlots;
};

// A Shadow Demon and the player it has paralyzed and is moving to.
struct ShadowDemon
{
    ObjectGuid demon = ObjectGuid::Empty;
    ObjectGuid target = ObjectGuid::Empty;
};

// One Shadowfiend infection, recorded by the mechanic tracker. Once the trap is assigned, hunter is
// empty if a Frost Trap already covered the position, or once the hunter has laid its trap.
struct IllidanInfection
{
    Position infectionPosition;
    ObjectGuid hunter = ObjectGuid::Empty;
    bool isTrapAssigned = false;
};

inline constexpr EncounterHelpers::ClassRole ILLIDAN_WARLOCK_TANK_ROLE = {
    .playerClass = CLASS_WARLOCK,
    .preferredSpec = WARLOCK_TAB_DEMONOLOGY,
    .role = { "class_role_illidan_warlock_tank", "Illidan warlock tank" },
    .candidate = { "class_role_class_warlock", "Warlock" },
    .preferredCandidate = { "class_role_class_demonology_warlock", "Demonology Warlock" },
};
// The raid has to get closer than 100 yd to Illidan to talk to Akama.
inline constexpr float ILLIDAN_CLASS_ROLE_ANNOUNCE_DISTANCE = 100.0f;
// The height of the second landings of the double stairs leading to the Temple summit.
inline constexpr float ILLIDAN_STAIRWELL_Z = 341.0f;
// 175 yd rearm distance matches the Council's because Illidan's XY position is nearly identical.
inline constexpr float ILLIDAN_CLASS_ROLE_REARM_DISTANCE = 175.0f;

inline constexpr uint32 ILLIDAN_PARASITIC_SHADOWFIEND_CACHE_INTERVAL_MS = 200;
inline constexpr uint32 ILLIDAN_TRAP_HUNTER_CACHE_INTERVAL_MS = 1000;
inline constexpr uint32 ILLIDAN_SPEECH_CACHE_INTERVAL_MS = 1000;
inline constexpr uint32 ILLIDAN_SHADOW_DEMON_CACHE_INTERVAL_MS = 200;

inline constexpr float ILLIDAN_PHASE_2_HEALTH_PCT = 65.0f;
inline constexpr float ILLIDAN_PHASE_5_HEALTH_PCT = 30.0f;
inline constexpr float ILLIDAN_DPS_COOLDOWN_RELEASE_HEALTH_PCT = 62.0f;
inline constexpr float ILLIDAN_ADD_SEARCH_RADIUS = ILLIDAN_CLASS_ROLE_ANNOUNCE_DISTANCE;
inline constexpr float ILLIDAN_SPEECH_SEARCH_RADIUS = ILLIDAN_CLASS_ROLE_ANNOUNCE_DISTANCE;
inline constexpr float ILLIDAN_POSITION_TOLERANCE = 0.5f;
inline constexpr float ILLIDAN_MIN_SEGMENT_LENGTH = 0.1f;

// Phases 1, 3, and 5

// After a humanoid phase start with a threat reset, the main tank gets up to 3s to grab Illidan.
// Hunters misdirect onto the main tank in that time.
inline constexpr uint32 ILLIDAN_MAIN_TANK_PICKUP_MS = 3 * IN_MILLISECONDS;
// Threat resets do not drop the victim. Thus, rather than considering the tank to have aggro when
// it is the victim, require the tank to have built at least a bit of threat. This is most relevant
// for the phase 2 to 3 transition, as Illidan keeps the main tank as its victim from phase 1.
inline constexpr float ILLIDAN_TANK_AGGRO_THREAT = 2000.0f;
// The tank ring has a radius of 25 yd from the grate center and is where Illidan is positioned
// throughout the encounter (moving around the ring to avoid Flame Crashes).
inline constexpr float ILLIDAN_RING_RADIUS = 25.0f;
// The main tank stands 6 yd outward of Illidan for him to face away from the raid.
inline constexpr float ILLIDAN_TANK_OFFSET = 6.0f;
// After the raid is freed from Shadow Prison, Illidan is passive for another 2.83s. During that
// time, the main tank moves 6 yd inward of Illidan to be between him and the raid.
inline constexpr float ILLIDAN_MAIEV_SCENE_TANK_OFFSET = 6.0f;
// Creatures step back from a victim whose center passes inside the distance from the creature's
// center that is equal to half of the combined combat reaches. Creatures check for that distance
// every 2 to 6s (random). Illidan's combat reach is 7.5 yd, resulting in a back-off distance of
// (7.5 + 1.5)/2.
inline constexpr float ILLIDAN_BACK_OFF_DISTANCE = 4.5f;
// If the closest point of a main tank movement through Illidan is within ILLIDAN_BACK_OFF_DISTANCE,
// the tank aims at a point 6 yd from him instead, one 60° leg round toward its destination, so the
// tank never triggers the back step, which can cause Illidan to step past the ring and screw up the
// positioning (including leaving some ranged out of spell range).
inline constexpr float ILLIDAN_ROUND_STEP_ANGLE = 60.0f * static_cast<float>(M_PI) / 180.0f;
// On each Flame Crash, the main tank walks to the ring point this far counterclockwise of Illidan.
// Illidan chases once the main tank gets about 9.5 yd away, and the net result is that each move
// is about 25° counterclockwise around the ring.
inline constexpr float ILLIDAN_RING_LEAD_ANGLE = 35.0f * static_cast<float>(M_PI) / 180.0f;
// A cast (Draw Soul often follows a Flame Crash) can freeze him while the tank walks to the lead
// point and stops. Once the cast ends, he closes only to 9.5 yd of it, moving only about 13°
// along the tank ring and facing incorrectly as a result. Thus, a step that moves less than the
// correct case (recall about 25°) is a failure, and the main tank moves again.
inline constexpr float ILLIDAN_RING_SHORT_STEP_ANGLE = 20.0f * static_cast<float>(M_PI) / 180.0f;
// The lead that moves Illidan away from the grate center. With Illidan under 15 yd from the center
// and no lead running, the main tank walks out to 28 yd on its own bearing (due north after a
// landing), and he follows to 22 to 28 yd out.
inline constexpr float ILLIDAN_PICKUP_DISTANCE = 15.0f;
inline constexpr float ILLIDAN_PICKUP_LEAD_RADIUS = 28.0f;
inline constexpr float ILLIDAN_LANDING_LEAD_ANGLE = 0.0f;
// Once at the pull, before the first Flame Crash, the main tank leads him 62° clockwise of north
// at 27 yd. Illidan stops about 58° clockwise, at grate angle 302. The first four Flame Crashes
// then land at about 305°, 335°, 5°, and 35° (even at 50% damage, you are very unlikely to have
// more than 4 and likely will not have more than 3, depending on DPS). This design is to allow
// the Flame tanking loops in phase 2 to be clear of residual Flame Crashes, which last for 2 min.
inline constexpr float ILLIDAN_OPENING_LEAD_ANGLE = -62.0f * static_cast<float>(M_PI) / 180.0f;
inline constexpr float ILLIDAN_OPENING_LEAD_RADIUS = 27.0f;
// A generous duration that stands for the pull still being in progress. The lead should be done
// well in advance, and the first Flame Crash is not cast until 25 to 30s in.
inline constexpr uint32 ILLIDAN_OPENING_LEAD_MS = 15 * IN_MILLISECONDS;
// A Flame Crash this close to the main tank is considered cast on it, triggering a ring move.
inline constexpr float ILLIDAN_FLAME_CRASH_ON_TANK_DISTANCE = 9.0f;

// Non-main-tank positioning

// Phases 3 and 5: 6 ranged positions at 6 yd radius and 12 at 12 yd, in that fill order.
inline constexpr size_t ILLIDAN_GRATE_INNER_SLOT_COUNT = 6;
inline constexpr size_t ILLIDAN_GRATE_OUTER_SLOT_COUNT = 12;
inline constexpr float ILLIDAN_GRATE_INNER_RADIUS = 6.0f;
inline constexpr float ILLIDAN_GRATE_OUTER_RADIUS = 12.0f;
inline constexpr size_t ILLIDAN_RANGED_SLOT_COUNT =
    1 + ILLIDAN_GRATE_INNER_SLOT_COUNT + ILLIDAN_GRATE_OUTER_SLOT_COUNT; // phases 3, 4, and 5
// Phase 4: 8 healer positions (apart from the 19 ranged DPS positions) and 15 melee positions.
inline constexpr size_t ILLIDAN_HEALER_SLOT_COUNT = 8;
inline constexpr size_t ILLIDAN_MELEE_SLOT_COUNT = 15;

// Shadowfiends

// A sprung trap at the infection position covers an infection if it outlasts the debuff by 5s. This
// essentially is the minimum acceptable duration of the slow on the Shadowfiends (design choice).
inline constexpr int32 ILLIDAN_FROST_TRAP_SLOW_MS = 5 * IN_MILLISECONDS;
// A Frost Trap not yet sprung that is 3 yd from the infection position is considered to cover it.
inline constexpr float ILLIDAN_FROST_TRAP_DISTANCE = 3.0f;
// No Shadowfiend debuff, yet a live Shadowfiend this close, is treated as the Shadowfiend having
// just been born. Acting can re-infect the bot. Used to hold AoE, heals, and pets.
inline constexpr float ILLIDAN_SHADOWFIEND_BIRTHER_DISTANCE = 8.0f;
// Below 50% HP, a Shadowfiend is considered to have built aggro on a target, reducing the risk of
// immediately infecting an adjacent bot, and thus the birther can act again and melee can attack it
// if it moves into melee range.
inline constexpr float ILLIDAN_SHADOWFIEND_FINISH_HEALTH_PCT = 50.0f;
// Shadowfiend infection position: 31 yd from the grate center (the main tank's circle), 135°
// counterclockwise of Illidan, past Draw Soul's reach along the ring (110° at 31 yd). Also far
// enough to be out of Shadow Blast range for a bot waiting out a residual phase 3/5 infection.
inline constexpr float ILLIDAN_INFECTION_POSITION_DISTANCE = 31.0f;
inline constexpr float ILLIDAN_INFECTION_POSITION_ANGLE =
    135.0f * static_cast<float>(M_PI) / 180.0f;

// Phase 2

// Assist tank indices for the Flames of Azzinoth, as GetGroupAssistTank() and IsAssistTankOfIndex()
// number them.
inline constexpr uint8 ILLIDAN_WEST_FLAME_TANK_INDEX = 0;
inline constexpr uint8 ILLIDAN_EAST_FLAME_TANK_INDEX = 1;
// The Eye Blast beam's radius, which bots keep clear of by an extra 3 yd.
inline constexpr float ILLIDAN_EYE_BLAST_HAZARD_RADIUS = 6.0f;
inline constexpr float ILLIDAN_EYE_BLAST_CLEARANCE = ILLIDAN_EYE_BLAST_HAZARD_RADIUS + 3.0f;
// Hold DPS for this long from the West Flame spawning (~0.7s before the East). This is especially
// important for the first 2s after spawn because the Flame is passive. The hold is longer than for
// main tank phases because Flame tanks need lots of fire resistance, which hurts threat generation.
inline constexpr uint32 ILLIDAN_FLAME_DPS_WAIT_MS = 6 * IN_MILLISECONDS;
// The imposed delay for bots to remove Dark Barrage, so they don't unrealistically remove it before
// the visual appears. This does cause bots to take one tick of damage (3000).
inline constexpr uint32 ILLIDAN_DARK_BARRAGE_REACTION_MS = 1500;

// Phase 4

// Demon Form ends, with a threat reset, this long after the transform starts (active for 60s).
inline constexpr uint32 ILLIDAN_DEMON_FORM_DURATION_MS = 72230;
// Shadow Demons appear this long after the Demon Form transformation starts (30s into the active
// portion of phase 4).
inline constexpr uint32 ILLIDAN_SHADOW_DEMON_SPAWN_MS = 42230;
// The transform to Demon Form takes 12.23s.
inline constexpr uint32 ILLIDAN_DEMON_FORM_DPS_WAIT_MS = 13 * IN_MILLISECONDS;
// Everyone stops attacking Illidan 3s before the end of Demon Form.
inline constexpr uint32 ILLIDAN_DEMON_FORM_END_HOLD_MS = 3 * IN_MILLISECONDS;
// Between 30% and 33% HP in Demon Form, only the warlock tank damages Illidan. Pushing him into
// phase 5 while in Demon Form is very bad because he will return to humanoid form without a threat
// wipe and will proceed to kill the warlock tank and then other ranged from phase 4 threat.
inline constexpr float ILLIDAN_DEMON_FORM_NO_PUSH_HEALTH_PCT = 33.0f;

inline constexpr float ILLIDAN_AURA_OF_DREAD_HAZARD_RADIUS = 15.0f;
inline constexpr float ILLIDAN_AURA_OF_DREAD_CLEARANCE =
    ILLIDAN_AURA_OF_DREAD_HAZARD_RADIUS + 2.0f;
inline constexpr float ILLIDAN_SHADOW_BLAST_HAZARD_RADIUS = 20.0f;
inline constexpr float ILLIDAN_SHADOW_BLAST_CLEARANCE = ILLIDAN_SHADOW_BLAST_HAZARD_RADIUS + 2.0f;

// When the Seed of Corruption and/or warlock tank Searing Pain action is active, use Life Tap when
// below 20% mana and above 50% HP to ensure enough mana to cast the applicable spell.
inline constexpr float ILLIDAN_LIFE_TAP_MANA_PCT = 20.0f;
inline constexpr float ILLIDAN_LIFE_TAP_HEALTH_PCT = 50.0f;
// The warlock tank's phase 4 position, 31 yd from Illidan and off his inward line. His reach is
// only 1.5 yd in Demon Form, so a standard 30 yd spell (like Searing Pain) has a 33 yd range.
inline constexpr float ILLIDAN_WARLOCK_POSITION_DISTANCE = 31.0f;
inline constexpr float ILLIDAN_WARLOCK_POSITION_ANGLE = 75.0f * static_cast<float>(M_PI) / 180.0f;

// The main tank waits beside other melee, on the first melee row one slot past a group of seven
// (5 melee DPS and 2 tanks) on the side away from the warlock tank.
inline constexpr float ILLIDAN_MAIN_TANK_WAIT_DISTANCE = 48.4f;
inline constexpr float ILLIDAN_MAIN_TANK_WAIT_ANGLE = -35.0f * static_cast<float>(M_PI) / 180.0f;
// 10s before the end of phase 4, the main tank moves to the pickup position in front of the raid,
// just outside Aura of Dread and 22.5 yd from the warlock tank, to prepare for the phase 3/5
// pickup. The walk is about 44 yards. There is no Flame Burst possible within the final 14s of
// phase 4, and Shadow Demons are cleared by then, so this should be safe.
inline constexpr uint32 ILLIDAN_MAIN_TANK_WALK_IN_MS = 10 * IN_MILLISECONDS;
inline constexpr float ILLIDAN_PICKUP_POSITION_DISTANCE = 17.0f;
inline constexpr float ILLIDAN_PICKUP_POSITION_ANGLE = 30.0f * static_cast<float>(M_PI) / 180.0f;
// For the last 1.3s of phase 4, the main tank moves in on the same bearing to 9 yd from Illidan to
// be in melee range as soon as he transforms back. This does eat one Aura of Dread tick.
inline constexpr uint32 ILLIDAN_MAIN_TANK_STEP_IN_MS = 1300;
inline constexpr float ILLIDAN_STEP_IN_POSITION_DISTANCE = 9.0f;

// The actual position of an NPC_WORLD_TRIGGER (22515).
inline Position const ILLIDAN_GRATE_CENTER =           { 676.021f, 305.455f, 353.582f };
// Illidan's starting position where he kneels, holding the Skull of Gul'dan, then stands to chat.
inline Position const ILLIDAN_SPEECH_POSITION =        { 705.732f, 304.988f, 353.925f };
// The main tank waits here at the pull, 15.5 yd north of the center.
inline Position const ILLIDAN_PULL_TANK_POSITION =     { 691.521f, 305.455f, 353.790f };
// As Illidan lands, everyone waits south of the landing position (6 yd away for the main tank and
// 22 yd away for everybody else).
inline Position const ILLIDAN_LANDING_TANK_POSITION =  { 670.648f, 304.761f, 353.920f };
inline Position const ILLIDAN_LANDING_STACK_POSITION = { 654.021f, 305.455f, 353.790f };
inline Position const ILLIDAN_LANDING_POSITION =       { 676.648f, 304.761f, 354.189f };
inline Position const ILLIDAN_N_GRATE_POSITION =       { 682.100f, 306.000f, 353.192f };
inline Position const ILLIDAN_E_GRATE_POSITION =       { 673.500f, 298.500f, 353.192f };
inline Position const ILLIDAN_W_GRATE_POSITION =       { 672.400f, 312.500f, 353.192f };
inline std::array const ILLIDAN_GRATE_POSITIONS = {
    ILLIDAN_N_GRATE_POSITION,
    ILLIDAN_E_GRATE_POSITION,
    ILLIDAN_W_GRATE_POSITION,
};

// The Flame tanks' Blaze steps, one per Flame Blast, in order. The Flame follows its tank in, so
// each step is walked into on a line that turns the Flame Blast cone away from the grate stacks.
inline Position const ILLIDAN_E_GLAIVE_WAITING_POSITION = { 677.656f, 294.066f, 353.192f };
inline std::array const ILLIDAN_E_GLAIVE_TANK_POSITIONS = {
    Position{ 657.100f, 291.200f, 354.000f },
    Position{ 658.600f, 280.700f, 354.000f },
    Position{ 672.100f, 277.700f, 354.000f },
    Position{ 682.600f, 279.200f, 354.000f },
    Position{ 685.600f, 289.700f, 354.000f },
    Position{ 669.100f, 288.200f, 354.000f },
};
// The West tank starts on its inner steps to allow maximum time for phase 1 Flame Crashes to
// despawn and to ensure maximum time in range of the West stack position (so melee there can hit
// the West Flame).
inline Position const ILLIDAN_W_GLAIVE_WAITING_POSITION = { 676.102f, 316.305f, 353.192f };
inline std::array const ILLIDAN_W_GLAIVE_TANK_POSITIONS = {
    Position{ 695.700f, 311.700f, 354.000f },
    Position{ 685.200f, 320.700f, 354.000f },
    Position{ 668.700f, 323.700f, 354.000f },
    Position{ 674.700f, 332.700f, 354.000f },
    Position{ 685.200f, 331.200f, 354.000f },
    Position{ 695.700f, 325.200f, 354.000f },
};

std::unordered_map<ObjectGuid, IllidanInfection> const* FindIllidanInfections(uint32 instanceId);
IllidanPhase GetIllidanPhase(Unit* illidan);
bool IsIllidanSpeechInProgress(Player* bot);
bool RecordIllidanPhase(uint32 instanceId, IllidanPhase phase);
bool IsIllidanPhaseStartWithin(Unit const& illidan, IllidanPhase phase, uint32 durationMs);
IllidanPhase GetIllidanPhaseBefore(Unit const& illidan, IllidanPhase phase);
bool IsIllidanDemonFormEnding(Unit const& illidan, uint32 durationMs);
IllidanDpsHold GetIllidanHoldWindow(Unit* illidan);
IllidanDpsHold GetIllidanDpsHold(PlayerbotAI* botAI, Unit* illidan);
bool IsFlameOfAzzinothDpsHeld(Unit const& illidan);
bool IsOnlineOnIllidanThreatList(Unit const& illidan, Player* bot);
bool IsIllidanTankAggroPending(Unit const& illidan, IllidanPhase phase);
bool DoIllidanTanksHaveAggro(PlayerbotAI* botAI, Unit const& illidan, IllidanPhase phase);
bool IsIllidanPickupPending(PlayerbotAI* botAI, Unit* illidan);
bool IsIllidanPickupExemptTarget(Unit* target, bool isAoe);
bool ShouldIllidanPetsBePassive(PlayerbotAI* botAI, Unit* illidan);
bool IsIllidanPickupChannelHeld(PlayerbotAI* botAI, Unit* illidan, Spell const& channel);
float GetGrateAngle(Position const& position);
Position GetRingPoint(float angle, float radius);
Position GetIllidanSettlePosition(Unit const& illidan);
Position GetGrateSlot(size_t index);
DemonFormLayout GetDemonFormLayout(Unit const& illidan);
Position GetIllidanInfectionPosition(Unit const& illidan);
Position GetIllidanRecordedInfectionPosition(PlayerbotAI* botAI, Unit const& illidan);
std::pair<Unit*, Unit*> GetFlamesOfAzzinoth(Player* bot);
bool IsWestFlameOfAzzinothTank(Player* bot);
bool IsEastFlameOfAzzinothTank(Player* bot);
bool IsFlameOfAzzinothTank(Player* bot);
Unit* GetAssignedFlameOfAzzinoth(Player* bot);
bool UpdateIllidanClassRoleAnnouncement(PlayerbotAI* botAI);
ObjectGuid GetIllidanWarlockTankGuid(Player* bot);
Player* GetIllidanWarlockTank(PlayerbotAI* botAI);
bool IsIllidanWarlockTank(PlayerbotAI* botAI);
bool CastIllidanLifeTapIfLow(PlayerbotAI* botAI);
GuidVector FindIllidanTrapHunterGuids(Player* bot);
bool IsIllidanTrapHunterReady(Player* hunter);
bool IsIllidanTrapHunterOnDuty(PlayerbotAI* botAI);
bool IsExemptFromIllidanInfectionRun(PlayerbotAI* botAI, Unit* illidan, Player* player);
bool IsIllidanInfectionPositionKeptInFlight(PlayerbotAI* botAI);
bool IsFrostTrapAt(Player* bot, Position const& infectionPosition, int32 minDurationMs);
int32 GetIllidanRangedIndex(PlayerbotAI* botAI, size_t slotCount, bool inDemonForm);
int32 GetIllidanHealerIndex(PlayerbotAI* botAI, size_t slotCount);
int32 GetIllidanMeleeIndex(PlayerbotAI* botAI, size_t slotCount);
int32 GetIllidanGrateIndex(PlayerbotAI* botAI, Unit const& illidan);
int32 GetIllidanGrateStackIndex(PlayerbotAI* botAI);
Aura* GetParasiticShadowfiendAura(Player* player);
bool HasParasiticShadowfiend(Player* player);
GuidVector FindParasiticShadowfiendGuids(PlayerbotAI* botAI);
Unit* GetNearestParasiticShadowfiend(PlayerbotAI* botAI, Unit const& unit);
bool IsNearNewParasiticShadowfiend(PlayerbotAI* botAI);
GuidVector FindPlayersWithParasiticShadowfiendGuids(Player* bot);
std::vector<ShadowDemon> FindShadowDemons(PlayerbotAI* botAI);
Unit* GetShadowDemonForRangedToAttack(PlayerbotAI* botAI, Unit const& from);
Unit* GetShadowDemonMovingToWarlockTank(PlayerbotAI* botAI);
Unit* GetShadowDemonForMeleeToAttack(PlayerbotAI* botAI, Unit const& from);
Unit* GetShadowDemonForPetToAttack(PlayerbotAI* botAI, Pet const& pet);
bool IsTargetingShadowDemon(PlayerbotAI* botAI);
Position GetClosestPointOnSegment(
    Position const& start, Position const& end, Position const& point);
EyeBlastDangerArea GetEyeBlastDangerArea(Player* bot);
bool IsPositionInEyeBlastDangerArea(Position const& pos, EyeBlastDangerArea const& area);
bool IsPathInEyeBlastDangerArea(
    Position const& from, Position const& to, EyeBlastDangerArea const& area);

// Shared encounter state

struct BtInstanceState
{
    std::optional<std::vector<NajentusSpineAssignment>> najentusSpineAssignments;
    std::optional<ObjectGuid> najentusSpineThrower;
    std::optional<uint32> najentusSpineThrowTime;
    std::optional<uint32> councilDpsWaitTimer;
    std::optional<IllidanPhase> illidanLastPhase;
    std::optional<IllidanPhase> illidanPreviousPhase;
    std::optional<uint32> illidanPhaseStartTime;
    std::optional<uint32> illidanFlamePhaseStartTime;
    std::optional<ObjectGuid> eastFlameGuid;
    std::optional<ObjectGuid> westFlameGuid;
    std::optional<IllidanPhase> illidanTankAggroPhase;
    std::optional<std::unordered_map<ObjectGuid, IllidanInfection>> illidanInfections;
    EncounterHelpers::ClassRoleAssignment zerevorMageTank;
    EncounterHelpers::ClassRoleAssignment illidanWarlockTank;
};

bool UpdateBtClassRoles(PlayerbotAI* botAI);
BtInstanceState& BtState(uint32 instanceId);
BtInstanceState* FindBtState(uint32 instanceId);
bool BtResetInstance(uint32 instanceId);

}

#endif
