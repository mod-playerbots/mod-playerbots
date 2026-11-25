/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_NAXX40_HORSEMEN_H
#define PLAYERBOTS_NAXX40_HORSEMEN_H

#include <vector>

class Action;
class Multiplier;
class Player;
class PlayerbotAI;
class Trigger;
class TriggerNode;

template <class T>
class NamedObjectContext;

namespace Naxx40Horsemen
{
bool IsNaxx40(Player* player);
void RegisterActions(NamedObjectContext<Action>& actions);
void RegisterTriggers(NamedObjectContext<Trigger>& triggers);
void InitTriggers(std::vector<TriggerNode*>& triggers);
void InitMultipliers(PlayerbotAI* ai, std::vector<Multiplier*>& multipliers);
}  // namespace Naxx40Horsemen

#endif
