/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "ToCDefinitions.h"

#include "PlayerbotAI.h"
#include "RaidCombatHelpers.h"
#include "RaidDefensive.h"
#include "Strategy.h"
#include "ToCActions_TwinValkyr.h"
#include "ToCEncounterGate.h"
#include "ToCHelpers_TwinValkyr.h"
#include "ToCMultipliers_TwinValkyr.h"
#include "ToCTriggers_TwinValkyr.h"

using namespace TrialOfTheCrusaderHelpers;

namespace Family = RaidEncounterRules::Family;
namespace Role = RaidEncounterRules::Role;

namespace
{
// The dps target trigger's test, so the rule and the node cover the same window
bool TwinValkyrDpsTargetOwned(PlayerbotAI* botAI)
{
    return IsRaidDpsBot(botAI->GetBot()) && ToCEncounterIsLive(botAI, ToCEncounter::TwinValkyr) &&
           GetTwinDpsTarget(botAI);
}

// The two hold triggers' twin tests, so a tank keeps tank assist only while no hold covers it
bool TwinValkyrTankTargetOwned(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    Unit* fjola = GetFjola(botAI);
    if (IsTwinAttackable(fjola) && IsTwinTank(bot, fjola))
        return true;

    Unit* eydis = GetEydis(botAI);
    return IsTwinAttackable(eydis) && IsTwinTank(bot, eydis);
}

void DefineTwinValkyr(EncounterBuilder& e)
{
    e.Node<TwinValkyrPactInterruptDutyTrigger, TwinValkyrInterruptPactAction>(ACTION_EMERGENCY + 8);
    e.Node<TwinValkyrVortexRequiresEssenceTrigger, TwinValkyrSwapEssenceForVortexAction>(ACTION_EMERGENCY + 7);

    // a Touch ticks only on the touched bot, and an essence of the Touch's colour absorbs it all
    e.Node<TwinValkyrTouchedRequiresEssenceTrigger, TwinValkyrSwapEssenceForTouchAction>(ACTION_EMERGENCY + 6);
    e.Node<TwinValkyrVortexRaidCoverTrigger, TwinValkyrCastVortexRaidCoverAction>(ACTION_EMERGENCY + 5);
    e.Node<TwinValkyrVortexDefensiveTrigger, TwinValkyrCastVortexDefensiveAction>(ACTION_EMERGENCY + 4);
    e.Node<TwinValkyrOrbIncomingTrigger, TwinValkyrDodgeOrbAction>(ACTION_EMERGENCY + 3);
    e.Node<TwinValkyrNextPortalDueTrigger, TwinValkyrWalkToNextPortalAction>(ACTION_EMERGENCY + 2);
    e.Node<TwinValkyrNeedsBaseEssenceTrigger, TwinValkyrTakeBaseEssenceAction>(ACTION_RAID + 5);
    e.Node<TwinValkyrEngagedByMainTankTrigger, TwinValkyrMainTankHoldLightTwinAction>(ACTION_RAID + 4);
    e.Node<TwinValkyrDarkbaneNeedsAssistTankTrigger, TwinValkyrAssistTankHoldDarkTwinAction>(ACTION_RAID + 3);
    e.Node<TwinValkyrRedirectThreatTrigger, TwinValkyrRedirectThreatAction>(ACTION_RAID + 2);
    e.Node<TwinValkyrDoorStandTrigger, TwinValkyrMoveToDoorStandAction>(ACTION_RAID + 1);
    e.Node<TwinValkyrDpsTargetTrigger, TwinValkyrFocusTwinAction>(ACTION_RAID);

    e.Multiplier<TwinValkyrControlTankMovementMultiplier>(Family::AnyMovement);
    e.Multiplier<TwinValkyrTauntGuardMultiplier>(Family::Spell);
    e.Multiplier<TwinValkyrInterruptHoldMultiplier>(Family::AnyAction);
    e.Multiplier<TwinValkyrRedirectGuardMultiplier>(Family::AnyMovement | Family::Spell);

    e.OwnTargeting("twin valkyr dps target guard", Role::Any, TwinValkyrDpsTargetOwned, Family::DpsAssist);
    e.OwnTargeting("twin valkyr tank target guard", Role::Any, TwinValkyrTankTargetOwned, Family::TankAssist);

    // the tunnel is past every orb, so the raid only leaves it for a portal, and those walks are the
    // encounter's own movers
    e.Block("twin valkyr door hold", Role::Any, TwinValkyrDoorHold,
            Family::Reach | Family::ReachHeal | Family::SetBehind | Family::RearFlank | Family::CombatFormationMove |
                Family::Follow | Family::Flee | Family::RunAway | Family::MoveRandom | Family::Charge | Family::Blink |
                Family::Disengage);

    e.Block("twin valkyr raid cover hold", Role::Any, TwinValkyrRaidCoverHeld, 0, RaidCoverClassNodes());

    // the burst row's flag covers every role, and a healer holds only this one for lust
    e.Block("twin valkyr power infusion hold", Role::Heal, TwinValkyrBurstHeld, 0,
            {"power infusion", "power infusion on party"});
}
}  // namespace

EncounterDefinition const& ToCTwinValkyrDefinition()
{
    static EncounterDefinition const definition(static_cast<uint32>(ToCEncounter::TwinValkyr), &ToCGateOpen,
                                                &DefineTwinValkyr);
    return definition;
}
