/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "BotSpellService.h"

#include "DBCStores.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "PlayerbotAIConfig.h"
#include "SpellAuraEffects.h"
#include "SpellAuras.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include "Unit.h"
#include "Util.h"
#include "Vehicle.h"
#include "VehicleDefines.h"

#ifndef WIN32
inline int strcmpi(char const* s1, char const* s2)
{
    for (; *s1 && *s2 && (toupper(*s1) == toupper(*s2)); ++s1, ++s2)
    {
    }
    return *s1 - *s2;
}
#endif

// ============================================================================
// Static method implementations (main logic)
// ============================================================================

bool BotSpellService::IsRealAuraStatic(Player* bot, AuraEffect const* aurEff, Unit const* unit)
{
    if (!unit || !unit->IsInWorld() || unit->IsDuringRemoveFromWorld())
        return false;

    if (!aurEff)
        return false;

    if (!unit->IsHostileTo(bot))
        return true;

    SpellInfo const* spellInfo = aurEff->GetSpellInfo();
    if (!spellInfo)
        return false;

    uint32 stacks = aurEff->GetBase()->GetStackAmount();
    if (stacks >= spellInfo->StackAmount)
        return true;

    if (aurEff->GetCaster() == bot || spellInfo->IsPositive() ||
        spellInfo->Effects[aurEff->GetEffIndex()].IsAreaAuraEffect())
        return true;

    return false;
}

bool BotSpellService::HasAuraByNameStatic(Player* bot, std::string const& name, Unit* unit, bool maxStack,
                                          bool checkIsOwner, int maxAuraAmount, bool checkDuration)
{
    if (!unit || !unit->IsInWorld() || unit->IsDuringRemoveFromWorld())
        return false;

    std::wstring wnamepart;
    if (!Utf8toWStr(name, wnamepart))
        return false;

    wstrToLower(wnamepart);

    int auraAmount = 0;

    for (uint32 auraType = SPELL_AURA_BIND_SIGHT; auraType < TOTAL_AURAS; auraType++)
    {
        Unit::AuraEffectList const& auras = unit->GetAuraEffectsByType((AuraType)auraType);
        if (auras.empty())
            continue;

        for (AuraEffect const* aurEff : auras)
        {
            if (!aurEff)
                continue;

            SpellInfo const* spellInfo = aurEff->GetSpellInfo();
            if (!spellInfo)
                continue;

            std::string_view const auraName = spellInfo->SpellName[0];
            if (auraName.empty() || auraName.length() != wnamepart.length() || !Utf8FitTo(auraName, wnamepart))
                continue;

            if (IsRealAuraStatic(bot, aurEff, unit))
            {
                if (checkIsOwner && aurEff->GetCasterGUID() != bot->GetGUID())
                    continue;

                if (checkDuration && aurEff->GetBase()->GetDuration() == -1)
                    continue;

                uint32 maxStackAmount = spellInfo->StackAmount;
                uint32 maxProcCharges = spellInfo->ProcCharges;

                if (maxStack)
                {
                    if (maxStackAmount && aurEff->GetBase()->GetStackAmount() >= maxStackAmount)
                        auraAmount++;

                    if (maxProcCharges && aurEff->GetBase()->GetCharges() >= maxProcCharges)
                        auraAmount++;
                }
                else
                {
                    auraAmount++;
                }

                if (maxAuraAmount < 0 && auraAmount > 0)
                    return true;
            }
        }
    }

    if (maxAuraAmount >= 0)
    {
        return auraAmount == maxAuraAmount || (auraAmount > 0 && auraAmount <= maxAuraAmount);
    }

    return false;
}

bool BotSpellService::HasAuraByIdStatic(uint32 spellId, Unit const* unit)
{
    if (!spellId || !unit)
        return false;

    return unit->HasAura(spellId);
}

bool BotSpellService::HasAnyAuraOfStatic(Player* bot, Unit* player, std::vector<std::string> const& auraNames)
{
    if (!player)
        return false;

    for (auto const& name : auraNames)
    {
        if (HasAuraByNameStatic(bot, name, player))
            return true;
    }

    return false;
}

