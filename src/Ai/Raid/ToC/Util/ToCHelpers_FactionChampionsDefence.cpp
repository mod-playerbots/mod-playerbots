/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "ToCHelpers_FactionChampionsDefence.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <initializer_list>
#include <iterator>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "Creature.h"
#include "Group.h"
#include "InstanceScript.h"
#include "Map.h"
#include "MotionMaster.h"
#include "ObjectAccessor.h"
#include "Pet.h"
#include "Player.h"
#include "PlayerbotTextMgr.h"
#include "Playerbots.h"
#include "PositionValue.h"
#include "PullStrategy.h"
#include "RaidDefensive.h"
#include "RaidInstanceState.h"
#include "SharedDefines.h"
#include "Spell.h"
#include "SpellAuras.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include "ThreatManager.h"
#include "Timer.h"
#include "ToCEncounterGate.h"
#include "ToCFactionChampionsGripChain.h"
#include "ToCHelpers_FactionChampions.h"
#include "ToCHelpers_FactionChampionsBall.h"
#include "ToCHelpers_Shared.h"
#include "ToCHelpers_Staging.h"

namespace TrialOfTheCrusaderHelpers
{

namespace
{
// Same radius as the encounter gate, which sees the whole arena from anywhere on its floor
constexpr float CHAMPION_SEARCH_RADIUS = 200.0f;
constexpr float DISPEL_RANGE = 30.0f;
constexpr float MASS_DISPEL_RANGE = 30.0f;
// Its friendly half, centre to centre around the point
constexpr float MASS_DISPEL_ALLY_RADIUS = 15.0f;
constexpr float PURGE_RANGE = 30.0f;
constexpr float TOTEM_DUTY_RANGE = 30.0f;

// Less than that left isn't worth a GCD
constexpr int32 CC_MIN_LEFT_MS = 2000;
// Room for Mass Dispel's 1.5 s cast
constexpr int32 SHIELD_MIN_LEFT_MS = 3000;
constexpr int32 BLADESTORM_MIN_LEFT_MS = 300;
// The kicker stays put this long after he starts Hellfire, so its kick can land
constexpr int32 HELLFIRE_KICK_WINDOW_MS = 2000;

// Script radii, centre to centre. Clearances add margin, more for the warrior, who walks at 5.6 yd/s
// while spinning
constexpr float BLADESTORM_RADIUS = 8.0f;
constexpr float BLADESTORM_CLEARANCE = 12.0f;
constexpr float HELLFIRE_RADIUS = 10.0f;
constexpr float HELLFIRE_CLEARANCE = 13.0f;
constexpr float IN_AOE_MARGIN = 1.0f;

// Only these need the kicker inside the fire, a ranged interrupt reaches him from the clearance
constexpr char const* MELEE_INTERRUPTS[] = {"kick", "pummel", "shield bash", "mind freeze"};
constexpr char const* RANGED_INTERRUPTS[] = {"counterspell", "wind shear", "silence", "strangulate",
                                             "silencing shot"};
// The felhunter's, cast through the warlock
constexpr std::array<uint32, 2> SPELL_LOCK_RANKS = {19647, 19244};
constexpr float SPELL_LOCK_RANGE = 30.0f;

constexpr float AOE_SEARCH_RADIUS = 30.0f;
constexpr float HEAL_REACH = 30.0f;
// a dodge this close to a melee champion trades the AoE for its swings
constexpr float DODGE_MELEE_GAP = 8.0f;
// The floor the Beasts spots are known to reach, round ARENA_CENTER
constexpr float ARENA_FLOOR_RADIUS = 44.0f;

// The player spell, one rank. Its taunt and attack-me effects bounce off a heroic champion, the pull
// still lands.
constexpr uint32 SPELL_DEATH_GRIP = 49576;
// Plus both combat reaches, about 34 yd centre to centre
constexpr float GRIP_RANGE = 30.0f;
// A DK this far off still gets the duty and walks to the stand
constexpr float GRIP_ELECT_REACH = 50.0f;
// The champion lands in front of the DK, so this is about how far it travels
constexpr float GRIP_STAND_DIST = 27.0f;
constexpr uint32 GRIP_BEARINGS = 16;
constexpr float GRIP_KEEP_RANGE = 29.0f;
constexpr uint32 GRIP_DUTY_MS = 8000;
// A DK this much farther from the pack than its target grips from where it stands
constexpr float GRIP_MIN_GAIN = 12.0f;
// A grip ready this soon still takes the duty, the cast just waits for it
constexpr uint32 GRIP_READY_SLACK_MS = 1000;
// Past the champion healers' 40 yd heal range, plus the pack's own spread
constexpr float GRIP_GOAL = 45.0f;
constexpr float CHAMPION_HEAL_REACH = 40.0f;

// the player's own grip, the chain's last link, comes a few seconds after the last bot DK's
constexpr uint32 GRIP_GUARD_TAIL_MS = 6000;

// A wall grip lands the melee champion this far off ARENA_CENTER at least, past where the raid stands
constexpr float WALL_MIN_OFF_CENTRE = 34.0f;
// the champion lands in front of the DK, so a stand this far off it at each bearing
constexpr std::array<float, 2> WALL_STAND_DISTS = {27.0f, 22.0f};
// the DK keeps its heals at the wall
constexpr float WALL_HEAL_REACH = 38.0f;
// a longer walk to the stand outlasts the 8 s duty
constexpr uint32 WALL_WALK_MAX_MS = 6000;
constexpr float WALL_ARRIVE = 1.5f;
// a grip any raider made this recently stays where it put the champion
constexpr uint32 WALL_AIRBORNE_MS = 8000;
// Chains of Ice goes on it this long after the grip, or never
constexpr uint32 WALL_CHAIN_MS = 4000;

constexpr float PIERCING_HOWL_RADIUS = 10.0f;
constexpr float CHAINS_OF_ICE_RANGE = 20.0f;
constexpr float CONCUSSIVE_SHOT_RANGE = 35.0f;
constexpr float FROST_SHOCK_RANGE = 25.0f;
constexpr uint32 SPELL_HAMSTRING = 1715;
constexpr uint32 SPELL_PIERCING_HOWL = 12323;
constexpr uint32 SPELL_CHAINS_OF_ICE = 45524;
// The slow starts at 95% and eases 10% a second, so the recast goes out with it near 35%
constexpr int32 ICE_RECAST_LEFT_MS = 4000;
// an ice duty DK out of Chains of Ice range walks in to this far off its champion
constexpr float ICE_STAND = 15.0f;
constexpr float ICE_ARRIVE = 2.0f;
constexpr uint32 SPELL_WING_CLIP = 2974;
constexpr uint32 SPELL_CONCUSSIVE_SHOT = 5116;
constexpr std::array<uint32, 7> FROST_SHOCK_RANKS = {49236, 49235, 25464, 10473, 10472, 8058, 8056};

// A melee champion this close to a victim off the tanks is on it. A Warrior champion gets disarmed
// before Bladestorm once this many of ours stand in its radius.
constexpr float PEEL_VICTIM_REACH = 10.0f;
constexpr uint32 PEEL_BLADESTORM_UNITS = 2;
// Frost Nova's 10 yd, less a yard so the champion is surely inside
constexpr float FROST_NOVA_REACH = 9.0f;
// Consecration and Death and Decay round a pack tank break a root at once
constexpr float ROOT_AOE_CLEAR = 12.0f;
constexpr uint32 SPELL_DISARM = 676;
constexpr uint32 SPELL_DISMANTLE = 51722;
constexpr uint32 SPELL_PSYCHIC_HORROR = 64044;
constexpr std::array<uint32, 6> FROST_NOVA_RANKS = {42917, 27088, 10230, 6131, 865, 122};
constexpr std::array<uint32, 8> ENTANGLING_ROOTS_RANKS = {53308, 26989, 9853, 9852, 5196, 5195, 1062, 339};
constexpr std::array<uint32, 4> HAMMER_OF_JUSTICE_RANKS = {10308, 5589, 5588, 853};
constexpr uint32 SPELL_REPENTANCE = 20066;
constexpr uint32 SPELL_BLIND = 2094;
// the arrow drops a trap whose freeze (the effect) is what lands and breaks
constexpr uint32 SPELL_FREEZING_ARROW = 60192;
constexpr uint32 SPELL_FREEZING_ARROW_EFFECT = 60210;

// Hand of Protection waits for four champions, actual or about to pick, the rest for two and this much
// health, so they're still up when the focused member drops. Four on a non-tank don't wait.
constexpr uint32 PROTECT_MIN_ATTACKERS = 2;
constexpr float PROTECT_HEALTH_PCT = 60.0f;
// A melee bot under Hand of Protection can't swing, so it only gets one this hurt. The predicted picks put four
// champions on one at the pull, before any hit lands.
constexpr float MELEE_HAND_HEALTH_PCT = 50.0f;
// a melee champion farther off its victim isn't hitting it yet
constexpr float PROTECT_MELEE_REACH = 8.0f;
constexpr uint32 SPELL_FORBEARANCE = 25771;
constexpr std::array<uint32, 3> HAND_OF_PROTECTION_RANKS = {10278, 5599, 1022};
constexpr uint32 SPELL_PAIN_SUPPRESSION = 33206;
constexpr uint32 SPELL_GUARDIAN_SPIRIT = 47788;
constexpr uint32 SPELL_HAND_OF_SACRIFICE = 6940;
constexpr uint32 SPELL_DIVINE_SACRIFICE = 64205;
// what Divine Sacrifice puts up with the talent: 20% less damage taken, raid wide, 6 s
constexpr uint32 SPELL_DIVINE_GUARDIAN = 70940;
constexpr float HAND_RANGE = 30.0f;
// The player spell, one rank. Its immunity strips a root or snare already on (purge on immunity), never Dazed.
constexpr uint32 SPELL_HAND_OF_FREEDOM_PLAYER = 1044;
// a rooted kiter with a melee champion this close is about to be caught
constexpr float FREEDOM_CHASER_REACH = 15.0f;
constexpr float PRIEST_EXTERNAL_RANGE = 40.0f;
// Hand of Sacrifice moves 30% of the damage onto the paladin
constexpr float SACRIFICE_MIN_HEALTH_PCT = 50.0f;
constexpr std::array<uint32, 5> LAY_ON_HANDS_RANKS = {48788, 27154, 10310, 2800, 633};
constexpr float LAY_ON_HANDS_PCT = 20.0f;
constexpr uint32 LAY_ON_HANDS_GAP_MS = 15000;
constexpr uint32 LAY_ON_HANDS_HOLD_MS = 2000;
constexpr float LAY_ON_HANDS_RANGE = 40.0f;

// A member's health is sampled this often and kept this long; a bot with this many champions on it whose
// loss over the window, run on for the lookahead, ends under the floor gets its personal defensive now
constexpr uint32 HEALTH_SAMPLE_MS = 250;
constexpr uint32 HEALTH_LOG_MS = 2500;
constexpr uint32 FOCUS_WINDOW_MS = 2000;
constexpr uint32 FOCUS_LOOKAHEAD_MS = 4000;
constexpr uint32 FOCUS_MIN_CHAMPIONS = 2;
constexpr uint32 FOCUS_PREEMPT_CHAMPIONS = 4;

// RecalculateThreat resets each player's threat to 10M x its score every 8.75-9.25 s, so two entries moving
// this much in a tick is one: damage and heal threat add a few thousand, Feint or Fade touch one entry.
constexpr uint32 REPICK_SHIFT_ENTRIES = 2;
constexpr float REPICK_SHIFT_PCT = 0.01f;
constexpr float REPICK_SHIFT_MIN = 100000.0f;
constexpr uint32 REPICK_MIN_MS = 8750;
constexpr uint32 REPICK_MAX_MS = 9250;
// how early the cover goes out, and how late a re-pick may still come
constexpr uint32 REPICK_LEAD_MS = 1000;
constexpr uint32 REPICK_LATE_MS = 250;
// the core's switch rule (ThreatManager::ReselectVictim): 110% for a target in melee range, else 130%
constexpr float SWITCH_MELEE = 1.1f;
constexpr float SWITCH_RANGED = 1.3f;
// a pick this close to the runner-up isn't counted
constexpr float PICK_MARGIN = 1.1f;
// GetThreatMod's terms: free distance and falloff, the health mod's cap and slope, the armour sample hit
constexpr float SCORE_MELEE_FREE = 5.0f;
constexpr float SCORE_RANGED_FREE = 35.0f;
constexpr float SCORE_MELEE_FACTOR = 15.0f;
constexpr float SCORE_RANGED_FACTOR = 25.0f;
constexpr uint32 SCORE_HEALTH_CAP = 40000;
constexpr float SCORE_HEALTH_CAPPED = 2.0f;
constexpr float SCORE_HEALTH_BASE = 60000.0f;
constexpr float SCORE_HEALTH_SCALE = 10000.0f;
constexpr uint32 SCORE_ARMOR_SAMPLE = 10000;

constexpr std::array<uint32, 7> REBIRTH_RANKS = {48477, 26994, 20748, 20747, 20742, 20739, 20484};
constexpr float REBIRTH_RANGE = 30.0f;
constexpr float CORPSE_PACK_RADIUS = 8.0f;
constexpr uint32 CORPSE_PACK_MAX = 1;

struct DispelSpell
{
    char const* name;
    std::array<uint32, 2> ranks;  // highest first, 0 pads
    bool removesMagic;
};

constexpr DispelSpell PRIEST_DISPEL_MAGIC{"dispel magic", {988, 527}, true};
constexpr DispelSpell PALADIN_CLEANSE{"cleanse", {4987, 0}, true};
constexpr DispelSpell MAGE_REMOVE_CURSE{"remove curse", {475, 0}, false};
constexpr DispelSpell DRUID_REMOVE_CURSE{"remove curse", {2782, 0}, false};
constexpr DispelSpell DRUID_ABOLISH_POISON{"abolish poison", {2893, 0}, false};
constexpr DispelSpell DRUID_CURE_POISON{"cure poison", {8946, 0}, false};
constexpr DispelSpell SHAMAN_CLEANSE_SPIRIT{"cleanse spirit", {51886, 0}, false};
constexpr DispelSpell SHAMAN_CURE_TOXINS{"cure toxins", {526, 0}, false};
constexpr DispelSpell SHAMAN_PURGE{"purge", {8012, 370}, false};

struct CountedCc
{
    uint32 spellId;
    DispelType type;
    bool healersOnly;
};

// Roots and Psychic Horror left out: a GCD on them buys little
constexpr CountedCc COUNTED_CC[] = {
    {SPELL_POLYMORPH, DISPEL_MAGIC, false},
    {SPELL_FEAR, DISPEL_MAGIC, false},
    {SPELL_PSYCHIC_SCREAM, DISPEL_MAGIC, false},
    {SPELL_REPENTANCE, DISPEL_MAGIC, false},
    {SPELL_HAMMER_OF_JUSTICE_HOLY, DISPEL_MAGIC, false},
    {SPELL_HAMMER_OF_JUSTICE_RET, DISPEL_MAGIC, false},
    {SPELL_SILENCE, DISPEL_MAGIC, true},
    {SPELL_STRANGULATE, DISPEL_MAGIC, true},
    {SPELL_HEX, DISPEL_CURSE, false},
    {SPELL_WYVERN_STING, DISPEL_POISON, false},
};

// Tried in this order on a member holding several
constexpr DispelType CC_DISPEL_ORDER[] = {DISPEL_MAGIC, DISPEL_CURSE, DISPEL_POISON};

struct HazardSource
{
    ObjectGuid guid;
    uint32 spellId = 0;
    Position pos;
    float radius = 0.0f;
    float clearance = 0.0f;
    ObjectGuid kicker;  // the one bot a fresh Hellfire leaves in place
};

// The bot that interrupts a warlock's Hellfire, and its spell
struct KickDuty
{
    ObjectGuid bot;
    char const* spell = nullptr;
    bool melee = false;
};

struct CcDispel
{
    ObjectGuid member;
    char const* spell = nullptr;
};

struct GripDuty
{
    ObjectGuid dk;
    ObjectGuid target;
    Position stand;
    uint32 sinceMs = 0;
    // the stand is wherever the DK is, so the grip goes out at once
    bool fromHere = false;
};

// A DK's grip of a melee champion off its victim to a stand by the arena wall, then its Chains of Ice
struct WallDuty
{
    ObjectGuid champion;
    Position stand;
    uint32 sinceMs = 0;
    uint32 grippedMs = 0;
    bool gripped = false;
};

// A champion, a group member or a corpse, and the spell the bot holding the duty casts on it
struct SpellDuty
{
    ObjectGuid target;
    uint32 spellId = 0;
};

struct LayOnHandsDuty
{
    ObjectGuid member;
    uint32 spellId = 0;
    uint32 sinceMs = 0;
};

struct HealthSample
{
    uint32 ms = 0;
    uint32 health = 0;
};

// player -> threat, sorted by guid, one entry per guid
using ThreatSnapshot = std::vector<std::pair<ObjectGuid, float>>;

struct RepickClock
{
    uint32 repickMs = 0;
    bool seen = false;
    ThreatSnapshot threat;  // each player's threat on the champion last tick
};

struct FactionChampionsDefenceState
{
    ObjectGuid physical{};
    ObjectGuid nextFocus{};
    std::unordered_map<ObjectGuid, RepickClock> clocks;  // champion -> its re-pick clock
    ThreatSnapshot threatScratch;                       // swapped with a clock's snapshot each refresh
    // player -> champions about to pick it, all and physical, inside their lead window only
    std::unordered_map<ObjectGuid, uint32> predicted;
    std::unordered_map<ObjectGuid, uint32> predictedPhysical;
    std::unordered_map<ObjectGuid, ObjectGuid> totemKillers;
    std::unordered_map<ObjectGuid, KickDuty> kicks;  // warlock -> interrupter
    GripDuty grip;
    std::unordered_map<ObjectGuid, WallDuty> walls;       // DK -> its wall grip
    std::unordered_map<ObjectGuid, uint32> airborneMs;    // melee champion -> last seen in the air
    std::unordered_map<ObjectGuid, SpellDuty> freedoms;   // paladin -> group member
    std::unordered_map<ObjectGuid, SpellDuty> snares;     // snarer -> melee champion
    std::unordered_map<ObjectGuid, SpellDuty> peels;      // peeler -> melee champion
    std::unordered_map<ObjectGuid, SpellDuty> protects;   // caster -> group member
    std::unordered_map<ObjectGuid, SpellDuty> battleRes;  // druid -> corpse
    std::unordered_map<ObjectGuid, uint32> layOnHands;    // member -> when its Lay on Hands was dealt
    std::unordered_map<ObjectGuid, LayOnHandsDuty> layOnHandsDuties;  // paladin -> its cast
    std::unordered_map<ObjectGuid, std::vector<HealthSample>> healthLog;
    uint32 healthSampleMs = 0;

