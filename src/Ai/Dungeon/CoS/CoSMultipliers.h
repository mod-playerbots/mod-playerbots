#ifndef PLAYERBOTS_COSMULTIPLIERS_H
#define PLAYERBOTS_COSMULTIPLIERS_H

#include "Multiplier.h"

class EpochMultiplier : public Multiplier
{
    public:
        EpochMultiplier(PlayerbotAI* ai) : Multiplier(ai, "chrono-lord epoch") {}

    public:
        virtual float GetValue(Action* action);
};

#endif
