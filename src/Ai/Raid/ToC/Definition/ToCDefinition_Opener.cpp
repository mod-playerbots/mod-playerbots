/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "ToCDefinitions.h"

#include <functional>

#include "RaidOpener.h"
#include "ToCEncounterGate.h"
#include "ToCHelpers_Opener.h"

namespace Family = RaidEncounterRules::Family;

namespace
{
void DefineOpener(EncounterBuilder& e)
{
    e.Multiplier<RaidOpenerHoldMultiplier>(Family::Spell | Family::Attack, "toc opener hold",
                                           std::cref(TrialOfTheCrusaderHelpers::TOC_OPENER));
}
}  // namespace

EncounterDefinition const& ToCOpenerDefinition()
{
    static EncounterDefinition const definition(static_cast<uint32>(ToCEncounter::None), &ToCRaidWideGateOpen,
                                                &DefineOpener);
    return definition;
}
