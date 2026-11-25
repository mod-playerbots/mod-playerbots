/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RAID_TOCDEFINITIONS_H
#define PLAYERBOTS_RAID_TOCDEFINITIONS_H

#include <vector>

#include "RaidEncounter.h"

EncounterDefinition const& ToCGormokDefinition();
EncounterDefinition const& ToCJormungarsDefinition();
EncounterDefinition const& ToCIcehowlDefinition();
EncounterDefinition const& ToCNorthrendBeastsDefinition();
EncounterDefinition const& ToCJaraxxusDefinition();
EncounterDefinition const& ToCAnubarakDefinition();
EncounterDefinition const& ToCFactionChampionsDefinition();
EncounterDefinition const& ToCFactionChampionsDefenceDefinition();
EncounterDefinition const& ToCFactionChampionsBallDefinition();
EncounterDefinition const& ToCTwinValkyrDefinition();
EncounterDefinition const& ToCStagingDefinition();
EncounterDefinition const& ToCResistAuraDefinition();
EncounterDefinition const& ToCOpenerDefinition();

// Every ToC definition, in node and veto order: equal relevances pop in insertion order, and the engine
// stops at the first multiplier to zero an action. The contexts and the strategy walk this.
inline std::vector<EncounterDefinition const*> const& ToCEncounterDefinitions()
{
    static std::vector<EncounterDefinition const*> const definitions = {
        &ToCGormokDefinition(),
        &ToCJormungarsDefinition(),
        &ToCIcehowlDefinition(),
        &ToCNorthrendBeastsDefinition(),
        &ToCJaraxxusDefinition(),
        &ToCAnubarakDefinition(),
        &ToCFactionChampionsDefinition(),
        &ToCFactionChampionsDefenceDefinition(),
        &ToCFactionChampionsBallDefinition(),
        &ToCTwinValkyrDefinition(),
        &ToCStagingDefinition(),
        &ToCResistAuraDefinition(),
        &ToCOpenerDefinition(),
    };
    return definitions;
}

#endif
