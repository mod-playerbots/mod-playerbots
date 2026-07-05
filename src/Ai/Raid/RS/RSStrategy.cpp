/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "RSStrategy.h"

#include "RSMultipliers.h"

void RaidRsStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{

    triggers.push_back(new TriggerNode("rs baltharus brand",
        { NextAction("rs baltharus brand", ACTION_RAID + 7) }));
    triggers.push_back(new TriggerNode("rs baltharus tank position",
        { NextAction("rs baltharus tank position", ACTION_RAID + 6) }));
    triggers.push_back(new TriggerNode("rs baltharus avoid front",
        { NextAction("rs baltharus avoid front", ACTION_RAID + 5) }));
    triggers.push_back(new TriggerNode("rs baltharus healer position",
        { NextAction("rs baltharus healer position", ACTION_RAID + 4) }));

    triggers.push_back(new TriggerNode("rs saviana conflagration",
        { NextAction("rs saviana conflagration", ACTION_RAID + 7) }));
    triggers.push_back(new TriggerNode("rs saviana avoid front",
        { NextAction("rs saviana avoid front", ACTION_RAID + 6) }));
    triggers.push_back(new TriggerNode("rs saviana tank position",
        { NextAction("rs saviana tank position", ACTION_RAID + 7) }));
    triggers.push_back(new TriggerNode("rs saviana melee spread",
        { NextAction("rs saviana melee spread", ACTION_RAID + 7) }));

    triggers.push_back(new TriggerNode("rs zarithrian adds",
        { NextAction("rs zarithrian adds", ACTION_RAID + 5),
          NextAction("attack rti target", ACTION_RAID + 4) }));

    triggers.push_back(new TriggerNode("rs zarithrian tank",
        { NextAction("rs zarithrian tank", ACTION_RAID + 6) }));

    triggers.push_back(new TriggerNode("rs halion start position",
        { NextAction("rs halion start position", ACTION_RAID + 6) }));
    triggers.push_back(new TriggerNode("rs halion combustion",
        { NextAction("rs halion combustion", ACTION_RAID + 7) }));
    triggers.push_back(new TriggerNode("rs halion meteor",
        { NextAction("rs halion meteor", ACTION_RAID + 7) }));
    triggers.push_back(new TriggerNode("rs halion tank position",
        { NextAction("rs halion tank position", ACTION_RAID + 6),
          NextAction("attack rti target", ACTION_RAID + 5) }));
    triggers.push_back(new TriggerNode("rs halion avoid cones",
        { NextAction("rs halion avoid cones", ACTION_RAID + 5) }));

    triggers.push_back(new TriggerNode("rs halion adds",
        { NextAction("rs halion adds", ACTION_RAID + 6),
          NextAction("attack rti target", ACTION_RAID + 5) }));
    triggers.push_back(new TriggerNode("rs halion add tank",
        { NextAction("rs halion add tank", ACTION_RAID + 6) }));

    triggers.push_back(new TriggerNode("rs halion enter portal",
        { NextAction("rs halion enter portal", ACTION_RAID + 8) }));
    triggers.push_back(new TriggerNode("rs halion cutter",
        { NextAction("rs halion cutter", ACTION_RAID + 8) }));
    triggers.push_back(new TriggerNode("rs halion consumption",
        { NextAction("rs halion consumption", ACTION_RAID + 7) }));
    triggers.push_back(new TriggerNode("rs halion heal consumption",
        { NextAction("rs halion heal consumption", ACTION_RAID + 7) }));
    triggers.push_back(new TriggerNode("rs halion p2 tank position",
        { NextAction("rs halion p2 tank position", ACTION_RAID + 6),
          NextAction("attack rti target", ACTION_RAID + 5) }));
    triggers.push_back(new TriggerNode("rs halion p2 avoid cones",
        { NextAction("rs halion p2 avoid cones", ACTION_RAID + 5),
          NextAction("attack rti target", ACTION_RAID + 4) }));

    triggers.push_back(new TriggerNode("rs trash main tank",
        { NextAction("rs trash main tank", ACTION_RAID + 6) }));
    triggers.push_back(new TriggerNode("rs trash assist tank",
        { NextAction("rs trash assist tank", ACTION_RAID + 6) }));
    triggers.push_back(new TriggerNode("rs trash ranged",
        { NextAction("rs trash ranged", ACTION_RAID + 5) }));
    triggers.push_back(new TriggerNode("rs trash melee flank",
        { NextAction("rear flank", ACTION_RAID + 5) }));
    triggers.push_back(new TriggerNode("rs trash adds",
        { NextAction("rs trash adds", ACTION_RAID + 5),
          NextAction("attack rti target", ACTION_RAID + 4) }));
}

void RaidRsStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new RsBaltharusBrandSafeMultiplier(botAI));
    multipliers.push_back(new RsSavianaBeaconMultiplier(botAI));
    multipliers.push_back(new RsSavianaMeleeSpreadMultiplier(botAI));
    multipliers.push_back(new RsZarithrianAddsMultiplier(botAI));
    multipliers.push_back(new RsZarithrianTankSwapMultiplier(botAI));
    multipliers.push_back(new RsHalionCombustionMultiplier(botAI));
    multipliers.push_back(new RsHalionMeteorMultiplier(botAI));
    multipliers.push_back(new RsHalionMeleeFlankMultiplier(botAI));
    multipliers.push_back(new RsHalionP2Multiplier(botAI));
    multipliers.push_back(new RsHalionHpBalanceMultiplier(botAI));
    multipliers.push_back(new RsHalionRealmIsolationMultiplier(botAI));
    multipliers.push_back(new RsTrashAddsMultiplier(botAI));
}
