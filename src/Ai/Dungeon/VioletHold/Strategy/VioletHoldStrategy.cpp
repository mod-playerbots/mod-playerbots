#include "VioletHoldStrategy.h"
#include "VioletHoldActions.h"
#include "VioletHoldMultipliers.h"

void WotlkDungeonVHStrategy::InitTriggers(std::vector<TriggerNode*> &triggers)
{
    // Erekem
    // This boss has many purgable buffs, purging/dispels could be merged into generic strats though
    triggers.push_back(
        new TriggerNode(
            "erekem target",
            {
                CreateNextAction<AttackErekemAction>(ACTION_RAID + 1.0f)
            }
        )
    );

    // Moragg
    // TODO: This guy has Optic Link which may require moving, add if needed

    // Ichoron
    triggers.push_back(
        new TriggerNode(
            "ichoron target",
            {
                CreateNextAction<AttackIchorGlobuleAction>(ACTION_RAID + 1.0f)
            }
        )
    );

    // Xevozz
    // TODO: Revisit in heroics, waypoints back and forth on stairs. Need to test with double beacon spawn

    // Lavanthor
    // Tank & spank

    // Zuramat the Obliterator
    triggers.push_back(
        new TriggerNode(
            "shroud of darkness",
            {
                CreateNextAction<StopAttackAction>(ACTION_HIGH + 5.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "void shift",
            {
                CreateNextAction<AttackVoidSentryAction>(ACTION_RAID + 1.0f)
            }
        )
    );

    // Cyanigosa
    triggers.push_back(
        new TriggerNode(
            "cyanigosa positioning",
            {
                CreateNextAction<RearFlankAction>(ACTION_MOVE + 5.0f)
            }
        )
    );
}

void WotlkDungeonVHStrategy::InitMultipliers(std::vector<Multiplier*> &multipliers)
{
    multipliers.push_back(new ErekemMultiplier(botAI));
    multipliers.push_back(new IchoronMultiplier(botAI));
    multipliers.push_back(new ZuramatMultiplier(botAI));
}
