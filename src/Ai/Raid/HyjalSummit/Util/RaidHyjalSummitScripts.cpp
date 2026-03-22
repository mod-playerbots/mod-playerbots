/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "RaidHyjalSummitHelpers.h"
#include "AllCreatureScript.h"
#include "ScriptMgr.h"
#include "Timer.h"

using namespace HyjalSummitHelpers;

// Records the position of each Doomfire NPC (18095) at regular intervals so that bots
// can avoid the persistent fire trail it leaves behind. Each sample is tagged with a
// timestamp and expires after TRAIL_DURATION ms, matching the lifetime of a Doomfire
// DynamicObject (18 seconds).
class DoomfireTrailScript : public AllCreatureScript
{
public:
    DoomfireTrailScript() : AllCreatureScript("DoomfireTrailScript") {}

    void OnAllCreatureUpdate(Creature* creature, uint32 /*diff*/) override
    {
        if (creature->GetEntry() != static_cast<uint32>(HyjalSummitNPCs::NPC_DOOMFIRE))
            return;

        uint32 now = getMSTime();
        ObjectGuid guid = creature->GetGUID();

        auto& lastSample = doomfireLastSampleTime[guid];
        if (getMSTimeDiff(lastSample, now) < 500)
            return;

        lastSample = now;

        uint32 instanceId = creature->GetMap()->GetInstanceId();
        auto& trail = doomfireTrails[instanceId];

        DoomfireTrailData data;
        data.position = creature->GetPosition();
        data.recordTime = now;
        trail.push_back(data);

        constexpr uint32 TRAIL_DURATION = 19000;
        trail.erase(std::remove_if(trail.begin(), trail.end(),
            [now](const DoomfireTrailData& d)
            {
                return getMSTimeDiff(d.recordTime, now) > TRAIL_DURATION;
            }), trail.end());
    }

    void OnCreatureRemoveWorld(Creature* creature) override
    {
        if (creature->GetEntry() != static_cast<uint32>(HyjalSummitNPCs::NPC_DOOMFIRE))
            return;

        doomfireLastSampleTime.erase(creature->GetGUID());
    }
};

void AddSC_HyjalSummitBotScripts()
{
    new DoomfireTrailScript();
}
