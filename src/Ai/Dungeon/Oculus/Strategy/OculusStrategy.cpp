#include "OculusStrategy.h"
#include "OculusActions.h"
#include "OculusMultipliers.h"

void WotlkDungeonOccStrategy::InitTriggers(std::vector<TriggerNode*> &triggers)
{
    // Drakos the Interrogator
    // TODO: May need work, TBA.
    triggers.push_back(
        new TriggerNode(
            "unstable sphere",
            {
                CreateNextAction<AvoidUnstableSphereAction>(ACTION_MOVE + 5.0f)
            }
        )
    );

    // DRAKES
    triggers.push_back(
        new TriggerNode(
            "drake mount",
            {
                CreateNextAction<MountDrakeAction>(ACTION_RAID + 5.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "drake dismount",
            {
                CreateNextAction<DismountDrakeAction>(ACTION_RAID + 5.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "group flying",
            {
                CreateNextAction<OccFlyDrakeAction>(ACTION_NORMAL + 1.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "drake combat",
            {
                CreateNextAction<OccDrakeAttackAction>(ACTION_NORMAL + 5.0f)
            }
        )
    );

    // Varos Cloudstrider
    // Seems to be no way to identify the marked cores, may need to hook boss AI..
    // triggers.push_back(
    // new TriggerNode(
    // "varos cloudstrider",
    //
    //   { CreateNextAction<AvoidEnergizeCoresAction>(ACTION_RAID + 5)
    // }
    // )
    // );

    // Mage-Lord Urom
    triggers.push_back(
        new TriggerNode(
            "arcane explosion",
            {
                CreateNextAction<AvoidArcaneExplosionAction>(ACTION_MOVE + 5.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "time bomb",
            {
                CreateNextAction<TimeBombSpreadAction>(ACTION_MOVE + 4.0f)
            }
        )
    );

    // Ley-Guardian Eregos
}

void WotlkDungeonOccStrategy::InitMultipliers(std::vector<Multiplier*> &multipliers)
{
    multipliers.push_back(new MountingDrakeMultiplier(botAI));
    multipliers.push_back(new OccFlyingMultiplier(botAI));
    multipliers.push_back(new UromMultiplier(botAI));
    multipliers.push_back(new EregosMultiplier(botAI));
}
