/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_SWPVALUECONTEXT_H
#define PLAYERBOTS_SWPVALUECONTEXT_H

#include "NamedObjectContext.h"
#include "ObjectGuid.h"
#include "SWPEncounter_KJ.h"
#include "SWPEncounter_Kalec.h"
#include "SWPEncounter_Twins.h"
#include "SWPShared.h"
#include "Position.h"
#include "Value.h"
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

class EredarTwinsThreatHoldsValue : public CalculatedValue<SwpHelpers::EredarTwinsThreatHolds>
{
public:
    EredarTwinsThreatHoldsValue(PlayerbotAI* botAI)
        : CalculatedValue<SwpHelpers::EredarTwinsThreatHolds>(
              botAI, "eredar twins threat holds",
              SwpHelpers::EREDAR_TWINS_THREAT_HOLDS_CACHE_INTERVAL_MS) {}

protected:
    SwpHelpers::EredarTwinsThreatHolds Calculate() override
    {
        return SwpHelpers::FindEredarTwinsThreatHolds(bot);
    }
};

class KalecgosSpectralRiftValue : public CalculatedValue<ObjectGuid>
{
public:
    KalecgosSpectralRiftValue(PlayerbotAI* botAI)
        : CalculatedValue<ObjectGuid>(
              botAI, "kalecgos spectral rift", SwpHelpers::SPECTRAL_RIFT_CACHE_INTERVAL_MS) {}

protected:
    ObjectGuid Calculate() override { return SwpHelpers::FindKalecgosSpectralRiftGuid(bot); }
};

class KiljaedenDragonOrbsValue : public CalculatedValue<GuidVector>
{
public:
    KiljaedenDragonOrbsValue(PlayerbotAI* botAI)
        : CalculatedValue<GuidVector>(
              botAI, "kiljaeden dragon orbs", SwpHelpers::DRAGON_ORB_CACHE_INTERVAL_MS) {}

protected:
    GuidVector Calculate() override { return SwpHelpers::FindKiljaedenDragonOrbGuids(bot); }
};

class RaidSwpValueContext : public NamedObjectContext<UntypedValue>
{
public:
    RaidSwpValueContext()
    {
        creators["eredar twins blaze"] = &RaidSwpValueContext::eredar_twins_blaze;
        creators["eredar twins threat holds"] = &RaidSwpValueContext::eredar_twins_threat_holds;
        creators["kalecgos spectral rift"] = &RaidSwpValueContext::kalecgos_spectral_rift;
        creators["kiljaeden dragon orbs"] = &RaidSwpValueContext::kiljaeden_dragon_orbs;
    }

private:
    static UntypedValue* eredar_twins_blaze(PlayerbotAI* botAI)
    {
        return new EredarTwinsBlazePositionsValue(botAI);
    }
    static UntypedValue* eredar_twins_threat_holds(PlayerbotAI* botAI)
    {
        return new EredarTwinsThreatHoldsValue(botAI);
    }
    static UntypedValue* kalecgos_spectral_rift(PlayerbotAI* botAI)
    {
        return new KalecgosSpectralRiftValue(botAI);
    }
    static UntypedValue* kiljaeden_dragon_orbs(PlayerbotAI* botAI)
    {
        return new KiljaedenDragonOrbsValue(botAI);
    }
};

#endif
