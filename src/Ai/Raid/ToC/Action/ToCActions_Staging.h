/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RAID_TOCACTIONS_STAGING_H
#define PLAYERBOTS_RAID_TOCACTIONS_STAGING_H

#include "ToCActions_NorthrendBeasts.h"

// Out of combat: walks only, never attacks, or the raid pulls itself
class ToCMoveToStagingStandAction : public NorthrendBeastsWalkAction
{
public:
    static constexpr char const* Name = "toc move to staging stand";

    ToCMoveToStagingStandAction(PlayerbotAI* botAI) : NorthrendBeastsWalkAction(botAI, Name) {}
    bool Execute(Event event) override;
};

#endif
