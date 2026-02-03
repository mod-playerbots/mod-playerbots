#include "TrialOfTheChampionStrategy.h"
#include "CreateNextAction.h"
#include "TrialOfTheChampionActions.h"

void WotlkDungeonToCStrategy::InitTriggers(std::vector<TriggerNode*> &triggers)
{
    triggers.push_back(
        new TriggerNode(
            "toc lance",
            {
                CreateNextAction<ToCLanceAction>(ACTION_RAID + 5.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "toc ue lance",
            {
                CreateNextAction<ToCUELanceAction>(ACTION_RAID + 2.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "toc mount near",
            {
                CreateNextAction<ToCMountAction>(ACTION_RAID + 4.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "toc mounted",
            {
                CreateNextAction<ToCMountedAction>(ACTION_RAID + 6.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "toc eadric",
            {
                CreateNextAction<ToCEadricAction>(ACTION_RAID + 3.0f)
            }
        )
    );

}

void WotlkDungeonToCStrategy::InitMultipliers(std::vector<Multiplier*>&)
{
}
