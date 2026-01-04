/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#pragma once

#include <vector>
#include "ItemTemplate.h"
#include "Player.h"

class InventoryService
{
public:

	static InventoryService& GetInstance()
	{
		static InventoryService instance;

		return instance;
	}

	InventoryService(const InventoryService&) = delete;
	InventoryService& operator=(const InventoryService&) = delete;

	std::vector<EquipmentSlots> getItemEquipmentSlots(const ItemTemplate* itemTemplate) const
	{
		if (itemTemplate == nullptr)
			return {};

		switch (itemTemplate->InventoryType)
		{
			case INVTYPE_HEAD:
				return { EQUIPMENT_SLOT_HEAD };

			case INVTYPE_NECK:
				return { EQUIPMENT_SLOT_NECK };

			case INVTYPE_SHOULDERS:
				return { EQUIPMENT_SLOT_SHOULDERS };

			case INVTYPE_BODY:
				return { EQUIPMENT_SLOT_BODY };

			case INVTYPE_ROBE:
			case INVTYPE_CHEST:
				return { EQUIPMENT_SLOT_CHEST };

			case INVTYPE_WAIST:
				return { EQUIPMENT_SLOT_WAIST };

			case INVTYPE_LEGS:
				return { EQUIPMENT_SLOT_LEGS };

			case INVTYPE_FEET:
				return { EQUIPMENT_SLOT_FEET };

			case INVTYPE_WRISTS:
				return { EQUIPMENT_SLOT_WRISTS };

			case INVTYPE_HANDS:
				return { EQUIPMENT_SLOT_HANDS };

			case INVTYPE_FINGER:
				return { EQUIPMENT_SLOT_FINGER1, EQUIPMENT_SLOT_FINGER2 };

			case INVTYPE_TRINKET:
				return { EQUIPMENT_SLOT_TRINKET1, EQUIPMENT_SLOT_TRINKET2 };

			case INVTYPE_WEAPON:
				return { EQUIPMENT_SLOT_MAINHAND, EQUIPMENT_SLOT_OFFHAND };

			case INVTYPE_WEAPONOFFHAND:
			case INVTYPE_SHIELD:
				return { EQUIPMENT_SLOT_OFFHAND };

			case INVTYPE_RANGED:
			case INVTYPE_THROWN:
			case INVTYPE_RELIC:
				return { EQUIPMENT_SLOT_RANGED };

			case INVTYPE_CLOAK:
				return { EQUIPMENT_SLOT_BACK };

			case INVTYPE_WEAPONMAINHAND:
			case INVTYPE_2HWEAPON:
				return { EQUIPMENT_SLOT_MAINHAND };

			case INVTYPE_TABARD:
				return { EQUIPMENT_SLOT_TABARD };

			case INVTYPE_BAG:
			case INVTYPE_HOLDABLE:
			case INVTYPE_AMMO:
			case INVTYPE_RANGEDRIGHT:
			case INVTYPE_QUIVER:
			default:
				return {};
		}
	}
private:
    InventoryService() = default;
    ~InventoryService() = default;
};
