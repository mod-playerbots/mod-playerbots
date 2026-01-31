#include "RaidOnyxiaStrategy.h"
#include "RaidOnyxiaActions.h"
#include "CreateNextAction.h"

void RaidOnyxiaStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // ----------- Phase 1 (100% - 65%) -----------

    triggers.push_back(
        new TriggerNode(
            "ony near tail",
            {
                CreateNextAction<RaidOnyxiaMoveToSideAction>(ACTION_RAID + 2.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "ony avoid eggs",
            {
                CreateNextAction<OnyxiaAvoidEggsAction>(ACTION_EMERGENCY + 5.0f)
            }
        )
    );

    // ----------- Phase 2 (65% - 40%) -----------

    triggers.push_back(
        new TriggerNode(
            "ony deep breath warning",
            {
                CreateNextAction<RaidOnyxiaMoveToSafeZoneAction>(ACTION_EMERGENCY + 5.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "ony fireball splash incoming",
            {
                CreateNextAction<RaidOnyxiaSpreadOutAction>(ACTION_EMERGENCY + 2.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "ony whelps spawn",
            {
                CreateNextAction<RaidOnyxiaKillWhelpsAction>(ACTION_RAID + 1.0f)
            }
        )
    );
}

void RaidOnyxiaStrategy::InitMultipliers(std::vector<Multiplier*>&)
{
    // Empty for now
}
