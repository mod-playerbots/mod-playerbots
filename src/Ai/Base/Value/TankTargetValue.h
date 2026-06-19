/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef PLAYERBOTS_TANKTARGETVALUE_H
#define PLAYERBOTS_TANKTARGETVALUE_H

#include "TargetValue.h"

class PlayerbotAI;

class TankTargetValue : public TargetValue
{
public:
    TankTargetValue(PlayerbotAI* botAI, std::string const name = "tank target") : TargetValue(botAI, name) {}

    Unit* Calculate() override;
};

#endif
