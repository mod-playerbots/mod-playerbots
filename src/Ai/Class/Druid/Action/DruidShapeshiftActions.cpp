/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "DruidShapeshiftActions.h"

#include "Playerbots.h"

bool CastBearFormAction::isUseful()
{
    return CastBuffSpellAction::isUseful() && !botAI->HasAura("dire bear form", GetTarget());
}

bool CastBearFormAction::isPossible()
{
    return CastBuffSpellAction::isPossible() && !botAI->HasAura("dire bear form", GetTarget());
}

std::vector<NextAction> CastDireBearFormAction::getAlternatives()
{
    return NextAction::merge({NextAction("bear form")}, CastSpellAction::getAlternatives());
}

bool CastTravelFormAction::isUseful()
{
    bool firstmount = bot->GetLevel() >= 20;

    // useful if no mount or with wsg flag
    return !bot->IsMounted() && (!firstmount || (bot->HasAura(23333) || bot->HasAura(23335) || bot->HasAura(34976))) &&
           !botAI->HasAura("dash", bot);
}

bool CastCasterFormAction::Execute(Event /*event*/)
{
    botAI->RemoveShapeshift();
    return true;
}

bool CastCasterFormAction::isUseful()
{
    return botAI->HasAnyAuraOf(GetTarget(), "dire bear form", "bear form", "cat form", "travel form", "aquatic form",
                               "flight form", "swift flight form", "moonkin form", nullptr) &&
           AI_VALUE2(uint8, "mana", "self target") > sPlayerbotAIConfig.mediumHealth;
}

bool CastCancelTreeFormAction::Execute(Event /*event*/)
{
    botAI->RemoveAura("tree of life");
    return true;
}

bool CastCancelTreeFormAction::isUseful() { return botAI->HasAura(33891, bot); }

bool CastTreeFormAction::isUseful()
{
    return GetTarget() && CastSpellAction::isUseful() && !botAI->HasAura(33891, bot);
}
