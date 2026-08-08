#include "MechTriggers.h"
#include "AiObject.h"
#include "MechShared.h"
#include "Playerbots.h"

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

    if (!MechanarFlames::IsFlameNearCached(bot))
        return false;

    return !MechanarFlames::HealerHoldsFire(bot);
}

bool SepethreaTrailTrigger::IsActive()
{
    if (!MechanarFlames::GetSepethrea(bot))
        return false;

    if (!MechanarFlames::InTrailDangerCached(bot))
        return false;

    return !MechanarFlames::HealerHoldsFire(bot);
}

bool SepethreaFocusBossTrigger::IsActive()
{
    return MechanarFlames::GetSepethrea(bot) != nullptr;
}
