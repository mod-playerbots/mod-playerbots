#include "ForgeOfSoulsStrategy.h"
#include "CreateNextAction.h"
#include "ForgeOfSoulsActions.h"
#include "ForgeOfSoulsMultipliers.h"

void WotlkDungeonFoSStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "move from bronjahm",
            {
                CreateNextAction<MoveFromBronjahmAction>(ACTION_MOVE + 5.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "switch to soul fragment",
            {
                CreateNextAction<AttackCorruptedSoulFragmentAction>(ACTION_RAID + 2.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "bronjahm position",
            {
                CreateNextAction<BronjahmGroupPositionAction>(ACTION_RAID + 1.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "devourer of souls",
            {
                CreateNextAction<DevourerOfSoulsAction>(ACTION_RAID + 1.0f)
            }
        )
    );
}

void WotlkDungeonFoSStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new BronjahmMultiplier(botAI));
}
