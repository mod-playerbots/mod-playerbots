/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_HORTRIGGERS_H
#define PLAYERBOTS_HORTRIGGERS_H

#include "Trigger.h"

class PlayerbotAI;

// Master trigger for the Halls of Reflection escape event: active while the
// Lich King chases the group down the tunnel.
class HorEscapeTrigger : public Trigger
{
public:
    HorEscapeTrigger(PlayerbotAI* botAI) : Trigger(botAI, "hor escape phase", 2) {}
    bool IsActive() override;
};

#endif
