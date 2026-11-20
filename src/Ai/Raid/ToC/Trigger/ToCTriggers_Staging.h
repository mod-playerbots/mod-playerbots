/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RAID_TOCTRIGGERS_STAGING_H
#define PLAYERBOTS_RAID_TOCTRIGGERS_STAGING_H

#include "Trigger.h"

// Raid-wide, so its gate never closes: the window is its own gate
class ToCStagingStandTrigger : public Trigger
{
public:
    static constexpr char const* Name = "toc staging stand";

    ToCStagingStandTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

#endif
