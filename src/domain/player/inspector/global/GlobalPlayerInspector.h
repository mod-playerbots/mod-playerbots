#pragma once

#include <cstdint>

#include "ObjectAccessor.h"
#include "SharedDefines.h"
#include "Player.h"
#include "GridNotifiers.h"

#include "AiObjectContext.h"
#include "AiFactory.h"
#include "PlayerbotAI.h"
#include "AbstractPlayerInspector.h"
#include "PlayerbotMgr.h"

class GlobalPlayerInspector : public AbstractPlayerInspector
{
public:
    GlobalPlayerInspector(
		uint64_t playerGUID
	) :
	AbstractPlayerInspector(playerGUID)
	{}

	bool isDrinking() const
	{
		const Player* const player = this->getCurrentPlayer();

		if (player == nullptr)
			return false;

		if (!player->IsSitState())
			return false;

		return player->HasAuraType(SPELL_AURA_MOD_POWER_REGEN) || player->HasAuraType(SPELL_AURA_MOD_POWER_REGEN_PERCENT);
	}

	bool isEating() const
	{
		const Player* const player = this->getCurrentPlayer();

		if (player == nullptr)
			return false;

		if (!player->IsSitState())
			return false;

		return player->HasAuraType(SPELL_AURA_MOD_REGEN) || player->HasAuraType(SPELL_AURA_MOD_HEALTH_REGEN_PERCENT);
	}

	bool shouldBeDrinking(float manaThresholdPercent) const
	{
		Player* const player = this->getCurrentPlayer();

		if (player == nullptr)
			return false;

		if (!player->HasActivePowerType(POWER_MANA))
			return false;

		if (player->IsInCombat())
			return false;

		if (player->GetPowerPct(POWER_MANA) > manaThresholdPercent)
			return false;

		if (player->IsUnderWater())
			return false;

		// Bots must not keep drinking if they have hostile entities close to them!
		if (this->hasHostileUnitsInRange(15.0f))
			return false;

		PlayerbotAI* botAI = PlayerbotsMgr::instance()->GetPlayerbotAI(player);

		if (botAI == nullptr)
			return false;

		AiObjectContext* context = botAI->GetAiObjectContext();

		if (context == nullptr)
			return false;

		bool hasAvailableLoot = context->GetValue<bool>("loot target");

		if (hasAvailableLoot)
			return false;

		return true;
	}

	bool hasHostileUnitsInRange(float range) const
	{
		const Player* const player = this->getCurrentPlayer();

		if (player == nullptr)
			return false;

		std::list<Unit*> targets;

		Acore::AnyUnfriendlyAttackableVisibleUnitInObjectRangeCheck u_check(player, 15.0f);
		Acore::UnitListSearcher<Acore::AnyUnfriendlyAttackableVisibleUnitInObjectRangeCheck> searcher(player, targets, u_check);
		Cell::VisitObjects(player, searcher, PlayerbotAIConfig::instance()->sightDistance);

		return !targets.empty();
	}
};
