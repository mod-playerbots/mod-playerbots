/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "GroupLeaderValue.h"

#include "Playerbots.h"

Unit* GroupLeaderValue::Calculate() { return botAI->GetGroupLeader(); }
