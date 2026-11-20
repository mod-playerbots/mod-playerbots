/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_TANKSWAPACTION_H
#define PLAYERBOTS_TANKSWAPACTION_H

#include "Action.h"

class PlayerbotAI;

// Generic tank swap: taunt whatever the bot is not currently tanking, and tell
// the human tanks in the group what is happening in both directions. Used by
// per-encounter triggers that decide when a swap is due (debuff stacks etc).
class TankSwapAction : public Action
{
public:
    TankSwapAction(PlayerbotAI* botAI) : Action(botAI, "tank swap") {}

    bool Execute(Event event) override;
    bool isUseful() override;

private:
    static uint32 TauntSpellFor(uint8 cls);
    void TellHumanTanks(std::string const& text) const;
};

#endif
