/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_XPGAINACTION_H
#define PLAYERBOTS_XPGAINACTION_H

#include "Action.h"

class PlayerbotAI;
class Unit;

class XpGainAction : public Action
{
public:
    XpGainAction(PlayerbotAI* botAI) : Action(botAI, "xp gain") {}

    bool Execute(Event event) override;

private:
    void GiveXP(uint32 xp, Unit* victim);
};

#endif
