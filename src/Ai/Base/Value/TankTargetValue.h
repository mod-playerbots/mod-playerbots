/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef PLAYERBOTS_TANKTARGETVALUE_H
#define PLAYERBOTS_TANKTARGETVALUE_H

#include "RtiTargetValue.h"

class PlayerbotAI;

class TankTargetValue : public RtiTargetValue
{
public:
    TankTargetValue(PlayerbotAI* botAI, std::string const name = "tank target") : RtiTargetValue(botAI, "rti", name) {}

    Unit* Calculate() override;
};

#endif