    uint32 memoMs = 0;
    bool memoValid = false;  // 0 is a real getMSTime value
    std::vector<HazardSource> hazards;
    std::vector<ObjectGuid> totems;  // champion totems, sorted by guid
    std::unordered_map<ObjectGuid, CcDispel> ccDispels;  // dispeller -> member
    ObjectGuid massDispeller;
    ObjectGuid massDispelTarget;
    ObjectGuid purger;
    ObjectGuid purgeTarget;
    char const* purgeSpell = nullptr;
    ObjectGuid packPurger;
    ObjectGuid packPurgeTarget;
    char const* packPurgeSpell = nullptr;
    std::vector<ObjectGuid> guarded;  // champions nothing may root, stun, fear or knock
    // owner -> the pet autocasts a guard switched off, switched back on once it ends
    std::unordered_map<ObjectGuid, std::vector<uint32>> petHolds;
};

RaidInstanceState<FactionChampionsDefenceState> defenceStates;

// A player's pull at a champion: the bot DKs grip it by post, the player last. Kept apart from the defence
// state, which only exists while the fight is live.
struct GripChainState
{
    ObjectGuid target{};
    FactionChampionsGripChain::Chain turns;
    // the last bot grip of a pull, kept past its close so the player's link is guarded too
    ObjectGuid lastTarget;
    uint32 lastLinkMs = 0;
};

RaidInstanceState<GripChainState> chainStates;

struct Member
{
    Player* player;
    bool healer;
};

// Alive group members on the bot's map, humans included. Just the bot without a group.
std::vector<Member> MembersOnMap(Player* bot)
{
    std::vector<Member> members;
    Map* map = bot->FindMap();
    auto const add = [&members, map](Player* player)
    {
        if (player && player->IsInWorld() && player->IsAlive() && player->FindMap() == map)
            members.push_back({player, PlayerbotAI::IsHeal(player)});
    };

    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
            add(ref->GetSource());
    }
    else
        add(bot);

