/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * /modules/playerbots/src/strategy/actions/player/inventoryand/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "ManageInventoryAction.h"

#include <cstdint>
#include <sstream>
#include <string>
#include <unordered_map>

#include "Log.h"
#include "Player.h"
#include "ChatHelper.h"
#include "ItemTemplate.h"

#include "GlobalPlayerInspector.h"
#include "GlobalItemInspector.h"
#include "PlayerbotAI.h"
#include "Event.h"
#include "RandomPlayerbotMgr.h"
#include "ItemActionStruct.h"

using InspectorFactory = std::function<ItemActionStruct(const uint32_t botGUID, const uint64_t itemGUID)>;
using SubclassMap = std::unordered_map<uint32_t, InspectorFactory>;
using ClassMap = std::unordered_map<uint32_t, SubclassMap>;

static const ClassMap inspectorFactories = {
	{
		ITEM_CLASS_ARMOR,
		{
			{
				ManageInventoryAction::ANY_SUBCLASS,
				[](const uint32_t botGUID, const uint64_t itemGUID)
				{
					return ArmorItemInspector(botGUID, itemGUID).determineItemAction();
				}
			}
		}
	},
	{
		ITEM_CLASS_CONSUMABLE,
		{
			{
				{
					ITEM_SUBCLASS_CONSUMABLE,
					[](const uint32_t botGUID, const uint64_t itemGUID)
					{
						return ConsumableConsumableInspector(botGUID, itemGUID).determineItemAction();
					}
				},
				{
					ITEM_SUBCLASS_ELIXIR,
					[](const uint32_t botGUID, const uint64_t itemGUID)
					{
						return ConsumableElixirInspector(botGUID, itemGUID).determineItemAction();
					}
				},
				{
					ITEM_SUBCLASS_POTION,
					[](const uint32_t botGUID, const uint64_t itemGUID)
					{
						return ConsumablePotionInspector(botGUID, itemGUID).determineItemAction();
					}
				}
			}
		}
	},
	{
		ITEM_CLASS_CONTAINER,
		{
			{
				ManageInventoryAction::ANY_SUBCLASS,
				[](const uint32_t botGUID, const uint64_t itemGUID)
				{
					return ContainerInspector(botGUID, itemGUID).determineItemAction();
				}
			}
		}
	},
	{
		ITEM_CLASS_GEM,
		{
			{
				ManageInventoryAction::ANY_SUBCLASS,
				[](const uint32_t botGUID, const uint64_t itemGUID)
				{
					return GemInspector(botGUID, itemGUID).determineItemAction();
				}
			}
		}
	},
	{
		ITEM_CLASS_GENERIC,
		{
			{
				ManageInventoryAction::ANY_SUBCLASS,
				[](const uint32_t botGUID, const uint64_t itemGUID)
				{
					return GenericItemInspector(botGUID, itemGUID).determineItemAction();
				}
			}
		}
	},
	{
		ITEM_CLASS_GLYPH,
		{
			{
				ManageInventoryAction::ANY_SUBCLASS,
				[](const uint32_t botGUID, const uint64_t itemGUID)
				{
					return GlyphInspector(botGUID, itemGUID).determineItemAction();
				}
			}
		}
	},
	{
		ITEM_CLASS_KEY,
		{
			{
				ManageInventoryAction::ANY_SUBCLASS,
				[](const uint32_t botGUID, const uint64_t itemGUID)
				{
					return KeyInspector(botGUID, itemGUID).determineItemAction();
				}
			}
		}
	},
	{
		ITEM_CLASS_MISC,
		{
			{
				ManageInventoryAction::ANY_SUBCLASS,
				[](const uint32_t botGUID, const uint64_t itemGUID)
				{
					return MiscellaneousItemInspector(botGUID, itemGUID).determineItemAction();
				}
			}
		}
	},
	{
		ITEM_CLASS_MONEY,
		{
			{
				ManageInventoryAction::ANY_SUBCLASS,
				[](const uint32_t botGUID, const uint64_t itemGUID)
				{
					return MoneyInspector(botGUID, itemGUID).determineItemAction();
				}
			}
		}
	},
	{
		ITEM_CLASS_PERMANENT,
		{
			{
				ManageInventoryAction::ANY_SUBCLASS,
				[](const uint32_t botGUID, const uint64_t itemGUID)
				{
					return PermanentItemInspector(botGUID, itemGUID).determineItemAction();
				}
			}
		}
	},
	{
		ITEM_CLASS_PROJECTILE,
		{
			{
				ManageInventoryAction::ANY_SUBCLASS,
				[](const uint32_t botGUID, const uint64_t itemGUID)
				{
					return ProjectItemInspector(botGUID, itemGUID).determineItemAction();
				}
			}
		}
	},
	{
		ITEM_CLASS_QUEST,
		{
			{
				ManageInventoryAction::ANY_SUBCLASS,
				[](const uint32_t botGUID, const uint64_t itemGUID)
				{
					return QuestItemInspector(botGUID, itemGUID).determineItemAction();
				}
			}
		}
	},
	{
		ITEM_CLASS_QUIVER,
		{
			{
				ManageInventoryAction::ANY_SUBCLASS,
				[](const uint32_t botGUID, const uint64_t itemGUID)
				{
					return QuiverItemInspector(botGUID, itemGUID).determineItemAction();
				}
			}
		}
	},
	{
		ITEM_CLASS_REAGENT,
		{
			{
				ManageInventoryAction::ANY_SUBCLASS,
				[](const uint32_t botGUID, const uint64_t itemGUID)
				{
					return ReagentItemInspector(botGUID, itemGUID).determineItemAction();
				}
			}
		}
	},
	{
		ITEM_CLASS_RECIPE,
		{
			{
				ManageInventoryAction::ANY_SUBCLASS,
				[](const uint32_t botGUID, const uint64_t itemGUID)
				{
					return RecipeItemInspector(botGUID, itemGUID).determineItemAction();
				}
			}
		}
	},
	{
		ITEM_CLASS_TRADE_GOODS,
		{
			{
				ManageInventoryAction::ANY_SUBCLASS,
				[](const uint32_t botGUID, const uint64_t itemGUID)
				{
					return TradeGoodItemInspector(botGUID, itemGUID).determineItemAction();
				}
			}
		}
	},
	{
		ITEM_CLASS_WEAPON,
		{
			{
				ManageInventoryAction::ANY_SUBCLASS,
				[](const uint32_t botGUID, const uint64_t itemGUID)
				{
					return WeaponItemInspector(botGUID, itemGUID).determineItemAction();
				}
			}
		}
	},
};

