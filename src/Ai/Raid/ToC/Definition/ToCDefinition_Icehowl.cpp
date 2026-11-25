/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "ToCDefinitions.h"

#include "Strategy.h"
#include "ToCActions_Icehowl.h"
#include "ToCActions_NorthrendBeasts.h"
#include "ToCEncounterGate.h"
#include "ToCMultipliers_Icehowl.h"
#include "ToCTriggers_Icehowl.h"
#include "ToCTriggers_NorthrendBeasts.h"

namespace Family = RaidEncounterRules::Family;

namespace
{
void DefineIcehowl(EncounterBuilder& e)
{
    e.Node<IcehowlTankDutyTrigger, IcehowlTankHoldBossAction>(ACTION_RAID + 1);
    e.Node<IcehowlChargeIncomingTrigger, IcehowlClearChargePathAction>(ACTION_EMERGENCY + 8);
    e.Node<IcehowlFrothingRageTrigger, IcehowlTankDefensiveAction>(ACTION_RAID + 6);

    // Ahead of the walk to him, which then runs inside the boost
    e.Node<IcehowlDazeNitroTrigger, IcehowlDazeNitroAction>(ACTION_RAID + 3);

    // Ahead of the breath stand: its walk holds the tick until it lands, and this switch doesn't stop it
    e.Node(
        "icehowl dps target",
        [](PlayerbotAI* botAI) -> Trigger*
        { return new NorthrendBeastsDpsTargetTrigger(botAI, "icehowl dps target", NorthrendBeastsStem::Icehowl); },
        "icehowl dps target",
        [](PlayerbotAI* botAI) -> Action* { return new NorthrendBeastsDpsTargetAction(botAI, "icehowl dps target"); },
        ACTION_RAID + 2);
    e.Node<IcehowlBreathSpreadTrigger, IcehowlMoveToBreathStandAction>(ACTION_RAID + 2);

    e.Multiplier<IcehowlChargeGuardMultiplier>(Family::AnyMovement | Family::Spell);
    e.Multiplier<IcehowlBreathSpreadGuardMultiplier>(Family::AnyMovement);
}
}  // namespace

EncounterDefinition const& ToCIcehowlDefinition()
{
    static EncounterDefinition const definition(static_cast<uint32>(ToCEncounter::NorthrendBeasts), &ToCGateOpen,
                                                &DefineIcehowl);
    return definition;
}
