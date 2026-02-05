/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "PlayerbotAI.h"
#include "WipeAction.h"

bool WipeAction::Execute(Event event)
{
    const Player* const owner = event.getOwner();
    const Player* const master = this->botAI->GetMaster();

    if (owner != nullptr && master != nullptr && master->GetGUID() != owner->GetGUID())
    {
        return false;
    }

    this->bot->Kill(this->bot, this->bot);

    return true;
}
