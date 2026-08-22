/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "BWLStrategy.h"
#include "BWLMultipliers.h"

void RaidBwlStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("often", {
        NextAction("bwl check onyxia scale cloak", ACTION_RAID) }));
    triggers.push_back(new TriggerNode("bwl suppression device", {
        NextAction("bwl turn off suppression device", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("bwl razorgore fire resistance", {
        NextAction("bwl razorgore fire resistance", ACTION_RAID) }));
    triggers.push_back(new TriggerNode("bwl razorgore not mind controlled", {
        NextAction("bwl razorgore avoid aoe", ACTION_RAID) }));
    triggers.push_back(new TriggerNode("bwl razorgore not mind controlled", {
        NextAction("bwl razorgore mark boss", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("bwl vaelastrasz fire resistance", {
        NextAction("bwl vaelastrasz fire resistance", ACTION_RAID) }));
    triggers.push_back(new TriggerNode("bwl vaelastrasz positioning", {
        NextAction("rear flank", ACTION_MOVE + 4) }));
    triggers.push_back(new TriggerNode("bwl vaelastrasz burning adrenaline", {
        NextAction("bwl vaelastrasz move away", ACTION_RAID + 5) }));

    triggers.push_back(new TriggerNode("bwl broodlord fire resistance", {
        NextAction("bwl broodlord fire resistance", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("bwl firemaw fire resistance", {
        NextAction("bwl firemaw fire resistance", ACTION_RAID) }));
    triggers.push_back(new TriggerNode("bwl flamegor fire resistance", {
        NextAction("bwl flamegor fire resistance", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("bwl affliction bronze", {
        NextAction("bwl use hourglass sand", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("bwl wild magic", {
        NextAction("ice block", ACTION_RAID) }));
    triggers.push_back(new TriggerNode("bwl nefarian fear ward", {
        NextAction("bwl nefarian fear ward", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("bwl death talon wyrmguard tank", {
        NextAction("bwl death talon wyrmguard tank move away", ACTION_RAID) }));
    triggers.push_back(new TriggerNode("bwl death talon wyrmguard ranged", {
        NextAction("bwl death talon wyrmguard ranged move away", ACTION_RAID) }));
}

void RaidBwlStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new RazorgoreTankMultiplier(botAI));
    multipliers.push_back(new VaelastraszTankMultiplier(botAI));
    multipliers.push_back(new VaelastraszBurningAdrenalineMultiplier(botAI));
}
