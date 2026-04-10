#include "AuchenaiCryptsMultipliers.h"
#include "AuchenaiCryptsActions.h"
#include "AuchenaiCryptsTriggers.h"
#include "MovementActions.h"
#include "ReachTargetActions.h"
#include "FollowActions.h"
#include "AiObjectContext.h"
#include "Playerbots.h"

// Shirrak the Dead Watcher

// Flee from Focus Fire and dont run back in
float ShirrakFleeFocusFireMultiplier::GetValue(Action* action)
{
    if (!AI_VALUE2(Unit*, "find target", "shirrak the dead watcher"))
        return 1.0f;

        std::list<Creature*> creatureList;
            bot->GetCreatureListWithEntryInGrid(creatureList, static_cast<uint32>(AuchenaiCryptsIDs::NPC_FOCUS_FIRE), 20.0f);

    for (Creature* flare : creatureList)
    {
        if (flare && flare->IsAlive())
        {
            if (dynamic_cast<CastReachTargetSpellAction*>(action))
                return 0.0f;

            float currentDistance = bot->GetDistance2d(flare);
            constexpr float safeDistance = 12.0f;
            constexpr float buffer = 5.0f;

            if (currentDistance < safeDistance + buffer && (
                dynamic_cast<CombatFormationMoveAction*>(action) ||
                dynamic_cast<ShirrakRangedKeepDistanceAction*>(action) ||
                dynamic_cast<FleeAction*>(action) ||
                dynamic_cast<FollowAction*>(action) ||
                dynamic_cast<ReachTargetAction*>(action) ||
                dynamic_cast<AvoidAoeAction*>(action)))
            {
                return 0.0f;
            }
        }
    }
    return 1.0f;
}
