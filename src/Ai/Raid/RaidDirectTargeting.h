/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RAIDDIRECTTARGETING_H
#define PLAYERBOTS_RAIDDIRECTTARGETING_H

#include <string>

#include "AttackAction.h"
#include "Define.h"

class PlayerbotAI;
class Unit;

// Base for the actions that put a bot on a target picked in code, never through a raid icon.
class RaidDirectAttackAction : public AttackAction
{
public:
    RaidDirectAttackAction(PlayerbotAI* botAI, std::string const name) : AttackAction(botAI, name) {}

protected:
    // Sends the pet every call, then false once the bot is already on target so lower nodes run.
    bool AttackDirect(Unit* target);
};

// The group skull, a player's since bots place none, on a unit belongs() accepts that is alive, in combat,
// selectable and attackable by this bot. Null without a group.
Unit* GetRaidHumanSkull(PlayerbotAI* botAI, bool (*belongs)(Unit*));

// A living pet CommandPetAttack would send to pick, not on it yet. Resolver triggers fire on this
// too: no generic node sends the pet, only their action does.
bool RaidPetOffPick(PlayerbotAI* botAI, Unit* pick);

// On mapId, puts a bot's rti of cross, square, triangle or diamond back to skull: rti is saved per
// bot and no direct targeting node reads those tags. rti cc stays, ToC's Faction Champions uses it.
void RaidRestoreLegacyRti(PlayerbotAI* botAI, uint32 mapId);

#endif
