#include "RaidBwlStrategy.h"

#include "CreateNextAction.h"
#include "RaidBwlActions.h"
#include "Strategy.h"

void RaidBwlStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "often",
            {
                CreateNextAction<BwlOnyxiaScaleCloakAuraCheckAction>(ACTION_RAID)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "bwl suppression device",
            {
                CreateNextAction<BwlTurnOffSuppressionDeviceAction>(ACTION_RAID)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "bwl affliction bronze",
            {
                CreateNextAction<BwlUseHourglassSandAction>(ACTION_RAID)
            }
        )
    );
}
