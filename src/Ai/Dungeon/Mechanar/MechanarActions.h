#ifndef PLAYERBOTS_MECHANARACTIONS_H
#define PLAYERBOTS_MECHANARACTIONS_H

#include "AttackAction.h"
#include "MovementActions.h"
#include "MechanarTriggers.h"

// Nethermancer Sepethrea: Raging Flames.

// The bot is fixated: walk the flame away from the party (out of its Inferno), staying
// in the room, and keep casting/healing between hops (the flame is slow, so this
// only fires when it closes in).
class SepethreaKiteFlameAction : public MovementAction
{
public:
    SepethreaKiteFlameAction(PlayerbotAI* botAI) : MovementAction(botAI, "sepethrea kite flame") {}
    bool Execute(Event event) override;

private:
    // End-of-hall turn maneuver commitment (see KITE_TURN_* in MechanarShared.h).
    // Per-tick re-scoring cannot execute a multi-second pass: the destination flips
    // sides every tick and the bot oscillates against the wall. So once entered, the
    // maneuver's side and direction stick until the pass completes (or the failsafe
    // expires). Plain members are safe: one action instance per bot AI context.
    uint32 _turnUntilMs = 0;      // 0 = not maneuvering; else failsafe deadline
    float _turnSide = 1.0f;       // +1 = pass along ROOM_X_MAX wall, -1 = ROOM_X_MIN
    float _turnEnd = 1.0f;        // +1 = trapped at ROOM_Y_MAX end, -1 = ROOM_Y_MIN
    bool _turnLateral = false;    // phase: true = swinging out, false = running the lane
    ObjectGuid _turnFlame;        // the flame this maneuver was planned against
};

// The bot is not the kiter but a flame is burning it: step out of the Inferno.
class SepethreaAvoidFlameAction : public MovementAction
{
public:
    SepethreaAvoidFlameAction(PlayerbotAI* botAI) : MovementAction(botAI, "sepethrea avoid flame") {}
    bool Execute(Event event) override;
};

// The bot is standing in the ribbon of persistent fire patches the elemental trails
// behind it: repel out of the whole ribbon in one hop. Replaces the stock "avoid aoe"
// action, whose single-patch, boss-relative, radius+1 flee just hops a bot from one patch
// of the overlapping trail into the next (and can be pulled straight back by combat move).
class SepethreaAvoidTrailAction : public MovementAction
{
public:
    SepethreaAvoidTrailAction(PlayerbotAI* botAI) : MovementAction(botAI, "sepethrea avoid trail") {}
    bool Execute(Event event) override;
};

// Pin every bot's DPS onto Sepethrea so nobody attacks the flame.
class SepethreaFocusBossAction : public AttackAction
{
public:
    SepethreaFocusBossAction(PlayerbotAI* botAI) : AttackAction(botAI, "sepethrea focus boss") {}
    bool Execute(Event event) override;
};

#endif
