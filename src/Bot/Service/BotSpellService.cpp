/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "BotSpellService.h"

#include "PlayerbotAI.h"

bool BotSpellService::CanCastSpell(std::string const& name, Unit* target, Item* itemTarget)
{
    if (botAI_)
    {
        return botAI_->CanCastSpell(name, target, itemTarget);
    }
    return false;
}

bool BotSpellService::CastSpell(std::string const& name, Unit* target, Item* itemTarget)
{
    if (botAI_)
    {
        return botAI_->CastSpell(name, target, itemTarget);
    }
    return false;
}

bool BotSpellService::CanCastSpell(uint32 spellId, Unit* target, bool checkHasSpell, Item* itemTarget, Item* castItem)
{
    if (botAI_)
    {
        return botAI_->CanCastSpell(spellId, target, checkHasSpell, itemTarget, castItem);
    }
    return false;
}

bool BotSpellService::CanCastSpell(uint32 spellId, GameObject* goTarget, bool checkHasSpell)
{
    if (botAI_)
    {
        return botAI_->CanCastSpell(spellId, goTarget, checkHasSpell);
    }
    return false;
}

bool BotSpellService::CanCastSpell(uint32 spellId, float x, float y, float z, bool checkHasSpell, Item* itemTarget)
{
    if (botAI_)
    {
        return botAI_->CanCastSpell(spellId, x, y, z, checkHasSpell, itemTarget);
    }
    return false;
}

bool BotSpellService::CastSpell(uint32 spellId, Unit* target, Item* itemTarget)
{
    if (botAI_)
    {
        return botAI_->CastSpell(spellId, target, itemTarget);
    }
    return false;
}

bool BotSpellService::CastSpell(uint32 spellId, float x, float y, float z, Item* itemTarget)
{
    if (botAI_)
    {
        return botAI_->CastSpell(spellId, x, y, z, itemTarget);
    }
    return false;
}

bool BotSpellService::HasAura(std::string const& spellName, Unit* player, bool maxStack, bool checkIsOwner,
                              int maxAmount, bool checkDuration)
{
    if (botAI_)
    {
        return botAI_->HasAura(spellName, player, maxStack, checkIsOwner, maxAmount, checkDuration);
    }
    return false;
}

bool BotSpellService::HasAura(uint32 spellId, Unit const* player)
{
    if (botAI_)
    {
        return botAI_->HasAura(spellId, player);
    }
    return false;
}

bool BotSpellService::HasAnyAuraOf(Unit* player, ...)
{
    // Variadic delegation is complex - for now return false
    // This will need proper implementation when migrating callers
    return false;
}

Aura* BotSpellService::GetAura(std::string const& spellName, Unit* unit, bool checkIsOwner, bool checkDuration,
                               int checkStack)
{
    if (botAI_)
    {
        return botAI_->GetAura(spellName, unit, checkIsOwner, checkDuration, checkStack);
    }
    return nullptr;
}

void BotSpellService::RemoveAura(std::string const& name)
{
    if (botAI_)
    {
        botAI_->RemoveAura(name);
    }
}

void BotSpellService::RemoveShapeshift()
{
    if (botAI_)
    {
        botAI_->RemoveShapeshift();
    }
}

bool BotSpellService::HasAuraToDispel(Unit* player, uint32 dispelType)
{
    if (botAI_)
    {
        return botAI_->HasAuraToDispel(player, dispelType);
    }
    return false;
}

bool BotSpellService::CanDispel(SpellInfo const* spellInfo, uint32 dispelType)
{
    if (botAI_)
    {
        return botAI_->canDispel(spellInfo, dispelType);
    }
    return false;
}

bool BotSpellService::IsInterruptableSpellCasting(Unit* player, std::string const& spell)
{
    if (botAI_)
    {
        return botAI_->IsInterruptableSpellCasting(player, spell);
    }
    return false;
}

void BotSpellService::InterruptSpell()
{
    if (botAI_)
    {
        botAI_->InterruptSpell();
    }
}

void BotSpellService::SpellInterrupted(uint32 spellId)
{
    if (botAI_)
    {
        botAI_->SpellInterrupted(spellId);
    }
}

int32 BotSpellService::CalculateGlobalCooldown(uint32 spellId)
{
    if (botAI_)
    {
        return botAI_->CalculateGlobalCooldown(spellId);
    }
    return 0;
}

void BotSpellService::WaitForSpellCast(Spell* spell)
{
    if (botAI_)
    {
        botAI_->WaitForSpellCast(spell);
    }
}

bool BotSpellService::CanCastVehicleSpell(uint32 spellId, Unit* target)
{
    if (botAI_)
    {
        return botAI_->CanCastVehicleSpell(spellId, target);
    }
    return false;
}

bool BotSpellService::CastVehicleSpell(uint32 spellId, Unit* target)
{
    if (botAI_)
    {
        return botAI_->CastVehicleSpell(spellId, target);
    }
    return false;
}

bool BotSpellService::CastVehicleSpell(uint32 spellId, float x, float y, float z)
{
    if (botAI_)
    {
        return botAI_->CastVehicleSpell(spellId, x, y, z);
    }
    return false;
}

bool BotSpellService::IsInVehicle(bool canControl, bool canCast, bool canAttack, bool canTurn, bool fixed)
{
    if (botAI_)
    {
        return botAI_->IsInVehicle(canControl, canCast, canAttack, canTurn, fixed);
    }
    return false;
}
