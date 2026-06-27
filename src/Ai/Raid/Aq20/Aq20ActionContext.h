#ifndef PLAYERBOTS_AQ20ACTIONCONTEXT_H
#define PLAYERBOTS_AQ20ACTIONCONTEXT_H

#include "Action.h"
#include "NamedObjectContext.h"
#include "Aq20Actions.h"

class RaidAq20ActionContext : public NamedObjectContext<Action>
{
public:
    RaidAq20ActionContext()
    {
        creators["aq20 use crystal"] = &RaidAq20ActionContext::use_crystal;
    }

private:
    static Action* use_crystal(PlayerbotAI* ai) { return new Aq20UseCrystalAction(ai); }
};

#endif
