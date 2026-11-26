/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_HORTRIGGERCONTEXT_H
#define PLAYERBOTS_HORTRIGGERCONTEXT_H

#include "HoRTriggers.h"
#include "NamedObjectContext.h"

class WotlkDungeonHoRTriggerContext : public NamedObjectContext<Trigger>
{
public:
    WotlkDungeonHoRTriggerContext()
    {
        creators["hor escape phase"] = &WotlkDungeonHoRTriggerContext::hor_escape_phase;
    }

private:
    static Trigger* hor_escape_phase(PlayerbotAI* botAI) { return new HorEscapeTrigger(botAI); }
};

#endif
