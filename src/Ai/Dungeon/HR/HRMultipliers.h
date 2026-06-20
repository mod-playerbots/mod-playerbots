#ifndef _PLAYERBOT_TBCDUNGEONHELLFIRERAMPARTSMULTIPLIERS_H
#define _PLAYERBOT_TBCDUNGEONHELLFIRERAMPARTSMULTIPLIERS_H

#include "Multiplier.h"

class OmorTreacherousAuraFleeFromPlayersMultiplier : public Multiplier
{
public:
    OmorTreacherousAuraFleeFromPlayersMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "omor treacherous aura flee from players") {}
    float GetValue(Action* action) override;
};

class OmorBaneOfTreacheryAuraFleeFromPlayersMultiplier : public Multiplier
{
public:
    OmorBaneOfTreacheryAuraFleeFromPlayersMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "omor bane of treachery aura flee from players") {}
    float GetValue(Action* action) override;
};

#endif
