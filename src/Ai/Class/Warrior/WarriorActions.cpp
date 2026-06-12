/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "WarriorActions.h"

#include "AiFactory.h"
#include "Playerbots.h"

namespace
{
constexpr uint32 SPELL_RETALIATION = 20230;
constexpr uint32 SPELL_DIVINE_SHIELD = 642;
constexpr uint32 SPELL_ICE_BLOCK = 45438;
constexpr uint32 SPELL_BLESSING_OF_PROTECTION = 41450;
constexpr uint32 SPELL_SHATTERING_THROW = 64382;
}

bool CastBerserkerRageAction::isPossible()
{
    if (botAI->IsInVehicle() && !botAI->IsInVehicle(false, false, true))
        return false;

    uint32 spellId = AI_VALUE2(uint32, "spell id", spell);
    if (!spellId)
        return false;

    if (!bot->HasSpell(spellId))
        return false;

    if (bot->HasSpellCooldown(spellId))
        return false;

    return true;
}

bool CastBerserkerRageAction::isUseful()
{
    return (bot->HasAuraType(SPELL_AURA_MOD_FEAR) ||
           bot->HasAuraWithMechanic(1 << MECHANIC_SLEEP) ||
           bot->HasAuraWithMechanic(1 << MECHANIC_SAPPED))
        && !botAI->HasAura("berserker rage", bot)
        && CastSpellAction::isUseful();
}

bool CastSunderArmorAction::isUseful()
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    if (!botAI->IsTank(bot, false))
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || member == bot || !member->IsAlive() || !member->IsInWorld() ||
                member->GetMapId() != bot->GetMapId())
            {
                continue;
            }

            if (member->getClass() == CLASS_WARRIOR &&
                botAI->IsTank(member, false))
                return false;
        }
    }

    Aura* aura = botAI->GetAura("sunder armor", GetTarget(), false, true);
    return !aura || aura->GetStackAmount() < 5 || aura->GetDuration() <= 6000;
}

Unit* CastVigilanceAction::GetTarget()
{
    Group* group = bot->GetGroup();
    if (!group)
    {
        return nullptr;
    }

    Player* currentVigilanceTarget = nullptr;
    Player* mainTank = nullptr;
    Player* assistTank1 = nullptr;
    Player* assistTank2 = nullptr;
    Player* highestGearScorePlayer = nullptr;
    uint32 highestGearScore = 0;

    // Iterate once through the group to gather all necessary information
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member == bot || !member->IsAlive())
            continue;

        // Check if member has Vigilance applied by the bot
        if (!currentVigilanceTarget && botAI->HasAura("vigilance", member, false, true))
        {
            currentVigilanceTarget = member;
        }

        // Identify Main Tank
        if (!mainTank && botAI->IsMainTank(member))
        {
            mainTank = member;
        }

        // Identify Assist Tanks
        if (assistTank1 == nullptr && botAI->IsAssistTankOfIndex(member, 0))
        {
            assistTank1 = member;
        }
        else if (assistTank2 == nullptr && botAI->IsAssistTankOfIndex(member, 1))
        {
            assistTank2 = member;
        }

        // Determine Highest Gear Score
        uint32 gearScore = botAI->GetEquipGearScore(member/*, false, false*/);
        if (gearScore > highestGearScore)
        {
            highestGearScore = gearScore;
            highestGearScorePlayer = member;
        }
    }

    // Determine the highest-priority target
    Player* highestPriorityTarget = mainTank ? mainTank :
                                      (assistTank1 ? assistTank1 :
                                      (assistTank2 ? assistTank2 : highestGearScorePlayer));

    // If no valid target, return nullptr
    if (!highestPriorityTarget)
    {
        return nullptr;
    }

    // If the current target is already the highest-priority target, do nothing
    if (currentVigilanceTarget == highestPriorityTarget)
    {
        return nullptr;
    }

    // Assign the new target
    Unit* targetUnit = highestPriorityTarget->ToUnit();
    if (targetUnit)
    {
        return targetUnit;
    }

    return nullptr;
}

