/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "ToCDefinitions.h"

#include "Strategy.h"
#include "ToCActions_FactionChampionsBall.h"
#include "ToCEncounterGate.h"
#include "ToCTriggers_FactionChampionsBall.h"

namespace
{
void DefineFactionChampionsBall(EncounterBuilder& e)
{
    e.Node<FactionChampionsOpenerTrigger, FactionChampionsOpenerAction>(ACTION_RAID + 4.4f);

    // instants, cast before the Nitro run they free up
    e.Node<FactionChampionsEscapeTrigger, FactionChampionsEscapeAction>(ACTION_RAID + 2.6f);
    e.Node<FactionChampionsChasedTrigger, FactionChampionsChasedAction>(ACTION_RAID + 2.5f);

    // over the kite, which takes every tick of its walk; under Nitro, whose faster boost would cancel it out
    e.Node<FactionChampionsSpeedBoostTrigger, FactionChampionsSpeedBoostAction>(ACTION_RAID + 2.48f);

    // an instant, so the kite's walk goes on after it
    e.Node<FactionChampionsChainsOfIceTrigger, FactionChampionsChainsOfIceAction>(ACTION_RAID + 2.47f);
    e.Node<FactionChampionsKiteTrigger, FactionChampionsKiteAction>(ACTION_RAID + 2.45f);
    e.Node<FactionChampionsIceReachTrigger, FactionChampionsIceReachAction>(ACTION_RAID + 2.44f);
    e.Node<FactionChampionsStepOffTrigger, FactionChampionsStepOffAction>(ACTION_RAID + 2.4f);
    e.Node<FactionChampionsIceTargetTrigger, FactionChampionsIceTargetAction>(ACTION_RAID + 2);
    e.Node<FactionChampionsCycloneStandTrigger, FactionChampionsCycloneStandAction>(ACTION_RAID + 1.9f);

    // pre-pull, above the staging walk, which returns false once parked
    e.Node<FactionChampionsDemonicCircleTrigger, FactionChampionsDemonicCircleAction>(ACTION_RAID + 1);
}
}  // namespace

EncounterDefinition const& ToCFactionChampionsBallDefinition()
{
    static EncounterDefinition const definition(static_cast<uint32>(ToCEncounter::FactionChampions), &ToCGateOpen,
                                                &DefineFactionChampionsBall);
    return definition;
}
