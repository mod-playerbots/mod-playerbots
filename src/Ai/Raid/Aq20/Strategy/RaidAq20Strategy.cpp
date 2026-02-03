#include "RaidAq20Strategy.h"

#include "CreateNextAction.h"
#include "RaidAq20Actions.h"
#include "Strategy.h"

void RaidAq20Strategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "aq20 move to crystal",
            {
                CreateNextAction<Aq20UseCrystalAction>(ACTION_RAID)
            }
        )
    );
}
