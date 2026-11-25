/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "RaidSplash.h"

#include "AiObjectContext.h"
#include "GenericSpellActions.h"
#include "PlayerbotAI.h"
#include "SpellInfo.h"
#include "SpellMgr.h"

std::set<std::string> const MULTI_TARGET_ACTIONS = {
    "bladestorm",    "sweeping strikes", "cleave",        "whirlwind",  "thunder clap",
    "shockwave",     "divine storm",     "howling blast", "blood boil", "fan of knives",
    "blade flurry",  "swipe (cat)",      "multi-shot",    "fire nova",  "chain lightning"};

bool SummonSplashes(Action* action)
{
    static std::set<std::string> const summoners = {"magma totem", "fire nova", "explosive trap"};
    return action && summoners.count(action->getName());
}

bool SpellSplashes(PlayerbotAI* botAI, Action* action)
{
    if (SummonSplashes(action))
        return true;

    CastSpellAction* spellAction = dynamic_cast<CastSpellAction*>(action);
    if (!spellAction || dynamic_cast<CastHealingSpellAction*>(action))
        return false;

    uint32 const spellId =
        botAI->GetAiObjectContext()->GetValue<uint32>("spell id", spellAction->getSpell())->Get();
    SpellInfo const* spellInfo = spellId ? sSpellMgr->GetSpellInfo(spellId) : nullptr;
    return spellInfo && (spellInfo->IsTargetingArea() || spellInfo->IsAffectingArea());
}

RaidSplashCache::Facts& RaidSplashCache::Of(Action* action)
{
    return _facts.Get(action,
                      [action]
                      {
                          Facts facts;
                          facts.aoeThreat = action->getThreatType() == Action::ActionThreatType::Aoe;
                          facts.multiTarget = MULTI_TARGET_ACTIONS.count(action->getName()) != 0;
                          facts.summon = SummonSplashes(action);
                          facts.cast = dynamic_cast<CastSpellAction*>(action);
                          facts.heal = dynamic_cast<CastHealingSpellAction*>(action) != nullptr;
                          return facts;
                      });
}

bool SpellSplashes(PlayerbotAI* botAI, Action* action, RaidSplashCache& cache)
{
    if (!action)
        return false;

    RaidSplashCache::Facts& facts = cache.Of(action);
    if (facts.summon)
        return true;

    if (!facts.cast || facts.heal)
        return false;

    uint32 const spellId = facts.spellId.Get(botAI, facts.cast);
    SpellInfo const* spellInfo = spellId ? sSpellMgr->GetSpellInfo(spellId) : nullptr;
    return spellInfo && (spellInfo->IsTargetingArea() || spellInfo->IsAffectingArea());
}
