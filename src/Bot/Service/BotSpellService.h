/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_BOT_SPELL_SERVICE_H
#define _PLAYERBOT_BOT_SPELL_SERVICE_H

#include "Bot/Interface/ISpellService.h"

#include <functional>
#include <vector>

class AiObjectContext;
class AuraEffect;
class ChatHelper;
class Player;
class PlayerbotAI;

/**
 * @brief Context for spell operations
 *
 * Provides all dependencies needed for spell operations without
 * requiring direct access to PlayerbotAI.
 */
struct SpellContext
{
    Player* bot;
    AiObjectContext* aiObjectContext;
    ChatHelper* chatHelper;
    std::function<void(uint32)> setNextCheckDelay;
    std::function<bool(std::string const&, int)> hasStrategy;
    std::function<bool()> hasRealPlayerMaster;
};

/**
 * @brief Implementation of ISpellService
 *
 * This service provides spell casting and aura management for bots.
 *
 * Static methods are provided for direct access without needing a service instance.
 * Instance methods implement the ISpellService interface for testability/mockability.
 */
class BotSpellService : public ISpellService
{
public:
    BotSpellService() = default;
    explicit BotSpellService(PlayerbotAI* ai) : _botAI(ai) {}
    ~BotSpellService() override = default;

    // ========================================================================
    // Static methods for direct access (main implementations)
    // These can be called without a service instance
    // ========================================================================

    // Aura checking - static versions
    static bool IsRealAuraStatic(Player* bot, AuraEffect const* aurEff, Unit const* unit);
    static bool HasAuraByNameStatic(Player* bot, std::string const& name, Unit* unit, bool maxStack = false,
                                    bool checkIsOwner = false, int maxAuraAmount = -1, bool checkDuration = false);
    static bool HasAuraByIdStatic(uint32 spellId, Unit const* unit);
    static bool HasAnyAuraOfStatic(Player* bot, Unit* player, std::vector<std::string> const& auraNames);
    static Aura* GetAuraStatic(Player* bot, std::string const& name, Unit* unit, bool checkIsOwner = false,
                               bool checkDuration = false, int checkStack = -1);

    // Dispel checking - static versions
    static bool HasAuraToDispelStatic(Player* bot, Unit* target, uint32 dispelType);
    static bool CanDispelStatic(SpellInfo const* spellInfo, uint32 dispelType,
                                std::vector<std::string> const& dispelWhitelist);

    // Interrupt checking - static versions
    static bool IsInterruptableSpellCastingStatic(Player* bot, Unit* target, uint32 interruptSpellId);
    static void SpellInterruptedStatic(Player* bot, uint32 spellId);
    static int32 CalculateGlobalCooldownStatic(uint32 spellId);

    // Vehicle - static versions
    static bool IsInVehicleStatic(Player* bot, bool canControl, bool canCast, bool canAttack, bool canTurn, bool fixed);

    // ========================================================================
    // Instance methods (ISpellService interface implementation)
    // These delegate to static methods or PlayerbotAI during transition
    // ========================================================================

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

    // Vector-based aura checking (safer alternative to variadic)
    bool HasAnyAuraOfVec(Unit* player, std::vector<std::string> const& auraNames);

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

    // Set the bot context for instance methods that need the bot
    void SetBotContext(PlayerbotAI* ai) { _botAI = ai; }
    PlayerbotAI* GetBotContext() const { return _botAI; }

private:
    PlayerbotAI* _botAI = nullptr;
};

#endif
