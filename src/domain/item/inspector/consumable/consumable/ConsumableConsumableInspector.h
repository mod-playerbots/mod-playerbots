#pragma once

#include <cstdint>

#include "SharedDefines.h"
#include "ItemTemplate.h"

#include "AbstractConsumableInspector.h"
#include "ItemActionStruct.h"
#include "ItemActionEnum.h"
#include "GlobalPlayerInspector.h"

class ConsumableConsumableInspector : public AbstractConsumableInspector
{
public:
	ConsumableConsumableInspector(
		uint64_t playerLowGUID,
		uint64_t itemLowGUID
	) : AbstractConsumableInspector(playerLowGUID, itemLowGUID)
	{}

	bool isInspectable() const
	{
		const uint8_t itemSubclass = this->getCurrentItemSubclass();

		return AbstractConsumableInspector::isInspectable() && itemSubclass == ITEM_SUBCLASS_CONSUMABLE;
	}

	ItemActionStruct determineItemAction() const
	{
		if (!this->isInspectable())
			return this->getDefaultItemAction();

		if (this->isForbiddenItem())
			return this->getForbiddenItemAction();

		const ObjectGuid playerGUID = ObjectGuid::Create<HighGuid::Player>(this->playerLowGUID);
		Player* player = ObjectAccessor::FindPlayer(playerGUID);

		if (player == nullptr)
			return this->getDefaultItemAction();

		const uint8_t playerClass = player->getClass();

		if (playerClass == CLASS_ROGUE && this->isRoguePoisonItem())
			return this->getDefaultItemAction();

		if (playerClass == CLASS_MAGE && this->isMageItem())
			return this->getDefaultItemAction();

		if (playerClass == CLASS_WARLOCK && this->isWarlockItem())
			return this->getDefaultItemAction();

		const ItemTemplate* const itemTemplate = this->getCurrentItemTemplate();

		if (itemTemplate == nullptr)
			return this->getDefaultItemAction();

		player = ObjectAccessor::FindPlayer(playerGUID);

		if (player == nullptr)
			return this->getDefaultItemAction();

		if (this->isQuestConsumableItem() && player->HasQuestForItem(itemTemplate->ItemId))
			return this->getDefaultItemAction();

		if (!this->isUnwantedItem())
			return this->getDefaultItemAction();

		return this->getSellAction();
	}

	bool isForbiddenItem() const
	{
		const std::unordered_set<uint32_t> forbiddenItems = this->getForbiddenItemsGUIDs();
		const ItemTemplate* const itemTemplate = this->getCurrentItemTemplate();

		if (itemTemplate == nullptr)
			return false;

		return forbiddenItems.contains(itemTemplate->ItemId);
	}

	bool isUnwantedItem() const
	{
		const std::unordered_set<uint32_t> unwantedItems = this->getUnwantedItemsGUIDs();
		const ItemTemplate* const itemTemplate = this->getCurrentItemTemplate();

		if (itemTemplate == nullptr)
			return false;

		return unwantedItems.contains(itemTemplate->ItemId);
	}

	bool isQuestConsumableItem() const
	{
		const std::unordered_set<uint32_t> questConumableItems = this->getQuestConsumableItemsGUIDs();
		const ItemTemplate* const itemTemplate = this->getCurrentItemTemplate();

		if (itemTemplate == nullptr)
			return false;

		return questConumableItems.contains(itemTemplate->ItemId);
	}

	bool isRoguePoisonItem() const
	{
		const std::unordered_set<uint32_t> roguePoisonItems = this->getRoguePoisonsItemsGUIDs();
		const ItemTemplate* const itemTemplate = this->getCurrentItemTemplate();

		if (itemTemplate == nullptr)
			return false;

		return roguePoisonItems.contains(itemTemplate->ItemId);
	}

	bool isUsefulConsumableItem() const
	{
		const std::unordered_set<uint32_t> usefulConsumableItems = this->getUsefulConsumableItemsGUIDs();
		const ItemTemplate* const itemTemplate = this->getCurrentItemTemplate();

		if (itemTemplate == nullptr)
			return false;

		return usefulConsumableItems.contains(itemTemplate->ItemId);
	}

	bool isWarlockItem() const
	{
		const std::unordered_set<uint32_t> warlockItems = this->getWarlockItemsGUIDs();
		const ItemTemplate* const itemTemplate = this->getCurrentItemTemplate();

		if (itemTemplate == nullptr)
			return false;

		return warlockItems.contains(itemTemplate->ItemId);
	}

	bool isMageItem() const
	{
		const std::unordered_set<uint32_t> mageItems = this->getMageItemsGUIDs();
		const ItemTemplate* const itemTemplate = this->getCurrentItemTemplate();

		if (itemTemplate == nullptr)
			return false;

		return mageItems.contains(itemTemplate->ItemId);
	}

protected:

