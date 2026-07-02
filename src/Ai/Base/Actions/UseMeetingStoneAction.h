/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_USEMEETINGSTONEACTION_H
#define PLAYERBOTS_USEMEETINGSTONEACTION_H

#include "MovementActions.h"

class Player;
class PlayerbotAI;

class SummonAction : public MovementAction
{
public:
    SummonAction(PlayerbotAI* botAI, std::string const name = "summon") : MovementAction(botAI, name) {}

    bool Execute(Event event) override;
    bool Teleport(Player* summoner, Player* player, bool preserveAuras);

protected:
    bool SummonUsingGos(Player* summoner, Player* player, bool preserveAuras);
    bool SummonUsingNpcs(Player* summoner, Player* player, bool preserveAuras);
};

class UseMeetingStoneAction : public SummonAction
{
public:
    UseMeetingStoneAction(PlayerbotAI* botAI) : SummonAction(botAI, "use meeting stone") {}

    bool Execute(Event event) override;
};

#endif
