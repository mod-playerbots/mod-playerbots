/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RAID_TOCACTIONS_SHARED_H
#define PLAYERBOTS_RAID_TOCACTIONS_SHARED_H

#include "Action.h"
#include "AttackAction.h"

// DragBossToAnchor's longest step, and how close to its anchor it stops
inline constexpr float TOC_DRAG_STEP = 5.0f;
inline constexpr float TOC_DRAG_DONE = 12.0f;

// Base for the main-tank actions that hold their boss at a fixed point of the room.
class ToCMainTankHoldAction : public AttackAction
{
public:
    ToCMainTankHoldAction(PlayerbotAI* botAI, std::string const name) : AttackAction(botAI, name) {}

protected:
    // Backs the bot up a step toward anchor, boss in tow, while it tanks boss further than TOC_DRAG_DONE
    // out. False once inside that or boss is on someone else.
    bool DragBossToAnchor(Unit* boss, Position const& anchor);
};

#endif
