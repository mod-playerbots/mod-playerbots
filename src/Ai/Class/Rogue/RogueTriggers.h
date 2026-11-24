/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ROGUETRIGGERS_H
#define PLAYERBOTS_ROGUETRIGGERS_H

#include "GenericTriggers.h"

class PlayerbotAI;

class KickInterruptSpellTrigger : public InterruptSpellTrigger
{
public:
    KickInterruptSpellTrigger(PlayerbotAI* botAI) : InterruptSpellTrigger(botAI, "kick") {}
};

class SliceAndDiceTrigger : public BuffTrigger
{
public:
    SliceAndDiceTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "slice and dice") {}
};

class HungerForBloodTrigger : public BuffTrigger
{
public:
    // Was BuffTrigger(botAI, "hunger for blood"): only fired when absent, never refreshed early.
    // Now uses checkDuration + beforeDuration: refreshes with 10s remaining.
    // (HBF lasts 60s; 10s early is enough for two GCDs - Rupture then buff - and leaves Rupture coverage).
    HungerForBloodTrigger(PlayerbotAI* botAI)
        : BuffTrigger(botAI, "hunger for blood", 1, false, true, 10000)
    {
    }
};

// HBF expiring + target not bleeding + 1-3 combo points
// -> Apply a low-CP Rupture first. HBF requires a bleed on the target, but the assassination rotation itself does not apply Rupture,
//   so if there is no bleed when HBF is about to expire, Rupture must create one so the next tick can refresh HBF.
// Limited to 1-3 CP: high CP saved for Envenom, low-CP Rupture only serves to apply a bleed.
class RuptureForHungerTrigger : public Trigger
{
public:
    RuptureForHungerTrigger(PlayerbotAI* botAI) : Trigger(botAI, "rupture for hunger", 1) {}
    bool IsActive() override;
};

class AdrenalineRushTrigger : public BoostTrigger
{
public:
    AdrenalineRushTrigger(PlayerbotAI* botAI) : BoostTrigger(botAI, "adrenaline rush") {}

    // bool isPossible();
};

class BladeFlurryTrigger : public BoostTrigger
{
public:
    BladeFlurryTrigger(PlayerbotAI* botAI) : BoostTrigger(botAI, "blade flurry") {}
};

class RiposteAvailableTrigger : public SpellCanBeCastTrigger
{
public:
    RiposteAvailableTrigger(PlayerbotAI* botAI) : SpellCanBeCastTrigger(botAI, "riposte") {}
};

class RuptureTrigger : public DebuffTrigger
{
public:
    RuptureTrigger(PlayerbotAI* botAI) : DebuffTrigger(botAI, "rupture", 1, true) {}
};

class ExposeArmorTrigger : public DebuffTrigger
{
public:
    ExposeArmorTrigger(PlayerbotAI* botAI) : DebuffTrigger(botAI, "expose armor") {}
    virtual bool IsActive() override;
};

class KickInterruptEnemyHealerSpellTrigger : public InterruptEnemyHealerTrigger
{
public:
    KickInterruptEnemyHealerSpellTrigger(PlayerbotAI* botAI) : InterruptEnemyHealerTrigger(botAI, "kick") {}
};

class InStealthTrigger : public HasAuraTrigger
{
public:
    InStealthTrigger(PlayerbotAI* botAI) : HasAuraTrigger(botAI, "stealth") {}
};

class NoStealthTrigger : public HasNoAuraTrigger
{
public:
    NoStealthTrigger(PlayerbotAI* botAI) : HasNoAuraTrigger(botAI, "stealth") {}
};

class UnstealthTrigger : public BuffTrigger
{
public:
    UnstealthTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "stealth", 3) {}

    bool IsActive() override;
};

class StealthTrigger : public Trigger
{
public:
    StealthTrigger(PlayerbotAI* botAI) : Trigger(botAI, "stealth") {}

    bool IsActive() override;
};

class SapTrigger : public HasCcTargetTrigger
{
public:
    SapTrigger(PlayerbotAI* botAI) : HasCcTargetTrigger(botAI, "sap") {}

    bool IsPossible();
};

class SprintTrigger : public BuffTrigger
{
public:
    SprintTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "sprint", 3) {}

    bool IsPossible();
    bool IsActive() override;
};

class MainHandWeaponNoEnchantTrigger : public BuffTrigger
{
public:
    MainHandWeaponNoEnchantTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "main hand", 1) {}
    virtual bool IsActive();
};

class OffHandWeaponNoEnchantTrigger : public BuffTrigger
{
public:
    OffHandWeaponNoEnchantTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "off hand", 1) {}
    virtual bool IsActive();
};

class TricksOfTheTradeOnMainTankTrigger : public BuffOnMainTankTrigger
{
public:
    TricksOfTheTradeOnMainTankTrigger(PlayerbotAI* botAI) : BuffOnMainTankTrigger(botAI, "tricks of the trade", true) {}
};

#endif
