#ifndef PLAYERBOTS_MECHANARSHARED_H
#define PLAYERBOTS_MECHANARSHARED_H

#include "Define.h"

#include <utility>
#include <vector>

class Player;
class Unit;
class Creature;

// Shared constants + lookups for the Nethermancer Sepethrea "kite the Raging
// Flames" behaviour. Bodies live in MechanarShared.cpp (not inline) so the header
// carries no dependency on the game-object definitions and can be included freely.
namespace MechanarFlames
{
    constexpr float INFERNO_RADIUS = 10.0f;       // Inferno AoE radius (Spell.dbc 35283)
    constexpr float INFERNO_AVOID_RANGE = 14.0f;  // a bystander bails when a flame is this close
    constexpr float INFERNO_AVOID_CLEAR = 19.0f;  // ...and repositions out to here (10yd AoE + margin)
    // Sepethrea summons two Raging Flames (summon spell 35275, count 2), so there are
    // always two Infernos and two trails in play. A bot is "flame-safe" only when it is
    // at least this far from every elemental centre (10yd Inferno plus 4yd movement
    // buffer). This keeps a kiter clear of the other elemental and stops a bystander or
    // melee from stepping out of one Inferno straight into the other.
    constexpr float INFERNO_SAFE_DIST = 14.0f;
    constexpr float KITE_THRESHOLD = 16.0f;       // the fixated bot hops once the flame closes to this
    constexpr float KITE_STEP = 14.0f;            // how far each kite hop travels

    // End-of-hall turnaround maneuver. The hop-when-flame-closes-to-16yd rhythm is right
    // for the open hall, but at the end of the hall it strands the bot against the wall
    // until the flame is 16yd out, and the per-tick hop scoring then flips between "one
    // more hop toward the wall" (keeps flame distance) and "turn back now" every tick, so
    // the bot oscillates in place while the flame walks into melee reach. The fix is a
    // committed two-phase pass: when fleeing along the hall with under KITE_TURN_RUNWAY of
    // it left and the flame inside KITE_TURN_THRESHOLD, (1) swing out to the nearer clear
    // side wall to build lateral separation, then (2) run down that wall to a lane point
    // KITE_TURN_PASS_BEHIND yards past the flame. That target is recomputed off the flame's
    // live position, so it retreats as the flame advances and the pass never stalls. The
    // maneuver holds its side choice until the bot is clearly past the flame with the
    // normal kite gap restored (KITE_TURN_EXIT_PAST beyond it and more than KITE_THRESHOLD
    // away). A one-move diagonal pass instead of the two-phase cuts straight over the flame
    // because the lateral separation develops too slowly, and a KITE_TURN_PASS_BEHIND below
    // roughly 20 parks the target inside the exit gap so the maneuver never ends; neither
    // value should be reduced.
    constexpr float KITE_TURN_RUNWAY = 16.0f;
    constexpr float KITE_TURN_THRESHOLD = 24.0f;
    constexpr float KITE_TURN_PASS_BEHIND = 20.0f;
    constexpr float KITE_TURN_WALL_INSET = 4.0f;   // pass lane hugs the side wall at this inset
    constexpr float KITE_TURN_EXIT_PAST = 3.0f;
    constexpr uint32 KITE_TURN_FAILSAFE_MS = 12000;  // abandon a stuck maneuver after this
    // While fixated, keep the flame at least this far from the rest of the party. A
    // freshly-fixated bot standing in the stack leads the flame out immediately, rather
    // than waiting for the slow elemental to walk all the way in to 16yd and rake everyone
    // en route, and only holds to DPS once it is this clear.
    constexpr float KITE_GROUP_CLEARANCE = 16.0f;
    // A healer stands in the Inferno rather than dropping a heal to dodge, so long as it
    // is itself above this threshold; below it, the healer disengages to survive.
    constexpr float HEALER_FIRE_BAIL_PCT = 50.0f;

