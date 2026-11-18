/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RAID_TOCHELPERS_STAGING_H
#define PLAYERBOTS_RAID_TOCHELPERS_STAGING_H

#include "Define.h"
#include "ObjectDefines.h"
#include "Position.h"
#include "ToCEncounterGate.h"

class Player;
class PlayerbotAI;

// Pre-pull stands: every boss turns hostile on the same script point each pull, so bots wait on a
// dealt stand around it, and only tanks stand inside its release clearance. Nothing resolves off map 649.
namespace TrialOfTheCrusaderHelpers
{

enum class ToCStagingBoss : uint8
{
    None,
    Gormok,
    Worms,
    Icehowl,
    Jaraxxus,
    Champions,
    Twins,
    Anubarak
};

// A parked bot walks back once pushed past UNPARK
inline constexpr float STAGING_PARK = 1.0f;
inline constexpr float STAGING_UNPARK = 2.5f;
// The deal moving a stand further than this drops the park and validates the new spot
inline constexpr float STAGING_STAND_MOVED = 0.5f;
// ValidateFloorPoint's answer is kept only this close to the raw stand
inline constexpr float STAGING_FLOOR_TOLERANCE = 3.0f;
// The duty tank stands this far from the release point. The margin covers a 1 yd park either side and a
// pet 3 yd off its owner.
inline constexpr float STAGING_DUTY_STAND = 3.0f;
inline constexpr float STAGING_RELEASE_MARGIN = 5.0f;
inline constexpr uint32 STAGING_DEAL_MS = 1000;
// the 3D gap between a stand and the 2D distance it was dealt at
inline constexpr float STAGING_3D_GAP = 0.5f;

// A forced attack narrows its pick to each passing unit's GetDistance, so anyone inside this beats the
// duty tank whenever the search reaches them first
constexpr float ReleaseClearance(float bossReach)
{
    return STAGING_DUTY_STAND + STAGING_RELEASE_MARGIN + bossReach + DEFAULT_COMBAT_REACH + STAGING_3D_GAP;
}

static_assert(STAGING_PARK < STAGING_UNPARK, "a walk has to end inside its own trigger");

// The raid-wide half of the window, per instance per ms: Barrett's gossip seen and gone at this stage (or
// Jaraxxus chained for a retry), nothing live, next arrival by guid slot. Stage 9 is Anub'arak.
ToCStagingBoss GetToCStagedArrival(PlayerbotAI* botAI);
// That, while the bot is alive, out of combat and in the arrival's room together with its master,
// else the group leader
ToCStagingBoss GetToCStagingBoss(PlayerbotAI* botAI);
ToCEncounter ToCStagingEncounter(PlayerbotAI* botAI);

struct ToCStagingRead
{
    bool held = false;  // in the bot's window with a dealt stand
    bool walk = false;  // not parked on it
    bool duty = false;  // a tank stand a forced attack walks onto
    Position spot;      // the stand, floor-validated
};

// Only a moved stand touches the park latch here, dropping it.
ToCStagingRead GetToCStagingRead(PlayerbotAI* botAI);
// The strategy's tick, before the trigger: a held bot parks while it needs no walk, unparks once it does
void UpdateToCStagingPark(PlayerbotAI* botAI);

// For the rules: no park write
bool IsToCStagingHeld(PlayerbotAI* botAI);
// Held, and walking or on a duty stand
bool IsToCStagingRestHeld(PlayerbotAI* botAI);

// The middle of the Faction Champions' landing slots for the bot's faction, where the pack lands
Position GetFactionChampionsLandingCentre(Player* bot);

// The grip chain DK's wait on the landing centre's line to the arena centre: post 0 on the tank line (a
// non-tank 3 yd behind), post 1 54 yd and post 2 78 yd down the lane
Position GetFactionChampionsChainPost(Player* bot, uint32 post, bool tank);

}

#endif
