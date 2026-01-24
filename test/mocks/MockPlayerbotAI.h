/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_MOCK_PLAYERBOT_AI_H
#define _PLAYERBOT_MOCK_PLAYERBOT_AI_H

#include <gmock/gmock.h>
#include <string>

// Forward declarations for types used in the interface
class Player;
class Unit;
class Item;
class Spell;
class Aura;
class SpellInfo;

/**
 * @brief Mock interface for PlayerbotAI
 *
 * This mock is used for testing components that depend on PlayerbotAI
 * without requiring a full game server context.
 *
 * Note: This is a simplified interface for testing purposes.
 * The actual PlayerbotAI class has many more methods.
 */
class IMockPlayerbotAI
{
public:
    virtual ~IMockPlayerbotAI() = default;

    // Role detection (static methods in real class)
    virtual bool IsTank() const = 0;
    virtual bool IsHeal() const = 0;
    virtual bool IsDps() const = 0;
    virtual bool IsRanged() const = 0;
    virtual bool IsMelee() const = 0;
    virtual bool IsCaster() const = 0;
    virtual bool IsMainTank() const = 0;

    // Spell casting
    virtual bool CanCastSpell(std::string const& name, Unit* target) = 0;
    virtual bool CastSpell(std::string const& name, Unit* target) = 0;
    virtual bool HasAura(std::string const& spellName, Unit* player) = 0;

    // Communication
    virtual bool TellMaster(std::string const& text) = 0;
    virtual bool TellError(std::string const& text) = 0;

    // Bot access
    virtual Player* GetBot() = 0;
    virtual Player* GetMaster() = 0;
};

/**
 * @brief Google Mock implementation of IMockPlayerbotAI
 */
class MockPlayerbotAI : public IMockPlayerbotAI
{
public:
    // Role detection
    MOCK_METHOD(bool, IsTank, (), (const, override));
    MOCK_METHOD(bool, IsHeal, (), (const, override));
    MOCK_METHOD(bool, IsDps, (), (const, override));
    MOCK_METHOD(bool, IsRanged, (), (const, override));
    MOCK_METHOD(bool, IsMelee, (), (const, override));
    MOCK_METHOD(bool, IsCaster, (), (const, override));
    MOCK_METHOD(bool, IsMainTank, (), (const, override));

    // Spell casting
    MOCK_METHOD(bool, CanCastSpell, (std::string const& name, Unit* target), (override));
    MOCK_METHOD(bool, CastSpell, (std::string const& name, Unit* target), (override));
    MOCK_METHOD(bool, HasAura, (std::string const& spellName, Unit* player), (override));

    // Communication
    MOCK_METHOD(bool, TellMaster, (std::string const& text), (override));
    MOCK_METHOD(bool, TellError, (std::string const& text), (override));

    // Bot access
    MOCK_METHOD(Player*, GetBot, (), (override));
    MOCK_METHOD(Player*, GetMaster, (), (override));
};

#endif
