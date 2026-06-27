
#ifndef PLAYERBOTS_EOEMULTIPLIERS_H
#define PLAYERBOTS_EOEMULTIPLIERS_H

#include "Multiplier.h"

class MalygosMultiplier : public Multiplier
{
public:
    MalygosMultiplier(PlayerbotAI* ai) : Multiplier(ai, "malygos") {}

public:
    virtual float GetValue(Action* action);
};

#endif
