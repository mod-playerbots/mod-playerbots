#include "Playerbots.h"
#include "MechanarTriggers.h"
#include "MechanarShared.h"
#include "AiObject.h"
#include "AiObjectContext.h"

// Nethermancer Sepethrea: Raging Flames kite.

bool SepethreaKiteFlameTrigger::IsActive()
{
    if (!MechanarFlames::GetFixatingFlame(bot))
        return false;

    // A fixated tank only kites while it also holds the boss (she then follows it around
    // the hall on threat). If she is loose on the party, reclaiming her outranks the kite:
    // the trigger yields, the movement lock lifts (see SepethreaKiteFlameMultiplier) and
    // SepethreaFocusBossAction redirects the tank onto her, while the flame trailing behind
    // merely melees the tank as it does its job.
    return !MechanarFlames::TankMustGrabBoss(bot);
}

bool SepethreaAvoidFlameTrigger::IsActive()
{
    // A bot fixated by either elemental is a kiter: the higher-priority "sepethrea kite
    // flame" action owns its movement and already steers it clear of the other elemental
    // and the trails, so this bystander net must not also fire for it.
    if (MechanarFlames::GetFixatingFlame(bot))
        return false;

    // Fire only when actually inside an Inferno's reach (either elemental; with two of
    // them, the nearest is what would clip the bot first).
    if (!MechanarFlames::GetNearestFlame(bot, MechanarFlames::INFERNO_AVOID_RANGE))
        return false;

    // A healer stands in the fire to keep its heal landing rather than dropping the
    // cast to dodge (the tank/kiter otherwise dies from the missed heal).
    if (MechanarFlames::HealerHoldsFire(bot))
        return false;

    return true;
}

bool SepethreaTrailTrigger::IsActive()
{
    // The Raging Flames elemental drops a ribbon of persistent 5yd fire patches (spell
    // 35278, a PERSISTENT_AREA_AURA dynobj, one per second, each lasting 6s) as it moves;
    // this is the "trail" that kills stacked DPS. It is detected by a direct grid search of
    // the fire-patch dynobjs (InTrailDanger), not via the stock "area debuff" value: that
    // hinged on the 35278 aura being applied to and negative-classified on the bot, and
    // drove the single-patch stock "avoid aoe" flee, which just hops a bot from one patch of
    // the overlapping trail into the next.
    if (!MechanarFlames::InTrailDanger(bot))
        return false;

    // The healer holds its ground to keep healing (same rule as the elemental dodge).
    return !MechanarFlames::HealerHoldsFire(bot);
}

bool SepethreaFocusBossTrigger::IsActive()
{
    if (!bot->IsInCombat())
        return false;
    Unit* boss = MechanarFlames::GetSepethrea(bot);
    return boss && boss->IsAlive() && boss->IsInCombat();
}
