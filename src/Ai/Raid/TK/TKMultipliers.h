/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_TKMULTIPLIERS_H
#define PLAYERBOTS_TKMULTIPLIERS_H

#include "Multiplier.h"

// Al'ar <Phoenix God>

class AlarSuppressGapClosersMultiplier : public Multiplier
{
public:
    AlarSuppressGapClosersMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "al'ar suppress gap closers") {}
    float GetValue(Action* action) override;
};

class AlarControlMovementMultiplier : public Multiplier
{
public:
    AlarControlMovementMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "al'ar control movement") {}
    float GetValue(Action* action) override;
};

class AlarDisableAutomaticTargetingMultiplier : public Multiplier
{
public:
    AlarDisableAutomaticTargetingMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "al'ar disable automatic targeting") {}
    float GetValue(Action* action) override;
};

class AlarStayAwayFromRebirthMultiplier : public Multiplier
{
public:
    AlarStayAwayFromRebirthMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "al'ar stay away from rebirth") {}
    float GetValue(Action* action) override;
};

class AlarControlTauntingMultiplier : public Multiplier
{
public:
    AlarControlTauntingMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "al'ar control taunting") {}
    float GetValue(Action* action) override;
};

// Void Reaver

class VoidReaverMaintainPositionsMultiplier : public Multiplier
{
public:
    VoidReaverMaintainPositionsMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "void reaver maintain positions") {}
    float GetValue(Action* action) override;
};

// High Astromancer Solarian

class HighAstromancerSolarianDisableMeleeTargetingMultiplier : public Multiplier
{
public:
    HighAstromancerSolarianDisableMeleeTargetingMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "high astromancer solarian disable melee targeting") {}
    float GetValue(Action* action) override;
};

class HighAstromancerSolarianWrathStayAwayMultiplier : public Multiplier
{
public:
    HighAstromancerSolarianWrathStayAwayMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "high astromancer solarian wrath stay away") {}
    float GetValue(Action* action) override;
};

// Kael'thas Sunstrider <Lord of the Blood Elves>

class KaelthasSunstriderWaitForDpsMultiplier : public Multiplier
{
public:
    KaelthasSunstriderWaitForDpsMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "kael'thas sunstrider wait for dps") {}
    float GetValue(Action* action) override;
};

class KaelthasSunstriderKiteThaladredMultiplier : public Multiplier
{
public:
    KaelthasSunstriderKiteThaladredMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "kael'thas sunstrider kiting thaladred") {}
    float GetValue(Action* action) override;
};

class KaelthasSunstriderControlMisdirectionMultiplier : public Multiplier
{
public:
    KaelthasSunstriderControlMisdirectionMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "kael'thas sunstrider control misdirection") {}
    float GetValue(Action* action) override;
};

class KaelthasSunstriderDisableWarlockTankSoulshatterMultiplier : public Multiplier
{
public:
    KaelthasSunstriderDisableWarlockTankSoulshatterMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "kael'thas sunstrider disable warlock tank soulshatter") {}
    float GetValue(Action* action) override;
};

class KaelthasSunstriderKeepDistanceFromCapernianMultiplier : public Multiplier
{
public:
    KaelthasSunstriderKeepDistanceFromCapernianMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "kael'thas sunstrider keep distance from capernian") {}
    float GetValue(Action* action) override;
};

class KaelthasSunstriderManageWeaponTankingMultiplier : public Multiplier
{
public:
    KaelthasSunstriderManageWeaponTankingMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "kael'thas sunstrider manage weapon tanking") {}
    float GetValue(Action* action) override;
};

class KaelthasSunstriderSuppressEquipUpgradeMultiplier : public Multiplier
{
public:
    KaelthasSunstriderSuppressEquipUpgradeMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "kael'thas sunstrider suppress equip upgrade") {}
    float GetValue(Action* action) override;
};

class KaelthasSunstriderManageAutomaticTargetingMultiplier : public Multiplier
{
public:
    KaelthasSunstriderManageAutomaticTargetingMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "kael'thas sunstrider manage automatic targeting") {}
    float GetValue(Action* action) override;
};

class KaelthasSunstriderDisableDisperseMultiplier : public Multiplier
{
public:
    KaelthasSunstriderDisableDisperseMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "kael'thas sunstrider disable disperse") {}
    float GetValue(Action* action) override;
};

class KaelthasSunstriderPrepareForPhase3Multiplier : public Multiplier
{
public:
    KaelthasSunstriderPrepareForPhase3Multiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "kael'thas sunstrider prepare for phase 3") {}
    float GetValue(Action* action) override;
};

class KaelthasSunstriderDelayCooldownsMultiplier : public Multiplier
{
public:
    KaelthasSunstriderDelayCooldownsMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "kael'thas sunstrider delay cooldowns") {}
    float GetValue(Action* action) override;
};

class KaelthasSunstriderStaySpreadDuringGravityLapseMultiplier : public Multiplier
{
public:
    KaelthasSunstriderStaySpreadDuringGravityLapseMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "kael'thas sunstrider stay spread during gravity lapse") {}
    float GetValue(Action* action) override;
};

#endif
