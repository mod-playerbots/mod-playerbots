#include "GundrakStrategy.h"
#include "GundrakMultipliers.h"

void WotlkDungeonGDStrategy::InitTriggers(std::vector<TriggerNode*> &triggers)
{
    // Moorabi

    // Drakkari Colossus

    // Slad'ran
    // TODO: Might need to add target priority for heroic on the snakes or to burn down boss.
    // Will re-test in heroic, decent dps groups should be able to blast him down with no funky strats.
    triggers.push_back(new TriggerNode("poison nova",
        { new NextAction("avoid poison nova", ACTION_RAID + 5) }));
    triggers.push_back(new TriggerNode("snake wrap",
        { new NextAction("attack snake wrap", ACTION_RAID + 4) }));

    // Gal'darah
    triggers.push_back(new TriggerNode("whirling slash",
        { new NextAction("avoid whirling slash", ACTION_RAID + 5) }));

    // Eck the Ferocious (Heroic only)
}

void WotlkDungeonGDStrategy::InitMultipliers(std::vector<Multiplier*> &multipliers)
{
    multipliers.push_back(new SladranMultiplier(botAI));
    multipliers.push_back(new GaldarahMultiplier(botAI));
}
