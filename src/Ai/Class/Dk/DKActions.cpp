/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "DKActions.h"

#include "Duration.h"
#include "GenericSpellActions.h"
#include "Playerbots.h"
#include "SpellInfo.h"
#include "SpellMgr.h"

std::vector<NextAction> CastDeathchillAction::getPrerequisites()
{
    return NextAction::merge({ NextAction("frost presence") },
                             CastSpellAction::getPrerequisites());
}

std::vector<NextAction> CastUnholyMeleeSpellAction::getPrerequisites()
{
    return NextAction::merge({ NextAction("unholy presence") },
                             CastMeleeSpellAction::getPrerequisites());
}

std::vector<NextAction> CastFrostMeleeSpellAction::getPrerequisites()
{
    return NextAction::merge({ NextAction("frost presence") },
                             CastMeleeSpellAction::getPrerequisites());
}

std::vector<NextAction> CastBloodMeleeSpellAction::getPrerequisites()
{
    return NextAction::merge({ NextAction("blood presence") },
                             CastMeleeSpellAction::getPrerequisites());
}

bool CastRaiseDeadAction::Execute(Event event)
{
    const bool result = CastBuffSpellAction::Execute(event);

    if (!result)
        return false;

    const uint32_t spellId = AI_VALUE2(uint32_t, "spell id", spell);

    bot->AddSpellCooldown(spellId, 0, 3 * 60 * 1000);

    return true;
}

Unit* CastHysteriaAction::GetTarget()
{
    Group* group = bot->GetGroup();
    if (!group)
    {
        if (!bot->HasAura(49016))
            return bot;
        return nullptr;
    }

    Unit* rangedDps = nullptr;
    Unit* tank = nullptr;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive())
            continue;

        if (member->GetMap() != bot->GetMap() || bot->GetDistance(member) > sPlayerbotAIConfig.spellDistance)
            continue;

        // Skip if already has hysteria
        if (member->HasAura(49016))
            continue;

        // Priority 1: Melee DPS
        if (botAI->IsMelee(member) && botAI->IsDps(member))
            return member;

        // Priority 2: Ranged DPS (physical, not casters)
        if (!rangedDps && botAI->IsRanged(member) && botAI->IsDps(member) && !botAI->IsCaster(member))
            rangedDps = member;

        // Priority 3: Tank
        if (!tank && botAI->IsTank(member))
            tank = member;
    }

    if (rangedDps)
        return rangedDps;

    if (tank)
        return tank;

    // Fallback to self if no hysteria
    if (!bot->HasAura(49016))
        return bot;

    return nullptr;
}
