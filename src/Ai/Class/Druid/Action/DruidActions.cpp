/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "DruidActions.h"

#include "Event.h"
#include "Playerbots.h"
#include "ServerFacade.h"
#include "AoeValues.h"
#include "TargetValue.h"
#include <unordered_map>

// Eclipse phase tracking — keyed by bot GUID so multiple Balance Druids are independent.
// Static lifetime means state survives action-object recreation (bot death, strategy reset, etc.).
//
// sSolarProcTime[guid]: time when Solar Eclipse (48517) was first detected.
//   Set by CastWrathAction (which IS selected during Solar via the eclipse trigger).
//   Read by CastStarfireAction to stay off Starfire for 30 s post-Solar.
//
// sLunarProcTime[guid]: time when Lunar Eclipse (48518) was first detected.
//   Set by CastStarfireAction (which IS selected during Lunar via the eclipse trigger).
//   Read by CastWrathAction to stay off Wrath for 30 s post-Lunar.
//
// Each function also cross-updates the other map as a belt-and-suspenders measure.
static std::unordered_map<ObjectGuid, time_t> sSolarProcTime;
static std::unordered_map<ObjectGuid, time_t> sLunarProcTime;

namespace
{
    bool PrepareThornsTarget(PlayerbotAI* botAI, Unit* target)
    {
        if (!target)
            return false;

        Aura* existingThorns = botAI->GetAura("thorns", target, true);
        if (!existingThorns)
            return true;

        target->RemoveOwnedAura(existingThorns, AURA_REMOVE_BY_CANCEL);
        return true;
    }
}

std::vector<NextAction> CastAbolishPoisonAction::getAlternatives()
{
    return NextAction::merge({ NextAction("cure poison") },
                             CastSpellAction::getPrerequisites());
}

std::vector<NextAction> CastAbolishPoisonOnPartyAction::getAlternatives()
{
    return NextAction::merge({ NextAction("cure poison on party") },
                             CastSpellAction::getPrerequisites());
}

bool CastLifebloomOnMainTankAction::isUseful()
{
    Unit* target = GetTarget();
    if (!target || !target->IsAlive() || !CastSpellAction::isUseful())
        return false;

    Aura* lifebloom = botAI->GetAura("lifebloom", target, true, true);
    return !lifebloom || lifebloom->GetStackAmount() < 3 || lifebloom->GetDuration() < 2000;
}

bool CastWrathAction::isUseful()
{
    time_t now = time(nullptr);
    ObjectGuid guid = bot->GetGUID();
    time_t& solarTime = sSolarProcTime[guid];
    time_t& lunarTime = sLunarProcTime[guid];

    // --- Update Solar Eclipse tracking ---
    // Wrath is selected during Solar Eclipse (eclipse trigger at 20.0f), so we reliably see it here.
    if (bot->HasAura(48517) && !solarTime)
        solarTime = now;
    // Lunar procced — Solar fishing window is over, new cycle begins
    if (bot->HasAura(48518) && solarTime)
        solarTime = 0;
    // 30 s cooldown window expired
    if (solarTime && (now - solarTime) >= 30)
        solarTime = 0;

    // --- Update Lunar Eclipse tracking (belt-and-suspenders in case Starfire isn't evaluated) ---
    if (bot->HasAura(48518) && !lunarTime)
        lunarTime = now;
    // Solar procced — Lunar fishing window is over
    if (bot->HasAura(48517) && lunarTime)
        lunarTime = 0;
    if (lunarTime && (now - lunarTime) >= 30)
        lunarTime = 0;

    // Block Wrath while in Lunar Eclipse / post-Lunar fishing window
    if (lunarTime)
        return false;

    return CastSpellAction::isUseful();
}

