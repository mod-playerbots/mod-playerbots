#include "CullingOfStratholmeMultipliers.h"
#include "GenericSpellActions.h"
#include "MovementActions.h"
#include "Action.h"
#include "Playerbots.h"

float EpochMultiplier::GetValue(Action& action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "chrono-lord epoch");

    if (boss == nullptr)
    {
        return 1.0f;
    }

    if (bot->getClass() == CLASS_HUNTER)
    {
        return 1.0f;
    }

    if (dynamic_cast<FleeAction*>(&action))
    {
        return 0.0f;
    }

    return 1.0f;
}
