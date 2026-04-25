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
    // Lunar Eclipse appeared — start blocking wrath so starfire can fish for Solar
    if (bot->HasAura(48518) && _lunarProcTime == 0)
        _lunarProcTime = now;
    // Solar Eclipse procced — fishing succeeded, no need to keep blocking wrath
    if (bot->HasAura(48517) && _lunarProcTime != 0)
        _lunarProcTime = 0;
    // Full 30s elapsed — cooldown window over
    if (_lunarProcTime && (now - _lunarProcTime) >= 30)
        _lunarProcTime = 0;
    if (_lunarProcTime)
        return false;
    return CastSpellAction::isUseful();
}

bool CastStarfireAction::isUseful()
{
    time_t now = time(nullptr);
    // Solar Eclipse appeared — start blocking starfire so wrath can fish for Lunar
    if (bot->HasAura(48517) && _solarProcTime == 0)
        _solarProcTime = now;
    // Lunar Eclipse procced — fishing succeeded, no need to keep blocking starfire
    if (bot->HasAura(48518) && _solarProcTime != 0)
        _solarProcTime = 0;
    // Full 30s elapsed — cooldown window over
    if (_solarProcTime && (now - _solarProcTime) >= 30)
        _solarProcTime = 0;
    if (_solarProcTime)
        return false;
    return CastSpellAction::isUseful();
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
