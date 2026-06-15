#ifndef PLAYERBOTS_HOSMULTIPLIERS_H
#define PLAYERBOTS_HOSMULTIPLIERS_H

#include "Multiplier.h"

class KrystallusMultiplier : public Multiplier
{
    public:
        KrystallusMultiplier(PlayerbotAI* ai) : Multiplier(ai, "krystallus") {}

    public:
        virtual float GetValue(Action* action);
};

class SjonnirMultiplier : public Multiplier
{
    public:
        SjonnirMultiplier(PlayerbotAI* ai) : Multiplier(ai, "sjonnir the ironshaper") {}

    public:
        virtual float GetValue(Action* action);
};

#endif
