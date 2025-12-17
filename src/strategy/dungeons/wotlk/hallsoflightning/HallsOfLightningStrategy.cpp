#include "HallsOfLightningStrategy.h"
#include "HallsOfLightningMultipliers.h"

void WotlkDungeonHoLStrategy::InitTriggers(std::vector<TriggerNode*> &triggers)
{
    // General Bjarngrim
    triggers.push_back(new TriggerNode("stormforged lieutenant",
        { new NextAction("bjarngrim target", ACTION_RAID + 5) }));
    triggers.push_back(new TriggerNode("whirlwind",
        { new NextAction("avoid whirlwind", ACTION_RAID + 4) }));

    // Volkhan
    triggers.push_back(new TriggerNode("volkhan",
        { new NextAction("volkhan target", ACTION_RAID + 5) }));

    // Ionar
    triggers.push_back(new TriggerNode("ionar disperse",
        { new NextAction("disperse position", ACTION_MOVE + 5) }));
    triggers.push_back(new TriggerNode("ionar tank aggro",
        { new NextAction("ionar tank position", ACTION_MOVE + 4) }));
    triggers.push_back(new TriggerNode("static overload",
        { new NextAction("static overload spread", ACTION_MOVE + 3) }));
    // TODO: Targeted player can dodge the ball, but a single player soaking it isn't too bad to heal
    triggers.push_back(new TriggerNode("ball lightning",
        { new NextAction("ball lightning spread", ACTION_MOVE + 2) }));

    // Loken
    triggers.push_back(new TriggerNode("lightning nova",
        { new NextAction("avoid lightning nova", ACTION_MOVE + 5) }));
    triggers.push_back(new TriggerNode("loken ranged",
        { new NextAction("loken stack", ACTION_MOVE + 4) }));
}

void WotlkDungeonHoLStrategy::InitMultipliers(std::vector<Multiplier*> &multipliers)
{
    multipliers.push_back(new BjarngrimMultiplier(botAI));
    multipliers.push_back(new VolkhanMultiplier(botAI));
    multipliers.push_back(new IonarMultiplier(botAI));
    multipliers.push_back(new LokenMultiplier(botAI));
}
