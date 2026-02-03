#include "UtgardePinnacleStrategy.h"
#include "ChooseTargetActions.h"
#include "CreateNextAction.h"
#include "UtgardePinnacleActions.h"
#include "UtgardePinnacleMultipliers.h"

void WotlkDungeonUPStrategy::InitTriggers(std::vector<TriggerNode*> &triggers)
{
    // Svala Sorrowgrave

    // Gortok Palehoof

    // Skadi the Ruthless
    // TODO: Harpoons launchable via GameObject. For now players should do them
    triggers.push_back(
        new TriggerNode(
            "freezing cloud",
            {
                CreateNextAction<AvoidFreezingCloudAction>(ACTION_RAID + 5.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "skadi whirlwind",
            {
                CreateNextAction<AvoidSkadiWhirlwindAction>(ACTION_RAID + 4.0f)
            }
        )
    );

    // King Ymiron
    // May need to avoid orb.. unclear if the generic avoid AoE does this well
    triggers.push_back(
        new TriggerNode(
            "ymiron bane",
            {
                CreateNextAction<DropTargetAction>(ACTION_RAID + 5.0f)
            }
        )
    );
}

void WotlkDungeonUPStrategy::InitMultipliers(std::vector<Multiplier*> &multipliers)
{
    multipliers.push_back(new SkadiMultiplier(botAI));
    multipliers.push_back(new YmironMultiplier(botAI));
}
