#pragma once

#include <cstdint>

#include "ItemActionEnum.h"

struct ItemActionStruct
{
	const ItemActionEnum action;
	const uint8_t inventorySlot;
	const uint32_t equipmentSlot;
};
