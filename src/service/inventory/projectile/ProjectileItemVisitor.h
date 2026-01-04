/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#pragma once

#include "ItemTemplate.h"
#include "AbstractItemVisitor.h"
#include "Player.h"
#include "SharedDefines.h"

class ProjectileItemVisitor : public AbstractItemVisitor
{
public:
	bool process(Player* player, Item* item) override
	{
		if (player == nullptr)
			return false;

		if (item == nullptr)
			return false;

		if (player->getClass() == CLASS_HUNTER)
			return true;

		// @TODO: Implement logic on projectile processing.
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

		if (itemTemplate->Class != ITEM_CLASS_PROJECTILE)
		{
			LOG_ERROR("playerbots", "Item {} was not of class PROJECTILE ({}), found {}", itemTemplate->ItemId, std::to_string(ITEM_CLASS_ARMOR), std::to_string(itemTemplate->Class));

			return false;
		}

		if (this->process(player, item))
		{
			LOG_ERROR("playerbots", "Item {} was a useful PROJECTILE item", itemTemplate->ItemId);

			return true;
		}

		this->magicSell(player, item);

		return true;
	}
};
