/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_GRUULMULTIPLIERS_H
#define PLAYERBOTS_GRUULMULTIPLIERS_H

#include "Multiplier.h"

class GruulsLairDelayDpsCooldownsMultiplier : public Multiplier
{
public:
    GruulsLairDelayDpsCooldownsMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "gruul's lair delay dps cooldowns") {}
    float GetValue(Action* action) override;
};

class HighKingMaulgarControlTankActionsMultiplier : public Multiplier
{
public:
    HighKingMaulgarControlTankActionsMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "high king maulgar control tank actions") {}
    float GetValue(Action* action) override;
};

class HighKingMaulgarRestrictTauntingMultiplier : public Multiplier
{
public:
    HighKingMaulgarRestrictTauntingMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "high king maulgar restrict taunting") {}
    float GetValue(Action* action) override;
};

class HighKingMaulgarDisableDpsAssistMultiplier : public Multiplier
{
public:
    HighKingMaulgarDisableDpsAssistMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "high king maulgar disable dps assist") {}
    float GetValue(Action* action) override;
};

class HighKingMaulgarAvoidWhirlwindMultiplier : public Multiplier
{
public:
    HighKingMaulgarAvoidWhirlwindMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "high king maulgar avoid whirlwind") {}
    float GetValue(Action* action) override;
};

class HighKingMaulgarControlHunterActionsMultiplier : public Multiplier
{
public:
    HighKingMaulgarControlHunterActionsMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "high king maulgar control hunter actions") {}
    float GetValue(Action* action) override;
};

class HighKingMaulgarControlMageTankActionsMultiplier : public Multiplier
{
public:
    HighKingMaulgarControlMageTankActionsMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "high king maulgar control mage tank actions") {}
    float GetValue(Action* action) override;
};

class GruulTheDragonkillerControlTankMovementMultiplier : public Multiplier
{
public:
    GruulTheDragonkillerControlTankMovementMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "gruul the dragonkiller control tank movement") {}
    float GetValue(Action* action) override;
};

class GruulTheDragonkillerStaySpreadForShatterMultiplier : public Multiplier
{
public:
    GruulTheDragonkillerStaySpreadForShatterMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "gruul the dragonkiller stay spread for shatter") {}
    float GetValue(Action* action) override;
};

class GruulTheDragonkillerHoldWhileSnaredMultiplier : public Multiplier
{
public:
    GruulTheDragonkillerHoldWhileSnaredMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "gruul the dragonkiller hold while snared") {}
    float GetValue(Action* action) override;
};

#endif
