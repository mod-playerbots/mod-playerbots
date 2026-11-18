/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RAID_TOCHELPERS_NORTHRENDBEASTS_H
#define PLAYERBOTS_RAID_TOCHELPERS_NORTHRENDBEASTS_H

#include <vector>

#include "Define.h"

class Player;
class PlayerbotAI;
class Unit;

// Which beasts are up and which tank holds each, read once per instance per ms on map 649 so every bot
// agrees. Heroic brings the next beast in on a timer, so a slow stage overlaps the next one.
namespace TrialOfTheCrusaderHelpers
{

enum class NorthrendBeast : uint8
{
    Gormok,
    Acidmaw,
    Dreadscale,
    Icehowl
};

// Deal order after None is the priority order: a duty with no tank left takes the one holding the
// lowest-priority duty below it.
enum class BeastsTankDuty : uint8
{
    None,
    Gormok,
    Icehowl,
    WormMobile,
    GormokSwap,
    WormStationary
};

// GetBeastsStageMask bits
inline constexpr uint32 BEASTS_STAGE_GORMOK = 1;
inline constexpr uint32 BEASTS_STAGE_WORMS = 2;
inline constexpr uint32 BEASTS_STAGE_ICEHOWL = 4;

// Alive and in combat, else nullptr. A beast still walking in isn't engaged.
Unit* GetEngagedBeast(PlayerbotAI* botAI, NorthrendBeast beast);

uint32 GetBeastsStageMask(PlayerbotAI* botAI);

// Bot or human, by strategy or by spec
bool IsBeastsTank(Player* player);

// The bot tanks the duty deal hands out from, flagged main tank first, then by guid. With nothing engaged
// roster[0] takes every main duty and roster[1] the swap and the stationary worm.
std::vector<Player*> GetBeastsTankRoster(Player* bot);

// None for a tank the deal left out and for every non-tank
BeastsTankDuty GetBeastsTankDuty(PlayerbotAI* botAI);

// Null while the duty's beast isn't engaged or no tank was left for it. Can be a human, but only for
// a beast he's already tanking.
Player* GetBeastsDutyHolder(PlayerbotAI* botAI, BeastsTankDuty duty);

// GormokSwap answers Gormok. The worm duties follow the form, so their beast changes when the worms
// swap.
Unit* GetBeastOfDuty(PlayerbotAI* botAI, BeastsTankDuty duty);

// The main duty on that beast, so Gormok never answers GormokSwap. None for anything that isn't an
// engaged beast, null included.
BeastsTankDuty GetDutyOfBeast(PlayerbotAI* botAI, Unit* beast);

// A DPS bot's target, in combat: a player's skull on the encounter, its snobold, Gormok, the mobile worm
// (by form, never by holder), Icehowl. Null off map 649 or for a bot that isn't IsRaidDpsBot.
Unit* GetBeastsDpsTarget(PlayerbotAI* botAI);

}

#endif
