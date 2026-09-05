#include "MechMultipliers.h"
#include "ChooseTargetActions.h"
#include "FollowActions.h"
#include "GenericSpellActions.h"
#include "MechActions.h"
#include "MechShared.h"
#include "MovementActions.h"
#include "Playerbots.h"
#include "ReachTargetActions.h"

float SepethreaKiteFlameMultiplier::GetValue(Action* action)
{
    if (dynamic_cast<SepethreaKiteFlameAction*>(action) ||
        dynamic_cast<SepethreaAvoidTrailAction*>(action) ||
        dynamic_cast<SepethreaAvoidFlameAction*>(action) ||
        dynamic_cast<SepethreaFocusBossAction*>(action))
        return 1.0f;

    bool const isMover =
        dynamic_cast<MovementAction*>(action) || dynamic_cast<CastReachTargetSpellAction*>(action);
    if (!isMover)
        return 1.0f;

    if (MechanarFlames::GetFixatingFlame(bot) && !MechanarFlames::TankMustGrabBoss(bot))
        return 0.0f;

    if (!MechanarFlames::HealerHoldsFire(bot) &&
        (MechanarFlames::InTrailDangerCached(bot) || MechanarFlames::IsFlameNearCached(bot)))
        return 0.0f;

    return 1.0f;
}

float SepethreaFocusBossMultiplier::GetValue(Action* action)
{
    bool const suppressed =
        dynamic_cast<DpsAssistAction*>(action) ||
        dynamic_cast<TankAssistAction*>(action) ||
        dynamic_cast<CastDebuffSpellOnAttackerAction*>(action) ||
        (action->getThreatType() == Action::ActionThreatType::Aoe && botAI->IsDps(bot) &&
         !dynamic_cast<CastHealingSpellAction*>(action));
    if (!suppressed)
        return 1.0f;

    if (!MechanarFlames::GetSepethrea(bot))
        return 1.0f;

    return 0.0f;
}
