/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "PaladinTriggers.h"
#include "GenericBuffUtils.h"
#include "PaladinHelper.h"
#include "Playerbots.h"

HolyShieldTrigger::HolyShieldTrigger(PlayerbotAI* botAI)
    : BuffTrigger(botAI, "holy shield", 1, false, false, ai::paladin::HOLY_SHIELD_REFRESH_MS)
{
}

bool SealTrigger::IsActive()
{
    Unit* target = GetTarget();
    return !botAI->HasAura("seal of justice", target) && !botAI->HasAura("seal of command", target) &&
           !botAI->HasAura("seal of vengeance", target) && !botAI->HasAura("seal of corruption", target) &&
           !botAI->HasAura("seal of righteousness", target) && !botAI->HasAura("seal of light", target) &&
           (!botAI->HasAura("seal of wisdom", target) || AI_VALUE2(uint8, "mana", "self target") > 70);
}

bool CrusaderAuraTrigger::IsActive()
{
    Unit* target = GetTarget();
    return AI_VALUE2(bool, "mounted", "self target") && !botAI->HasAura("crusader aura", target);
}

bool DivineShieldLowHealthTrigger::IsActive()
{
    return botAI->HasAura("divine shield", bot) && AI_VALUE2(uint8, "health", "self target") < 80;
}

bool HolyWrathUndeadAoeTrigger::IsActive()
{
    if (!bot->IsInCombat())
        return false;
    // Fast reject: not enough enemies around at all.
    if (AI_VALUE(uint8, "aoe count") < 3)
        return false;

    uint8 undeadOrDemonInRadius = 0;
    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    for (auto& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive())
            continue;

        Creature* creature = unit->ToCreature();
        if (!creature)
            continue;

        CreatureTemplate const* proto = creature->GetCreatureTemplate();
        if (!proto)
            continue;

        if (proto->type != CREATURE_TYPE_UNDEAD && proto->type != CREATURE_TYPE_DEMON)
            continue;

        // Holy Wrath (48817) is a 15 yard self-centered AoE.
        if (unit->GetExactDist2d(bot) <= 15.0f)
            ++undeadOrDemonInRadius;
    }

    return undeadOrDemonInRadius >= 3;
}

Unit* HandOfFreedomOnPartyTrigger::GetTarget()
{
    bool const selfImpaired = botAI->IsMovementImpaired(bot);
    bool const hasSelfHand = selfImpaired && ai::paladin::HasAnyPaladinHandFromCaster(bot, bot);

    if (!bot->GetGroup())
    {
        if (selfImpaired && !hasSelfHand)
            return bot;

        return nullptr;
    }

    if (selfImpaired && !hasSelfHand)
        return bot;

    return Trigger::GetTarget();
}

bool HandOfFreedomOnPartyTrigger::IsActive()
{
    Unit* target = GetTarget();
    if (!target)
        return false;

    if (target != bot &&
        bot->GetExactDist2dSq(target->GetPositionX(), target->GetPositionY()) > 30.0f * 30.0f)
        return false;

    if (!botAI->CanCastSpell("hand of freedom", target))
        return false;

    return !ai::paladin::HasAnyPaladinHandFromCaster(target, bot) && botAI->IsMovementImpaired(target);
}

bool NotSensingUndeadTrigger::IsActive()
{
    return !botAI->HasAura("sense undead", bot);
}

bool HolyShieldSuddenDropTrigger::IsActive()
{
    uint8 const health = AI_VALUE2(uint8, "health", "self target");

    // Sudden collapse: 60% or more of max health gone since the last check.
    bool const suddenDrop = lastHealthPct > health && lastHealthPct - health >= 60;
    lastHealthPct = health;

    if (!suddenDrop)
        return false;

    // Only holy paladins benefit: they keep healing through Divine Shield.
    // Prot / ret paladins lose their attacks while immune and the tank guard
    // inside IsHeal covers protection anyway.
    return PlayerbotAI::IsHeal(bot);
}
