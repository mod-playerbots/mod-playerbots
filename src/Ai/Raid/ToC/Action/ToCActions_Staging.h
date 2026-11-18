/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RAID_TOCACTIONS_STAGING_H
#define PLAYERBOTS_RAID_TOCACTIONS_STAGING_H

#include <string>

#include "NamedObjectContext.h"
#include "ToCActions_NorthrendBeasts.h"

// Out of combat: walks only, never attacks, or the raid pulls itself
class ToCMoveToStagingStandAction : public NorthrendBeastsWalkAction
{
public:
    ToCMoveToStagingStandAction(PlayerbotAI* botAI, std::string const name = "toc move to staging stand")
        : NorthrendBeastsWalkAction(botAI, name) {}
    bool Execute(Event event) override;
};

class ToCStagingActionContext : public NamedObjectContext<Action>
{
public:
    ToCStagingActionContext()
    {
        creators["toc move to staging stand"] = &ToCStagingActionContext::toc_move_to_staging_stand;
    }

private:
    static Action* toc_move_to_staging_stand(PlayerbotAI* botAI) { return new ToCMoveToStagingStandAction(botAI); }
};

#endif
