#ifndef PLAYERBOTS_MECHACTIONCONTEXT_H
#define PLAYERBOTS_MECHACTIONCONTEXT_H

#include "Action.h"
#include "MechActions.h"
#include "NamedObjectContext.h"

class TbcDungeonMechanarActionContext : public NamedObjectContext<Action>
{
public:
    TbcDungeonMechanarActionContext()
    {
        creators["sepethrea kite flame"] =
            &TbcDungeonMechanarActionContext::sepethrea_kite_flame;
        creators["sepethrea avoid flame"] =
            &TbcDungeonMechanarActionContext::sepethrea_avoid_flame;
        creators["sepethrea avoid trail"] =
            &TbcDungeonMechanarActionContext::sepethrea_avoid_trail;
        creators["sepethrea focus boss"] =
            &TbcDungeonMechanarActionContext::sepethrea_focus_boss;
    }

private:
    static Action* sepethrea_kite_flame(PlayerbotAI* botAI) { return new SepethreaKiteFlameAction(botAI); }
    static Action* sepethrea_avoid_flame(PlayerbotAI* botAI) { return new SepethreaAvoidFlameAction(botAI); }
    static Action* sepethrea_avoid_trail(PlayerbotAI* botAI) { return new SepethreaAvoidTrailAction(botAI); }
    static Action* sepethrea_focus_boss(PlayerbotAI* botAI) { return new SepethreaFocusBossAction(botAI); }
};

#endif
