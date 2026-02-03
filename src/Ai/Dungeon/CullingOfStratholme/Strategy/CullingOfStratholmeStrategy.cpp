#include "CullingOfStratholmeStrategy.h"
#include "CreateNextAction.h"
#include "CullingOfStratholmeActions.h"
#include "CullingOfStratholmeMultipliers.h"

void WotlkDungeonCoSStrategy::InitTriggers(std::vector<TriggerNode*> &triggers)
{
    // Meathook
    // Can tank this in a fixed position to allow healer to LoS the stun, probably not necessary

    // Salramm the Fleshcrafter
    triggers.push_back(
        new TriggerNode(
            "explode ghoul",
            {
                CreateNextAction<ExplodeGhoulSpreadAction>(ACTION_MOVE + 5.0f)
            }
        )
    );

    // Chrono-Lord Epoch
    // Not sure if this actually works, I think I've seen him charge melee characters..?
    triggers.push_back(
        new TriggerNode(
            "epoch ranged",
            {
                CreateNextAction<EpochStackAction>(ACTION_MOVE + 5.0f)
            }
        )
    );

    // Mal'Ganis

    // Infinite Corruptor (Heroic only)
}

void WotlkDungeonCoSStrategy::InitMultipliers(std::vector<Multiplier*> &multipliers)
{
    multipliers.push_back(new EpochMultiplier(botAI));
}
