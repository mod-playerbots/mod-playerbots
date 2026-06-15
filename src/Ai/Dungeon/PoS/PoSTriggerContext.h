/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_POSTRIGGERCONTEXT_H
#define _PLAYERBOT_POSTRIGGERCONTEXT_H

#include "NamedObjectContext.h"
#include "AiObjectContext.h"
#include "PoSTriggers.h"

class WotlkDungeonPoSTriggerContext : public NamedObjectContext<Trigger>
{
public:
    WotlkDungeonPoSTriggerContext()
    {
        creators["ick and krick"] = &WotlkDungeonPoSTriggerContext::ick_and_krick;
        creators["tyrannus"] = &WotlkDungeonPoSTriggerContext::tyrannus;
    }

private:
    static Trigger* ick_and_krick(PlayerbotAI* ai) { return new IckAndKrickTrigger(ai); }
    static Trigger* tyrannus(PlayerbotAI* ai) { return new TyrannusTrigger(ai); }
};

#endif
