/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_MOCK_BOT_SERVICES_H
#define _PLAYERBOT_MOCK_BOT_SERVICES_H

#include <gmock/gmock.h>
#include "Bot/Interface/IBotContext.h"
#include "Bot/Interface/IChatService.h"
#include "Bot/Interface/IItemService.h"
#include "Bot/Interface/IRoleService.h"
#include "Bot/Interface/ISpellService.h"

/**
 * @brief Mock implementation of IBotContext for testing
 */
class MockBotContext : public IBotContext
{
public:
    MOCK_METHOD(Player*, GetBot, (), (override));
    MOCK_METHOD(Player*, GetMaster, (), (override));
    MOCK_METHOD(void, SetMaster, (Player* newMaster), (override));
    MOCK_METHOD(BotState, GetState, (), (const, override));
    MOCK_METHOD(bool, IsInCombat, (), (const, override));
    MOCK_METHOD(bool, IsRealPlayer, (), (const, override));
    MOCK_METHOD(bool, HasRealPlayerMaster, (), (const, override));
    MOCK_METHOD(bool, HasActivePlayerMaster, (), (const, override));
    MOCK_METHOD(bool, IsAlt, (), (const, override));
    MOCK_METHOD(Creature*, GetCreature, (ObjectGuid guid), (override));
    MOCK_METHOD(Unit*, GetUnit, (ObjectGuid guid), (override));
    MOCK_METHOD(Player*, GetPlayer, (ObjectGuid guid), (override));
    MOCK_METHOD(GameObject*, GetGameObject, (ObjectGuid guid), (override));
    MOCK_METHOD(WorldObject*, GetWorldObject, (ObjectGuid guid), (override));
    MOCK_METHOD(AreaTableEntry const*, GetCurrentArea, (), (const, override));
    MOCK_METHOD(AreaTableEntry const*, GetCurrentZone, (), (const, override));
    MOCK_METHOD(std::vector<Player*>, GetPlayersInGroup, (), (override));
    MOCK_METHOD(Player*, GetGroupLeader, (), (override));
    MOCK_METHOD(bool, IsSafe, (Player* player), (const, override));
    MOCK_METHOD(bool, IsSafe, (WorldObject* obj), (const, override));
    MOCK_METHOD(bool, IsOpposing, (Player* player), (const, override));
    MOCK_METHOD(bool, CanMove, (), (const, override));
    MOCK_METHOD(bool, HasPlayerNearby, (float range), (const, override));
    MOCK_METHOD(bool, HasPlayerNearby, (WorldPosition* pos, float range), (const, override));
    MOCK_METHOD(bool, HasManyPlayersNearby, (uint32 triggerValue, float range), (const, override));
};

/**
 * @brief Mock implementation of IRoleService for testing
 */
class MockRoleService : public IRoleService
{
public:
    MOCK_METHOD(bool, IsTank, (Player* player, bool bySpec), (const, override));
    MOCK_METHOD(bool, IsHeal, (Player* player, bool bySpec), (const, override));
    MOCK_METHOD(bool, IsDps, (Player* player, bool bySpec), (const, override));
    MOCK_METHOD(bool, IsRanged, (Player* player, bool bySpec), (const, override));
    MOCK_METHOD(bool, IsMelee, (Player* player, bool bySpec), (const, override));
    MOCK_METHOD(bool, IsCaster, (Player* player, bool bySpec), (const, override));
    MOCK_METHOD(bool, IsRangedDps, (Player* player, bool bySpec), (const, override));
    MOCK_METHOD(bool, IsCombo, (Player* player), (const, override));
    MOCK_METHOD(bool, IsBotMainTank, (Player* player), (const, override));
    MOCK_METHOD(bool, IsMainTank, (Player* player), (const, override));
    MOCK_METHOD(bool, IsAssistTank, (Player* player), (const, override));
    MOCK_METHOD(bool, IsAssistTankOfIndex, (Player* player, int index, bool ignoreDeadPlayers), (const, override));
    MOCK_METHOD(uint32, GetGroupTankNum, (Player* player), (const, override));
    MOCK_METHOD(int32, GetAssistTankIndex, (Player* player), (const, override));
    MOCK_METHOD(int32, GetGroupSlotIndex, (Player* player), (const, override));
    MOCK_METHOD(int32, GetRangedIndex, (Player* player), (const, override));
    MOCK_METHOD(int32, GetRangedDpsIndex, (Player* player), (const, override));
    MOCK_METHOD(int32, GetMeleeIndex, (Player* player), (const, override));
    MOCK_METHOD(int32, GetClassIndex, (Player* player, uint8 cls), (const, override));
    MOCK_METHOD(bool, IsHealAssistantOfIndex, (Player* player, int index), (const, override));
    MOCK_METHOD(bool, IsRangedDpsAssistantOfIndex, (Player* player, int index), (const, override));
    MOCK_METHOD(bool, HasAggro, (Unit* unit), (const, override));
};

