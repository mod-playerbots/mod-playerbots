#pragma once

#include <cstdint>

#include "ItemActionEnum.h"

struct ItemActionStruct
{
	const ItemActionEnum action;
	const uint8_t bagSlot;
	const uint8_t containerSlot;
	const uint32_t equipmentSlot;
};
