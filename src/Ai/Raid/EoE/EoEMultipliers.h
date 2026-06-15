
#ifndef _PLAYERBOT_EOEMULTIPLIERS_H
#define _PLAYERBOT_EOEMULTIPLIERS_H

#include "Multiplier.h"

class MalygosMultiplier : public Multiplier
{
public:
    MalygosMultiplier(PlayerbotAI* ai) : Multiplier(ai, "malygos") {}

public:
    virtual float GetValue(Action* action);
};

#endif
