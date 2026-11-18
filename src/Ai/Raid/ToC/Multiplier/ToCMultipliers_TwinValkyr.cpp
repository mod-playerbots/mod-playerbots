/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "ToCMultipliers_TwinValkyr.h"

#include <cstring>
#include <string>

#include "Action.h"
#include "EncounterHelpers.h"
#include "HunterActions.h"
#include "MovementActions.h"
#include "Playerbots.h"
#include "RaidCombatHelpers.h"
#include "RaidOpener.h"
#include "RogueActions.h"
#include "ToCEncounterGate.h"
#include "ToCHelpers_TwinValkyr.h"

using namespace TrialOfTheCrusaderHelpers;

namespace
{
// The shield's absorb carries interrupt immunity, so these only work once it breaks
constexpr char const* SHIELD_BLOCKED_INTERRUPTS[] = {"kick",       "pummel",      "shield bash", "counterspell",
                                                      "wind shear", "mind freeze", "spell lock"};
// Silence and stun, both in the twins' immunity set
constexpr char const* REFUSED_INTERRUPTS[] = {"silencing shot", "strangulate", "silence", "hammer of justice", "bash"};

// "kick on enemy healer" and "hammer of justice on snare target" cast the same spell
bool NamesSpell(std::string const& action, char const* spell)
{
    size_t const length = std::strlen(spell);
    return action.compare(0, length, spell) == 0 &&
           (action.size() == length || action.compare(length, 4, " on ") == 0);
}

TwinInterruptKind InterruptKindOf(std::string const& action)
{
    for (char const* spell : SHIELD_BLOCKED_INTERRUPTS)
        if (NamesSpell(action, spell))
            return TwinInterruptKind::Kick;

    for (char const* spell : REFUSED_INTERRUPTS)
        if (NamesSpell(action, spell))
            return TwinInterruptKind::Refused;

    return TwinInterruptKind::None;
}

// Self-cast ones like challenging shout, and righteous defense on an ally, hit whatever the bot is
// fighting
Unit* ActionTwinTarget(Action* action, AiObjectContext* context)
{
    Unit* target = action->GetTarget();
    if (TwinColourOf(target) != TwinColour::None)
        return target;

    Unit* current = context->GetValue<Unit*>("current target")->Get();
    return TwinColourOf(current) != TwinColour::None ? current : nullptr;
}
}  // namespace

float TwinValkyrControlTankMovementMultiplier::GetValue(Action* action)
{
    if (!dynamic_cast<CombatFormationMoveAction*>(action))
        return 1.0f;

    if (!ToCEncounterIsLive(botAI, ToCEncounter::TwinValkyr))
        return 1.0f;

    Unit* victim = bot->GetVictim();
    return victim && IsTwinTank(bot, victim) ? 0.0f : 1.0f;
}

float TwinValkyrTauntGuardMultiplier::GetValue(Action* action)
{
    if (!action || !EncounterHelpers::IsTauntAction(bot, action))
        return 1.0f;

    if (!ToCEncounterIsLive(botAI, ToCEncounter::TwinValkyr))
        return 1.0f;

    Unit* twin = ActionTwinTarget(action, context);
    if (!twin)
        return 1.0f;

    Player* owner = GetTwinTank(bot, twin);
    return owner && owner != bot ? 0.0f : 1.0f;
}

float TwinValkyrInterruptHoldMultiplier::GetValue(Action* action)
{
    if (!action)
        return 1.0f;

    TwinInterruptKind const kind = _kinds.Get(action, [action] { return InterruptKindOf(action->getName()); });
    if (kind == TwinInterruptKind::None)
        return 1.0f;

    if (!ToCEncounterIsLive(botAI, ToCEncounter::TwinValkyr))
        return 1.0f;

    if (kind == TwinInterruptKind::Kick)
        return GetShieldedTwin(botAI) ? 0.0f : 1.0f;

    return ActionTwinTarget(action, context) ? 0.0f : 1.0f;
}

float TwinValkyrRedirectGuardMultiplier::GetValue(Action* action)
{
    if (bot->getClass() != CLASS_HUNTER && bot->getClass() != CLASS_ROGUE)
        return 1.0f;

    bool const classRedirect = _redirects.Get(action,
                                             [action]
                                             {
                                                 return dynamic_cast<CastMisdirectionOnMainTankAction*>(action) ||
                                                        dynamic_cast<CastTricksOfTheTradeOnMainTankAction*>(action) ||
                                                        dynamic_cast<RaidOpenerRedirectAction*>(action);
                                             });
    if (!classRedirect)
        return 1.0f;

    return ToCEncounterIsLive(botAI, ToCEncounter::TwinValkyr) ? 0.0f : 1.0f;
}

// A DPS bot's cooldowns stack with lust. Healers hold only Power Infusion, a rule of the encounter's,
// and tanks nothing.
RaidBurstWindow ToCTwinValkyrBurstWindow(PlayerbotAI* botAI)
{
    RaidBurstWindow window;
    window.allowLust = TwinValkyrLustWindowOpen(botAI);
    window.allowAll = !IsRaidDpsBot(botAI->GetBot()) || !TwinValkyrBurstHeld(botAI);
    return window;
}
