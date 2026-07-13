#include "MechMultipliers.h"
#include "MechActions.h"
#include "MechShared.h"
#include "MovementActions.h"
#include "ReachTargetActions.h"
#include "ChooseTargetActions.h"
#include "GenericSpellActions.h"
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

float SepethreaFocusBossMultiplier::GetValue(Action* action)
{
    if (!dynamic_cast<DpsAssistAction*>(action) &&
        !dynamic_cast<TankAssistAction*>(action) &&
        !dynamic_cast<CastDebuffSpellOnAttackerAction*>(action))
        return 1.0f;

    if (!MechanarFlames::GetSepethrea(bot))
        return 1.0f;

    return 0.0f;
}