bool ManageInventoryAction::Execute(Event event)
{
	LOG_ERROR("playerbots", "starting inventory management action");

	std::unordered_map<std::string, ItemActionStruct> itemActions = {};

	if (this->bot == nullptr)
	{
		LOG_ERROR("playerbots", "ManageInventoryAction::Execute bot is nullptr");

		return false;
	}

	const std::string botName = this->bot->GetName();

	LOG_ERROR("playerbots", "managing inventory for {}", botName);

	if (!RandomPlayerbotMgr::instance()->IsRandomBot(bot))
		return true;

	this->iterateBags();

	LOG_ERROR("playerbots", "done processing bags for {}", botName);

	if (this->botAI != nullptr)
	{
		const Player* const master = botAI->GetMaster();

		LOG_ERROR("playerbots", "fetched master for {}", botName);

		if (master != nullptr)
		{
			const std::string masterName = master->GetName();

			LOG_ERROR("playerbots", "telling master ({}) of bot {}", masterName, botName);

			std::ostringstream out;

			out << "I am done sorting my inventory.";

			if (this->botAI != nullptr)
			{
				this->botAI->TellMaster(out);
			}

			LOG_ERROR("playerbots", "told master ({}) of bot {}", masterName, botName);
		}
	}

	for (const auto& [itemGUID, action] : this->itemActions)
	{

		std::string actionName = "UNKNOWN_ACTION";

		switch (action.action)
		{
			case ItemActionEnum::NONE:
				actionName = "ITEM_ACTION_NONE";
				break;
			case ItemActionEnum::EQUIP:
				actionName = "ITEM_ACTION_EQUIP";
				break;
			case ItemActionEnum::SELL:
				actionName = "ITEM_ACTION_SELL";
				break;
			case ItemActionEnum::DESTROY:
				actionName = "ITEM_ACTION_DESTROY";
				break;
		}

		LOG_ERROR("playerbots", "Item {} action: {}, inventorySlot: {}, equipmentSlot: {}", std::to_string(itemGUID), actionName, std::to_string(action.inventorySlot), std::to_string(action.equipmentSlot));
	}

	LOG_ERROR("playerbots", "Done processing inventory");

	return true;
	// if (this->bot != nullptr)
	// {
	// 	bot->SaveToDB(false, false);

	// 	return true;
	// }

	// LOG_ERROR("playerbots", "ManageInventoryAction::Execute can't save, bot is nullptr");

	// return false;
}

