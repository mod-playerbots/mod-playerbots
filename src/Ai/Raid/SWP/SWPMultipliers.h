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
#include <optional>
#include <string>

// Shared

class SunwellEncounterMultiplier : public Multiplier
{
public:
    SunwellEncounterMultiplier(PlayerbotAI* botAI, std::string const name)
        : Multiplier(botAI, name) {}
    SunwellEncounterMultiplier(
        PlayerbotAI* botAI, std::string const name, SwpHelpers::SwpBosses boss)
        : Multiplier(botAI, name), _boss(boss) {}

    float GetValue(Action* action) final
    {
        bool const inProgress = _boss ?
            EncounterHelpers::IsBossInProgress(
                bot, SwpHelpers::SWP_MAP_ID, SwpHelpers::Id(*_boss)) :
            EncounterHelpers::IsEncounterInProgress(bot, SwpHelpers::SWP_MAP_ID);
        return inProgress ? GetValueInEncounter(action) : 1.0f;
    }

protected:
    virtual float GetValueInEncounter(Action* action) = 0;
    Unit* GetEncounterBoss()
    {
        return _boss ? SwpHelpers::GetSwpBoss(bot, *_boss) : nullptr;
    }

private:
    std::optional<SwpHelpers::SwpBosses> const _boss;
};

class SunwellNoEncounterDrinkingMultiplier : public SunwellEncounterMultiplier
{
public:
    SunwellNoEncounterDrinkingMultiplier(PlayerbotAI* botAI)
        : SunwellEncounterMultiplier(botAI, "sunwell no encounter drinking") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

// Trash

class VolatileFiendRestrictApproachMultiplier : public Multiplier
{
public:
    VolatileFiendRestrictApproachMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "volatile fiend restrict approach") {}
    float GetValue(Action* action) override;
};

// Shared Boss

