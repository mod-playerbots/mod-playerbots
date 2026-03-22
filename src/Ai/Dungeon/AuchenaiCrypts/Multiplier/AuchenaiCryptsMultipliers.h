#ifndef _PLAYERBOT_TBCDUNGEONAUCHENAICRYPTSMULTIPLIERS_H
#define _PLAYERBOT_TBCDUNGEONAUCHENAICRYPTSMULTIPLIERS_H

#include "Multiplier.h"

class FleeFocusFireMultiplier : public Multiplier
{
public:
    FleeFocusFireMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "flee focus fire") {}
    float GetValue(Action* action) override;
};

#endif
