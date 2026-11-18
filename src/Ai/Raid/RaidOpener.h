/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RAIDOPENER_H
#define PLAYERBOTS_RAIDOPENER_H

#include <string>
#include <vector>

#include "Action.h"
#include "Define.h"
#include "Multiplier.h"
#include "RaidRedirectThreat.h"
#include "RaidSplash.h"
#include "Trigger.h"

class Player;
class PlayerbotAI;
class Unit;

// A DPS bot holds its damage on a live boss until a group tank has held it for RAID_OPENER_DWELL_MS, or
// for RAID_OPENER_CAP_MS from its pull. A hunter or rogue with a redirect running is free.

inline constexpr uint32 RAID_OPENER_DWELL_MS = 2000;
// A boss no tank picks up in this time needs a taunt, and DPS standing idle doesn't help it
inline constexpr uint32 RAID_OPENER_CAP_MS = 5000;

// One per raid. Nothing resolves off mapId.
struct RaidOpenerConfig
{
    uint32 mapId;
    // boss entries of the live encounter, empty when none is live or it has no opener
    std::vector<uint32> const& (*liveBosses)(PlayerbotAI* botAI);
};

// unit is a boss still in its opener and this bot holds its damage on it
bool IsRaidOpenerHeld(PlayerbotAI* botAI, RaidOpenerConfig const& config, Unit* unit);

// The boss this bot holds on within reach of it, for area damage. nullptr if none.
Unit* GetRaidOpenerHeldBossNear(PlayerbotAI* botAI, RaidOpenerConfig const& config, float reach);

// This bot holds on some boss
bool RaidOpenerHolds(PlayerbotAI* botAI, RaidOpenerConfig const& config);

// A hunter or rogue that holds and can redirect: the tank holding its current target, else the tank
// on any boss it holds on. nullptr if none.
Player* GetRaidOpenerRedirectTank(PlayerbotAI* botAI, RaidOpenerConfig const& config);

// A DPS bot's auto attack running on a boss still in its opener: picking the boss starts the swing.
class RaidOpenerSwingTrigger : public Trigger
{
public:
    RaidOpenerSwingTrigger(PlayerbotAI* botAI, std::string const name, RaidOpenerConfig const& config)
        : Trigger(botAI, name), _config(config) {}
    bool IsActive() override;

private:
    RaidOpenerConfig const& _config;
};

// A hunter or rogue waiting on a boss a tank holds, with Misdirection or Tricks of the Trade ready
class RaidOpenerRedirectTrigger : public Trigger
{
public:
    RaidOpenerRedirectTrigger(PlayerbotAI* botAI, std::string const name, RaidOpenerConfig const& config)
        : Trigger(botAI, name), _config(config) {}
    bool IsActive() override;

private:
    RaidOpenerConfig const& _config;
};

// Ends the swing a boss pick started. The boss stays the bot's target, so melee keep walking in.
class RaidOpenerStopSwingAction : public Action
{
public:
    RaidOpenerStopSwingAction(PlayerbotAI* botAI, std::string const name, RaidOpenerConfig const& config)
        : Action(botAI, name), _config(config) {}
    bool Execute(Event event) override;
    bool isUseful() override;

private:
    RaidOpenerConfig const& _config;
};

// Misdirection or Tricks of the Trade on the tank holding the boss, which frees the bot's damage
class RaidOpenerRedirectAction : public RaidRedirectThreatAction
{
public:
    RaidOpenerRedirectAction(PlayerbotAI* botAI, std::string const name, RaidOpenerConfig const& config)
        : RaidRedirectThreatAction(botAI, name), _config(config) {}

protected:
    Player* GetRedirectTank() override;
    Unit* GetThreatDumpTarget() override { return nullptr; }

private:
    RaidOpenerConfig const& _config;
};

// A DPS bot's damage on a boss still in its opener, and its area damage near one. The first pick of
// the boss goes through so melee run in; a repeat pick would restart the swing the opener stopped.
class RaidOpenerHoldMultiplier : public Multiplier
{
public:
    RaidOpenerHoldMultiplier(PlayerbotAI* botAI, std::string const name, RaidOpenerConfig const& config)
        : Multiplier(botAI, name), _config(config) {}
    float GetValue(Action* action) override;

private:
    RaidOpenerConfig const& _config;
    RaidSplashCache _splash;
};

#endif
