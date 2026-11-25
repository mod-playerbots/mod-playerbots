/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "ToCMultipliers_Jaraxxus.h"
#include "ToCActions_Jaraxxus.h"
#include "ToCData.h"
#include "ToCEncounterGate.h"
#include "ToCHelpers_Jaraxxus.h"
#include "EncounterHelpers.h"
#include "GenericSpellActions.h"
#include "MovementActions.h"
#include "Playerbots.h"
#include "RaidCombatHelpers.h"
#include "RaidSplash.h"

using namespace TrialOfTheCrusaderHelpers;
using namespace EncounterHelpers;

namespace
{
// Area damage from this close counts as hitting him
constexpr float JARAXXUS_AREA_THREAT_REACH = 30.0f;
}  // namespace

float JaraxxusControlTankMovementMultiplier::GetValue(Action* action)
{
    if (!dynamic_cast<CombatFormationMoveAction*>(action))
        return 1.0f;

    if (!PlayerbotAI::IsTank(bot) && !PlayerbotAI::IsTank(bot, true))
        return 1.0f;

    // open, not live: his adds outlive the kill and still need holding
    if (!ToCEncounterGateOpen(botAI, ToCEncounter::Jaraxxus))
        return 1.0f;

    Unit* victim = bot->GetVictim();
    if (!victim)
        return 1.0f;

    bool const held = victim->GetEntry() == static_cast<uint32>(ToCNpcs::NPC_JARAXXUS) || IsJaraxxusAdd(victim);
    return held ? 0.0f : 1.0f;
}

float JaraxxusTauntGuardMultiplier::GetValue(Action* action)
{
    if (!action || !IsTauntAction(bot, action))
        return 1.0f;

    Unit* target = AI_VALUE(Unit*, "current target");
    if (!target || target->GetEntry() != static_cast<uint32>(ToCNpcs::NPC_JARAXXUS))
        return 1.0f;

    if (!ToCEncounterIsLive(botAI, ToCEncounter::Jaraxxus))
        return 1.0f;

    Unit* victim = target->GetVictim();
    Player* holder = victim ? victim->ToPlayer() : nullptr;
    if (!holder || holder == bot)
        return 1.0f;

    return (PlayerbotAI::IsTank(holder) || PlayerbotAI::IsTank(holder, true)) ? 0.0f : 1.0f;
}

float JaraxxusKissCastHoldMultiplier::GetValue(Action* action)
{
    CastSpellAction* cast = dynamic_cast<CastSpellAction*>(action);
    if (!cast)
        return 1.0f;

    if (!HasMistressKiss(bot))
        return 1.0f;

    // open, not live: heroic Mistresses outlive the kill and keep kissing
    if (!ToCEncounterGateOpen(botAI, ToCEncounter::Jaraxxus))
        return 1.0f;

    return JaraxxusSpellHasCastTime(botAI, cast->getSpell()) ? 0.0f : 1.0f;
}

float JaraxxusAvoidAoeGuardMultiplier::GetValue(Action* action)
{
    if (!dynamic_cast<AvoidAoeAction*>(action))
        return 1.0f;

    if (!ToCEncounterGateOpen(botAI, ToCEncounter::Jaraxxus))
        return 1.0f;

    return JaraxxusAnyLegionFlame(botAI) ? 0.0f : 1.0f;
}

float JaraxxusThreatCapMultiplier::GetValue(Action* action)
{
    if (!action || dynamic_cast<CastHealingSpellAction*>(action) || !JaraxxusThreatCapped(bot))
        return 1.0f;

    Unit* boss = GetJaraxxus(botAI);
    if (!boss)
        return 1.0f;

    Unit* target = action->GetTarget();
    if (target == boss || (!target && AI_VALUE(Unit*, "current target") == boss))
        return 0.0f;

    if (IsRaidAreaDamage(botAI, action, _splash) && bot->GetExactDist2d(boss) <= JARAXXUS_AREA_THREAT_REACH)
        return 0.0f;

    return 1.0f;
}

// Lust at the pull: he never enrages, and the heroic portal at 20 s still falls inside it.
RaidBurstWindow ToCJaraxxusBurstWindow(PlayerbotAI* /*botAI*/)
{
    return {};
}
