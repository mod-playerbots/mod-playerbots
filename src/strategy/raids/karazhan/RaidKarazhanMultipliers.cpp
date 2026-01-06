#include "RaidKarazhanMultipliers.h"
#include "RaidKarazhanActions.h"
#include "RaidKarazhanHelpers.h"
#include "AttackAction.h"
#include "ChooseTargetActions.h"
#include "DruidActions.h"
#include "FollowActions.h"
#include "GenericActions.h"
#include "HunterActions.h"
#include "MageActions.h"
#include "Playerbots.h"
#include "PriestActions.h"
#include "ReachTargetActions.h"
#include "RogueActions.h"
#include "ShamanActions.h"

using namespace KarazhanHelpers;

// Keep tanks from jumping back and forth between Attumen and Midnight
float AttumenTheHuntsmanDisableTankAssistMultiplier::GetValue(Action* action)
{
    Unit* midnight = AI_VALUE2(Unit*, "find target", "midnight");
    if (!midnight)
        return 1.0f;

    Unit* attumen = AI_VALUE2(Unit*, "find target", "attumen the huntsman");
    if (!attumen)
        return 1.0f;

    if (bot->GetVictim() != nullptr && dynamic_cast<TankAssistAction*>(action))
        return 0.0f;

    return 1.0f;
}

// Try to get rid of jittering when bots are stacked behind Attumen
float AttumenTheHuntsmanStayStackedMultiplier::GetValue(Action* action)
{
    Unit* attumenMounted = GetFirstAliveUnitByEntry(botAI, NPC_ATTUMEN_THE_HUNTSMAN_MOUNTED);
    if (!attumenMounted)
        return 1.0f;

    if (!botAI->IsMainTank(bot) && attumenMounted->GetVictim() != bot)
    {
        if (dynamic_cast<CombatFormationMoveAction*>(action) ||
            dynamic_cast<FleeAction*>(action) ||
            dynamic_cast<CastBlinkBackAction*>(action) ||
            dynamic_cast<CastDisengageAction*>(action) ||
            dynamic_cast<CastReachTargetSpellAction*>(action))
            return 0.0f;
    }

    return 1.0f;
}

// Give the main tank 8 seconds to grab aggro when Attumen mounts Midnight
// In reality it's shorter because it takes Attumen a few seconds to aggro after mounting
float AttumenTheHuntsmanWaitForDpsMultiplier::GetValue(Action* action)
{
    Unit* attumenMounted = GetFirstAliveUnitByEntry(botAI, NPC_ATTUMEN_THE_HUNTSMAN_MOUNTED);
    if (!attumenMounted)
        return 1.0f;

    const uint32 instanceId = attumenMounted->GetMap()->GetInstanceId();
    const time_t now = std::time(nullptr);
    const uint8 dpsWaitSeconds = 8;

    auto it = attumenDpsWaitTimer.find(instanceId);
    if (it == attumenDpsWaitTimer.end() || (now - it->second) < dpsWaitSeconds)
    {
        if (!botAI->IsMainTank(bot))
        {
            if (dynamic_cast<AttackAction*>(action) || (dynamic_cast<CastSpellAction*>(action) &&
                !dynamic_cast<CastHealingSpellAction*>(action)))
                return 0.0f;
        }
    }

    return 1.0f;
}

// The assist tank should stay on the boss to be 2nd on aggro and tank Hateful Bolts
float TheCuratorDisableTankAssistMultiplier::GetValue(Action* action)
{
    Unit* curator = AI_VALUE2(Unit*, "find target", "the curator");
    if (!curator)
        return 1.0f;

    if (bot->GetVictim() != nullptr && dynamic_cast<TankAssistAction*>(action))
        return 0.0f;

    return 1.0f;
}

// Save Bloodlust/Heroism for Evocation (100% increased damage)
float TheCuratorDelayBloodlustAndHeroismMultiplier::GetValue(Action* action)
{
    Unit* curator = AI_VALUE2(Unit*, "find target", "the curator");
    if (!curator)
        return 1.0f;

    if (!curator->HasAura(SPELL_CURATOR_EVOCATION))
    {
        if (dynamic_cast<CastBloodlustAction*>(action) ||
            dynamic_cast<CastHeroismAction*>(action))
            return 0.0f;
    }

    return 1.0f;
}

