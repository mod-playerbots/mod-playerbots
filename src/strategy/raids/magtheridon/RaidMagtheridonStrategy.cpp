#include "RaidMagtheridonStrategy.h"
#include "RaidMagtheridonMultipliers.h"

void RaidMagtheridonStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("magtheridon use manticron cube", NextAction::array(0,
        new NextAction("magtheridon use manticron cube", ACTION_EMERGENCY + 10), nullptr)));

    triggers.push_back(new TriggerNode("magtheridon manage timers and assignments", NextAction::array(0,
        new NextAction("magtheridon manage timers and assignments", ACTION_EMERGENCY), nullptr)));

    triggers.push_back(new TriggerNode("magtheridon burning abyssal warlock cc", NextAction::array(0,
        new NextAction("magtheridon burning abyssal warlock cc", ACTION_RAID + 3), nullptr)));

    triggers.push_back(new TriggerNode("magtheridon spread ranged", NextAction::array(0,
        new NextAction("magtheridon spread ranged", ACTION_RAID + 2), nullptr)));

    triggers.push_back(new TriggerNode("magtheridon hellfire channeler misdirection", NextAction::array(0,
        new NextAction("magtheridon hellfire channeler misdirection", ACTION_RAID + 2), nullptr)));

    triggers.push_back(new TriggerNode("magtheridon position boss", NextAction::array(0,
        new NextAction("magtheridon position boss", ACTION_RAID + 2), nullptr)));

    triggers.push_back(new TriggerNode("magtheridon hellfire channeler main tank", NextAction::array(0,
        new NextAction("magtheridon hellfire channeler main tank", ACTION_RAID + 1), nullptr)));

    triggers.push_back(new TriggerNode("magtheridon hellfire channeler nw channeler tank", NextAction::array(0,
        new NextAction("magtheridon hellfire channeler nw channeler tank", ACTION_RAID + 1), nullptr)));

    triggers.push_back(new TriggerNode("magtheridon hellfire channeler ne channeler tank", NextAction::array(0,
        new NextAction("magtheridon hellfire channeler ne channeler tank", ACTION_RAID + 1), nullptr)));

    triggers.push_back(new TriggerNode("magtheridon dps priority", NextAction::array(0,
        new NextAction("magtheridon dps priority", ACTION_RAID + 1), nullptr)));
}

void RaidMagtheridonStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new MagtheridonUseManticronCubeMultiplier(botAI));
    multipliers.push_back(new MagtheridonWaitToAttackMultiplier(botAI));
    multipliers.push_back(new MagtheridonDisableOffTankAssistMultiplier(botAI));
}
