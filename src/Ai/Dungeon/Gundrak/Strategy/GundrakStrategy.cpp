#include "GundrakStrategy.h"
#include "CreateNextAction.h"
#include "GundrakActions.h"
#include "GundrakMultipliers.h"

void WotlkDungeonGDStrategy::InitTriggers(std::vector<TriggerNode*> &triggers)
{
    // Moorabi

    // Drakkari Colossus

    // Slad'ran
    // TODO: Might need to add target priority for heroic on the snakes or to burn down boss.
    // Will re-test in heroic, decent dps groups should be able to blast him down with no funky strats.
    triggers.push_back(
        new TriggerNode(
            "poison nova",
            {
                CreateNextAction<AvoidPoisonNovaAction>(ACTION_RAID + 5.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "snake wrap",
            {
                CreateNextAction<AttackSnakeWrapAction>(ACTION_RAID + 4.0f)
            }
        )
    );

    // Gal'darah
    triggers.push_back(
        new TriggerNode(
            "whirling slash",
            {
                CreateNextAction<AvoidWhirlingSlashAction>(ACTION_RAID + 5.0f)
            }
        )
    );

    // Eck the Ferocious (Heroic only)
}

void WotlkDungeonGDStrategy::InitMultipliers(std::vector<Multiplier*> &multipliers)
{
    multipliers.push_back(new SladranMultiplier(botAI));
    multipliers.push_back(new GaldarahMultiplier(botAI));
}