bool CastVigilanceAction::Execute(Event /*event*/)
{
    Unit* target = GetTarget();
    if (!target || target == bot)
        return false;

    return botAI->CastSpell("vigilance", target);
}

bool CastRetaliationAction::isUseful()
{
    if (!bot->HasSpell(SPELL_RETALIATION) || bot->HasSpellCooldown(SPELL_RETALIATION) ||
        bot->HasAura(SPELL_RETALIATION))
    {
        return false;
    }

    uint8 meleeAttackers = 0;
    GuidVector attackers = AI_VALUE(GuidVector, "attackers");

    for (ObjectGuid const& guid : attackers)
    {
        Unit* attacker = botAI->GetUnit(guid);
        if (!attacker || !attacker->IsAlive() || attacker->GetVictim() != bot)
            continue;

        // Check if the attacker is melee-based using unit_class
        if (attacker->IsCreature())
        {
            Creature* creature = attacker->ToCreature();
            if (creature && (creature->IsClass(CLASS_WARRIOR)
                || creature->IsClass(CLASS_ROGUE)
                || creature->IsClass(CLASS_PALADIN)))
            {
                ++meleeAttackers;
            }
        }
        else if (attacker->IsPlayer())
        {
            Player* playerAttacker = attacker->ToPlayer();
            if (playerAttacker && botAI->IsMelee(playerAttacker)) // Reuse existing Player melee check
            {
                ++meleeAttackers;
            }
        }

        // Early exit if we already have enough melee attackers
        if (meleeAttackers >= 2)
            break;
    }

    // Only cast Retaliation if there are at least 2 melee attackers
    return meleeAttackers >= 2;
}

Unit* CastShatteringThrowAction::GetTarget()
{
    GuidVector enemies = AI_VALUE(GuidVector, "possible targets");

    for (ObjectGuid const& guid : enemies)
    {
        Unit* enemy = botAI->GetUnit(guid);
        if (!enemy || !enemy->IsAlive() || enemy->IsFriendlyTo(bot))
            continue;

        if (bot->IsWithinDistInMap(enemy, 25.0f) &&
            (enemy->HasAura(SPELL_DIVINE_SHIELD) ||
             enemy->HasAura(SPELL_ICE_BLOCK) ||
             enemy->HasAura(SPELL_BLESSING_OF_PROTECTION)))
        {
            return enemy;
        }
    }

    return nullptr;
}

bool CastShatteringThrowAction::Execute(Event /*event*/)
{
    Unit* target = GetTarget();
    if (!target)
        return false;

    return botAI->CastSpell("shattering throw", target);
}

bool CastShatteringThrowAction::isUseful()
{
    if (!bot->HasSpell(SPELL_SHATTERING_THROW) || bot->HasSpellCooldown(SPELL_SHATTERING_THROW))
        return false;

    GuidVector enemies = AI_VALUE(GuidVector, "possible targets");

    for (ObjectGuid const& guid : enemies)
    {
        Unit* enemy = botAI->GetUnit(guid);
        if (!enemy || !enemy->IsAlive() || enemy->IsFriendlyTo(bot))
            continue;

        if (bot->IsWithinDistInMap(enemy, 25.0f) &&
            (enemy->HasAura(SPELL_DIVINE_SHIELD) ||
             enemy->HasAura(SPELL_ICE_BLOCK) ||
             enemy->HasAura(SPELL_BLESSING_OF_PROTECTION)))
        {
            return true;
        }
    }

    return false;
}

bool CastShatteringThrowAction::isPossible()
{
    Unit* target = GetTarget();
    if (!target)
        return false;

    // Range check: Shattering Throw is 30 yards
    if (!bot->IsWithinDistInMap(target, 30.0f))
        return false;

    // Check line of sight
    if (!bot->IsWithinLOSInMap(target))
        return false;

    // If the minimal checks above pass, simply return true.
    return true;
}
