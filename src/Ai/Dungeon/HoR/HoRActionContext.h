/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_HORACTIONCONTEXT_H
#define PLAYERBOTS_HORACTIONCONTEXT_H

#include "Action.h"
#include "HoRActions.h"
#include "NamedObjectContext.h"

class WotlkDungeonHoRActionContext : public NamedObjectContext<Action>
{
public:
    WotlkDungeonHoRActionContext()
    {
        creators["hor escape attack"] = &WotlkDungeonHoRActionContext::hor_escape_attack;
        creators["hor escape tank"] = &WotlkDungeonHoRActionContext::hor_escape_tank;
        creators["hor escape follow"] = &WotlkDungeonHoRActionContext::hor_escape_follow;
    }

private:
    static Action* hor_escape_attack(PlayerbotAI* botAI) { return new HorEscapeAttackAction(botAI); }
    static Action* hor_escape_tank(PlayerbotAI* botAI) { return new HorEscapeTankAction(botAI); }
    static Action* hor_escape_follow(PlayerbotAI* botAI) { return new HorEscapeFollowAction(botAI); }
};

#endif
