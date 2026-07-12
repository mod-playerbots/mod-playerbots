#include "MechanarMultipliers.h"
#include "MechanarActions.h"
#include "MechanarShared.h"
#include "MovementActions.h"
#include "ReachTargetActions.h"
#include "ChooseTargetActions.h"
#include "FollowActions.h"
#include "AiObjectContext.h"
#include "Playerbots.h"

// Movement lock for the fixated (or actively fire-dodging) bot. While a flame is kiting
// the bot, or it has to escape a trail or Inferno right now, the Sepethrea movers own its
// movement completely; nothing else may issue a step.
float SepethreaKiteFlameMultiplier::GetValue(Action* action)
{
    // The three Sepethrea movers always run (the kite outranks the two dodges, so on a
    // move tick they never fight; on a hold tick a dodge can still step the bot off a
    // patch). Early-accept them before any grid search.
    if (dynamic_cast<SepethreaKiteFlameAction*>(action) ||
        dynamic_cast<SepethreaAvoidTrailAction*>(action) ||
        dynamic_cast<SepethreaAvoidFlameAction*>(action))
        return 1.0f;

    // Only a movement action (or the gap-closer casts Charge/Intercept, which are
    // CastSpellActions, not MovementActions) can fight the kite or dodge. Every other action
    // (the whole spell rotation and heals) is never touched, so skip the grid searches
    // entirely for them. This is the common case (most of the queue).
    bool const isMover =
        dynamic_cast<MovementAction*>(action) || dynamic_cast<CastReachTargetSpellAction*>(action);
    if (!isMover)
        return 1.0f;

    // Lock every other mover while the bot is the flame's fixate target. A blacklist
    // approach (chase/formation/follow/flee) misses rogue SetBehindTarget,
    // MoveOutOfEnemyContact, the stock AvoidAoe, and every DungeonClear assist/regroup/follow
    // mover, all of which would grab control on the kite's hold ticks and drag the bot back
    // toward the boss or fire. That control fight is the source of the melee oscillation
    // (and the ranged bots wandering into the flames). Whitelisting only the three movers
    // above and zeroing the rest removes it.
    // Exception: a fixated tank whose boss is loose must be free to chase and reclaim her
    // (its kite trigger is off for the same condition); a locked-out tank would stand there
    // while she eats the party.
    if (MechanarFlames::GetFixatingFlame(bot) && !MechanarFlames::TankMustGrabBoss(bot))
        return 0.0f;

    // Not fixated, but standing in a trail patch or an Inferno the bot must leave now
    // (and it is not a healer deliberately holding its ground to heal): same lock, so the
    // dodge commits its hop cleanly instead of being interleaved with a combat or DC mover
    // that shoves the bot straight back into the fire on the very same tick.
    if (!MechanarFlames::HealerHoldsFire(bot) &&
        (MechanarFlames::InTrailDanger(bot) ||
         MechanarFlames::GetNearestFlame(bot, MechanarFlames::INFERNO_AVOID_RANGE) != nullptr))
        return 0.0f;

    return 1.0f;
}

// Keep the tank off the un-tankable flame for the duration of the fight.
float SepethreaTankFocusMultiplier::GetValue(Action* action)
{
    // Cheap gates first: GetValue runs for every queued action every tick, so the grid
    // search for Sepethrea must only happen for the handful of target actions.
    if (!PlayerbotAI::IsTank(bot))
        return 1.0f;

    // The stock target-acquisition actions that would (re-)grab the flame. The tank's
    // boss target is supplied instead by SepethreaFocusBossAction; its spell rotation
    // (CastSpellAction and similar, not AttackActions) is left fully intact.
    if (!dynamic_cast<TankAssistAction*>(action) &&
        !dynamic_cast<AggressiveTargetAction*>(action) &&
        !dynamic_cast<AttackAnythingAction*>(action))
        return 1.0f;

    // Only during Sepethrea's own fight (she must be alive and engaged), so the tank still
    // picks up ordinary trash normally everywhere else on the map.
    Unit* boss = MechanarFlames::GetSepethrea(bot);
    if (!boss || !boss->IsInCombat())
        return 1.0f;

    return 0.0f;
}
