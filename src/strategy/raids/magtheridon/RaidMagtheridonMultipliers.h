#ifndef _PLAYERBOT_RAIDMAGTHERIDONMULTIPLIERS_H
#define _PLAYERBOT_RAIDMAGTHERIDONMULTIPLIERS_H

#include "Multiplier.h"

class MagtheridonUseManticronCubeMultiplier : public Multiplier
{
public:
    MagtheridonUseManticronCubeMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "magtheridon use manticron cube multiplier") {}
    float GetValue(Action* action) override;
};

class MagtheridonWaitToAttackMultiplier : public Multiplier
{
public:
    MagtheridonWaitToAttackMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "magtheridon wait to attack multiplier") {}
    float GetValue(Action* action) override;
};

class MagtheridonDisableOffTankAssistMultiplier : public Multiplier
{
public:
    MagtheridonDisableOffTankAssistMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "magtheridon disable off tank assist multiplier") {}
    float GetValue(Action* action) override;
};

#endif
