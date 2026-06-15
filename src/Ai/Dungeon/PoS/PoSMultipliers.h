#ifndef PLAYERBOTS_POSMULTIPLIERS_H
#define PLAYERBOTS_POSMULTIPLIERS_H

#include "Multiplier.h"

class IckAndKrickMultiplier : public Multiplier
{
    public:
    IckAndKrickMultiplier(PlayerbotAI* ai) : Multiplier(ai, "ick and krick") {}

    public:
        virtual float GetValue(Action* action);
};

class GarfrostMultiplier : public Multiplier
{
public:
    GarfrostMultiplier(PlayerbotAI* ai) : Multiplier(ai, "garfrost") { }

    float GetValue(Action* action) override;
};

#endif