bool CastStarfireAction::isUseful()
{
    time_t now = time(nullptr);
    ObjectGuid guid = bot->GetGUID();
    time_t& solarTime = sSolarProcTime[guid];
    time_t& lunarTime = sLunarProcTime[guid];

    // --- Update Lunar Eclipse tracking ---
    // Starfire is selected during Lunar Eclipse (eclipse trigger at 20.0f), so we reliably see it here.
    if (bot->HasAura(48518) && !lunarTime)
        lunarTime = now;
    // Solar procced — Lunar fishing window is over, new cycle begins
    if (bot->HasAura(48517) && lunarTime)
        lunarTime = 0;
    // 30 s cooldown window expired
    if (lunarTime && (now - lunarTime) >= 30)
        lunarTime = 0;

    // --- Update Solar Eclipse tracking (belt-and-suspenders in case Wrath isn't evaluated) ---
    if (bot->HasAura(48517) && !solarTime)
        solarTime = now;
    // Lunar procced — Solar fishing window is over
    if (bot->HasAura(48518) && solarTime)
        solarTime = 0;
    if (solarTime && (now - solarTime) >= 30)
        solarTime = 0;

    // Block Starfire while in Solar Eclipse / post-Solar fishing window
    if (solarTime)
        return false;

    return CastSpellAction::isUseful();
bool CastThornsAction::Execute(Event event)
{
    return PrepareThornsTarget(botAI, GetTarget()) && CastBuffSpellAction::Execute(event);
}

bool CastThornsOnPartyAction::Execute(Event event)
{
    return PrepareThornsTarget(botAI, GetTarget()) && BuffOnPartyAction::Execute(event);
}

bool CastThornsOnMainTankAction::Execute(Event event)
{
    return PrepareThornsTarget(botAI, GetTarget()) && BuffOnMainTankAction::Execute(event);
}

Value<Unit*>* CastEntanglingRootsCcAction::GetTargetValue()
{
    return context->GetValue<Unit*>("cc target", "entangling roots");
}

bool CastEntanglingRootsCcAction::Execute(Event /*event*/) { return botAI->CastSpell("entangling roots", GetTarget()); }

Value<Unit*>* CastHibernateCcAction::GetTargetValue() { return context->GetValue<Unit*>("cc target", "hibernate"); }

bool CastHibernateCcAction::Execute(Event /*event*/) { return botAI->CastSpell("hibernate", GetTarget()); }

Value<Unit*>* CastCycloneCcAction::GetTargetValue() { return context->GetValue<Unit*>("cc target", "cyclone"); }

bool CastCycloneCcAction::Execute(Event /*event*/) { return botAI->CastSpell("cyclone", GetTarget()); }
bool CastTyphoonAction::isUseful()
{
    bool facingTarget = AI_VALUE2(bool, "facing", "current target");
    bool targetClose  = ServerFacade::instance().IsDistanceLessOrEqualThan(
        AI_VALUE2(float, "distance", GetTargetName()), 15.f);
    return facingTarget && targetClose;
}

bool CastStarfallAction::isUseful()
{
    if (!CastSpellAction::isUseful())
        return false;

    // Avoid breaking CC
    WorldLocation aoePos = *context->GetValue<WorldLocation>("aoe position");
    Unit* ccTarget = context->GetValue<Unit*>("current cc target")->Get();
    if (ccTarget && ccTarget->IsAlive())
    {
        float dist2d = ServerFacade::instance().GetDistance2d(ccTarget, aoePos.GetPositionX(), aoePos.GetPositionY());
        if (ServerFacade::instance().IsDistanceLessOrEqualThan(dist2d, sPlayerbotAIConfig.aoeRadius))
            return false;
    }

    return true;
}

std::vector<NextAction> CastReviveAction::getPrerequisites()
{
    return NextAction::merge({ NextAction("caster form") },
                             ResurrectPartyMemberAction::getPrerequisites());
}

std::vector<NextAction> CastRebirthAction::getPrerequisites()
{
    return NextAction::merge({ NextAction("caster form") },
                             ResurrectPartyMemberAction::getPrerequisites());
}

bool CastRebirthAction::isUseful()
{
    return CastSpellAction::isUseful() &&
           AI_VALUE2(float, "distance", GetTargetName()) <= sPlayerbotAIConfig.spellDistance;
}

Unit* CastRejuvenationOnNotFullAction::GetTarget()
{
    Group* group = bot->GetGroup();
    MinValueCalculator calc(100);
    for (GroupReference* gref = group->GetFirstMember(); gref; gref = gref->next())
    {
        Player* player = gref->GetSource();
        if (!player)
            continue;
        if (player->isDead() || player->IsFullHealth())
        {
            continue;
        }
        if (player->GetDistance2d(bot) > sPlayerbotAIConfig.spellDistance)
        {
            continue;
        }
        if (botAI->HasAura("rejuvenation", player))
        {
            continue;
        }
        calc.probe(player->GetHealthPct(), player);
    }
    return (Unit*)calc.param;
}

bool CastRejuvenationOnNotFullAction::isUseful()
{
    return GetTarget();
}