/**
 * @brief Mock implementation of ISpellService for testing
 */
class MockSpellService : public ISpellService
{
public:
    MOCK_METHOD(bool, CanCastSpell, (std::string const& name, Unit* target, Item* itemTarget), (override));
    MOCK_METHOD(bool, CastSpell, (std::string const& name, Unit* target, Item* itemTarget), (override));
    MOCK_METHOD(bool, CanCastSpell,
                (uint32 spellId, Unit* target, bool checkHasSpell, Item* itemTarget, Item* castItem), (override));
    MOCK_METHOD(bool, CanCastSpell, (uint32 spellId, GameObject* goTarget, bool checkHasSpell), (override));
    MOCK_METHOD(bool, CanCastSpell, (uint32 spellId, float x, float y, float z, bool checkHasSpell, Item* itemTarget),
                (override));
    MOCK_METHOD(bool, CastSpell, (uint32 spellId, Unit* target, Item* itemTarget), (override));
    MOCK_METHOD(bool, CastSpell, (uint32 spellId, float x, float y, float z, Item* itemTarget), (override));
    MOCK_METHOD(bool, HasAura,
                (std::string const& spellName, Unit* player, bool maxStack, bool checkIsOwner, int maxAmount,
                 bool checkDuration),
                (override));
    MOCK_METHOD(bool, HasAura, (uint32 spellId, Unit const* player), (override));
    // NOTE: HasAnyAuraOf uses variadic args (...) which GMock cannot mock.
    // Provide a stub implementation that returns false.
    bool HasAnyAuraOf(Unit* /*player*/, ...) override { return false; }
    MOCK_METHOD(Aura*, GetAura,
                (std::string const& spellName, Unit* unit, bool checkIsOwner, bool checkDuration, int checkStack),
                (override));
    MOCK_METHOD(void, RemoveAura, (std::string const& name), (override));
    MOCK_METHOD(void, RemoveShapeshift, (), (override));
    MOCK_METHOD(bool, HasAuraToDispel, (Unit * player, uint32 dispelType), (override));
    MOCK_METHOD(bool, CanDispel, (SpellInfo const* spellInfo, uint32 dispelType), (override));
    MOCK_METHOD(bool, IsInterruptableSpellCasting, (Unit * player, std::string const& spell), (override));
    MOCK_METHOD(void, InterruptSpell, (), (override));
    MOCK_METHOD(void, SpellInterrupted, (uint32 spellId), (override));
    MOCK_METHOD(int32, CalculateGlobalCooldown, (uint32 spellId), (override));
    MOCK_METHOD(void, WaitForSpellCast, (Spell * spell), (override));
    MOCK_METHOD(bool, CanCastVehicleSpell, (uint32 spellId, Unit* target), (override));
    MOCK_METHOD(bool, CastVehicleSpell, (uint32 spellId, Unit* target), (override));
    MOCK_METHOD(bool, CastVehicleSpell, (uint32 spellId, float x, float y, float z), (override));
    MOCK_METHOD(bool, IsInVehicle, (bool canControl, bool canCast, bool canAttack, bool canTurn, bool fixed),
                (override));
};

