/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_SWPVALUECONTEXT_H
#define PLAYERBOTS_SWPVALUECONTEXT_H

#include "NamedObjectContext.h"
#include "SWPActions.h"
#include "SWPEncounter_KJ.h"
#include "SWPEncounter_Kalec.h"
#include "SWPEncounter_Muru.h"
#include "SWPEncounter_Twins.h"
#include "Value.h"
#include "Position.h"
#include <vector>

class EredarTwinsBlazePositionsValue : public CalculatedValue<std::vector<Position>>
{
public:
    EredarTwinsBlazePositionsValue(PlayerbotAI* botAI)
        : CalculatedValue<std::vector<Position>>(
              botAI, "eredar twins blaze", SwpHelpers::EREDAR_TWINS_BLAZE_CACHE_INTERVAL_MS) {}

protected:
    std::vector<Position> Calculate() override
    {
        return SwpHelpers::FindEredarTwinsBlazePositions(bot);
    }
};

// Collapses four per-tick sweeps of "possible targets no los"
class MuruEncounterTargetsValue : public CalculatedValue<SwpHelpers::MuruEncounterGuids>
{
public:
    MuruEncounterTargetsValue(PlayerbotAI* botAI)
        : CalculatedValue<SwpHelpers::MuruEncounterGuids>(
              botAI, "muru encounter targets",
              SwpHelpers::MURU_ENCOUNTER_TARGETS_CACHE_INTERVAL_MS) {}

protected:
    SwpHelpers::MuruEncounterGuids Calculate() override
    {
        return SwpHelpers::FindMuruEncounterGuids(botAI);
    }
};

// The four values below replace grid searches that are otherwise run by corresponding triggers and
// actions each tick.
class MuruVoidZonesValue : public CalculatedValue<GuidVector>
{
public:
    MuruVoidZonesValue(PlayerbotAI* botAI)
        : CalculatedValue<GuidVector>(
              botAI, "muru void zones", SwpHelpers::MURU_VOID_ZONE_CACHE_INTERVAL_MS) {}

protected:
    GuidVector Calculate() override { return SwpHelpers::FindMuruVoidZoneGuids(bot); }
};

class SwpVolatileFiendValue : public CalculatedValue<ObjectGuid>
{
public:
    SwpVolatileFiendValue(PlayerbotAI* botAI)
        : CalculatedValue<ObjectGuid>(
              botAI, "swp volatile fiend", SwpHelpers::SWP_VOLATILE_FIEND_CACHE_INTERVAL_MS) {}

protected:
    ObjectGuid Calculate() override { return SwpHelpers::FindSwpVolatileFiendGuid(bot); }
};

class KalecgosSpectralRiftValue : public CalculatedValue<ObjectGuid>
{
public:
    KalecgosSpectralRiftValue(PlayerbotAI* botAI)
        : CalculatedValue<ObjectGuid>(
              botAI, "kalecgos spectral rift",
              SwpHelpers::KALECGOS_SPECTRAL_RIFT_CACHE_INTERVAL_MS) {}

protected:
    ObjectGuid Calculate() override { return SwpHelpers::FindKalecgosSpectralRiftGuid(bot); }
};

class MuruSingularityValue : public CalculatedValue<ObjectGuid>
{
public:
    MuruSingularityValue(PlayerbotAI* botAI)
        : CalculatedValue<ObjectGuid>(
              botAI, "muru singularity", SwpHelpers::MURU_SINGULARITY_CACHE_INTERVAL_MS) {}

protected:
    ObjectGuid Calculate() override { return SwpHelpers::FindMuruSingularityGuid(bot); }
};

class KiljaedenDragonOrbsValue : public CalculatedValue<GuidVector>
{
public:
    KiljaedenDragonOrbsValue(PlayerbotAI* botAI)
        : CalculatedValue<GuidVector>(
              botAI, "kiljaeden dragon orbs",
              SwpHelpers::KILJAEDEN_DRAGON_ORB_CACHE_INTERVAL_MS) {}

protected:
    GuidVector Calculate() override { return SwpHelpers::FindKiljaedenDragonOrbGuids(bot); }
};

class RaidSunwellValueContext : public NamedObjectContext<UntypedValue>
{
public:
    RaidSunwellValueContext()
    {
        creators["eredar twins blaze"] = &RaidSunwellValueContext::eredar_twins_blaze;
        creators["muru encounter targets"] = &RaidSunwellValueContext::muru_encounter_targets;
        creators["muru void zones"] = &RaidSunwellValueContext::muru_void_zones;
        creators["swp volatile fiend"] = &RaidSunwellValueContext::swp_volatile_fiend;
        creators["kalecgos spectral rift"] = &RaidSunwellValueContext::kalecgos_spectral_rift;
        creators["muru singularity"] = &RaidSunwellValueContext::muru_singularity;
        creators["kiljaeden dragon orbs"] = &RaidSunwellValueContext::kiljaeden_dragon_orbs;
    }

private:
    static UntypedValue* eredar_twins_blaze(PlayerbotAI* botAI) {
        return new EredarTwinsBlazePositionsValue(botAI);
    }
    static UntypedValue* muru_encounter_targets(PlayerbotAI* botAI) {
        return new MuruEncounterTargetsValue(botAI);
    }
    static UntypedValue* muru_void_zones(PlayerbotAI* botAI) {
        return new MuruVoidZonesValue(botAI);
    }
    static UntypedValue* swp_volatile_fiend(PlayerbotAI* botAI) {
        return new SwpVolatileFiendValue(botAI);
    }
    static UntypedValue* kalecgos_spectral_rift(PlayerbotAI* botAI) {
        return new KalecgosSpectralRiftValue(botAI);
    }
    static UntypedValue* muru_singularity(PlayerbotAI* botAI) {
        return new MuruSingularityValue(botAI);
    }
    static UntypedValue* kiljaeden_dragon_orbs(PlayerbotAI* botAI) {
        return new KiljaedenDragonOrbsValue(botAI);
    }
};

#endif