class SunwellControlMisdirectionMultiplier : public SunwellEncounterMultiplier
{
public:
    SunwellControlMisdirectionMultiplier(PlayerbotAI* botAI)
        : SunwellEncounterMultiplier(botAI, "sunwell control misdirection") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class SunwellDelayDpsCooldownsMultiplier : public SunwellEncounterMultiplier
{
public:
    SunwellDelayDpsCooldownsMultiplier(PlayerbotAI* botAI)
        : SunwellEncounterMultiplier(botAI, "sunwell delay dps cooldowns") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

// Kalecgos

class KalecgosWaitToDecurseMultiplier : public SunwellEncounterMultiplier
{
public:
    KalecgosWaitToDecurseMultiplier(PlayerbotAI* botAI)
        : SunwellEncounterMultiplier(
            botAI, "kalecgos wait to decurse", SwpHelpers::SwpBosses::DATA_KALECGOS) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class KalecgosControlMovementMultiplier : public SunwellEncounterMultiplier
{
public:
    KalecgosControlMovementMultiplier(PlayerbotAI* botAI)
        : SunwellEncounterMultiplier(
            botAI, "kalecgos control movement", SwpHelpers::SwpBosses::DATA_KALECGOS) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class KalecgosRestrictTauntMultiplier : public SunwellEncounterMultiplier
{
public:
    KalecgosRestrictTauntMultiplier(PlayerbotAI* botAI)
        : SunwellEncounterMultiplier(
            botAI, "kalecgos restrict taunt", SwpHelpers::SwpBosses::DATA_KALECGOS) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class KalecgosSuppressAssistTankPullThreatMultiplier : public SunwellEncounterMultiplier
{
public:
    KalecgosSuppressAssistTankPullThreatMultiplier(PlayerbotAI* botAI)
        : SunwellEncounterMultiplier(
            botAI, "kalecgos suppress assist tank pull threat",
            SwpHelpers::SwpBosses::DATA_KALECGOS) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class KalecgosEnterSpectralRiftMultiplier : public SunwellEncounterMultiplier
{
public:
    KalecgosEnterSpectralRiftMultiplier(PlayerbotAI* botAI)
        : SunwellEncounterMultiplier(
            botAI, "kalecgos enter spectral rift", SwpHelpers::SwpBosses::DATA_KALECGOS) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class KalecgosHoldDpsMultiplier : public SunwellEncounterMultiplier
{
public:
    KalecgosHoldDpsMultiplier(PlayerbotAI* botAI)
        : SunwellEncounterMultiplier(
            botAI, "kalecgos hold dps", SwpHelpers::SwpBosses::DATA_KALECGOS) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

// Brutallus

class BrutallusControlMovementMultiplier : public SunwellEncounterMultiplier
{
public:
    BrutallusControlMovementMultiplier(PlayerbotAI* botAI)
        : SunwellEncounterMultiplier(
            botAI, "brutallus control movement", SwpHelpers::SwpBosses::DATA_BRUTALLUS) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class BrutallusNoKillingSpreeWhenNearbyBurnMultiplier : public SunwellEncounterMultiplier
{
public:
    BrutallusNoKillingSpreeWhenNearbyBurnMultiplier(PlayerbotAI* botAI)
        : SunwellEncounterMultiplier(
            botAI, "brutallus no killing spree when nearby burn",
            SwpHelpers::SwpBosses::DATA_BRUTALLUS) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class BrutallusRestrictTauntMultiplier : public SunwellEncounterMultiplier
{
public:
    BrutallusRestrictTauntMultiplier(PlayerbotAI* botAI)
        : SunwellEncounterMultiplier(
            botAI, "brutallus restrict taunt", SwpHelpers::SwpBosses::DATA_BRUTALLUS) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class BrutallusNoRebirthWhileBurningMultiplier : public SunwellEncounterMultiplier
{
public:
    BrutallusNoRebirthWhileBurningMultiplier(PlayerbotAI* botAI)
        : SunwellEncounterMultiplier(
            botAI, "brutallus no rebirth while burning", SwpHelpers::SwpBosses::DATA_BRUTALLUS) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

// Felmyst

class FelmystControlMovementMultiplier : public SunwellEncounterMultiplier
{
public:
    FelmystControlMovementMultiplier(PlayerbotAI* botAI)
        : SunwellEncounterMultiplier(
            botAI, "felmyst control movement", SwpHelpers::SwpBosses::DATA_FELMYST) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class FelmystWaitForLandingDpsMultiplier : public SunwellEncounterMultiplier
{
public:
    FelmystWaitForLandingDpsMultiplier(PlayerbotAI* botAI)
        : SunwellEncounterMultiplier(
            botAI, "felmyst wait for landing dps", SwpHelpers::SwpBosses::DATA_FELMYST) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class FelmystPrioritizeEncapsulateAvoidanceMultiplier : public SunwellEncounterMultiplier
{
public:
    FelmystPrioritizeEncapsulateAvoidanceMultiplier(PlayerbotAI* botAI)
        : SunwellEncounterMultiplier(
            botAI, "felmyst prioritize encapsulate avoidance",
            SwpHelpers::SwpBosses::DATA_FELMYST) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class FelmystKeepHealersInSideStacksMultiplier : public SunwellEncounterMultiplier
{
public:
    FelmystKeepHealersInSideStacksMultiplier(PlayerbotAI* botAI)
        : SunwellEncounterMultiplier(
            botAI, "felmyst keep healers in side stacks", SwpHelpers::SwpBosses::DATA_FELMYST) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class FelmystPrioritizeFogAvoidanceMultiplier : public SunwellEncounterMultiplier
{
public:
    FelmystPrioritizeFogAvoidanceMultiplier(PlayerbotAI* botAI)
        : SunwellEncounterMultiplier(
            botAI, "felmyst prioritize fog avoidance", SwpHelpers::SwpBosses::DATA_FELMYST) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class FelmystPrioritizeDemonicVaporAvoidanceMultiplier : public SunwellEncounterMultiplier
{
public:
    FelmystPrioritizeDemonicVaporAvoidanceMultiplier(PlayerbotAI* botAI)
        : SunwellEncounterMultiplier(
            botAI, "felmyst prioritize demonic vapor avoidance",
            SwpHelpers::SwpBosses::DATA_FELMYST) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class FelmystFocusAttacksOnCharmedPlayerMultiplier : public SunwellEncounterMultiplier
{
public:
    FelmystFocusAttacksOnCharmedPlayerMultiplier(PlayerbotAI* botAI)
        : SunwellEncounterMultiplier(
            botAI, "felmyst focus attacks on charmed player",
            SwpHelpers::SwpBosses::DATA_FELMYST) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class FelmystDisableDotsOnAddsMultiplier : public SunwellEncounterMultiplier
{
public:
    FelmystDisableDotsOnAddsMultiplier(PlayerbotAI* botAI)
        : SunwellEncounterMultiplier(
            botAI, "felmyst disable dots on adds", SwpHelpers::SwpBosses::DATA_FELMYST) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

// Eredar Twins

class EredarTwinsDisableAutoTargetingMultiplier : public SunwellEncounterMultiplier
{
public:
    EredarTwinsDisableAutoTargetingMultiplier(PlayerbotAI* botAI)
        : SunwellEncounterMultiplier(
            botAI, "eredar twins disable auto targeting",
            SwpHelpers::SwpBosses::DATA_EREDAR_TWINS) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class EredarTwinsHoldDpsAtStartMultiplier : public SunwellEncounterMultiplier
{
public:
    EredarTwinsHoldDpsAtStartMultiplier(PlayerbotAI* botAI)
        : SunwellEncounterMultiplier(
            botAI, "eredar twins hold dps at start", SwpHelpers::SwpBosses::DATA_EREDAR_TWINS) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class EredarTwinsControlThreatMultiplier : public SunwellEncounterMultiplier
{
public:
    EredarTwinsControlThreatMultiplier(PlayerbotAI* botAI)
        : SunwellEncounterMultiplier(
            botAI, "eredar twins control threat", SwpHelpers::SwpBosses::DATA_EREDAR_TWINS) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class EredarTwinsControlMovementMultiplier : public SunwellEncounterMultiplier
{
public:
    EredarTwinsControlMovementMultiplier(PlayerbotAI* botAI)
        : SunwellEncounterMultiplier(
            botAI, "eredar twins control movement", SwpHelpers::SwpBosses::DATA_EREDAR_TWINS) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class EredarTwinsIsolateConflagrationMultiplier : public SunwellEncounterMultiplier
{
public:
    EredarTwinsIsolateConflagrationMultiplier(PlayerbotAI* botAI)
        : SunwellEncounterMultiplier(
            botAI, "eredar twins isolate conflagration",
            SwpHelpers::SwpBosses::DATA_EREDAR_TWINS) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

// M'uru

class MuruDisableDefaultTargetingMultiplier : public SunwellEncounterMultiplier
{
public:
    MuruDisableDefaultTargetingMultiplier(PlayerbotAI* botAI)
        : SunwellEncounterMultiplier(
            botAI, "m'uru disable default targeting", SwpHelpers::SwpBosses::DATA_MURU) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class MuruControlMovementMultiplier : public SunwellEncounterMultiplier
{
public:
    MuruControlMovementMultiplier(PlayerbotAI* botAI)
        : SunwellEncounterMultiplier(
            botAI, "m'uru control movement", SwpHelpers::SwpBosses::DATA_MURU) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class MuruHoldDpsOnEntropiusMultiplier : public SunwellEncounterMultiplier
{
public:
    MuruHoldDpsOnEntropiusMultiplier(PlayerbotAI* botAI)
        : SunwellEncounterMultiplier(
            botAI, "m'uru hold dps on entropius", SwpHelpers::SwpBosses::DATA_MURU) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class MuruLetMagesStealSpellFuryMultiplier : public SunwellEncounterMultiplier
{
public:
    MuruLetMagesStealSpellFuryMultiplier(PlayerbotAI* botAI)
        : SunwellEncounterMultiplier(
            botAI, "m'uru let mages steal spell fury", SwpHelpers::SwpBosses::DATA_MURU) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

// Kil'jaeden <The Deceiver>

class KiljaedenSingleTargetHandsMultiplier : public SunwellEncounterMultiplier
{
public:
    KiljaedenSingleTargetHandsMultiplier(PlayerbotAI* botAI)
        : SunwellEncounterMultiplier(
            botAI, "kil'jaeden single target hands", SwpHelpers::SwpBosses::DATA_KILJAEDEN) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class KiljaedenHoldDpsAtStartMultiplier : public SunwellEncounterMultiplier
{
public:
    KiljaedenHoldDpsAtStartMultiplier(PlayerbotAI* botAI)
        : SunwellEncounterMultiplier(
            botAI, "kil'jaeden hold dps at start", SwpHelpers::SwpBosses::DATA_KILJAEDEN) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class KiljaedenControlMovementAndTargetingMultiplier : public SunwellEncounterMultiplier
{
public:
    KiljaedenControlMovementAndTargetingMultiplier(PlayerbotAI* botAI)
        : SunwellEncounterMultiplier(
            botAI, "kil'jaeden control movement and targeting",
            SwpHelpers::SwpBosses::DATA_KILJAEDEN) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class KiljaedenPrioritizeDarknessProtectionMultiplier : public SunwellEncounterMultiplier
{
public:
    KiljaedenPrioritizeDarknessProtectionMultiplier(PlayerbotAI* botAI)
        : SunwellEncounterMultiplier(
            botAI, "kil'jaeden prioritize darkness protection",
            SwpHelpers::SwpBosses::DATA_KILJAEDEN) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class KiljaedenControlDragonMultiplier : public SunwellEncounterMultiplier
{
public:
    KiljaedenControlDragonMultiplier(PlayerbotAI* botAI)
        : SunwellEncounterMultiplier(
            botAI, "kil'jaeden control dragon", SwpHelpers::SwpBosses::DATA_KILJAEDEN) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

#endif
