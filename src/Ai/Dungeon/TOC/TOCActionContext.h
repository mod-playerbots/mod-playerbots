/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_TOCACTIONCONTEXT_H
#define PLAYERBOTS_TOCACTIONCONTEXT_H

#include "Action.h"
#include "NamedObjectContext.h"
#include "TOCActions.h"

class WotlkDungeonToCActionContext : public NamedObjectContext<Action>
{
    public:
        WotlkDungeonToCActionContext() {
            creators["toc lance"] = &WotlkDungeonToCActionContext::toc_lance;
            creators["toc ue lance"] = &WotlkDungeonToCActionContext::toc_ue_lance;
            creators["toc mount"] = &WotlkDungeonToCActionContext::toc_mount;
            creators["toc mounted"] = &WotlkDungeonToCActionContext::toc_mounted;
            creators["toc eadric"] = &WotlkDungeonToCActionContext::toc_eadric;
        }
    private:
        static Action* toc_lance(PlayerbotAI* ai) { return new ToCLanceAction(ai); }
        static Action* toc_ue_lance(PlayerbotAI* ai) { return new ToCUELanceAction(ai); }
        static Action* toc_mount(PlayerbotAI* ai) { return new ToCMountAction(ai); }
        static Action* toc_mounted(PlayerbotAI* ai) { return new ToCMountedAction(ai); }
        static Action* toc_eadric(PlayerbotAI* ai) { return new ToCEadricAction(ai); }
};

#endif