Aura* BotSpellService::GetAuraStatic(Player* bot, std::string const& name, Unit* unit, bool checkIsOwner,
                                      bool checkDuration, int checkStack)
{
    if (!unit || !unit->IsInWorld() || unit->IsDuringRemoveFromWorld())
        return nullptr;

    std::wstring wnamepart;
    if (!Utf8toWStr(name, wnamepart))
        return nullptr;

    wstrToLower(wnamepart);

    for (uint32 auraType = SPELL_AURA_BIND_SIGHT; auraType < TOTAL_AURAS; ++auraType)
    {
        Unit::AuraEffectList const& auras = unit->GetAuraEffectsByType((AuraType)auraType);
        if (auras.empty())
            continue;

        for (AuraEffect const* aurEff : auras)
        {
            SpellInfo const* spellInfo = aurEff->GetSpellInfo();
            if (!spellInfo)
                continue;

            std::string const& auraName = spellInfo->SpellName[0];

            if (auraName.empty() || auraName.length() != wnamepart.length() || !Utf8FitTo(auraName, wnamepart))
                continue;

            if (!IsRealAuraStatic(bot, aurEff, unit))
                continue;

            if (checkIsOwner && aurEff->GetCasterGUID() != bot->GetGUID())
                continue;

            if (checkDuration && aurEff->GetBase()->GetDuration() == -1)
                continue;

            if (checkStack != -1 && aurEff->GetBase()->GetStackAmount() < checkStack)
                continue;

            return aurEff->GetBase();
        }
    }

    return nullptr;
}

bool BotSpellService::HasAuraToDispelStatic(Player* bot, Unit* target, uint32 dispelType)
{
    if (!target || !target->IsAlive() || !target->IsInWorld() || target->IsDuringRemoveFromWorld())
        return false;

    if (!bot || !bot->IsAlive())
        return false;

    bool isFriend = bot->IsFriendlyTo(target);

    Unit::VisibleAuraMap const* visibleAuras = target->GetVisibleAuras();
    if (!visibleAuras)
        return false;

    for (Unit::VisibleAuraMap::const_iterator itr = visibleAuras->begin(); itr != visibleAuras->end(); ++itr)
    {
        if (!itr->second)
            continue;

        Aura* aura = itr->second->GetBase();
        if (!aura || aura->IsPassive() || aura->IsRemoved())
            continue;

        if (sPlayerbotAIConfig->dispelAuraDuration && aura->GetDuration() &&
            aura->GetDuration() < (int32)sPlayerbotAIConfig->dispelAuraDuration)
            continue;

        SpellInfo const* spellInfo = aura->GetSpellInfo();
        if (!spellInfo)
            continue;

        bool isPositiveSpell = spellInfo->IsPositive();
        if (isPositiveSpell && isFriend)
            continue;

        if (!isPositiveSpell && !isFriend)
            continue;

        // Use an empty whitelist since we don't have access to PlayerbotAI's dispel_whitelist
        std::vector<std::string> emptyWhitelist;
        if (CanDispelStatic(spellInfo, dispelType, emptyWhitelist))
            return true;
    }

    return false;
}

bool BotSpellService::CanDispelStatic(SpellInfo const* spellInfo, uint32 dispelType,
                                       std::vector<std::string> const& dispelWhitelist)
{
    if (spellInfo->Dispel != dispelType)
        return false;

    if (!spellInfo->SpellName[0])
    {
        return true;
    }

    for (auto const& wl : dispelWhitelist)
    {
        if (strcmpi((const char*)spellInfo->SpellName[0], wl.c_str()) == 0)
        {
            return false;
        }
    }

    return !spellInfo->SpellName[0] || (strcmpi((const char*)spellInfo->SpellName[0], "demon skin") &&
                                        strcmpi((const char*)spellInfo->SpellName[0], "mage armor") &&
                                        strcmpi((const char*)spellInfo->SpellName[0], "ice armor") &&
                                        strcmpi((const char*)spellInfo->SpellName[0], "frost armor") &&
                                        strcmpi((const char*)spellInfo->SpellName[0], "molten armor") &&
                                        strcmpi((const char*)spellInfo->SpellName[0], "demon armor"));
}

