/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "RaidZulAmanStrategy.h"
#include "RaidZulAmanMultipliers.h"

void RaidZulAmanStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // Trash
    triggers.push_back(new TriggerNode("amani'shi medicine man summoned ward", {
        NextAction("amani'shi medicine man mark ward", ACTION_RAID + 1) }));

    // Akil'zon <Eagle Avatar>
    triggers.push_back(new TriggerNode("akil'zon pulling boss", {
        NextAction("akil'zon misdirect boss to main tank", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode("akil'zon boss engaged by main tank", {
        NextAction("akil'zon main tank position boss", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("akil'zon boss casts static disruption", {
        NextAction("akil'zon spread ranged", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("akil'zon electrical storm incoming", {
        NextAction("akil'zon move to eye of the storm", ACTION_EMERGENCY + 6) }));

    triggers.push_back(new TriggerNode("akil'zon bots need to prepare for electrical storm", {
        NextAction("akil'zon manage electrical storm timer", ACTION_EMERGENCY + 10) }));

    // Nalorakk <Bear Avatar>
    triggers.push_back(new TriggerNode("nalorakk pulling boss", {
        NextAction("nalorakk misdirect boss to main tank", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("nalorakk boss switches forms", {
        NextAction("nalorakk tanks position boss", ACTION_EMERGENCY + 1) }));

    triggers.push_back(new TriggerNode("nalorakk boss casts surge", {
        NextAction("nalorakk spread ranged", ACTION_RAID + 1) }));

    // Jan'alai <Dragonhawk Avatar>
    triggers.push_back(new TriggerNode("jan'alai pulling boss", {
        NextAction("jan'alai misdirect boss to main tank", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode("jan'alai boss engaged by main tank", {
        NextAction("jan'alai main tank position boss", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("jan'alai boss casts flame breath", {
        NextAction("jan'alai spread ranged in circle", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("jan'alai boss summoning fire bombs", {
        NextAction("jan'alai avoid fire bombs", ACTION_EMERGENCY + 6) }));

    triggers.push_back(new TriggerNode("jan'alai amani'shi hatchers spawned", {
        NextAction("jan'alai mark amani'shi hatchers", ACTION_RAID + 2) }));

    // Halazzi <Lynx Avatar>
    triggers.push_back(new TriggerNode("halazzi pulling boss", {
        NextAction("halazzi misdirect boss to main tank", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode("halazzi boss engaged by main tank", {
        NextAction("halazzi main tank position boss", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("halazzi boss summons spirit lynx", {
        NextAction("halazzi first assist tank attack spirit lynx", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("halazzi determining dps target", {
        NextAction("halazzi assign dps priority", ACTION_RAID + 1) }));

    // Hex Lord Malacrass
    triggers.push_back(new TriggerNode("hex lord malacrass pulling boss", {
        NextAction("hex lord malacrass misdirect boss to main tank", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode("hex lord malacrass determining kill order", {
        NextAction("hex lord malacrass assign dps priority", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("hex lord malacrass boss is channeling whirlwind", {
        NextAction("hex lord malacrass run away from whirlwind", ACTION_EMERGENCY + 6) }));

    triggers.push_back(new TriggerNode("hex lord malacrass boss has spell reflection", {
        NextAction("hex lord malacrass casters stop attacking", ACTION_EMERGENCY + 6) }));

    triggers.push_back(new TriggerNode("hex lord malacrass boss placed freezing trap", {
        NextAction("hex lord malacrass move away from freezing trap", ACTION_EMERGENCY + 1) }));

    // Zul'jin
    triggers.push_back(new TriggerNode("zul'jin main tank needs aggro upon pull or phase change", {
        NextAction("zul'jin misdirect boss to main tank", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode("zul'jin boss engaged by main tank", {
        NextAction("zul'jin main tank position boss", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("zul'jin boss is channeling whirlwind in troll form", {
        NextAction("zul'jin run away from whirlwind", ACTION_EMERGENCY + 6) }));

    triggers.push_back(new TriggerNode("zul'jin boss is summoning cyclones in eagle form", {
        NextAction("zul'jin avoid cyclones", ACTION_EMERGENCY + 1) }));

    triggers.push_back(new TriggerNode("zul'jin boss casts aoe abilities in dragonhawk form", {
        NextAction("zul'jin spread ranged", ACTION_RAID + 1) }));
}

void RaidZulAmanStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    // Akil'zon <Eagle Avatar>
    multipliers.push_back(new AkilzonDisableCombatFormationMoveMultiplier(botAI));
    multipliers.push_back(new AkilzonStayInEyeOfTheStormMultiplier(botAI));

    // Nalorakk <Bear Avatar>
    multipliers.push_back(new NalorakkDisableTankActionsMultiplier(botAI));
    multipliers.push_back(new NalorakkControlMisdirectionMultiplier(botAI));

    // Jan'alai <Dragonhawk Avatar>
    multipliers.push_back(new JanalaiDisableTankActionsMultiplier(botAI));
    multipliers.push_back(new JanalaiDisableCombatFormationMoveMultiplier(botAI));
    multipliers.push_back(new JanalaiStayAwayFromFireBombsMultiplier(botAI));
    multipliers.push_back(new JanalaiDoNotCrowdControlHatchersMultiplier(botAI));
    multipliers.push_back(new JanalaiDelayBloodlustAndHeroismMultiplier(botAI));

    // Halazzi <Lynx Avatar>
    multipliers.push_back(new HalazziDisableTankActionsMultiplier(botAI));
    multipliers.push_back(new HalazziControlMisdirectionMultiplier(botAI));

    // Hex Lord Malacrass
    multipliers.push_back(new HexLordMalacrassAvoidWhirlwindMultiplier(botAI));
    multipliers.push_back(new HexLordMalacrassStopAttackingDuringSpellReflectionMultiplier(botAI));
    multipliers.push_back(new HexLordMalacrassDoNotDispelUnstableAfflictionMultiplier(botAI));

    // Zul'jin
    multipliers.push_back(new ZuljinDisableTankFaceMultiplier(botAI));
    multipliers.push_back(new ZuljinAvoidWhirlwindMultiplier(botAI));
    multipliers.push_back(new ZuljinDisableAvoidAoeMultiplier(botAI));
    multipliers.push_back(new ZuljinDelayBloodlustAndHeroismMultiplier(botAI));
}