	std::unordered_set<uint32_t> getForbiddenItemsGUIDs() const
	{
		return {
			1199, // Charged Soulstone
			1700, // Admin Warlord's Claymore
			1995, // Deprecated Cat's Paw
			3438, // Ankh of Resurrection
			4420, // NPC Equip 4420 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			4423, // NPC Equip 4423 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			4842, // test => Illegal item that should not even be present in the core as it is not present in any WotLK database
			5046, // Locked Gift => Illegal item that should not even be present in the core as it is not present in any WotLK database
			5047, // Skull Wrapping Paper => Illegal item that should not even be present in the core as it is not present in any WotLK database
			5224, // NPC Equip 5224 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			5225, // NPC Equip 5225 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			5378, // Shane Test (DELETE ME) => Illegal item that should not even be present in the core as it is not present in any WotLK database
			6244, // ggggfg => Illegal item that should not even be present in the core as it is not present in any WotLK database
			6638, // Air Sapta => Apparently this sapta is the only one not needed by shamans?
			6852, // Eternal Eye => Quest item seemingly linked to no quest
			6927, // Big Will's Ear => Quest item seemingly linked to no quest. It was supposedly dropped by Big Will but the quest requires killing him without looting him.
			7147, // NPC Equip 7147 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			7168, // NPC Equip 7168 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			7388, // Skull Key => Quest item seemingly linked to no quest
			9232, // NPC Equip 9232 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			9319, // Nimboya's Laden Pike => Quest item seemingly linked to no quest
			10419, // NPC Equip 10419 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			10448, // NPC Equip 10448 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			10449, // NPC Equip 10449 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			10451, // NPC Equip 10451 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			10452, // NPC Equip 10452 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			10453, // NPC Equip 10453 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			11182, // NPC Equip 11182 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			11183, // NPC Equip 11183 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			11616, // DEBUG Samophlange Manual Page
			12246, // NPC Equip 12246 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			12385, // test => Illegal item that should not even be present in the core as it is not present in any WotLK database
			12439, // NPC Equip 12439 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			12441, // NPC Equip 12441 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			13294, // NPC Equip 13294 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			17350, // NPC Equip 17350 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			17729, // NPC Equip 17729 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			18595, // Blood Opal => Item not present in WotLK.
			19063, // NPC Equip 19063 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			19977, // NPC Equip 19977 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			20365, // NPC Equip 20365 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			20386, // NPC Equip 20386 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			20473, // NPC Equip 20473 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			20905, // NPC Equip 20905 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			21238, // Winter Veil Cookie UNUSED
			21556, // NPC Equip 21556 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			21739, // Lunar Festival Invitation DEBUG
			21832, // NPC Equip 21832 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			21834, // NPC Equip 21834 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			21922, // NPC Equip 21922 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			21961, // NPC Equip 21961 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			21965, // NPC Equip 21965 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			21966, // NPC Equip 21966 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			21967, // NPC Equip 21967 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			21968, // NPC Equip 21968 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			21969, // NPC Equip 21969 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			21970, // NPC Equip 21970 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			21971, // NPC Equip 21971 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			21972, // NPC Equip 21972 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			21973, // NPC Equip 21973 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			21974, // NPC Equip 21974 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			21976, // NPC Equip 21976 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			21977, // NPC Equip 21977 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			21978, // NPC Equip 21978 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			22118, // NPC Equip 22118 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			22124, // NPC Equip 22124 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			22125, // NPC Equip 22125 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			22126, // NPC Equip 22126 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			22127, // NPC Equip 22127 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			22129, // NPC Equip 22129 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			22323, // NPC Equip 22323 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			23163, // Performer's Wand => Unobtainable item
			23164, // Bubbly Beverage => Unobtainable item
			23175, // Tasty Summer Treat => Unobtainable item (iCoke promotion)
			23176, // Fizzy Energy Drink => Unobtainable item (iCoke promotion?)
			23209, // NPC Equip 23209 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			23210, // NPC Equip 23210 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			23496, // NPC Equip 23496 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			23640, // NPC Equip 23640 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			23641, // NPC Equip 23641 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			23715, // Permanent Lung Juice Cocktail => Unobtainable item
			23718, // Permanent Ground Scorpok Assay => Unobtainable item
			23719, // Permanent Cerebral Cortex Compound => Unobtainable item
			23721, // Permanent Gizzard Gum => Unobtainable item
			23722, // Permanent R.O.I.D.S. => Unobtainable item
			23794, // Permanent Sheen of Zanza => Unobtainable item
			23795, // Permanent Spirit of Zanza => Unobtainable item
			23796, // Permanent Swiftness of Zanza => Unobtainable item
			23872, // NPC Equip 23872 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			23982, // NPC Equip 23982 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			26046, // NPC Equip 26046 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			26047, // NPC Equip 26047 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			27504, // NPC Equip 27504 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			28036, // NPC Equip 28036 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			28110, // Fat Gnome and Little Elf => Unobtainable item, probably removed due to tasteless reference to a crime against humanity.
			28131, // Reaver Buster Launcher => Seemingly unobtainable item.
			29585, // NPC Equip 29585 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			29805, // Socrethar's Head => TBC Beta quest item, unobtainable.
			29868, // QAEnchant Gloves +26 Attack Power
			29877, // NPC Equip 29877 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			31954, // NPC Equip 31954 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			32424, // NPC Equip 32424 => Wrong item, should be "Blade's Edge Ogre Brew"
			32426, // NPC Equip 32426 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			32669, // NPC Equip 32669 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			32913, // NPC Equip 32913 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			33063, // Really Tough Brewfest Bread
			33090, // NPC Equip 33090 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			33100, // NPC Equip 33100 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			33217, // NPC Equip 33217 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			33218, // Goblin Gumbo
			33219, // Goblin Gumbo Kettle
			33570, // NPC Equip 33570 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			33572, // NPC Equip 33572 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			33573, // NPC Equip 33573 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			33574, // NPC Equip 33574 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			33600, // NPC Equip 33600 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			33601, // NPC Equip 33601 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			33602, // NPC Equip 33602 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			33603, // NPC Equip 33603 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			33623, // NPC Equip 33623 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			33624, // NPC Equip 33624 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			33625, // NPC Equip 33625 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			33626, // NPC Equip 33626 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			33785, // NPC Equip 33785 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			33786, // NPC Equip 33786 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			33787, // NPC Equip 33787 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			33788, // NPC Equip 33788 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			33800, // NPC Equip 33800 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			34046, // NPC Equip 34046 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			34047, // NPC Equip 34047 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			34048, // NPC Equip 34048 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			34071, // NPC Equip 34071 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			34151, // NPC Equip 34151 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			34152, // NPC Equip 34152 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			34153, // NPC Equip 34153 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			34154, // NPC Equip 34154 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			34155, // NPC Equip 34155 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			34156, // NPC Equip 34156 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			34250, // NPC Equip 34250 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			34251, // NPC Equip 34251 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			34252, // NPC Equip 34252 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			35120, // NPC Equip 35120 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			35667, // NPC Equip 35667 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			38165, // NPC Equip 38165 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			38236, // NPC Equip 38236 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			38685, // NPC Equip 38685 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			38702, // NPC Equip 38702 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			38704, // NPC Equip 38704 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			39163, // Test expire transform
			39600, // NPC Equip 39600 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			40308, // NPC Equip 40308 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			42437, // NPC Equip 42437 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			42764, // NPC Equip 42764 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			44624, // NPC Equip 44624 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			44628, // NPC Equip 44628 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			44630, // NPC Equip 44630 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			44804, // NPC Equip 44804 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			44805, // NPC Equip 44805 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			44807, // Indalamar's Holy Hand Grenade
			44813, // NPC Equip 44813 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			44816, // NPC Equip 44816 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			44832, // Squirt Gun [PH]
			44866, // Faithful Mule
			44867, // NPC Equip 44867 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			44964, // NPC Equip 44964 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			45126, // Trusty Mount [PH]
			46717, // NPC Equip 46717 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			46765, // Blue War Fuel (Mountain Dew Promotion)
			46766, // Red War Fuel (Mountain Dew Promotion)
			48416, // NPC Equip 48416 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			48601, // Red Rider Air Rifle Ammo
			49223, // Permission Slip
			49224, // NPC Equip 49224 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			49225, // NPC Equip 49225 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			49349, // NPC Equip 49349 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			49372, // NPC Equip 49372 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			49373, // Permission Slip
			49374, // NPC Equip 49374 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			49692, // NPC Equip 49692 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			50076, // NPC Equip 50076 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			50091, // NPC Equip 50091 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			50092, // NPC Equip 50092 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			50093, // Pet Prep: A Beginner's Guide
			50164, // NPC Equip 50164 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			52202, // Elemental Sapta
			52361, // NPC Equip 52361 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			53476, // NPC Equip 53476 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			53477, // NPC Equip 53477 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			54647, // NPC Equip 54647 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			54822, // Sen'jin Overcloak
		};
	}