bool BotSpellService::IsInterruptableSpellCastingStatic(Player* /*bot*/, Unit* target, uint32 interruptSpellId)
{
    if (!target || !target->IsInWorld() || target->IsDuringRemoveFromWorld())
        return false;

    if (!interruptSpellId || !target->IsNonMeleeSpellCast(true))
        return false;

    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(interruptSpellId);
    if (!spellInfo)
        return false;

    for (uint8 i = EFFECT_0; i <= EFFECT_2; i++)
    {
        if ((spellInfo->InterruptFlags & SPELL_INTERRUPT_FLAG_INTERRUPT) &&
            spellInfo->PreventionType == SPELL_PREVENTION_TYPE_SILENCE)
            return true;

        if (spellInfo->Effects[i].Effect == SPELL_EFFECT_INTERRUPT_CAST &&
            !target->IsImmunedToSpellEffect(spellInfo, i))
            return true;

        if ((spellInfo->Effects[i].Effect == SPELL_EFFECT_APPLY_AURA) &&
            spellInfo->Effects[i].ApplyAuraName == SPELL_AURA_MOD_SILENCE)
            return true;
    }

    return false;
}

void BotSpellService::SpellInterruptedStatic(Player* /*bot*/, uint32 /*spellId*/)
{
    // This tracks interrupted spells - implementation would need access to PlayerbotAI's lastInterruptedSpellTime
    // For static version, this is a no-op since we can't store state
}

int32 BotSpellService::CalculateGlobalCooldownStatic(uint32 /*spellId*/)
{
    return sPlayerbotAIConfig->globalCoolDown;
}

bool BotSpellService::IsInVehicleStatic(Player* bot, bool canControl, bool canCast, bool canAttack, bool canTurn,
                                         bool fixed)
{
    Vehicle* vehicle = bot->GetVehicle();
    if (!vehicle)
        return false;

    Unit* vehicleBase = vehicle->GetBase();
    if (!vehicleBase || !vehicleBase->IsAlive())
        return false;

    if (!vehicle->GetVehicleInfo())
        return false;

    VehicleSeatEntry const* seat = vehicle->GetSeatForPassenger(bot);
    if (!seat)
        return false;

    if (!(canControl || canCast || canAttack || canTurn || fixed))
        return true;

    if (canControl)
        return seat->CanControl() && !(vehicle->GetVehicleInfo()->m_flags & VEHICLE_FLAG_FIXED_POSITION);

    if (canCast)
        return (seat->m_flags & VEHICLE_SEAT_FLAG_CAN_CAST) != 0;

    if (canAttack)
        return (seat->m_flags & VEHICLE_SEAT_FLAG_CAN_ATTACK) != 0;

    if (canTurn)
        return (seat->m_flags & VEHICLE_SEAT_FLAG_ALLOW_TURNING) != 0;

    if (fixed)
        return (vehicle->GetVehicleInfo()->m_flags & VEHICLE_FLAG_FIXED_POSITION) != 0;

    return false;
}

// ============================================================================
// Instance method implementations (IChatService interface)
// These delegate to static methods or PlayerbotAI during transition
// ============================================================================

bool BotSpellService::CanCastSpell(std::string const& name, Unit* target, Item* itemTarget)
{
    if (_botAI)
    {
        return _botAI->CanCastSpell(name, target, itemTarget);
    }
    return false;
}

bool BotSpellService::CastSpell(std::string const& name, Unit* target, Item* itemTarget)
{
    if (_botAI)
    {
        return _botAI->CastSpell(name, target, itemTarget);
    }
    return false;
}

bool BotSpellService::CanCastSpell(uint32 spellId, Unit* target, bool checkHasSpell, Item* itemTarget, Item* castItem)
{
    if (_botAI)
    {
        return _botAI->CanCastSpell(spellId, target, checkHasSpell, itemTarget, castItem);
    }
    return false;
}

bool BotSpellService::CanCastSpell(uint32 spellId, GameObject* goTarget, bool checkHasSpell)
{
    if (_botAI)
    {
        return _botAI->CanCastSpell(spellId, goTarget, checkHasSpell);
    }
    return false;
}

bool BotSpellService::CanCastSpell(uint32 spellId, float x, float y, float z, bool checkHasSpell, Item* itemTarget)
{
    if (_botAI)
    {
        return _botAI->CanCastSpell(spellId, x, y, z, checkHasSpell, itemTarget);
    }
    return false;
}

bool BotSpellService::CastSpell(uint32 spellId, Unit* target, Item* itemTarget)
{
    if (_botAI)
    {
        return _botAI->CastSpell(spellId, target, itemTarget);
    }
    return false;
}

