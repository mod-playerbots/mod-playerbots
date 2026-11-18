/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "ToCMultipliers_Jormungars.h"

#include <string>

#include "AttackAction.h"
#include "GenericSpellActions.h"
#include "HunterActions.h"
#include "MovementActions.h"
#include "Playerbots.h"
#include "ReachTargetActions.h"
#include "RogueActions.h"
#include "ToCEncounterGate.h"
#include "ToCHelpers_Jormungars.h"
#include "ToCHelpers_NorthrendBeasts.h"

using namespace TrialOfTheCrusaderHelpers;

namespace
{
// Spells that move the bot: the charge family leaps it back to its target, blink and disengage throw
// it a fixed distance. The last two by name, so no class headers.
bool IsSpellMover(Action* action)
{
    if (!dynamic_cast<CastSpellAction*>(action))
        return false;

    if (dynamic_cast<CastReachTargetSpellAction*>(action))
        return true;

    std::string const name = action->getName();
    return name == "blink" || name == "disengage";
}

bool StaysFree(std::string const& name)
{
    if (name == "avoid aoe")
        return true;

    // The reposition walk itself, and on heroic Gormok's walks and Icehowl's charge dodge
    ToCEncounter encounter = ToCEncounter::None;
    return ToCEncounterOfTrigger(name, encounter) && encounter == ToCEncounter::NorthrendBeasts;
}
}  // namespace

float NorthrendWormsMoveGuardMultiplier::GetValue(Action* action)
{
    auto const classify = [action]
    {
        Facts facts;
        facts.spellMover = IsSpellMover(action);
        // AttackAction is a MovementAction too, and zeroing it would leave the bot without a target
        facts.mover = dynamic_cast<MovementAction*>(action) && !dynamic_cast<AttackAction*>(action);
        facts.staysFree = !facts.spellMover && facts.mover && StaysFree(action->getName());
        return facts;
    };

    Facts const& facts = _facts.Get(action, classify);
    if (!facts.spellMover && !facts.mover)
        return 1.0f;

    if (!ToCEncounterIsLive(botAI, ToCEncounter::NorthrendBeasts) || !IsWormWalkInFlight(botAI))
        return 1.0f;

    if (facts.spellMover)
        return 0.0f;

    return facts.staysFree ? 1.0f : 0.0f;
}

float NorthrendWormsBileReachGuardMultiplier::GetValue(Action* action)
{
    auto const classify = [action]
    { return dynamic_cast<ReachMeleeAction*>(action) || dynamic_cast<CastReachTargetSpellAction*>(action); };

    if (!_reach.Get(action, classify))
        return 1.0f;

    if (!ToCEncounterIsLive(botAI, ToCEncounter::NorthrendBeasts))
        return 1.0f;

    return IsWormBileCarrierHeldOut(botAI) ? 0.0f : 1.0f;
}

float NorthrendWormsRedirectGuardMultiplier::GetValue(Action* action)
{
    if (bot->getClass() != CLASS_HUNTER && bot->getClass() != CLASS_ROGUE)
        return 1.0f;

    auto const classify = [action]
    {
        return dynamic_cast<CastMisdirectionOnMainTankAction*>(action) ||
               dynamic_cast<CastTricksOfTheTradeOnMainTankAction*>(action);
    };

    if (!_classRedirect.Get(action, classify))
        return 1.0f;

    if (!ToCEncounterIsLive(botAI, ToCEncounter::NorthrendBeasts))
        return 1.0f;

    return (GetBeastsStageMask(botAI) & BEASTS_STAGE_WORMS) ? 0.0f : 1.0f;
}

void AddToCJormungarsMultipliers(PlayerbotAI* botAI, std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new NorthrendWormsMoveGuardMultiplier(botAI));
    multipliers.push_back(new NorthrendWormsBileReachGuardMultiplier(botAI));
    multipliers.push_back(new NorthrendWormsRedirectGuardMultiplier(botAI));
}
