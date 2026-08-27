/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_SWPMULTIPLIERS_H
#define PLAYERBOTS_SWPMULTIPLIERS_H

#include "Multiplier.h"

// Kalecgos

class KalecgosControlMisdirectionMultiplier : public Multiplier
{
public:
    KalecgosControlMisdirectionMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "kalecgos control misdirection") {}
    float GetValue(Action* action) override;
};

class KalecgosWaitToDecurseMultiplier : public Multiplier
{
public:
    KalecgosWaitToDecurseMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "kalecgos wait to decurse") {}
    float GetValue(Action* action) override;
};

class KalecgosControlMovementMultiplier : public Multiplier
{
public:
    KalecgosControlMovementMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "kalecgos control movement") {}
    float GetValue(Action* action) override;
};

class KalecgosRestrictTauntMultiplier : public Multiplier
{
public:
    KalecgosRestrictTauntMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "kalecgos restrict taunt") {}
    float GetValue(Action* action) override;
};

class KalecgosSuppressAssistTankPullThreatMultiplier : public Multiplier
{
public:
    KalecgosSuppressAssistTankPullThreatMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "kalecgos suppress assist tank pull threat") {}
    float GetValue(Action* action) override;
};

class KalecgosEnterSpectralRiftMultiplier : public Multiplier
{
public:
    KalecgosEnterSpectralRiftMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "kalecgos enter spectral rift") {}
    float GetValue(Action* action) override;
};

class KalecgosDelayCooldownsForSathrovarrMultiplier : public Multiplier
{
public:
    KalecgosDelayCooldownsForSathrovarrMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "kalecgos delay cooldowns for sathrovarr") {}
    float GetValue(Action* action) override;
};

// Brutallus

class BrutallusControlMisdirectionMultiplier : public Multiplier
{
public:
    BrutallusControlMisdirectionMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "brutallus control misdirection") {}
    float GetValue(Action* action) override;
};

class BrutallusControlMovementMultiplier : public Multiplier
{
public:
    BrutallusControlMovementMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "brutallus control movement") {}
    float GetValue(Action* action) override;
};

class BrutallusNoKillingSpreeWhenNearbyBurnMultiplier : public Multiplier
{
public:
    BrutallusNoKillingSpreeWhenNearbyBurnMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "brutallus no killing spree when nearby burn") {}
    float GetValue(Action* action) override;
};

class BrutallusRestrictTauntMultiplier : public Multiplier
{
public:
    BrutallusRestrictTauntMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "brutallus restrict taunt") {}
    float GetValue(Action* action) override;
};

class BrutallusDelayCooldownsMultiplier : public Multiplier
{
public:
    BrutallusDelayCooldownsMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "brutallus delay cooldowns") {}
    float GetValue(Action* action) override;
};

// Felmyst

class FelmystControlMovementMultiplier : public Multiplier
{
public:
    FelmystControlMovementMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "felmyst control movement") {}
    float GetValue(Action* action) override;
};

class FelmystWaitForLandingDpsMultiplier : public Multiplier
{
public:
    FelmystWaitForLandingDpsMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "felmyst wait for landing dps") {}
    float GetValue(Action* action) override;
};

class FelmystPrioritizeEncapsulateAvoidanceMultiplier : public Multiplier
{
public:
    FelmystPrioritizeEncapsulateAvoidanceMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "felmyst prioritize encapsulate avoidance") {}
    float GetValue(Action* action) override;
};

class FelmystPrioritizeFogAvoidanceMultiplier : public Multiplier
{
public:
    FelmystPrioritizeFogAvoidanceMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "felmyst prioritize fog avoidance") {}
    float GetValue(Action* action) override;
};

class FelmystPrioritizeDemonicVaporAvoidanceMultiplier : public Multiplier
{
public:
    FelmystPrioritizeDemonicVaporAvoidanceMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "felmyst prioritize demonic vapor avoidance") {}
    float GetValue(Action* action) override;
};

class FelmystFocusAttacksOnCharmedPlayerMultiplier : public Multiplier
{
public:
    FelmystFocusAttacksOnCharmedPlayerMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "felmyst focus attacks on charmed player") {}
    float GetValue(Action* action) override;
};

