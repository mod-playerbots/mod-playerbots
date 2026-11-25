/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_FOURHORSEMEN40_H
#define PLAYERBOTS_FOURHORSEMEN40_H

#include "Action.h"
#include "NamedObjectContext.h"
#include "ObjectGuid.h"
#include "Trigger.h"

class Multiplier;
class Player;
class PlayerbotAI;

// Adapted from reddevilnlp/mod-playerbots at 13492578.
namespace FourHorsemen40
{
enum class Preparation
{
    Inactive,
    Staging,
    Ready
};

Preparation PrepareOpening(PlayerbotAI& leader);
bool PullOpening(PlayerbotAI& leader);
bool RestoreWarriorStance(PlayerbotAI& ai);
bool HasTemporaryStance(Player const& player);
bool ProtectsTankSupplies(Player* player);
bool ConflictsWithTankSupplies(uint32 spell);
ObjectGuid GetOpeningHolder(Player const& member);
bool StagePullHolder(PlayerbotAI& holder);
void AddRearHealingScript();
}  // namespace FourHorsemen40

class FourHorsemen40ActionContext : public NamedObjectContext<Action>
{
public:
    FourHorsemen40ActionContext();
};

class FourHorsemen40TriggerContext : public NamedObjectContext<Trigger>
{
public:
    FourHorsemen40TriggerContext();
};

void AddFourHorsemen40Triggers(std::vector<TriggerNode*>& triggers);
void AddFourHorsemen40Multipliers(PlayerbotAI* botAI, std::vector<Multiplier*>& multipliers);

#endif
