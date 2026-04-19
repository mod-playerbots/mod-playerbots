#ifndef _PLAYERBOT_TBCDUNGEONAUCHENAICRYPTSMULTIPLIERS_H
#define _PLAYERBOT_TBCDUNGEONAUCHENAICRYPTSMULTIPLIERS_H

#include "Multiplier.h"

class ShirrakFleeFocusFireMultiplier : public Multiplier
{
public:
    ShirrakFleeFocusFireMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "shirrak flee focus fire") {}
    float GetValue(Action* action) override;
};

#endif
