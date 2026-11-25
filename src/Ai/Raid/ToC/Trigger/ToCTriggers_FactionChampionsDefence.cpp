/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "ToCTriggers_FactionChampionsDefence.h"
#include "ToCHelpers_FactionChampions.h"
#include "ToCHelpers_FactionChampionsDefence.h"
#include "Playerbots.h"
#include "RaidDefensive.h"
#include "RaidDirectTargeting.h"

using namespace TrialOfTheCrusaderHelpers;

bool FactionChampionsCrowdControlTrigger::IsActive()
{
    uint32 spellId = 0;
    return FactionChampionsCcTarget(botAI, spellId) != nullptr;
}

bool FactionChampionsPeelTrigger::IsActive()
{
    uint32 spellId = 0;
    return FactionChampionsPeelTarget(botAI, spellId) != nullptr;
}

bool FactionChampionsAvoidAoeTrigger::IsActive() { return FactionChampionsInAoe(botAI); }

bool FactionChampionsMassDispelTrigger::IsActive() { return FactionChampionsMassDispelTarget(botAI) != nullptr; }

bool FactionChampionsDispelCcTrigger::IsActive()
{
    char const* spell = nullptr;
    return FactionChampionsDispelCcTarget(botAI, spell) != nullptr;
}

bool FactionChampionsKillTotemTrigger::IsActive()
{
    Unit* totem = FactionChampionsTotemTarget(botAI);
    return totem && (AI_VALUE(Unit*, "current target") != totem || RaidPetOffPick(botAI, totem));
}

bool FactionChampionsPhysicalSwitchTrigger::IsActive()
{
    Unit* target = FactionChampionsPhysicalSwitchTarget(botAI);
    return target && AI_VALUE(Unit*, "current target") != target;
}

bool FactionChampionsPurgeKillTargetTrigger::IsActive()
{
    char const* spell = nullptr;
    return FactionChampionsPurgeTarget(botAI, spell) != nullptr;
}

bool FactionChampionsChannelInterruptTrigger::IsActive()
{
    char const* spell = nullptr;
    return FactionChampionsChannelInterruptTarget(botAI, spell) != nullptr;
}

bool FactionChampionsDeathGripTrigger::IsActive()
{
    Position stand;
    return FactionChampionsGripTarget(botAI, stand) != nullptr;
}

bool FactionChampionsChainGripTrigger::IsActive()
{
    Position stand;
    bool walkOnly = false;
    return FactionChampionsChainGripTarget(botAI, stand, walkOnly) != nullptr;
}

bool FactionChampionsWallGripTrigger::IsActive()
{
    Position stand;
    uint32 spellId = 0;
    bool gripped = false;
    return FactionChampionsWallGrip(botAI, stand, spellId, &gripped) && (spellId || !gripped);
}

bool FactionChampionsFreedomTrigger::IsActive()
{
    uint32 spellId = 0;
    return FactionChampionsFreedomTarget(botAI, spellId) != nullptr;
}

bool FactionChampionsSnareTrigger::IsActive()
{
    uint32 spellId = 0;
    return FactionChampionsSnareTarget(botAI, spellId) != nullptr;
}

bool FactionChampionsProtectTrigger::IsActive()
{
    uint32 spellId = 0;
    return FactionChampionsProtectTarget(botAI, spellId) != nullptr;
}

bool FactionChampionsBattleResTrigger::IsActive()
{
    uint32 spellId = 0;
    return FactionChampionsBattleResTarget(botAI, spellId) != nullptr;
}

bool FactionChampionsPersonalDefensiveTrigger::IsActive()
{
    RaidPredictedHit hit;
    return static_cast<bool>(
        GetRaidPersonalDefensive(botAI, TOC_DEFENSIVE_SCOPE, FactionChampionsPersonalHit(botAI, hit) ? &hit : nullptr));
}