	std::unordered_set<uint32_t> getUnwantedItemsGUIDs() const
	{
		return {
			835, // Large Rope Net
			1127, // Flash Bundle
			1176, // Smelling Salts
			1187, // Spiked Collar
			1191, // Bag of Marbles
			1322, // Fishliver Oil
			1434, // Glowing Wax Stick
			1970, // Restoring Balm
			3434, // Slumber Sand
			3456, // Dog Whistle
			4546, // Call of the Raptor
			4598, // Goblin Fishing Pole
			4941, // Really Sticky Glue
			4945, // Faintly Glowing Skull
			4952, // Stormstout
			4953, // Trogg Brew
			5421, // Fiery Blaze Enchantment
			5457, // Severed Voodoo Claw
			5845, // Flank of Meat
			5951, // Moist Towelette
			7516, // Tabetha's Instructions => Bots don't need instructions.
			8410, // R.O.I.D.S.
			8411, // Lung Juice Cocktail
			8412, // Ground Scorpok Assay
			8423, // Cerebral Cortex Compound
			8424, // Gizzard Gum,
			8432, // Eau de Mixilpixil
			10684, // Colossal Parachute
			10830, // M73 Frag Grenade
			12924, // Ritual Candle
			13149, // Eldarathian Tome of Summoning Vol. 1
			13151, // The Mystic Studies of Hor'ank
			13152, // The Story With No Conclusion
			13153, // Tome of Mal'cin Vorail
			13154, // Jael'marin's Studies of the Arcane
			13508, // Eye of Arachnida
			13509, // Clutch of Foresight
			13514, // Wail of the Banshee
			13813, // Blessed Sunfruit Juice
			14894, // Lily Root
			15723, // Tea with Sugar
			15778, // Mechanical Yeti
			17048, // Rumsey Rum
			18209, // Energized Sparkplug
			18269, // Gordok Green Grog
			18284, // Kreeg's Stout Beatdown
			18297, // Thornling Seed
			19060, // Warsong Gulch Enriched Ration
			19061, // Warsong Gulch Iron Ration
			19062, // Warsong Gulch Field Ration
			19318, // Bottled Alterac Spring Water
			19997, // Harvest Nectar
			20062, // Arathi Basin Enriched Ration
			20063, // Arathi Basin Field Ration
			20064, // Arathi Basin Iron Ration
			20079, // Spirit of Zanza
			20080, // Sheen of Zanza
			20081, // Swiftness of Zanza
			20222, // Defiler's Enriched Ration
			20223, // Defiler's Field Ration
			20224, // Defiler's Iron Ration
			20225, // Highlander's Enriched Ration
			20226, // Highlander's Field Ration
			20227, // Highlander's Iron Ration
			20388, // Lollipop
			20389, // Candy Corn
			20390, // Candy Bar
			20397, // Hallowed Wand - Pirate
			20398, // Hallowed Wand - Ninja
			20399, // Hallowed Wand - Leper Gnome
			20409, // Hallowed Wand - Ghost
			20410, // Hallowed Wand - Bat
			20411, // Hallowed Wand - Skeleton
			20413, // Hallowed Wand - Random
			20414, // Hallowed Wand - Wisp
			20557, // Hallow's End Pumpkin Treat
			21212, // Fresh Holly
			21241, // Winter Veil Eggnog
			21325, // Mechanical Greench
			21537, // Festival Dumplings
			21711, // Lunar Festival Invitation
			21744, // Lucky Rocket Cluster
			21745, // Elder's Moonstone
			21815, // Love Token
			22192, // Bloodkelp Elixir of Dodging
			22193, // Bloodkelp Elixir of Resistance
			22236, // Buttermilk Delight
			22237, // Dark Desire
			22238, // Very Berry Cream
			22239, // Sweet Surprise
			22259, // Unbestowed Friendship Bracelet
			22260, // Friendship Bracelet
			22743, // Bloodsail Sash
			22778, // Scourgebane Infusion
			22779, // Scourgebane Draught
			22795, // Fel Blossom
			23161, // Freshly-Squeezed Lemonade
			23194, // Lesser Mark of the Dawn
			23195, // Mark of the Dawn
			23196, // Greater Mark of the Dawn
			23211, // Toasted Smorc
			23215, // Bag of Smorc Ingredients
			23246, // Fiery Festival Brew
			23326, // Midsummer Sausage
			23327, // Fire-toasted Bun
			23379, // Cinder Bracers
			23435, // Elderberry Pie
			23492, // Suntouched Special Reserve
			23584, // Loch Modan Lager
			23585, // Stouthammer Lite
			23586, // Aerie Peak Pale Ale
			23985, // Crystal of Vitality
			23986, // Crystal of Insight
			23989, // Crystal of Ferocity
			24006, // Grunt's Waterskin
			24007, // Footman's Waterskin
			24408, // Edible Stalks
			24421, // Nagrand Cherry
			24429, // Expedition Flare
			24540, // Edible Fern
			25498, // Rough Stone Statue
			25880, // Coarse Stone Statue
			25881, // Heavy Stone Statue
			25882, // Solid Stone Statue
			25883, // Dense Stone Statue
			25884, // Primal Stone Statue
			27317, // Elemental Sapta
			27553, // Crimson Steer Energy Drink
			29482, // Ethereum Essence
			30309, // Stonebreaker Brew
			30499, // Brightstone Wine
			30615, // Halaani Whiskey
			30858, // Peon Sleep Potion
			31337, // Orb of the Blackwhelp
			31449, // Distilled Stalker Sight
			31450, // Sealth of the Stalker
			31451, // Pure Energy
			31535, // Bloodboil Poison
			32563, // Grilled Picnic Treat
			32971, // Water Bucket
			33023, // Savory Sausage
			33024, // Pickled Sausage
			33025, // Spicy Smoked Sausage
			33026, // The Golden Link
			33028, // Barleybrew Light
			33028, // Barleybrew Dark
			33030, // Barleybrew Clear
			33031, // Thunder 45
			33032, // Thunderbrew Ale
			33033, // Thunderbrew Stout
			33034, // Gordok Grog
			33035, // Ogre Mead
			33036, // Mudder's Milk
			33043, // The Essential Brewfest Pretzel
			33226, // Tricky Treat
			33234, // Iced Berry Slush
			33236, // Fizzy Faire Drink "Classic"
			33246, // Funnel Cake
			33254, // Afrazi Forest Strider Drumstick
			33312, // Mana Sapphire
			33929, // Brewfest Brew
			33956, // Harkor's Home Brew
			34017, // Small Step Brew
			34018, // Long Stride Brew
			34019, // Path of Brew
			34020, // Jungle River Water
			34021, // Brewdo Magic
			34022, // Stout Shrunken Head
			34044, // B-Ball
			34063, // Dried Sausage
			34064, // Succulent Sausage
			34065, // Spiced Onion Cheese
			34068, // Weighted Jack-o'-Lantern
			34077, // Crudely Wrapped Gift
			34410, // Honeyed Holiday Ham
			34412, // Sparkling Apple Cider
			34599, // Juggling Torch
			35223, // Papa Hummel's Old-Fashioned Pet Biscuit
			35946, // Fizzcrank Practice Parachute
			36877, // Folded Letter
			37431, // Fetch Ball
			37488, // Wild Winter Pilsner
			37489, // Izzard's Ever Flavor
			37490, // Aromatic Honey Brew
			37491, // Metok's Bubble Bock
			37492, // Springtime Stout
			37493, // Blackrock Lager
			37494, // Stranglethorn Brew
			37495, // Draenic Pale Ale
			37496, // Binary Brew
			37497, // Autumnal Acorn Ale
			37498, // Bartlett's Bitter Brew
			37499, // Lord of Frost's Private Label
			37582, // Pyroblast Cinnamon Ball
			37583, // G.N.E.R.D.S.
			37584, // Soothing Spearmint Candy
			37585, // Chewy Fel Taffy
			37604, // Tooth Pick
			37750, // Fresh Brewfest Hops
			37863, // Direbrew's Remote
			37878, // Worg's Blood Elixir
			37898, // Wild Winter Pilsner
			37899, // Izzard's Ever Flavor
			37900, // Aromatic Honey Brew
			37901, // Metok's Bubble Bock
			37902, // Springtime Stout
			37903, // Blackrock Lager
			37904, // Stranglethorn Brew
			37905, // Draenic Pale Ale
			37906, // Binary Brew
			37907, // Autumnal Acorn Ale
			37908, // Bartlett's Bitter Brew
			37909, // Lord of Frost's Private Label
			37925, // Experimental Mixture
			38291, // Ethereal Mutagen
			38294, // Ethereal Liqueur
			38300, // Diluted Ethereum Essence
			38320, // Dire Brew
			38518, // Cro's Apple
			38577, // Party G.R.E.N.A.D.E.
			38587, // Empty Brewfest Stein
			38626, // Empty Brew Bottle
			39476, // Fresh Goblin Brewfest Hops
			39477, // Fresh Dwarven Brewfest Hops
			39738, // Thunderbrew's Hard Ale
			42342, // Bag of Popcorn
			42350, // Bag of Peanuts
			42381, // Anguish Ale
			42436, // Chocolate Celebration Cake
			42438, // Lovely Cake
			42439, // Big Berry Pie
			43088, // Dalaran Apple Bowl
			43135, // Fate Rune of Fleet Feet
			43352, // Pet Grooming Kit
			43462, // Airy Pale Ale
			43470, // Worg Tooth Oatmeal Stout
			43471, // Rork Red Ribbon
			43472, // Snowfall Lager
			43473, // Drakefire Chile Ale
			43489, // Mohawk Grenade
			43626, // Happy Pet Snack
			44064, // Nepeta Leaf
			44065, // Oracle Secret Solution
			44435, // Windle's Lighter
			44481, // Grindgear Toy Gorilla
			44482, // Trusty Copper Racer
			44599, // Zippy Copper Racer
			44601, // Heavy Copper Racer
			44610, // Fresh Dalaran Bread
			44612, // Dalaran Swiss Wheel
			44613, // Aged Dalaran Sharp Wheel
			44621, // Bottle of Dalaran White
			44622, // Cask of Dalaran White
			44623, // Bottle of Dalaran Red
			44625, // Bottle of Aged Dalaran Red
			44626, // Cask of Aged Dalaran Red
			44627, // Bottle of Peaked Dalaran Red
			44629, // Cask of Peaked Dalaran Red
			44632, // Cask of Dalaran Red
			44698, // Intravenous Healing Potion
			44716, // Mysterious Fermented Liquid
			44792, // Blossoming Branch
			44812, // Turkey Shooter
			44817, // The Mischief Maker
			44818, // Noblegarden Egg
			44844, // Turkey Caller
			44849, // Tiny Green Ragdoll
			44943, // Icy Prism
			45047, // Sandbox Tiger
			46319, // Tournament Brew
			46399, // Thunder's Plunder
			46400, // Barleybrew Gold
			46401, // Crimson Stripe
			46402, // Promise of the Pandaren
			46403, // Chuganpug's Delight
			46711, // Spirit Candle
			46718, // Orange Marigold
			46725, // Red Rider Air Rifle
			46783, // Pink Gumball
			49288, // Little Ivory Raptor Whistle
			49289, // Little White Stallion Bridle
			49856, // "VICTORY" Perfume
			49857, // "Enchantress" Perfume
			49858, // "Forever" Perfume
			49859, // "Bravado" Cologne
			49860, // "Wizardry" Cologne
			49861, // "STALWART" Cologne
			49936, // Lovely Stormwind Card
			49937, // Lovely Undercity Card
			49938, // Lovely Darnassus Card
			49939, // Lovely Orgrimmar Card
			49940, // Lovely Ironforge Card
			49941, // Lovely Thunder Bluff Card
			49942, // Lovely Exodar Card
			49943, // Lovely Silvermoon City Card
			50163, // Lovely Rose
			54455, // Paint Bomb
		};
	}

