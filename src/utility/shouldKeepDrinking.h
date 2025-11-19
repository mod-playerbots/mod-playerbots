#ifndef SHOULD_KEEP_DRINKING_H
#define SHOULD_KEEP_DRINKING_H

#include "ServerFacade.h"
#include "Player.h"

#include "PlayerbotAI.h"
#include "UseItemAction.h"
#include "SharedDefines.h"

inline bool shouldKeepDrinking(PlayerbotAI* botAI)
{
	Player* bot = botAI->GetBot();

	if (!bot->HasActivePowerType(POWER_MANA))
		return false;

	if (bot->GetPowerPct(POWER_MANA) > 95.0f)
		return false;

	if (bot->IsUnderWater())
		return false;

	if (bot->IsInCombat())
		return false;

	Player* master = botAI->GetMaster();

	if (master && master->IsInWorld())
	{
		if (master->IsInCombat())
			return false;

		float distance = sServerFacade->GetDistance2d(bot, master);

		if (distance > 20.0f)
			return false;

		if (master->IsSitState())
			return true;
	}

	return true;
}

#endif
