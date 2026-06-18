#ifndef _PLAYERBOT_RAIDAQ40MULTIPLIERS_H
#define _PLAYERBOT_RAIDAQ40MULTIPLIERS_H

#include "Multiplier.h"

class Aq40GenericMultiplier : public Multiplier
{
public:
    Aq40GenericMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "aq40 generic") {}
    float GetValue(Action* action) override;
};

class Aq40SkeramMultiplier : public Multiplier
{
public:
    Aq40SkeramMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "aq40 skeram") {}
    float GetValue(Action* action) override;
};

class Aq40BugTrioMultiplier : public Multiplier
{
public:
    Aq40BugTrioMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "aq40 bug trio") {}
    float GetValue(Action* action) override;
};

class Aq40SarturaMultiplier : public Multiplier
{
public:
    Aq40SarturaMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "aq40 sartura") {}
    float GetValue(Action* action) override;
};

class Aq40FankrissMultiplier : public Multiplier
{
public:
    Aq40FankrissMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "aq40 fankriss") {}
    float GetValue(Action* action) override;
};

class Aq40HuhuranMultiplier : public Multiplier
{
public:
    Aq40HuhuranMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "aq40 huhuran") {}
    float GetValue(Action* action) override;
};

class Aq40TwinMultiplier : public Multiplier
{
public:
    Aq40TwinMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "aq40 twin") {}
    float GetValue(Action* action) override;
};

class Aq40OuroMultiplier : public Multiplier
{
public:
    Aq40OuroMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "aq40 ouro") {}
    float GetValue(Action* action) override;
};


class Aq40ViscidusMultiplier : public Multiplier
{
public:
    Aq40ViscidusMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "aq40 viscidus") {}
    float GetValue(Action* action) override;
};

class Aq40CthunMultiplier : public Multiplier
{
public:
    Aq40CthunMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "aq40 cthun") {}
    float GetValue(Action* action) override;
};

#endif
