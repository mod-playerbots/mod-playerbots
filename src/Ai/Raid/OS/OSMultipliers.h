
#ifndef _PLAYERBOT_OSMULTIPLIERS_H
#define _PLAYERBOT_OSMULTIPLIERS_H

#include "Multiplier.h"

class SartharionMultiplier : public Multiplier
{
public:
    SartharionMultiplier(PlayerbotAI* ai) : Multiplier(ai, "sartharion") {}

public:
    virtual float GetValue(Action* action);
};

#endif
