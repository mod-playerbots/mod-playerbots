#include "TrialOfTheChampionStrategy.h"
#include "TrialOfTheChampionMultipliers.h"

void WotlkDungeonToCStrategy::InitTriggers(std::vector<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode("toc lance",
        { new NextAction("toc lance", ACTION_RAID + 5) }));
    triggers.push_back(new TriggerNode("toc ue lance",
        { new NextAction("toc ue lance", ACTION_RAID + 2) }));
    triggers.push_back(new TriggerNode("toc mount near",
        { new NextAction("toc mount", ACTION_RAID + 4) }));
    triggers.push_back(new TriggerNode("toc mounted",
        { new NextAction("toc mounted", ACTION_RAID + 6) }));
    triggers.push_back(new TriggerNode("toc eadric",
        { new NextAction("toc eadric", ACTION_RAID + 3) }));

}

void WotlkDungeonToCStrategy::InitMultipliers(std::vector<Multiplier*> &multipliers)
{
}
