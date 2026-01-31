#include "RaidVoAStrategy.h"
#include "Trigger.h"
#include "vector"

#include "CreateNextAction.h"
#include "RaidVoAActions.h"
#include "BossAuraActions.h"

void RaidVoAStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    //
    // Emalon the Storm Watcher
    //
    triggers.push_back(
        new TriggerNode(
            "emalon lighting nova trigger",
            {
                CreateNextAction<EmalonLightingNovaAction>(ACTION_RAID + 1.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "emalon mark boss trigger",
            {
                CreateNextAction<EmalonMarkBossAction>(ACTION_RAID)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "emalon overcharge trigger",
            {
                CreateNextAction<EmalonOverchargeAction>(ACTION_RAID)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "emalon fall from floor trigger",
            {
                CreateNextAction<EmalonFallFromFloorAction>(ACTION_RAID)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "emalon nature resistance trigger",
            {
                CreateNextAction<BossNatureResistanceAction>(ACTION_RAID)
            }
        )
    );

    //
    // Koralon the Flame Watcher
    //

    triggers.push_back(
        new TriggerNode(
            "koralon fire resistance trigger",
            {
                CreateNextAction<BossFireResistanceAction>(ACTION_RAID)
            }
        )
    );
}
