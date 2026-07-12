#ifndef PLAYERBOTS_MECHANARTRIGGERS_H
#define PLAYERBOTS_MECHANARTRIGGERS_H

#include "Trigger.h"
#include "GenericTriggers.h"
#include "DungeonStrategyUtils.h"

// The Mechanar (map 554): Nethermancer Sepethrea.
//
// She summons a single Raging Flames elemental on engage. It fixates a random party
// member (AddThreat 1,000,000, an unbreakable taunt), re-rolling to a new random member
// each time it casts Inferno (a 10-yd ground-fire AoE, roughly every 20-30s). The
// elemental runs at ~4 yd/s versus a player's 7, so the fixated bot must simply walk it
// away from the party and keep it out of the Inferno radius while everyone else burns the
// boss. It must never be attacked (it is quelled for free when the boss dies), and its
// huge fixate threat would otherwise make bots pick it as their DPS target, so the whole
// party is also pinned onto the boss.
enum class MechanarIDs : uint32
{
    NPC_NETHERMANCER_SEPETHREA = 19221,
    NPC_RAGING_FLAMES          = 20481,  // normal
    NPC_RAGING_FLAMES_HEROIC   = 21538,  // heroic variant ("Raging Flames (1)")
};

// A live Raging Flames elemental is fixated on this bot (it is the flame's current
// victim), so this bot is the one that must kite it.
class SepethreaKiteFlameTrigger : public Trigger
{
public:
    SepethreaKiteFlameTrigger(PlayerbotAI* botAI) : Trigger(botAI, "sepethrea kite flame") {}
    bool IsActive() override;
};

// A live Raging Flames elemental is close enough to burn this bot with Inferno (whether
// or not it is the flame's fixate target): step out of the fire. A safety net for the
// brief window after the fixate switches to a new bot, or when the kiter drags it past a
// bystander.
class SepethreaAvoidFlameTrigger : public Trigger
{
public:
    SepethreaAvoidFlameTrigger(PlayerbotAI* botAI) : Trigger(botAI, "sepethrea avoid flame") {}
    bool IsActive() override;
};

// The bot is standing in one of the persistent fire patches the elemental drops as it
// moves (spell 35278): step off it. Needed because the stock "avoid aoe" strategy that
// would normally handle these is disabled for master-less dungeon-clear bots.
class SepethreaTrailTrigger : public Trigger
{
public:
    SepethreaTrailTrigger(PlayerbotAI* botAI) : Trigger(botAI, "sepethrea trail") {}
    bool IsActive() override;
};

// Sepethrea is engaged (alive, and this bot is in combat): force every bot's DPS
// onto her so nobody ever attacks the flame.
class SepethreaFocusBossTrigger : public Trigger
{
public:
    SepethreaFocusBossTrigger(PlayerbotAI* botAI) : Trigger(botAI, "sepethrea focus boss") {}
    bool IsActive() override;
};

#endif
