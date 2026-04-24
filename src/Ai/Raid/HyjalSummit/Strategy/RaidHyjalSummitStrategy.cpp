/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "RaidHyjalSummitStrategy.h"
#include "RaidHyjalSummitMultipliers.h"

void RaidHyjalSummitStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // General
    triggers.push_back(new TriggerNode("hyjal summit bot is not in combat", {
        NextAction("hyjal summit erase trackers", ACTION_EMERGENCY + 11) }));

    // Rage Winterchill
    triggers.push_back(new TriggerNode("rage winterchill pulling boss", {
        NextAction("rage winterchill misdirect boss to main tank", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode("rage winterchill boss engaged by main tank", {
        NextAction("rage winterchill main tank position boss", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("rage winterchill boss casts death and decay on ranged", {
        NextAction("rage winterchill spread ranged in circle", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("rage winterchill melee is standing in death and decay", {
        NextAction("rage winterchill melee get out of death and decay", ACTION_EMERGENCY + 1) }));

    // Anetheron
    triggers.push_back(new TriggerNode("anetheron pulling boss or infernal", {
        NextAction("anetheron misdirect boss and infernals to tanks", ACTION_RAID + 3) }));

    triggers.push_back(new TriggerNode("anetheron boss engaged by main tank", {
        NextAction("anetheron main tank position boss", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("anetheron boss casts carrion swarm", {
        NextAction("anetheron spread ranged in circle", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode("anetheron bot is targeted by infernal", {
        NextAction("anetheron bring infernal to infernal tank", ACTION_EMERGENCY + 2) }));

    triggers.push_back(new TriggerNode("anetheron infernals need to be kept away from raid", {
        NextAction("anetheron first assist tank pick up infernals", ACTION_EMERGENCY + 1) }));

    triggers.push_back(new TriggerNode("anetheron infernals continue to spawn", {
        NextAction("anetheron assign dps priority", ACTION_RAID + 1) }));

    // Kaz'rogal
    triggers.push_back(new TriggerNode("kaz'rogal pulling boss", {
        NextAction("kaz'rogal misdirect boss to main tank", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode("kaz'rogal boss engaged by main tank", {
        NextAction("kaz'rogal main tank position boss", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("kaz'rogal boss engaged by assist tanks", {
        NextAction("kaz'rogal assist tanks move in front of boss", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("kaz'rogal low mana bots need escape path", {
        NextAction("kaz'rogal spread ranged in arc", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("kaz'rogal bot is low on mana", {
        NextAction("kaz'rogal low mana bot take defensive measures", ACTION_EMERGENCY + 1) }));

    triggers.push_back(new TriggerNode("kaz'rogal mark deals shadow damage", {
        NextAction("kaz'rogal cast shadow protection spell", ACTION_EMERGENCY + 6) }));

    // Azgalor
    triggers.push_back(new TriggerNode("azgalor pulling boss", {
        NextAction("azgalor misdirect boss to main tank", ACTION_RAID + 3) }));

    triggers.push_back(new TriggerNode("azgalor boss engaged by main tank", {
        NextAction("azgalor main tank position boss", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("azgalor main tank is positioning boss", {
        NextAction("azgalor wait at safe position", ACTION_EMERGENCY + 1) }));

    triggers.push_back(new TriggerNode("azgalor boss engaged by ranged", {
        NextAction("azgalor disperse ranged", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode("azgalor boss casts rain of fire on melee", {
        NextAction("azgalor melee get out of fire and swap targets", ACTION_EMERGENCY + 2) }));

    triggers.push_back(new TriggerNode("azgalor bot is doomed", {
        NextAction("azgalor move to doomguard tank", ACTION_EMERGENCY + 3) }));

    triggers.push_back(new TriggerNode("azgalor doomguards must be controlled", {
        NextAction("azgalor first assist tank position doomguard", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("azgalor doomguards must die", {
        NextAction("azgalor ranged dps prioritize doomguards", ACTION_RAID + 1) }));

    // Archimonde
    triggers.push_back(new TriggerNode("archimonde pulling boss", {
        NextAction("archimonde misdirect boss to main tank", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode("archimonde boss engaged by main tank", {
        NextAction("archimonde move boss to initial position", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode("archimonde boss casts fear", {
        NextAction("archimonde cast fear immunity spell", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode("archimonde boss casts air burst", {
        NextAction("archimonde spread to avoid air burst", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("archimonde boss summoned doomfire", {
        NextAction("archimonde avoid doomfire", ACTION_EMERGENCY + 6) }));

    triggers.push_back(new TriggerNode("archimonde bot stood in doomfire", {
        NextAction("archimonde remove doomfire dot", ACTION_EMERGENCY + 7) }));
}

void RaidHyjalSummitStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    // Trash
    multipliers.push_back(new HyjalSummitTimeBloodlustAndHeroismMultiplier(botAI));

    // Rage Winterchill
    multipliers.push_back(new RageWinterchillDisableCombatFormationMoveMultiplier(botAI));
    multipliers.push_back(new RageWinterchillMeleeControlAvoidanceMultiplier(botAI));

    // Anetheron
    multipliers.push_back(new AnetheronDisableTankActionsMultiplier(botAI));
    multipliers.push_back(new AnetheronDisableCombatFormationMoveMultiplier(botAI));
    multipliers.push_back(new AnetheronControlMisdirectionMultiplier(botAI));

    // Kaz'rogal
    multipliers.push_back(new KazrogalLowManaBotStayAwayFromGroupMultiplier(botAI));
    multipliers.push_back(new KazrogalKeepAspectOfTheViperActiveMultiplier(botAI));
    multipliers.push_back(new KazrogalControlMovementMultiplier(botAI));

    // Azgalor
    multipliers.push_back(new AzgalorDisableTankActionsMultiplier(botAI));
    multipliers.push_back(new AzgalorDoomedBotPrioritizePositioningMultiplier(botAI));
    multipliers.push_back(new AzgalorMeleeDpsControlAvoidanceMultiplier(botAI));

    // Archimonde
    multipliers.push_back(new ArchimondeDisableCombatFormationMoveMultiplier(botAI));
}
