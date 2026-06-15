#ifndef _PLAYERBOT_ANMULTIPLIERS_H
#define _PLAYERBOT_ANMULTIPLIERS_H

#include "Multiplier.h"

class KrikthirMultiplier : public Multiplier
{
    public:
        KrikthirMultiplier(PlayerbotAI* ai) : Multiplier(ai, "krik'thir the gatewatcher") {}

    public:
        virtual float GetValue(Action* action);
};

#endif
