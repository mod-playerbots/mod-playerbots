#ifndef PLAYERBOTS_MECHMULTIPLIERS_H
#define PLAYERBOTS_MECHMULTIPLIERS_H

#include "Multiplier.h"

class SepethreaKiteFlameMultiplier : public Multiplier
{
public:
    SepethreaKiteFlameMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "sepethrea kite flame") {}
    float GetValue(Action* action) override;
};

class SepethreaFocusBossMultiplier : public Multiplier
{
public:
    SepethreaFocusBossMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "sepethrea focus boss") {}
    float GetValue(Action* action) override;
};

#endif