void ManageInventoryAction::iterateBags()
{
	LOG_ERROR("playerbots", "Processing main bag");

	if (this->bot == nullptr)
	{
		LOG_ERROR("playerbots", "ManageInventoryAction::iterateBags bot became nullptr");

		return;
	}

	const GlobalPlayerInspector playerInspector(this->bot->GetGUID().GetRawValue());

    for (uint8_t slot = INVENTORY_SLOT_ITEM_START; slot < INVENTORY_SLOT_ITEM_END; slot++)
    {
		LOG_ERROR("playerbots", "Processing item in main bag slot {}", std::to_string(slot));

		Item* item = playerInspector.getItemByPosition(INVENTORY_SLOT_BAG_0, slot);

		if (item == nullptr)
		{
			LOG_ERROR("playerbots", "Item in main bag slot {} did not exist", std::to_string(slot));

			continue;
		}

		const uint64_t itemLowGUID = item->GetGUID().GetRawValue();

		this->processItem(itemLowGUID);
    }

    for (uint8_t slot = INVENTORY_SLOT_BAG_START; slot < INVENTORY_SLOT_BAG_END; ++slot)
    {
		LOG_ERROR("playerbots", "Processing bag in slot {}", std::to_string(slot));

		if (this->bot == nullptr)
		{
			LOG_ERROR("playerbots", "ManageInventoryAction::iterateBags bot became nullptr");

			return;
		}

		const Item* const possibleBag = playerInspector.getItemByPosition(INVENTORY_SLOT_BAG_0, slot);

		if (possibleBag == nullptr)
			continue;

		const uint64_t bagLowGUID = possibleBag->GetGUID().GetRawValue();

		this->iterateBag(bagLowGUID);
    }

	if (this->bot == nullptr)
	{
		LOG_ERROR("playerbots", "ManageInventoryAction::iterateBags bot became nullptr");

		return;
	}

	LOG_ERROR("playerbots", "done processing bags for {}", this->bot->GetName());
}

void ManageInventoryAction::iterateBag(const uint64_t bagGUID)
{
	const GlobalPlayerInspector playerInspector(this->bot->GetGUID().GetRawValue());
	const Item* const possibleBag = playerInspector.getItemByGUID(bagGUID);

	if (possibleBag == nullptr)
		return;

	const Bag* bag = possibleBag->ToBag();

	if (bag == nullptr)
	{
		LOG_ERROR("playerbots", "Bag did not exist");

		return;
	}

	const uint8_t size = bag->GetBagSize();

	for (uint8_t slot = 0; slot < size; ++slot)
	{
		LOG_ERROR("playerbots", "Processing item {} in bag", std::to_string(slot));

		if (bag == nullptr)
		{
			LOG_ERROR("playerbots", "ManageInventoryAction::iterateBag bag became nullptr");

			return;
		}

		Item* item = bag->GetItemByPos(slot);

		if (item == nullptr)
		{
			LOG_ERROR("playerbots", "Item {} in bag did not exist", std::to_string(slot));

			continue;
		}

		const uint64_t itemGUID = item->GetGUID().GetRawValue();

		this->processItem(itemGUID);
	}

	LOG_ERROR("playerbots", "done processing bag {}", std::to_string(bag->GetSlot()));
}

