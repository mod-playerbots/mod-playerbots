/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "ToCDefinitions.h"

#include "Strategy.h"
#include "ToCActions_Gormok.h"
#include "ToCActions_NorthrendBeasts.h"
#include "ToCEncounterGate.h"
#include "ToCMultipliers_Gormok.h"
#include "ToCTriggers_Gormok.h"
#include "ToCTriggers_NorthrendBeasts.h"

namespace Family = RaidEncounterRules::Family;

namespace
{
void DefineGormok(EncounterBuilder& e)
{
    e.Node<GormokTankDutyTrigger, GormokTankHoldBossAction>(ACTION_RAID + 1);
    e.Node<GormokSwapTankDutyTrigger, GormokSwapTankHoldAction>(ACTION_RAID + 1);
    e.Node(
        "gormok dps target",
        [](PlayerbotAI* botAI) -> Trigger*
        { return new NorthrendBeastsDpsTargetTrigger(botAI, "gormok dps target", NorthrendBeastsStem::Gormok); },
        "gormok dps target",
        [](PlayerbotAI* botAI) -> Action* { return new NorthrendBeastsDpsTargetAction(botAI, "gormok dps target"); },
        ACTION_RAID + 2);
    e.Node<GormokSnobolledTrigger, GormokBringSnoboldToMeleeAction>(ACTION_RAID + 3);
    e.Node<GormokStompRangeTrigger, GormokLeaveStompRangeAction>(ACTION_RAID + 3);
    e.Node<GormokTankSwapNeededTrigger, GormokTankSwapTauntAction>(ACTION_RAID + 5);
    e.Node<GormokTankDefensiveTrigger, GormokTankDefensiveAction>(ACTION_RAID + 6);
    e.Node<GormokImpaleResetTrigger, GormokHandOfProtectionAction>(ACTION_RAID + 5);

    // Off the global cooldown, and the hand pacifies the tank till it goes
    e.Node<GormokCancelHandOfProtectionTrigger, GormokCancelHandOfProtectionAction>(ACTION_EMERGENCY + 1);
    e.Node<GormokFireBombIncomingTrigger, GormokDodgeFireBombAction>(ACTION_EMERGENCY + 2);

    e.Multiplier<GormokSnoboldCarrierMultiplier>(Family::AnyMovement | Family::Spell);
}
}  // namespace

EncounterDefinition const& ToCGormokDefinition()
{
    static EncounterDefinition const definition(static_cast<uint32>(ToCEncounter::NorthrendBeasts), &ToCGateOpen,
                                                &DefineGormok);
    return definition;
}
