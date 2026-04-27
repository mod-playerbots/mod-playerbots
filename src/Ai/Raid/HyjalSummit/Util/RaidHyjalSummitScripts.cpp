/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "RaidHyjalSummitHelpers.h"
#include "AllCreatureScript.h"
#include "ObjectAccessor.h"
#include "Player.h"
#include "RaidBossHelpers.h"
#include "DynamicObjectScript.h"
#include "Playerbots.h"
#include "ScriptMgr.h"
#include "Spell.h"
#include "Timer.h"

using namespace HyjalSummitHelpers;

static Player* GetFirstPlayerSpellTarget(Spell* spell, Unit* caster)
{
    if (!spell || !caster)
        return nullptr;

    if (Unit* unitTarget = spell->m_targets.GetUnitTarget())
        return unitTarget->ToPlayer();

    std::list<TargetInfo> const& targets = *spell->GetUniqueTargetInfo();
    for (TargetInfo const& targetInfo : targets)
    {
        if (Player* target = ObjectAccessor::GetPlayer(*caster, targetInfo.targetGUID))
            return target;
    }

    return nullptr;
}

static bool ShouldInterruptForArchimondeAirBurst(PlayerbotAI* botAI, Player* bot, Player* target)
{
    if (!target)
        return false;

    Player* mainTank = GetGroupMainTank(botAI, bot);
    if (!mainTank || bot == mainTank)
        return false;

    float distanceToMainTank = bot->GetExactDist2d(mainTank);

    return (target == mainTank || target == bot) &&
           distanceToMainTank < AIR_BURST_SAFE_DISTANCE;
}

// Records the active Rain of Fire dynamic object so that melee bots can avoid it by running
// away from Azgalor or swapping to a Doomguard; the standard FleePosition() logic to avoid aoe
// can take melee in front of Azgalor, resulting in them getting cleaved
class AzgalorRainOfFireScript : public DynamicObjectScript
{
public:
    AzgalorRainOfFireScript() : DynamicObjectScript("AzgalorRainOfFireScript") {}

    void OnUpdate(DynamicObject* dynobj, uint32 /*diff*/) override
    {
        if (dynobj->GetSpellId() != static_cast<uint32>(HyjalSummitSpells::SPELL_RAIN_OF_FIRE))
            return;

        uint32 instanceId = dynobj->GetMap()->GetInstanceId();
        if (GetActiveAzgalorRainOfFire(instanceId))
            return;

        uint32 now = getMSTime();
        auto instanceIt = rainOfFirePosition.find(instanceId);
        if (instanceIt != rainOfFirePosition.end() &&
            getMSTimeDiff(instanceIt->second.spawnTime, now) < RAIN_OF_FIRE_REACQUIRE_DELAY)
        {
            return;
        }

        bool shouldTrackRainOfFire = false;
        Map::PlayerList const& players = dynobj->GetMap()->GetPlayers();
        for (Map::PlayerList::const_iterator it = players.begin(); it != players.end(); ++it)
        {
            Player* player = it->GetSource();
            if (!player || !player->IsAlive())
                continue;

            PlayerbotAI* botAI = GET_PLAYERBOT_AI(player);
            if (!botAI || !botAI->HasStrategy("hyjal", BOT_STATE_COMBAT))
                continue;

            shouldTrackRainOfFire = true;
            break;
        }

        if (!shouldTrackRainOfFire)
            return;

        rainOfFirePosition[instanceId] = RainOfFireData{ dynobj->GetPosition(), now };
    }
};

// Records the position of each Doomfire NPC at regular intervals so that bots can avoid
// the persistent fire trail it leaves behind. Each sample is tagged with a timestamp and
// expires after TRAIL_DURATION ms, matching the lifetime of a Doomfire DynamicObject (18s)
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
            if (!botAI || !botAI->HasStrategy("hyjal", BOT_STATE_COMBAT) ||
                creature->GetDistance(player) > DOOMFIRE_DANGER_RANGE)
            {
                continue;
            }

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

class ArchimondeAirBurstSpellListenerScript : public AllSpellScript
{
public:
    ArchimondeAirBurstSpellListenerScript() :
        AllSpellScript("ArchimondeAirBurstSpellListenerScript") {}

    void OnSpellCast(
        Spell* spell, Unit* caster, SpellInfo const* spellInfo, bool /*skipCheck*/) override
    {
        if (!spell || !caster || !spellInfo)
            return;

        if (spellInfo->Id != static_cast<uint32>(HyjalSummitSpells::SPELL_AIR_BURST))
            return;

        Player* target = GetFirstPlayerSpellTarget(spell, caster);
        if (!target)
            return;

        archimondeAirBurstTargets[caster->GetMap()->GetInstanceId()] =
            AirBurstData{ target->GetGUID(), getMSTime() };

        Map::PlayerList const& players = caster->GetMap()->GetPlayers();
        for (Map::PlayerList::const_iterator it = players.begin(); it != players.end(); ++it)
        {
            Player* player = it->GetSource();
            if (!player || !player->IsAlive())
                continue;

            PlayerbotAI* botAI = GET_PLAYERBOT_AI(player);
            if (!botAI || !botAI->HasStrategy("hyjal", BOT_STATE_COMBAT) ||
                !ShouldInterruptForArchimondeAirBurst(botAI, player, target))
            {
                continue;
            }

            botAI->RequestSpellInterrupt();
        }
    }
};

void AddSC_HyjalSummitBotScripts()
{
    new AzgalorRainOfFireScript();
    new ArchimondeDoomfireTrailScript();
    new ArchimondeAirBurstSpellListenerScript();
}
