#ifndef PLAYERBOTS_ACMULTIPLIERS_H
#define PLAYERBOTS_ACMULTIPLIERS_H

#include "Multiplier.h"

class ShirrakFleeFocusFireMultiplier : public Multiplier
{
public:
    ShirrakFleeFocusFireMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "shirrak flee focus fire") {}
    float GetValue(Action* action) override;
};

#endif
