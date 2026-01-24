/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_ISPELL_SERVICE_H
#define _PLAYERBOT_ISPELL_SERVICE_H

#include "Common.h"

class Unit;
class Item;
class Spell;
class Aura;
class SpellInfo;
class GameObject;

/**
 * @brief Interface for spell casting and aura management
 *
 * This interface abstracts spell-related operations from PlayerbotAI,
 * enabling isolated testing of spell logic.
 */
class ISpellService
{
public:
    virtual ~ISpellService() = default;

    // Spell casting by name
    virtual bool CanCastSpell(std::string const& name, Unit* target, Item* itemTarget = nullptr) = 0;
    virtual bool CastSpell(std::string const& name, Unit* target, Item* itemTarget = nullptr) = 0;

    // Spell casting by ID
    virtual bool CanCastSpell(uint32 spellId, Unit* target, bool checkHasSpell = true, Item* itemTarget = nullptr,
                              Item* castItem = nullptr) = 0;
    virtual bool CanCastSpell(uint32 spellId, GameObject* goTarget, bool checkHasSpell = true) = 0;
    virtual bool CanCastSpell(uint32 spellId, float x, float y, float z, bool checkHasSpell = true,
                              Item* itemTarget = nullptr) = 0;

    virtual bool CastSpell(uint32 spellId, Unit* target, Item* itemTarget = nullptr) = 0;
    virtual bool CastSpell(uint32 spellId, float x, float y, float z, Item* itemTarget = nullptr) = 0;

    // Aura management
    virtual bool HasAura(std::string const& spellName, Unit* player, bool maxStack = false, bool checkIsOwner = false,
                         int maxAmount = -1, bool checkDuration = false) = 0;
    virtual bool HasAura(uint32 spellId, Unit const* player) = 0;
    virtual bool HasAnyAuraOf(Unit* player, ...) = 0;

    virtual Aura* GetAura(std::string const& spellName, Unit* unit, bool checkIsOwner = false,
                          bool checkDuration = false, int checkStack = -1) = 0;
    virtual void RemoveAura(std::string const& name) = 0;
    virtual void RemoveShapeshift() = 0;

    // Dispel
    virtual bool HasAuraToDispel(Unit* player, uint32 dispelType) = 0;
    virtual bool CanDispel(SpellInfo const* spellInfo, uint32 dispelType) = 0;

    // Interrupt
    virtual bool IsInterruptableSpellCasting(Unit* player, std::string const& spell) = 0;
    virtual void InterruptSpell() = 0;
    virtual void SpellInterrupted(uint32 spellId) = 0;

    // Cooldown
    virtual int32 CalculateGlobalCooldown(uint32 spellId) = 0;
    virtual void WaitForSpellCast(Spell* spell) = 0;

    // Vehicle spells
    virtual bool CanCastVehicleSpell(uint32 spellId, Unit* target) = 0;
    virtual bool CastVehicleSpell(uint32 spellId, Unit* target) = 0;
    virtual bool CastVehicleSpell(uint32 spellId, float x, float y, float z) = 0;
    virtual bool IsInVehicle(bool canControl = false, bool canCast = false, bool canAttack = false,
                             bool canTurn = false, bool fixed = false) = 0;
};

#endif
