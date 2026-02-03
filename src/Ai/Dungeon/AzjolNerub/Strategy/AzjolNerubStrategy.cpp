#include "AzjolNerubStrategy.h"
#include "AzjolNerubActions.h"
#include "AzjolNerubMultipliers.h"
#include "CreateNextAction.h"

void WotlkDungeonANStrategy::InitTriggers(std::vector<TriggerNode*> &triggers)
{
    // Krik'thir the Gatewatcher
    // TODO: Add CC trigger while web wraps are casting?
    // TODO: Bring healer closer than ranged dps to avoid fixates?
    triggers.push_back(
        new TriggerNode(
            "krik'thir web wrap",
            {
                CreateNextAction<AttackWebWrapAction>(ACTION_RAID + 5.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "krik'thir watchers",
            {
                CreateNextAction<WatchersTargetAction>(ACTION_RAID + 4.0f)
            }
        )
    );

    // Hadronox
    // The core AC triggers are very buggy with this boss, but default strat seems to play correctly

    //Anub'arak
    // TODO: No clear way to track these spikes. They don't seem to appear as gameobjects or triggers,
    // and cast time is instant so no way to check currently casting location.
    // May need to hook boss AI.. might be able to just heal through it for now.
    // triggers.push_back(
    // new TriggerNode(
    // "anub'arak impale",
    //     { NextAction("TODO", ACTION_MOVE + 5) }));
    triggers.push_back(
        new TriggerNode(
            "anub'arak pound",
            {
                CreateNextAction<AnubarakDodgePoundAction>(ACTION_MOVE + 5.0f)
            }
        )
    );
}

void WotlkDungeonANStrategy::InitMultipliers(std::vector<Multiplier*> &multipliers)
{
    multipliers.push_back(new KrikthirMultiplier(botAI));
}
