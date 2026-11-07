/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "UldMultipliers_IronAssembly.h"

#include "AttackAction.h"
#include "BurstCooldowns.h"
#include "ChooseTargetActions.h"
#include "EncounterHelpers.h"
#include "FollowActions.h"
#include "GenericSpellActions.h"
#include "HunterActions.h"
#include "MageActions.h"
#include "MovementActions.h"
#include "PaladinActions.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "PriestActions.h"
#include "ReachTargetActions.h"
#include "RogueActions.h"
#include "SharedDefines.h"
#include "Spell.h"
#include "SpellMgr.h"
#include "Timer.h"
#include "UldActions.h"
#include "UldEncounter_IronAssembly.h"
#include "UldHardMode.h"
#include "UldScripts.h"
#include "UldTriggers.h"
#include "VehicleActions.h"

#include <set>
#include <string>

using namespace EncounterHelpers;

namespace
{
// One round at the pull. Lust is out by name: Sated makes it once per 10 min. Trinkets go through by
// name since the action carries no spell to measure.
bool IronAssemblyOpenerBurstAllowed(PlayerbotAI* botAI, Player* bot, std::string const& name)
{
    if (name == "bloodlust" || name == "heroism")
        return false;

    if (!IronAssemblyOpenerBurstWindowOpen(botAI))
        return false;

    if (name == "use trinket")
        return true;

    uint32 const spellId = botAI->GetAiObjectContext()->GetValue<uint32>("spell id", name)->Get();
    return spellId && SpellCooldownMs(bot, spellId) <= ULDUAR_IRON_ASSEMBLY_OPENER_BURST_MAX_COOLDOWN_MS;
}
}  // namespace

// Iron Assembly
float IronAssemblyHoldDpsCooldownsMultiplier::GetValue(Action* action)
{
    if (!IsIronAssemblyHardModeActive(botAI))
        return 1.0f;

    // Both predicates, each misses some: IsDpsCooldownAction has no potion, tinker or priest case, and its
    // racial branch keys on race, not HasSpell. IsDps keeps healers and tanks out of the name branch.
    std::string const name = action->getName();
    if (IsManaReturnCooldown(bot, name))
        return 1.0f;

    bool const burst = IsDpsCooldownAction(bot, action) ||
                       (PlayerbotAI::IsDps(bot) && IsBurstCooldownAction(name));
    if (!burst)
        return 1.0f;

    if (!IronAssemblyFormationActive(botAI))
        return 1.0f;

    // Released the moment Steelbreaker is the last one standing. Under the normal order he dies first
    // and this would never release, which is why it is gated on the option rather than the phase.
    if (IsSteelbreakerEmpowered(botAI))
        return 1.0f;

    return IronAssemblyOpenerBurstAllowed(botAI, bot, name) ? 1.0f : 0.0f;
}

float IronAssemblyTauntGuardMultiplier::GetValue(Action* action)
{
    // Taunts first: this runs on every popped action and the lookups below reach the council sweep.
    if (!action || !IsTauntAction(bot, action))
        return 1.0f;

    if (!IronAssemblyFormationActive(botAI) || !IsSteelbreakerEmpowered(botAI))
        return 1.0f;

    // No current target check, unlike Thorim's guard: righteous defense goes on the ally, and nothing
    // else in this phase is worth a taunt. The Meltdown swap is the one taunt off another tank.
    if (!IronAssemblyHeldByOtherTank(bot, GetIronAssemblyMember(botAI, NPC_STEELBREAKER)))
        return 1.0f;

    return IronAssemblyMeltdownRoleFor(botAI, bot) == IronAssemblyMeltdownRole::Taunt ? 1.0f : 0.0f;
}
