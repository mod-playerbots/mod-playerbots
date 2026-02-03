#include "HallsOfStoneStrategy.h"
#include "CreateNextAction.h"
#include "HallsOfStoneActions.h"
#include "HallsOfStoneMultipliers.h"

void WotlkDungeonHoSStrategy::InitTriggers(std::vector<TriggerNode*> &triggers)
{
    // Maiden of Grief
    // TODO: Jump into damage during shock of sorrow?

    // Krystallus
    // TODO: I think bots need to dismiss pets on this, or they nuke players they are standing close to
    triggers.push_back(
        new TriggerNode(
            "ground slam",
            {
                CreateNextAction<ShatterSpreadAction>(ACTION_RAID + 5.0f)
            }
        )
    );

    // Tribunal of Ages
    // Seems fine, maybe add focus targeting strat if needed on heroic.
    // Main issue is dps will immediately rambo in and sometimes die before tank gets aggro,
    // this is mostly an issue with the bot AI as they do it on every fight

    // Sjonnir The Ironshaper
    // Possibly tank in place in the middle of the room, assign a dps to adds?
    triggers.push_back(
        new TriggerNode(
            "lightning ring",
            {
                CreateNextAction<AvoidLightningRingAction>(ACTION_RAID + 5.0f)
            }
        )
    );
}

void WotlkDungeonHoSStrategy::InitMultipliers(std::vector<Multiplier*> &multipliers)
{
    multipliers.push_back(new KrystallusMultiplier(botAI));
    multipliers.push_back(new SjonnirMultiplier(botAI));
}
