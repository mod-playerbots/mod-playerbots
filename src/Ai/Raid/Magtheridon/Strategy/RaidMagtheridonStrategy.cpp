#include "RaidMagtheridonStrategy.h"
#include "RaidMagtheridonMultipliers.h"

void RaidMagtheridonStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("magtheridon incoming blast nova", {
        NextAction("magtheridon use manticron cube", ACTION_EMERGENCY + 10) }));

    triggers.push_back(new TriggerNode("magtheridon need to manage timers and assignments", {
        NextAction("magtheridon manage timers and assignments", ACTION_EMERGENCY + 1) }));

    triggers.push_back(new TriggerNode("magtheridon burning abyssal spawned", {
        NextAction("magtheridon warlock cc burning abyssal", ACTION_RAID + 3) }));

    triggers.push_back(new TriggerNode("magtheridon boss engaged by ranged", {
        NextAction("magtheridon spread ranged", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode("magtheridon pulling west and east channelers", {
        NextAction("magtheridon misdirect hellfire channelers", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode("magtheridon boss engaged by main tank", {
        NextAction("magtheridon main tank position boss", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode("magtheridon first three channelers engaged by main tank", {
        NextAction("magtheridon main tank attack first three channelers", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("magtheridon nw channeler engaged by first assist tank", {
        NextAction("magtheridon first assist tank attack nw channeler", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("magtheridon ne channeler engaged by second assist tank", {
        NextAction("magtheridon second assist tank attack ne channeler", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("magtheridon determining kill order", {
        NextAction("magtheridon assign dps priority", ACTION_RAID + 1) }));
}

void RaidMagtheridonStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new MagtheridonUseManticronCubeMultiplier(botAI));
    multipliers.push_back(new MagtheridonWaitToAttackMultiplier(botAI));
    multipliers.push_back(new MagtheridonDisableOffTankAssistMultiplier(botAI));
}