    // A fixated healer's kite is leashed to the tank. Heal range is 40yd, so if the healer
    // runs the flame down the hall like a DPS would, the tank drops out of range and dies.
    // Past HEALER_KITE_LEASH the hop scoring penalises opening further tank distance
    // (HEALER_KITE_LEASH_WEIGHT per yard, which outweighs the ~14yd flame-distance gain of
    // a hop, so the healer turns the flame back toward the tank instead), and no hop may
    // land past HEALER_KITE_LEASH_MAX unless it closes on the tank. The margin under 40yd
    // covers the tank moving away mid-hop and heal-cast pushback. A tighter leash confines
    // the healer to so short a stretch of hall that the frequent reversals past the flame
    // raise its Inferno exposure; these values keep heal range unbroken at low exposure.
    constexpr float HEALER_KITE_LEASH = 34.0f;
    constexpr float HEALER_KITE_LEASH_MAX = 38.0f;
    constexpr float HEALER_KITE_LEASH_WEIGHT = 3.0f;
    // A leashed kite must periodically reverse past its own flame, since the leash forbids
    // running on forever. The destination-only score cannot tell a straight-through
    // reversal (path grazes the flame at 0yd) from a wall-hugging arc around it, so leashed
    // hops also pay this per-yard penalty for the path's closest approach inside the flame's
    // Inferno radius. This biases reversals wide while staying a soft cost, so a true corner
    // (arc walled off) can still force the bot straight through.
    constexpr float HEALER_KITE_FLAME_PATH_WEIGHT = 2.5f;

    // The persistent ground fire the elemental trails behind it. Spell 35281 (a periodic
    // aura on the flame) casts 35278 every second at the flame's current spot. 35278 is a
    // PERSISTENT_AREA_AURA (Effect 27) that spawns a 5yd DynamicObject fire patch lasting
    // 6s (Spell.dbc 35278: radius idx 8 = 5yd, duration idx 32 = 6s), ticking 35312 damage
    // each second. As the flame walks it lays a ribbon of these overlapping patches, which
    // is what kills stacked DPS. See SepethreaAvoidTrailAction.
    constexpr uint32 TRAIL_PATCH_SPELL = 35278;
    constexpr float TRAIL_PATCH_RADIUS = 5.0f;    // fallback if the dynobj radius reads 0
    constexpr float TRAIL_DANGER_MARGIN = 2.5f;   // bail when this close to a patch edge (step out early)
    constexpr float TRAIL_SCAN = 18.0f;           // enumerate patches within this of the bot for repulsion
    constexpr float TRAIL_CLEAR_STEP = 10.0f;     // how far to hop out of the ribbon in one move

    // One Raging-Flames ground fire patch (a live 35278 DynamicObject): its 2D centre
    // and effective radius.
    struct TrailPatch
    {
        float x;
        float y;
        float radius;
    };

    // Fill `out` with every live trail fire patch (35278 dynobj) within `scanRadius`
    // (2D) of `bot`. Uses a direct grid search of DynamicObjects, independent of whether
    // the patch aura is applied to or positive-classified on the bot, so it detects the
    // trail reliably.
    void CollectTrailPatches(Player* bot, float scanRadius, std::vector<TrailPatch>& out);

    // True if `bot` is standing in (or within TRAIL_DANGER_MARGIN of the edge of) any
    // trail fire patch, i.e. it should step out now.
    bool InTrailDanger(Player* bot);

    // Sepethrea's chamber (map 554, floor 2) kite containment. The chamber is a long,
    // narrow hall: its long axis is world Y (~80yd, y from -26 to 54), its width is world
    // X (only ~23yd, x from 285 to 308). Its one opening is to the NW, the elevator/bridge
    // corridor (mouth ~x274, landing ~(265,52)), and beyond it the Pathaleon bridge. A
    // fixated bot kites the flame, and when that bot is the tank the boss follows on
    // threat: if the kite reaches the NW corridor it drags the fight onto the bridge and
    // starts the Pathaleon gauntlet, wiping the party. The kite must therefore run the
    // flame up and down the room's length (Y), not across its short width (where it wedges
    // the bot against a side wall), and must never cross to the low-X corridor.
    //
    // The walkable fight floor is modelled as a fixed rectangle spanning the full length
    // and capping the low-X side well short of the corridor mouth. Every interior cell was
    // verified on-navmesh against the live map-554 mmaps; the rectangle excludes the
    // corridor, the NW opening, a raised platform at the NE (x above ~309), and the boss
    // alcove. Vertices are map (x,y), CCW. Kite scoring adds a long-axis (Y) reward so bots
    // use the length.
    //
    //     x=308 +----------------------------------------+ (long axis: Y ->)
    //           |  <- corridor/NW  fight hall (length)   |
    //     x=285 +----------------------------------------+
    //         y=-26                                     y=54   (x=285 caps the low-X
    //                                                            side off the corridor)
    constexpr float ROOM_ANCHOR_X = 300.0f;  // fight core (boss pull spot); kite fallback target
    constexpr float ROOM_ANCHOR_Y = 6.0f;

