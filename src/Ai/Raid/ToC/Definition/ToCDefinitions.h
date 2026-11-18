/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RAID_TOCDEFINITIONS_H
#define PLAYERBOTS_RAID_TOCDEFINITIONS_H

#include "RaidEncounter.h"

// Rules only, no rows or tick: every node stays in its stem's context.
EncounterDefinition const& ToCNorthrendBeastsDefinition();
EncounterDefinition const& ToCJaraxxusDefinition();
EncounterDefinition const& ToCFactionChampionsDefinition();
EncounterDefinition const& ToCTwinValkyrDefinition();
EncounterDefinition const& ToCAnubarakDefinition();
EncounterDefinition const& ToCStagingDefinition();
EncounterDefinition const& ToCResistAuraDefinition();
EncounterDefinition const& ToCOpenerDefinition();

#endif
