/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "ToCDefinitions.h"

#include "Strategy.h"
#include "ToCActions_FactionChampionsDefence.h"
#include "ToCEncounterGate.h"
#include "ToCMultipliers_FactionChampionsDefence.h"
#include "ToCTriggers_FactionChampionsDefence.h"

namespace Family = RaidEncounterRules::Family;

namespace
{
void DefineFactionChampionsDefence(EncounterBuilder& e)
{
    e.Node<FactionChampionsPersonalDefensiveTrigger, FactionChampionsPersonalDefensiveAction>(ACTION_EMERGENCY + 8.4f);
    e.Node<FactionChampionsAvoidAoeTrigger, FactionChampionsAvoidAoeAction>(ACTION_EMERGENCY + 6);
    e.Node<FactionChampionsChannelInterruptTrigger, FactionChampionsChannelInterruptAction>(ACTION_INTERRUPT + 4);
    e.Node<FactionChampionsProtectTrigger, FactionChampionsProtectAction>(ACTION_EMERGENCY + 4);
    e.Node<FactionChampionsFreedomTrigger, FactionChampionsFreedomAction>(ACTION_EMERGENCY + 3);

    // over the ice duty nodes, so a link leaves its champion for it
    e.Node<FactionChampionsChainGripTrigger, FactionChampionsChainGripAction>(ACTION_RAID + 4.6f);
    e.Node<FactionChampionsDeathGripTrigger, FactionChampionsDeathGripAction>(ACTION_RAID + 4.5f);
    e.Node<FactionChampionsBattleResTrigger, FactionChampionsBattleResAction>(ACTION_RAID + 4.2f);
    e.Node<FactionChampionsMassDispelTrigger, FactionChampionsMassDispelAction>(ACTION_RAID + 5);
    e.Node<FactionChampionsDispelCcTrigger, FactionChampionsDispelCcAction>(ACTION_RAID + 3.5f);
    e.Node<FactionChampionsCrowdControlTrigger, FactionChampionsCrowdControlAction>(ACTION_RAID + 3.4f);
    e.Node<FactionChampionsPeelTrigger, FactionChampionsPeelAction>(ACTION_RAID + 3.3f);
    e.Node<FactionChampionsSnareTrigger, FactionChampionsSnareAction>(ACTION_RAID + 3.2f);
    e.Node<FactionChampionsKillTotemTrigger, FactionChampionsKillTotemAction>(ACTION_RAID + 2.7f);
    e.Node<FactionChampionsPhysicalSwitchTrigger, FactionChampionsPhysicalSwitchAction>(ACTION_RAID + 2.5f);

    // under the kite and the Nitro run, over the holds and the focus
    e.Node<FactionChampionsWallGripTrigger, FactionChampionsWallGripAction>(ACTION_RAID + 2.3f);
    e.Node<FactionChampionsPurgeKillTargetTrigger, FactionChampionsPurgeKillTargetAction>(ACTION_RAID);

    e.Multiplier<FactionChampionsGripGuardMultiplier>(Family::AnyAction);
    e.Multiplier<FactionChampionsAoeGuardMultiplier>(Family::AnyMovement);
    e.Multiplier<FactionChampionsPhysicalSwitchGuardMultiplier>(Family::AnyMovement);
    e.Multiplier<FactionChampionsDispelGuardMultiplier>(Family::Spell);
    e.Multiplier<FactionChampionsPurgeGuardMultiplier>(Family::Spell);
}
}  // namespace

EncounterDefinition const& ToCFactionChampionsDefenceDefinition()
{
    static EncounterDefinition const definition(static_cast<uint32>(ToCEncounter::FactionChampions), &ToCGateOpen,
                                                &DefineFactionChampionsDefence);
    return definition;
}