// Don't charge back in when running from Arcane Explosion
float ShadeOfAranArcaneExplosionDisableChargeMultiplier::GetValue(Action* action)
{
    Unit* aran = AI_VALUE2(Unit*, "find target", "shade of aran");
    if (!aran)
        return 1.0f;

    if (aran->HasUnitState(UNIT_STATE_CASTING) &&
        aran->FindCurrentSpellBySpellId(SPELL_ARCANE_EXPLOSION))
    {
        if (dynamic_cast<CastReachTargetSpellAction*>(action))
            return 0.0f;

        if (bot->GetDistance2d(aran) >= 20.0f)
        {
            if (dynamic_cast<CombatFormationMoveAction*>(action) ||
                dynamic_cast<FleeAction*>(action) ||
                dynamic_cast<FollowAction*>(action) ||
                dynamic_cast<ReachTargetAction*>(action) ||
                dynamic_cast<AvoidAoeAction*>(action))
                return 0.0f;
        }
    }

    return 1.0f;
}

// I will not move when Flame Wreath is cast or the raid blows up
float ShadeOfAranFlameWreathDisableMovementMultiplier::GetValue(Action* action)
{
    Unit* aran = AI_VALUE2(Unit*, "find target", "shade of aran");
    if (!aran)
        return 1.0f;

    if (IsFlameWreathActive(botAI, bot))
    {
        if (dynamic_cast<CombatFormationMoveAction*>(action) ||
            dynamic_cast<FleeAction*>(action) ||
            dynamic_cast<FollowAction*>(action) ||
            dynamic_cast<ReachTargetAction*>(action) ||
            dynamic_cast<AvoidAoeAction*>(action) ||
            dynamic_cast<CastKillingSpreeAction*>(action) ||
            dynamic_cast<CastBlinkBackAction*>(action) ||
            dynamic_cast<CastDisengageAction*>(action) ||
            dynamic_cast<CastReachTargetSpellAction*>(action))
            return 0.0f;
    }

    return 1.0f;
}

// Try to rid of the jittering when blocking beams
float NetherspiteKeepBlockingBeamMultiplier::GetValue(Action* action)
{
    Unit* netherspite = AI_VALUE2(Unit*, "find target", "netherspite");
    if (!netherspite || netherspite->HasAura(SPELL_NETHERSPITE_BANISHED))
        return 1.0f;

    auto [redBlocker, greenBlocker, blueBlocker] = GetCurrentBeamBlockers(botAI, bot);

    if (bot == redBlocker)
    {
        if (dynamic_cast<CombatFormationMoveAction*>(action))
            return 0.0f;
    }

    if (bot == blueBlocker)
    {
        if (dynamic_cast<CombatFormationMoveAction*>(action) ||
            dynamic_cast<ReachTargetAction*>(action))
            return 0.0f;
    }

    if (bot == greenBlocker)
    {
        if (dynamic_cast<CombatFormationMoveAction*>(action) ||
            dynamic_cast<ReachTargetAction*>(action) ||
            dynamic_cast<FleeAction*>(action) ||
            dynamic_cast<CastKillingSpreeAction*>(action) ||
            dynamic_cast<CastReachTargetSpellAction*>(action))
            return 0.0f;
    }

    return 1.0f;
}

// Give tanks 5 seconds to get aggro during phase transitions
float NetherspiteWaitForDpsMultiplier::GetValue(Action* action)
{
    Unit* netherspite = AI_VALUE2(Unit*, "find target", "netherspite");
    if (!netherspite || netherspite->HasAura(SPELL_NETHERSPITE_BANISHED))
        return 1.0f;

    const uint32 instanceId = netherspite->GetMap()->GetInstanceId();
    const time_t now = std::time(nullptr);
    const uint8 dpsWaitSeconds = 5;

    auto it = netherspiteDpsWaitTimer.find(instanceId);
    if (it == netherspiteDpsWaitTimer.end() || (now - it->second) < dpsWaitSeconds)
    {
        if (!botAI->IsTank(bot))
        {
            if (dynamic_cast<AttackAction*>(action) || (dynamic_cast<CastSpellAction*>(action) &&
                !dynamic_cast<CastHealingSpellAction*>(action)))
            return 0.0f;
        }
    }

     return 1.0f;
}

// Disable standard "avoid aoe" strategy, which may interfere with scripted avoidance
float PrinceMalchezaarDisableAvoidAoeMultiplier::GetValue(Action* action)
{
    Unit* malchezaar = AI_VALUE2(Unit*, "find target", "prince malchezaar");
    if (!malchezaar)
        return 1.0f;

    if (dynamic_cast<AvoidAoeAction*>(action))
        return 0.0f;

    return 1.0f;
}

// Don't run back into Shadow Nova when Enfeebled
float PrinceMalchezaarEnfeebleKeepDistanceMultiplier::GetValue(Action* action)
{
    Unit* malchezaar = AI_VALUE2(Unit*, "find target", "prince malchezaar");
    if (!malchezaar)
        return 1.0f;

    if (bot->HasAura(SPELL_ENFEEBLE))
    {
        if (dynamic_cast<MovementAction*>(action) &&
            !dynamic_cast<PrinceMalchezaarEnfeebledAvoidHazardAction*>(action))
            return 0.0f;
    }

    return 1.0f;
}

