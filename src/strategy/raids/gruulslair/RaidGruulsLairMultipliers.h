#ifndef _PLAYERBOT_RAIDGRUULSLAIRMULTIPLIERS_H
#define _PLAYERBOT_RAIDGRUULSLAIRMULTIPLIERS_H

#include "Multiplier.h"

class HighKingMaulgarDisableTankAssistMultiplier : public Multiplier
{
public:
    HighKingMaulgarDisableTankAssistMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "high king maulgar disable tank assist multiplier") {}
    float GetValue(Action* action) override;
};

class HighKingMaulgarAvoidWhirlwindMultiplier : public Multiplier
{
public:
    HighKingMaulgarAvoidWhirlwindMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "high king maulgar avoid whirlwind multiplier") {}
    float GetValue(Action* action) override;
};

class HighKingMaulgarDisableArcaneShotOnKroshMultiplier : public Multiplier
{
public:
    HighKingMaulgarDisableArcaneShotOnKroshMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "high king maulgar disable arcane shot on krosh multiplier") {}
    float GetValue(Action* action) override;
};

class HighKingMaulgarDisableMageTankAOEMultiplier : public Multiplier
{
public:
    HighKingMaulgarDisableMageTankAOEMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "high king maulgar disable mage tank aoe multiplier") {}
    float GetValue(Action* action) override;
};

class GruulTheDragonkillerMainTankMovementMultiplier : public Multiplier
{
public:
    GruulTheDragonkillerMainTankMovementMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "gruul the dragonkiller main tank movement multiplier") {}
    float GetValue(Action* action) override;
};

class GruulTheDragonkillerGroundSlamMultiplier : public Multiplier
{
public:
    GruulTheDragonkillerGroundSlamMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "gruul the dragonkiller ground slam multiplier") {}
    float GetValue(Action* action) override;
};

#endif