    return members;
}

void SortByRole(std::vector<Member>& members, bool healersFirst)
{
    std::sort(members.begin(), members.end(),
              [healersFirst](Member const& a, Member const& b)
              {
                  if (a.healer != b.healer)
                      return a.healer == healersFirst;

                  return a.player->GetGUID() < b.player->GetGUID();
              });
}

// A negative duration is a permanent aura
bool LastsAtLeast(Aura const* aura, int32 ms) { return aura && (aura->GetDuration() < 0 || aura->GetDuration() >= ms); }

// Hex silences without taking control, so both are needed
bool InControl(Player* player)
{
    return !player->HasUnitState(UNIT_STATE_LOST_CONTROL) && !player->HasUnitFlag(UNIT_FLAG_SILENCED);
}

uint32 KnownRank(Player* player, DispelSpell const& spell)
{
    for (uint32 rank : spell.ranks)
        if (rank && player->HasSpell(rank))
            return rank;

    return 0;
}

bool CanPayFor(Player* player, uint32 spellId)
{
    SpellInfo const* info = sSpellMgr->GetSpellInfo(spellId);
    if (!info || info->PowerType >= static_cast<uint32>(MAX_POWERS))
        return false;

    return info->CalcPowerCost(player, info->GetSchoolMask()) <=
           static_cast<int32>(player->GetPower(Powers(info->PowerType)));
}

bool CarriesUnstableAffliction(Unit* unit)
{
    return unit && unit->HasAura(sSpellMgr->GetSpellIdForDifficulty(SPELL_UNSTABLE_AFFLICTION, unit));
}

Unit* CurrentTarget(PlayerbotAI* botAI)
{
    return botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Get();
}

struct DispelChoice
{
    DispelSpell const* spell = nullptr;
    uint32 spellId = 0;
};

DispelChoice FirstKnown(Player* player, std::initializer_list<DispelSpell const*> options)
{
    for (DispelSpell const* option : options)
        if (uint32 const rank = KnownRank(player, *option))
            return {option, rank};

    return {};
}

DispelChoice CcDispelSpell(Player* dispeller, DispelType type)
{
    switch (dispeller->getClass())
    {
        case CLASS_PRIEST:
            return type == DISPEL_MAGIC ? FirstKnown(dispeller, {&PRIEST_DISPEL_MAGIC}) : DispelChoice{};
        case CLASS_PALADIN:
            return type == DISPEL_MAGIC || type == DISPEL_POISON ? FirstKnown(dispeller, {&PALADIN_CLEANSE})
                                                                 : DispelChoice{};
        case CLASS_MAGE:
            return type == DISPEL_CURSE ? FirstKnown(dispeller, {&MAGE_REMOVE_CURSE}) : DispelChoice{};
        case CLASS_DRUID:
            if (type == DISPEL_CURSE)
                return FirstKnown(dispeller, {&DRUID_REMOVE_CURSE});

            return type == DISPEL_POISON ? FirstKnown(dispeller, {&DRUID_ABOLISH_POISON, &DRUID_CURE_POISON})
                                         : DispelChoice{};
        case CLASS_SHAMAN:
            if (type == DISPEL_CURSE)
                return FirstKnown(dispeller, {&SHAMAN_CLEANSE_SPIRIT});

            return type == DISPEL_POISON ? FirstKnown(dispeller, {&SHAMAN_CLEANSE_SPIRIT, &SHAMAN_CURE_TOXINS})
                                         : DispelChoice{};
        default:
            return {};
    }
}

// One bit per DispelType of the counted CC the member holds
uint32 CountedCcTypes(Member const& member)
{
    uint32 types = 0;
    for (CountedCc const& cc : COUNTED_CC)
        if ((!cc.healersOnly || member.healer) && LastsAtLeast(member.player->GetAura(cc.spellId), CC_MIN_LEFT_MS))
            types |= 1u << cc.type;

    return types;
}

void UpdateCcDispels(FactionChampionsDefenceState& state, std::vector<Member> const& members)
{
    state.ccDispels.clear();

    std::vector<std::pair<Member, uint32>> held;
    for (Member const& member : members)
        if (uint32 const types = CountedCcTypes(member))
            held.push_back({member, types});

    if (held.empty())
        return;

    std::sort(held.begin(), held.end(),
              [](auto const& a, auto const& b)
              {
                  if (a.first.healer != b.first.healer)
                      return a.first.healer;

                  return a.first.player->GetGUID() < b.first.player->GetGUID();
              });

    // Busy casters skip, so an idle dispeller takes the member instead of waiting on the cast
    std::vector<Member> dispellers;
    for (Member const& member : members)
        if (GET_PLAYERBOT_AI(member.player) && InControl(member.player) && !member.player->IsNonMeleeSpellCast(false))
            dispellers.push_back(member);

    SortByRole(dispellers, false);
    std::vector<bool> taken(dispellers.size(), false);

    for (auto const& [member, types] : held)
    {
        bool const backfires = CarriesUnstableAffliction(member.player);
        bool assigned = false;
        for (DispelType type : CC_DISPEL_ORDER)
        {
            if (!(types & (1u << type)))
                continue;

            for (size_t i = 0; i < dispellers.size() && !assigned; ++i)
            {
                Player* dispeller = dispellers[i].player;
                if (taken[i] || dispeller == member.player)
                    continue;

                // Counterspell, Spell Lock or Earth Shock on a cast locks the school's dispels too
                DispelChoice const choice = CcDispelSpell(dispeller, type);
                if (!choice.spell || (choice.spell->removesMagic && backfires) ||
                    dispeller->HasSpellCooldown(choice.spellId) || !CanPayFor(dispeller, choice.spellId) ||
                    !dispeller->IsWithinDistInMap(member.player, DISPEL_RANGE) ||
                    !dispeller->IsWithinLOSInMap(member.player))
                    continue;

                state.ccDispels[dispeller->GetGUID()] = {member.player->GetGUID(), choice.spell->name};
                taken[i] = true;
                assigned = true;
            }

            if (assigned)
                break;
        }
    }
}

Unit* MassDispelTargetNow(PlayerbotAI* botAI, Unit* killTarget)
{
    if (killTarget && (LastsAtLeast(killTarget->GetAura(SPELL_HAND_OF_PROTECTION), SHIELD_MIN_LEFT_MS) ||
                       LastsAtLeast(killTarget->GetAura(SPELL_DIVINE_SHIELD), SHIELD_MIN_LEFT_MS)))
        return killTarget;

    Unit* suspended = FactionChampionsSuspendedTarget(botAI);
    return suspended && LastsAtLeast(suspended->GetAura(SPELL_DIVINE_SHIELD), SHIELD_MIN_LEFT_MS) ? suspended
                                                                                                 : nullptr;
}

void UpdateMassDispel(FactionChampionsDefenceState& state, PlayerbotAI* botAI, std::vector<Member> const& members,
                      Unit* killTarget)
{
    state.massDispeller.Clear();
    state.massDispelTarget.Clear();

    Unit* target = MassDispelTargetNow(botAI, killTarget);
    if (!target)
        return;

    // The friendly half would pull it off a raider near the point and backlash on the priest
    bool const backfires = std::any_of(members.begin(), members.end(),
                                       [target](Member const& member)
                                       {
                                           return member.player->GetExactDist(target) <= MASS_DISPEL_ALLY_RADIUS &&
                                                  CarriesUnstableAffliction(member.player);
                                       });
    if (backfires)
        return;

    std::vector<Member> priests;
    for (Member const& member : members)
    {
        Player* player = member.player;
        if (player->getClass() == CLASS_PRIEST && GET_PLAYERBOT_AI(player) && player->HasSpell(SPELL_MASS_DISPEL) &&
            !player->HasSpellCooldown(SPELL_MASS_DISPEL) && InControl(player) && CanPayFor(player, SPELL_MASS_DISPEL))
            priests.push_back(member);
    }

    SortByRole(priests, false);
    for (Member const& priest : priests)
    {
        // A point cast: the core adds the priest's own size to the range, not the champion's
        if (priest.player->IsWithinDist3d(target, MASS_DISPEL_RANGE) && priest.player->IsWithinLOSInMap(target))
        {
            state.massDispeller = priest.player->GetGUID();
            state.massDispelTarget = target->GetGUID();
            return;
        }
    }
}

// Thorns and Nature's Grasp don't count: Thorns alone would keep a purger busy all pull
bool WorthPurging(Unit* target)
{
    uint32 const buffs[] = {
        SPELL_HAND_OF_PROTECTION,
        SPELL_EARTH_SHIELD,
        SPELL_HEROISM,
        SPELL_BLOODLUST,
        SPELL_AVENGING_WRATH,
        SPELL_BARKSKIN,
        sSpellMgr->GetSpellIdForDifficulty(SPELL_HAND_OF_FREEDOM, target),
        sSpellMgr->GetSpellIdForDifficulty(SPELL_POWER_WORD_SHIELD, target),
        sSpellMgr->GetSpellIdForDifficulty(SPELL_RENEW, target),
        sSpellMgr->GetSpellIdForDifficulty(SPELL_RIPTIDE, target),
        sSpellMgr->GetSpellIdForDifficulty(SPELL_REJUVENATION, target),
        sSpellMgr->GetSpellIdForDifficulty(SPELL_LIFEBLOOM, target),
        sSpellMgr->GetSpellIdForDifficulty(SPELL_REGROWTH, target),
    };

    return std::any_of(std::begin(buffs), std::end(buffs), [target](uint32 buff) { return target->HasAura(buff); });
}

// Bloodlust and Avenging Wrath feed the champions' damage on the raid, Hand of Freedom shrugs off the
// snares and an ice duty's Chains of Ice
bool CarriesPackBuff(Unit* champion)
{
    return champion->HasAura(SPELL_HEROISM) || champion->HasAura(SPELL_BLOODLUST) ||
           champion->HasAura(SPELL_AVENGING_WRATH) ||
           champion->HasAura(sSpellMgr->GetSpellIdForDifficulty(SPELL_HAND_OF_FREEDOM, champion));
}

// Purge or Dispel Magic ready, by a dps shaman or priest bot
DispelSpell const* ReadyPurge(Member const& member)
{
    Player* player = member.player;
    if (member.healer || !GET_PLAYERBOT_AI(player))
        return nullptr;

    DispelSpell const* option = player->getClass() == CLASS_SHAMAN   ? &SHAMAN_PURGE
                                : player->getClass() == CLASS_PRIEST ? &PRIEST_DISPEL_MAGIC
                                                                     : nullptr;
    uint32 const rank = option ? KnownRank(player, *option) : 0;
    return rank && !player->HasSpellCooldown(rank) && CanPayFor(player, rank) ? option : nullptr;
}

// Class purges on champions are vetoed, so a purger with no shot at the kill target hands the duty on.
// The next purger takes the pack's offensive buffs, melee champions on an ice duty first.
void UpdatePurge(FactionChampionsDefenceState& state, std::vector<Member> const& members,
                 std::vector<Unit*> const& champions, Unit* killTarget)
{
    state.purger.Clear();
    state.purgeTarget.Clear();
    state.purgeSpell = nullptr;
    state.packPurger.Clear();
    state.packPurgeTarget.Clear();
    state.packPurgeSpell = nullptr;

    Player* purger = nullptr;
    DispelSpell const* spell = nullptr;
    for (Member const& member : members)
    {
        Player* player = member.player;
        DispelSpell const* option = ReadyPurge(member);
        if (!option)
            continue;

        if (killTarget && (!InControl(player) || !player->IsWithinDistInMap(killTarget, PURGE_RANGE)))
            continue;

        if (!purger || player->GetGUID() < purger->GetGUID())
        {
            purger = player;
            spell = option;
        }
    }

    if (purger)
    {
        state.purger = purger->GetGUID();
        state.purgeSpell = spell->name;
        if (killTarget && WorthPurging(killTarget))
            state.purgeTarget = killTarget->GetGUID();
    }

    std::vector<Unit*> buffed;
    for (Unit* champion : champions)
        if (champion != killTarget && CarriesPackBuff(champion))
            buffed.push_back(champion);

    auto const onDuty = [&members](Unit* champion)
    {
        return std::any_of(members.begin(), members.end(), [champion](Member const& member)
                           { return FactionChampionsIceDutyOf(member.player) == champion->GetGUID(); });
    };

    std::stable_sort(buffed.begin(), buffed.end(), [&onDuty](Unit* a, Unit* b) { return onDuty(a) && !onDuty(b); });

    for (Unit* champion : buffed)
    {
        Player* chosen = nullptr;
        DispelSpell const* chosenSpell = nullptr;
        for (Member const& member : members)
        {
            Player* player = member.player;
            DispelSpell const* option = player != purger ? ReadyPurge(member) : nullptr;
            if (!option || !InControl(player) || !player->IsWithinDistInMap(champion, PURGE_RANGE) ||
                (chosen && chosen->GetGUID() < player->GetGUID()))
                continue;

            chosen = player;
            chosenSpell = option;
        }

        if (chosen)
        {
            state.packPurger = chosen->GetGUID();
            state.packPurgeTarget = champion->GetGUID();
            state.packPurgeSpell = chosenSpell->name;
            return;
        }
    }
}

bool PhysicallyImmune(Unit* unit) { return unit->IsImmunedToDamage(SPELL_SCHOOL_MASK_NORMAL); }

// Hand of Protection. Each half asked apart: Ice Block is a physical aura plus a magic one, and
// IsImmunedToDamage wants one aura covering the whole mask
void UpdatePhysicalLatch(FactionChampionsDefenceState& state, PlayerbotAI* botAI, Unit* killTarget)
{
    ObjectGuid next;
    if (killTarget && PhysicallyImmune(killTarget) && !killTarget->IsImmunedToDamage(SPELL_SCHOOL_MASK_MAGIC))
    {
        ObjectGuid const latched = state.physical;
        Unit* pick = nullptr;
        // never one in the pack, the melee would follow it in
        auto const isLatched = [botAI, latched](Unit* champion)
        {
            return champion->GetGUID() == latched && !PhysicallyImmune(champion) &&
                   FactionChampionsIsPulled(botAI, champion);
        };
        if (!latched.IsEmpty())
            pick = FactionChampionsNextTarget(botAI, isLatched);

        if (!pick)
        {
            auto const isOut = [botAI](Unit* champion)
            { return !PhysicallyImmune(champion) && FactionChampionsIsPulled(botAI, champion); };
            pick = FactionChampionsNextTarget(botAI, isOut);
        }

        if (pick)
            next = pick->GetGUID();
    }

    // empty once the immunity ends, which ends the physical switch
    state.physical = next;
}

bool IsWarrior(uint32 entry)
{
    return entry == static_cast<uint32>(ToCFactionChampions::NPC_ALLIANCE_WARRIOR) ||
           entry == static_cast<uint32>(ToCFactionChampions::NPC_HORDE_WARRIOR);
}

// AoE casters and totems, so one grid walk covers both
std::vector<uint32> const& ScanEntries()
{
    static std::vector<uint32> const entries = {
        static_cast<uint32>(ToCFactionChampions::NPC_ALLIANCE_WARRIOR),
        static_cast<uint32>(ToCFactionChampions::NPC_HORDE_WARRIOR),
        static_cast<uint32>(ToCFactionChampions::NPC_ALLIANCE_WARLOCK),
        static_cast<uint32>(ToCFactionChampions::NPC_HORDE_WARLOCK),
        NPC_GROUNDING_TOTEM,
        NPC_WINDFURY_TOTEM,
        NPC_TREMOR_TOTEM,
    };

    return entries;
}

// The raid's own totems share the entries
bool IsChampionTotem(Player* bot, Creature* totem)
{
    ObjectGuid const owner = totem->GetOwnerGUID();
    return owner.IsCreature() && IsFactionChampion(owner.GetEntry()) && bot->IsValidAttackTarget(totem);
}

// Cooldown only, no CanCastSpell or power test: the top ranks of Kick, Pummel and Shield Bash sit on
// the GCD and energy dips under Kick's cost between swings, so either would flip the kicker every swing
char const* ReadyMeleeInterrupt(Player* player, PlayerbotAI* botAI, Unit* warlock)
{
    if (!player->IsWithinMeleeRange(warlock))
        return nullptr;

    for (char const* name : MELEE_INTERRUPTS)
    {
        uint32 const id = botAI->GetAiObjectContext()->GetValue<uint32>("spell id", name)->Get();
        if (id && player->HasSpell(id) && !player->HasSpellCooldown(id))
            return name;
    }

    return nullptr;
}

// Off the GCD, but a caster mid-cast can't send one
char const* ReadyRangedInterrupt(Player* player, PlayerbotAI* botAI, Unit* warlock)
{
    if (player->IsNonMeleeSpellCast(false))
        return nullptr;

    for (char const* name : RANGED_INTERRUPTS)
    {
        uint32 const id = botAI->GetAiObjectContext()->GetValue<uint32>("spell id", name)->Get();
        SpellInfo const* info = id ? sSpellMgr->GetSpellInfo(id) : nullptr;
        if (info && player->HasSpell(id) && !player->HasSpellCooldown(id) &&
            player->IsWithinCombatRange(warlock, info->GetMaxRange(false, player)) &&
            player->IsWithinLOSInMap(warlock))
            return name;
    }

    Pet* pet = player->GetPet();
    if (pet && pet->IsAlive() && !pet->HasUnitState(UNIT_STATE_LOST_CONTROL))
        for (uint32 rank : SPELL_LOCK_RANKS)
            if (pet->HasSpell(rank) && !pet->HasSpellCooldown(rank) && pet->IsWithinCombatRange(warlock, SPELL_LOCK_RANGE))
                return "spell lock";

    return nullptr;
}

// Nearest bot with an interrupt that reaches the warlock, a ranged one first: it needn't stand in the fire
KickDuty ElectChannelKicker(std::vector<Member> const& members, Unit* warlock)
{
    KickDuty best;
    float bestDistance = 0.0f;
    for (bool melee : {false, true})
    {
        for (Member const& member : members)
        {
            Player* player = member.player;
            PlayerbotAI* memberAI = GET_PLAYERBOT_AI(player);
            if (!memberAI || !InControl(player))
                continue;

            char const* spell = melee ? ReadyMeleeInterrupt(player, memberAI, warlock)
                                      : ReadyRangedInterrupt(player, memberAI, warlock);
            float const distance = player->GetExactDist2d(warlock);
            if (spell && (best.bot.IsEmpty() || distance < bestDistance))
            {
                best = {player->GetGUID(), spell, melee};
                bestDistance = distance;
            }
        }

        if (!best.bot.IsEmpty())
            break;
    }

    return best;
}

// The holder keeps it while its interrupt is still ready, a second tick may find the channel already ended
KickDuty KeepOrElectKicker(FactionChampionsDefenceState& state, std::vector<Member> const& members, Unit* warlock)
{
    auto const held = state.kicks.find(warlock->GetGUID());
    if (held != state.kicks.end())
    {
        ObjectGuid const bot = held->second.bot;
        auto const holder = std::find_if(members.begin(), members.end(),
                                         [bot](Member const& member) { return member.player->GetGUID() == bot; });
        PlayerbotAI* holderAI = holder != members.end() ? GET_PLAYERBOT_AI(holder->player) : nullptr;
        if (holderAI && InControl(holder->player))
        {
            Player* player = holder->player;
            char const* spell = held->second.melee ? ReadyMeleeInterrupt(player, holderAI, warlock)
                                                   : ReadyRangedInterrupt(player, holderAI, warlock);
            if (spell)
                return {player->GetGUID(), spell, held->second.melee};
        }
    }

    return ElectChannelKicker(members, warlock);
}

void UpdateHazards(FactionChampionsDefenceState& state, Player* bot, std::vector<Member> const& members)
{
    static ToCEntrySet const scanSet(ScanEntries());
    std::vector<Creature*> creatures;
    GetCreatureListWithEntriesInGrid(bot, creatures, scanSet, CHAMPION_SEARCH_RADIUS);

    state.hazards.clear();
    state.totems.clear();
    std::unordered_map<ObjectGuid, KickDuty> kicks;
    for (Creature* creature : creatures)
    {
        if (!creature || !creature->IsAlive())
            continue;

        if (creature->IsTotem())
        {
            if (IsChampionTotem(bot, creature))
                state.totems.push_back(creature->GetGUID());

            continue;
        }

        HazardSource source;
        source.guid = creature->GetGUID();
        source.pos = creature->GetPosition();
        if (IsWarrior(creature->GetEntry()))
        {
            if (!LastsAtLeast(creature->GetAura(SPELL_BLADESTORM), BLADESTORM_MIN_LEFT_MS))
                continue;

            source.spellId = SPELL_BLADESTORM;
            source.radius = BLADESTORM_RADIUS;
            source.clearance = BLADESTORM_CLEARANCE;
        }
        else
        {
            uint32 const hellfire = sSpellMgr->GetSpellIdForDifficulty(SPELL_HELLFIRE, creature);
            Aura const* aura = creature->GetAura(hellfire);
            if (!aura)
                continue;

            source.spellId = hellfire;
            source.radius = HELLFIRE_RADIUS;
            source.clearance = HELLFIRE_CLEARANCE;
            KickDuty const kick = KeepOrElectKicker(state, members, creature);
            if (!kick.bot.IsEmpty())
                kicks[creature->GetGUID()] = kick;

            // only a melee interrupt needs its kicker left inside the fire
            if (kick.melee && aura->GetMaxDuration() >= 0 &&
                aura->GetMaxDuration() - aura->GetDuration() < HELLFIRE_KICK_WINDOW_MS)
                source.kicker = kick.bot;
        }

        state.hazards.push_back(source);
    }

    state.kicks = std::move(kicks);
    std::sort(state.totems.begin(), state.totems.end());
}

bool CanKillTotem(Member const& member, Creature* totem)
{
    Player* player = member.player;
    return !member.healer && GET_PLAYERBOT_AI(player) && !player->HasUnitState(UNIT_STATE_LOST_CONTROL) &&
           player->GetExactDist2d(totem) <= TOTEM_DUTY_RANGE;
}

// Latched, so a bot doesn't swap totems as the distances shift
void UpdateTotemKillers(FactionChampionsDefenceState& state, Player* bot, std::vector<Member> const& members)
{
    Map* map = bot->FindMap();
    auto const countedTotem = [&state, map](ObjectGuid guid) -> Creature*
    {
        bool const counts = std::find(state.totems.begin(), state.totems.end(), guid) != state.totems.end();
        return counts && map ? map->GetCreature(guid) : nullptr;
    };

    std::vector<ObjectGuid> released;
    for (auto const& hold : state.totemKillers)
    {
        Creature* totem = countedTotem(hold.second);
        ObjectGuid const killerGuid = hold.first;
        auto const killer = std::find_if(members.begin(), members.end(), [killerGuid](Member const& member)
                                         { return member.player->GetGUID() == killerGuid; });
        if (!totem || killer == members.end() || !CanKillTotem(*killer, totem))
            released.push_back(hold.first);
    }

    for (ObjectGuid const& killer : released)
        state.totemKillers.erase(killer);

    for (ObjectGuid const& guid : state.totems)
    {
        auto const& holds = state.totemKillers;
        bool const held =
            std::any_of(holds.begin(), holds.end(), [guid](auto const& hold) { return hold.second == guid; });
        Creature* totem = countedTotem(guid);
        if (held || !totem)
            continue;

        Player* nearest = nullptr;
        float nearestDist = 0.0f;
        for (Member const& member : members)
        {
            if (!CanKillTotem(member, totem) || holds.count(member.player->GetGUID()))
                continue;

            float const dist = member.player->GetExactDist2d(totem);
            bool const closer = !nearest || dist < nearestDist ||
                                (dist == nearestDist && member.player->GetGUID() < nearest->GetGUID());
            if (closer)
            {
                nearest = member.player;
                nearestDist = dist;
            }
        }

        if (nearest)
            state.totemKillers[nearest->GetGUID()] = guid;
    }
}

template <std::size_t N>
uint32 FirstKnownRank(Player* player, std::array<uint32, N> const& ranks)
{
    for (uint32 rank : ranks)
        if (player->HasSpell(rank))
            return rank;

    return 0;
}

bool SpellReady(Player* player, uint32 spellId)
{
    return spellId && player->HasSpell(spellId) && !player->HasSpellCooldown(spellId) && CanPayFor(player, spellId);
}

// Not casting, so an instant goes out this tick and a dodge isn't pinned
bool FreeToAct(Player* player) { return InControl(player) && !player->IsNonMeleeSpellCast(false); }

bool IsTankMember(Player* player) { return PlayerbotAI::IsTank(player) || PlayerbotAI::IsTank(player, true); }

std::vector<Member>::const_iterator FindMember(std::vector<Member> const& members, ObjectGuid guid)
{
    return std::find_if(members.begin(), members.end(),
                        [guid](Member const& member) { return member.player->GetGUID() == guid; });
}

bool IsPhysicalChampion(uint32 entry)
{
    return IsFactionChampionMelee(entry) || entry == static_cast<uint32>(ToCFactionChampions::NPC_ALLIANCE_HUNTER) ||
           entry == static_cast<uint32>(ToCFactionChampions::NPC_HORDE_HUNTER);
}

bool GripLands(Unit* target)
{
    SpellInfo const* info = sSpellMgr->GetSpellInfo(SPELL_DEATH_GRIP);
    return info && target->IsInCombat() && !target->IsImmunedToSpell(info);
}

// Still inside the pack or in heal reach of another champion healer
bool WantsPull(Unit* target, std::vector<Unit*> const& champions, Position const& packCentre)
{
    if (target->GetExactDist2d(&packCentre) < GRIP_GOAL)
        return true;

    return std::any_of(champions.begin(), champions.end(),
                       [target](Unit* champion)
                       {
                           return champion != target && IsFactionChampionHealer(champion->GetEntry()) &&
                                  champion->GetExactDist2d(target) <= CHAMPION_HEAL_REACH;
                       });
}

// Before the pull too, where GripLands' combat test would refuse every champion
bool GripCanLand(Unit* target)
{
    SpellInfo const* info = sSpellMgr->GetSpellInfo(SPELL_DEATH_GRIP);
    return info && !target->IsImmunedToSpell(info);
}

// Death Grip's pull, or a Leap, still in the air
bool MidJump(Unit* unit)
{
    return unit->GetMotionMaster()->GetMotionSlotType(MOTION_SLOT_CONTROLLED) == EFFECT_MOTION_TYPE;
}

GripChainState* ChainFor(Player* bot)
{
    if (!bot || !bot->GetInstanceId() || bot->GetMapId() != TRIAL_OF_THE_CRUSADER_MAP_ID)
        return nullptr;

    InstanceScript* instance = bot->GetInstanceScript();
    if (!instance ||
        ToCEncounterOfStage(instance->GetData(TOC_DATA_INSTANCE_PROGRESS)) != ToCEncounter::FactionChampions)
        return nullptr;

    return &chainStates.For(bot->GetInstanceId());
}

void CloseChain(GripChainState& chain)
{
    chain.target = ObjectGuid::Empty;
    chain.turns = FactionChampionsGripChain::Chain();
}

Player* ChainDk(Player* bot, uint64 guid)
{
    Player* dk = ObjectAccessor::GetPlayer(*bot, ObjectGuid(guid));
    return dk && dk->IsInWorld() && dk->FindMap() == bot->FindMap() ? dk : nullptr;
}

// Skips every link that can't grip
void AdvanceChain(Player* bot, GripChainState& chain, uint32 now)
{
    using namespace FactionChampionsGripChain;
    auto const stateOf = [bot](uint64 guid)
    {
        Player* dk = ChainDk(bot, guid);
        LinkState link;
        link.present = dk && dk->IsAlive() && InControl(dk);
        link.gripCooldownMs = link.present ? dk->GetSpellCooldownDelay(SPELL_DEATH_GRIP) : 0;
        return link;
    };

    for (Skipped const& skip : Advance(chain.turns, now, stateOf))
    {
        Player* dk = ChainDk(bot, skip.guid);
        if (!dk)
            continue;

        PlayerbotAI* dkAI = skip.reason == Skip::Cooldown ? GET_PLAYERBOT_AI(dk) : nullptr;
        if (dkAI)
        {
            uint32 const cooldown = dk->GetSpellCooldownDelay(SPELL_DEATH_GRIP);
            dkAI->TellMaster(PlayerbotTextMgr::instance().GetBotTextOrDefault(
                "toc_death_grip_skip", "Death Grip ready in %seconds s, skipping my grip",
                {{"%seconds", std::to_string((cooldown + 999) / 1000)}}));
        }
    }
}

// The open pull's champion, alive, after passing the turn on from links that can't grip
Creature* OpenChain(Player* bot, GripChainState*& chain)
{
    chain = ChainFor(bot);
    if (!chain || chain->target.IsEmpty())
        return nullptr;

    Map* map = bot->FindMap();
    Creature* target = map ? map->GetCreature(chain->target) : nullptr;
    if (target && target->IsAlive())
        AdvanceChain(bot, *chain, getMSTime());

    if (!target || !target->IsAlive() || FactionChampionsGripChain::Closed(chain->turns))
    {
        CloseChain(*chain);
        return nullptr;
    }

    return target;
}

// The bot DKs knowing Death Grip and in control get the links, by post. With none the pull still guards
// its champion while the player grips.
void StartChain(Player* bot, GripChainState& chain, Unit* target)
{
    std::vector<FactionChampionsGripChain::Dk> dks;
    for (Member const& member : MembersOnMap(bot))
    {
        Player* player = member.player;
        if (player->getClass() == CLASS_DEATH_KNIGHT && GET_PLAYERBOT_AI(player) &&
            player->HasSpell(SPELL_DEATH_GRIP) && InControl(player))
            dks.push_back({player->GetGUID().GetRawValue(), PlayerbotAI::IsTank(player)});
    }

    uint32 const now = getMSTime();
    CloseChain(chain);
    chain.target = target->GetGUID();
    chain.turns = FactionChampionsGripChain::Open(FactionChampionsGripChain::OrderLinks(dks), now);

    if (FactionChampionsGripChain::Closed(chain.turns))
    {
        chain.lastTarget = target->GetGUID();
        chain.lastLinkMs = now;
    }
}

// With no pull open, a kill target already pulled out that still wants a pull: a grip someone made goes on
Unit* GripCandidate(PlayerbotAI* botAI, std::vector<Unit*> const& champions, Unit* killTarget,
                    Position const& packCentre)
{
    GripChainState* chain = nullptr;
    if (OpenChain(botAI->GetBot(), chain))
        return nullptr;

    if (killTarget && !MidJump(killTarget) && FactionChampionsIsPulled(botAI, killTarget) && GripLands(killTarget) &&
        WantsPull(killTarget, champions, packCentre))
        return killTarget;

    return nullptr;
}

uint32 WalkMs(Player* player, Position const& spot)
{
    float const speed = player->GetSpeed(MOVE_RUN);
    return speed > 0.0f ? static_cast<uint32>(player->GetExactDist2d(&spot) / speed * IN_MILLISECONDS) : 0;
}

// Ready by the cast, or with walk by the time the DK reaches its stand
bool GripReadyInTime(Player* dk, Position const& stand, bool walk)
{
    uint32 const window = GRIP_READY_SLACK_MS + (walk ? WalkMs(dk, stand) : 0);
    return dk->GetSpellCooldownDelay(SPELL_DEATH_GRIP) <= window;
}

// In reach now, and the pull still carries the target away from the pack
bool GripsFromHere(Player* dk, Unit* target, Position const& packCentre)
{
    return dk->GetExactDist2d(&packCentre) >= target->GetExactDist2d(&packCentre) + GRIP_MIN_GAIN &&
           dk->IsWithinCombatRange(target, GRIP_RANGE) && dk->IsWithinLOSInMap(target);
}

// fromHere: only a DK that grips the target from where it stands. A DK with no ice duty first, its Chains of
// Ice lapse while it walks.
Player* ElectGripper(std::vector<Member> const& members, Unit* target, Position const& stand, bool fromHere,
                     Position const& packCentre)
{
    Player* chosen = nullptr;
    bool chosenIce = false;
    for (Member const& member : members)
    {
        Player* player = member.player;
        if (player->getClass() != CLASS_DEATH_KNIGHT || !GET_PLAYERBOT_AI(player) || !InControl(player) ||
            !player->HasSpell(SPELL_DEATH_GRIP) ||
            !GripReadyInTime(player, fromHere ? player->GetPosition() : stand, false))
            continue;

        if (fromHere ? !GripsFromHere(player, target, packCentre) : player->GetExactDist2d(target) > GRIP_ELECT_REACH)
            continue;

        bool const ice = !FactionChampionsIceDutyOf(player).IsEmpty();
        if (!chosen || (ice != chosenIce ? !ice : player->GetGUID() < chosen->GetGUID()))
        {
            chosen = player;
            chosenIce = ice;
        }
    }

    return chosen;
}

// The champion lands where the DK stands, so the stand on the floor farthest from the pack. Chained
// grips, a human's included, each carry it another 27 yd.
bool DeriveGripStand(Unit* target, Position const& packCentre, Position& stand)
{
    bool found = false;
    float best = 0.0f;
    for (uint32 bearing = 0; bearing < GRIP_BEARINGS; ++bearing)
    {
        float const angle = 2.0f * static_cast<float>(M_PI) * bearing / GRIP_BEARINGS;
        Position const spot(target->GetPositionX() + GRIP_STAND_DIST * std::cos(angle),
                            target->GetPositionY() + GRIP_STAND_DIST * std::sin(angle), target->GetPositionZ());
        if (ARENA_CENTER.GetExactDist2d(spot) > ARENA_FLOOR_RADIUS)
            continue;

        float const away = packCentre.GetExactDist2d(spot);
        if (!found || away > best)
        {
            stand = spot;
            best = away;
            found = true;
        }
    }

    return found;
}

// Latched until the grip goes out, the target changes or 8 s pass, so the DK walks to one spot. Elected
// fresh every tick until then, so a DK in reach of the landing grips with no walk.
void UpdateGrip(FactionChampionsDefenceState& state, PlayerbotAI* botAI, std::vector<Member> const& members,
                std::vector<Unit*> const& champions, Unit* killTarget, uint32 now)
{
    GripDuty& duty = state.grip;
    Position anchor;
    Position centre;
    Unit* target = nullptr;
    if (FactionChampionsPack(botAI, anchor, centre))
        target = GripCandidate(botAI, champions, killTarget, centre);

    if (!duty.dk.IsEmpty())
    {
        auto const holder = FindMember(members, duty.dk);
        Player* dk = holder != members.end() ? holder->player : nullptr;
        bool const holds = dk && InControl(dk) && target && target->GetGUID() == duty.target &&
                           GripReadyInTime(dk, duty.stand, false) && getMSTimeDiff(duty.sinceMs, now) < GRIP_DUTY_MS &&
                           (duty.fromHere ? GripsFromHere(dk, target, centre)
                                          : target->GetExactDist2d(&duty.stand) <= GRIP_KEEP_RANGE);
        if (holds)
        {
            if (duty.fromHere)
                duty.stand = dk->GetPosition();

            return;
        }

        duty = GripDuty();
    }

    Player* dk = nullptr;
    Position stand;
    bool fromHere = false;
    if (target)
    {
        dk = ElectGripper(members, target, stand, true, centre);
        fromHere = dk != nullptr;
        if (!dk && DeriveGripStand(target, centre, stand))
            dk = ElectGripper(members, target, stand, false, centre);
    }

    if (!dk)
        return;

    duty.fromHere = fromHere;
    duty.dk = dk->GetGUID();
    duty.target = target->GetGUID();
    duty.stand = fromHere ? dk->GetPosition() : stand;
    duty.sinceMs = now;
}

bool FrostRuneReady(Player* player)
{
    for (uint8 index = 0; index < MAX_RUNES; ++index)
    {
        RuneType const rune = player->GetCurrentRune(index);
        if (!player->GetRuneCooldown(index) && (rune == RUNE_FROST || rune == RUNE_DEATH))
            return true;
    }

    return false;
}

// The snare this bot can put on champion from where it stands, 0 when none
uint32 ReadySnare(Player* snarer, Unit* champion)
{
    bool const inMelee = snarer->IsWithinMeleeRange(champion);
    switch (snarer->getClass())
    {
        case CLASS_WARRIOR:
            if (inMelee && SpellReady(snarer, SPELL_HAMSTRING))
                return SPELL_HAMSTRING;

            return snarer->GetExactDist2d(champion) <= PIERCING_HOWL_RADIUS && SpellReady(snarer, SPELL_PIERCING_HOWL)
                       ? SPELL_PIERCING_HOWL
                       : 0;
        case CLASS_DEATH_KNIGHT:
            return snarer->IsWithinCombatRange(champion, CHAINS_OF_ICE_RANGE) &&
                           SpellReady(snarer, SPELL_CHAINS_OF_ICE) && FrostRuneReady(snarer)
                       ? SPELL_CHAINS_OF_ICE
                       : 0;
        case CLASS_HUNTER:
            if (inMelee)
                return SpellReady(snarer, SPELL_WING_CLIP) ? SPELL_WING_CLIP : 0;

            return snarer->IsWithinCombatRange(champion, CONCUSSIVE_SHOT_RANGE) &&
                           SpellReady(snarer, SPELL_CONCUSSIVE_SHOT)
                       ? SPELL_CONCUSSIVE_SHOT
                       : 0;
        case CLASS_SHAMAN:
        {
            uint32 const rank = FirstKnownRank(snarer, FROST_SHOCK_RANKS);
            return snarer->IsWithinCombatRange(champion, FROST_SHOCK_RANGE) && SpellReady(snarer, rank) ? rank : 0;
        }
        default:
            return 0;
    }
}

// Hand of Freedom, or Divine Shield, Ice Block and Cyclone
bool SnareImmune(Unit* champion)
{
    if (champion->HasAuraTypeWithMiscvalue(SPELL_AURA_MECHANIC_IMMUNITY, MECHANIC_SNARE))
        return true;

    return champion->IsImmunedToDamage(SPELL_SCHOOL_MASK_NORMAL) &&
           champion->IsImmunedToDamage(SPELL_SCHOOL_MASK_MAGIC);
}

// The champion's victim as a group member, null when it's on none
Member const* VictimMember(std::vector<Member> const& members, Unit* champion)
{
    Unit* victim = champion->GetVictim();
    auto const member = victim ? FindMember(members, victim->GetGUID()) : members.end();
    return member != members.end() ? &*member : nullptr;
}

// Not the grip duty's DK or a grip chain link still to grip, and no pack tank, which holds the pack, unless
// it has an ice duty
bool WallDk(FactionChampionsDefenceState const& state, Player* player)
{
    PlayerbotAI* playerAI = GET_PLAYERBOT_AI(player);
    return player->getClass() == CLASS_DEATH_KNIGHT && playerAI && player->HasSpell(SPELL_DEATH_GRIP) &&
           InControl(player) && player->GetGUID() != state.grip.dk && !FactionChampionsChainLink(playerAI) &&
           (!FactionChampionsIceDutyOf(player).IsEmpty() || !FactionChampionsIsPackTank(player));
}

// An ice duty DK takes only its own champion, any other DK one on a raider that's not a tank and not on
// another DK's ice duty
bool WallVictim(std::vector<Member> const& members, Player* dk, Unit* champion, Member const* victim)
{
    if (!victim || victim->player == dk)
        return false;

    ObjectGuid const own = FactionChampionsIceDutyOf(dk);
    if (!own.IsEmpty())
        return own == champion->GetGUID();

    return !IsTankMember(victim->player) &&
           std::none_of(members.begin(), members.end(), [champion](Member const& member)
                        { return FactionChampionsIceDutyOf(member.player) == champion->GetGUID(); });
}

// The 8 s since anyone's grip keeps a human's grip where it put the champion
bool WallGrippable(FactionChampionsDefenceState const& state, PlayerbotAI* botAI, Unit* champion, Unit* killTarget,
                   uint32 now)
{
    auto const air = state.airborneMs.find(champion->GetGUID());
    return IsFactionChampionMelee(champion->GetEntry()) && champion != killTarget && !MidJump(champion) &&
           (air == state.airborneMs.end() || getMSTimeDiff(air->second, now) >= WALL_AIRBORNE_MS) &&
           !champion->HasAuraType(SPELL_AURA_MOD_ROOT) && !champion->HasUnitState(UNIT_STATE_LOST_CONTROL) &&
           std::find(state.guarded.begin(), state.guarded.end(), champion->GetGUID()) == state.guarded.end() &&
           GripLands(champion) && !FactionChampionsCcProtected(botAI, champion);
}

// The champion lands where the DK stands, so the stand on the floor 34-44 yd off ARENA_CENTER, 27 or 22 yd
// off the champion, farthest from its victim and 12+ yd farther than the champion is now
bool DeriveWallStand(FactionChampionsDefenceState const& state, Player* dk, Unit* champion, Unit* victim,
                     std::vector<Member> const& members, Position& stand)
{
    float const minAway = victim->GetExactDist2d(champion) + GRIP_MIN_GAIN;
    bool found = false;
    float best = 0.0f;
    for (float distance : WALL_STAND_DISTS)
    {
        for (uint32 bearing = 0; bearing < GRIP_BEARINGS; ++bearing)
        {
            float const angle = 2.0f * static_cast<float>(M_PI) * bearing / GRIP_BEARINGS;
            Position const spot(champion->GetPositionX() + distance * std::cos(angle),
                                champion->GetPositionY() + distance * std::sin(angle), champion->GetPositionZ());
            float const offCentre = ARENA_CENTER.GetExactDist2d(spot);
            float const away = victim->GetExactDist2d(&spot);
            if (offCentre > ARENA_FLOOR_RADIUS || offCentre < WALL_MIN_OFF_CENTRE || away < minAway ||
                (found && away <= best) || WalkMs(dk, spot) > WALL_WALK_MAX_MS)
                continue;

            bool const healed = std::any_of(members.begin(), members.end(),
                                            [dk, &spot](Member const& member)
                                            {
                                                return member.healer && member.player != dk &&
                                                       member.player->GetExactDist2d(&spot) <= WALL_HEAL_REACH;
                                            });
            bool const clear = std::none_of(state.hazards.begin(), state.hazards.end(),
                                            [&spot](HazardSource const& source)
                                            { return spot.GetExactDist2d(source.pos) < source.clearance; });
            if (!healed || !clear)
                continue;

            stand = spot;
            best = away;
            found = true;
        }
    }

    return found;
}

// Latched through the grip to its Chains of Ice. Before the grip it drops once the cooldown outlasts the walk,
// the stand is past 29 yd of the champion, 8 s pass, someone else grips it, or it leaves its victim.
void UpdateWalls(FactionChampionsDefenceState& state, PlayerbotAI* botAI, std::vector<Member> const& members,
                 std::vector<Unit*> const& champions, Unit* killTarget, uint32 now)
{
    for (Unit* champion : champions)
        if (IsFactionChampionMelee(champion->GetEntry()) && MidJump(champion))
            state.airborneMs[champion->GetGUID()] = now;

    auto const findChampion = [&champions](ObjectGuid guid) -> Unit*
    {
        auto const found = std::find_if(champions.begin(), champions.end(),
                                        [guid](Unit* champion) { return champion->GetGUID() == guid; });
        return found != champions.end() ? *found : nullptr;
    };

    std::vector<ObjectGuid> released;
    for (auto const& [dkGuid, duty] : state.walls)
    {
        auto const member = FindMember(members, dkGuid);
        Player* dk = member != members.end() ? member->player : nullptr;
        Unit* champion = dk ? findChampion(duty.champion) : nullptr;
        bool keep = champion && champion != killTarget && WallDk(state, dk);
        if (keep && duty.gripped)
            keep = getMSTimeDiff(duty.grippedMs, now) < WALL_CHAIN_MS &&
                   !champion->HasAuraType(SPELL_AURA_MOD_DECREASE_SPEED) && !SnareImmune(champion);
        else if (keep)
            keep = getMSTimeDiff(duty.sinceMs, now) < GRIP_DUTY_MS && GripReadyInTime(dk, duty.stand, true) &&
                   champion->GetExactDist2d(&duty.stand) <= GRIP_KEEP_RANGE &&
                   WallVictim(members, dk, champion, VictimMember(members, champion)) &&
                   WallGrippable(state, botAI, champion, killTarget, now);

        if (!keep)
            released.push_back(dkGuid);
    }

    for (ObjectGuid const& dk : released)
        state.walls.erase(dk);

    std::vector<Player*> dks;
    for (Member const& member : members)
    {
        Player* player = member.player;
        if (!state.walls.count(player->GetGUID()) && WallDk(state, player) &&
            player->GetSpellCooldownDelay(SPELL_DEATH_GRIP) <= GRIP_READY_SLACK_MS + WALL_WALK_MAX_MS)
            dks.push_back(player);
    }

    std::sort(dks.begin(), dks.end(),
              [](Player* a, Player* b)
              {
                  bool const aIce = !FactionChampionsIceDutyOf(a).IsEmpty();
                  if (aIce != !FactionChampionsIceDutyOf(b).IsEmpty())
                      return aIce;

                  return a->GetGUID() < b->GetGUID();
              });

    struct Candidate
    {
        Unit* champion;
        Member const* victim;
    };

    for (Player* dk : dks)
    {
        std::vector<Candidate> candidates;
        for (Unit* champion : champions)
        {
            bool const taken = std::any_of(state.walls.begin(), state.walls.end(), [champion](auto const& held)
                                           { return held.second.champion == champion->GetGUID(); });
            Member const* victim = VictimMember(members, champion);
            if (!taken && dk->GetExactDist2d(champion) <= GRIP_ELECT_REACH &&
                WallVictim(members, dk, champion, victim) &&
                WallGrippable(state, botAI, champion, killTarget, now))
                candidates.push_back({champion, victim});
        }

        std::sort(candidates.begin(), candidates.end(),
                  [](Candidate const& a, Candidate const& b)
                  {
                      if (a.victim->healer != b.victim->healer)
                          return a.victim->healer;

                      if (a.victim->player->GetHealthPct() != b.victim->player->GetHealthPct())
                          return a.victim->player->GetHealthPct() < b.victim->player->GetHealthPct();

                      return a.champion->GetGUID() < b.champion->GetGUID();
                  });

        for (Candidate const& candidate : candidates)
        {
            Position stand;
            if (!DeriveWallStand(state, dk, candidate.champion, candidate.victim->player, members, stand) ||
                !GripReadyInTime(dk, stand, true))
                continue;

            state.walls[dk->GetGUID()] = {candidate.champion->GetGUID(), stand, now, 0, false};
            break;
        }
    }
}

void UpdateSnares(FactionChampionsDefenceState& state, PlayerbotAI* botAI, std::vector<Member> const& members,
                  std::vector<Unit*> const& champions)
{
    state.snares.clear();

    struct Needy
    {
        Unit* champion;
        bool healerVictim;
        float victimHealth;
    };

    std::vector<Needy> needy;
    for (Unit* champion : champions)
    {
        if (!IsFactionChampionMelee(champion->GetEntry()) || champion->HasUnitState(UNIT_STATE_LOST_CONTROL) ||
            champion->HasAuraType(SPELL_AURA_MOD_DECREASE_SPEED) || SnareImmune(champion))
            continue;

        Unit* victim = champion->GetVictim();
        auto const member = victim ? FindMember(members, victim->GetGUID()) : members.end();
        // the kite only gains ground on a slowed chaser
        bool const kited = member != members.end() && FactionChampionsKitesFrom(member->player, champion);
        // one off the tank and out of the pack is on its way to the raid
        bool const loose = FactionChampionsIsEscapee(botAI, champion);
        if (!kited && !loose)
            continue;

        needy.push_back({champion, kited && member->healer, kited ? victim->GetHealthPct() : 100.0f});
    }

    if (needy.empty())
        return;

    std::sort(needy.begin(), needy.end(),
              [](Needy const& a, Needy const& b)
              {
                  if (a.healerVictim != b.healerVictim)
                      return a.healerVictim;

                  if (a.victimHealth != b.victimHealth)
                      return a.victimHealth < b.victimHealth;

                  return a.champion->GetGUID() < b.champion->GetGUID();
              });

    // The DK walking to its grip stand runs its grip node first, and a wall grip or ice duty DK keeps its frost
    // runes for its own champion
    std::vector<Player*> snarers;
    for (Member const& member : members)
        if (!member.healer && member.player->GetGUID() != state.grip.dk &&
            !state.walls.count(member.player->GetGUID()) && FactionChampionsIceDutyOf(member.player).IsEmpty() &&
            GET_PLAYERBOT_AI(member.player) && FreeToAct(member.player))
            snarers.push_back(member.player);

    std::vector<bool> taken(snarers.size(), false);
    for (Needy const& entry : needy)
    {
        Unit* champion = entry.champion;
        std::vector<std::pair<float, size_t>> byDistance;
        for (size_t i = 0; i < snarers.size(); ++i)
            if (!taken[i])
                byDistance.push_back({snarers[i]->GetExactDist2d(champion), i});

        std::sort(byDistance.begin(), byDistance.end());
        for (auto const& nearest : byDistance)
        {
            size_t const i = nearest.second;
            Player* snarer = snarers[i];
            uint32 const spellId = ReadySnare(snarer, champion);
            SpellInfo const* info = spellId ? sSpellMgr->GetSpellInfo(spellId) : nullptr;
            if (!info || champion->IsImmunedToSpell(info) || !snarer->IsWithinLOSInMap(champion))
                continue;

            state.snares[snarer->GetGUID()] = {champion->GetGUID(), spellId};
            taken[i] = true;
            break;
        }
    }
}

// Disarm wants Battle or Defensive Stance
bool StanceAllows(Player* player, uint32 spellId)
{
    SpellInfo const* info = sSpellMgr->GetSpellInfo(spellId);
    return info && info->CheckShapeshift(player->GetShapeshiftForm()) == SPELL_CAST_OK;
}

bool InSpellRange(Player* caster, Unit* target, uint32 spellId)
{
    SpellInfo const* info = spellId ? sSpellMgr->GetSpellInfo(spellId) : nullptr;
    return info && caster->IsWithinCombatRange(target, info->GetMaxRange(false, caster));
}

uint32 ReadyInRange(Player* caster, Unit* target, uint32 spellId)
{
    return SpellReady(caster, spellId) && InSpellRange(caster, target, spellId) ? spellId : 0;
}

// What lands on the champion: the Freezing Arrow trap's freeze, else the spell itself
uint32 PeelLanding(uint32 spellId) { return spellId == SPELL_FREEZING_ARROW ? SPELL_FREEZING_ARROW_EFFECT : spellId; }

// A hit ends it. Roots aside: they go on a champion that is still being hit, as before.
bool PeelBreaks(uint32 spellId)
{
    SpellInfo const* landing = spellId ? sSpellMgr->GetSpellInfo(PeelLanding(spellId)) : nullptr;
    return landing && !landing->HasAura(SPELL_AURA_MOD_ROOT) && FactionChampionsBreaksOnDamage(landing->Id);
}

// Nobody of ours swings at it, selects it or ticks a DoT on it
bool ClearOfOurHits(std::vector<Member> const& members, Unit* champion)
{
    for (Member const& member : members)
    {
        Player* player = member.player;
        if (player->GetVictim() == champion || player->GetTarget() == champion->GetGUID())
            return false;

        Pet* pet = player->GetPet();
        if (pet && pet->IsAlive() && pet->GetVictim() == champion)
            return false;
    }

    return !FactionChampionsCarriesGroupDot(champion);
}

// The peel this bot can put on champion from where it stands, 0 when none. Roots and Frost Nova stay off
// a champion near a pack tank. A DK has none: its grips go to the wall.
uint32 ReadyPeel(Player* peeler, Unit* champion, bool nearPackTank, bool danger)
{
    bool const inMelee = peeler->IsWithinMeleeRange(champion);
    bool const disarmed = champion->HasUnitFlag(UNIT_FLAG_DISARMED);
    switch (peeler->getClass())
    {
        case CLASS_PALADIN:
        {
            // the danger champion only, and not at the third stun step, which holds 1.5 s
            uint32 const rank = danger ? FirstKnownRank(peeler, HAMMER_OF_JUSTICE_RANKS) : 0;
            SpellInfo const* info = rank ? sSpellMgr->GetSpellInfo(rank) : nullptr;
            if (info && SpellReady(peeler, rank) && InSpellRange(peeler, champion, rank) &&
                champion->GetDiminishing(GetDiminishingReturnsGroupForSpell(info, false)) < DIMINISHING_LEVEL_3)
                return rank;

            return ReadyInRange(peeler, champion, SPELL_REPENTANCE);
        }
        case CLASS_WARRIOR:
            return inMelee && !disarmed && StanceAllows(peeler, SPELL_DISARM) && SpellReady(peeler, SPELL_DISARM)
                       ? SPELL_DISARM
                       : 0;
        case CLASS_ROGUE:
            if (inMelee && !disarmed && SpellReady(peeler, SPELL_DISMANTLE))
                return SPELL_DISMANTLE;

            return ReadyInRange(peeler, champion, SPELL_BLIND);
        case CLASS_HUNTER:
            return ReadyInRange(peeler, champion, SPELL_FREEZING_ARROW);
        case CLASS_WARLOCK:
            return ReadyInRange(peeler, champion, FirstKnownRank(peeler, DEATH_COIL_RANKS));
        case CLASS_MAGE:
        {
            uint32 const rank = FirstKnownRank(peeler, FROST_NOVA_RANKS);
            return !nearPackTank && peeler->GetExactDist2d(champion) <= FROST_NOVA_REACH && SpellReady(peeler, rank)
                       ? rank
                       : 0;
        }
        case CLASS_DRUID:
        {
            // a cat would leave its form for it
            if (nearPackTank || PlayerbotAI::IsHeal(peeler) || PlayerbotAI::IsMelee(peeler))
                return 0;

            uint32 const rank = FirstKnownRank(peeler, ENTANGLING_ROOTS_RANKS);
            return SpellReady(peeler, rank) && InSpellRange(peeler, champion, rank) ? rank : 0;
        }
        case CLASS_PRIEST:
            return SpellReady(peeler, SPELL_PSYCHIC_HORROR) && InSpellRange(peeler, champion, SPELL_PSYCHIC_HORROR)
                       ? SPELL_PSYCHIC_HORROR
                       : 0;
        default:
            return 0;
    }
}

uint32 GroupUnitsWithin(std::vector<Member> const& members, Unit* centre, float radius)
{
    uint32 count = 0;
    for (Member const& member : members)
    {
        if (member.player->GetExactDist2d(centre) <= radius)
            ++count;

        Pet* pet = member.player->GetPet();
        if (pet && pet->IsAlive() && pet->GetExactDist2d(centre) <= radius)
            ++count;
    }

    return count;
}

void UpdatePeel(FactionChampionsDefenceState& state, PlayerbotAI* botAI, std::vector<Member> const& members,
                std::vector<Unit*> const& champions)
{
    state.peels.clear();

    struct Needy
    {
        Unit* champion;
        bool danger;
        bool warrior;
        bool healerVictim;
        float victimHealth;
    };

    Unit* const danger = FactionChampionsDangerTarget(botAI);
    std::vector<Needy> needy;
    for (Unit* champion : champions)
    {
        // an iced one's DK has it, and a stun or fear there would pop the trinket and the Chains with it
        if (!IsFactionChampionMelee(champion->GetEntry()) || champion->HasUnitState(UNIT_STATE_LOST_CONTROL) ||
            champion->HasAuraType(SPELL_AURA_MOD_ROOT) || FactionChampionsIced(botAI, champion) ||
            std::find(state.guarded.begin(), state.guarded.end(), champion->GetGUID()) != state.guarded.end())
            continue;

        Unit* victim = champion->GetVictim();
        auto const member = victim ? FindMember(members, victim->GetGUID()) : members.end();
        bool const onRaid = member != members.end() && !IsTankMember(member->player) &&
                            champion->GetExactDist2d(victim) <= PEEL_VICTIM_REACH;
        bool const warrior = IsWarrior(champion->GetEntry());
        bool const crowded = warrior && GroupUnitsWithin(members, champion, BLADESTORM_RADIUS) >= PEEL_BLADESTORM_UNITS;
        // kept stunned or disarmed wherever it is
        bool const isDanger = champion == danger && !champion->HasUnitFlag(UNIT_FLAG_DISARMED);
        if (!onRaid && !crowded && !isDanger)
            continue;

        needy.push_back(
            {champion, isDanger, warrior, onRaid && member->healer, onRaid ? victim->GetHealthPct() : 100.0f});
    }

    std::sort(needy.begin(), needy.end(),
              [](Needy const& a, Needy const& b)
              {
                  if (a.danger != b.danger)
                      return a.danger;

                  if (a.warrior != b.warrior)
                      return a.warrior;

                  if (a.healerVictim != b.healerVictim)
                      return a.healerVictim;

                  if (a.victimHealth != b.victimHealth)
                      return a.victimHealth < b.victimHealth;

                  return a.champion->GetGUID() < b.champion->GetGUID();
              });

    Position anchor;
    Position centre;
    std::vector<Player*> peelers;
    if (!needy.empty() && FactionChampionsPack(botAI, anchor, centre))
        for (Member const& member : members)
            if (member.player->GetGUID() != state.grip.dk && GET_PLAYERBOT_AI(member.player) &&
                FreeToAct(member.player) && !FactionChampionsHasCcDuty(botAI, member.player->GetGUID()))
                peelers.push_back(member.player);

    std::vector<bool> taken(peelers.size(), false);
    for (Needy const& entry : needy)
    {
        Unit* champion = entry.champion;
        bool const nearPackTank = std::any_of(members.begin(), members.end(),
                                              [champion](Member const& member)
                                              {
                                                  return FactionChampionsIsPackTank(member.player) &&
                                                         member.player->GetExactDist2d(champion) < ROOT_AOE_CLEAR;
                                              });

        std::vector<std::pair<float, size_t>> byDistance;
        for (size_t i = 0; i < peelers.size(); ++i)
            if (!taken[i])
                byDistance.push_back({peelers[i]->GetExactDist2d(champion), i});

        std::sort(byDistance.begin(), byDistance.end());
        for (auto const& nearest : byDistance)
        {
            size_t const i = nearest.second;
            Player* peeler = peelers[i];
            uint32 const spellId = ReadyPeel(peeler, champion, nearPackTank, entry.danger);
            SpellInfo const* info = spellId ? sSpellMgr->GetSpellInfo(spellId) : nullptr;
            SpellInfo const* landing = spellId ? sSpellMgr->GetSpellInfo(PeelLanding(spellId)) : nullptr;
            if (!info || !landing || champion->IsImmunedToSpell(landing) || !peeler->IsWithinLOSInMap(champion))
                continue;

            if (PeelBreaks(spellId) && !ClearOfOurHits(members, champion))
                continue;

            DiminishingGroup const drGroup = GetDiminishingReturnsGroupForSpell(landing, false);
            if (drGroup != DIMINISHING_NONE && champion->GetDiminishing(drGroup) >= DIMINISHING_LEVEL_IMMUNE)
                continue;

            // Frost Nova would freeze a gripped healer flying past too
            if (!state.guarded.empty() && info->IsAffectingArea())
                continue;

            state.peels[peeler->GetGUID()] = {champion->GetGUID(), spellId};
            taken[i] = true;
            break;
        }
    }
}

// Same maths as the champion script's GetThreatMod: lowest current health first, ranged blind to distance
// up to 35 yd, melee to 5 yd and also weighing armor
float ChampionScore(Creature* champion, Player* player, bool melee)
{
    float const free = melee ? SCORE_MELEE_FREE : SCORE_RANGED_FREE;
    float const factor = melee ? SCORE_MELEE_FACTOR : SCORE_RANGED_FACTOR;
    float const dist = std::max(champion->GetDistance2d(player) - free, 0.0f);
    uint32 const health = player->GetHealth();
    float const healthMod = health > SCORE_HEALTH_CAP
                                ? SCORE_HEALTH_CAPPED
                                : (SCORE_HEALTH_BASE - static_cast<float>(health)) / SCORE_HEALTH_SCALE;
    float const armorMod =
        melee ? static_cast<float>(Unit::CalcArmorReducedDamage(champion, player, SCORE_ARMOR_SAMPLE, nullptr)) /
                    static_cast<float>(SCORE_ARMOR_SAMPLE)
              : 1.0f;
    return factor / (factor + dist) * healthMod * armorMod;
}

float const* FindThreat(ThreatSnapshot const& snapshot, ObjectGuid const& guid)
{
    auto const found = std::lower_bound(snapshot.begin(), snapshot.end(), guid,
                                        [](auto const& entry, ObjectGuid const& key) { return entry.first < key; });
    return found != snapshot.end() && found->first == guid ? &found->second : nullptr;
}

// A repeated guid keeps its last threat
void SetThreat(ThreatSnapshot& snapshot, ObjectGuid const& guid, float threat)
{
    auto const found = std::lower_bound(snapshot.begin(), snapshot.end(), guid,
                                        [](auto const& entry, ObjectGuid const& key) { return entry.first < key; });
    if (found != snapshot.end() && found->first == guid)
    {
        found->second = threat;
        return;
    }

    snapshot.insert(found, {guid, threat});
}

bool InLeadWindow(RepickClock const& clock, uint32 now)
{
    if (!clock.seen)
        return false;

    uint32 const since = getMSTimeDiff(clock.repickMs, now);
    return since + REPICK_LEAD_MS >= REPICK_MIN_MS && since <= REPICK_MAX_MS + REPICK_LATE_MS;
}

// The player the champion goes to at its next re-pick, or null when it's too close to call. Players only,
// with threat on it, as the script; an immune one is suppressed and never picked.
Player* PredictPick(Creature* champion, bool melee)
{
    Player* best = nullptr;
    float bestScore = 0.0f;
    float secondScore = 0.0f;
    float currentScore = 0.0f;
    Unit* const current = champion->GetVictim();
    for (ThreatReference const* ref : champion->GetThreatMgr().GetUnsortedThreatList())
    {
        Unit* victim = ref->GetVictim();
        Player* player = victim ? victim->ToPlayer() : nullptr;
        if (!player || !player->IsAlive() || ref->GetThreat() <= 0.0f || !ref->IsOnline() ||
            player->IsImmunedToDamage(SPELL_SCHOOL_MASK_NORMAL))
            continue;

        float const score = ChampionScore(champion, player, melee);
        if (player == current)
            currentScore = score;

        if (score > bestScore)
        {
            secondScore = bestScore;
            bestScore = score;
            best = player;
        }
        else if (score > secondScore)
            secondScore = score;
    }

    if (!best)
        return nullptr;

    if (currentScore > 0.0f && best != current)
    {
        float const needed = champion->IsWithinMeleeRange(best) ? SWITCH_MELEE : SWITCH_RANGED;
        if (bestScore <= currentScore * needed)
            return current->ToPlayer();
    }

    return bestScore >= secondScore * PICK_MARGIN ? best : nullptr;
}

// Spots each champion's re-pick off its threat list, and inside the second before the next one predicts
// whom it goes to
void UpdateFocusClock(FactionChampionsDefenceState& state, std::vector<Unit*> const& champions, uint32 now)
{
    state.predicted.clear();
    state.predictedPhysical.clear();
    for (Unit* unit : champions)
    {
        Creature* champion = unit->ToCreature();
        if (!champion)
            continue;

        RepickClock& clock = state.clocks[champion->GetGUID()];
        ThreatSnapshot& threat = state.threatScratch;
        threat.clear();
        uint32 moved = 0;
        for (ThreatReference const* ref : champion->GetThreatMgr().GetUnsortedThreatList())
        {
            Unit* victim = ref->GetVictim();
            if (!victim || !victim->IsPlayer())
                continue;

            float const value = ref->GetThreat();
            SetThreat(threat, victim->GetGUID(), value);
            float const* was = FindThreat(clock.threat, victim->GetGUID());
            if (was && std::fabs(value - *was) > std::max(*was * REPICK_SHIFT_PCT, REPICK_SHIFT_MIN))
                ++moved;
        }

        clock.threat.swap(threat);
        if (moved >= REPICK_SHIFT_ENTRIES)
        {
            clock.repickMs = now;
            clock.seen = true;
        }

        if (!InLeadWindow(clock, now) || champion->HasUnitState(UNIT_STATE_LOST_CONTROL))
            continue;

        if (Player* pick = PredictPick(champion, IsFactionChampionMelee(champion->GetEntry())))
        {
            ++state.predicted[pick->GetGUID()];
            if (IsPhysicalChampion(champion->GetEntry()))
                ++state.predictedPhysical[pick->GetGUID()];
        }
    }

    ObjectGuid next;
    uint32 most = 0;
    for (auto const& entry : state.predicted)
        if (entry.second > most || (entry.second == most && entry.first < next))
        {
            next = entry.first;
            most = entry.second;
        }

    state.nextFocus = most >= FOCUS_PREEMPT_CHAMPIONS ? next : ObjectGuid::Empty;
}

uint32 CountFor(std::unordered_map<ObjectGuid, uint32> const& counts, ObjectGuid guid)
{
    auto const found = counts.find(guid);
    return found != counts.end() ? found->second : 0;
}

bool HasAnyAura(Unit* unit, std::initializer_list<uint32> spells)
{
    return std::any_of(spells.begin(), spells.end(), [unit](uint32 spellId) { return unit->HasAura(spellId); });
}

// Already shielded from the hits, by one of the four or by its own immunity
bool ProtectCovered(Player* member)
{
    return member->IsImmunedToDamage(SPELL_SCHOOL_MASK_NORMAL) ||
           HasAnyAura(member, {HAND_OF_PROTECTION_RANKS[0], HAND_OF_PROTECTION_RANKS[1], HAND_OF_PROTECTION_RANKS[2],
                               SPELL_PAIN_SUPPRESSION, SPELL_GUARDIAN_SPIRIT, SPELL_HAND_OF_SACRIFICE});
}

// A melee bot can't swing under it for 10 s, so it only goes on one already hurt, still cheaper than
// dying to a six champion focus
bool HandOfProtectionFits(Member const& member)
{
    Player* player = member.player;
    if (IsTankMember(player) || player->HasAura(SPELL_FORBEARANCE))
        return false;

    return member.healer || PlayerbotAI::IsRanged(player) || player->GetHealthPct() <= MELEE_HAND_HEALTH_PCT;
}

// The spell id of the external this caster can put on member now, in the order they're worth spending.
// others: Hand of Sacrifice, Pain Suppression and Guardian Spirit may go too.
uint32 ReadyExternal(Player* caster, Player* member, bool handOfProtection, bool others)
{
    bool const self = caster == member;
    float const distance = self ? 0.0f : caster->GetExactDist(member);
    switch (caster->getClass())
    {
        case CLASS_PALADIN:
        {
            uint32 const rank = FirstKnownRank(caster, HAND_OF_PROTECTION_RANKS);
            if (handOfProtection && distance <= HAND_RANGE && SpellReady(caster, rank))
                return rank;

            bool const canSacrifice = others && !self && distance <= HAND_RANGE &&
                                      caster->GetHealthPct() >= SACRIFICE_MIN_HEALTH_PCT;
            return canSacrifice && SpellReady(caster, SPELL_HAND_OF_SACRIFICE) ? SPELL_HAND_OF_SACRIFICE : 0;
        }
        case CLASS_PRIEST:
            if (!others || distance > PRIEST_EXTERNAL_RANGE)
                return 0;

            if (SpellReady(caster, SPELL_PAIN_SUPPRESSION))
                return SPELL_PAIN_SUPPRESSION;

            return SpellReady(caster, SPELL_GUARDIAN_SPIRIT) ? SPELL_GUARDIAN_SPIRIT : 0;
        default:
            return 0;
    }
}

// Hand of Protection first: immune to physical, its target is dropped by every champion, casters too, and
// they all move on at once
uint32 ExternalRank(uint32 spellId)
{
    if (std::find(HAND_OF_PROTECTION_RANKS.begin(), HAND_OF_PROTECTION_RANKS.end(), spellId) !=
        HAND_OF_PROTECTION_RANKS.end())
        return 0;

    switch (spellId)
    {
        case SPELL_PAIN_SUPPRESSION:
            return 1;
        case SPELL_GUARDIAN_SPIRIT:
            return 2;
        default:
            return 3;
    }
}

// A non-tank with four or more on it that no external went to gets Divine Sacrifice from a paladin with
// Divine Guardian in reach. One at a time: a second one while it runs adds nothing.
void DealDivineSacrifice(FactionChampionsDefenceState& state, std::vector<Member> const& members,
                         std::vector<Member> const& focused, std::unordered_map<ObjectGuid, uint32> const& attackers,
                         std::vector<Member> const& casters, std::vector<bool>& taken)
{
    SpellInfo const* guardian = sSpellMgr->GetSpellInfo(SPELL_DIVINE_GUARDIAN);
    float const reach = guardian ? guardian->Effects[EFFECT_0].CalcRadius() : 0.0f;
    if (reach <= 0.0f || std::any_of(members.begin(), members.end(), [](Member const& member)
                                     { return member.player->HasAura(SPELL_DIVINE_GUARDIAN); }))
        return;

    for (Member const& member : focused)
    {
        auto const hits = attackers.find(member.player->GetGUID());
        if (hits == attackers.end() || hits->second < FOCUS_PREEMPT_CHAMPIONS || IsTankMember(member.player) ||
            std::any_of(state.protects.begin(), state.protects.end(),
                        [&member](auto const& duty) { return duty.second.target == member.player->GetGUID(); }))
            continue;

        for (size_t i = 0; i < casters.size(); ++i)
        {
            Player* caster = casters[i].player;
            if (taken[i] || caster->getClass() != CLASS_PALADIN || !RaidHasDivineGuardian(caster) ||
                caster->GetHealthPct() < SACRIFICE_MIN_HEALTH_PCT || !SpellReady(caster, SPELL_DIVINE_SACRIFICE) ||
                caster->GetExactDist(member.player) > reach)
                continue;

            state.protects[caster->GetGUID()] = {member.player->GetGUID(), SPELL_DIVINE_SACRIFICE};
            taken[i] = true;
            return;
        }
    }
}

void UpdateProtect(FactionChampionsDefenceState& state, std::vector<Member> const& members,
                   std::vector<Unit*> const& champions)
{
    state.protects.clear();

    // casters count too: four of them kill a healer from full in a second or two
    std::unordered_map<ObjectGuid, uint32> attackers;
    for (Unit* champion : champions)
    {
        if (champion->HasUnitState(UNIT_STATE_LOST_CONTROL))
            continue;

        Unit* victim = champion->GetVictim();
        if (!victim || !victim->IsPlayer() ||
            (IsFactionChampionMelee(champion->GetEntry()) && champion->GetExactDist2d(victim) > PROTECT_MELEE_REACH))
            continue;

        ++attackers[victim->GetGUID()];
    }

    // cover goes out on the champions' next pick, not a second or two after their hits land
    for (auto const& entry : state.predicted)
    {
        uint32& count = attackers[entry.first];
        count = std::max(count, entry.second);
    }

    std::vector<Member> focused;
    std::unordered_map<ObjectGuid, bool> othersFor;
    std::unordered_map<ObjectGuid, bool> handFor;
    for (Member const& member : members)
    {
        auto const hits = attackers.find(member.player->GetGUID());
        if (hits == attackers.end() || ProtectCovered(member.player))
            continue;

        float const health = member.player->GetHealthPct();
        bool const others = (hits->second >= FOCUS_PREEMPT_CHAMPIONS && !IsTankMember(member.player)) ||
                            (hits->second >= PROTECT_MIN_ATTACKERS && health <= PROTECT_HEALTH_PCT);
        // every champion drops the immune member, casters too, so it's kept for a real focus: one each per
        // fight
        bool const hand = hits->second >= FOCUS_PREEMPT_CHAMPIONS && HandOfProtectionFits(member);
        if (others || hand)
        {
            focused.push_back(member);
            othersFor[member.player->GetGUID()] = others;
            handFor[member.player->GetGUID()] = hand;
        }
    }

    std::sort(focused.begin(), focused.end(),
              [](Member const& a, Member const& b)
              {
                  if (a.healer != b.healer)
                      return a.healer;

                  if (a.player->GetHealthPct() != b.player->GetHealthPct())
                      return a.player->GetHealthPct() < b.player->GetHealthPct();

                  return a.player->GetGUID() < b.player->GetGUID();
              });

    std::vector<Member> casters;
    for (Member const& member : members)
    {
        uint8 const playerClass = member.player->getClass();
        if ((playerClass == CLASS_PALADIN || playerClass == CLASS_PRIEST) && GET_PLAYERBOT_AI(member.player) &&
            FreeToAct(member.player))
            casters.push_back(member);
    }

    // Healers heal, so the others go first
    SortByRole(casters, false);
    std::vector<bool> taken(casters.size(), false);
    auto const casterIndex = [&casters](ObjectGuid guid)
    {
        for (size_t i = 0; i < casters.size(); ++i)
            if (casters[i].player->GetGUID() == guid)
                return i;

        return casters.size();
    };

    // Held until the cast goes out: the gap below would drop a fresh deal on the next refresh
    uint32 const now = getMSTime();
    for (auto it = state.layOnHandsDuties.begin(); it != state.layOnHandsDuties.end();)
    {
        auto const caster = FindMember(members, it->first);
        auto const member = FindMember(members, it->second.member);
        if (caster == members.end() || member == members.end() ||
            caster->player->HasSpellCooldown(it->second.spellId) ||
            getMSTimeDiff(it->second.sinceMs, now) >= LAY_ON_HANDS_HOLD_MS)
        {
            it = state.layOnHandsDuties.erase(it);
            continue;
        }

        state.protects[it->first] = {it->second.member, it->second.spellId};
        size_t const i = casterIndex(it->first);
        if (i < casters.size())
            taken[i] = true;

        ++it;
    }

    for (Member const& member : focused)
    {
        bool const handOfProtection = handFor[member.player->GetGUID()];
        bool const others = othersFor[member.player->GetGUID()];
        size_t chosen = casters.size();
        uint32 chosenSpell = 0;
        for (size_t i = 0; i < casters.size(); ++i)
        {
            if (taken[i])
                continue;

            Player* caster = casters[i].player;
            uint32 const spellId = ReadyExternal(caster, member.player, handOfProtection, others);
            if (!spellId || (chosenSpell && ExternalRank(spellId) >= ExternalRank(chosenSpell)))
                continue;

            if (caster != member.player && !caster->IsWithinLOSInMap(member.player))
                continue;

            chosen = i;
            chosenSpell = spellId;
        }

        if (chosen == casters.size())
            continue;

        state.protects[casters[chosen].player->GetGUID()] = {member.player->GetGUID(), chosenSpell};
        taken[chosen] = true;
    }

    DealDivineSacrifice(state, members, focused, attackers, casters, taken);

    // Lay on Hands once per member in LAY_ON_HANDS_GAP_MS: each paladin's own node would land all of
    // theirs on one bot at once
    std::vector<Member> dying;
    for (Member const& member : members)
    {
        Player* player = member.player;
        auto const last = state.layOnHands.find(player->GetGUID());
        if (player->GetHealthPct() > LAY_ON_HANDS_PCT || ProtectCovered(player) ||
            (last != state.layOnHands.end() && getMSTimeDiff(last->second, now) < LAY_ON_HANDS_GAP_MS) ||
            std::none_of(champions.begin(), champions.end(),
                         [player](Unit* champion) { return champion->GetVictim() == player; }))
            continue;

        dying.push_back(member);
    }

    std::sort(dying.begin(), dying.end(),
              [](Member const& a, Member const& b)
              {
                  if (a.healer != b.healer)
                      return a.healer;

                  return a.player->GetHealthPct() < b.player->GetHealthPct();
              });

    for (Member const& member : dying)
    {
        for (size_t i = 0; i < casters.size(); ++i)
        {
            Player* caster = casters[i].player;
            if (taken[i] || caster == member.player || caster->getClass() != CLASS_PALADIN)
                continue;

            uint32 const rank = FirstKnownRank(caster, LAY_ON_HANDS_RANKS);
            if (!SpellReady(caster, rank) || caster->GetExactDist(member.player) > LAY_ON_HANDS_RANGE ||
                !caster->IsWithinLOSInMap(member.player))
                continue;

            state.protects[caster->GetGUID()] = {member.player->GetGUID(), rank};
            state.layOnHandsDuties[caster->GetGUID()] = {member.player->GetGUID(), rank, now};
            state.layOnHands[member.player->GetGUID()] = now;
            taken[i] = true;
            break;
        }
    }
}

// What Hand of Freedom strips on landing, so not Dazed, put on by a champion: the DK champion's Chains of Ice
// roots for its whole 10 s
bool ChampionRootOrSnare(Player* member)
{
    uint64 const freed = (1ULL << MECHANIC_ROOT) | (1ULL << MECHANIC_SNARE);
    for (auto const& applied : member->GetAppliedAuras())
    {
        Aura const* aura = applied.second->GetBase();
        ObjectGuid const caster = aura->GetCasterGUID();
        if (caster.IsCreature() && IsFactionChampion(caster.GetEntry()) &&
            (aura->GetSpellInfo()->GetAllEffectsMechanicMask() & freed))
            return true;
    }

    return false;
}

// After UpdateProtect, whose paladins are spoken for
void UpdateFreedom(FactionChampionsDefenceState& state, std::vector<Member> const& members,
                   std::vector<Unit*> const& champions)
{
    state.freedoms.clear();

    std::vector<Member> held;
    for (Member const& member : members)
    {
        Player* player = member.player;
        if (player->HasAura(SPELL_HAND_OF_FREEDOM_PLAYER) || !ChampionRootOrSnare(player))
            continue;

        if (std::any_of(champions.begin(), champions.end(),
                        [player](Unit* champion)
                        {
                            return FactionChampionsKitesFrom(player, champion) &&
                                   player->GetExactDist2d(champion) <= FREEDOM_CHASER_REACH;
                        }))
            held.push_back(member);
    }

    if (held.empty())
        return;

    std::sort(held.begin(), held.end(),
              [](Member const& a, Member const& b)
              {
                  if (a.healer != b.healer)
                      return a.healer;

                  if (a.player->GetHealthPct() != b.player->GetHealthPct())
                      return a.player->GetHealthPct() < b.player->GetHealthPct();

                  return a.player->GetGUID() < b.player->GetGUID();
              });

    std::vector<Player*> paladins;
    for (Member const& member : members)
    {
        Player* player = member.player;
        if (player->getClass() == CLASS_PALADIN && GET_PLAYERBOT_AI(player) && FreeToAct(player) &&
            !state.protects.count(player->GetGUID()) && SpellReady(player, SPELL_HAND_OF_FREEDOM_PLAYER))
            paladins.push_back(player);
    }

    std::vector<bool> taken(paladins.size(), false);
    for (Member const& member : held)
    {
        size_t chosen = paladins.size();
        for (size_t i = 0; i < paladins.size(); ++i)
        {
            Player* paladin = paladins[i];
            if (taken[i] || !paladin->IsWithinDistInMap(member.player, HAND_RANGE) ||
                !paladin->IsWithinLOSInMap(member.player))
                continue;

            if (chosen == paladins.size() ||
                paladin->GetExactDist2d(member.player) < paladins[chosen]->GetExactDist2d(member.player))
                chosen = i;
        }

        if (chosen == paladins.size())
            continue;

        state.freedoms[paladins[chosen]->GetGUID()] = {member.player->GetGUID(), SPELL_HAND_OF_FREEDOM_PLAYER};
        taken[chosen] = true;
    }
}

void UpdateHealthLog(FactionChampionsDefenceState& state, std::vector<Member> const& members, uint32 now)
{
    if (getMSTimeDiff(state.healthSampleMs, now) < HEALTH_SAMPLE_MS)
        return;

    state.healthSampleMs = now;
    std::unordered_map<ObjectGuid, std::vector<HealthSample>> kept;
    for (Member const& member : members)
    {
        std::vector<HealthSample>& log = kept[member.player->GetGUID()];
        auto const held = state.healthLog.find(member.player->GetGUID());
        if (held != state.healthLog.end())
            for (HealthSample const& sample : held->second)
                if (getMSTimeDiff(sample.ms, now) <= HEALTH_LOG_MS)
                    log.push_back(sample);

        log.push_back({now, member.player->GetHealth()});
    }

    state.healthLog = std::move(kept);
}

std::vector<Player*> CorpsesOnMap(Player* bot)
{
    std::vector<Player*> corpses;
    Group* group = bot->GetGroup();
    Map* map = bot->FindMap();
    if (!group || !map)
        return corpses;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* player = ref->GetSource();
        if (!player || !player->IsInWorld() || player->FindMap() != map || player->IsAlive() ||
            player->getDeathState() != DeathState::Corpse || player->isResurrectRequested())
            continue;

        // A Soulstone or Reincarnation still pending brings it back without a Rebirth
        if (!player->HasPlayerFlag(PLAYER_FLAGS_GHOST) && player->GetUInt32Value(PLAYER_SELF_RES_SPELL))
            continue;

        corpses.push_back(player);
    }

