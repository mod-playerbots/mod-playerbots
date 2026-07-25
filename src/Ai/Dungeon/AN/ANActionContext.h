/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ANACTIONCONTEXT_H
#define PLAYERBOTS_ANACTIONCONTEXT_H

#include "ANActions.h"
#include "Action.h"
#include "NamedObjectContext.h"

class WotlkDungeonANActionContext : public NamedObjectContext<Action>
{
    public:
        WotlkDungeonANActionContext() {
            creators["attack web wrap"] = &WotlkDungeonANActionContext::attack_web_wrap;
            creators["krik'thir priority"] = &WotlkDungeonANActionContext::krikthir_priority;
            creators["dodge pound"] = &WotlkDungeonANActionContext::dodge_pound;
        }
    private:
        static Action* attack_web_wrap(PlayerbotAI* ai) { return new AttackWebWrapAction(ai); }
        static Action* krikthir_priority(PlayerbotAI* ai) { return new WatchersTargetAction(ai); }
        static Action* dodge_pound(PlayerbotAI* ai) { return new AnubarakDodgePoundAction(ai); }
};

#endif
