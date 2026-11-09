/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "DTKTriggers.h"
#include "Playerbots.h"
#include "ThreatHelpers.h"

bool CorpseExplodeTrigger::IsActive()
{
    if (!FindThreatTarget(bot, "trollgore")) { return false; }

    float distance = 6.0f;  // 5 unit radius, 1 unit added as buffer
    GuidVector corpses = AI_VALUE(GuidVector, "nearest corpses");
    for (auto i = corpses.begin(); i != corpses.end(); ++i)
    {
        Unit* unit = botAI->GetUnit(*i);
        if (unit && unit->GetEntry() == NPC_DRAKKARI_INVADER)
        {
            if (bot->GetExactDist2d(unit) < distance)
            {
                return true;
            }
        }
    }
    return false;
}

bool ArcaneFieldTrigger::IsActive()
{
    Unit* boss = FindThreatTarget(bot, "novos the summoner");
    return boss && boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_ARCANE_FIELD);
}

bool GiftOfTharonjaTrigger::IsActive()
{
    return bot->HasAura(SPELL_GIFT_OF_THARONJA);
}
