#pragma once

#include <cstdint>

#include "ObjectGuid.h"
#include "ObjectAccessor.h"
#include "SharedDefines.h"
#include "Player.h"

class AbstractPlayerFacade
{
public:
    AbstractPlayerFacade(
		uint64_t playerGUID
	) :
	playerGUID(playerGUID)
	{}

	AbstractPlayerFacade& operator=(AbstractPlayerFacade const&) = delete;

	Player* getCurrentPlayer() const
	{
		const ObjectGuid playerFullGUID = ObjectGuid::Create<HighGuid::Player>(this->playerGUID);
		Player* const player = ObjectAccessor::FindPlayer(playerFullGUID);

		return player;
	}

	bool isMutable() const
	{
		return true;
	}

protected:
	const uint64_t playerGUID;
};
