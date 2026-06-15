/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_UNLOCKITEMACTION_H
#define _PLAYERBOT_UNLOCKITEMACTION_H

#include "Action.h"

class PlayerbotAI;

class UnlockItemAction : public Action
{
public:
    UnlockItemAction(PlayerbotAI* botAI) : Action(botAI, "unlock item") { }

    bool Execute(Event event) override;

private:
    void UnlockItem(Item* item);
};

#endif
