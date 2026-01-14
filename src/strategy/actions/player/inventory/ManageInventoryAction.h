/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#pragma once

#include <string>
#include <memory>
#include <unordered_map>

#include "Bag.h"
#include "ItemTemplate.h"

#include "Action.h"
#include "Log.h"
#include "PlayerbotAI.h"
#include "ItemActionStruct.h"
#include "AbstractItemInspector.h"
#include "ArmorItemInspector.h"
#include "ConsumableConsumableInspector.h"
#include "ConsumableElixirInspector.h"
#include "ConsumablePotionInspector.h"
#include "ContainerInspector.h"
#include "GemInspector.h"
#include "GenericItemInspector.h"
#include "GlyphInspector.h"
#include "ItemTemplate.h"
#include "KeyInspector.h"
#include "MiscellaneousItemInspector.h"
#include "MoneyInspector.h"
#include "PermanentItemInspector.h"
#include "ProjectItemInspector.h"
#include "QuestItemInspector.h"
#include "QuiverItemInspector.h"
#include "ReagentItemInspector.h"
#include "RecipeItemInspector.h"
#include "TradeGoodItemInspector.h"
#include "WeaponItemInspector.h"


class ManageInventoryAction : public Action
{
public:
    ManageInventoryAction(PlayerbotAI* botAI) : Action(botAI, "manage_inventory") {}

	static constexpr uint32_t ANY_SUBCLASS = std::numeric_limits<uint32_t>::max();

	bool isPossible() override
	{
		LOG_ERROR("playerbots", "Starting manage inventory isPossible evaluation");

		const Player* const bot = botAI->GetBot();

		if (bot == nullptr)
		{
			LOG_ERROR("playerbots", "Manage inventory impossible bot is nullptr");

			return false;
		}

		const std::string botName = bot->GetName();

		if (!bot->IsInWorld())
		{
			LOG_ERROR("playerbots", "Manage inventory impossible bot '{}' is non in world", botName);

			return false;
		}

		const WorldSession* const session = bot->GetSession();

		if (session == nullptr)
		{
			LOG_ERROR("playerbots", "Manage inventory impossible bot '{}' session is nullptr", botName);

			return false;
		}

		if (session->isLogingOut())
		{
			LOG_ERROR("playerbots", "Manage inventory impossible bot '{}' is logging out", botName);

			return false;
		}

		return true;
	}

    bool Execute(Event event) override;

private:

	std::unordered_map<uint64_t, ItemActionStruct> itemActions = {};

    void iterateBags();
	void iterateBag(const uint32_t bagSlot);
	void processItem(const uint64_t itemGUID);
	template <typename InspectorT>
	void determineItemAction(const uint32_t botLowGUID, const uint64_t itemLowGUID);
};
