#ifndef PLAYERBOTS_MECHANARMULTIPLIERS_H
#define PLAYERBOTS_MECHANARMULTIPLIERS_H

#include "Multiplier.h"

class SepethreaKiteFlameMultiplier : public Multiplier
{
public:
    SepethreaKiteFlameMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "sepethrea kite flame") {}
    float GetValue(Action* action) override;
};

class SepethreaTankFocusMultiplier : public Multiplier
{
public:
    SepethreaTankFocusMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "sepethrea tank focus") {}
    float GetValue(Action* action) override;
};

#endif
