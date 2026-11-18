/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RAID_TOCTRIGGERS_JORMUNGARS_H
#define PLAYERBOTS_RAID_TOCTRIGGERS_JORMUNGARS_H

#include "ToCTriggers_NorthrendBeasts.h"
#include "Trigger.h"

class NorthrendWormsMobileTankDutyTrigger : public Trigger
{
public:
    static constexpr char const* Name = "northrend worms mobile tank duty";

    NorthrendWormsMobileTankDutyTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class NorthrendWormsStationaryTankDutyTrigger : public Trigger
{
public:
    static constexpr char const* Name = "northrend worms stationary tank duty";

    NorthrendWormsStationaryTankDutyTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class NorthrendWormsRedirectThreatTrigger : public Trigger
{
public:
    static constexpr char const* Name = "northrend worms redirect threat";

    NorthrendWormsRedirectThreatTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

// Pools, Spew, Sweep, Bile, the spray spread, melee stands, reach round a pool and the Toxin cure
// walk, all through one mover. Off while the bot stands on its held spot.
class NorthrendWormsMisplacedTrigger : public Trigger
{
public:
    static constexpr char const* Name = "northrend worms misplaced";

    NorthrendWormsMisplacedTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

// No worm is engaged and the bot stands in a pool one left behind
class NorthrendWormsPoolAfterWormsTrigger : public Trigger
{
public:
    static constexpr char const* Name = "northrend worms pool after worms";

    NorthrendWormsPoolAfterWormsTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

// A healer's Toxin carrier, one no Bile can cure, is hurt and in heal reach
class NorthrendWormsToxinCarrierHurtTrigger : public Trigger
{
public:
    static constexpr char const* Name = "northrend worms toxin carrier hurt";

    NorthrendWormsToxinCarrierHurtTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

// The bot's own uncurable Toxin has it stuck and its immunity is ready
class NorthrendWormsToxinStuckTrigger : public Trigger
{
public:
    static constexpr char const* Name = "northrend worms toxin stuck";

    NorthrendWormsToxinStuckTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

#endif
