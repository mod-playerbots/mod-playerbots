/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_SWPMULTIPLIERS_H
#define PLAYERBOTS_SWPMULTIPLIERS_H

#include "EncounterHelpers.h"
#include "Multiplier.h"
#include "SWPShared.h"
#include <string>

// General

class SunwellPlateauEncounterMultiplier : public Multiplier
{
public:
    SunwellPlateauEncounterMultiplier(PlayerbotAI* botAI, std::string const name)
        : Multiplier(botAI, name) {}

    float GetValue(Action* action) final
    {
        return EncounterHelpers::IsEncounterInProgress(bot, SwpHelpers::SWP_MAP_ID)
            ? GetValueInEncounter(action) : 1.0f;
    }

protected:
    virtual float GetValueInEncounter(Action* action) = 0;
};

class SunwellPlateauNoEncounterDrinkingMultiplier : public Multiplier
{
public:
    SunwellPlateauNoEncounterDrinkingMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "sunwell plateau no encounter drinking") {}
    float GetValue(Action* action) override;
};

// Trash

class VolatileFiendRestrictApproachMultiplier : public Multiplier
{
public:
    VolatileFiendRestrictApproachMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "volatile fiend restrict approach") {}
    float GetValue(Action* action) override;
};

// Kalecgos

class KalecgosControlMisdirectionMultiplier : public SunwellPlateauEncounterMultiplier
{
public:
    KalecgosControlMisdirectionMultiplier(PlayerbotAI* botAI)
        : SunwellPlateauEncounterMultiplier(botAI, "kalecgos control misdirection") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class KalecgosWaitToDecurseMultiplier : public SunwellPlateauEncounterMultiplier
{
public:
    KalecgosWaitToDecurseMultiplier(PlayerbotAI* botAI)
        : SunwellPlateauEncounterMultiplier(botAI, "kalecgos wait to decurse") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class KalecgosControlMovementMultiplier : public SunwellPlateauEncounterMultiplier
{
public:
    KalecgosControlMovementMultiplier(PlayerbotAI* botAI)
        : SunwellPlateauEncounterMultiplier(botAI, "kalecgos control movement") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class KalecgosRestrictTauntMultiplier : public SunwellPlateauEncounterMultiplier
{
public:
    KalecgosRestrictTauntMultiplier(PlayerbotAI* botAI)
        : SunwellPlateauEncounterMultiplier(botAI, "kalecgos restrict taunt") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class KalecgosSuppressAssistTankPullThreatMultiplier : public SunwellPlateauEncounterMultiplier
{
public:
    KalecgosSuppressAssistTankPullThreatMultiplier(PlayerbotAI* botAI)
        : SunwellPlateauEncounterMultiplier(botAI, "kalecgos suppress assist tank pull threat") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class KalecgosEnterSpectralRiftMultiplier : public SunwellPlateauEncounterMultiplier
{
public:
    KalecgosEnterSpectralRiftMultiplier(PlayerbotAI* botAI)
        : SunwellPlateauEncounterMultiplier(botAI, "kalecgos enter spectral rift") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class KalecgosDelayCooldownsForSathrovarrMultiplier : public SunwellPlateauEncounterMultiplier
{
public:
    KalecgosDelayCooldownsForSathrovarrMultiplier(PlayerbotAI* botAI)
        : SunwellPlateauEncounterMultiplier(botAI, "kalecgos delay cooldowns for sathrovarr") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

// Brutallus

class BrutallusControlMisdirectionMultiplier : public SunwellPlateauEncounterMultiplier
{
public:
    BrutallusControlMisdirectionMultiplier(PlayerbotAI* botAI)
        : SunwellPlateauEncounterMultiplier(botAI, "brutallus control misdirection") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class BrutallusControlMovementMultiplier : public SunwellPlateauEncounterMultiplier
{
public:
    BrutallusControlMovementMultiplier(PlayerbotAI* botAI)
        : SunwellPlateauEncounterMultiplier(botAI, "brutallus control movement") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class BrutallusNoKillingSpreeWhenNearbyBurnMultiplier : public SunwellPlateauEncounterMultiplier
{
public:
    BrutallusNoKillingSpreeWhenNearbyBurnMultiplier(PlayerbotAI* botAI)
        : SunwellPlateauEncounterMultiplier(botAI, "brutallus no killing spree when nearby burn") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class BrutallusRestrictTauntMultiplier : public SunwellPlateauEncounterMultiplier
{
public:
    BrutallusRestrictTauntMultiplier(PlayerbotAI* botAI)
        : SunwellPlateauEncounterMultiplier(botAI, "brutallus restrict taunt") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class BrutallusDelayCooldownsMultiplier : public SunwellPlateauEncounterMultiplier
{
public:
    BrutallusDelayCooldownsMultiplier(PlayerbotAI* botAI)
        : SunwellPlateauEncounterMultiplier(botAI, "brutallus delay cooldowns") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

// Felmyst

class FelmystControlMovementMultiplier : public SunwellPlateauEncounterMultiplier
{
public:
    FelmystControlMovementMultiplier(PlayerbotAI* botAI)
        : SunwellPlateauEncounterMultiplier(botAI, "felmyst control movement") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class FelmystWaitForLandingDpsMultiplier : public SunwellPlateauEncounterMultiplier
{
public:
    FelmystWaitForLandingDpsMultiplier(PlayerbotAI* botAI)
        : SunwellPlateauEncounterMultiplier(botAI, "felmyst wait for landing dps") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class FelmystPrioritizeEncapsulateAvoidanceMultiplier : public SunwellPlateauEncounterMultiplier
{
public:
    FelmystPrioritizeEncapsulateAvoidanceMultiplier(PlayerbotAI* botAI)
        : SunwellPlateauEncounterMultiplier(botAI, "felmyst prioritize encapsulate avoidance") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class FelmystPrioritizeFogAvoidanceMultiplier : public SunwellPlateauEncounterMultiplier
{
public:
    FelmystPrioritizeFogAvoidanceMultiplier(PlayerbotAI* botAI)
        : SunwellPlateauEncounterMultiplier(botAI, "felmyst prioritize fog avoidance") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class FelmystPrioritizeDemonicVaporAvoidanceMultiplier : public SunwellPlateauEncounterMultiplier
{
public:
    FelmystPrioritizeDemonicVaporAvoidanceMultiplier(PlayerbotAI* botAI)
        : SunwellPlateauEncounterMultiplier(botAI, "felmyst prioritize demonic vapor avoidance") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class FelmystFocusAttacksOnCharmedPlayerMultiplier : public SunwellPlateauEncounterMultiplier
{
public:
    FelmystFocusAttacksOnCharmedPlayerMultiplier(PlayerbotAI* botAI)
        : SunwellPlateauEncounterMultiplier(botAI, "felmyst focus attacks on charmed player") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class FelmystDontDotAddsMultiplier : public SunwellPlateauEncounterMultiplier
{
public:
    FelmystDontDotAddsMultiplier(PlayerbotAI* botAI)
        : SunwellPlateauEncounterMultiplier(botAI, "felmyst don't dot adds") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class FelmystDelayCooldownsMultiplier : public SunwellPlateauEncounterMultiplier
{
public:
    FelmystDelayCooldownsMultiplier(PlayerbotAI* botAI)
        : SunwellPlateauEncounterMultiplier(botAI, "felmyst delay cooldowns") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

// Eredar Twins

class EredarTwinsDisableAutomaticTargetingMultiplier : public SunwellPlateauEncounterMultiplier
{
public:
    EredarTwinsDisableAutomaticTargetingMultiplier(PlayerbotAI* botAI)
        : SunwellPlateauEncounterMultiplier(botAI, "eredar twins disable automatic targeting") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class EredarTwinsControlMisdirectionMultiplier : public SunwellPlateauEncounterMultiplier
{
public:
    EredarTwinsControlMisdirectionMultiplier(PlayerbotAI* botAI)
        : SunwellPlateauEncounterMultiplier(botAI, "eredar twins misdirect bosses to tanks") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class EredarTwinsHoldDpsAtStartMultiplier : public SunwellPlateauEncounterMultiplier
{
public:
    EredarTwinsHoldDpsAtStartMultiplier(PlayerbotAI* botAI)
        : SunwellPlateauEncounterMultiplier(botAI, "eredar twins hold dps at start") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class EredarTwinsControlThreatMultiplier : public SunwellPlateauEncounterMultiplier
{
public:
    EredarTwinsControlThreatMultiplier(PlayerbotAI* botAI)
        : SunwellPlateauEncounterMultiplier(botAI, "eredar twins control threat") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class EredarTwinsControlMovementMultiplier : public SunwellPlateauEncounterMultiplier
{
public:
    EredarTwinsControlMovementMultiplier(PlayerbotAI* botAI)
        : SunwellPlateauEncounterMultiplier(botAI, "eredar twins control movement") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class EredarTwinsIsolateConflagrationMultiplier : public SunwellPlateauEncounterMultiplier
{
public:
    EredarTwinsIsolateConflagrationMultiplier(PlayerbotAI* botAI)
        : SunwellPlateauEncounterMultiplier(botAI, "eredar twins isolate conflagration") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class EredarTwinsDelayCooldownsMultiplier : public SunwellPlateauEncounterMultiplier
{
public:
    EredarTwinsDelayCooldownsMultiplier(PlayerbotAI* botAI)
        : SunwellPlateauEncounterMultiplier(botAI, "eredar twins delay cooldowns") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

// M'uru

class MuruDisableDefaultTargetingMultiplier : public SunwellPlateauEncounterMultiplier
{
public:
    MuruDisableDefaultTargetingMultiplier(PlayerbotAI* botAI)
        : SunwellPlateauEncounterMultiplier(botAI, "m'uru disable default targeting") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class MuruControlMisdirectionMultiplier : public SunwellPlateauEncounterMultiplier
{
public:
    MuruControlMisdirectionMultiplier(PlayerbotAI* botAI)
        : SunwellPlateauEncounterMultiplier(botAI, "m'uru control misdirection") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class MuruControlMovementMultiplier : public SunwellPlateauEncounterMultiplier
{
public:
    MuruControlMovementMultiplier(PlayerbotAI* botAI)
        : SunwellPlateauEncounterMultiplier(botAI, "m'uru control movement") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class MuruDelayCooldownsMultiplier : public SunwellPlateauEncounterMultiplier
{
public:
    MuruDelayCooldownsMultiplier(PlayerbotAI* botAI)
        : SunwellPlateauEncounterMultiplier(botAI, "m'uru delay cooldowns") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

// Kil'jaeden <The Deceiver>

// The next two are ungated for the reason given in SWPTriggers.h: they run during the Hands of
// the Deceiver phase, which precedes IN_PROGRESS. Holding cooldowns matters most there, and both
// are suppression multipliers, so the gated default of 1.0f would permit precisely what they
// exist to forbid.

class KiljaedenDelayCooldownsMultiplier : public Multiplier
{
public:
    KiljaedenDelayCooldownsMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "kil'jaeden delay cooldowns") {}
    float GetValue(Action* action) override;
};

class KiljaedenSingleTargetHandsMultiplier : public Multiplier
{
public:
    KiljaedenSingleTargetHandsMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "kil'jaeden single target hands") {}
    float GetValue(Action* action) override;
};

class KiljaedenControlMovementAndTargetingMultiplier : public SunwellPlateauEncounterMultiplier
{
public:
    KiljaedenControlMovementAndTargetingMultiplier(PlayerbotAI* botAI)
        : SunwellPlateauEncounterMultiplier(botAI, "kil'jaeden control movement and targeting") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class KiljaedenPrioritizeDarknessProtectionMultiplier : public SunwellPlateauEncounterMultiplier
{
public:
    KiljaedenPrioritizeDarknessProtectionMultiplier(PlayerbotAI* botAI)
        : SunwellPlateauEncounterMultiplier(botAI, "kil'jaeden prioritize darkness protection") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class KiljaedenControlDragonMultiplier : public SunwellPlateauEncounterMultiplier
{
public:
    KiljaedenControlDragonMultiplier(PlayerbotAI* botAI)
        : SunwellPlateauEncounterMultiplier(botAI, "kil'jaeden dragon buff and protect raid") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

#endif
