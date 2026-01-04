#pragma once

#include "SharedDefines.h"
#include "ItemTemplate.h"
#include "SpellMgr.h"

#include "AbstractItemInspector.h"

class AbstractConsumableInspector : public AbstractItemInspector
{
public:
	bool isInspectable() const
	{
		const uint8_t itemClass = this->getCurrentItemClass();

		return itemClass == ITEM_CLASS_CONSUMABLE;
	}

	bool isHealthPotion() const
	{
		const ItemTemplate* const itemTemplate = this->getCurrentItemTemplate();

		if (itemTemplate == nullptr)
			return false;

		const SpellMgr* const spellMgr = SpellMgr::instance();

		if (spellMgr == nullptr)
			return false;

		const SpellInfo* const spellInfo = spellMgr->GetSpellInfo(itemTemplate->Spells[0].SpellId);

		if (spellInfo == nullptr)
			return false;

		if (spellInfo->HasEffect(SPELL_EFFECT_HEAL))
			return true;

		return false;
	}

	bool isManaPotion() const
	{
		const ItemTemplate* itemTemplate = this->getCurrentItemTemplate();

		if (itemTemplate == nullptr)
			return false;

		const SpellMgr* const spellMgr = SpellMgr::instance();

		if (spellMgr == nullptr)
			return false;

		const SpellInfo* const spellInfo = spellMgr->GetSpellInfo(itemTemplate->Spells[0].SpellId);

		if (spellInfo == nullptr)
			return false;

		if (spellInfo->HasEffect(SPELL_EFFECT_ENERGIZE) && spellInfo->PowerType == POWER_MANA)
			return true;

		return false;
	}

protected:
    AbstractConsumableInspector(
		uint64_t playerGUID,
		uint64_t itemGUID
	) :
	AbstractItemInspector(playerGUID, itemGUID)
	{}

	AbstractConsumableInspector& operator=(AbstractConsumableInspector const&) = delete;
};