    return corpses;
}

bool BeingResurrected(std::vector<Member> const& members, ObjectGuid corpse)
{
    return std::any_of(members.begin(), members.end(),
                       [corpse](Member const& member)
                       {
                           Spell const* spell = member.player->GetCurrentSpell(CURRENT_GENERIC_SPELL);
                           if (!spell || spell->m_targets.GetUnitTargetGUID() != corpse)
                               return false;

                           SpellInfo const* info = spell->GetSpellInfo();
                           return info->HasEffect(SPELL_EFFECT_RESURRECT) ||
                                  info->HasEffect(SPELL_EFFECT_RESURRECT_NEW);
                       });
}

bool PackCrowded(std::vector<Unit*> const& champions, Unit* spot)
{
    uint32 const nearby = static_cast<uint32>(
        std::count_if(champions.begin(), champions.end(), [spot](Unit* champion)
                      { return champion->GetExactDist2d(spot) <= CORPSE_PACK_RADIUS; }));
    return nearby > CORPSE_PACK_MAX;
}

void UpdateBattleRes(FactionChampionsDefenceState& state, Player* bot, std::vector<Member> const& members,
                     std::vector<Unit*> const& champions)
{
    state.battleRes.clear();

    // A dps raised mid-focus dies again inside 30 s, so Rebirth is kept for healers and tanks
    std::vector<Player*> corpses;
    for (Player* corpse : CorpsesOnMap(bot))
    {
        if (!PlayerbotAI::IsHeal(corpse) && !IsTankMember(corpse))
            continue;

        if (!BeingResurrected(members, corpse->GetGUID()))
            corpses.push_back(corpse);
    }

    if (corpses.empty())
        return;

    std::sort(corpses.begin(), corpses.end(),
              [](Player* a, Player* b)
              {
                  if (PlayerbotAI::IsHeal(a) != PlayerbotAI::IsHeal(b))
                      return PlayerbotAI::IsHeal(a);

                  return a->GetGUID() < b->GetGUID();
              });

    std::vector<std::pair<Player*, uint32>> druids;
    for (Member const& member : members)
    {
        Player* player = member.player;
        // the raised bot comes back where the druid stands, not at its corpse
        if (player->getClass() != CLASS_DRUID || !GET_PLAYERBOT_AI(player) || IsTankMember(player) ||
            !FreeToAct(player) || PackCrowded(champions, player))
            continue;

        uint32 const rank = FirstKnownRank(player, REBIRTH_RANKS);
        if (SpellReady(player, rank))
            druids.push_back({player, rank});
    }

    std::vector<bool> taken(druids.size(), false);
    for (Player* corpse : corpses)
    {
        size_t chosen = druids.size();
        float chosenDistance = 0.0f;
        for (size_t i = 0; i < druids.size(); ++i)
        {
            Player* druid = druids[i].first;
            float const distance = druid->GetExactDist(corpse);
            if (taken[i] || !druid->IsWithinDistInMap(corpse, REBIRTH_RANGE) ||
                (chosen != druids.size() && distance >= chosenDistance))
                continue;

            chosen = i;
            chosenDistance = distance;
        }

        if (chosen == druids.size() || !druids[chosen].first->IsWithinLOSInMap(corpse))
            continue;

        state.battleRes[druids[chosen].first->GetGUID()] = {corpse->GetGUID(), druids[chosen].second};
        taken[chosen] = true;
    }
}

