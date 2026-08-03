/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_GRUULMULTIPLIERS_H
#define PLAYERBOTS_GRUULMULTIPLIERS_H

#include "Multiplier.h"

class HighKingMaulgarDelayBloodlustAndHeroismMultiplier : public Multiplier
{
public:
    HighKingMaulgarDelayBloodlustAndHeroismMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "high king maulgar delay bloodlust and heroism multiplier") {}
    float GetValue(Action* action) override;
};

class HighKingMaulgarControlTankActionsMultiplier : public Multiplier
{
public:
    HighKingMaulgarControlTankActionsMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "high king maulgar control tank actions multiplier") {}
    float GetValue(Action* action) override;
};

class HighKingMaulgarAvoidWhirlwindMultiplier : public Multiplier
{
public:
    HighKingMaulgarAvoidWhirlwindMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "high king maulgar avoid whirlwind multiplier") {}
    float GetValue(Action* action) override;
};

class HighKingMaulgarDisableArcaneShotOnKroshMultiplier : public Multiplier
{
public:
    HighKingMaulgarDisableArcaneShotOnKroshMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "high king maulgar disable arcane shot on krosh multiplier") {}
    float GetValue(Action* action) override;
};

class HighKingMaulgarDisableMageTankAoeMultiplier : public Multiplier
{
public:
    HighKingMaulgarDisableMageTankAoeMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "high king maulgar disable mage tank aoe multiplier") {}
    float GetValue(Action* action) override;
};

class GruulTheDragonkillerDelayBloodlustAndHeroismMultiplier : public Multiplier
{
public:
    GruulTheDragonkillerDelayBloodlustAndHeroismMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "gruul the dragonkiller delay bloodlust and heroism multiplier") {}
    float GetValue(Action* action) override;
};

class GruulTheDragonkillerControlTankMovementMultiplier : public Multiplier
{
public:
    GruulTheDragonkillerControlTankMovementMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "gruul the dragonkiller control tank movement multiplier") {}
    float GetValue(Action* action) override;
};

class GruulTheDragonkillerStaySpreadForShatterMultiplier : public Multiplier
{
public:
    GruulTheDragonkillerStaySpreadForShatterMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "gruul the dragonkiller stay spread for shatter multiplier") {}
    float GetValue(Action* action) override;
};

#endif
