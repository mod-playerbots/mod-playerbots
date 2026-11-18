/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "ToCDefinitions.h"

#include "Strategy.h"
#include "ToCActions_Jormungars.h"
#include "ToCActions_NorthrendBeasts.h"
#include "ToCEncounterGate.h"
#include "ToCMultipliers_Jormungars.h"
#include "ToCTriggers_Jormungars.h"
#include "ToCTriggers_NorthrendBeasts.h"

namespace Family = RaidEncounterRules::Family;

namespace
{
void DefineJormungars(EncounterBuilder& e)
{
    e.Node<NorthrendWormsMobileTankDutyTrigger, NorthrendWormsTankHoldMobileWormAction>(ACTION_RAID + 1);
    e.Node<NorthrendWormsStationaryTankDutyTrigger, NorthrendWormsTankHoldStationaryWormAction>(ACTION_RAID + 1);
    e.Node<NorthrendWormsRedirectThreatTrigger, NorthrendWormsRedirectThreatAction>(ACTION_RAID + 1);
    e.Node(
        "northrend worms dps target",
        [](PlayerbotAI* botAI) -> Trigger*
        { return new NorthrendBeastsDpsTargetTrigger(botAI, "northrend worms dps target", NorthrendBeastsStem::Worms); },
        "northrend worms dps target",
        [](PlayerbotAI* botAI) -> Action*
        { return new NorthrendBeastsDpsTargetAction(botAI, "northrend worms dps target"); },
        ACTION_RAID + 2);
    e.Node<NorthrendWormsMisplacedTrigger, NorthrendWormsRepositionAction>(ACTION_EMERGENCY + 6);
    e.Node<NorthrendWormsPoolAfterWormsTrigger, NorthrendWormsLeavePoolAction>(ACTION_EMERGENCY + 6);
    e.Node<NorthrendWormsToxinStuckTrigger, NorthrendWormsPurgeToxinAction>(ACTION_EMERGENCY + 5);

    // Above the staging and breath stand walks, which hold the tick while in flight: on the move only the
    // instants go out
    e.Node<NorthrendWormsToxinCarrierHurtTrigger, NorthrendWormsHealToxinCarrierAction>(ACTION_RAID + 3);

    e.Multiplier<NorthrendWormsMoveGuardMultiplier>(Family::AnyMovement | Family::Spell);
    e.Multiplier<NorthrendWormsBileReachGuardMultiplier>(Family::AnyMovement | Family::Spell);
    e.Multiplier<NorthrendWormsRedirectGuardMultiplier>(Family::Spell);
}
}  // namespace

EncounterDefinition const& ToCJormungarsDefinition()
{
    static EncounterDefinition const definition(static_cast<uint32>(ToCEncounter::NorthrendBeasts), &ToCGateOpen,
                                                &DefineJormungars);
    return definition;
}
