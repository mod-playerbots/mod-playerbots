#include "Playerbots.h"
#include "MechTriggers.h"
#include "MechShared.h"
#include "AiObject.h"
#include "AiObjectContext.h"

bool SepethreaKiteFlameTrigger::IsActive()
{
    if (!MechanarFlames::GetSepethrea(bot))
        return false;

    if (!MechanarFlames::GetFixatingFlame(bot))
        return false;

    return !MechanarFlames::TankMustGrabBoss(bot);
}

bool SepethreaAvoidFlameTrigger::IsActive()
{
    if (!MechanarFlames::GetSepethrea(bot))
        return false;

    if (MechanarFlames::GetFixatingFlame(bot))
        return false;

    if (!MechanarFlames::GetNearestFlame(bot, MechanarFlames::INFERNO_AVOID_RANGE))
        return false;

    return !MechanarFlames::HealerHoldsFire(bot);
}

bool SepethreaTrailTrigger::IsActive()
{
    if (!MechanarFlames::GetSepethrea(bot))
        return false;

    if (!MechanarFlames::InTrailDanger(bot))
        return false;

    return !MechanarFlames::HealerHoldsFire(bot);
}

bool SepethreaFocusBossTrigger::IsActive()
{
    return MechanarFlames::GetSepethrea(bot) != nullptr;
}
