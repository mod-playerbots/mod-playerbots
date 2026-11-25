/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ONYMULTIPLIERS_H
#define PLAYERBOTS_ONYMULTIPLIERS_H

#include "EncounterHelpers.h"
#include "Multiplier.h"
#include "OnyHelpers.h"
#include <string>

class OnyxiasLairEncounterMultiplier : public Multiplier
{
public:
    OnyxiasLairEncounterMultiplier(
        PlayerbotAI* botAI, std::string const name) : Multiplier(botAI, name) {}

    float GetValue(Action* action) final
    {
        return EncounterHelpers::IsEncounterInProgress(bot, OnyxiaHelpers::ONYXIAS_LAIR_MAP_ID) ?
            GetValueInEncounter(action) : 1.0f;
    }

protected:
    virtual float GetValueInEncounter(Action* action) = 0;
};

class OnyxiaPhaseThreeThreatMultiplier : public OnyxiasLairEncounterMultiplier
{
public:
    OnyxiaPhaseThreeThreatMultiplier(PlayerbotAI* botAI)
        : OnyxiasLairEncounterMultiplier(botAI, "onyxia phase three threat") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

#endif
