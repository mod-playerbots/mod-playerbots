#include "RaidOsStrategy.h"
#include "CreateNextAction.h"
#include "RaidOsActions.h"
#include "RaidOsMultipliers.h"
#include "MovementActions.h"

void RaidOsStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "sartharion tank",
            {
                CreateNextAction<SartharionTankPositionAction>(ACTION_MOVE)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "twilight fissure",
            {
                CreateNextAction<AvoidTwilightFissureAction>(ACTION_RAID + 2.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "flame tsunami",
            {
                CreateNextAction<AvoidFlameTsunamiAction>(ACTION_RAID + 1.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "sartharion dps",
            {
                CreateNextAction<SartharionAttackPriorityAction>(ACTION_RAID)
            }
        )
    );
    // Flank dragon positioning
    triggers.push_back(
        new TriggerNode(
            "sartharion melee positioning",
            {
                CreateNextAction<RearFlankAction>(ACTION_MOVE + 4.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "twilight portal enter",
            {
                CreateNextAction<EnterTwilightPortalAction>(ACTION_RAID + 1.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
        "twilight portal exit",
            {
                CreateNextAction<ExitTwilightPortalAction>(ACTION_RAID + 1.0f)
            }
        )
    );
}

void RaidOsStrategy::InitMultipliers(std::vector<Multiplier*> &multipliers)
{
    multipliers.push_back(new SartharionMultiplier(botAI));
}
