#include "RaidMagtheridonStrategy.h"
#include "RaidMagtheridonMultipliers.h"

void RaidMagtheridonStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("magtheridon incoming blast nova", NextAction::array(0,
        new NextAction("magtheridon use manticron cube", ACTION_EMERGENCY + 10), nullptr)));

    triggers.push_back(new TriggerNode("magtheridon need to manage timers and assignments", NextAction::array(0,
        new NextAction("magtheridon manage timers and assignments", ACTION_EMERGENCY + 1), nullptr)));

    triggers.push_back(new TriggerNode("magtheridon burning abyssal spawned", NextAction::array(0,
        new NextAction("magtheridon warlock cc burning abyssal", ACTION_RAID + 3), nullptr)));

    triggers.push_back(new TriggerNode("magtheridon boss engaged by ranged", NextAction::array(0,
        new NextAction("magtheridon spread ranged", ACTION_RAID + 2), nullptr)));

    triggers.push_back(new TriggerNode("magtheridon pulling west and east channelers", NextAction::array(0,
        new NextAction("magtheridon misdirect hellfire channelers", ACTION_RAID + 2), nullptr)));

    triggers.push_back(new TriggerNode("magtheridon boss engaged by main tank", NextAction::array(0,
        new NextAction("magtheridon main tank position boss", ACTION_RAID + 2), nullptr)));

    triggers.push_back(new TriggerNode("magtheridon first three channelers engaged by main tank", NextAction::array(0,
        new NextAction("magtheridon main tank attack first three channelers", ACTION_RAID + 1), nullptr)));

    triggers.push_back(new TriggerNode("magtheridon nw channeler engaged by first assist tank", NextAction::array(0,
        new NextAction("magtheridon first assist tank attack nw channeler", ACTION_RAID + 1), nullptr)));

    triggers.push_back(new TriggerNode("magtheridon ne channeler engaged by second assist tank", NextAction::array(0,
        new NextAction("magtheridon second assist tank attack ne channeler", ACTION_RAID + 1), nullptr)));

    triggers.push_back(new TriggerNode("magtheridon determining kill order", NextAction::array(0,
        new NextAction("magtheridon assign dps priority", ACTION_RAID + 1), nullptr)));
}

void RaidMagtheridonStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new MagtheridonUseManticronCubeMultiplier(botAI));
    multipliers.push_back(new MagtheridonWaitToAttackMultiplier(botAI));
    multipliers.push_back(new MagtheridonDisableOffTankAssistMultiplier(botAI));
}
