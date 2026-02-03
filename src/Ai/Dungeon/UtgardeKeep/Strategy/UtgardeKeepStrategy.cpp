#include "UtgardeKeepStrategy.h"
#include "CreateNextAction.h"
#include "UtgardeKeepActions.h"
#include "UtgardeKeepMultipliers.h"

void WotlkDungeonUKStrategy::InitTriggers(std::vector<TriggerNode*> &triggers)
{
    // Prince Keleseth
    triggers.push_back(
        new TriggerNode(
            "keleseth frost tomb",
            {
                CreateNextAction<AttackFrostTombAction>(ACTION_RAID + 1.0f)
            }
        )
    );

    // Skarvald the Constructor & Dalronn the Controller
    triggers.push_back(
        new TriggerNode(
            "dalronn priority",
            {
                CreateNextAction<AttackDalronnAction>(ACTION_RAID + 1.0f)
            }
        )
    );

    // Ingvar the Plunderer

    // Doesn't work yet, this action doesn't get processed until the existing cast finishes
    // triggers.push_back(
    // new TriggerNode(
    // "ingvar staggering roar",
    //                  {
    // CreateNextAction("ingvar stop casting", ACTION_RAID + 1)
    // }
    // )
    // );

    // No easy way to check LoS here, the pillars do not seem to count as gameobjects.
    // Not implemented for now, unsure if this is needed as a good group can probably burst through the boss
    // and just eat the debuff.
    // triggers.push_back(
    // new TriggerNode(
    // "ingvar dreadful roar",
    //                  {
    // CreateNextAction("ingvar hide los", ACTION_RAID + 1)
    // }
    // )
    // );
    triggers.push_back(
        new TriggerNode(
            "ingvar smash tank",
            {
                CreateNextAction<IngvarDodgeSmashAction>(ACTION_MOVE + 5.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "ingvar smash tank return",
            {
                CreateNextAction<IngvarSmashReturnAction>(ACTION_MOVE + 5.0f)
            }
        )
    );
    // Buggy... if not behind target, ai can get stuck running towards and away from target.
    // I think for ranged chars, a custom action should be added that doesn't attempt to run into melee.
    // This is a bandaid for now, needs to be improved.
    triggers.push_back(
        new TriggerNode(
            "not behind ingvar",
            {
                CreateNextAction<SetBehindTargetAction>(ACTION_MOVE + 1.0f)
            }
        )
    );

}

void WotlkDungeonUKStrategy::InitMultipliers(std::vector<Multiplier*> &multipliers)
{
    multipliers.push_back(new PrinceKelesethMultiplier(botAI));
    multipliers.push_back(new SkarvaldAndDalronnMultiplier(botAI));
    multipliers.push_back(new IngvarThePlundererMultiplier(botAI));
}
