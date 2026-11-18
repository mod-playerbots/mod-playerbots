/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "ToCDefinitions.h"

#include "PlayerbotAI.h"
#include "RaidDefensive.h"
#include "ToCEncounterGate.h"
#include "ToCHelpers_Jaraxxus.h"
#include "ToCMultipliers_Jaraxxus.h"

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
