/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "SethStrategy.h"
#include "SethMultipliers.h"
#include "SethTriggers.h"

void TbcDungeonSethekkHallsStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("time-lost controller drops charming totem", {
        NextAction("time-lost controller mark charming totem with skull", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("sethekk prophet casts fear", {
        NextAction("sethekk prophet set tremor totem", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("darkweaver syth boss summons elementals", {
        NextAction("darkweaver syth mark elementals with skull", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("anzu encounter has two phases", {
        NextAction("anzu alternate marks on boss", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("anzu bird spirits provide buffs", {
        NextAction("anzu cast heal over time spell on bird spirit", ACTION_HIGH) }));

    triggers.push_back(new TriggerNode("talon king ikiss boss engaged by tank", {
        NextAction("talon king ikiss tank move boss to pillar position", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("talon king ikiss ranged prepare for arcane explosion", {
        NextAction("talon king ikiss ranged stay near victim of boss", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("talon king ikiss boss casting arcane explosion", {
        NextAction("talon king ikiss los arcane explosion", ACTION_EMERGENCY + 10) }));

    triggers.push_back(new TriggerNode("talon king ikiss boss out of los", {
        NextAction("talon king ikiss move to within los", ACTION_RAID + 1) }));
}

void TbcDungeonSethekkHallsStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new SethekkProphetSetTremorTotemMultiplier(botAI));
    multipliers.push_back(new AnzuControlSpellCastingWithSpellBombMultiplier(botAI));
    multipliers.push_back(new TalonKingIkissDelayBloodlustAndHeroismMultiplier(botAI));
    multipliers.push_back(new TalonKingIkissControlMovementMultiplier(botAI));
}