	std::unordered_set<uint32_t> getQuestConsumableItemsGUIDs() const
	{
		return {
			1262, // Keg of Thunderbrew Lager
			5880, // Crate With Holes
			6074, // War Horn Mouthpiece
			6464, // Wailing Essence
			6486, // Singed Scale
			6635, // Earth Sapta
			6636, // Fire Sapta
			6637, // Water Sapta
			6781, // Bartleby's Mug
			6782, // Marshal Haggard's Badge
			6812, // Case of Elunite
			6841, // Vial of Phlogiston
			6842, // Furen's Instructions
			6851, // Essence of the Exile
			6926, // Furen's Notes
			6929, // Bath'rah's Parchment
			7127, // Powdered Azurite
			7227, // Balnir Snapdragons
			7266, // Ur's Treatise on Shadow Magic
			7627, // Dolanaar Delivery
			7629, // Ukor's Burden
			7970, // E.C.A.C.
			8048, // Emerald Dreamcatcher
			8095, // Hinott's Oil
			8548, // Divino-matic Rod
			9546, // Simple Scroll
			9569, // Hallowed Scroll
			10621, // Runed Scroll
			10663, // Essence of Hakkar
			10687, // Empty Vial Labeled #1
			10688, // Empty Vial Labeled #2
			10689, // Empty Vial Labeled #3
			10690, // Empty Vial Labeled #4
			11148, // Samophlange Manual Page
			11405, // Giant Silver Vein
			11413, // Nagmara's Filled Vial
			11914, // Empty Cursed Ooze Jar
			11948, // Empty Tainted Ooze Jar
			11953, // Empty Pure Sample Jar
			12533, // Roughshod Pike
			12650, // Attuned Dampener
			12712, // Warosh's Mojo
			12884, // Arnak's Hoof
			12885, // Pamela's Doll
			12886, // Pamela's Doll's Head
			12894, // Joseph's Wedding Ring
			12922, // Empty Canteen
			13155, // Resonating Skull
			13156, // Mystic Crystal
			13562, // Remains of Trey Lightforge
			13703, // Kodo Bone
			13761, // Frozen Eggs
			14542, // Kravel's Crate
			15314, // Bundle of Relics
			16282, // Bundle of Hides
			20470, // Solanian's Scrying Orb
			20471, // Scroll of Scourge Magic
			20472, // Solanian's Journal
			23361, // Cleansing Vial
			23417, // Sanctified Crystal
			23645, // Seer's Relic
			24330, // Drain Schematics
			24355, // Ironvine Seeds
			24407, // Uncatalogued Species
			24474, // Violet Scrying Crystal
			25465, // Stormcrow Amulet
			28038, // Seaforium PU-36 Explosive Nether Modulator
			28132, // Area 52 Special
			28513, // Demonic Rune Stone
			28554, // Shredder Spare Parts
			28571, // Blank Scroll
			28580, // B'naar Console Transcription
			28607, // Sunfury Disguise
			28635, // Sunfury Arcanist Robes
			28636, // Sunfury Researcher Gloves
			28637, // Sunfury Guardsman Medallion
			28784, // Unyielding Banner Scrap => Maybe unused quest item?
			29162, // Galaxis Soul Shard
			29324, // Warp-Attuned Orb
			29443, // Bloodmaul Brutebane Brew
			29624, // First Half of Socrethar's Stone
			29625, // Second Half of Socrethar's Stone
			29699, // Socrethar's Teleportation Stone
			29778, // Phase Disruptor
			29796, // Socrethar's Teleportation Stone
			29905, // Kael's Vial Remnant
			29906, // Vashj's Vial Remnant
			30260, // Voren'thal's Package
			30540, // Tally's Waiver (Unsigned)
			30811, // Scroll of Demonic Unbanishing
			31121, // Costume Scraps
			31122, // Overseer Disguise
			31495, // Grishnath Orb
			31517, // Dire Pinfeather
			31518, // Exorcism Feather
			31702, // Challenge from the Horde
			31795, // Draenei Prayer Beads
			32406, // Skyguard Blasting Charges
			32602, // Crystalforged Darkrune
			32848, // Explosives Package
			33079, // Murloc Costume
			33099, // Intact Plague Container
			33277, // Tome of Thomas Thomson
			33349, // Plague Vials
			33614, // Empty Apothecary's Flask => Possibly unaccessible to players but still linked to a quest.
			33615, // Flask of Vrykul Blood => Possibly unaccessible to players but still linked to a quest.
			33616, // Unstable Mix => Possibly unaccessible to players but still linked to a quest.
			33617, // Balanced Concoction => Possibly unaccessible to players but still linked to a quest.
			33619, // Lysander's Strain
			33621, // Plague Spray
			33797, // Portable Brewfest Keg
			34023, // Empty Apothecary's Flask
			34024, // Flask of Vrykul Blood
			34076, // Fish Bladder
			34475, // Arcane Charges
			35704, // Incendiary Explosives
			37173, // Geomancer's Orb
			37198, // Prototype Neural Needler
			37265, // Tua'kea's Breathing Bladder
			37661, // Gossamer Potion
			37877, // Silver Feather
			38629, // Orders from the Lich King
			38657, // Freya's Ward
			39698, // Light-infused Artifact
			40390, // Vic's Emergency Air Tank
			40482, // Dual-plagued Brain
			40725, // Steam-Powered Auctioneer
			44576, // Bright Flare
			44806, // Brightly Colored Shell Fragment
			45784, // Thorim's Sigil
			45786, // Hodir's Sigil
			45787, // Mimiron's Sigil
			45788, // Freya's Sigil
			45791, // Sigils of the Watchers
			45814, // Freya's Sigil
			45815, // Hodir's Sigil
			45816, // Mimiron's Sigil
			45817, // Thorim's Sigil
			45855, // Sigils of the Watchers
		};
	}

