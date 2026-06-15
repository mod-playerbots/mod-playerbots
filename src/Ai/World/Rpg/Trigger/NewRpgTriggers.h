/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_NEWRPGTRIGGERS_H
#define _PLAYERBOT_NEWRPGTRIGGERS_H

#include "NewRpgStrategy.h"
#include "Trigger.h"

class NewRpgStatusTrigger : public Trigger
{
public:
    NewRpgStatusTrigger(PlayerbotAI* botAI, NewRpgStatus status = RPG_IDLE)
        : Trigger(botAI, "new rpg status"), status(status)
    {
    }
    bool IsActive() override;

protected:
    NewRpgStatus status;
};

#endif
