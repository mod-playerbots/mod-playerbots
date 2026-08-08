/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_TKMULTIPLIERS_H
#define PLAYERBOTS_TKMULTIPLIERS_H

#include "Multiplier.h"

// Al'ar <Phoenix God>

class AlarMoveBetweenPlatformsMultiplier : public Multiplier
{
public:
    AlarMoveBetweenPlatformsMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "al'ar move between platforms multiplier") {}
    float GetValue(Action* action) override;
};

class AlarDisableDisperseMultiplier : public Multiplier
{
public:
    AlarDisableDisperseMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "al'ar disable disperse multiplier") {}
    float GetValue(Action* action) override;
};

class AlarDisableTankAssistMultiplier : public Multiplier
{
public:
    AlarDisableTankAssistMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "al'ar disable tank assist multiplier") {}
    float GetValue(Action* action) override;
};

class AlarStayAwayFromRebirthMultiplier : public Multiplier
{
public:
    AlarStayAwayFromRebirthMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "al'ar stay away from rebirth multiplier") {}
    float GetValue(Action* action) override;
};

class AlarPhase2NoTankingIfArmorMeltedMultiplier : public Multiplier
{
public:
    AlarPhase2NoTankingIfArmorMeltedMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "al'ar phase 2 no tanking if armor melted multiplier") {}
    float GetValue(Action* action) override;
};

// Void Reaver

class VoidReaverMaintainPositionsMultiplier : public Multiplier
{
public:
    VoidReaverMaintainPositionsMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "void reaver maintain positions multiplier") {}
    float GetValue(Action* action) override;
};

// High Astromancer Solarian

class HighAstromancerSolarianDisableTankAssistMultiplier : public Multiplier
{
public:
    HighAstromancerSolarianDisableTankAssistMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "high astromancer solarian disable tank assist multiplier") {}
    float GetValue(Action* action) override;
};

class HighAstromancerSolarianMaintainPositionMultiplier : public Multiplier
{
public:
    HighAstromancerSolarianMaintainPositionMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "high astromancer solarian maintain position multiplier") {}
    float GetValue(Action* action) override;
};

// Kael'thas Sunstrider <Lord of the Blood Elves>

class KaelthasSunstriderWaitForDpsMultiplier : public Multiplier
{
public:
    KaelthasSunstriderWaitForDpsMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "kael'thas sunstrider wait for dps multiplier") {}
    float GetValue(Action* action) override;
};

class KaelthasSunstriderKiteThaladredMultiplier : public Multiplier
{
public:
    KaelthasSunstriderKiteThaladredMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "kael'thas sunstrider kite thaladred multiplier") {}
    float GetValue(Action* action) override;
};

class KaelthasSunstriderControlMisdirectionMultiplier : public Multiplier
{
public:
    KaelthasSunstriderControlMisdirectionMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "kael'thas sunstrider control misdirection multiplier") {}
    float GetValue(Action* action) override;
};

class KaelthasSunstriderKeepDistanceFromCapernianMultiplier : public Multiplier
{
public:
    KaelthasSunstriderKeepDistanceFromCapernianMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "kael'thas sunstrider keep distance from capernian multiplier") {}
    float GetValue(Action* action) override;
};

class KaelthasSunstriderManageWeaponTankingMultiplier : public Multiplier
{
public:
    KaelthasSunstriderManageWeaponTankingMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "kael'thas sunstrider manage weapon tanking multiplier") {}
    float GetValue(Action* action) override;
};

class KaelthasSunstriderDisableAdvisorTankAssistMultiplier : public Multiplier
{
public:
    KaelthasSunstriderDisableAdvisorTankAssistMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "kael'thas sunstrider disable advisor tank assist multiplier") {}
    float GetValue(Action* action) override;
};

class KaelthasSunstriderDisableDisperseMultiplier : public Multiplier
{
public:
    KaelthasSunstriderDisableDisperseMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "kael'thas sunstrider disable disperse multiplier") {}
    float GetValue(Action* action) override;
};

class KaelthasSunstriderDelayCooldownsMultiplier : public Multiplier
{
public:
    KaelthasSunstriderDelayCooldownsMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "kael'thas sunstrider delay cooldowns multiplier") {}
    float GetValue(Action* action) override;
};

class KaelthasSunstriderStaySpreadDuringGravityLapseMultiplier : public Multiplier
{
public:
    KaelthasSunstriderStaySpreadDuringGravityLapseMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "kael'thas sunstrider stay spread during gravity lapse multiplier") {}
    float GetValue(Action* action) override;
};

#endif