void ManageInventoryAction::processItem(const uint64_t itemGUID)
{
	if (this->bot == nullptr)
	{
		LOG_ERROR("playerbots", "ManageInventoryAction::processItem bot became nullptr");

		return;
	}

	const uint32_t botGUID = this->bot->GetGUID().GetRawValue();
	const GlobalPlayerInspector playerInspector(botGUID);
	const GlobalItemInspector itemInspector(botGUID, itemGUID);
	const ItemUpdateState itemState = itemInspector.getItemUpdateState();
	const uint32_t itemTemplateLowGUID = itemInspector.getCurrentItemTemplateLowGUID();

	if (itemState == ITEM_REMOVED)
	{
		LOG_ERROR("playerbots", "Item {} (template {}) was already flagged for removal.", itemGUID, itemTemplateLowGUID);

		return;
	}

	if (itemState == ITEM_CHANGED)
	{
		LOG_ERROR("playerbots", "Item {} (template {}) was already modified.", itemGUID, itemTemplateLowGUID);

		return;
	}

	if (!itemInspector.itemIsInWorld())
	{
		LOG_ERROR("playerbots", "Item {} (template {}) is not in world.", itemGUID, itemTemplateLowGUID);

		return;
	}

	const uint8_t itemSlot = itemInspector.getItemSlot();

	if (itemSlot == NULL_SLOT)
	{
		LOG_ERROR("playerbots", "Item {} (template {}) is null slot.", itemGUID, itemTemplateLowGUID);

		return;
	}

	if (itemInspector.itemIsInUnsafeContainer())
	{
		LOG_ERROR("playerbots", "Item {} (template {}) is in unsafe container", itemGUID, itemTemplateLowGUID);

		return;
	}

	const uint32_t itemClass = itemInspector.getCurrentItemClass();
	const uint32_t itemSubClass = itemInspector.getCurrentItemSubclass();

	LOG_ERROR("playerbots", "processing item {}", std::to_string(itemTemplateLowGUID));

	ClassMap::const_iterator classIterator = inspectorFactories.find(itemClass);

	if (classIterator == inspectorFactories.end())
	{
		LOG_ERROR("playerbots", "Unable to locate service for item class {} item service", std::to_string(itemClass));

		return;
	}

	const SubclassMap& subclassMap = classIterator->second;

	SubclassMap::const_iterator subclassIterator = subclassMap.find(itemSubClass);

	if (subclassIterator == subclassMap.end())
	{
		subclassIterator = subclassMap.find(ManageInventoryAction::ANY_SUBCLASS);
	}

	if (subclassIterator == subclassMap.end())
	{
		LOG_ERROR("playerbots", "Unable to locate service for item class {} subclass {} item service", std::to_string(itemClass), std::to_string(itemSubClass));

		return;
	}

	const InspectorFactory& factoryFunction = subclassIterator->second;

	ItemActionStruct action = factoryFunction(botGUID, itemGUID);

	LOG_ERROR("playerbots", "Done visiting item {}", std::to_string(itemTemplateLowGUID));
}

template <typename InspectorT>
void ManageInventoryAction::determineItemAction(const uint32_t botLowGUID, const uint64_t itemLowGUID)
{
	InspectorT inspector(botLowGUID, itemLowGUID);
	const ItemActionStruct action = inspector.determineItemAction();
	this->itemActions.insert({itemLowGUID, action});
}