// After UpdateGrip, which reads the duty this tick
void UpdateGuard(FactionChampionsDefenceState& state, Player* bot, std::vector<Unit*> const& champions, uint32 now)
{
    state.guarded.clear();
    GripChainState* chain = nullptr;
    if (Creature* chained = OpenChain(bot, chain))
        state.guarded.push_back(chained->GetGUID());

    if (chain && !chain->lastTarget.IsEmpty() && getMSTimeDiff(chain->lastLinkMs, now) < GRIP_GUARD_TAIL_MS)
        state.guarded.push_back(chain->lastTarget);

    if (!state.grip.dk.IsEmpty())
        state.guarded.push_back(state.grip.target);

    for (Unit* champion : champions)
        if (MidJump(champion))
            state.guarded.push_back(champion->GetGUID());
}

bool HoldsDirectly(SpellInfo const* info)
{
    return info->HasAura(SPELL_AURA_MOD_ROOT) || info->HasAura(SPELL_AURA_MOD_STUN) ||
           info->HasAura(SPELL_AURA_MOD_FEAR) || info->HasAura(SPELL_AURA_MOD_CONFUSE) ||
           info->HasEffect(SPELL_EFFECT_CHARGE) || info->HasEffect(SPELL_EFFECT_KNOCK_BACK);
}

