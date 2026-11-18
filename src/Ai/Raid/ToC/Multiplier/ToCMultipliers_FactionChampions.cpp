/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "ToCMultipliers_FactionChampions.h"

#include <set>
#include <string>

#include "ToCEncounterGate.h"
#include "ToCHelpers_FactionChampions.h"
#include "GenericSpellActions.h"
#include "HunterActions.h"
#include "Playerbots.h"
#include "RaidSplash.h"
#include "RogueActions.h"
#include "Spell.h"
#include "SpellInfo.h"
#include "SpellMgr.h"

using namespace TrialOfTheCrusaderHelpers;

namespace
{
// Bounces and tank splash the generic area test misses
std::set<std::string> const OWN_SPLASH = {"death and decay", "pestilence", "hammer of the righteous",
                                          "avenger's shield", "blood boil", "consecration", "swipe (bear)"};
}  // namespace

// Harmful splash only: area buffs like Battle Shout land on the raid
bool FactionChampionsSuppressAoeMultiplier::Splashes(Action* action)
{
    RaidSplashCache::Facts& facts = _splash.Of(action);
    bool const ownSplash = _ownSplash.Get(action, [action] { return OWN_SPLASH.count(action->getName()) != 0; });

    // the summon spell reads as positive, but what it drops burns every champion round it
    if (ownSplash || facts.summon)
        return true;

    if (!SpellSplashes(botAI, action, _splash))
        return false;

    if (!facts.cast)
        return true;

    uint32 const spellId = facts.spellId.Get(botAI, facts.cast);
    SpellInfo const* info = spellId ? sSpellMgr->GetSpellInfo(spellId) : nullptr;
    return !info || !info->IsPositive();
}

// Also every own splash: a Death and Decay or Consecration on the pack breaks every Polymorph and Fear in
// it, and Pestilence puts a DoT on each champion, which rules them out for both
float FactionChampionsSuppressAoeMultiplier::GetValue(Action* action)
{
    // AoE heals land on the raid, not the champions, so they stay
    if (!action || _splash.Of(action).heal)
        return 1.0f;

    if (!_splash.Of(action).aoeThreat && !Splashes(action))
        return 1.0f;

    return ToCEncounterIsLive(botAI, ToCEncounter::FactionChampions) ? 0.0f : 1.0f;
}

float FactionChampionsThreatRedirectVetoMultiplier::GetValue(Action* action)
{
    auto const classify = [action]
    {
        Facts facts;
        facts.mainTankRedirect = dynamic_cast<CastMisdirectionOnMainTankAction*>(action) ||
                                 dynamic_cast<CastTricksOfTheTradeOnMainTankAction*>(action);
        return facts;
    };

    Facts const& facts = _facts.Get(action, classify);
    if (!facts.mainTankRedirect)
        return 1.0f;

    return ToCEncounterIsLive(botAI, ToCEncounter::FactionChampions) ? 0.0f : 1.0f;
}

bool FactionChampionsAntiFearTotemGuardMultiplier::FearWindowActive()
{
    return FactionChampionsFearWindowActive(botAI);
}

float FactionChampionsCcGuardMultiplier::GetValue(Action* action)
{
    auto const classify = [action]
    {
        Facts facts;
        facts.classCc = dynamic_cast<CastCrowdControlSpellAction*>(action);
        facts.spreadDot = dynamic_cast<CastDebuffSpellOnAttackerAction*>(action) ||
                          dynamic_cast<CastDebuffSpellOnMeleeAttackerAction*>(action);
        return facts;
    };

    Facts const& facts = _facts.Get(action, classify);
    if ((!facts.classCc && !facts.spreadDot) || !ToCEncounterIsLive(botAI, ToCEncounter::FactionChampions))
        return 1.0f;

    if (facts.classCc)
        return 0.0f;

    // a spread DoT on a champion off the kill target blocks Polymorph, Fear and Hex on it for its whole run.
    // GetTarget last, it scans every attacker.
    Unit* target = action->GetTarget();
    return target && target->IsCreature() && IsFactionChampion(target->GetEntry()) &&
                   target != FactionChampionsKillTarget(botAI)
               ? 0.0f
               : 1.0f;
}

namespace
{
bool Interrupts(SpellInfo const* info)
{
    return info->HasEffect(SPELL_EFFECT_INTERRUPT_CAST) || info->HasAura(SPELL_AURA_MOD_SILENCE);
}

bool EndsWith(std::string const& text, std::string const& tail)
{
    return text.size() >= tail.size() && text.compare(text.size() - tail.size(), tail.size(), tail) == 0;
}
}  // namespace

float FactionChampionsLateInterruptMultiplier::GetValue(Action* action)
{
    auto const classify = [action]
    {
        Facts facts;
        facts.cast = dynamic_cast<CastSpellAction*>(action);
        facts.healerKick = facts.cast && EndsWith(action->getName(), " on enemy healer");
        return facts;
    };

    Facts& facts = _facts.Get(action, classify);
    if (!facts.cast || !ToCEncounterIsLive(botAI, ToCEncounter::FactionChampions))
        return 1.0f;

    uint32 const spellId = facts.spellId.Get(botAI, facts.cast);
    SpellInfo const* info = spellId ? sSpellMgr->GetSpellInfo(spellId) : nullptr;
    if (!info)
        return 1.0f;

    if (!Interrupts(info) && !facts.healerKick)
        return FactionChampionsCounterspellHold(botAI) && info->CalcCastTime(bot) > 0 ? 0.0f : 1.0f;

    // GetTarget last, it scans every attacker
    Unit* target = action->GetTarget();
    if (!target || !target->IsCreature() || !IsFactionChampion(target->GetEntry()))
        return 1.0f;

    Spell* spell = target->GetCurrentSpell(CURRENT_GENERIC_SPELL);
    return spell && spell->GetCastTimeRemaining() > LATE_INTERRUPT_MS ? 0.0f : 1.0f;
}

void AddToCFactionChampionsMultipliers(PlayerbotAI* botAI, std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new FactionChampionsLateInterruptMultiplier(botAI));
    multipliers.push_back(new FactionChampionsSuppressAoeMultiplier(botAI));
    multipliers.push_back(new FactionChampionsThreatRedirectVetoMultiplier(botAI));
    multipliers.push_back(new FactionChampionsAntiFearTotemGuardMultiplier(botAI));
    multipliers.push_back(new FactionChampionsCcGuardMultiplier(botAI));
}

// Lust and every cooldown go at the pull, each early kill makes the rest easier
RaidBurstWindow ToCFactionChampionsBurstWindow(PlayerbotAI* /*botAI*/)
{
    return {true, true};
}
