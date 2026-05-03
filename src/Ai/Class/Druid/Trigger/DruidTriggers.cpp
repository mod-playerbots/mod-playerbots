/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "DruidTriggers.h"
#include "Creature.h"
#include "DynamicObject.h"
#include "Player.h"
#include "Playerbots.h"

bool FaerieFireTrigger::IsActive()
{
    if (!BuffTrigger::IsActive())
        return false;

    Unit* target = GetTarget();
    if (!target)
        return false;

    Creature* creature = target->ToCreature();
    return creature && creature->GetCreatureTemplate()->rank == CREATURE_ELITE_WORLDBOSS;
}

bool MarkOfTheWildOnPartyTrigger::IsActive()
{
    return BuffOnPartyTrigger::IsActive() && !botAI->HasAura("gift of the wild", GetTarget());
}

bool MarkOfTheWildTrigger::IsActive()
{
    return BuffTrigger::IsActive() && !botAI->HasAura("gift of the wild", GetTarget());
}

bool ThornsOnPartyTrigger::IsActive()
{
    return BuffOnPartyTrigger::IsActive() && !botAI->HasAura("thorns", GetTarget());
}

bool EntanglingRootsKiteTrigger::IsActive()
{
    return DebuffTrigger::IsActive() && AI_VALUE(uint8, "attacker count") < 3 && !GetTarget()->GetPower(POWER_MANA);
}

bool ThornsTrigger::IsActive() { return BuffTrigger::IsActive() && !botAI->HasAura("thorns", GetTarget()); }

bool BearFormTrigger::IsActive() { return !botAI->HasAnyAuraOf(bot, "bear form", "dire bear form", nullptr); }

bool TreeFormTrigger::IsActive() { return !botAI->HasAura(33891, bot); }

bool CatFormTrigger::IsActive() { return !botAI->HasAura("cat form", bot); }

const std::set<uint32> HurricaneChannelCheckTrigger::HURRICANE_SPELL_IDS = {
    16914,  // Hurricane Rank 1
    17401,  // Hurricane Rank 2
    17402,  // Hurricane Rank 3
    27012,  // Hurricane Rank 4
    48467   // Hurricane Rank 5
};

bool HurricaneChannelCheckTrigger::IsActive()
{
    Player* bot = botAI->GetBot();

    if (Spell* spell = bot->GetCurrentSpell(CURRENT_CHANNELED_SPELL))
    {
        if (!HURRICANE_SPELL_IDS.count(spell->m_spellInfo->Id))
            return false;

        // Find this bot's own Hurricane DynamicObject
        DynamicObject* dynObj = nullptr;
        for (uint32 spellId : HURRICANE_SPELL_IDS)
        {
            dynObj = bot->GetDynObject(spellId);
            if (dynObj)
                break;
        }

        if (!dynObj)
            return false;

        // Count attackers actually inside the Hurricane AoE
        float radius = dynObj->GetRadius();
        GuidVector attackers = AI_VALUE(GuidVector, "attackers");
        uint32 count = 0;
        for (ObjectGuid const& guid : attackers)
        {
            Unit* unit = botAI->GetUnit(guid);
            if (!unit || !unit->IsAlive())
                continue;
            if (unit->GetDistance(dynObj->GetPosition()) <= radius)
                count++;
        }

        return count < minEnemies;
    }

    return false;
}
