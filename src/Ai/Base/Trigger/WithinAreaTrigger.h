/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_WITHINAREATRIGGER_H
#define PLAYERBOTS_WITHINAREATRIGGER_H

#include "Trigger.h"

class PlayerbotAI;

struct AreaTrigger;

class WithinAreaTrigger : public Trigger
{
public:
    WithinAreaTrigger(PlayerbotAI* botAI) : Trigger(botAI, "within area trigger") {}

    bool IsActive() override;

private:
    bool IsPointInAreaTriggerZone(AreaTrigger const* atEntry, uint32 mapid, float x, float y, float z, float delta);
};

#endif
