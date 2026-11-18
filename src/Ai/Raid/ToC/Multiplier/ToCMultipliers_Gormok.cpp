/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "ToCMultipliers_Gormok.h"

#include <string>

#include "AttackAction.h"
#include "GenericSpellActions.h"
#include "MovementActions.h"
#include "Playerbots.h"
#include "ToCEncounterGate.h"
#include "ToCHelpers_Gormok.h"

using namespace TrialOfTheCrusaderHelpers;

namespace
{
// Both fire on a carrier that close to Gormok and throw it back out of his melee. By name, so no class
// headers.
bool IsBlinkOrDisengage(Action* action)
{
    if (!dynamic_cast<CastSpellAction*>(action))
        return false;

    std::string const name = action->getName();
    return name == "blink" || name == "disengage";
}

bool StaysFree(std::string const& name)
{
    if (name == "avoid aoe")
        return true;

    // The beasts' own movers stay, the carrier walk included: on heroic a carrier can still have a
    // worm or Icehowl's charge to dodge
    ToCEncounter encounter = ToCEncounter::None;
    return ToCEncounterOfTrigger(name, encounter) && encounter == ToCEncounter::NorthrendBeasts;
}
}  // namespace

float GormokSnoboldCarrierMultiplier::GetValue(Action* action)
{
    auto const classify = [action]
    {
        Facts facts;
        facts.spellMover = IsBlinkOrDisengage(action);
        // AttackAction is a MovementAction too, and zeroing it would strand the carrier without a target
        facts.mover = dynamic_cast<MovementAction*>(action) && !dynamic_cast<AttackAction*>(action);
        facts.staysFree = !facts.spellMover && facts.mover && StaysFree(action->getName());
        return facts;
    };

    Facts const& facts = _facts.Get(action, classify);
    if (!facts.spellMover && !facts.mover)
        return 1.0f;

    if (!ToCEncounterIsLive(botAI, ToCEncounter::NorthrendBeasts) || !GetGormokForSnoboldCarrier(botAI))
        return 1.0f;

    if (facts.spellMover)
        return 0.0f;

    return facts.staysFree ? 1.0f : 0.0f;
}

void AddToCGormokMultipliers(PlayerbotAI* botAI, std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new GormokSnoboldCarrierMultiplier(botAI));
}
