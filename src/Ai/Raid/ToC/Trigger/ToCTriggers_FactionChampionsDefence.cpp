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
#include "Strategy.h"

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

void AddToCFactionChampionsDefenceTriggerNodes(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("faction champions personal defensive", {
        NextAction("faction champions personal defensive", ACTION_EMERGENCY + 8.4f) }));

    triggers.push_back(new TriggerNode("faction champions avoid aoe", {
        NextAction("faction champions avoid aoe", ACTION_EMERGENCY + 6) }));

    triggers.push_back(new TriggerNode("faction champions channel interrupt", {
        NextAction("faction champions channel interrupt", ACTION_INTERRUPT + 4) }));

    triggers.push_back(new TriggerNode("faction champions protect", {
        NextAction("faction champions protect", ACTION_EMERGENCY + 4) }));

    triggers.push_back(new TriggerNode("faction champions freedom", {
        NextAction("faction champions freedom", ACTION_EMERGENCY + 3) }));

    // the chat command's own trigger: a tank bot takes the pull too, and the chain opens once either way
    triggers.push_back(new TriggerNode("pull", {
        NextAction("faction champions chain request", ACTION_RAID + 4.7f) }));

    // over the ice duty nodes, so a link leaves its champion for it
    triggers.push_back(new TriggerNode("faction champions chain grip", {
        NextAction("faction champions chain grip", ACTION_RAID + 4.6f) }));

    triggers.push_back(new TriggerNode("faction champions death grip", {
        NextAction("faction champions death grip", ACTION_RAID + 4.5f) }));

    triggers.push_back(new TriggerNode("faction champions battle res", {
        NextAction("faction champions battle res", ACTION_RAID + 4.2f) }));

    triggers.push_back(new TriggerNode("faction champions mass dispel", {
        NextAction("faction champions mass dispel", ACTION_RAID + 5) }));

    triggers.push_back(new TriggerNode("faction champions dispel cc", {
        NextAction("faction champions dispel cc", ACTION_RAID + 3.5f) }));

    triggers.push_back(new TriggerNode("faction champions crowd control", {
        NextAction("faction champions crowd control", ACTION_RAID + 3.4f) }));

    triggers.push_back(new TriggerNode("faction champions peel", {
        NextAction("faction champions peel", ACTION_RAID + 3.3f) }));

    triggers.push_back(new TriggerNode("faction champions snare", {
        NextAction("faction champions snare", ACTION_RAID + 3.2f) }));

    triggers.push_back(new TriggerNode("faction champions kill totem", {
        NextAction("faction champions kill totem", ACTION_RAID + 2.7f) }));

    triggers.push_back(new TriggerNode("faction champions physical switch", {
        NextAction("faction champions physical switch", ACTION_RAID + 2.5f) }));

    // under the kite and the Nitro run, over the holds and the focus
    triggers.push_back(new TriggerNode("faction champions wall grip", {
        NextAction("faction champions wall grip", ACTION_RAID + 2.3f) }));

    triggers.push_back(new TriggerNode("faction champions purge kill target", {
        NextAction("faction champions purge kill target", ACTION_RAID) }));
}