/**
 * @brief Mock implementation of IChatService for testing
 */
class MockChatService : public IChatService
{
public:
    MOCK_METHOD(bool, TellMaster, (std::string const& text, PlayerbotSecurityLevel securityLevel), (override));
    MOCK_METHOD(bool, TellMaster, (std::ostringstream & stream, PlayerbotSecurityLevel securityLevel), (override));
    MOCK_METHOD(bool, TellMasterNoFacing, (std::string const& text, PlayerbotSecurityLevel securityLevel), (override));
    MOCK_METHOD(bool, TellMasterNoFacing, (std::ostringstream & stream, PlayerbotSecurityLevel securityLevel),
                (override));
    MOCK_METHOD(bool, TellError, (std::string const& text, PlayerbotSecurityLevel securityLevel), (override));
    MOCK_METHOD(bool, SayToGuild, (std::string const& msg), (override));
    MOCK_METHOD(bool, SayToWorld, (std::string const& msg), (override));
    MOCK_METHOD(bool, SayToChannel, (std::string const& msg, uint32 channelId), (override));
    MOCK_METHOD(bool, SayToParty, (std::string const& msg), (override));
    MOCK_METHOD(bool, SayToRaid, (std::string const& msg), (override));
    MOCK_METHOD(bool, Say, (std::string const& msg), (override));
    MOCK_METHOD(bool, Yell, (std::string const& msg), (override));
    MOCK_METHOD(bool, Whisper, (std::string const& msg, std::string const& receiverName), (override));
    MOCK_METHOD(bool, PlaySound, (uint32 emote), (override));
    MOCK_METHOD(bool, PlayEmote, (uint32 emote), (override));
    MOCK_METHOD(void, Ping, (float x, float y), (override));
};

/**
 * @brief Mock implementation of IItemService for testing
 */
class MockItemService : public IItemService
{
public:
    MOCK_METHOD(Item*, FindPoison, (), (const, override));
    MOCK_METHOD(Item*, FindAmmo, (), (const, override));
    MOCK_METHOD(Item*, FindBandage, (), (const, override));
    MOCK_METHOD(Item*, FindOpenableItem, (), (const, override));
    MOCK_METHOD(Item*, FindLockedItem, (), (const, override));
    MOCK_METHOD(Item*, FindConsumable, (uint32 itemId), (const, override));
    MOCK_METHOD(Item*, FindStoneFor, (Item * weapon), (const, override));
    MOCK_METHOD(Item*, FindOilFor, (Item * weapon), (const, override));
    MOCK_METHOD(void, ImbueItem, (Item * item, uint32 targetFlag, ObjectGuid targetGUID), (override));
    MOCK_METHOD(void, ImbueItem, (Item * item, uint8 targetInventorySlot), (override));
    MOCK_METHOD(void, ImbueItem, (Item * item, Unit* target), (override));
    MOCK_METHOD(void, ImbueItem, (Item * item), (override));
    MOCK_METHOD(void, EnchantItem, (uint32 spellId, uint8 slot), (override));
    MOCK_METHOD(std::vector<Item*>, GetInventoryAndEquippedItems, (), (const, override));
    MOCK_METHOD(std::vector<Item*>, GetInventoryItems, (), (const, override));
    MOCK_METHOD(uint32, GetInventoryItemsCountWithId, (uint32 itemId), (const, override));
    MOCK_METHOD(bool, HasItemInInventory, (uint32 itemId), (const, override));
    MOCK_METHOD(InventoryResult, CanEquipItem, (uint8 slot, uint16& dest, Item* pItem, bool swap, bool notLoading),
                (const, override));
    MOCK_METHOD(uint8, FindEquipSlot, (ItemTemplate const* proto, uint32 slot, bool swap), (const, override));
    MOCK_METHOD(uint32, GetEquipGearScore, (Player* player), (const, override));
    MOCK_METHOD((std::vector<std::pair<Quest const*, uint32>>), GetCurrentQuestsRequiringItemId, (uint32 itemId),
                (const, override));
};

#endif
