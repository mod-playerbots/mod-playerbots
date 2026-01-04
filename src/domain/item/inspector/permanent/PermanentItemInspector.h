#pragma once

#include <cstdint>

#include "SharedDefines.h"
#include "ItemTemplate.h"

#include "AbstractItemInspector.h"
#include "ItemActionStruct.h"
#include "ItemActionEnum.h"

class PermanentItemInspector : public AbstractItemInspector
{
public:
	PermanentItemInspector(
		uint64_t playerLowGUID,
		uint64_t itemLowGUID
	) : AbstractItemInspector(playerLowGUID, itemLowGUID)
	{}

	bool isInspectable() const
	{
		const uint8_t itemClass = this->getCurrentItemClass();

		return AbstractItemInspector::isInspectable() && itemClass == ITEM_CLASS_PERMANENT;
	}

	ItemActionStruct determineItemAction() const override
	{
		if (!this->isInspectable())
			return this->getDefaultItemAction();

		if (this->isForbiddenItem())
			return this->getForbiddenItemAction();

		return this->getSellAction();
	}

	bool isForbiddenItem() const
	{
		return true;
	}
};
