/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RAID_TOCMULTIPLIERS_JORMUNGARS_H
#define PLAYERBOTS_RAID_TOCMULTIPLIERS_JORMUNGARS_H

#include <vector>

#include "Multiplier.h"
#include "RaidCombatHelpers.h"

// Holds other movers while a reposition walk is in flight, since every MoveTo clears the MotionMaster
// and any of them would cancel it. Attacks, `avoid aoe` and the beasts' own movers stay.
class NorthrendWormsMoveGuardMultiplier : public Multiplier
{
public:
    NorthrendWormsMoveGuardMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "northrend worms move guard") {}
    float GetValue(Action* action) override;

private:
    struct Facts
    {
        bool spellMover = false;
        bool mover = false;
        bool staysFree = false;
    };

    RaidActionCache<Facts> _facts;
};

// A Burning Bile carrier the reposition walked off the raid stays off it: its pulse hits everyone
// within 10 yd, melee included, and `reach melee` or a charge would put it back in the stack.
class NorthrendWormsBileReachGuardMultiplier : public Multiplier
{
public:
    NorthrendWormsBileReachGuardMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "northrend worms bile reach guard") {}
    float GetValue(Action* action) override;

private:
    RaidActionCache<bool> _reach;
};

// The class redirects pick one tank for the whole raid. Here each worm has its own holder, and the
// encounter's redirect node casts them.
class NorthrendWormsRedirectGuardMultiplier : public Multiplier
{
public:
    NorthrendWormsRedirectGuardMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "northrend worms redirect guard") {}
    float GetValue(Action* action) override;

private:
    RaidActionCache<bool> _classRedirect;
};

void AddToCJormungarsMultipliers(PlayerbotAI* botAI, std::vector<Multiplier*>& multipliers);

#endif
