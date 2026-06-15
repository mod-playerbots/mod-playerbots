/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_UNLOCKTRADEDITEMACTION_H
#define _PLAYERBOT_UNLOCKTRADEDITEMACTION_H

#include "Action.h"

class PlayerbotAI;

class UnlockTradedItemAction : public Action
{
public:
    UnlockTradedItemAction(PlayerbotAI* botAI) : Action(botAI, "unlock traded item") {}

    bool Execute(Event event) override;

private:
    bool CanUnlockItem(Item* item);
    void UnlockItem(Item* item);
};

#endif
