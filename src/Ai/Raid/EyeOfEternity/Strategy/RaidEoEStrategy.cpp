#include "RaidEoEStrategy.h"
#include "RaidEoEActions.h"
#include "RaidEoEMultipliers.h"
#include "Strategy.h"

void RaidEoEStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "malygos",
            {
                CreateNextAction<MalygosPositionAction>(ACTION_MOVE)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "malygos",
            {
                CreateNextAction<MalygosTargetAction>(ACTION_RAID + 1.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "group flying",
            {
                CreateNextAction<EoEFlyDrakeAction>(ACTION_NORMAL + 1.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "drake combat",
            {
                CreateNextAction<EoEDrakeAttackAction>(ACTION_NORMAL + 5.0f)
            }
        )
    );
}

void RaidEoEStrategy::InitMultipliers(std::vector<Multiplier*> &multipliers)
{
    multipliers.push_back(new MalygosMultiplier(botAI));
}
