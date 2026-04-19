/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "RaidHyjalSummitHelpers.h"
#include "AllCreatureScript.h"
#include "DynamicObjectScript.h"
#include "Playerbots.h"
#include "ScriptMgr.h"
#include "Timer.h"

using namespace HyjalSummitHelpers;

// Records the position of each Doomfire NPC (18095) at regular intervals so that bots
// can avoid the persistent fire trail it leaves behind. Each sample is tagged with a
// timestamp and expires after TRAIL_DURATION ms, matching the lifetime of a Doomfire
// DynamicObject (18 seconds)
class ArchimondeDoomfireTrailScript : public AllCreatureScript
{
public:
    ArchimondeDoomfireTrailScript() : AllCreatureScript("ArchimondeDoomfireTrailScript") {}

    void OnAllCreatureUpdate(Creature* creature, uint32 /*diff*/) override
    {
        if (creature->GetEntry() != static_cast<uint32>(HyjalSummitNpcs::NPC_DOOMFIRE))
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

        constexpr uint32 TRAIL_DURATION = 18000;
        trail.erase(std::remove_if(trail.begin(), trail.end(),
            [now](const DoomfireTrailData& d)
            {
                return getMSTimeDiff(d.recordTime, now) > TRAIL_DURATION;
            }), trail.end());

        constexpr float DOOMFIRE_DANGER_RANGE = 10.0f;
        Map::PlayerList const& players = creature->GetMap()->GetPlayers();
        for (Map::PlayerList::const_iterator it = players.begin(); it != players.end(); ++it)
        {
            Player* player = it->GetSource();
            if (!player || !player->IsAlive())
                continue;

            PlayerbotAI* botAI = GET_PLAYERBOT_AI(player);
            if (!botAI)
                continue;

            if (creature->GetDistance(player) > DOOMFIRE_DANGER_RANGE)
                continue;

            botAI->RequestSpellInterrupt();
        }
    }

    void OnCreatureRemoveWorld(Creature* creature) override
    {
        if (creature->GetEntry() != static_cast<uint32>(HyjalSummitNpcs::NPC_DOOMFIRE))
            return;

        doomfireLastSampleTime.erase(creature->GetGUID());
    }
};

// Records the position of each Rain of Fire dynamic object at spawn so that melee bots
// can avoid it by running away from Azgalor; the standard FleePosition() logic to
// avoid aoe can take melee in front of Azgalor, resulting in them getting cleaved
class AzgalorRainOfFireScript : public DynamicObjectScript
{
public:
    AzgalorRainOfFireScript() : DynamicObjectScript("AzgalorRainOfFireScript") {}

    void OnUpdate(DynamicObject* dynobj, uint32 /*diff*/) override
    {
        if (dynobj->GetSpellId() != static_cast<uint32>(HyjalSummitSpells::SPELL_RAIN_OF_FIRE))
            return;

        uint32 instanceId = dynobj->GetMap()->GetInstanceId();
        uint32 now = getMSTime();
        auto& instanceMap = rainOfFirePosition[instanceId];
        ObjectGuid guid = dynobj->GetGUID();

        if (instanceMap.find(guid) == instanceMap.end())
            instanceMap[guid] = { dynobj->GetPosition(), now };
    }
};

void AddSC_HyjalSummitBotScripts()
{
    new ArchimondeDoomfireTrailScript();
    new AzgalorRainOfFireScript();
}
