/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_THREATHELPERS_H
#define PLAYERBOTS_THREATHELPERS_H

#include <string_view>

class Player;
class Unit;

// The "find target" value without the value machinery. Like it, this sees only units that have the
// bot on their threat list, ignores case, and returns null if the bot has no group.
Unit* FindThreatTarget(Player* bot, std::string_view name);

#endif