class FelmystDontDotAddsMultiplier : public Multiplier
{
public:
    FelmystDontDotAddsMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "felmyst don't dot adds") {}
    float GetValue(Action* action) override;
};

class FelmystDelayCooldownsMultiplier : public Multiplier
{
public:
    FelmystDelayCooldownsMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "felmyst delay cooldowns") {}
    float GetValue(Action* action) override;
};

// Eredar Twins

class EredarTwinsDisableAutomaticTargetingMultiplier : public Multiplier
{
public:
    EredarTwinsDisableAutomaticTargetingMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "eredar twins disable automatic targeting") {}
    float GetValue(Action* action) override;
};

class EredarTwinsControlMisdirectionMultiplier : public Multiplier
{
public:
    EredarTwinsControlMisdirectionMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "eredar twins misdirect bosses to tanks") {}
    float GetValue(Action* action) override;
};

class EredarTwinsHoldDpsAtStartMultiplier : public Multiplier
{
public:
    EredarTwinsHoldDpsAtStartMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "eredar twins hold dps at start") {}
    float GetValue(Action* action) override;
};

class EredarTwinsControlThreatMultiplier : public Multiplier
{
public:
    EredarTwinsControlThreatMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "eredar twins control threat") {}
    float GetValue(Action* action) override;
};

class EredarTwinsControlMovementMultiplier : public Multiplier
{
public:
    EredarTwinsControlMovementMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "eredar twins control movement") {}
    float GetValue(Action* action) override;
};

class EredarTwinsIsolateConflagrationMultiplier : public Multiplier
{
public:
    EredarTwinsIsolateConflagrationMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "eredar twins isolate conflagration") {}
    float GetValue(Action* action) override;
};

class EredarTwinsDelayCooldownsMultiplier : public Multiplier
{
public:
    EredarTwinsDelayCooldownsMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "eredar twins delay cooldowns") {}
    float GetValue(Action* action) override;
};

// M'uru

class MuruDisableDefaultTargetingMultiplier : public Multiplier
{
public:
    MuruDisableDefaultTargetingMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "m'uru disable default targeting") {}
    float GetValue(Action* action) override;
};

class MuruControlMisdirectionMultiplier : public Multiplier
{
public:
    MuruControlMisdirectionMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "m'uru control misdirection") {}
    float GetValue(Action* action) override;
};

class MuruControlMovementMultiplier : public Multiplier
{
public:
    MuruControlMovementMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "m'uru control movement") {}
    float GetValue(Action* action) override;
};

class MuruDelayCooldownsMultiplier : public Multiplier
{
public:
    MuruDelayCooldownsMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "m'uru delay cooldowns") {}
    float GetValue(Action* action) override;
};

// Kil'jaeden <The Deceiver>

class KiljaedenDelayCooldownsMultiplier : public Multiplier
{
public:
    KiljaedenDelayCooldownsMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "kil'jaeden delay cooldowns") {}
    float GetValue(Action* action) override;
};

class KiljaedenTanksFocusAssignedHandOnlyMultiplier : public Multiplier
{
public:
    KiljaedenTanksFocusAssignedHandOnlyMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "kil'jaeden tanks focus assigned hand only") {}
    float GetValue(Action* action) override;
};

class KiljaedenDpsFocusAssignedHandOnlyMultiplier : public Multiplier
{
public:
    KiljaedenDpsFocusAssignedHandOnlyMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "kil'jaeden dps focus assigned hand only") {}
    float GetValue(Action* action) override;
};

class KiljaedenControlMovementAndTargetingMultiplier : public Multiplier
{
public:
    KiljaedenControlMovementAndTargetingMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "kil'jaeden control movement and targeting") {}
    float GetValue(Action* action) override;
};

class KiljaedenPrioritizeDarknessProtectionMultiplier : public Multiplier
{
public:
    KiljaedenPrioritizeDarknessProtectionMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "kil'jaeden prioritize darkness protection") {}
    float GetValue(Action* action) override;
};

class KiljaedenControlDragonMultiplier : public Multiplier
{
public:
    KiljaedenControlDragonMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "kil'jaeden dragon buff and protect raid") {}
    float GetValue(Action* action) override;
};

#endif
