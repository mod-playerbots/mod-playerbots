/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_BOT_SPELL_SERVICE_H
#define _PLAYERBOT_BOT_SPELL_SERVICE_H

#include "Bot/Interface/ISpellService.h"

class PlayerbotAI;

/**
 * @brief Implementation of ISpellService
 *
 * This service provides spell casting and aura management for bots,
 * extracting this functionality from PlayerbotAI for better testability.
 *
 * The service delegates to PlayerbotAI methods during the transition period.
 */
class BotSpellService : public ISpellService
{
public:
    explicit BotSpellService(PlayerbotAI* ai) : botAI_(ai) {}
    ~BotSpellService() override = default;

    // Spell casting by name
    bool CanCastSpell(std::string const& name, Unit* target, Item* itemTarget = nullptr) override;
    bool CastSpell(std::string const& name, Unit* target, Item* itemTarget = nullptr) override;

    // Spell casting by ID
    bool CanCastSpell(uint32 spellId, Unit* target, bool checkHasSpell = true, Item* itemTarget = nullptr,
                      Item* castItem = nullptr) override;
    bool CanCastSpell(uint32 spellId, GameObject* goTarget, bool checkHasSpell = true) override;
    bool CanCastSpell(uint32 spellId, float x, float y, float z, bool checkHasSpell = true,
                      Item* itemTarget = nullptr) override;

    bool CastSpell(uint32 spellId, Unit* target, Item* itemTarget = nullptr) override;
    bool CastSpell(uint32 spellId, float x, float y, float z, Item* itemTarget = nullptr) override;

    // Aura management
    bool HasAura(std::string const& spellName, Unit* player, bool maxStack = false, bool checkIsOwner = false,
                 int maxAmount = -1, bool checkDuration = false) override;
    bool HasAura(uint32 spellId, Unit const* player) override;
    bool HasAnyAuraOf(Unit* player, ...) override;

    Aura* GetAura(std::string const& spellName, Unit* unit, bool checkIsOwner = false, bool checkDuration = false,
                  int checkStack = -1) override;
    void RemoveAura(std::string const& name) override;
    void RemoveShapeshift() override;

    // Dispel
    bool HasAuraToDispel(Unit* player, uint32 dispelType) override;
    bool CanDispel(SpellInfo const* spellInfo, uint32 dispelType) override;

    // Interrupt
    bool IsInterruptableSpellCasting(Unit* player, std::string const& spell) override;
    void InterruptSpell() override;
    void SpellInterrupted(uint32 spellId) override;

    // Cooldown
    int32 CalculateGlobalCooldown(uint32 spellId) override;
    void WaitForSpellCast(Spell* spell) override;

    // Vehicle spells
    bool CanCastVehicleSpell(uint32 spellId, Unit* target) override;
    bool CastVehicleSpell(uint32 spellId, Unit* target) override;
    bool CastVehicleSpell(uint32 spellId, float x, float y, float z) override;
    bool IsInVehicle(bool canControl = false, bool canCast = false, bool canAttack = false, bool canTurn = false,
                     bool fixed = false) override;

private:
    PlayerbotAI* botAI_;
};

#endif
