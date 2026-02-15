#include "HallsOfLightningStrategy.h"
#include "CreateNextAction.h"
#include "HallsOfLightningActions.h"
#include "HallsOfLightningMultipliers.h"

void WotlkDungeonHoLStrategy::InitTriggers(std::vector<TriggerNode*> &triggers)
{
    // General Bjarngrim
    triggers.push_back(
        new TriggerNode(
            "stormforged lieutenant",
            {
                CreateNextAction<BjarngrimTargetAction>(ACTION_RAID + 5.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "whirlwind",
            {
                CreateNextAction<AvoidWhirlwindAction>(ACTION_RAID + 4.0f)
            }
        )
    );

    // Volkhan
    triggers.push_back(
        new TriggerNode(
            "volkhan",
            {
                CreateNextAction<VolkhanTargetAction>(ACTION_RAID + 5.0f)
            }
        )
    );

    // Ionar
    triggers.push_back(
        new TriggerNode(
            "ionar disperse",
            {
                CreateNextAction<DispersePositionAction>(ACTION_MOVE + 5.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "ionar tank aggro",
            {
                CreateNextAction<IonarTankPositionAction>(ACTION_MOVE + 4.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "static overload",
            {
                CreateNextAction<StaticOverloadSpreadAction>(ACTION_MOVE + 3.0f)
            }
        )
    );
    // TODO: Targeted player can dodge the ball, but a single player soaking it isn't too bad to heal
    triggers.push_back(
        new TriggerNode(
            "ball lightning",
            {
                CreateNextAction<BallLightningSpreadAction>(ACTION_MOVE + 2.0f)
            }
        )
    );

    // Loken
    triggers.push_back(
        new TriggerNode(
            "lightning nova",
            {
                CreateNextAction<AvoidLightningNovaAction>(ACTION_MOVE + 5.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "loken ranged",
            {
                CreateNextAction<LokenStackAction>(ACTION_MOVE + 4.0f)
            }
        )
    );
}

void WotlkDungeonHoLStrategy::InitMultipliers(std::vector<Multiplier*> &multipliers)
{
    multipliers.push_back(new BjarngrimMultiplier(botAI));
    multipliers.push_back(new VolkhanMultiplier(botAI));
    multipliers.push_back(new IonarMultiplier(botAI));
    multipliers.push_back(new LokenMultiplier(botAI));
}
