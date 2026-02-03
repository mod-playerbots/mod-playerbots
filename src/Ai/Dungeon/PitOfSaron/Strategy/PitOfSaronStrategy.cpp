#include "PitOfSaronStrategy.h"
#include "CreateNextAction.h"
#include "PitOfSaronActions.h"
#include "PitOfSaronMultipliers.h"

void WotlkDungeonPoSStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "ick and krick",
            {
                CreateNextAction<IckAndKrickAction>(ACTION_RAID + 5.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "tyrannus",
            {
                CreateNextAction<TyrannusAction>(ACTION_RAID + 5.0f)
            }
        )
    );
}

void WotlkDungeonPoSStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new IckAndKrickMultiplier(botAI));
}
