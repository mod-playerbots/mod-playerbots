/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RAID_TOCRAIDTRIGGERCONTEXT_H
#define PLAYERBOTS_RAID_TOCRAIDTRIGGERCONTEXT_H

#include "NamedObjectContext.h"
#include "ToCDefinitions.h"

class RaidTrialOfTheCrusaderTriggerContext : public NamedObjectContext<Trigger>
{
public:
    RaidTrialOfTheCrusaderTriggerContext() : NamedObjectContext<Trigger>()
    {
        for (EncounterDefinition const* encounter : ToCEncounterDefinitions())
            encounter->RegisterTriggers(creators);
    }
};

#endif
