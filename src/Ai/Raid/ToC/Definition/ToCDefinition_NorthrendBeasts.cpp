/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "ToCDefinitions.h"

#include "PlayerbotAI.h"
#include "ToCEncounterGate.h"
#include "ToCHelpers_Jormungars.h"
#include "ToCHelpers_NorthrendBeasts.h"

using namespace TrialOfTheCrusaderHelpers;

namespace Family = RaidEncounterRules::Family;
namespace Role = RaidEncounterRules::Role;

namespace
{
bool BeastsDpsTargetOwned(PlayerbotAI* botAI) { return GetBeastsDpsTarget(botAI) != nullptr; }

bool BeastsTankTargetOwned(PlayerbotAI* botAI)
{
    BeastsTankDuty const duty = GetBeastsTankDuty(botAI);
    return duty != BeastsTankDuty::None && GetBeastOfDuty(botAI, duty) != nullptr;
}

// a submerge drops every bot's target, and the out of combat follow then drags casters off their spots
bool WormEngaged(PlayerbotAI* botAI) { return (GetBeastsStageMask(botAI) & BEASTS_STAGE_WORMS) != 0; }

void DefineNorthrendBeasts(EncounterBuilder& e)
{
    e.OwnTargeting("northrend beasts dps target guard", Role::Any, BeastsDpsTargetOwned, Family::DpsAssist);
    // a tank with no beast up for its duty keeps tank assist, the only target source it has then
    e.OwnTargeting("northrend beasts tank target guard", Role::Any, BeastsTankTargetOwned, Family::TankAssist);

    // a bot on its worm spot stays there and the reposition walks it back if pushed off. a hunter's spot is
    // past minimum range, so flee goes too. reach goes by name, so a battle res walk isn't blocked
    e.Block("northrend worms spot hold", Role::Any, IsWormSpotHeld,
            Family::SetBehind | Family::RearFlank | Family::CombatFormationMove | Family::Follow | Family::Flee |
                Family::RunAway | Family::MoveRandom | Family::Charge | Family::Blink | Family::Disengage,
            {"reach melee", "reach spell"});
    // reach walks a straight line through the pool, the reposition takes the bot round it instead
    e.Block("northrend worms pool reach guard", Role::Any, WormReachCrossesPool, 0, {"reach melee", "reach spell"});
    e.Block("northrend worms follow guard", Role::Any, WormEngaged, Family::Follow);
}
}  // namespace

EncounterDefinition const& ToCNorthrendBeastsDefinition()
{
    static EncounterDefinition const definition(static_cast<uint32>(ToCEncounter::NorthrendBeasts), &ToCGateOpen,
                                                &DefineNorthrendBeasts);
    return definition;
}