bool BotSpellService::CastSpell(uint32 spellId, float x, float y, float z, Item* itemTarget)
{
    if (_botAI)
    {
        return _botAI->CastSpell(spellId, x, y, z, itemTarget);
    }
    return false;
}

bool BotSpellService::HasAura(std::string const& spellName, Unit* player, bool maxStack, bool checkIsOwner,
                              int maxAmount, bool checkDuration)
{
    if (_botAI)
    {
        return HasAuraByNameStatic(_botAI->GetBot(), spellName, player, maxStack, checkIsOwner, maxAmount, checkDuration);
    }
    return false;
}

bool BotSpellService::HasAura(uint32 spellId, Unit const* player)
{
    return HasAuraByIdStatic(spellId, player);
}

bool BotSpellService::HasAnyAuraOf(Unit* player, ...)
{
    if (!_botAI || !player)
        return false;

    va_list vl;
    va_start(vl, player);

    const char* cur;
    while ((cur = va_arg(vl, const char*)) != nullptr)
    {
        if (HasAura(cur, player))
        {
            va_end(vl);
            return true;
        }
    }

    va_end(vl);
    return false;
}

bool BotSpellService::HasAnyAuraOfVec(Unit* player, std::vector<std::string> const& auraNames)
{
    if (_botAI)
    {
        return HasAnyAuraOfStatic(_botAI->GetBot(), player, auraNames);
    }
    return false;
}

Aura* BotSpellService::GetAura(std::string const& spellName, Unit* unit, bool checkIsOwner, bool checkDuration,
                               int checkStack)
{
    if (_botAI)
    {
        return GetAuraStatic(_botAI->GetBot(), spellName, unit, checkIsOwner, checkDuration, checkStack);
    }
    return nullptr;
}

void BotSpellService::RemoveAura(std::string const& name)
{
    if (_botAI)
    {
        _botAI->RemoveAura(name);
    }
}

void BotSpellService::RemoveShapeshift()
{
    if (_botAI)
    {
        _botAI->RemoveShapeshift();
    }
}

bool BotSpellService::HasAuraToDispel(Unit* player, uint32 dispelType)
{
    if (_botAI)
    {
        return HasAuraToDispelStatic(_botAI->GetBot(), player, dispelType);
    }
    return false;
}

bool BotSpellService::CanDispel(SpellInfo const* spellInfo, uint32 dispelType)
{
    if (_botAI)
    {
        return _botAI->canDispel(spellInfo, dispelType);
    }
    return false;
}

bool BotSpellService::IsInterruptableSpellCasting(Unit* player, std::string const& spell)
{
    if (_botAI)
    {
        return _botAI->IsInterruptableSpellCasting(player, spell);
    }
    return false;
}

void BotSpellService::InterruptSpell()
{
    if (_botAI)
    {
        _botAI->InterruptSpell();
    }
}

void BotSpellService::SpellInterrupted(uint32 spellId)
{
    if (_botAI)
    {
        _botAI->SpellInterrupted(spellId);
    }
}

int32 BotSpellService::CalculateGlobalCooldown(uint32 spellId)
{
    return CalculateGlobalCooldownStatic(spellId);
}

void BotSpellService::WaitForSpellCast(Spell* spell)
{
    if (_botAI)
    {
        _botAI->WaitForSpellCast(spell);
    }
}

bool BotSpellService::CanCastVehicleSpell(uint32 spellId, Unit* target)
{
    if (_botAI)
    {
        return _botAI->CanCastVehicleSpell(spellId, target);
    }
    return false;
}

bool BotSpellService::CastVehicleSpell(uint32 spellId, Unit* target)
{
    if (_botAI)
    {
        return _botAI->CastVehicleSpell(spellId, target);
    }
    return false;
}

bool BotSpellService::CastVehicleSpell(uint32 spellId, float x, float y, float z)
{
    if (_botAI)
    {
        return _botAI->CastVehicleSpell(spellId, x, y, z);
    }
    return false;
}

bool BotSpellService::IsInVehicle(bool canControl, bool canCast, bool canAttack, bool canTurn, bool fixed)
{
    if (_botAI)
    {
        return IsInVehicleStatic(_botAI->GetBot(), canControl, canCast, canAttack, canTurn, fixed);
    }
    return false;
}
