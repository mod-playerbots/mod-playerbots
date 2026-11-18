/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "ToCMultipliers_FactionChampionsDefence.h"
#include "ToCActions_FactionChampions.h"
#include "ToCEncounterGate.h"
#include "ToCHelpers_FactionChampions.h"
#include "ToCHelpers_FactionChampionsDefence.h"
#include "GenericSpellActions.h"
#include "MageActions.h"
#include "MovementActions.h"
#include "PetsAction.h"
#include "Playerbots.h"
#include "ReachTargetActions.h"
#include "ShamanActions.h"
#include "SpellInfo.h"
#include "SpellMgr.h"

using namespace TrialOfTheCrusaderHelpers;

float FactionChampionsAoeGuardMultiplier::GetValue(Action* action)
{
    auto const classify = [action]
    { return dynamic_cast<ReachMeleeAction*>(action) || dynamic_cast<SetBehindTargetAction*>(action); };

    if (!_meleeMove.Get(action, classify))
        return 1.0f;

    if (!ToCEncounterIsLive(botAI, ToCEncounter::FactionChampions))
        return 1.0f;

    return FactionChampionsReachIntoAoe(botAI) ? 0.0f : 1.0f;
}

float FactionChampionsPhysicalSwitchGuardMultiplier::GetValue(Action* action)
{
    if (!dynamic_cast<FactionChampionsFocusPriorityAction*>(action))
        return 1.0f;

    if (!ToCEncounterIsLive(botAI, ToCEncounter::FactionChampions))
        return 1.0f;

    return FactionChampionsPhysicalSwitchTarget(botAI) ? 0.0f : 1.0f;
}

float FactionChampionsDispelGuardMultiplier::GetValue(Action* action)
{
    // Cleanse removes magic too, whichever aura type its node aimed at. Neither cure class ever changes
    // its spell.
    auto const classify = [action]
    {
        CastSpellAction* cure = dynamic_cast<CastCureSpellAction*>(action);
        if (!cure)
            cure = dynamic_cast<CurePartyMemberAction*>(action);

        return cure && (cure->getSpell() == "dispel magic" || cure->getSpell() == "cleanse");
    };

    if (!_magicDispel.Get(action, classify))
        return 1.0f;

    if (!ToCEncounterIsLive(botAI, ToCEncounter::FactionChampions))
        return 1.0f;

    Unit* target = action->GetTarget();
    return target && FactionChampionsDispelBackfires(botAI, target) ? 0.0f : 1.0f;
}

float FactionChampionsPurgeGuardMultiplier::GetValue(Action* action)
{
    auto const classify = [action]
    { return dynamic_cast<CastPurgeAction*>(action) || dynamic_cast<CastSpellstealAction*>(action); };

    if (!_purge.Get(action, classify))
        return 1.0f;

    if (!ToCEncounterIsLive(botAI, ToCEncounter::FactionChampions))
        return 1.0f;

    Unit* target = action->GetTarget();
    return target && target->IsCreature() && IsFactionChampion(target->GetEntry()) ? 0.0f : 1.0f;
}

float FactionChampionsGripGuardMultiplier::GetValue(Action* action)
{
    auto const classify = [action]
    {
        Facts facts;
        facts.petToggle = dynamic_cast<TogglePetSpellAutoCastAction*>(action);
        facts.cast = facts.petToggle ? nullptr : dynamic_cast<CastSpellAction*>(action);
        return facts;
    };

    Facts& facts = _facts.Get(action, classify);
    if (!facts.petToggle && !facts.cast)
        return 1.0f;

    if (!ToCEncounterIsLive(botAI, ToCEncounter::FactionChampions) || !FactionChampionsGripGuardActive(botAI))
        return 1.0f;

    if (facts.petToggle)
        return 0.0f;

    uint32 const spellId = facts.spellId.Get(botAI, facts.cast);
    SpellInfo const* info = spellId ? sSpellMgr->GetSpellInfo(spellId) : nullptr;
    if (!info || !FactionChampionsSpellHoldsTarget(spellId))
        return 1.0f;

    if (info->IsAffectingArea() || info->IsTargetingArea() || info->IsPositive())
        return 0.0f;

    // GetTarget last, it scans every attacker
    return FactionChampionsGripGuarded(botAI, action->GetTarget()) ? 0.0f : 1.0f;
}