bool TauntsOrThreatens(SpellInfo const* info)
{
    return info->HasAura(SPELL_AURA_MOD_TAUNT) || info->HasEffect(SPELL_EFFECT_ATTACK_ME) ||
           info->HasEffect(SPELL_EFFECT_THREAT);
}

bool IsAutocast(Pet* pet, uint32 spellId)
{
    for (uint8 pos = 0; pos < pet->GetPetAutoSpellSize(); ++pos)
        if (pet->GetPetAutoSpellOnPos(pos) == spellId)
            return true;

    return false;
}

// Null unless live, so no duty outlives the fight
FactionChampionsDefenceState* Refresh(PlayerbotAI* botAI)
{
    if (!ToCEncounterIsLive(botAI, ToCEncounter::FactionChampions))
        return nullptr;

    Player* bot = botAI->GetBot();
    if (!bot->GetInstanceId())
        return nullptr;

    FactionChampionsDefenceState& state = defenceStates.For(bot->GetInstanceId());
    uint32 const now = getMSTime();
    if (state.memoValid && state.memoMs == now)
        return &state;

    state.memoMs = now;
    state.memoValid = true;

    Unit* killTarget = FactionChampionsKillTarget(botAI);
    std::vector<Member> const members = MembersOnMap(bot);
    std::vector<Unit*> const champions = FactionChampionsInCombat(botAI);
    UpdatePhysicalLatch(state, botAI, killTarget);
    UpdateHazards(state, bot, members);
    UpdateTotemKillers(state, bot, members);
    UpdateCcDispels(state, members);
    UpdateMassDispel(state, botAI, members, killTarget);
    UpdatePurge(state, members, champions, killTarget);
    UpdateGrip(state, botAI, members, champions, killTarget, now);
    UpdateGuard(state, bot, champions, now);
    UpdateWalls(state, botAI, members, champions, killTarget, now);
    UpdateSnares(state, botAI, members, champions);
    UpdatePeel(state, botAI, members, champions);
    UpdateFocusClock(state, champions, now);
    UpdateProtect(state, members, champions);
    UpdateFreedom(state, members, champions);
    UpdateBattleRes(state, bot, members, champions);
    UpdateHealthLog(state, members, now);
    return &state;
}

