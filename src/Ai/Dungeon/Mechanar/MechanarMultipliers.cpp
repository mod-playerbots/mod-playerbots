#include "MechanarMultipliers.h"
#include "MechanarActions.h"
#include "MechanarShared.h"
#include "MovementActions.h"
#include "ReachTargetActions.h"
#include "ChooseTargetActions.h"
#include "FollowActions.h"
#include "AiObjectContext.h"
#include "Playerbots.h"

float SepethreaKiteFlameMultiplier::GetValue(Action* action)
{
    if (dynamic_cast<SepethreaKiteFlameAction*>(action) ||
        dynamic_cast<SepethreaAvoidTrailAction*>(action) ||
        dynamic_cast<SepethreaAvoidFlameAction*>(action))
        return 1.0f;

    bool const isMover =
        dynamic_cast<MovementAction*>(action) || dynamic_cast<CastReachTargetSpellAction*>(action);
    if (!isMover)
        return 1.0f;

    if (MechanarFlames::GetFixatingFlame(bot) && !MechanarFlames::TankMustGrabBoss(bot))
        return 0.0f;

    if (!MechanarFlames::HealerHoldsFire(bot) &&
        (MechanarFlames::InTrailDanger(bot) ||
         MechanarFlames::GetNearestFlame(bot, MechanarFlames::INFERNO_AVOID_RANGE) != nullptr))
        return 0.0f;

    return 1.0f;
}

float SepethreaTankFocusMultiplier::GetValue(Action* action)
{
    if (!PlayerbotAI::IsTank(bot))
        return 1.0f;

    if (!dynamic_cast<TankAssistAction*>(action) &&
        !dynamic_cast<AggressiveTargetAction*>(action) &&
        !dynamic_cast<AttackAnythingAction*>(action))
        return 1.0f;

    Unit* boss = MechanarFlames::GetSepethrea(bot);
    if (!boss || !boss->IsInCombat())
        return 1.0f;

    return 0.0f;
}
