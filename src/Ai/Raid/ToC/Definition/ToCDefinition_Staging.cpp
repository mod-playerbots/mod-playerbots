/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "ToCDefinitions.h"

#include "PlayerbotAI.h"
#include "Strategy.h"
#include "ToCActions_Staging.h"
#include "ToCEncounterGate.h"
#include "ToCHelpers_Staging.h"
#include "ToCTriggers_Staging.h"

using namespace TrialOfTheCrusaderHelpers;

namespace Family = RaidEncounterRules::Family;
namespace Role = RaidEncounterRules::Role;

namespace
{
void DefineStaging(EncounterBuilder& e)
{
    // the walk holds the tick while in flight, the stand gives it back once parked
    e.Node<ToCStagingStandTrigger, ToCMoveToStagingStandAction>(ACTION_RAID);

    // follow and stay walk a parked bot back to its player. out of combat tank and dps assist pull on a
    // skull, and Anub'arak turns attackable once anyone is inside 80 yd
    e.Block("toc staging hold", Role::Any, IsToCStagingHeld,
            Family::Follow | Family::CombatFormationMove | Family::SetBehind | Family::TankFace | Family::RearFlank |
                Family::MoveRandom | Family::TankAssist | Family::DpsAssist,
            {"return to stay position", "return"});
    // eating blinds a bot for 12-18 s
    e.Block("toc staging rest guard", Role::Any, IsToCStagingRestHeld, 0, {"eat", "drink"});
    // a druid tank drops bear form out of combat and nothing shifts him back
    e.Block("toc staging tank form guard", Role::Tank, IsToCStagingHeld, 0, {"caster form"});
}
}  // namespace

EncounterDefinition const& ToCStagingDefinition()
{
    static EncounterDefinition const definition(static_cast<uint32>(ToCEncounter::None), &ToCRaidWideGateOpen,
                                                &DefineStaging);
    return definition;
}
