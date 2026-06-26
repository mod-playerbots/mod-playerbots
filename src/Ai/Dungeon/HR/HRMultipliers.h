/*
* This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
* information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
* or (at your option) any later version.
*/

#ifndef _PLAYERBOT_TBCDUNGEONHELLFIRERAMPARTSMULTIPLIERS_H
#define _PLAYERBOT_TBCDUNGEONHELLFIRERAMPARTSMULTIPLIERS_H

#include "Multiplier.h"

class OmorTreacheryAuraFleeFromPlayersMultiplier : public Multiplier
{
public:
    OmorTreacheryAuraFleeFromPlayersMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "omor treachery aura flee from players") {}
    float GetValue(Action* action) override;
};

class OmorTreacheryAuraFleeFromTankMultiplier : public Multiplier
{
public:
    OmorTreacheryAuraFleeFromTankMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "omor treachery aura flee from tank") {}
    float GetValue(Action* action) override;
};

#endif
