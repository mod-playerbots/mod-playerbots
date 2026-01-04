/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#pragma once

#include <string>
#include "ItemTemplate.h"
#include "AbstractItemVisitor.h"
#include "Player.h"

/**
 * @brief Handles the inventory management of generic items (class 8).
 *
 * @remark There is only one 1 generic item in the game (6987 - Fish Scale) and it never made it live.
 * This category of item is thus always useless.
 *
 */
class GenericItemVisitor : public AbstractItemVisitor
{
public:
	bool process(Player* player, Item* item) override
	{
		if (player == nullptr)
			return false;

		if (player == nullptr)
			return false;

		return false;
	}

	bool visit(Player* player, Item* item) override
	{
		if (player == nullptr)
			return false;

		const ItemTemplate* itemTemplate = item->GetTemplate();

		if (itemTemplate == nullptr)
		{
			LOG_ERROR("playerbots", "Item {} had nullptr template", std::to_string(item->GetGUID().GetRawValue()));

			return false;
		}

		if (itemTemplate->Class != ITEM_CLASS_GENERIC)
		{
			LOG_ERROR("playerbots", "Item {} was not of class GENERIC ({}), found {}", itemTemplate->ItemId, std::to_string(ITEM_CLASS_ARMOR), std::to_string(itemTemplate->Class));

			return false;
		}

		LOG_ERROR("playerbots", "Player (bot) {} (#{}) had item of impossible category GENERIC (8) #{}",
			player->GetName(), std::to_string(player->GetGUID().GetEntry()), std::to_string(item->GetTemplate()->ItemId));

		this->destroy(player, item);

		return true;
	}
};