	std::unordered_set<uint32_t> getRoguePoisonsItemsGUIDs() const
	{
		return {
			2892, // Deadly Poison
			2893, // Deadly Poison II
			3775, // Crippling Poison
			3776, // Crippling Poison II
			5237, // Mind-numbing Poison
			6947, // Instant Poison
			6949, // Instant Poison II
			6950, // Instant Poison III
			6951, // Mind-numbing Poison II => Useless in WotLK but useful in earlier expansions, kept for IP use.
			8926, // Instant Poison IV
			8927, // Instant Poison V
			8928, // Instant Poison VI
			8984, // Deadly Poison III
			8985, // Deadly Poison IV
			9186, // Mind-numbing Poison III => Useless in WotLK but useful in earlier expansions, kept for IP use.
			10918, // Wound Poison
			10920, // Wound Poison II
			10921, // Wound Poison III
			10922, // Wound Poison IV
			20844, // Deadly Poison V
			21835, // Anesthetic Poison
			21927, // Instant Poison VII
			22053, // Deadly Poison VI
			22054, // Deadly Poison VII
			22055, // Wound Poison V
		};
	}

	std::unordered_set<uint32_t> getUsefulConsumableItemsGUIDs() const
	{
		return {
			18606, // Alliance Battle Standard
			18607, // Horde Battle Standard
			19045, // Stormpike Battle Standard
			19046, // Frostwolf Battle Standard
			19150, // Sentinel Basic Care Package
			19151, // Sentinel Standard Care Package
			19152, // Sentinel Advanced Care Package
			19153, // Outrider Advanced Care Package
			19154, // Outrider Basic Care Package
			19155, // Outrider Standard Care Package
			19182, // Darkmoon Faire Prize Ticket
			19425, // Mysterious Lockbox
			20228, // Defiler's Advanced Care Package
			20229, // Defiler's Basic Care Package
			20230, // Defiler's Standard Care Package
			20231, // Arathor Advanced Care Package
			20233, // Arathor Basic Care Package
			20236, // Arathor Standard Care Package
			21740, // Small Rocket Recipes
			21741, // Cluster Rocket Recipes
			21742, // Large Rocket Recipes
			21743, // Large Cluster Rocket Recipes
			21746, // Lucky Red Envelope
			24140, // Blackened Urn => Useless in WotLK but useful in earlier expansions, kept for IP use.
			24289, // Chrono-beacon
			24494, // Tears of the Goddess
			24520, // Honor Hold Favor
			24522, // Thrallmar Favor
			24579, // Mark of Honor Hold
			24581, // Mark of Thrallmar
			27388, // Mr. Pinchy
			29735, // Holy Dust
			30690, // Power Converter
			32408, // Naj'entus Spine
			32542, // Imp in a Ball
			33865, // Amani Hex Stick
			33926, // Sealed Scroll Case
			33927, // Brewfest Pony Keg
			33928, // Hollowed Bone Decanter
			34583, // Aldor Supplies Package
			34584, // Scryer Supplies Package
			34585, // Scryer Supplies Package
			34587, // Aldor Supplies Package
			34592, // Aldor Supplies Package
			34593, // Scryer Supplies Package
			34594, // Scryer Supplies Package
			34595, // Aldor Supplies Package
			35232, // Shattered Sun Supplies
			34686, // Brazier of Dancing Flames
			34850, // Midsummer Ground Flower
			35512, // Pocket Full of Snow
			35945, // Brilliant Glass
			36748, // Dark Brewmaiden's Brew
			36862, // Worn Troll Dice
			36863, // Decahedral Dwarven Dice
			37815, // Emerald Essence
			37859, // Amber Essence
			37860, // Ruby Essence
			38186, // Ethereal Credit
			38233, // Path of Illidan
			39878, // Mysterious Egg
			39883, // Cracked Egg
			40110, // Haunted Memento
			41426, // Magically Wrapped Gift
			44113, // Small Spice Bag
			44606, // Toy Train Set
			44717, // Disgusting Jar
			44718, // Ripe Disgusting Jar
			44751, // Hyldnir Spoils
			45011, // Stormwind Banner
			45013, // Thunder Bluff Banner
			45014, // Orgrimmar Banner
			45015, // Sen'jin Banner
			45016, // Undercity Banner
			45017, // Silvermoon City Banner
			45018, // Ironforge Banner
			45019, // Gnomeregan Banner
			45020, // Exodar Banner
			45021, // Darnassus Banner
			45038, // Fragment of Val'anyr
			45039, // Shattered Fragments of Val'anyr
			45057, // Wind-Up Train Wrecker
			45063, // Foam Sword Rack
			45506, // Archivum Data Disc
			45705, // Argent Tournament Invitation
			45798, // Heroic Celestial Planetarium Key
			45857, // Archivum Data Disc
			45896, // Unbound Fragments of Val'anyr
			45897, // Reforged Hammer of Ancient Kings
			46029, // Magnetic Core
			46779, // Path of Cenarius
			46780, // Ogre Pinata
			46843, // Argent Crusader's Banner
			46847, // Seaforium Bombs
			47030, // Huge Seaforium Bombs
			47541, // Argent Pony Bridle
			49631, // Standard Apothecary Serving Kit
			50307, // Infernal Spear
			50471, // The Heartbreaker
			54212, // Instant Statue Pedestal
			54437, // Tiny Green Ragdoll
			54438, // Tiny Blue Ragdoll
			54651, // Gnomeregan Pride
			54653, // Darkspear Pride
		};
	}

	std::unordered_set<uint32_t> getWarlockItemsGUIDs() const
	{
		return {
			5232, // Minor Soulstone
			16892, // Lesser Soulstone
			16893, // Soulstone
			16895, // Greater Soulstone
			16896, // Major Soulstone
			22116, // Master Soulstone
			36895, // Demonic Soulstone
		};
	}

	std::unordered_set<uint32_t> getMageItemsGUIDs() const
	{
		return {
			5513, // Mana Jade
			8007, // Mana Citrine
			8008, // Mana Ruby
			22044, // Mana Emerald
			36799, // Mana Gem
		};
	}
};
