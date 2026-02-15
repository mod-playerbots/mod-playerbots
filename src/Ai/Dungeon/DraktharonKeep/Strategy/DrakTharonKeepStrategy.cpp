#include "DrakTharonKeepStrategy.h"
#include "CreateNextAction.h"
#include "DrakTharonKeepActions.h"
#include "DrakTharonKeepMultipliers.h"
#include "ReachTargetActions.h"

void WotlkDungeonDTKStrategy::InitTriggers(std::vector<TriggerNode*> &triggers)
{
    // Trollgore
    triggers.push_back(
        new TriggerNode(
            "corpse explode",
            {
                CreateNextAction<CorpseExplodeSpreadAction>(ACTION_MOVE + 5.0f)
            }
        )
    );

    // Novos the Summoner
    // TODO: Can be improved - it's a pretty easy fight but complex to program, revisit if needed
    triggers.push_back(
        new TriggerNode(
            "arcane field",
            {
                CreateNextAction<AvoidArcaneFieldAction>(ACTION_MOVE + 5.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "arcane field",
            {
                CreateNextAction<NovosDefaultPositionAction>(ACTION_MOVE + 4.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "arcane field",
            {
                CreateNextAction<NovosTargetPriorityAction>(ACTION_NORMAL + 1.0f)
            }
        )
    );

    // King Dred
    // TODO: Fear ward / tremor totem, or general anti-fear strat development

    //The Prophet Tharon'ja
    triggers.push_back(
        new TriggerNode(
            "gift of tharon'ja",
            {
                CreateNextAction<CastTouchOfLifeAction>(ACTION_NORMAL + 5.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "gift of tharon'ja",
            {
                CreateNextAction<CastBoneArmorAction>(ACTION_NORMAL + 4.0f)
            }
        )
    );
    // Run ranged chars (who would normally stand at range) into melee, to dps in skeleton form
    triggers.push_back(
        new TriggerNode(
            "tharon'ja out of melee",
            {
                CreateNextAction<ReachMeleeAction>(ACTION_NORMAL + 3.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "gift of tharon'ja",
            {
                CreateNextAction<CastTauntAction>(ACTION_NORMAL + 2.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "gift of tharon'ja",
            {
                CreateNextAction<CastSlayingStrikeAction>(ACTION_NORMAL + 2.0f)
            }
        )
    );
}

void WotlkDungeonDTKStrategy::InitMultipliers(std::vector<Multiplier*> &multipliers)
{
    multipliers.push_back(new NovosMultiplier(botAI));
    multipliers.push_back(new TharonjaMultiplier(botAI));
}
