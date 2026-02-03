/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "DruidActions.h"

#include "DruidShapeshiftActions.h"
#include "Event.h"
#include "Playerbots.h"
#include "ServerFacade.h"
#include "TargetValue.h"

std::vector<NextAction> CastAbolishPoisonAction::getAlternatives()
{
    std::vector<NextAction> alternatives = CastSpellAction::getAlternatives();

    alternatives.push_back(CreateNextAction<CastCurePoisonAction>(1.0f));

    return alternatives;
}

std::vector<NextAction> CastAbolishPoisonOnPartyAction::getAlternatives()
{
    std::vector<NextAction> alternatives = CastSpellAction::getAlternatives();

    alternatives.push_back(CreateNextAction<CastCurePoisonOnPartyAction>(1.0f));

    return alternatives;
}

Value<Unit*>* CastEntanglingRootsCcAction::GetTargetValue()
{
    return context->GetValue<Unit*>("cc target", "entangling roots");
}

bool CastEntanglingRootsCcAction::Execute(Event) { return botAI->CastSpell("entangling roots", GetTarget()); }

Value<Unit*>* CastHibernateCcAction::GetTargetValue() { return context->GetValue<Unit*>("cc target", "hibernate"); }

bool CastHibernateCcAction::Execute(Event) { return botAI->CastSpell("hibernate", GetTarget()); }
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

    // Avoid single-target usage on initial pull
    uint8 aoeCount = *context->GetValue<uint8>("aoe count");
    if (aoeCount < 2)
    {
        Unit* target = context->GetValue<Unit*>("current target")->Get();
        if (!target || (!botAI->HasAura("moonfire", target) && !botAI->HasAura("insect swarm", target)))
            return false;
    }

    return true;
}

std::vector<NextAction> CastReviveAction::getPrerequisites()
{
    std::vector<NextAction> prerequisites = ResurrectPartyMemberAction::getPrerequisites();

    prerequisites.push_back(CreateNextAction<CastCasterFormAction>(1.0f));

    return prerequisites;
}

std::vector<NextAction> CastRebirthAction::getPrerequisites()
{
    std::vector<NextAction> prerequisites = ResurrectPartyMemberAction::getPrerequisites();

    prerequisites.push_back(CreateNextAction<CastCasterFormAction>(1.0f));

    return prerequisites;
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
