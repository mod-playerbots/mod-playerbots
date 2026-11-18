/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "ToCDefinitions.h"

#include "PlayerbotAI.h"
#include "RaidDefensive.h"
#include "Strategy.h"
#include "ToCActions_Jaraxxus.h"
#include "ToCEncounterGate.h"
#include "ToCHelpers_Jaraxxus.h"
#include "ToCMultipliers_Jaraxxus.h"
#include "ToCTriggers_Jaraxxus.h"

using namespace TrialOfTheCrusaderHelpers;

namespace Family = RaidEncounterRules::Family;
namespace Role = RaidEncounterRules::Role;

namespace
{
bool JaraxxusDpsTargetOwned(PlayerbotAI* botAI) { return GetJaraxxusDpsTarget(botAI) != nullptr; }

bool JaraxxusTankTargetOwned(PlayerbotAI* botAI)
{
    return JaraxxusMainTankHolds(botAI) || JaraxxusAssistTankHolds(botAI, 0) || JaraxxusAssistTankHolds(botAI, 1);
}

bool JaraxxusPortalHeld(PlayerbotAI* botAI) { return GetJaraxxusPortalToClear(botAI) != nullptr; }

void DefineJaraxxus(EncounterBuilder& e)
{
    e.Node<JaraxxusFelFireballInterruptibleTrigger, JaraxxusInterruptFelFireballAction>(ACTION_EMERGENCY + 9);

    // above the flame walk: an instant goes out on the move, and a carrier's walk holds every tick
    e.Node<JaraxxusRaidCoverTrigger, JaraxxusCastRaidCoverAction>(ACTION_EMERGENCY + 8.6f);
    e.Node<JaraxxusPersonalDefensiveTrigger, JaraxxusCastPersonalDefensiveAction>(ACTION_EMERGENCY + 8.4f);
    e.Node<JaraxxusLegionFlameNearbyTrigger, JaraxxusAvoidLegionFlameAction>(ACTION_EMERGENCY + 8);
    e.Node<JaraxxusPinnedCastTrigger, JaraxxusBreakPinnedCastAction>(ACTION_EMERGENCY + 7);
    e.Node<JaraxxusPortalSpawnTrigger, JaraxxusLeavePortalAction>(ACTION_EMERGENCY + 6);
    e.Node<JaraxxusEngagedByMainTankTrigger, JaraxxusMainTankHoldBossAction>(ACTION_RAID + 5);
    e.Node<JaraxxusAddNeedsAssistTankTrigger, JaraxxusAssistTankHoldAddAction>(ACTION_RAID + 4);
    e.Node<JaraxxusSecondAddNeedsAssistTankTrigger, JaraxxusAssistTankHoldSecondAddAction>(ACTION_RAID + 3);
    e.Node<JaraxxusNetherPowerActiveTrigger, JaraxxusRemoveNetherPowerAction>(ACTION_RAID + 2);
    e.Node<JaraxxusDpsTargetTrigger, JaraxxusDpsTargetAction>(ACTION_RAID + 1);

    // health reads full under the absorb, so no generic heal picks the target. stays under critical heals
    e.Node<JaraxxusIncinerateFleshOnRaidTrigger, JaraxxusHealIncinerateTargetAction>(ACTION_MEDIUM_HEAL + 5);

    e.Multiplier<JaraxxusControlTankMovementMultiplier>(Family::AnyMovement);
    e.Multiplier<JaraxxusTauntGuardMultiplier>(Family::Spell);
    e.Multiplier<JaraxxusKissCastHoldMultiplier>(Family::Spell);
    e.Multiplier<JaraxxusAvoidAoeGuardMultiplier>(Family::AnyMovement);

    e.OwnTargeting("jaraxxus dps target guard", Role::Any, JaraxxusDpsTargetOwned, Family::DpsAssist);
    // a tank between holds keeps tank assist, the only target source it has then
    e.OwnTargeting("jaraxxus tank target guard", Role::Any, JaraxxusTankTargetOwned, Family::TankAssist);

    // reach goes by name, so a walk to a battle res isn't blocked
    e.Block("jaraxxus portal hold", Role::Any, JaraxxusPortalHeld,
            Family::SetBehind | Family::RearFlank | Family::CombatFormationMove | Family::Follow |
                Family::MoveRandom | Family::Charge,
            {"reach melee"});
    // their spots ignore the flames, and the dodge would walk the bot straight back out
    e.Block("jaraxxus flame stand", Role::Any, JaraxxusFlameNear,
            Family::SetBehind | Family::RearFlank | Family::CombatFormationMove);
    // the cover node spends them on the Burning Inferno itself, the class nodes would go early or twice
    e.Block("jaraxxus raid cover hold", Role::Any, JaraxxusRaidCoverHeld, 0, RaidCoverClassNodes());
    // neither does anything against fire on everyone or a burning trail
    e.Block("jaraxxus magic button guard", Role::Any, JaraxxusMagicHitOnBot, 0, {"evasion", "deterrence"});

    e.Multiplier<JaraxxusThreatCapMultiplier>(Family::Spell | Family::Melee);
}
}  // namespace

EncounterDefinition const& ToCJaraxxusDefinition()
{
    static EncounterDefinition const definition(static_cast<uint32>(ToCEncounter::Jaraxxus), &ToCGateOpen,
                                                &DefineJaraxxus);
    return definition;
}
