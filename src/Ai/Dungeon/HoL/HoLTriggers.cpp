/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "HoLTriggers.h"
#include "Playerbots.h"
#include "ThreatHelpers.h"

bool StormforgedLieutenantTrigger::IsActive()
{
    if (!PlayerbotAI::IsDps(bot)) { return false; }

    GuidVector targets = AI_VALUE(GuidVector, "possible targets");

    for (auto& target : targets)
    {
        Unit* unit = botAI->GetUnit(target);
        if (unit && unit->IsInCombat() &&
            unit->GetEntry() == NPC_STORMFORGED_LIEUTENANT)
        {
            return true;
        }
    }
    return false;
}

bool BjarngrimWhirlwindTrigger::IsActive()
{
    Unit* boss = FindThreatTarget(bot, "general bjarngrim");
    if (!boss) { return false; }

    return boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_WHIRLWIND_BJARNGRIM);
}

bool VolkhanTrigger::IsActive()
{
    return FindThreatTarget(bot, "volkhan") && PlayerbotAI::IsDps(bot);
}

bool IonarStaticOverloadTrigger::IsActive()
{
    GuidVector members = AI_VALUE(GuidVector, "group members");
    for (auto& member : members)
    {
        Unit* unit = botAI->GetUnit(member);
        if (unit && unit->HasAura(SPELL_STATIC_OVERLOAD))
        {
            return true;
        }
    }
    return false;
}

bool IonarBallLightningTrigger::IsActive()
{
    if (PlayerbotAI::IsMelee(bot)) { return false; }

    Unit* boss = FindThreatTarget(bot, "ionar");
    if (!boss) { return false; }

    return boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_BALL_LIGHTNING);
}

bool IonarTankAggroTrigger::IsActive()
{
    if (!PlayerbotAI::IsTank(bot)) { return false; }

    if (!FindThreatTarget(bot, "ionar")) { return false; }

    return AI_VALUE2(bool, "has aggro", "current target");
}

bool IonarDisperseTrigger::IsActive()
{
    Unit* boss = FindThreatTarget(bot, "ionar");
    if (!boss) { return false; }

    return !bot->CanSeeOrDetect(boss) || boss->FindCurrentSpellBySpellId(SPELL_DISPERSE);
}

bool LokenRangedTrigger::IsActive()
{
    return !PlayerbotAI::IsMelee(bot) && FindThreatTarget(bot, "loken");
}

bool LokenLightningNovaTrigger::IsActive()
{
    Unit* boss = FindThreatTarget(bot, "loken");
    if (!boss) { return false; }

    return boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_LIGHTNING_NOVA);
}
