#include "Playerbots.h"
#include "MechanarTriggers.h"
#include "MechanarShared.h"
#include "AiObject.h"
#include "AiObjectContext.h"

bool SepethreaKiteFlameTrigger::IsActive()
{
    if (!MechanarFlames::GetFixatingFlame(bot))
        return false;

    return !MechanarFlames::TankMustGrabBoss(bot);
}

bool SepethreaAvoidFlameTrigger::IsActive()
{
    if (MechanarFlames::GetFixatingFlame(bot))
        return false;

    if (!MechanarFlames::GetNearestFlame(bot, MechanarFlames::INFERNO_AVOID_RANGE))
        return false;

    if (MechanarFlames::HealerHoldsFire(bot))
        return false;

    return true;
}

bool SepethreaTrailTrigger::IsActive()
{
    if (!MechanarFlames::InTrailDanger(bot))
        return false;

    return !MechanarFlames::HealerHoldsFire(bot);
}

bool SepethreaFocusBossTrigger::IsActive()
{
    if (!bot->IsInCombat())
        return false;
    Unit* boss = MechanarFlames::GetSepethrea(bot);
    return boss && boss->IsAlive() && boss->IsInCombat();
}