// Wait until Phase 3 to use Bloodlust/Heroism
float PrinceMalchezaarDelayBloodlustAndHeroismMultiplier::GetValue(Action* action)
{
    Unit* malchezaar = AI_VALUE2(Unit*, "find target", "prince malchezaar");
    if (!malchezaar)
        return 1.0f;

    if (malchezaar->GetHealthPct() > 30.0f)
    {
        if (dynamic_cast<CastBloodlustAction*>(action) ||
            dynamic_cast<CastHeroismAction*>(action))
            return 0.0f;
    }

    return 1.0f;
}

// Pets tend to run out of bounds and cause skeletons to spawn off the map
// Pets also tend to pull adds from inside of the tower through the floor
// This multiplier DOES NOT impact Hunter and Warlock pets
// Hunter and Warlock pets are addressed in ControlPetAggressionAction
float NightbaneDisablePetsMultiplier::GetValue(Action* action)
{
    Unit* nightbane = AI_VALUE2(Unit*, "find target", "nightbane");
    if (!nightbane)
        return 1.0f;

    if (dynamic_cast<CastForceOfNatureAction*>(action) ||
        dynamic_cast<CastFeralSpiritAction*>(action) ||
        dynamic_cast<CastFireElementalTotemAction*>(action) ||
        dynamic_cast<CastFireElementalTotemMeleeAction*>(action) ||
        dynamic_cast<CastSummonWaterElementalAction*>(action) ||
        dynamic_cast<CastShadowfiendAction*>(action))
        return 0.0f;

    if (nightbane->GetPositionZ() > NIGHTBANE_FLIGHT_Z)
    {
        if (dynamic_cast<PetAttackAction*>(action))
            return 0.0f;
    }

    return 1.0f;
}

// Give the main tank 8 seconds to get aggro during phase transitions
float NightbaneWaitForDpsMultiplier::GetValue(Action* action)
{
    Unit* nightbane = AI_VALUE2(Unit*, "find target", "nightbane");
    if (!nightbane || nightbane->GetPositionZ() > NIGHTBANE_FLIGHT_Z)
        return 1.0f;

    const uint32 instanceId = nightbane->GetMap()->GetInstanceId();
    const time_t now = std::time(nullptr);
    const uint8 dpsWaitSeconds = 8;

    auto it = nightbaneDpsWaitTimer.find(instanceId);
    if (it == nightbaneDpsWaitTimer.end() || (now - it->second) < dpsWaitSeconds)
    {
        if (!botAI->IsMainTank(bot))
        {
            if (dynamic_cast<AttackAction*>(action) || (dynamic_cast<CastSpellAction*>(action) &&
                !dynamic_cast<CastHealingSpellAction*>(action)))
                return 0.0f;
        }
    }

    return 1.0f;
}

// The "avoid aoe" strategy must be disabled for the main tank
// Otherwise, the main tank will spin Nightbane to avoid Charred Earth and wipe the raid
// It is also disabled for all bots during the flight phase
float NightbaneDisableAvoidAoeMultiplier::GetValue(Action* action)
{
    Unit* nightbane = AI_VALUE2(Unit*, "find target", "nightbane");
    if (!nightbane)
        return 1.0f;

    if (nightbane->GetPositionZ() > NIGHTBANE_FLIGHT_Z || botAI->IsMainTank(bot))
    {
        if (dynamic_cast<AvoidAoeAction*>(action))
            return 0.0f;
    }

    return 1.0f;
}

// Disable some movement actions that conflict with the strategies
float NightbaneDisableMovementMultiplier::GetValue(Action* action)
{
    Unit* nightbane = AI_VALUE2(Unit*, "find target", "nightbane");
    if (!nightbane)
        return 1.0f;

    if (dynamic_cast<CastBlinkBackAction*>(action) ||
        dynamic_cast<CastDisengageAction*>(action) ||
        dynamic_cast<FleeAction*>(action))
        return 0.0f;

    // Disable CombatFormationMoveAction for all bots except:
    // (1) main tank and (2) only during the ground phase, other melee
    if (botAI->IsRanged(bot) ||
        (botAI->IsMelee(bot) && !botAI->IsMainTank(bot) &&
         nightbane->GetPositionZ() > NIGHTBANE_FLIGHT_Z))
    {
        if (dynamic_cast<CombatFormationMoveAction*>(action))
            return 0.0f;
    }

    return 1.0f;
}