bool IgnoredBy(HazardSource const& source, Player* bot) { return source.kicker == bot->GetGUID(); }

// A Bladestorm whose warrior chases the bot: a dodge stops at the nearest clear spot and he walks back in, the
// kite keeps running from him, clear of his clearance
bool LeftToKite(HazardSource const& source, Player* bot)
{
    if (source.spellId != SPELL_BLADESTORM || bot->HasUnitState(UNIT_STATE_LOST_CONTROL) ||
        bot->HasAuraType(SPELL_AURA_MOD_ROOT))
        return false;

    Unit* warrior = ObjectAccessor::GetUnit(*bot, source.guid);
    return warrior && FactionChampionsKitesFrom(bot, warrior);
}

bool DodgeSkips(HazardSource const& source, Player* bot) { return IgnoredBy(source, bot) || LeftToKite(source, bot); }

Creature* AliveCreature(Player* bot, ObjectGuid guid)
{
    Map* map = bot->FindMap();
    if (!map || guid.IsEmpty())
        return nullptr;

    Creature* creature = map->GetCreature(guid);
    return creature && creature->IsAlive() ? creature : nullptr;
}

bool IsPhysicalAttacker(Player* bot)
{
    return !PlayerbotAI::IsHeal(bot) && (PlayerbotAI::IsMelee(bot) || bot->getClass() == CLASS_HUNTER);
}

Unit* DutyTarget(std::unordered_map<ObjectGuid, SpellDuty> const& duties, Player* bot, uint32& spellId)
{
    auto const duty = duties.find(bot->GetGUID());
    if (duty == duties.end())
        return nullptr;

    spellId = duty->second.spellId;
    return ObjectAccessor::GetUnit(*bot, duty->second.target);
}

// FindNearestPositionClearOfHazards answers Position() when nothing is clear
bool IsNoSpot(Position const& spot) { return !spot.GetPositionX() && !spot.GetPositionY(); }
}

Unit* FactionChampionsDispelCcTarget(PlayerbotAI* botAI, char const*& spell)
{
    FactionChampionsDefenceState* state = Refresh(botAI);
    if (!state)
        return nullptr;

    Player* bot = botAI->GetBot();
    auto const duty = state->ccDispels.find(bot->GetGUID());
    if (duty == state->ccDispels.end())
        return nullptr;

    Player* member = ObjectAccessor::GetPlayer(*bot, duty->second.member);
    if (!member || !member->IsAlive())
        return nullptr;

    spell = duty->second.spell;
    return member;
}

Unit* FactionChampionsMassDispelTarget(PlayerbotAI* botAI)
{
    if (!botAI || botAI->GetBot()->getClass() != CLASS_PRIEST)
        return nullptr;

    FactionChampionsDefenceState* state = Refresh(botAI);
    if (!state || state->massDispeller != botAI->GetBot()->GetGUID())
        return nullptr;

    return AliveCreature(botAI->GetBot(), state->massDispelTarget);
}

Unit* FactionChampionsPurgeTarget(PlayerbotAI* botAI, char const*& spell)
{
    FactionChampionsDefenceState* state = Refresh(botAI);
    if (!state)
        return nullptr;

    ObjectGuid const bot = botAI->GetBot()->GetGUID();
    bool const kill = state->purger == bot;
    if (!kill && state->packPurger != bot)
        return nullptr;

    Creature* target = AliveCreature(botAI->GetBot(), kill ? state->purgeTarget : state->packPurgeTarget);
    if (!target)
        return nullptr;

    spell = kill ? state->purgeSpell : state->packPurgeSpell;
    return target;
}

Unit* FactionChampionsPhysicalSwitchTarget(PlayerbotAI* botAI)
{
    // an ice duty DK stays on its own champion
    if (!botAI || !IsPhysicalAttacker(botAI->GetBot()) || !FactionChampionsIceDutyOf(botAI->GetBot()).IsEmpty())
        return nullptr;

    FactionChampionsDefenceState* state = Refresh(botAI);
    return state ? AliveCreature(botAI->GetBot(), state->physical) : nullptr;
}

Unit* FactionChampionsTotemTarget(PlayerbotAI* botAI)
{
    FactionChampionsDefenceState* state = Refresh(botAI);
    if (!state)
        return nullptr;

    auto const hold = state->totemKillers.find(botAI->GetBot()->GetGUID());
    return hold != state->totemKillers.end() ? AliveCreature(botAI->GetBot(), hold->second) : nullptr;
}

bool FactionChampionsInAoe(PlayerbotAI* botAI)
{
    FactionChampionsDefenceState* state = Refresh(botAI);
    if (!state)
        return false;

    Player* bot = botAI->GetBot();
    return std::any_of(state->hazards.begin(), state->hazards.end(),
                       [bot](HazardSource const& source)
                       {
                           return !DodgeSkips(source, bot) &&
                                  bot->GetExactDist2d(source.pos) <= source.radius + IN_AOE_MARGIN;
                       });
}

std::vector<EncounterHelpers::HazardCircle> FactionChampionsAoeClearances(PlayerbotAI* botAI)
{
    std::vector<EncounterHelpers::HazardCircle> clearances;
    FactionChampionsDefenceState* state = Refresh(botAI);
    if (!state)
        return clearances;

    for (HazardSource const& source : state->hazards)
        if (!IgnoredBy(source, botAI->GetBot()))
            clearances.emplace_back(source.pos, source.clearance);

    return clearances;
}

bool FactionChampionsReachIntoAoe(PlayerbotAI* botAI)
{
    if (!botAI || !PlayerbotAI::IsMelee(botAI->GetBot()))
        return false;

    FactionChampionsDefenceState* state = Refresh(botAI);
    Unit* target = state ? CurrentTarget(botAI) : nullptr;
    if (!target)
        return false;

    // Sized to where the bot would stand: up to its melee range on the near side of the target
    Player* bot = botAI->GetBot();
    float const standOff = bot->GetMeleeRange(target);
    return std::any_of(state->hazards.begin(), state->hazards.end(),
                       [bot, target, standOff](HazardSource const& source)
                       {
                           return !IgnoredBy(source, bot) &&
                                  (target->GetGUID() == source.guid ||
                                   target->GetExactDist2d(source.pos) <= source.radius + IN_AOE_MARGIN + standOff);
                       });
}

bool FactionChampionsDispelBackfires(PlayerbotAI* botAI, Unit* target)
{
    return botAI && ToCEncounterIsLive(botAI, ToCEncounter::FactionChampions) && CarriesUnstableAffliction(target);
}

Unit* FactionChampionsChannelInterruptTarget(PlayerbotAI* botAI, char const*& spell)
{
    FactionChampionsDefenceState* state = Refresh(botAI);
    if (!state)
        return nullptr;

    Player* bot = botAI->GetBot();
    for (auto const& kick : state->kicks)
    {
        if (kick.second.bot != bot->GetGUID())
            continue;

        spell = kick.second.spell;
        return AliveCreature(bot, kick.first);
    }

    return nullptr;
}

bool FactionChampionsSpellHoldsTarget(uint32 spellId)
{
    SpellInfo const* info = spellId ? sSpellMgr->GetSpellInfo(spellId) : nullptr;
    if (!info)
        return false;

    if (HoldsDirectly(info))
        return true;

    // Intimidation's stun and a pet Charge's root sit one trigger down
    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
        if (SpellInfo const* triggered = sSpellMgr->GetSpellInfo(info->Effects[i].TriggerSpell))
            if (HoldsDirectly(triggered))
                return true;

    for (int32 key : {static_cast<int32>(spellId), static_cast<int32>(spellId) + SPELL_LINK_HIT})
        if (std::vector<int32> const* linked = sSpellMgr->GetSpellLinked(key))
            for (int32 next : *linked)
                if (SpellInfo const* triggered = next > 0 ? sSpellMgr->GetSpellInfo(next) : nullptr)
                    if (HoldsDirectly(triggered))
                        return true;

    return false;
}

bool FactionChampionsChainGuarded(Player* bot, Unit* champion)
{
    if (!bot || !champion)
        return false;

    if (MidJump(champion))
        return true;

    GripChainState* chain = nullptr;
    Creature* chained = OpenChain(bot, chain);
    if (chained && chained->GetGUID() == champion->GetGUID())
        return true;

    return chain && chain->lastTarget == champion->GetGUID() &&
           getMSTimeDiff(chain->lastLinkMs, getMSTime()) < GRIP_GUARD_TAIL_MS;
}

bool FactionChampionsGripGuarded(PlayerbotAI* botAI, Unit* champion)
{
    FactionChampionsDefenceState* state = champion ? Refresh(botAI) : nullptr;
    return state && std::find(state->guarded.begin(), state->guarded.end(), champion->GetGUID()) != state->guarded.end();
}

bool FactionChampionsGripGuardActive(PlayerbotAI* botAI)
{
    FactionChampionsDefenceState* state = Refresh(botAI);
    return state && !state->guarded.empty();
}

// Safe to call twice a tick, the trigger and the action both ask
uint32 FactionChampionsPetAutocastToggle(PlayerbotAI* botAI, bool& enable)
{
    Player* bot = botAI ? botAI->GetBot() : nullptr;
    Pet* pet = bot ? bot->GetPet() : nullptr;
    FactionChampionsDefenceState* state = pet && pet->IsAlive() ? Refresh(botAI) : nullptr;
    if (!state)
        return 0;

    std::vector<uint32>& held = state->petHolds[bot->GetGUID()];
    held.erase(std::remove_if(held.begin(), held.end(), [pet](uint32 spellId)
                              { return !pet->HasSpell(spellId) || IsAutocast(pet, spellId); }),
               held.end());

    // champions are immune to taunt on heroic, and a pet's threat never reaches the 10M the script seeds
    Map* map = bot->FindMap();
    bool const heroic = map && map->IsHeroic();
    bool const guard = !state->guarded.empty();
    for (uint8 pos = 0; pos < pet->GetPetAutoSpellSize(); ++pos)
    {
        uint32 const spellId = pet->GetPetAutoSpellOnPos(pos);
        SpellInfo const* info = sSpellMgr->GetSpellInfo(spellId);
        if (!info)
            continue;

        if (heroic && TauntsOrThreatens(info))
        {
            enable = false;
            return spellId;
        }

        if (guard && FactionChampionsSpellHoldsTarget(spellId))
        {
            if (std::find(held.begin(), held.end(), spellId) == held.end())
                held.push_back(spellId);

            enable = false;
            return spellId;
        }
    }

    if (guard || held.empty())
        return 0;

    enable = true;
    return held.front();
}

bool FactionChampionsGripPending(PlayerbotAI* botAI)
{
    FactionChampionsDefenceState* state = Refresh(botAI);
    Unit* killTarget = state ? FactionChampionsKillTarget(botAI) : nullptr;
    return killTarget && !state->grip.dk.IsEmpty() && state->grip.target == killTarget->GetGUID() &&
           !FactionChampionsIsPulled(botAI, killTarget);
}

Unit* FactionChampionsGripTarget(PlayerbotAI* botAI, Position& stand)
{
    if (!botAI || botAI->GetBot()->getClass() != CLASS_DEATH_KNIGHT)
        return nullptr;

    FactionChampionsDefenceState* state = Refresh(botAI);
    if (!state || state->grip.dk != botAI->GetBot()->GetGUID())
        return nullptr;

    Creature* target = AliveCreature(botAI->GetBot(), state->grip.target);
    if (target)
        stand = state->grip.stand;

    return target;
}

void FactionChampionsTakePullRequest(PlayerbotAI* botAI)
{
    Player* bot = botAI ? botAI->GetBot() : nullptr;
    PullStrategy* strategy = bot ? PullStrategy::Get(botAI) : nullptr;
    if (!strategy || (!strategy->IsPullPendingToStart() && !strategy->HasPullStarted()))
        return;

    Unit* target = strategy->GetTarget();
    GripChainState* chain = nullptr;
    OpenChain(bot, chain);
    if (!chain || !target || !target->IsCreature() || !IsFactionChampion(target->GetEntry()))
        return;

    // every tank hears the one pull, and a bot DK's request may have opened it already
    if (chain->target != target->GetGUID())
        StartChain(bot, *chain, target);

    // what pull end undoes. The pet only went passive if pull start already ran.
    if (!strategy->IsPullPendingToStart())
        if (Pet* pet = bot->GetPet())
            pet->SetReactState(strategy->GetPetReactState());

    AiObjectContext* context = botAI->GetAiObjectContext();
    context->GetValue<PositionMap&>("position")->Get().erase("pull");
    Value<Unit*>* current = context->GetValue<Unit*>("current target");
    if (current->Get() == target)
        current->Set(nullptr);

    strategy->OnPullEnded();
}

bool FactionChampionsRequestChain(PlayerbotAI* botAI, Player* requester)
{
    Player* bot = botAI ? botAI->GetBot() : nullptr;
    Unit* target = requester ? requester->GetSelectedUnit() : nullptr;
    if (!bot || bot->getClass() != CLASS_DEATH_KNIGHT || !bot->HasSpell(SPELL_DEATH_GRIP) || !target ||
        !target->IsCreature() || !target->IsAlive() || !IsFactionChampion(target->GetEntry()))
        return false;

    GripChainState* chain = nullptr;
    OpenChain(bot, chain);
    if (!chain || chain->target == target->GetGUID())
        return false;

    StartChain(bot, *chain, target);
    return true;
}

bool FactionChampionsChainLink(PlayerbotAI* botAI)
{
    Player* bot = botAI ? botAI->GetBot() : nullptr;
    GripChainState* chain = nullptr;
    return bot && OpenChain(bot, chain) &&
           FactionChampionsGripChain::Pending(chain->turns, bot->GetGUID().GetRawValue());
}