    // The containment rectangle's bounds (the polygon in MechanarShared.cpp is built from
    // these): X spans the hall's short width, Y its long length. The turn maneuver refers
    // to them by name: the end walls are the Y bounds, the pass lanes hug the X bounds.
    constexpr float ROOM_X_MIN = 285.0f;
    constexpr float ROOM_X_MAX = 308.0f;
    constexpr float ROOM_Y_MIN = -26.0f;
    constexpr float ROOM_Y_MAX = 54.0f;

    // The room's long axis is world Y; the kite fan rewards headings aligned with it by
    // this weight (yards-equivalent, added to the flee-distance score) so a fixated bot
    // runs the flame down the length of the hall rather than across its short width.
    constexpr float KITE_LONG_AXIS_WEIGHT = 7.0f;

    // True if map point (x,y) is inside the kite containment polygon.
    bool InRoom(float x, float y);

    // True if the whole hop a->b stays inside the polygon: both ends and the midpoint are
    // sampled. The midpoint sample is defensive should the containment shape ever be made
    // non-convex.
    bool SegmentInRoom(float ax, float ay, float bx, float by);

    // If (x,y) is outside the polygon, move it to the nearest point just inside it;
    // no-op when already inside. Pulls a drifted bot / dodge destination back into the
    // room (never toward the NW opening).
    void ClampIntoRoom(float& x, float& y);

    // True if `bot` is a healer that should keep casting through the fire right now rather
    // than dodge: it is a healer, a party member currently needs healing, and the healer
    // itself is still above HEALER_FIRE_BAIL_PCT (below that it disengages to survive).
    // Used to suppress both the elemental-dodge and the trail-dodge.
    bool HealerHoldsFire(Player* bot);

    // True if `bot` is a tank that must (re)claim Sepethrea immediately: she is alive,
    // engaged, and her current victim is not a tank-role player, so the boss is loose on
    // the party (a fresh pull before first aggro, or her threat-reducing Arcane Blast left
    // the tank below a DPS). This outranks kiting: a loose boss can one-shot a cloth wearer
    // while a flame merely melees the tank, so a fixated tank abandons its kite (kite
    // trigger off, movement lock lifted, focus action redirects it onto her) until she is
    // back on a tank. Kiting while boss aggro is held stays fine, since she follows the
    // kiting tank on threat.
    bool TankMustGrabBoss(Player* bot);

    // True if `unit` is a Raging Flames elemental (normal or heroic entry).
    bool IsFlame(Unit* unit);

    // The nearest live Raging Flames elemental within `radius` (2D) of `bot`, or null.
    Creature* GetNearestFlame(Player* bot, float radius);

    // Centres (x,y) of every alive Raging Flames elemental within 100yd of `bot`, except
    // `ignoreFlame` (pass the one fixated on the caller: a kiter flees that one directly
    // and needs to avoid only the other; pass nullptr to collect them all). Used to keep
    // bots clear of every elemental's Inferno, not just the nearest or its own.
    void CollectAvoidFlames(Player* bot, Unit* ignoreFlame, std::vector<std::pair<float, float>>& out);

    // The live Raging Flames elemental currently fixated on `bot` (its victim), or null;
    // i.e. whether a flame is chasing this bot.
    Unit* GetFixatingFlame(Player* bot);

    // The live Nethermancer Sepethrea near `bot`, or null.
    Unit* GetSepethrea(Player* bot);
}

#endif