bool FactionChampionsChainGripDue(PlayerbotAI* botAI)
{
    Player* bot = botAI ? botAI->GetBot() : nullptr;
    GripChainState* chain = nullptr;
    Creature* target = bot ? OpenChain(bot, chain) : nullptr;
    return target &&
           FactionChampionsGripChain::Due(chain->turns, bot->GetGUID().GetRawValue(), getMSTime(), MidJump(target));
}

Unit* FactionChampionsChainGripTarget(PlayerbotAI* botAI, Position& stand, bool& walkOnly)
{
    Player* bot = botAI ? botAI->GetBot() : nullptr;
    GripChainState* chain = nullptr;
    Creature* target = bot ? OpenChain(bot, chain) : nullptr;
    uint64 const self = bot ? bot->GetGUID().GetRawValue() : 0;
    if (!target || !FactionChampionsGripChain::Pending(chain->turns, self))
        return nullptr;

    uint32 const index = static_cast<uint32>(FactionChampionsGripChain::PostOf(chain->turns, self));
    Position const post = GetFactionChampionsChainPost(bot, index, PlayerbotAI::IsTank(bot));
    walkOnly = !FactionChampionsGripChain::Due(chain->turns, self, getMSTime(), MidJump(target));
    if (walkOnly)
    {
        // before the pull staging keeps it on its stand, the same post
        if (IsToCStagingHeld(botAI))
            return nullptr;

        stand = post;
        return target;
    }

    // an immune champion waits out the turn
    if (!GripCanLand(target))
        return nullptr;

    Position anchor;
    Position centre = GetFactionChampionsLandingCentre(bot);
    FactionChampionsPack(botAI, anchor, centre);
    bool const away = !target->IsInCombat() ||
                      bot->GetExactDist2d(&centre) >= target->GetExactDist2d(&centre) + GRIP_MIN_GAIN;
    if (away && bot->IsWithinCombatRange(target, GRIP_RANGE) && bot->IsWithinLOSInMap(target))
    {
        stand = bot->GetPosition();
        return target;
    }

    float const span = target->GetExactDist2d(&post);
    float const share = span > GRIP_STAND_DIST ? GRIP_STAND_DIST / span : 1.0f;
    stand.Relocate(target->GetPositionX() + (post.GetPositionX() - target->GetPositionX()) * share,
                   target->GetPositionY() + (post.GetPositionY() - target->GetPositionY()) * share,
                   post.GetPositionZ());
    Position fallback;
    if (ARENA_CENTER.GetExactDist2d(stand) > ARENA_FLOOR_RADIUS && DeriveGripStand(target, centre, fallback))
        stand = fallback;

    return target;
}

void FactionChampionsTrackChainGrip(PlayerbotAI* botAI, Unit* target)
{
    Player* bot = botAI ? botAI->GetBot() : nullptr;
    // not OpenChain: its advance would read the cooldown this grip just started and skip the link
    GripChainState* chain = bot ? ChainFor(bot) : nullptr;
    if (!target || !target->IsAlive() || !chain || chain->target != target->GetGUID())
        return;

    uint64 const self = bot->GetGUID().GetRawValue();
    if (!FactionChampionsGripChain::Pending(chain->turns, self))
        return;

    uint32 const now = getMSTime();
    chain->lastTarget = target->GetGUID();
    chain->lastLinkMs = now;
    FactionChampionsGripChain::TrackGrip(chain->turns, self, now);
    if (FactionChampionsGripChain::Closed(chain->turns))
        CloseChain(*chain);
}

ObjectGuid FactionChampionsChainTarget(Player* bot)
{
    GripChainState* chain = nullptr;
    Creature* target = OpenChain(bot, chain);
    return target ? target->GetGUID() : ObjectGuid::Empty;
}

Unit* FactionChampionsSnareTarget(PlayerbotAI* botAI, uint32& spellId)
{
    FactionChampionsDefenceState* state = Refresh(botAI);
    Unit* champion = state ? DutyTarget(state->snares, botAI->GetBot(), spellId) : nullptr;
    return champion && champion->IsAlive() ? champion : nullptr;
}

Unit* FactionChampionsPeelTarget(PlayerbotAI* botAI, uint32& spellId)
{
    FactionChampionsDefenceState* state = Refresh(botAI);
    Unit* champion = state ? DutyTarget(state->peels, botAI->GetBot(), spellId) : nullptr;
    return champion && champion->IsAlive() ? champion : nullptr;
}

bool FactionChampionsBreakablePeelAt(PlayerbotAI* botAI, Unit* champion)
{
    FactionChampionsDefenceState* state = champion ? Refresh(botAI) : nullptr;
    if (!state)
        return false;

    return std::any_of(state->peels.begin(), state->peels.end(),
                       [champion](auto const& peel)
                       { return peel.second.target == champion->GetGUID() && PeelBreaks(peel.second.spellId); });
}

Unit* FactionChampionsProtectTarget(PlayerbotAI* botAI, uint32& spellId)
{
    FactionChampionsDefenceState* state = Refresh(botAI);
    Unit* member = state ? DutyTarget(state->protects, botAI->GetBot(), spellId) : nullptr;
    return member && member->IsAlive() ? member : nullptr;
}

Unit* FactionChampionsBattleResTarget(PlayerbotAI* botAI, uint32& spellId)
{
    if (!botAI || botAI->GetBot()->getClass() != CLASS_DRUID)
        return nullptr;

    FactionChampionsDefenceState* state = Refresh(botAI);
    Unit* corpse = state ? DutyTarget(state->battleRes, botAI->GetBot(), spellId) : nullptr;
    return corpse && !corpse->IsAlive() ? corpse : nullptr;
}

bool FactionChampionsPersonalHit(PlayerbotAI* botAI, RaidPredictedHit& hit)
{
    FactionChampionsDefenceState* state = Refresh(botAI);
    if (!state)
        return false;

    Player* bot = botAI->GetBot();
    uint32 on = 0;
    bool physical = false;
    for (Unit* champion : FactionChampionsInCombat(botAI))
    {
        if (champion->GetVictim() != bot)
            continue;

        ++on;
        physical = physical || IsPhysicalChampion(champion->GetEntry());
    }

    uint32 const predicted = CountFor(state->predicted, bot->GetGUID());
    if (predicted > on)
    {
        on = predicted;
        physical = physical || CountFor(state->predictedPhysical, bot->GetGUID()) > 0;
    }

    hit.focus = on >= FOCUS_PREEMPT_CHAMPIONS;
    if (on < FOCUS_MIN_CHAMPIONS)
        return false;

    uint32 const now = getMSTime();
    uint32 const health = bot->GetHealth();
    float const floor = bot->GetMaxHealth() * RAID_DEFENSIVE_FLOOR_PCT / 100.0f;
    float projected = 0.0f;
    auto const log = state->healthLog.find(bot->GetGUID());
    if (log != state->healthLog.end())
    {
        auto const from = std::find_if(log->second.begin(), log->second.end(), [now](HealthSample const& sample)
                                       { return getMSTimeDiff(sample.ms, now) <= FOCUS_WINDOW_MS; });
        uint32 const elapsed = from != log->second.end() ? getMSTimeDiff(from->ms, now) : 0;
        if (elapsed >= HEALTH_SAMPLE_MS && from->health > health)
            projected = static_cast<float>(from->health - health) * FOCUS_LOOKAHEAD_MS / static_cast<float>(elapsed);
    }

    // this many kill a non-tank from full before the log shows the loss. Just past the floor, so
    // a class cooldown goes now and the immunities and the healthstone still wait for a projected kill.
    if (on >= FOCUS_PREEMPT_CHAMPIONS && !IsTankMember(bot))
        projected = std::max({projected, static_cast<float>(health) - floor + 1.0f, 1.0f});

    if (projected <= 0.0f || static_cast<float>(health) - projected >= floor)
        return false;

    hit.startMs = now;
    hit.durationMs = FOCUS_LOOKAHEAD_MS;
    hit.damage = static_cast<uint32>(projected);
    hit.school = physical ? SPELL_SCHOOL_MASK_NORMAL : SPELL_SCHOOL_MASK_MAGIC;
    hit.measured = true;
    return true;
}

ObjectGuid FactionChampionsNextFocus(PlayerbotAI* botAI)
{
    FactionChampionsDefenceState* state = Refresh(botAI);
    return state ? state->nextFocus : ObjectGuid::Empty;
}

namespace
{
// This DK's Chains of Ice is off its champion or near its end, and the champion can take one: not CC'd,
// held by our breakable CC, snare immune or in the air
Unit* IceDue(PlayerbotAI* botAI)
{
    Player* bot = botAI ? botAI->GetBot() : nullptr;
    Unit* champion = bot ? FactionChampionsIceTarget(botAI) : nullptr;
    if (!champion)
        return nullptr;

    Aura const* chains = champion->GetAura(SPELL_CHAINS_OF_ICE, bot->GetGUID());
    if (chains && (chains->GetDuration() < 0 || chains->GetDuration() > ICE_RECAST_LEFT_MS))
        return nullptr;

    SpellInfo const* info = sSpellMgr->GetSpellInfo(SPELL_CHAINS_OF_ICE);
    if (!info || champion->IsImmunedToSpell(info) || SnareImmune(champion) || MidJump(champion) ||
        champion->HasUnitState(UNIT_STATE_LOST_CONTROL) || FactionChampionsCcProtected(botAI, champion))
        return nullptr;

    return champion;
}
}  // namespace

Unit* FactionChampionsIceChains(PlayerbotAI* botAI, uint32& spellId)
{
    spellId = 0;
    Unit* champion = IceDue(botAI);
    Player* bot = champion ? botAI->GetBot() : nullptr;
    if (!bot || !FreeToAct(bot) || !SpellReady(bot, SPELL_CHAINS_OF_ICE) || !FrostRuneReady(bot) ||
        !bot->IsWithinCombatRange(champion, CHAINS_OF_ICE_RANGE) || !bot->IsWithinLOSInMap(champion))
        return nullptr;

    spellId = SPELL_CHAINS_OF_ICE;
    return champion;
}

bool FactionChampionsIceReach(PlayerbotAI* botAI, Position& stand)
{
    Unit* champion = IceDue(botAI);
    Player* bot = champion ? botAI->GetBot() : nullptr;
    FactionChampionsDefenceState* state = bot ? Refresh(botAI) : nullptr;
    if (!state || !InControl(bot) || bot->HasAuraType(SPELL_AURA_MOD_ROOT) || !FrostRuneReady(bot) ||
        (bot->IsWithinCombatRange(champion, CHAINS_OF_ICE_RANGE) && bot->IsWithinLOSInMap(champion)))
        return false;

    // its own side of the champion, turning off it for the floor's edge and the AoE
    float const base = champion->GetAngle(bot);
    for (float turn : {0.0f, 30.0f, -30.0f, 60.0f, -60.0f, 90.0f, -90.0f})
    {
        float const angle = base + turn * static_cast<float>(M_PI) / 180.0f;
        Position const spot(champion->GetPositionX() + ICE_STAND * std::cos(angle),
                            champion->GetPositionY() + ICE_STAND * std::sin(angle), champion->GetPositionZ());
        if (ARENA_CENTER.GetExactDist2d(spot) > ARENA_FLOOR_RADIUS ||
            std::any_of(state->hazards.begin(), state->hazards.end(), [&spot](HazardSource const& source)
                        { return spot.GetExactDist2d(source.pos) < source.clearance; }))
            continue;

        if (bot->GetExactDist2d(&spot) <= ICE_ARRIVE)
            return false;

        stand = spot;
        return true;
    }

    return false;
}

Position FactionChampionsAoeDodgeSpot(PlayerbotAI* botAI)
{
    FactionChampionsDefenceState* state = Refresh(botAI);
    if (!state)
        return Position();

    Player* bot = botAI->GetBot();
    std::vector<EncounterHelpers::HazardCircle> clearances;
    // The bot stands in at least one, its walk out may cross that one but no other
    std::vector<EncounterHelpers::HazardCircle> others;
    for (HazardSource const& source : state->hazards)
    {
        if (DodgeSkips(source, bot))
            continue;

        clearances.emplace_back(source.pos, source.clearance);
        if (bot->GetExactDist2d(source.pos) > source.radius + IN_AOE_MARGIN)
            others.emplace_back(source.pos, source.radius);
    }

    std::vector<Position> healers;
    for (Member const& member : MembersOnMap(bot))
        if (member.healer && member.player != bot)
            healers.push_back(member.player->GetPosition());

    EncounterHelpers::HazardSweepCache cache;
    if (healers.empty())
        return EncounterHelpers::FindNearestPositionClearOfHazards(bot, clearances, AOE_SEARCH_RADIUS, &cache);

    float x = 0.0f;
    float y = 0.0f;
    for (Position const& healer : healers)
    {
        x += healer.GetPositionX();
        y += healer.GetPositionY();
    }

    std::vector<Position> melee;
    for (Unit* champion : FactionChampionsInCombat(botAI))
        if (IsFactionChampionMelee(champion->GetEntry()))
            melee.push_back(champion->GetPosition());

    float const count = static_cast<float>(healers.size());
    Position const centre(x / count, y / count, bot->GetPositionZ());
    Position const from = bot->GetPosition();
    auto const accept = [&healers, &melee, &others, &from](float spotX, float spotY)
    {
        bool const reached = std::any_of(healers.begin(), healers.end(), [spotX, spotY](Position const& healer)
                                         { return healer.GetExactDist2d(spotX, spotY) <= HEAL_REACH; });
        bool const offMelee = std::none_of(melee.begin(), melee.end(), [spotX, spotY](Position const& champion)
                                           { return champion.GetExactDist2d(spotX, spotY) < DODGE_MELEE_GAP; });
        Position const spot(spotX, spotY, from.GetPositionZ());
        return reached && offMelee &&
               std::none_of(others.begin(), others.end(),
                            [&from, &spot](EncounterHelpers::HazardCircle const& circle)
                            { return EncounterHelpers::SegmentCrossesCircle(from, spot, circle); });
    };

    constexpr float ringStep = 2.0f;
    constexpr float angleStep = static_cast<float>(M_PI) / 8.0f;
    Position spot = EncounterHelpers::FindNearestPositionClearOfHazards(bot, clearances, AOE_SEARCH_RADIUS, ringStep,
                                                                        angleStep, &centre, accept, &cache);
    if (IsNoSpot(spot))
        spot = EncounterHelpers::FindNearestPositionClearOfHazards(bot, clearances, AOE_SEARCH_RADIUS, ringStep,
                                                                   angleStep, &centre, {}, &cache);

    return spot;
}

Unit* FactionChampionsWallGrip(PlayerbotAI* botAI, Position& stand, uint32& spellId, bool* gripped)
{
    spellId = 0;
    if (!botAI || botAI->GetBot()->getClass() != CLASS_DEATH_KNIGHT)
        return nullptr;

    FactionChampionsDefenceState* state = Refresh(botAI);
    if (!state)
        return nullptr;

    Player* bot = botAI->GetBot();
    auto const duty = state->walls.find(bot->GetGUID());
    Creature* champion = duty != state->walls.end() ? AliveCreature(bot, duty->second.champion) : nullptr;
    if (!champion)
        return nullptr;

    if (gripped)
        *gripped = duty->second.gripped;

    stand = duty->second.stand;
    if (!duty->second.gripped && bot->GetExactDist2d(&stand) > WALL_ARRIVE)
        return champion;

    // at the stand, or slowing the champion where it landed: the DK stays put
    stand = bot->GetPosition();
    if (!duty->second.gripped)
    {
        if (!MidJump(champion) && bot->IsWithinCombatRange(champion, GRIP_RANGE) && bot->IsWithinLOSInMap(champion) &&
            SpellReady(bot, SPELL_DEATH_GRIP))
            spellId = SPELL_DEATH_GRIP;

        return champion;
    }

    SpellInfo const* chains = sSpellMgr->GetSpellInfo(SPELL_CHAINS_OF_ICE);
    if (chains && !MidJump(champion) && !champion->IsImmunedToSpell(chains) &&
        bot->IsWithinCombatRange(champion, CHAINS_OF_ICE_RANGE) && SpellReady(bot, SPELL_CHAINS_OF_ICE) &&
        FrostRuneReady(bot) && bot->IsWithinLOSInMap(champion))
        spellId = SPELL_CHAINS_OF_ICE;

    return champion;
}

void FactionChampionsTrackWallGrip(PlayerbotAI* botAI, Unit* champion)
{
    // no Refresh: on a new ms it reruns UpdateWalls, which sees Death Grip on cooldown and drops the duty first
    Player* bot = botAI ? botAI->GetBot() : nullptr;
    if (!champion || !bot || !bot->GetInstanceId() || !ToCEncounterIsLive(botAI, ToCEncounter::FactionChampions))
        return;

    FactionChampionsDefenceState& state = defenceStates.For(bot->GetInstanceId());
    auto const duty = state.walls.find(bot->GetGUID());
    if (duty == state.walls.end() || duty->second.champion != champion->GetGUID() || duty->second.gripped)
        return;

    duty->second.gripped = true;
    duty->second.grippedMs = getMSTime();
}

Unit* FactionChampionsFreedomTarget(PlayerbotAI* botAI, uint32& spellId)
{
    if (!botAI || botAI->GetBot()->getClass() != CLASS_PALADIN)
        return nullptr;

    FactionChampionsDefenceState* state = Refresh(botAI);
    Unit* member = state ? DutyTarget(state->freedoms, botAI->GetBot(), spellId) : nullptr;
    return member && member->IsAlive() ? member : nullptr;
}

}
